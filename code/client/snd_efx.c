#include "qal.h"
#include "snd_efx.h"

static qboolean efx_supported = qfalse;
static qboolean eaxReverbSupported = qfalse;
static ALuint reverbSlot = 0;
static ALuint reverbEffect = 0;
static int currentPreset = -1;

typedef struct {
	float gain;
	float decayTime;
	float decayHFRatio;
	float roomRolloff;
	float reflectionsGain;
	float reflectionsDelay;
	float lateReverbGain;
	float lateReverbDelay;
	float diffusion;
	float density;
	float airAbsorptionGainHF;

	// EAX specific
	float gainLF;
	float decayLFRatio;
	float echoTime;
	float echoDepth;
	float modulationTime;
	float modulationDepth;
	float hfReference;
	float lfReference;
} reverbParams_t;

static reverbParams_t currentParams;
static reverbParams_t targetParams;
static qboolean firstPresetSet = qtrue;

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

	// Probe for EAX REVERB support
	qalGetError(); // Clear error
	qalEffecti( reverbEffect, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB );
	if ( qalGetError() == AL_NO_ERROR ) {
		eaxReverbSupported = qtrue;
		Com_Printf( "OpenAL EFX: AL_EFFECT_EAXREVERB is supported.\n" );
	} else {
		eaxReverbSupported = qfalse;
		Com_Printf( "OpenAL EFX: AL_EFFECT_EAXREVERB is not supported, falling back to AL_EFFECT_REVERB.\n" );
	}

	efx_supported = qtrue;
	firstPresetSet = qtrue;
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
	firstPresetSet = qtrue;
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

	switch ( presetNum ) {
		case REVERB_PRESET_BUNKER:
			// Tight concrete space / metallic reflections - clearly audible
			targetParams.gain = 0.40f;
			targetParams.decayTime = 1.1f;
			targetParams.decayHFRatio = 0.3f;
			targetParams.roomRolloff = 0.15f;
			targetParams.reflectionsGain = 0.55f;
			targetParams.reflectionsDelay = 0.015f;
			targetParams.lateReverbGain = 0.80f;
			targetParams.lateReverbDelay = 0.022f;
			targetParams.diffusion = 0.75f;
			targetParams.density = 0.8f;
			targetParams.airAbsorptionGainHF = 0.99f;
			// EAX specific
			targetParams.gainLF = 0.35f;
			targetParams.decayLFRatio = 0.6f;
			targetParams.echoTime = 0.12f;
			targetParams.echoDepth = 0.15f;
			targetParams.modulationTime = 0.25f;
			targetParams.modulationDepth = 0.2f;
			targetParams.hfReference = 5000.0f;
			targetParams.lfReference = 250.0f;
			break;

		case REVERB_PRESET_ROOM:
			// Moderate room acoustics - clearly present but subtle
			targetParams.gain = 0.28f;
			targetParams.decayTime = 0.65f;
			targetParams.decayHFRatio = 0.5f;
			targetParams.roomRolloff = 0.20f;
			targetParams.reflectionsGain = 0.40f;
			targetParams.reflectionsDelay = 0.010f;
			targetParams.lateReverbGain = 0.60f;
			targetParams.lateReverbDelay = 0.015f;
			targetParams.diffusion = 0.80f;
			targetParams.density = 0.8f;
			targetParams.airAbsorptionGainHF = 0.99f;
			// EAX specific
			targetParams.gainLF = 0.25f;
			targetParams.decayLFRatio = 0.7f;
			targetParams.echoTime = 0.15f;
			targetParams.echoDepth = 0.05f;
			targetParams.modulationTime = 0.25f;
			targetParams.modulationDepth = 0.05f;
			targetParams.hfReference = 5000.0f;
			targetParams.lfReference = 250.0f;
			break;

		case REVERB_PRESET_LARGE_HALL:
			// Cathedral / Castle Hall - spacious reverb wash, clearly noticeable
			targetParams.gain = 0.50f;
			targetParams.decayTime = 2.2f;
			targetParams.decayHFRatio = 0.4f;
			targetParams.roomRolloff = 0.12f;
			targetParams.reflectionsGain = 0.65f;
			targetParams.reflectionsDelay = 0.030f;
			targetParams.lateReverbGain = 0.85f;
			targetParams.lateReverbDelay = 0.045f;
			targetParams.diffusion = 0.90f;
			targetParams.density = 0.8f;
			targetParams.airAbsorptionGainHF = 0.97f;
			// EAX specific
			targetParams.gainLF = 0.45f;
			targetParams.decayLFRatio = 0.8f;
			targetParams.echoTime = 0.25f;
			targetParams.echoDepth = 0.1f;
			targetParams.modulationTime = 0.25f;
			targetParams.modulationDepth = 0.15f;
			targetParams.hfReference = 5000.0f;
			targetParams.lfReference = 250.0f;
			break;

		case REVERB_PRESET_CRYPT:
			// Subterranean damp crypt - dark, long-tailed decay
			targetParams.gain = 0.45f;
			targetParams.decayTime = 3.2f;
			targetParams.decayHFRatio = 0.25f;
			targetParams.roomRolloff = 0.10f;
			targetParams.reflectionsGain = 0.60f;
			targetParams.reflectionsDelay = 0.040f;
			targetParams.lateReverbGain = 0.90f;
			targetParams.lateReverbDelay = 0.060f;
			targetParams.diffusion = 0.85f;
			targetParams.density = 0.8f;
			targetParams.airAbsorptionGainHF = 0.95f;
			// EAX specific
			targetParams.gainLF = 0.40f;
			targetParams.decayLFRatio = 0.9f;
			targetParams.echoTime = 0.3f;
			targetParams.echoDepth = 0.2f;
			targetParams.modulationTime = 0.25f;
			targetParams.modulationDepth = 0.25f;
			targetParams.hfReference = 5000.0f;
			targetParams.lfReference = 250.0f;
			break;

		case REVERB_PRESET_TUNNEL:
			// Tunnel corridor
			targetParams.gain = 0.38f;
			targetParams.decayTime = 1.4f;
			targetParams.decayHFRatio = 0.35f;
			targetParams.roomRolloff = 0.15f;
			targetParams.reflectionsGain = 0.70f;
			targetParams.reflectionsDelay = 0.010f;
			targetParams.lateReverbGain = 0.75f;
			targetParams.lateReverbDelay = 0.020f;
			targetParams.diffusion = 0.70f;
			targetParams.density = 0.8f;
			targetParams.airAbsorptionGainHF = 0.99f;
			// EAX specific
			targetParams.gainLF = 0.30f;
			targetParams.decayLFRatio = 0.6f;
			targetParams.echoTime = 0.2f;
			targetParams.echoDepth = 0.3f;
			targetParams.modulationTime = 0.25f;
			targetParams.modulationDepth = 0.1f;
			targetParams.hfReference = 5000.0f;
			targetParams.lfReference = 250.0f;
			break;

		case REVERB_PRESET_SEWER:
			// Sewer wet bricks
			targetParams.gain = 0.35f;
			targetParams.decayTime = 1.2f;
			targetParams.decayHFRatio = 0.30f;
			targetParams.roomRolloff = 0.20f;
			targetParams.reflectionsGain = 0.50f;
			targetParams.reflectionsDelay = 0.012f;
			targetParams.lateReverbGain = 0.75f;
			targetParams.lateReverbDelay = 0.025f;
			targetParams.diffusion = 0.80f;
			targetParams.density = 0.8f;
			targetParams.airAbsorptionGainHF = 0.96f;
			// EAX specific
			targetParams.gainLF = 0.35f;
			targetParams.decayLFRatio = 0.7f;
			targetParams.echoTime = 0.25f;
			targetParams.echoDepth = 0.25f;
			targetParams.modulationTime = 0.25f;
			targetParams.modulationDepth = 0.2f;
			targetParams.hfReference = 5000.0f;
			targetParams.lfReference = 250.0f;
			break;

		case REVERB_PRESET_DEPOT:
			// Depot hangar / factory floor
			targetParams.gain = 0.48f;
			targetParams.decayTime = 2.6f;
			targetParams.decayHFRatio = 0.45f;
			targetParams.roomRolloff = 0.12f;
			targetParams.reflectionsGain = 0.55f;
			targetParams.reflectionsDelay = 0.035f;
			targetParams.lateReverbGain = 0.80f;
			targetParams.lateReverbDelay = 0.050f;
			targetParams.diffusion = 0.85f;
			targetParams.density = 0.8f;
			targetParams.airAbsorptionGainHF = 0.96f;
			// EAX specific
			targetParams.gainLF = 0.40f;
			targetParams.decayLFRatio = 0.8f;
			targetParams.echoTime = 0.22f;
			targetParams.echoDepth = 0.15f;
			targetParams.modulationTime = 0.25f;
			targetParams.modulationDepth = 0.1f;
			targetParams.hfReference = 5000.0f;
			targetParams.lfReference = 250.0f;
			break;

		case REVERB_PRESET_OUTDOORS:
		default:
			// Subtle open air reverb (not completely silent, for realism)
			targetParams.gain = 0.05f;
			targetParams.decayTime = 0.40f;
			targetParams.decayHFRatio = 0.5f;
			targetParams.roomRolloff = 0.0f;
			targetParams.reflectionsGain = 0.05f;
			targetParams.reflectionsDelay = 0.005f;
			targetParams.lateReverbGain = 0.05f;
			targetParams.lateReverbDelay = 0.01f;
			targetParams.diffusion = 0.90f;
			targetParams.density = 0.8f;
			targetParams.airAbsorptionGainHF = 0.99f;
			// EAX specific
			targetParams.gainLF = 0.05f;
			targetParams.decayLFRatio = 0.5f;
			targetParams.echoTime = 0.25f;
			targetParams.echoDepth = 0.0f;
			targetParams.modulationTime = 0.25f;
			targetParams.modulationDepth = 0.0f;
			targetParams.hfReference = 5000.0f;
			targetParams.lfReference = 250.0f;
			break;
	}

	if ( firstPresetSet ) {
		currentParams = targetParams;
		firstPresetSet = qfalse;
		S_EFX_Update(); // apply immediately on start/init
	}
}

void S_EFX_AttachSource(unsigned int alSource) {
	if ( !efx_supported || !alSource ) {
		return;
	}

	// Attach source to auxiliary slot so it sends audio to the reverb effect
	qalSource3i( alSource, AL_AUXILIARY_SEND_FILTER, reverbSlot, 0, AL_FILTER_NULL );
}

void S_EFX_Update(void) {
	static int lastTime = 0;
	int now = Sys_Milliseconds();
	int dt = now - lastTime;
	if (dt < 0) dt = 0;
	lastTime = now;

	if ( !efx_supported || !reverbEffect || !reverbSlot ) {
		return;
	}

	// Interpolate currentParams towards targetParams over ~300ms
	// If ratio is >= 1.0, currentParams gets snapped to targetParams
	float ratio = (float)dt / 300.0f;
	if (ratio > 1.0f) ratio = 1.0f;
	if (ratio < 0.0f) ratio = 0.0f;

	#define INTERP(x) currentParams.x += (targetParams.x - currentParams.x) * ratio

	INTERP(gain);
	INTERP(decayTime);
	INTERP(decayHFRatio);
	INTERP(roomRolloff);
	INTERP(reflectionsGain);
	INTERP(reflectionsDelay);
	INTERP(lateReverbGain);
	INTERP(lateReverbDelay);
	INTERP(diffusion);
	INTERP(density);
	INTERP(airAbsorptionGainHF);

	if ( eaxReverbSupported ) {
		INTERP(gainLF);
		INTERP(decayLFRatio);
		INTERP(echoTime);
		INTERP(echoDepth);
		INTERP(modulationTime);
		INTERP(modulationDepth);
		INTERP(hfReference);
		INTERP(lfReference);
	}
	#undef INTERP

	if ( eaxReverbSupported ) {
		qalEffecti( reverbEffect, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB );
		qalEffectf( reverbEffect, AL_EAXREVERB_GAIN, currentParams.gain );
		qalEffectf( reverbEffect, AL_EAXREVERB_DECAY_TIME, currentParams.decayTime );
		qalEffectf( reverbEffect, AL_EAXREVERB_DECAY_HFRATIO, currentParams.decayHFRatio );
		qalEffectf( reverbEffect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, currentParams.roomRolloff );
		qalEffectf( reverbEffect, AL_EAXREVERB_REFLECTIONS_GAIN, currentParams.reflectionsGain );
		qalEffectf( reverbEffect, AL_EAXREVERB_REFLECTIONS_DELAY, currentParams.reflectionsDelay );
		qalEffectf( reverbEffect, AL_EAXREVERB_LATE_REVERB_GAIN, currentParams.lateReverbGain );
		qalEffectf( reverbEffect, AL_EAXREVERB_LATE_REVERB_DELAY, currentParams.lateReverbDelay );
		qalEffectf( reverbEffect, AL_EAXREVERB_DIFFUSION, currentParams.diffusion );
		qalEffectf( reverbEffect, AL_EAXREVERB_DENSITY, currentParams.density );
		qalEffectf( reverbEffect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, currentParams.airAbsorptionGainHF );

		qalEffectf( reverbEffect, AL_EAXREVERB_GAINLF, currentParams.gainLF );
		qalEffectf( reverbEffect, AL_EAXREVERB_DECAY_LFRATIO, currentParams.decayLFRatio );
		qalEffectf( reverbEffect, AL_EAXREVERB_ECHO_TIME, currentParams.echoTime );
		qalEffectf( reverbEffect, AL_EAXREVERB_ECHO_DEPTH, currentParams.echoDepth );
		qalEffectf( reverbEffect, AL_EAXREVERB_MODULATION_TIME, currentParams.modulationTime );
		qalEffectf( reverbEffect, AL_EAXREVERB_MODULATION_DEPTH, currentParams.modulationDepth );
		qalEffectf( reverbEffect, AL_EAXREVERB_HFREFERENCE, currentParams.hfReference );
		qalEffectf( reverbEffect, AL_EAXREVERB_LFREFERENCE, currentParams.lfReference );
	} else {
		qalEffecti( reverbEffect, AL_EFFECT_TYPE, AL_EFFECT_REVERB );
		qalEffectf( reverbEffect, AL_REVERB_GAIN, currentParams.gain );
		qalEffectf( reverbEffect, AL_REVERB_DECAY_TIME, currentParams.decayTime );
		qalEffectf( reverbEffect, AL_REVERB_DECAY_HFRATIO, currentParams.decayHFRatio );
		qalEffectf( reverbEffect, AL_REVERB_ROOM_ROLLOFF_FACTOR, currentParams.roomRolloff );
		qalEffectf( reverbEffect, AL_REVERB_REFLECTIONS_GAIN, currentParams.reflectionsGain );
		qalEffectf( reverbEffect, AL_REVERB_REFLECTIONS_DELAY, currentParams.reflectionsDelay );
		qalEffectf( reverbEffect, AL_REVERB_LATE_REVERB_GAIN, currentParams.lateReverbGain );
		qalEffectf( reverbEffect, AL_REVERB_LATE_REVERB_DELAY, currentParams.lateReverbDelay );
		qalEffectf( reverbEffect, AL_REVERB_DIFFUSION, currentParams.diffusion );
		qalEffectf( reverbEffect, AL_REVERB_DENSITY, currentParams.density );
		qalEffectf( reverbEffect, AL_REVERB_AIR_ABSORPTION_GAINHF, currentParams.airAbsorptionGainHF );
	}

	qalAuxiliaryEffectSloti( reverbSlot, AL_EFFECTSLOT_EFFECT, reverbEffect );
}

void S_EFX_UpdateGain(float gain) {
	if ( !efx_supported || !reverbSlot ) {
		return;
	}

	if ( gain < 0.0f ) gain = 0.0f;
	if ( gain > 2.0f ) gain = 2.0f;

	if ( qalAuxiliaryEffectSlotf ) {
		qalAuxiliaryEffectSlotf( reverbSlot, AL_EFFECTSLOT_GAIN, gain );
	}
}
