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
