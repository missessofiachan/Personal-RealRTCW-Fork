#include "qal.h"
#include "snd_efx.h"

static qboolean efx_supported = qfalse;
static ALuint reverbSlot = 0;
static ALuint reverbEffect = 0;
static int currentPreset = -1;

void S_EFX_Init(void) {
	ALenum err;

	QAL_InitEFX();

	// Check if pointers were successfully loaded
	if ( !qalGenEffects || !qalGenAuxiliaryEffectSlots || !qalEffecti || !qalEffectf || !qalAuxiliaryEffectSloti ) {
		efx_supported = qfalse;
		Com_Printf( "OpenAL EFX functions not available.\n" );
		return;
	}

	qalGetError(); // Clear error

	qalGenAuxiliaryEffectSlots( 1, &reverbSlot );
	err = qalGetError();
	if ( err != AL_NO_ERROR ) {
		Com_Printf( "Failed to generate EFX auxiliary slot: %d\n", err );
		efx_supported = qfalse;
		return;
	}

	// Enable automatic panning/distance cue processing for auxiliary sends
	// to prevent overwhelming close-up/far-away volume feedback.
	qalAuxiliaryEffectSloti( reverbSlot, AL_EFFECTSLOT_AUXILIARY_SEND_AUTO, AL_TRUE );

	qalGenEffects( 1, &reverbEffect );
	err = qalGetError();
	if ( err != AL_NO_ERROR ) {
		qalDeleteAuxiliaryEffectSlots( 1, &reverbSlot );
		reverbSlot = 0;
		Com_Printf( "Failed to generate EFX effect: %d\n", err );
		efx_supported = qfalse;
		return;
	}

	efx_supported = qtrue;
	Com_Printf( "OpenAL EFX Environmental Reverb initialized successfully.\n" );

	S_EFX_SetPreset( REVERB_PRESET_OUTDOORS );
}

void S_EFX_Shutdown(void) {
	if ( !efx_supported ) {
		return;
	}

	if ( reverbSlot ) {
		qalDeleteAuxiliaryEffectSlots( 1, &reverbSlot );
		reverbSlot = 0;
	}

	if ( reverbEffect ) {
		qalDeleteEffects( 1, &reverbEffect );
		reverbEffect = 0;
	}

	efx_supported = qfalse;
	currentPreset = -1;
}

void S_EFX_SetPreset(int presetNum) {
	if ( !efx_supported ) {
		return;
	}

	if ( currentPreset == presetNum ) {
		return; // No change
	}

	currentPreset = presetNum;

	if (presetNum >= 0 && presetNum <= 7) {
		const char* presetNames[] = { "NONE", "BUNKER", "ROOM", "LARGE HALL", "CRYPT", "TUNNEL", "SEWER", "DEPOT" };
		Com_Printf( "^5EFX Reverb Activated: ^7%s\n", presetNames[presetNum] );
	}

	// Set effect type to Reverb
	qalEffecti( reverbEffect, AL_EFFECT_TYPE, AL_EFFECT_REVERB );

	switch ( presetNum ) {
		case REVERB_PRESET_BUNKER:
			// Tight concrete space / metallic reflections - Comfort Tuned (Warm & Cozy)
			qalEffectf( reverbEffect, AL_REVERB_GAIN, 0.18f );
			qalEffectf( reverbEffect, AL_REVERB_DECAY_TIME, 1.1f );
			qalEffectf( reverbEffect, AL_REVERB_DECAY_HFRATIO, 0.3f ); // Low HF ratio = warm, soft echo, no harsh metallic piercing
			qalEffectf( reverbEffect, AL_REVERB_ROOM_ROLLOFF_FACTOR, 0.15f );
			qalEffectf( reverbEffect, AL_REVERB_REFLECTIONS_GAIN, 0.35f );
			qalEffectf( reverbEffect, AL_REVERB_REFLECTIONS_DELAY, 0.015f );
			qalEffectf( reverbEffect, AL_REVERB_LATE_REVERB_GAIN, 0.45f );
			qalEffectf( reverbEffect, AL_REVERB_LATE_REVERB_DELAY, 0.022f );
			break;

		case REVERB_PRESET_ROOM:
			// Moderate room acoustics - Comfort Tuned (Subtle atmosphere)
			qalEffectf( reverbEffect, AL_REVERB_GAIN, 0.10f );
			qalEffectf( reverbEffect, AL_REVERB_DECAY_TIME, 0.65f );
			qalEffectf( reverbEffect, AL_REVERB_DECAY_HFRATIO, 0.5f );
			qalEffectf( reverbEffect, AL_REVERB_ROOM_ROLLOFF_FACTOR, 0.20f );
			qalEffectf( reverbEffect, AL_REVERB_REFLECTIONS_GAIN, 0.25f );
			qalEffectf( reverbEffect, AL_REVERB_REFLECTIONS_DELAY, 0.010f );
			qalEffectf( reverbEffect, AL_REVERB_LATE_REVERB_GAIN, 0.35f );
			qalEffectf( reverbEffect, AL_REVERB_LATE_REVERB_DELAY, 0.015f );
			break;

		case REVERB_PRESET_LARGE_HALL:
			// Cathedral / Castle Hall acoustics (crypts) - Comfort Tuned (Spacious but soft)
			qalEffectf( reverbEffect, AL_REVERB_GAIN, 0.22f );
			qalEffectf( reverbEffect, AL_REVERB_DECAY_TIME, 2.2f );
			qalEffectf( reverbEffect, AL_REVERB_DECAY_HFRATIO, 0.4f ); // Softened highs to prevent ear fatigue over long play
			qalEffectf( reverbEffect, AL_REVERB_ROOM_ROLLOFF_FACTOR, 0.12f );
			qalEffectf( reverbEffect, AL_REVERB_REFLECTIONS_GAIN, 0.45f );
			qalEffectf( reverbEffect, AL_REVERB_REFLECTIONS_DELAY, 0.030f );
			qalEffectf( reverbEffect, AL_REVERB_LATE_REVERB_GAIN, 0.55f );
			qalEffectf( reverbEffect, AL_REVERB_LATE_REVERB_DELAY, 0.045f );
			break;

		case REVERB_PRESET_CRYPT:
			// Subterranean damp crypt / catacombs - Dark, atmospheric, long decay but deeply muffled highs
			qalEffectf( reverbEffect, AL_REVERB_GAIN, 0.20f );
			qalEffectf( reverbEffect, AL_REVERB_DECAY_TIME, 3.2f );
			qalEffectf( reverbEffect, AL_REVERB_DECAY_HFRATIO, 0.25f ); // Extremely low HF decay ratio = zero high pitch ring
			qalEffectf( reverbEffect, AL_REVERB_ROOM_ROLLOFF_FACTOR, 0.10f );
			qalEffectf( reverbEffect, AL_REVERB_REFLECTIONS_GAIN, 0.40f );
			qalEffectf( reverbEffect, AL_REVERB_REFLECTIONS_DELAY, 0.040f );
			qalEffectf( reverbEffect, AL_REVERB_LATE_REVERB_GAIN, 0.60f );
			qalEffectf( reverbEffect, AL_REVERB_LATE_REVERB_DELAY, 0.060f );
			break;

		case REVERB_PRESET_TUNNEL:
			// Flutter echo of brick/concrete corridor
			qalEffectf( reverbEffect, AL_REVERB_GAIN, 0.18f );
			qalEffectf( reverbEffect, AL_REVERB_DECAY_TIME, 1.4f );
			qalEffectf( reverbEffect, AL_REVERB_DECAY_HFRATIO, 0.35f );
			qalEffectf( reverbEffect, AL_REVERB_ROOM_ROLLOFF_FACTOR, 0.15f );
			qalEffectf( reverbEffect, AL_REVERB_REFLECTIONS_GAIN, 0.55f ); // High reflections = bouncing off tunnel walls
			qalEffectf( reverbEffect, AL_REVERB_REFLECTIONS_DELAY, 0.010f );
			qalEffectf( reverbEffect, AL_REVERB_LATE_REVERB_GAIN, 0.45f );
			qalEffectf( reverbEffect, AL_REVERB_LATE_REVERB_DELAY, 0.020f );
			break;

		case REVERB_PRESET_SEWER:
			// Wet brick sewer arches - Damped liquid-like echoes
			qalEffectf( reverbEffect, AL_REVERB_GAIN, 0.15f );
			qalEffectf( reverbEffect, AL_REVERB_DECAY_TIME, 1.2f );
			qalEffectf( reverbEffect, AL_REVERB_DECAY_HFRATIO, 0.30f );
			qalEffectf( reverbEffect, AL_REVERB_ROOM_ROLLOFF_FACTOR, 0.20f );
			qalEffectf( reverbEffect, AL_REVERB_REFLECTIONS_GAIN, 0.30f );
			qalEffectf( reverbEffect, AL_REVERB_REFLECTIONS_DELAY, 0.012f );
			qalEffectf( reverbEffect, AL_REVERB_LATE_REVERB_GAIN, 0.50f );
			qalEffectf( reverbEffect, AL_REVERB_LATE_REVERB_DELAY, 0.025f );
			break;

		case REVERB_PRESET_DEPOT:
			// Large empty depot hangar / factory floors
			qalEffectf( reverbEffect, AL_REVERB_GAIN, 0.22f );
			qalEffectf( reverbEffect, AL_REVERB_DECAY_TIME, 2.6f );
			qalEffectf( reverbEffect, AL_REVERB_DECAY_HFRATIO, 0.45f );
			qalEffectf( reverbEffect, AL_REVERB_ROOM_ROLLOFF_FACTOR, 0.12f );
			qalEffectf( reverbEffect, AL_REVERB_REFLECTIONS_GAIN, 0.35f );
			qalEffectf( reverbEffect, AL_REVERB_REFLECTIONS_DELAY, 0.035f );
			qalEffectf( reverbEffect, AL_REVERB_LATE_REVERB_GAIN, 0.50f );
			qalEffectf( reverbEffect, AL_REVERB_LATE_REVERB_DELAY, 0.050f );
			break;

		case REVERB_PRESET_OUTDOORS:
		default:
			// Subtle open air / no reverb
			qalEffectf( reverbEffect, AL_REVERB_GAIN, 0.0f );
			qalEffectf( reverbEffect, AL_REVERB_DECAY_TIME, 0.1f );
			qalEffectf( reverbEffect, AL_REVERB_DECAY_HFRATIO, 0.3f );
			qalEffectf( reverbEffect, AL_REVERB_ROOM_ROLLOFF_FACTOR, 0.0f );
			qalEffectf( reverbEffect, AL_REVERB_REFLECTIONS_GAIN, 0.0f );
			qalEffectf( reverbEffect, AL_REVERB_REFLECTIONS_DELAY, 0.005f );
			qalEffectf( reverbEffect, AL_REVERB_LATE_REVERB_GAIN, 0.0f );
			qalEffectf( reverbEffect, AL_REVERB_LATE_REVERB_DELAY, 0.01f );
			break;
	}

	// Bind effect to slot
	qalAuxiliaryEffectSloti( reverbSlot, AL_EFFECTSLOT_EFFECT, reverbEffect );
}

void S_EFX_AttachSource(unsigned int alSource) {
	if ( !efx_supported || !alSource ) {
		return;
	}

	// Attach source to auxiliary slot so it sends audio to the reverb effect
	qalSource3i( alSource, AL_AUXILIARY_SEND_FILTER, reverbSlot, 0, AL_FILTER_NULL );
}
