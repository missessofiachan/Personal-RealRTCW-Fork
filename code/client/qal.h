/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2005 Stuart Dalton (badcdev@gmail.com)
Copyright (C) 2026

This file is part of Quake III Arena / RealRTCW source code.
===========================================================================
*/

#ifndef QAL_H
#define QAL_H

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

#ifdef USE_OPENAL_DLOPEN
#define AL_NO_PROTOTYPES
#define ALC_NO_PROTOTYPES
#endif

#ifdef USE_LOCAL_HEADERS
#include "../AL/al.h"
#include "../AL/alc.h"
#include "../AL/efx.h"
#include "../AL/alext.h"
#else
#ifdef _MSC_VER
  #include <al.h>
  #include <alc.h>
  #include <efx.h>
  #include <alext.h>
#else
  #include <AL/al.h>
  #include <AL/alc.h>
  #include <AL/efx.h>
  #include <AL/alext.h>
#endif
#endif

/* Hack to enable compiling both on OpenAL SDK and OpenAL-soft. */
#ifndef ALC_ENUMERATE_ALL_EXT
#  define ALC_ENUMERATE_ALL_EXT 1
#  define ALC_DEFAULT_ALL_DEVICES_SPECIFIER        0x1012
#  define ALC_ALL_DEVICES_SPECIFIER                0x1013
#endif

// ===========================================================================
// X-MACRO MASTER LISTS (Single Source of Truth for .h and .c)
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
    X(LPALLISTENER3I, qalListener3i, "alListener3i") \
    X(LPALLISTENERIV, qalListeneriv, "alListeneriv") \
    X(LPALGETLISTENERF, qalGetListenerf, "alGetListenerf") \
    X(LPALGETLISTENER3F, qalGetListener3f, "alGetListener3f") \
    X(LPALGETLISTENERFV, qalGetListenerfv, "alGetListenerfv") \
    X(LPALGETLISTENERI, qalGetListeneri, "alGetListeneri") \
    X(LPALGETLISTENER3I, qalGetListener3i, "alGetListener3i") \
    X(LPALGETLISTENERIV, qalGetListeneriv, "alGetListeneriv") \
    X(LPALGENSOURCES, qalGenSources, "alGenSources") \
    X(LPALDELETESOURCES, qalDeleteSources, "alDeleteSources") \
    X(LPALISSOURCE, qalIsSource, "alIsSource") \
    X(LPALSOURCEF, qalSourcef, "alSourcef") \
    X(LPALSOURCE3F, qalSource3f, "alSource3f") \
    X(LPALSOURCEFV, qalSourcefv, "alSourcefv") \
    X(LPALSOURCEI, qalSourcei, "alSourcei") \
    X(LPALSOURCE3I, qalSource3i, "alSource3i") \
    X(LPALSOURCEIV, qalSourceiv, "alSourceiv") \
    X(LPALGETSOURCEF, qalGetSourcef, "alGetSourcef") \
    X(LPALGETSOURCE3F, qalGetSource3f, "alGetSource3f") \
    X(LPALGETSOURCEFV, qalGetSourcefv, "alGetSourcefv") \
    X(LPALGETSOURCEI, qalGetSourcei, "alGetSourcei") \
    X(LPALGETSOURCE3I, qalGetSource3i, "alGetSource3i") \
    X(LPALGETSOURCEIV, qalGetSourceiv, "alGetSourceiv") \
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
    X(LPALBUFFERF, qalBufferf, "alBufferf") \
    X(LPALBUFFER3F, qalBuffer3f, "alBuffer3f") \
    X(LPALBUFFERFV, qalBufferfv, "alBufferfv") \
    X(LPALBUFFERI, qalBufferi, "alBufferi") \
    X(LPALBUFFER3I, qalBuffer3i, "alBuffer3i") \
    X(LPALBUFFERIV, qalBufferiv, "alBufferiv") \
    X(LPALGETBUFFERF, qalGetBufferf, "alGetBufferf") \
    X(LPALGETBUFFER3F, qalGetBuffer3f, "alGetBuffer3f") \
    X(LPALGETBUFFERFV, qalGetBufferfv, "alGetBufferfv") \
    X(LPALGETBUFFERI, qalGetBufferi, "alGetBufferi") \
    X(LPALGETBUFFER3I, qalGetBuffer3i, "alGetBuffer3i") \
    X(LPALGETBUFFERIV, qalGetBufferiv, "alGetBufferiv") \
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
// PREPROCESSOR RESOLUTION
// ===========================================================================

#ifdef USE_OPENAL_DLOPEN

    // Automatically generate 'extern' declarations for the whole list
    #define X(type, var, name) extern type var;
    AL_CORE_FUNCTIONS
    ALC_CORE_FUNCTIONS
    AL_EFX_FUNCTIONS
    AL_HRTF_FUNCTIONS
    #undef X

#else

    // Static linkage fallback macros (C Preprocessor doesn't allow emitting #define via macros, 
    // so we must map them manually here for static builds)
    #define qalEnable alEnable
    #define qalDisable alDisable
    #define qalIsEnabled alIsEnabled
    #define qalGetString alGetString
    #define qalGetBooleanv alGetBooleanv
    #define qalGetIntegerv alGetIntegerv
    #define qalGetFloatv alGetFloatv
    #define qalGetDoublev alGetDoublev
    #define qalGetBoolean alGetBoolean
    #define qalGetInteger alGetInteger
    #define qalGetFloat alGetFloat
    #define qalGetDouble alGetDouble
    #define qalGetError alGetError
    #define qalIsExtensionPresent alIsExtensionPresent
    #define qalGetProcAddress alGetProcAddress
    #define qalGetEnumValue alGetEnumValue
    #define qalListenerf alListenerf
    #define qalListener3f alListener3f
    #define qalListenerfv alListenerfv
    #define qalListeneri alListeneri
    #define qalListener3i alListener3i
    #define qalListeneriv alListeneriv
    #define qalGetListenerf alGetListenerf
    #define qalGetListener3f alGetListener3f
    #define qalGetListenerfv alGetListenerfv
    #define qalGetListeneri alGetListeneri
    #define qalGetListener3i alGetListener3i
    #define qalGetListeneriv alGetListeneriv
    #define qalGenSources alGenSources
    #define qalDeleteSources alDeleteSources
    #define qalIsSource alIsSource
    #define qalSourcef alSourcef
    #define qalSource3f alSource3f
    #define qalSourcefv alSourcefv
    #define qalSourcei alSourcei
    #define qalSource3i alSource3i
    #define qalSourceiv alSourceiv
    #define qalGetSourcef alGetSourcef
    #define qalGetSource3f alGetSource3f
    #define qalGetSourcefv alGetSourcefv
    #define qalGetSourcei alGetSourcei
    #define qalGetSource3i alGetSource3i
    #define qalGetSourceiv alGetSourceiv
    #define qalSourcePlayv alSourcePlayv
    #define qalSourceStopv alSourceStopv
    #define qalSourceRewindv alSourceRewindv
    #define qalSourcePausev alSourcePausev
    #define qalSourcePlay alSourcePlay
    #define qalSourceStop alSourceStop
    #define qalSourceRewind alSourceRewind
    #define qalSourcePause alSourcePause
    #define qalSourceQueueBuffers alSourceQueueBuffers
    #define qalSourceUnqueueBuffers alSourceUnqueueBuffers
    #define qalGenBuffers alGenBuffers
    #define qalDeleteBuffers alDeleteBuffers
    #define qalIsBuffer alIsBuffer
    #define qalBufferData alBufferData
    #define qalBufferf alBufferf
    #define qalBuffer3f alBuffer3f
    #define qalBufferfv alBufferfv
    #define qalBufferi alBufferi
    #define qalBuffer3i alBuffer3i
    #define qalBufferiv alBufferiv
    #define qalGetBufferf alGetBufferf
    #define qalGetBuffer3f alGetBuffer3f
    #define qalGetBufferfv alGetBufferfv
    #define qalGetBufferi alGetBufferi
    #define qalGetBuffer3i alGetBuffer3i
    #define qalGetBufferiv alGetBufferiv
    #define qalDopplerFactor alDopplerFactor
    #define qalSpeedOfSound alSpeedOfSound
    #define qalDistanceModel alDistanceModel

    #define qalcCreateContext alcCreateContext
    #define qalcMakeContextCurrent alcMakeContextCurrent
    #define qalcProcessContext alcProcessContext
    #define qalcSuspendContext alcSuspendContext
    #define qalcDestroyContext alcDestroyContext
    #define qalcGetCurrentContext alcGetCurrentContext
    #define qalcGetContextsDevice alcGetContextsDevice
    #define qalcOpenDevice alcOpenDevice
    #define qalcCloseDevice alcCloseDevice
    #define qalcGetError alcGetError
    #define qalcIsExtensionPresent alcIsExtensionPresent
    #define qalcGetProcAddress alcGetProcAddress
    #define qalcGetEnumValue alcGetEnumValue
    #define qalcGetString alcGetString
    #define qalcGetIntegerv alcGetIntegerv
    #define qalcCaptureOpenDevice alcCaptureOpenDevice
    #define qalcCaptureCloseDevice alcCaptureCloseDevice
    #define qalcCaptureStart alcCaptureStart
    #define qalcCaptureStop alcCaptureStop
    #define qalcCaptureSamples alcCaptureSamples

    #define qalGenEffects alGenEffects
    #define qalDeleteEffects alDeleteEffects
    #define qalIsEffect alIsEffect
    #define qalEffecti alEffecti
    #define qalEffectf alEffectf
    #define qalEffectfv alEffectfv
    #define qalGenAuxiliaryEffectSlots alGenAuxiliaryEffectSlots
    #define qalDeleteAuxiliaryEffectSlots alDeleteAuxiliaryEffectSlots
    #define qalAuxiliaryEffectSloti alAuxiliaryEffectSloti
    #define qalAuxiliaryEffectSlotf alAuxiliaryEffectSlotf
    #define qalAuxiliaryEffectSlotfv alAuxiliaryEffectSlotfv
    #define qalGenFilters alGenFilters
    #define qalDeleteFilters alDeleteFilters
    #define qalIsFilter alIsFilter
    #define qalFilteri alFilteri
    #define qalFilterf alFilterf

    #define qalcResetDeviceSOFT alcResetDeviceSOFT
    #define qalcGetStringiSOFT alcGetStringiSOFT

#endif // USE_OPENAL_DLOPEN

qboolean QAL_Init(const char *libname);
void QAL_InitEFX(void);
void QAL_Shutdown( void );

#endif // QAL_H
