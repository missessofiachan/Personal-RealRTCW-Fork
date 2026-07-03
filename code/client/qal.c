/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2005 Stuart Dalton (badcdev@gmail.com)
Copyright (C) 2026

This file is part of Quake III Arena / RealRTCW source code.
===========================================================================
*/

#ifdef USE_OPENAL
#ifdef USE_OPENAL_DLOPEN

#include "qal.h"
#include "../sys/sys_loadlib.h"

static void *OpenALLib = NULL;
static qboolean alinit_fail = qfalse;

// ===========================================================================
// X-MACRO LIST DEFINITIONS
// ===========================================================================

#define AL_CORE_FUNCTIONS \
    X(LPALENABLE, qalEnable, "alEnable") \
    X(LPALDISABLE, qalDisable, "alDisable") \
    X(LPALISENABLED, qalIsEnabled, "alIsEnabled") \
    X(LPALGETSTRING, qalGetString, "alGetString") \
    X(LPALGETBOOLEANV, qalGetBooleanv, "alGetBooleanv") \
    X(LPALGETINTEGERV, qalGetIntegerv, "alGetIntegerv") \
    X(LPALGETFLOATV, qalGetFloatv, "alGetFloatv") \
    X(LPALGETDOUBLEV, qalGetDoublev, "alGetDoublev") \
    X(LPALGETBOOLEAN, qalGetBoolean, "alGetBoolean") \
    X(LPALGETINTEGER, qalGetInteger, "alGetInteger") \
    X(LPALGETFLOAT, qalGetFloat, "alGetFloat") \
    X(LPALGETDOUBLE, qalGetDouble, "alGetDouble") \
    X(LPALGETERROR, qalGetError, "alGetError") \
    X(LPALISEXTENSIONPRESENT, qalIsExtensionPresent, "alIsExtensionPresent") \
    X(LPALGETPROCADDRESS, qalGetProcAddress, "alGetProcAddress") \
    X(LPALGETENUMVALUE, qalGetEnumValue, "alGetEnumValue") \
    X(LPALLISTENERF, qalListenerf, "alListenerf") \
    X(LPALLISTENER3F, qalListener3f, "alListener3f") \
    X(LPALLISTENERFV, qalListenerfv, "alListenerfv") \
    X(LPALLISTENERI, qalListeneri, "alListeneri") \
    X(LPALGETLISTENERF, qalGetListenerf, "alGetListenerf") \
    X(LPALGETLISTENER3F, qalGetListener3f, "alGetListener3f") \
    X(LPALGETLISTENERFV, qalGetListenerfv, "alGetListenerfv") \
    X(LPALGETLISTENERI, qalGetListeneri, "alGetListeneri") \
    X(LPALGENSOURCES, qalGenSources, "alGenSources") \
    X(LPALDELETESOURCES, qalDeleteSources, "alDeleteSources") \
    X(LPALISSOURCE, qalIsSource, "alIsSource") \
    X(LPALSOURCEF, qalSourcef, "alSourcef") \
    X(LPALSOURCE3F, qalSource3f, "alSource3f") \
    X(LPALSOURCEFV, qalSourcefv, "alSourcefv") \
    X(LPALSOURCEI, qalSourcei, "alSourcei") \
    X(LPALSOURCE3I, qalSource3i, "alSource3i") \
    X(LPALGETSOURCEF, qalGetSourcef, "alGetSourcef") \
    X(LPALGETSOURCE3F, qalGetSource3f, "alGetSource3f") \
    X(LPALGETSOURCEFV, qalGetSourcefv, "alGetSourcefv") \
    X(LPALGETSOURCEI, qalGetSourcei, "alGetSourcei") \
    X(LPALSOURCEPLAYV, qalSourcePlayv, "alSourcePlayv") \
    X(LPALSOURCESTOPV, qalSourceStopv, "alSourceStopv") \
    X(LPALSOURCEREWINDV, qalSourceRewindv, "alSourceRewindv") \
    X(LPALSOURCEPAUSEV, qalSourcePausev, "alSourcePausev") \
    X(LPALSOURCEPLAY, qalSourcePlay, "alSourcePlay") \
    X(LPALSOURCESTOP, qalSourceStop, "alSourceStop") \
    X(LPALSOURCEREWIND, qalSourceRewind, "alSourceRewind") \
    X(LPALSOURCEPAUSE, qalSourcePause, "alSourcePause") \
    X(LPALSOURCEQUEUEBUFFERS, qalSourceQueueBuffers, "alSourceQueueBuffers") \
    X(LPALSOURCEUNQUEUEBUFFERS, qalSourceUnqueueBuffers, "alSourceUnqueueBuffers") \
    X(LPALGENBUFFERS, qalGenBuffers, "alGenBuffers") \
    X(LPALDELETEBUFFERS, qalDeleteBuffers, "alDeleteBuffers") \
    X(LPALISBUFFER, qalIsBuffer, "alIsBuffer") \
    X(LPALBUFFERDATA, qalBufferData, "alBufferData") \
    X(LPALGETBUFFERF, qalGetBufferf, "alGetBufferf") \
    X(LPALGETBUFFERI, qalGetBufferi, "alGetBufferi") \
    X(LPALDOPPLERFACTOR, qalDopplerFactor, "alDopplerFactor") \
    X(LPALSPEEDOFSOUND, qalSpeedOfSound, "alSpeedOfSound") \
    X(LPALDISTANCEMODEL, qalDistanceModel, "alDistanceModel")

#define ALC_CORE_FUNCTIONS \
    X(LPALCCREATECONTEXT, qalcCreateContext, "alcCreateContext") \
    X(LPALCMAKECONTEXTCURRENT, qalcMakeContextCurrent, "alcMakeContextCurrent") \
    X(LPALCPROCESSCONTEXT, qalcProcessContext, "alcProcessContext") \
    X(LPALCSUSPENDCONTEXT, qalcSuspendContext, "alcSuspendContext") \
    X(LPALCDESTROYCONTEXT, qalcDestroyContext, "alcDestroyContext") \
    X(LPALCGETCURRENTCONTEXT, qalcGetCurrentContext, "alcGetCurrentContext") \
    X(LPALCGETCONTEXTSDEVICE, qalcGetContextsDevice, "alcGetContextsDevice") \
    X(LPALCOPENDEVICE, qalcOpenDevice, "alcOpenDevice") \
    X(LPALCCLOSEDEVICE, qalcCloseDevice, "alcCloseDevice") \
    X(LPALCGETERROR, qalcGetError, "alcGetError") \
    X(LPALCISEXTENSIONPRESENT, qalcIsExtensionPresent, "alcIsExtensionPresent") \
    X(LPALCGETPROCADDRESS, qalcGetProcAddress, "alcGetProcAddress") \
    X(LPALCGETENUMVALUE, qalcGetEnumValue, "alcGetEnumValue") \
    X(LPALCGETSTRING, qalcGetString, "alcGetString") \
    X(LPALCGETINTEGERV, qalcGetIntegerv, "alcGetIntegerv") \
    X(LPALCCAPTUREOPENDEVICE, qalcCaptureOpenDevice, "alcCaptureOpenDevice") \
    X(LPALCCAPTURECLOSEDEVICE, qalcCaptureCloseDevice, "alcCaptureCloseDevice") \
    X(LPALCCAPTURESTART, qalcCaptureStart, "alcCaptureStart") \
    X(LPALCCAPTURESTOP, qalcCaptureStop, "alcCaptureStop") \
    X(LPALCCAPTURESAMPLES, qalcCaptureSamples, "alcCaptureSamples")

#define AL_EFX_FUNCTIONS \
    X(LPALGENEFFECTS, qalGenEffects, "alGenEffects") \
    X(LPALDELETEEFFECTS, qalDeleteEffects, "alDeleteEffects") \
    X(LPALISEFFECT, qalIsEffect, "alIsEffect") \
    X(LPALEFFECTI, qalEffecti, "alEffecti") \
    X(LPALEFFECTF, qalEffectf, "alEffectf") \
    X(LPALEFFECTFV, qalEffectfv, "alEffectfv") \
    X(LPALGENAUXILIARYEFFECTSLOTS, qalGenAuxiliaryEffectSlots, "alGenAuxiliaryEffectSlots") \
    X(LPALDELETEAUXILIARYEFFECTSLOTS, qalDeleteAuxiliaryEffectSlots, "alDeleteAuxiliaryEffectSlots") \
    X(LPALAUXILIARYEFFECTSLOTI, qalAuxiliaryEffectSloti, "alAuxiliaryEffectSloti") \
    X(LPALAUXILIARYEFFECTSLOTF, qalAuxiliaryEffectSlotf, "alAuxiliaryEffectSlotf") \
    X(LPALAUXILIARYEFFECTSLOTFV, qalAuxiliaryEffectSlotfv, "alAuxiliaryEffectSlotfv") \
    X(LPALGENFILTERS, qalGenFilters, "alGenFilters") \
    X(LPALDELETEFILTERS, qalDeleteFilters, "alDeleteFilters") \
    X(LPALISFILTER, qalIsFilter, "alIsFilter") \
    X(LPALFILTERI, qalFilteri, "alFilteri") \
    X(LPALFILTERF, qalFilterf, "alFilterf")

#define AL_HRTF_FUNCTIONS \
    X(LPALCRESETDEVICESOFT, qalcResetDeviceSOFT, "alcResetDeviceSOFT") \
    X(LPALCGETSTRINGISOFT, qalcGetStringiSOFT, "alcGetStringiSOFT")

// ===========================================================================
// GLOBAL FUNCTION POINTER ALLOCATION
// ===========================================================================

#define X(type, var, name) type var = NULL;
AL_CORE_FUNCTIONS
ALC_CORE_FUNCTIONS
AL_EFX_FUNCTIONS
AL_HRTF_FUNCTIONS
#undef X

static void *GPA(const char *str)
{
    void *rv = Sys_LoadFunction(OpenALLib, str);
    if (!rv)
    {
        Com_Printf("QAL Error: Can't load symbol %s\n", str);
        alinit_fail = qtrue;
        return NULL;
    }
    return rv;
}

/*
=================
QAL_Init
=================
*/
qboolean QAL_Init(const char *libname)
{
    if (OpenALLib)
        return qtrue;

    if (!(OpenALLib = Sys_LoadDll(libname, qtrue)))
        return qfalse;

    alinit_fail = qfalse;

#define X(type, var, name) var = (type)GPA(name);
    AL_CORE_FUNCTIONS
    ALC_CORE_FUNCTIONS
#undef X

    if (alinit_fail)
    {
        QAL_Shutdown();
        Com_Printf("QAL Fatal: Core OpenAL symbols missing.\n");
        return qfalse;
    }

    // Handle HRTF extension safely via core alcGetProcAddress fallback
    qalcResetDeviceSOFT = (LPALCRESETDEVICESOFT)qalcGetProcAddress(NULL, "alcResetDeviceSOFT");
    qalcGetStringiSOFT = (LPALCGETSTRINGISOFT)qalcGetProcAddress(NULL, "alcGetStringiSOFT");

    return qtrue;
}

/*
=================
QAL_InitEFX
=================
*/
void QAL_InitEFX(void)
{
#define X(type, var, name) var = (type)qalGetProcAddress(name);
    AL_EFX_FUNCTIONS
#undef X
}

/*
=================
QAL_Shutdown
=================
*/
void QAL_Shutdown(void)
{
    if (OpenALLib)
    {
        Sys_UnloadLibrary(OpenALLib);
        OpenALLib = NULL;
    }

#define X(type, var, name) var = NULL;
    AL_CORE_FUNCTIONS
    ALC_CORE_FUNCTIONS
    AL_EFX_FUNCTIONS
    AL_HRTF_FUNCTIONS
#undef X
}

#define USE_OPENAL_LEGACY_STUBS
#ifndef USE_OPENAL_LEGACY_STUBS
// Keep structural compatibility for non-dlopen implementations if needed
#endif

#else // !USE_OPENAL_DLOPEN

qboolean QAL_Init(const char *libname) { return qtrue; }
void QAL_InitEFX(void) {}
void QAL_Shutdown(void) {}

#endif // USE_OPENAL_DLOPEN
#endif // USE_OPENAL
