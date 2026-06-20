#include "cg_local.h"
#include "cg_flashlight.h"

qboolean cg_flashlightOn = qfalse;
float cg_flashlightEnergy = 1.0f; // range 0.0f to 1.0f

void CG_setClientFlags( void );

void CG_Flashlight_Toggle_f( void ) {
	if ( !cg_flashlightOn && cg_flashlightEnergy < 0.10f ) {
		// Prevent turning on when battery is depleted (less than 10%)
		return;
	}

	cg_flashlightOn = !cg_flashlightOn;

	// Only play sound and update flags if we are fully loaded in-game
	if ( cg.snap ) {
		if ( cgs.media.selectSound ) {
			trap_S_StartLocalSound( cgs.media.selectSound, CHAN_LOCAL );
		}
		CG_setClientFlags();
	}
}

void CG_Flashlight_Frame( void ) {
	trace_t tr;
	vec3_t end;
	float distance;
	float spotSize;

	if ( !cg.snap ) {
		return;
	}

	// 1. Drain battery if ON, recharge if OFF
	if ( cg_flashlightOn ) {
		cg_flashlightEnergy -= (float)cg.frametime / 20000.0f; // drains completely in 20 seconds
		if ( cg_flashlightEnergy <= 0.0f ) {
			cg_flashlightEnergy = 0.0f;
			cg_flashlightOn = qfalse;
			if ( cgs.media.selectSound ) {
				trap_S_StartLocalSound( cgs.media.selectSound, CHAN_LOCAL );
			}
			CG_setClientFlags();
		}
	} else {
		cg_flashlightEnergy += (float)cg.frametime / 10000.0f; // recharges fully in 10 seconds
		if ( cg_flashlightEnergy > 1.0f ) {
			cg_flashlightEnergy = 1.0f;
		}
	}

	if ( !cg_flashlightOn ) {
		return;
	}

	// 2. Project the primary spot light where the player is looking
	VectorMA( cg.refdef.vieworg, 600, cg.refdef.viewaxis[0], end );
	CG_Trace( &tr, cg.refdef.vieworg, NULL, NULL, end, cg.predictedPlayerState.clientNum, MASK_SOLID );

	distance = tr.fraction * 600.0f;
	spotSize = 280.0f;
	if ( distance > 100.0f ) {
		spotSize = 220.0f + ( distance * 0.07f );
	}

	// WWII Halogen/Filament warm incandescent color temperature (solid, steady light)
	// 1. Concentrated bright hotspot (inner cone)
	trap_R_AddLightToScene( tr.endpos, spotSize * 0.5f, 1.0f, 0.88f, 0.70f, 0 );

	// 2. Wide, dim ambient spill (outer cone)
	trap_R_AddLightToScene( tr.endpos, spotSize * 1.6f, 0.35f, 0.30f, 0.22f, 0 );
}

void CG_Flashlight_DrawHUD( void ) {
	vec4_t color = { 1.0f, 0.6f, 0.1f, 0.8f };       // Bright HL2-style orange/yellow
	vec4_t bgcolor = { 0.2f, 0.2f, 0.2f, 0.4f };     // Translucent dark gray background
	vec4_t bordercolor = { 1.0f, 0.6f, 0.1f, 0.25f }; // Subtle orange border

	// Don't draw if flashlight is off and battery is fully charged (keeps HUD clean)
	if ( !cg_flashlightOn && cg_flashlightEnergy >= 1.0f ) {
		return;
	}

	if ( !cg.snap || cg.snap->ps.stats[STAT_HEALTH] <= 0 || cg.cameraMode ) {
		return;
	}

	// Draw "FLASHLIGHT" label (virtual 640x480 screen coordinates)
	CG_DrawStringExt( 510, 420, "FLASHLIGHT", color, qfalse, qtrue, 6, 9, 0 );

	// Draw bar border
	CG_DrawRect( 509, 431, 102, 8, 1, bordercolor );

	// Draw bar background
	CG_FillRect( 510, 432, 100, 6, bgcolor );

	// Draw bar fill
	CG_FillRect( 510, 432, (int)(100.0f * cg_flashlightEnergy), 6, color );
}
