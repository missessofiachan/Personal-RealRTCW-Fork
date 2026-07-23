/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

// common.c -- misc functions used in client and server

#include "q_shared.h"
#include "qcommon.h"
#include "gp_jobsystem.h"
#include <setjmp.h>
#ifndef _WIN32
#include <netinet/in.h>
#include <sys/stat.h> // umask
#else
#include <winsock.h>
#endif

int demo_protocols[] =
{ 59, 58, 57, 0 };

#define MAXPRINTMSG 4096

#define MAX_NUM_ARGVS   50

#define MIN_DEDICATED_COMHUNKMEGS 1
#define MIN_COMHUNKMEGS		256
#define DEF_COMHUNKMEGS 	1024
#define DEF_COMZONEMEGS		128
#define DEF_COMHUNKMEGS_S	XSTRING(DEF_COMHUNKMEGS)
#define DEF_COMZONEMEGS_S	XSTRING(DEF_COMZONEMEGS)

int com_argc;
char    *com_argv[MAX_NUM_ARGVS + 1];


jmp_buf abortframe;     // an ERR_DROP occured, exit the entire frame


FILE *debuglogfile;
static fileHandle_t pipefile;
static fileHandle_t logfile;
fileHandle_t com_journalFile;               // events are written here
fileHandle_t com_journalDataFile;           // config files are written here

cvar_t  *com_speeds;
cvar_t  *com_developer;
cvar_t  *com_dedicated;
cvar_t  *com_timescale;
cvar_t  *com_fixedtime;
cvar_t  *com_journal;
cvar_t  *com_maxfps;
cvar_t	*com_altivec;
cvar_t  *com_timedemo;
cvar_t  *com_sv_running;
cvar_t  *com_cl_running;
cvar_t  *com_logfile;       // 1 = buffer log, 2 = flush after each print
cvar_t	*com_pipefile;
cvar_t  *com_showtrace;
cvar_t  *com_version;
cvar_t  *com_blood;
cvar_t  *com_buildScript;   // for automated data building scripts
#ifdef CINEMATICS_INTRO
cvar_t  *com_introPlayed;
#endif
cvar_t  *cl_paused;
cvar_t  *sv_paused;
cvar_t  *cl_packetdelay;
cvar_t  *sv_packetdelay;
cvar_t  *com_cameraMode;
cvar_t  *com_recommendedSet;
cvar_t	*com_ansiColor;
cvar_t	*com_unfocused;
cvar_t	*com_maxfpsUnfocused;
cvar_t	*com_minimized;
cvar_t	*com_maxfpsMinimized;
cvar_t	*com_abnormalExit;
cvar_t	*com_pause;
cvar_t	*com_standalone;
cvar_t	*com_gamename;
cvar_t	*com_atmosphericEffect;	/// Berserker
cvar_t	*com_protocol;
#ifdef LEGACY_PROTOCOL
cvar_t	*com_legacyprotocol;
#endif
cvar_t	*com_basegame;
cvar_t  *com_homepath;
cvar_t	*com_busyWait;
#ifndef DEDICATED
cvar_t  *con_autochat;
#endif

#if idx64
	int (*Q_VMftol)(void);
#elif id386
	long (QDECL *Q_ftol)(float f);
	int (QDECL *Q_VMftol)(void);
	void (QDECL *Q_SnapVector)(vec3_t vec);
#endif

// Rafael Notebook
cvar_t  *cl_notebook;

cvar_t  *com_hunkused;      // Ridah

// com_speeds times
int time_game;
int time_frontend;          // renderer frontend time
int time_backend;           // renderer backend time

int com_frameTime;
int com_frameNumber;

qboolean	com_errorEntered = qfalse;
qboolean	com_fullyInitialized = qfalse;
qboolean	com_gameRestarting = qfalse;
qboolean	com_gameClientRestarting = qfalse;

char com_errorMessage[MAXPRINTMSG];

void Com_WriteConfig_f( void );
void CIN_CloseAllVideos( void );

//============================================================================

static char *rd_buffer;
static int rd_buffersize;
static void ( *rd_flush )( char *buffer );

void Com_BeginRedirect( char *buffer, int buffersize, void ( *flush )( char *) ) {
	if ( !buffer || !buffersize || !flush ) {
		return;
	}
	rd_buffer = buffer;
	rd_buffersize = buffersize;
	rd_flush = flush;

	*rd_buffer = 0;
}

void Com_EndRedirect( void ) {
	if ( rd_flush ) {
		rd_flush( rd_buffer );
	}

	rd_buffer = NULL;
	rd_buffersize = 0;
	rd_flush = NULL;
}

/*
=============
Com_Printf

Both client and server can use this, and it will output
to the apropriate place.

A raw string should NEVER be passed as fmt, because of "%f" type crashers.
=============
*/
void QDECL Com_Printf( const char *fmt, ... ) {
	va_list argptr;
	char msg[MAXPRINTMSG];
	static qboolean opening_qconsole = qfalse;

	va_start( argptr,fmt );
	Q_vsnprintf( msg, sizeof (msg), fmt, argptr );
	va_end( argptr );

	if ( rd_buffer ) {
		if ( ( strlen( msg ) + strlen( rd_buffer ) ) > ( rd_buffersize - 1 ) ) {
			rd_flush( rd_buffer );
			*rd_buffer = 0;
		}
		Q_strcat( rd_buffer, rd_buffersize, msg );
		// show_bug.cgi?id=51
		// only flush the rcon buffer when it's necessary, avoid fragmenting
		//rd_flush(rd_buffer);
		//*rd_buffer = 0;
		return;
	}

#ifndef DEDICATED
	CL_ConsolePrint( msg );
#endif

	// echo to dedicated console and early console
	Sys_Print( msg );

	// logfile
	if ( com_logfile && com_logfile->integer ) {
		// TTimo: only open the qconsole.log if the filesystem is in an initialized state
		//   also, avoid recursing in the qconsole.log opening (i.e. if fs_debug is on)
		if ( !logfile && FS_Initialized() && !opening_qconsole ) {
			struct tm *newtime;
			time_t aclock;

			opening_qconsole = qtrue;

			time( &aclock );
			newtime = localtime( &aclock );

			logfile = FS_FOpenFileWrite( "rtcwconsole.log" );    //----(SA)	changed name for Wolf
			if(logfile)
			{
				Com_Printf( "logfile opened on %s\n", asctime( newtime ) );
			
				if ( com_logfile->integer > 1 )
				{
					// force it to not buffer so we get valid
					// data even if we are crashing
					FS_ForceFlush(logfile);
				}
			}
			else
			{
				Com_Printf("Opening rtcwconsole.log failed!\n");
				Cvar_SetValue("logfile", 0);
			}

			opening_qconsole = qfalse;
		}
		if ( logfile && FS_Initialized() ) {
			FS_Write( msg, strlen( msg ), logfile );
		}
	}
}


/*
================
Com_DPrintf

A Com_Printf that only shows up if the "developer" cvar is set
================
*/
void QDECL Com_DPrintf( const char *fmt, ... ) {
	va_list argptr;
	char msg[MAXPRINTMSG];

	if ( !com_developer || !com_developer->integer ) {
		return;         // don't confuse non-developers with techie stuff...
	}

	va_start( argptr,fmt );
	Q_vsnprintf( msg, sizeof ( msg ), fmt, argptr );
	va_end( argptr );

	Com_Printf( "%s", msg );
}

/*
=============
Com_Error

Both client and server can use this, and it will
do the appropriate thing.
=============
*/
void QDECL Com_Error( int code, const char *fmt, ... ) {
	va_list argptr;
	static int lastErrorTime;
	static int errorCount;
	int currentTime;
	qboolean restartClient;

	if(com_errorEntered)
		Sys_Error("recursive error after: %s", com_errorMessage);

	com_errorEntered = qtrue;

	Cvar_Set("com_errorCode", va("%i", code));

	// when we are running automated scripts, make sure we
	// know if anything failed
	if ( com_buildScript && com_buildScript->integer ) {

		// ERR_ENDGAME is not really an error, don't die if building a script
		if ( code != ERR_ENDGAME ) {
			code = ERR_FATAL;
		}
	}

	// if we are getting a solid stream of ERR_DROP, do an ERR_FATAL
	currentTime = Sys_Milliseconds();
	if ( currentTime - lastErrorTime < 100 ) {
		if ( ++errorCount > 3 ) {
			code = ERR_FATAL;
		}
	} else {
		errorCount = 0;
	}
	lastErrorTime = currentTime;

	va_start( argptr,fmt );
	Q_vsnprintf (com_errorMessage, sizeof(com_errorMessage),fmt,argptr);
	va_end( argptr );

	if ( code != ERR_DISCONNECT && code != ERR_NEED_CD && code != ERR_ENDGAME ) {
		Cvar_Set( "com_errorMessage", com_errorMessage );
	}

	restartClient = com_gameClientRestarting && !( com_cl_running && com_cl_running->integer );

	com_gameRestarting = qfalse;
	com_gameClientRestarting = qfalse;

	if (code == ERR_DISCONNECT || code == ERR_SERVERDISCONNECT) {
		VM_Forced_Unload_Start();
		SV_Shutdown( "Server disconnected" );
		if ( restartClient ) {
			CL_Init();
		}
		CL_Disconnect( qtrue );
		CL_FlushMemory();
		VM_Forced_Unload_Done();
		// make sure we can get at our local stuff
		FS_PureServerSetLoadedPaks("", "");
		com_errorEntered = qfalse;
		longjmp( abortframe, -1 );
	} else if ( code == ERR_ENDGAME ) {  //----(SA)	added
		VM_Forced_Unload_Start();
		SV_Shutdown( "endgame" );
		if ( restartClient ) {
			CL_Init();
		}
		if ( com_cl_running && com_cl_running->integer ) {
			CL_Disconnect( qtrue );
			CL_FlushMemory();
			VM_Forced_Unload_Done();
#ifndef DEDICATED
			CL_EndgameMenu();
#endif
		}

		FS_PureServerSetLoadedPaks("", "");

		com_errorEntered = qfalse;
		longjmp( abortframe, -1 );
	} else if (code == ERR_DROP) {
		Com_Printf( "********************\nERROR: %s\n********************\n", com_errorMessage );
		VM_Forced_Unload_Start();
		SV_Shutdown (va("Server crashed: %s",  com_errorMessage));
		if ( restartClient ) {
			CL_Init();
		}
		CL_Disconnect( qtrue );
		CL_FlushMemory();
		VM_Forced_Unload_Done();
		FS_PureServerSetLoadedPaks("", "");
		com_errorEntered = qfalse;
		longjmp( abortframe, -1 );
	} else if ( code == ERR_NEED_CD ) {
		VM_Forced_Unload_Start();
		SV_Shutdown( "Server didn't have CD" );
		if ( restartClient ) {
			CL_Init();
		}
		if ( com_cl_running && com_cl_running->integer ) {
			CL_Disconnect( qtrue );
			CL_FlushMemory();
			VM_Forced_Unload_Done();
			CL_CDDialog();
		} else {
			Com_Printf( "Server didn't have CD\n" );
			VM_Forced_Unload_Done();
		}

		FS_PureServerSetLoadedPaks("", "");

		com_errorEntered = qfalse;
		longjmp( abortframe, -1 );
	} else {
		VM_Forced_Unload_Start();
		CL_Shutdown(va("Client fatal crashed: %s", com_errorMessage), qtrue, qtrue);
		SV_Shutdown(va("Server fatal crashed: %s", com_errorMessage));
		VM_Forced_Unload_Done();
	}

	Com_Shutdown();

	Sys_Error( "%s", com_errorMessage );
}


/*
=============
Com_Quit_f

Both client and server can use this, and it will
do the apropriate things.
=============
*/
void Com_Quit_f( void ) {
	// don't try to shutdown if we are in a recursive error
	char *p = Cmd_Args( );
	if ( !com_errorEntered ) {
		// Some VMs might execute "quit" command directly,
		// which would trigger an unload of active VM error.
		// Sys_Quit will kill this process anyways, so
		// a corrupt call stack makes no difference
		VM_Forced_Unload_Start();
		SV_Shutdown(p[0] ? p : "Server quit");
		CL_Shutdown(p[0] ? p : "Client quit", qtrue, qtrue);
		VM_Forced_Unload_Done();
		Com_Shutdown();
		FS_Shutdown( qtrue );
	}
	Sys_Quit();
}



/*
============================================================================

COMMAND LINE FUNCTIONS

+ characters seperate the commandLine string into multiple console
command lines.

All of these are valid:

quake3 +set test blah +map test
quake3 set test blah+map test
quake3 set test blah + map test

============================================================================
*/

#define MAX_CONSOLE_LINES   32
int com_numConsoleLines;
char    *com_consoleLines[MAX_CONSOLE_LINES];

/*
==================
Com_ParseCommandLine

Break it up into multiple console lines
==================
*/
void Com_ParseCommandLine( char *commandLine ) {
	com_consoleLines[0] = commandLine;
	com_numConsoleLines = 1;

	while ( *commandLine ) {
		// look for a + separating character
		// if commandLine came from a file, we might have real line seperators
		if ( *commandLine == '+' || *commandLine == '\n' ) {
			if ( com_numConsoleLines == MAX_CONSOLE_LINES ) {
				return;
			}
			com_consoleLines[com_numConsoleLines] = commandLine + 1;
			com_numConsoleLines++;
			*commandLine = 0;
		}
		commandLine++;
	}
}


/*
===================
Com_SafeMode

Check for "safe" on the command line, which will
skip loading of wolfconfig.cfg
===================
*/
qboolean Com_SafeMode( void ) {
	int i;

	for ( i = 0 ; i < com_numConsoleLines ; i++ ) {
		Cmd_TokenizeString( com_consoleLines[i] );
		if ( !Q_stricmp( Cmd_Argv( 0 ), "safe" )
			 || !Q_stricmp( Cmd_Argv( 0 ), "cvar_restart" ) ) {
			com_consoleLines[i][0] = 0;
			return qtrue;
		}
	}
	return qfalse;
}


/*
===============
Com_StartupVariable

Searches for command line parameters that are set commands.
If match is not NULL, only that cvar will be looked for.
That is necessary because cddir and basedir need to be set
before the filesystem is started, but all other sets should
be after execing the config and default.
===============
*/
void Com_StartupVariable( const char *match ) {
	int i;
	char    *s;

	for ( i = 0 ; i < com_numConsoleLines ; i++ ) {
		Cmd_TokenizeString( com_consoleLines[i] );
		if ( strcmp( Cmd_Argv( 0 ), "set" ) ) {
			continue;
		}

		s = Cmd_Argv( 1 );

		if(!match || !strcmp(s, match))
		{
			if(Cvar_Flags(s) == CVAR_NONEXISTENT)
				Cvar_Get(s, Cmd_ArgsFrom(2), CVAR_USER_CREATED);
			else
				Cvar_Set2(s, Cmd_ArgsFrom(2), qfalse);
		}
	}
}


/*
=================
Com_AddStartupCommands

Adds command line parameters as script statements
Commands are seperated by + signs

Returns qtrue if any late commands were added, which
will keep the demoloop from immediately starting
=================
*/
qboolean Com_AddStartupCommands( void ) {
	int i;
	qboolean added;

	added = qfalse;
	// quote every token, so args with semicolons can work
	for ( i = 0 ; i < com_numConsoleLines ; i++ ) {
		if ( !com_consoleLines[i] || !com_consoleLines[i][0] ) {
			continue;
		}

		// set commands already added with Com_StartupVariable
		if ( !Q_stricmpn( com_consoleLines[i], "set ", 4 ) ) {
			continue;
		}

		added = qtrue;
		Cbuf_AddText( com_consoleLines[i] );
		Cbuf_AddText( "\n" );
	}

	return added;
}


//============================================================================

void Info_Print( const char *s ) {
	char	key[BIG_INFO_KEY];
	char	value[BIG_INFO_VALUE];
	char    *o;
	int l;

	if ( *s == '\\' ) {
		s++;
	}
	while ( *s )
	{
		o = key;
		while ( *s && *s != '\\' )
			*o++ = *s++;

		l = o - key;
		if ( l < 20 ) {
			memset( o, ' ', 20 - l );
			key[20] = 0;
		} else {
			*o = 0;
		}
		Com_Printf( "%s ", key );

		if ( !*s ) {
			Com_Printf( "MISSING VALUE\n" );
			return;
		}

		o = value;
		s++;
		while ( *s && *s != '\\' )
			*o++ = *s++;
		*o = 0;

		if ( *s ) {
			s++;
		}
		Com_Printf( "%s\n", value );
	}
}

#define Q_LOWERASCII(c) (((c) >= 'A' && (c) <= 'Z') ? ((c) | 0x20) : (c))
#define Q_PATHCHAR(c, isPath) ((isPath) && ((c) == '\\' || (c) == ':') ? '/' : (c))

/*
============================
Com_StringContains_Internal
============================
*/
static char *Com_StringContains_Internal( const char *str1, const char *str2, qboolean casesensitive, qboolean isPath ) {
	int len1 = 0;
	int len2 = 0;
	const char *p;
	int len, i, j;

	for ( p = str1; *p; p++ ) len1++;
	for ( p = str2; *p; p++ ) len2++;
	len = len1 - len2;

	for ( i = 0; i <= len; i++, str1++ ) {
		for ( j = 0; str2[j]; j++ ) {
			char c1 = Q_PATHCHAR( str1[j], isPath );
			char c2 = Q_PATHCHAR( str2[j], isPath );
			
			if ( casesensitive ) {
				if ( c1 != c2 ) {
					break;
				}
			} else {
				if ( Q_LOWERASCII( c1 ) != Q_LOWERASCII( c2 ) ) {
					break;
				}
			}
		}
		if ( !str2[j] ) {
			return (char *)str1;
		}
	}
	return NULL;
}
/*
====================
Com_Filter_Internal
====================
*/
static int Com_Filter_Internal( const char *filter, const char *name, qboolean casesensitive, qboolean isPath ) {
	char buf[MAX_TOKEN_CHARS];
	const char *ptr;
	int i, found;

	while ( *filter ) {
		if ( *filter == '*' ) {
			filter++;
			for ( i = 0; *filter && i < MAX_TOKEN_CHARS - 1; i++ ) {
				if ( *filter == '*' || *filter == '?' ) {
					break;
				}
				buf[i] = Q_PATHCHAR( *filter, isPath );
				filter++;
			}
			buf[i] = '\0';
			if ( buf[0] != '\0' ) {
				ptr = Com_StringContains_Internal( name, buf, casesensitive, isPath );
				if ( !ptr ) {
					return qfalse;
				}
				int matchLen = 0;
				while ( buf[matchLen] ) matchLen++;
				name = ptr + matchLen;
			}
		} else if ( *filter == '?' ) {
			if ( !*name ) {
				return qfalse;
			}
			filter++;
			name++;
		} else if ( *filter == '[' && *( filter + 1 ) == '[' ) {
			char c1 = Q_PATHCHAR( *name, isPath );
			char c2 = Q_PATHCHAR( *(filter + 1), isPath );
			if ( casesensitive ) {
				if ( c1 != c2 ) return qfalse;
			} else {
				if ( Q_LOWERASCII( c1 ) != Q_LOWERASCII( c2 ) ) return qfalse;
			}
			filter += 2;
			name++;
		} else if ( *filter == '[' ) {
			filter++;
			found = qfalse;
			char nameChar = Q_PATHCHAR( *name, isPath );
			if ( !casesensitive ) nameChar = Q_LOWERASCII( nameChar );

			while ( *filter && !found ) {
				if ( *filter == ']' && *( filter + 1 ) != ']' ) {
					break;
				}
				if ( *( filter + 1 ) == '-' && *( filter + 2 ) && ( *( filter + 2 ) != ']' || *( filter + 3 ) == ']' ) ) {
					char fMin = Q_PATHCHAR( *filter, isPath );
					char fMax = Q_PATHCHAR( *(filter + 2), isPath );
					if ( !casesensitive ) {
						fMin = Q_LOWERASCII( fMin );
						fMax = Q_LOWERASCII( fMax );
					}
					if ( nameChar >= fMin && nameChar <= fMax ) {
						found = qtrue;
					}
					filter += 3;
				} else {
					char fChar = Q_PATHCHAR( *filter, isPath );
					if ( !casesensitive ) fChar = Q_LOWERASCII( fChar );
					if ( fChar == nameChar ) {
						found = qtrue;
					}
					filter++;
				}
			}
			if ( !found ) {
				return qfalse;
			}
			while ( *filter ) {
				if ( *filter == ']' && *( filter + 1 ) != ']' ) {
					break;
				}
				filter++;
			}
			filter++;
			name++;
		} else {
			char c1 = Q_PATHCHAR( *filter, isPath );
			char c2 = Q_PATHCHAR( *name, isPath );
			if ( casesensitive ) {
				if ( c1 != c2 ) {
					return qfalse;
				}
			} else {
				if ( Q_LOWERASCII( c1 ) != Q_LOWERASCII( c2 ) ) {
					return qfalse;
				}
			}
			filter++;
			name++;
		}
	}
	
	if ( *name && *(filter - 1) != '*' ) {
		return qfalse;
	}
	
	return qtrue;
}

/*
============
Com_StringContains
============
*/
char *Com_StringContains( char *str1, char *str2, int casesensitive ) {
	return Com_StringContains_Internal( str1, str2, (qboolean)casesensitive, qfalse );
}

/*
============
Com_Filter
============
*/
int Com_Filter( char *filter, char *name, int casesensitive ) {
	return Com_Filter_Internal( filter, name, (qboolean)casesensitive, qfalse );
}

/*
============
Com_FilterPath
============
*/
int Com_FilterPath( char *filter, char *name, int casesensitive ) {
	return Com_Filter_Internal( filter, name, (qboolean)casesensitive, qtrue );
}

/*
================
Com_RealTime
================
*/
int Com_RealTime( qtime_t *qtime ) {
	time_t t;
	struct tm *tms;

	t = time( NULL );
	if ( !qtime ) {
		return t;
	}
	tms = localtime( &t );
	if ( tms ) {
		qtime->tm_sec = tms->tm_sec;
		qtime->tm_min = tms->tm_min;
		qtime->tm_hour = tms->tm_hour;
		qtime->tm_mday = tms->tm_mday;
		qtime->tm_mon = tms->tm_mon;
		qtime->tm_year = tms->tm_year;
		qtime->tm_wday = tms->tm_wday;
		qtime->tm_yday = tms->tm_yday;
		qtime->tm_isdst = tms->tm_isdst;
	}
	return t;
}


/*
==============================================================================

MIMALLOC REPLACEMENT FOR ZONE MEMORY ALLOCATION
				(Thread-Safe, High-Performance, SIMD-Aligned)

==============================================================================
*/

#define USE_MIMALLOC_ZONE

#ifdef USE_MIMALLOC_ZONE
#include "../mimalloc/include/mimalloc.h"
#endif

// Cross-platform spinlock primitive used by Hunk and Event Queue locks
#ifdef _WIN32
#include <windows.h>
typedef volatile LONG zoneLock_t;
#define ZONE_LOCK_INIT(l)     (*(l) = 0)
#define ZONE_LOCK(l)          while(InterlockedExchange((l), 1) == 1) { Sleep(0); }
#define ZONE_UNLOCK(l)        InterlockedExchange((l), 0)
#else
#include <sched.h>
typedef volatile int zoneLock_t;
#define ZONE_LOCK_INIT(l)     (*(l) = 0)
#define ZONE_LOCK(l)          while(__sync_lock_test_and_set((l), 1)) { sched_yield(); }
#define ZONE_UNLOCK(l)        __sync_lock_release((l))
#endif

static int s_zoneTotal = 0;

/*
========================
Com_InitZoneMemory
========================
*/
void Com_InitZoneMemory( void ) {
#ifdef USE_MIMALLOC_ZONE
	// Configure mimalloc for premium game memory handling
	mi_option_set(mi_option_large_os_pages, 1);
	mi_option_set(mi_option_eager_commit, 1);
#endif
	s_zoneTotal = 0;
}

/*
========================
Z_Free
========================
*/
void Z_Free( void *ptr ) {
	if ( !ptr ) {
		return;
	}

#ifdef USE_MIMALLOC_ZONE
	mi_free( ptr );
#else
	free( ptr );
#endif
}

/*
========================
Z_Malloc
========================
*/
void *Z_Malloc( int size ) {
	void *rawResult = NULL;

	if ( size <= 0 ) {
		Com_Error( ERR_FATAL, "Z_Malloc: Allocation bounds error with size %i", size );
	}

#ifdef USE_MIMALLOC_ZONE
	rawResult = mi_malloc( size );
	if ( !rawResult ) {
		Com_Error( ERR_FATAL, "Z_Malloc: Allocation bounds error for %i bytes", size );
	}
	s_zoneTotal += size;
	Com_Memset( rawResult, 0, size );
#else
	rawResult = malloc( size );
	if ( !rawResult ) {
		Com_Error( ERR_FATAL, "Z_Malloc: Allocation bounds error for %i bytes", size );
	}
	Com_Memset( rawResult, 0, size );
#endif

	return rawResult;
}





#if 0
/*
================
Z_TagMalloc
================
*/
void *Z_TagMalloc( int size, int tag ) {

	if ( tag != TAG_RENDERER ) {
		assert( 0 );
	}

	if ( g_numTaggedAllocs < MAX_TAG_ALLOCS ) {
		void *ptr = Z_Malloc( size );
		g_taggedAllocations[g_numTaggedAllocs++] = ptr;
		return ptr;
	} else {
		Com_Error( ERR_FATAL, "Z_TagMalloc: out of tagged allocation space\n" );
	}
	return NULL;
}

/*
================
Z_FreeTags
================
*/
void Z_FreeTags( int tag ) {
	int i;

	if ( tag != TAG_RENDERER ) {
		assert( 0 );
	}

	for ( i = 0; i < g_numTaggedAllocs; i++ ) {
		free( g_taggedAllocations[i] );
	}

	g_numTaggedAllocs = 0;
}

#endif

/*
========================
CopyString

 NOTE:	never write over the memory CopyString returns because
		memory from a memstatic_t might be returned
========================
*/
char *CopyString( const char *in ) {
	char    *out;

	out = Z_Malloc( strlen( in ) + 1 );
	strcpy( out, in );
	return out;
}

/*
==============================================================================

Goals:
	reproducable without history effects -- no out of memory errors on weird map to map changes
	allow restarting of the client without fragmentation
	minimize total pages in use at run time
	minimize total pages needed during load time

  Single block of memory with stack allocators coming from both ends towards the middle.

  One side is designated the temporary memory allocator.

  Temporary memory can be allocated and freed in any order.

  A highwater mark is kept of the most in use at any time.

  When there is no temporary memory allocated, the permanent and temp sides
  can be switched, allowing the already touched temp memory to be used for
  permanent storage.

  Temp memory must never be allocated on two ends at once, or fragmentation
  could occur.

  If we have any in-use temp memory, additional temp allocations must come from
  that side.

  If not, we can choose to make either side the new temp side and push future
  permanent allocations to the other side.  Permanent allocations should be
  kept on the side that has the current greatest wasted highwater mark.

==============================================================================
*/


#define HUNK_MAGIC  0x89537892
#define HUNK_FREE_MAGIC 0x89537893

typedef struct {
	int magic;
	int size;
} hunkHeader_t;

typedef struct {
	int mark;
	int permanent;
	int temp;
	int tempHighwater;
} hunkUsed_t;

typedef struct hunkblock_s {
	int size;
	byte printed;
	struct hunkblock_s *next;
	char *label;
	char *file;
	int line;
} hunkblock_t;

static hunkblock_t *hunkblocks;

static hunkUsed_t hunk_low, hunk_high;
static hunkUsed_t  *hunk_permanent, *hunk_temp;

static byte    *s_hunkData = NULL;
static int s_hunkTotal;

static int s_zoneTotal;

// Atomic Spinlock dedicated purely to securing Hunk state changes across worker threads
static zoneLock_t g_hunkSpinlock = 0;
//static	int		s_smallZoneTotal; // TTimo: unused


/*
=================
Com_Meminfo_f
=================
*/
void Com_Meminfo_f( void ) {
	int unused;

#ifdef USE_MULTI_SEGMENT
	ZONE_LOCK( &g_hunkSpinlock );
#endif

	Com_Printf( "%8i bytes total hunk\n", s_hunkTotal );
	Com_Printf( "%8i bytes total zone\n", s_zoneTotal );
	Com_Printf( "\n" );
	Com_Printf( "%8i low mark\n", hunk_low.mark );
	Com_Printf( "%8i low permanent\n", hunk_low.permanent );
	if ( hunk_low.temp != hunk_low.permanent ) {
		Com_Printf( "%8i low temp\n", hunk_low.temp );
	}
	Com_Printf( "%8i low tempHighwater\n", hunk_low.tempHighwater );
	Com_Printf( "\n" );
	Com_Printf( "%8i high mark\n", hunk_high.mark );
	Com_Printf( "%8i high permanent\n", hunk_high.permanent );
	if ( hunk_high.temp != hunk_high.permanent ) {
		Com_Printf( "%8i high temp\n", hunk_high.temp );
	}
	Com_Printf( "%8i high tempHighwater\n", hunk_high.tempHighwater );
	Com_Printf( "\n" );
	Com_Printf( "%8i total hunk in use\n", hunk_low.permanent + hunk_high.permanent );
	unused = 0;
	if ( hunk_low.tempHighwater > hunk_low.permanent ) {
		unused += hunk_low.tempHighwater - hunk_low.permanent;
	}
	if ( hunk_high.tempHighwater > hunk_high.permanent ) {
		unused += hunk_high.tempHighwater - hunk_high.permanent;
	}
	Com_Printf( "%8i unused highwater\n", unused );
	Com_Printf( "\n" );

#ifdef USE_MULTI_SEGMENT
	ZONE_UNLOCK( &g_hunkSpinlock );
#endif
}

#ifdef USE_MIMALLOC_ZONE
/*
=================
Com_MimallocStats_f
=================
*/
static void Com_MimallocStats_f( void ) {
	mi_stats_print(NULL); // print to stderr by default
}
#endif

/*
===============
Com_TouchMemory

Touch all known used data to make sure it is paged in
===============
*/
void Com_TouchMemory( void ) {
	int start, end;
	int i, j;
	unsigned sum;

	start = Sys_Milliseconds();
	sum = 0;

#ifdef USE_MULTI_SEGMENT
	ZONE_LOCK( &g_hunkSpinlock );
#endif

	j = hunk_low.permanent >> 2;
	for ( i = 0 ; i < j ; i += 16 ) { // Adjusted stride to 16 ints (64 bytes) to step clean cache lines
		sum += ( (int *)s_hunkData )[i];
	}

	i = ( s_hunkTotal - hunk_high.permanent ) >> 2;
	j = hunk_high.permanent >> 2;
	for (  ; i < j ; i += 16 ) {
		sum += ( (int *)s_hunkData )[i];
	}

#ifdef USE_MULTI_SEGMENT
	ZONE_UNLOCK( &g_hunkSpinlock );
#endif

	end = Sys_Milliseconds();
	Com_Printf( "Com_TouchMemory: %i msec (diagnostic checksum: %u)\n", end - start, sum );
}






/*
=================
Hunk_Log
=================
*/
void Hunk_Log( void ) {
	hunkblock_t *block;
	char buf[4096];
	int size, numBlocks;

	if ( !logfile || !FS_Initialized() ) {
		return;
	}
	size = 0;
	numBlocks = 0;
	Com_sprintf( buf, sizeof( buf ), "\r\n================\r\nHunk log\r\n================\r\n" );
	FS_Write( buf, strlen( buf ), logfile );
	
#ifdef USE_MULTI_SEGMENT
	ZONE_LOCK( &g_hunkSpinlock );
#endif

	for ( block = hunkblocks ; block; block = block->next ) {
#ifdef HUNK_DEBUG
		Com_sprintf( buf, sizeof( buf ), "size = %8d: %s, line: %d (%s)\r\n", block->size, block->file, block->line, block->label );
		FS_Write( buf, strlen( buf ), logfile );
#endif
		size += block->size;
		numBlocks++;
	}

#ifdef USE_MULTI_SEGMENT
	ZONE_UNLOCK( &g_hunkSpinlock );
#endif

	Com_sprintf( buf, sizeof( buf ), "%d Hunk memory\r\n", size );
	FS_Write( buf, strlen( buf ), logfile );
	Com_sprintf( buf, sizeof( buf ), "%d hunk blocks\r\n", numBlocks );
	FS_Write( buf, strlen( buf ), logfile );
}

/*
=================
Hunk_SmallLog
=================
*/
void Hunk_SmallLog( void ) {
	hunkblock_t *block, *block2;
	char buf[4096];
	int size, locsize, numBlocks;

	if ( !logfile || !FS_Initialized() ) {
		return;
	}
	
#ifdef USE_MULTI_SEGMENT
	ZONE_LOCK( &g_hunkSpinlock );
#endif

	for ( block = hunkblocks ; block; block = block->next ) {
		block->printed = qfalse;
	}
	size = 0;
	numBlocks = 0;
	Com_sprintf( buf, sizeof( buf ), "\r\n================\r\nHunk Small log\r\n================\r\n" );
	FS_Write( buf, strlen( buf ), logfile );
	for ( block = hunkblocks; block; block = block->next ) {
		if ( block->printed ) {
			continue;
		}
		locsize = block->size;
		for ( block2 = block->next; block2; block2 = block2->next ) {
			if ( block->line != block2->line ) {
				continue;
			}
			if ( Q_stricmp( block->file, block2->file ) ) {
				continue;
			}
			size += block2->size;
			locsize += block2->size;
			block2->printed = qtrue;
		}
#ifdef HUNK_DEBUG
		Com_sprintf( buf, sizeof( buf ), "size = %8d: %s, line: %d (%s)\r\n", locsize, block->file, block->line, block->label );
		FS_Write( buf, strlen( buf ), logfile );
#endif
		size += block->size;
		numBlocks++;
	}

#ifdef USE_MULTI_SEGMENT
	ZONE_UNLOCK( &g_hunkSpinlock );
#endif

	Com_sprintf( buf, sizeof( buf ), "%d Hunk memory\r\n", size );
	FS_Write( buf, strlen( buf ), logfile );
	Com_sprintf( buf, sizeof( buf ), "%d hunk blocks\r\n", numBlocks );
	FS_Write( buf, strlen( buf ), logfile );
}

/*
=================
Com_InitHunkMemory
=================
*/
void Com_InitHunkMemory( void ) {
	cvar_t  *cv;
	int nMinAlloc;
	char *pMsg = NULL;

	if ( FS_LoadStack() != 0 ) {
		Com_Error( ERR_FATAL, "Hunk initialization failed. File system load stack not zero" );
	}

	cv = Cvar_Get( "com_hunkMegs", DEF_COMHUNKMEGS_S, CVAR_LATCH | CVAR_ARCHIVE );

	if ( com_dedicated && com_dedicated->integer ) {
		nMinAlloc = MIN_DEDICATED_COMHUNKMEGS;
		pMsg = "Minimum com_hunkMegs for a dedicated server is %i, allocating %i megs.\n";
	} else {
		nMinAlloc = MIN_COMHUNKMEGS;
		pMsg = "Minimum com_hunkMegs is %i, allocating %i megs.\n";
	}

	if ( cv->integer < nMinAlloc ) {
		s_hunkTotal = 1024 * 1024 * nMinAlloc;
		Com_Printf( pMsg, nMinAlloc, s_hunkTotal / ( 1024 * 1024 ) );
	} else {
		s_hunkTotal = cv->integer * 1024 * 1024;
	}

#ifdef USE_MULTI_SEGMENT
	ZONE_LOCK_INIT( &g_hunkSpinlock );
#endif

	s_hunkData = malloc( s_hunkTotal + 63 ); // Pad extra for 64-byte boundary adjustments
	if ( !s_hunkData ) {
		Com_Error( ERR_FATAL, "Hunk data failed to allocate %i megs", s_hunkTotal / ( 1024 * 1024 ) );
	}
	
	// Force perfect 64-byte cacheline alignment on the base pointer allocation row
	s_hunkData = (byte *) ( ( (intptr_t)s_hunkData + 63 ) & ~63 );
	Hunk_Clear();

	Cmd_AddCommand( "meminfo", Com_Meminfo_f );
#ifdef HUNK_DEBUG
	Cmd_AddCommand( "hunklog", Hunk_Log );
	Cmd_AddCommand( "hunksmalllog", Hunk_SmallLog );
#endif
}

/*
====================
Hunk_MemoryRemaining
====================
*/
int Hunk_MemoryRemaining( void ) {
	int low, high;

#ifdef USE_MULTI_SEGMENT
	ZONE_LOCK( &g_hunkSpinlock );
#endif

	low = hunk_low.permanent > hunk_low.temp ? hunk_low.permanent : hunk_low.temp;
	high = hunk_high.permanent > hunk_high.temp ? hunk_high.permanent : hunk_high.temp;

#ifdef USE_MULTI_SEGMENT
	ZONE_UNLOCK( &g_hunkSpinlock );
#endif

	return s_hunkTotal - ( low + high );
}

/*
===================
Hunk_SetMark
===================
*/
void Hunk_SetMark( void ) {
#ifdef USE_MULTI_SEGMENT
	ZONE_LOCK( &g_hunkSpinlock );
#endif

	hunk_low.mark = hunk_low.permanent;
	hunk_high.mark = hunk_high.permanent;

#ifdef USE_MULTI_SEGMENT
	ZONE_UNLOCK( &g_hunkSpinlock );
#endif
}



/*
=================
Hunk_ClearToMark

The client calls this before starting a vid_restart or snd_restart
=================
*/
void Hunk_ClearToMark( void ) {
#ifdef USE_MULTI_SEGMENT
	ZONE_LOCK( &g_hunkSpinlock );
#endif

	hunk_low.permanent = hunk_low.temp = hunk_low.mark;
	hunk_high.permanent = hunk_high.temp = hunk_high.mark;

#ifdef USE_MULTI_SEGMENT
	ZONE_UNLOCK( &g_hunkSpinlock );
#endif
}

/*
=================
Hunk_CheckMark
=================
*/
qboolean Hunk_CheckMark( void ) {
	qboolean res;
#ifdef USE_MULTI_SEGMENT
	ZONE_LOCK( &g_hunkSpinlock );
#endif

	res = ( hunk_low.mark || hunk_high.mark ) ? qtrue : qfalse;

#ifdef USE_MULTI_SEGMENT
	ZONE_UNLOCK( &g_hunkSpinlock );
#endif
	return res;
}

void CL_ShutdownCGame( void );
void CL_ShutdownUI( void );
void SV_ShutdownGameProgs( void );

/*
=================
Hunk_Clear

The server calls this before shutting down or loading a new map
=================
*/
void Hunk_Clear( void ) {
#ifndef DEDICATED
	CL_ShutdownCGame();
	CL_ShutdownUI();
#endif
	SV_ShutdownGameProgs();
#ifndef DEDICATED
	CIN_CloseAllVideos();
#endif

#ifdef USE_MULTI_SEGMENT
	ZONE_LOCK( &g_hunkSpinlock );
#endif

	hunk_low.mark = 0;
	hunk_low.permanent = 0;
	hunk_low.temp = 0;
	hunk_low.tempHighwater = 0;

	hunk_high.mark = 0;
	hunk_high.permanent = 0;
	hunk_high.temp = 0;
	hunk_high.tempHighwater = 0;

	hunk_permanent = &hunk_low;
	hunk_temp = &hunk_high;

	Cvar_Set( "com_hunkused", va( "%i", hunk_low.permanent + hunk_high.permanent ) );
	Com_Printf( "Hunk_Clear: reset the hunk ok\n" );
	VM_Clear(); 
	
#ifdef HUNK_DEBUG
	hunkblocks = NULL;
#endif

#ifdef USE_MULTI_SEGMENT
	ZONE_UNLOCK( &g_hunkSpinlock );
#endif
}

static void Hunk_SwapBanks( void ) {
	hunkUsed_t  *swap;

	if ( hunk_temp->temp != hunk_temp->permanent ) {
		return;
	}

	if ( hunk_temp->tempHighwater - hunk_temp->permanent >
		 hunk_permanent->tempHighwater - hunk_permanent->permanent ) {
		swap = hunk_temp;
		hunk_temp = hunk_permanent;
		hunk_permanent = swap;
	}
}

/*
=================
Hunk_Alloc

Allocate permanent (until the hunk is cleared) memory
=================
*/
#ifdef HUNK_DEBUG
void *Hunk_AllocDebug( int size, ha_pref preference, char *label, char *file, int line ) {
#else
void *Hunk_Alloc( int size, ha_pref preference ) {
#endif
	void    *buf;

	if ( s_hunkData == NULL ) {
		Com_Error( ERR_FATAL, "Hunk_Alloc: Hunk memory system not initialized" );
	}

#ifdef USE_MULTI_SEGMENT
	ZONE_LOCK( &g_hunkSpinlock );
#endif

	Hunk_SwapBanks();

#ifdef HUNK_DEBUG
	size += sizeof( hunkblock_t );
#endif

	// Overhaul Alignment: Round allocations perfectly up to 64-byte Cache-Lines
	size = ( size + 63 ) & ~63;

	if ( hunk_low.temp + hunk_high.temp + size > s_hunkTotal ) {
#ifdef HUNK_DEBUG
		Hunk_Log();
		Hunk_SmallLog();
#ifdef USE_MULTI_SEGMENT
		ZONE_UNLOCK( &g_hunkSpinlock );
#endif
		Com_Error(ERR_DROP, "Hunk_Alloc failed on %i: %s, line: %d (%s)", size, file, line, label);
#else
#ifdef USE_MULTI_SEGMENT
		ZONE_UNLOCK( &g_hunkSpinlock );
#endif
		Com_Error(ERR_DROP, "Hunk_Alloc failed on %i", size);
#endif
	}

	if ( hunk_permanent == &hunk_low ) {
		buf = ( void * )( s_hunkData + hunk_permanent->permanent );
		hunk_permanent->permanent += size;
	} else {
		hunk_permanent->permanent += size;
		buf = ( void * )( s_hunkData + s_hunkTotal - hunk_permanent->permanent );
	}

	hunk_permanent->temp = hunk_permanent->permanent;

	Com_Memset( buf, 0, size );

#ifdef HUNK_DEBUG
	{
		hunkblock_t *block;

		block = (hunkblock_t *) buf;
		block->size = size - sizeof( hunkblock_t );
		block->file = file;
		block->label = label;
		block->line = line;
		block->next = hunkblocks;
		hunkblocks = block;
		buf = ( (byte *) buf ) + sizeof( hunkblock_t );
	}
#endif

	if ( ( hunk_low.permanent + hunk_high.permanent ) > com_hunkused->integer + 10000 ) {
		Cvar_Set( "com_hunkused", va( "%i", hunk_low.permanent + hunk_high.permanent ) );
	}

#ifdef USE_MULTI_SEGMENT
	ZONE_UNLOCK( &g_hunkSpinlock );
#endif

	return buf;
}

/*
=================
Hunk_AllocateTempMemory
=================
*/
void *Hunk_AllocateTempMemory( int size ) {
	void        *buf;
	hunkHeader_t    *hdr;
	int alignedHeaderSize;

	if ( s_hunkData == NULL ) {
		return Z_Malloc( size );
	}

#ifdef USE_MULTI_SEGMENT
	ZONE_LOCK( &g_hunkSpinlock );
#endif

	Hunk_SwapBanks();

	// Enforce 64-byte structural spacing inside temporary asset stacks
	alignedHeaderSize = ( sizeof( hunkHeader_t ) + 63 ) & ~63;
	size = ( ( size + 63 ) & ~63 ) + alignedHeaderSize;

	if ( hunk_temp->temp + hunk_permanent->permanent + size > s_hunkTotal ) {
#ifdef USE_MULTI_SEGMENT
		ZONE_UNLOCK( &g_hunkSpinlock );
#endif
		Com_Error( ERR_DROP, "Hunk_AllocateTempMemory: failed on %i", size );
	}

	if ( hunk_temp == &hunk_low ) {
		buf = ( void * )( s_hunkData + hunk_temp->temp );
		hunk_temp->temp += size;
	} else {
		hunk_temp->temp += size;
		buf = ( void * )( s_hunkData + s_hunkTotal - hunk_temp->temp );
	}

	if ( hunk_temp->temp > hunk_temp->tempHighwater ) {
		hunk_temp->tempHighwater = hunk_temp->temp;
	}

	hdr = (hunkHeader_t *)buf;
	buf = ( void * )( (byte *)hdr + alignedHeaderSize );

	hdr->magic = HUNK_MAGIC;
	hdr->size = size;

#ifdef USE_MULTI_SEGMENT
	ZONE_UNLOCK( &g_hunkSpinlock );
#endif

	return buf;
}

/*
==================
Hunk_FreeTempMemory
==================
*/
void Hunk_FreeTempMemory( void *buf ) {
	hunkHeader_t    *hdr;
	int alignedHeaderSize;

	if ( !buf ) {
		return;
	}

	if ( s_hunkData == NULL ) {
		Z_Free( buf );
		return;
	}

#ifdef USE_MULTI_SEGMENT
	ZONE_LOCK( &g_hunkSpinlock );
#endif

	alignedHeaderSize = ( sizeof( hunkHeader_t ) + 63 ) & ~63;
	hdr = (hunkHeader_t *)( (byte *)buf - alignedHeaderSize );
	
	if ( hdr->magic != HUNK_MAGIC ) {
#ifdef USE_MULTI_SEGMENT
		ZONE_UNLOCK( &g_hunkSpinlock );
#endif
		Com_Printf( "Hunk_FreeTempMemory: bad magic verification key (0x%x)\n", hdr->magic );
		return;
	}

	hdr->magic = HUNK_FREE_MAGIC;

	if ( hunk_temp == &hunk_low ) {
		if ( hdr == ( void * )( s_hunkData + hunk_temp->temp - hdr->size ) ) {
			hunk_temp->temp -= hdr->size;
		} else {
			Com_Printf( "Hunk_FreeTempMemory: block freed out of stack sequence order\n" );
		}
	} else {
		if ( hdr == ( void * )( s_hunkData + s_hunkTotal - hunk_temp->temp ) ) {
			hunk_temp->temp -= hdr->size;
		} else {
			Com_Printf( "Hunk_FreeTempMemory: block freed out of stack sequence order\n" );
		}
	}

#ifdef USE_MULTI_SEGMENT
	ZONE_UNLOCK( &g_hunkSpinlock );
#endif
}

/*
=================
Hunk_ClearTempMemory

The temp space is no longer needed. Reset the temp watermark back to permanent boundaries.
=================
*/
void Hunk_ClearTempMemory( void ) {
	if ( s_hunkData != NULL ) {
#ifdef USE_MULTI_SEGMENT
		ZONE_LOCK( &g_hunkSpinlock );
#endif

		hunk_temp->temp = hunk_temp->permanent;

#ifdef USE_MULTI_SEGMENT
		ZONE_UNLOCK( &g_hunkSpinlock );
#endif
	}
}


/*
===================================================================

EVENTS AND JOURNALING

In addition to these events, .cfg files are also copied to the
journaled file
===================================================================
*/


/*
=================
Com_InitJournaling
=================
*/
void Com_InitJournaling( void ) {
	Com_StartupVariable( "journal" );
	com_journal = Cvar_Get( "journal", "0", CVAR_INIT );
	if ( !com_journal->integer ) {
		return;
	}

	if ( com_journal->integer == 1 ) {
		Com_Printf( "Journaling events\n" );
		com_journalFile = FS_FOpenFileWrite( "journal.dat" );
		com_journalDataFile = FS_FOpenFileWrite( "journaldata.dat" );
	} else if ( com_journal->integer == 2 ) {
		Com_Printf( "Replaying journaled events\n" );
		FS_FOpenFileRead( "journal.dat", &com_journalFile, qtrue );
		FS_FOpenFileRead( "journaldata.dat", &com_journalDataFile, qtrue );
	}

	if ( !com_journalFile || !com_journalDataFile ) {
		Cvar_Set( "com_journal", "0" );
		com_journalFile = 0;
		com_journalDataFile = 0;
		Com_Printf( "Couldn't open journal files\n" );
	}
}

/*
========================================================================

EVENT LOOP

========================================================================
*/

#define MAX_QUEUED_EVENTS  256
#define MASK_QUEUED_EVENTS ( MAX_QUEUED_EVENTS - 1 )

static sysEvent_t  eventQueue[ MAX_QUEUED_EVENTS ];
static int         eventHead = 0;
static int         eventTail = 0;

#define MAX_PUSHED_EVENTS              1024 
static int com_pushedEventsHead = 0;
static int com_pushedEventsTail = 0;
static sysEvent_t com_pushedEvents[MAX_PUSHED_EVENTS];

// Spinlock dedicated purely to securing the engine input stream across worker threads
static zoneLock_t g_eventQueueSpinlock = 0;

/*
================
Com_QueueEvent

A time of 0 will get the current time
Ptr should either be null, or point to a block of data that can
be freed by the game later.
/*
================
Com_QueueEvent

Thread-safe, high-precision event injection
================
*/
void Com_QueueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr )
{
	sysEvent_t  *ev;

	// Acquire event loop atomic lock
	ZONE_LOCK( &g_eventQueueSpinlock );

	// High-FPS Optimization: Coalesce raw mouse input streams inside the queue to prevent frame stutter
	if ( type == SE_MOUSE && eventHead != eventTail )
	{
		ev = &eventQueue[ ( eventHead + MAX_QUEUED_EVENTS - 1 ) & MASK_QUEUED_EVENTS ];

		if ( ev->evType == SE_MOUSE )
		{
			ev->evValue += value;
			ev->evValue2 += value2;
			
			// If pointer data was passed to a mouse event mistakenly, prevent memory leaks
			if ( ptr ) {
				ZONE_UNLOCK( &g_eventQueueSpinlock );
				Z_Free( ptr );
			} else {
				ZONE_UNLOCK( &g_eventQueueSpinlock );
			}
			return;
		}
	}

	ev = &eventQueue[ eventHead & MASK_QUEUED_EVENTS ];

	// Handle Queue Overflows gracefully without leaking OS resources
	if ( eventHead - eventTail >= MAX_QUEUED_EVENTS )
	{
		Com_Printf("WARNING: Com_QueueEvent circular ring buffer overflow dropped event type %i\n", type);
		if ( ev->evPtr )
		{
			void *deadPtr = ev->evPtr;
			ev->evPtr = NULL;
			Z_Free( deadPtr );
		}
		eventTail++;
	}

	if ( time == 0 )
	{
		time = Sys_Milliseconds();
	}

	ev->evTime = time;
	ev->evType = type;
	ev->evValue = value;
	ev->evValue2 = value2;
	ev->evPtrLength = ptrLength;
	ev->evPtr = ptr;

	eventHead++;

	ZONE_UNLOCK( &g_eventQueueSpinlock );
}

/*
================
Com_GetSystemEvent

================
*/
sysEvent_t Com_GetSystemEvent( void )
{
	sysEvent_t  ev;
	char        *s;

	ZONE_LOCK( &g_eventQueueSpinlock );

	// If the queue contains unprocessed events, harvest them instantly
	if ( eventHead > eventTail )
	{
		ev = eventQueue[ eventTail & MASK_QUEUED_EVENTS ];
		eventQueue[ eventTail & MASK_QUEUED_EVENTS ].evPtr = NULL; // Clear ownership
		eventTail++;
		ZONE_UNLOCK( &g_eventQueueSpinlock );
		return ev;
	}

	ZONE_UNLOCK( &g_eventQueueSpinlock );

	// Poll system console commands safely outside of the ring-buffer lock boundary
	s = Sys_ConsoleInput();
	if ( s )
	{
		char  *b;
		int   len;

		len = (int)strlen( s ) + 1;
		b = Z_Malloc( len );
		if ( b ) {
			strcpy( b, s );
			Com_QueueEvent( 0, SE_CONSOLE, 0, 0, len, b );
		}
	}

	ZONE_LOCK( &g_eventQueueSpinlock );

	if ( eventHead > eventTail )
	{
		ev = eventQueue[ eventTail & MASK_QUEUED_EVENTS ];
		eventQueue[ eventTail & MASK_QUEUED_EVENTS ].evPtr = NULL;
		eventTail++;
		ZONE_UNLOCK( &g_eventQueueSpinlock );
		return ev;
	}

	// Queue is totally dry, pass a clean fallback ticking event frame back
	Com_Memset( &ev, 0, sizeof( ev ) );
	ev.evTime = Sys_Milliseconds();

	ZONE_UNLOCK( &g_eventQueueSpinlock );
	return ev;
}

/*
=================
Com_GetRealEvent
=================
*/
sysEvent_t  Com_GetRealEvent( void ) {
	int r;
	sysEvent_t ev;

	if ( com_journal->integer == 2 ) {
		r = FS_Read( &ev, sizeof( ev ), com_journalFile );
		if ( r != sizeof( ev ) ) {
			Com_Error( ERR_FATAL, "Com_GetRealEvent: Journal read streaming failure" );
		}
		if ( ev.evPtrLength ) {
			ev.evPtr = Z_Malloc( ev.evPtrLength );
			r = FS_Read( ev.evPtr, ev.evPtrLength, com_journalFile );
			if ( r != ev.evPtrLength ) {
				Com_Error( ERR_FATAL, "Com_GetRealEvent: Journal payload read streaming failure" );
			}
		}
	} else {
		ev = Com_GetSystemEvent();

		if ( com_journal->integer == 1 ) {
			r = FS_Write( &ev, sizeof( ev ), com_journalFile );
			if ( r != sizeof( ev ) ) {
				Com_Error( ERR_FATAL, "Com_GetRealEvent: Journal write disk serialization failure" );
			}
			if ( ev.evPtrLength && ev.evPtr ) {
				r = FS_Write( ev.evPtr, ev.evPtrLength, com_journalFile );
				if ( r != ev.evPtrLength ) {
					Com_Error( ERR_FATAL, "Com_GetRealEvent: Journal payload write disk serialization failure" );
				}
			}
		}
	}

	return ev;
}

/*
=================
Com_InitPushEvent
=================
*/
void Com_InitPushEvent( void ) {
	ZONE_LOCK( &g_eventQueueSpinlock );
	Com_Memset( com_pushedEvents, 0, sizeof( com_pushedEvents ) );
	com_pushedEventsHead = 0;
	com_pushedEventsTail = 0;
	ZONE_UNLOCK( &g_eventQueueSpinlock );
}

/*
=================
Com_PushEvent
=================
*/
void Com_PushEvent( sysEvent_t *event ) {
	sysEvent_t      *ev;

	if ( !event ) {
		return;
	}

	ZONE_LOCK( &g_eventQueueSpinlock );

	ev = &com_pushedEvents[ com_pushedEventsHead & ( MAX_PUSHED_EVENTS - 1 ) ];

	if ( com_pushedEventsHead - com_pushedEventsTail >= MAX_PUSHED_EVENTS ) {
		if ( ev->evPtr ) {
			void *deadPtr = ev->evPtr;
			ev->evPtr = NULL;
			Z_Free( deadPtr );
		}
		com_pushedEventsTail++;
	}

	*ev = *event;
	com_pushedEventsHead++;

	ZONE_UNLOCK( &g_eventQueueSpinlock );
}

/*
=================
Com_GetEvent
=================
*/
sysEvent_t  Com_GetEvent( void ) {
	sysEvent_t ev;

	ZONE_LOCK( &g_eventQueueSpinlock );
	if ( com_pushedEventsHead > com_pushedEventsTail ) {
		ev = com_pushedEvents[ com_pushedEventsTail & ( MAX_PUSHED_EVENTS - 1 ) ];
		com_pushedEvents[ com_pushedEventsTail & ( MAX_PUSHED_EVENTS - 1 ) ].evPtr = NULL; // Relinquish ownership
		com_pushedEventsTail++;
		ZONE_UNLOCK( &g_eventQueueSpinlock );
		return ev;
	}
	ZONE_UNLOCK( &g_eventQueueSpinlock );

	return Com_GetRealEvent();
}

/*
=================
Com_RunAndTimeServerPacket
=================
*/
void Com_RunAndTimeServerPacket( netadr_t *evFrom, msg_t *buf ) {
	int t1, t2, msec;

	t1 = 0;

	if ( com_speeds->integer ) {
		t1 = Sys_Milliseconds();
	}

	SV_PacketEvent( *evFrom, buf );

	if ( com_speeds->integer ) {
		t2 = Sys_Milliseconds();
		msec = t2 - t1;
		if ( com_speeds->integer == 3 ) {
			Com_Printf( "SV_PacketEvent time: %i\n", msec );
		}
	}
}

/*
=================
Com_EventLoop

Returns last event time
=================
*/
int Com_EventLoop( void ) {
	sysEvent_t ev;
	netadr_t evFrom;
	byte bufData[MAX_MSGLEN];
	msg_t buf;

	MSG_Init( &buf, bufData, sizeof( bufData ) );

	while ( 1 ) {
		ev = Com_GetEvent();

		// if no more events are available
		if ( ev.evType == SE_NONE ) {
			// manually send packet events for the loopback channel
			while ( NET_GetLoopPacket( NS_CLIENT, &evFrom, &buf ) ) {
				CL_PacketEvent( evFrom, &buf );
			}

			while ( NET_GetLoopPacket( NS_SERVER, &evFrom, &buf ) ) {
				// if the server just shut down, flush the events
				if ( com_sv_running->integer ) {
					Com_RunAndTimeServerPacket( &evFrom, &buf );
				}
			}

			return ev.evTime;
		}


		switch ( ev.evType ) {
		case SE_KEY:
			CL_KeyEvent( ev.evValue, ev.evValue2, ev.evTime );
			break;
		case SE_CHAR:
			CL_CharEvent( ev.evValue );
			break;
		case SE_MOUSE:
			CL_MouseEvent( ev.evValue, ev.evValue2, ev.evTime );
			break;
		case SE_JOYSTICK_AXIS:
			CL_JoystickEvent( ev.evValue, ev.evValue2, ev.evTime );
			break;
		case SE_CONSOLE:
			Cbuf_AddText( (char *)ev.evPtr );
			Cbuf_AddText( "\n" );
			break;
		default:
			Com_Error( ERR_FATAL, "Com_EventLoop: bad event type %i", ev.evType );
			break;
		}

		// free any block data
		if ( ev.evPtr ) {
			Z_Free( ev.evPtr );
		}
	}

	return 0;   // never reached
}

/*
================
Com_Milliseconds

Can be used for profiling, but will be journaled accurately
================
*/
int Com_Milliseconds( void ) {
	sysEvent_t ev;

	// get events and push them until we get a null event with the current time
	do {

		ev = Com_GetRealEvent();
		if ( ev.evType != SE_NONE ) {
			Com_PushEvent( &ev );
		}
	} while ( ev.evType != SE_NONE );

	return ev.evTime;
}

//============================================================================

/*
=============
Com_Error_f

Just throw a fatal error to
test error shutdown procedures
=============
*/
static void __attribute__((__noreturn__)) Com_Error_f (void) {
	if ( Cmd_Argc() > 1 ) {
		Com_Error( ERR_DROP, "Testing drop error" );
	} else {
		Com_Error( ERR_FATAL, "Testing fatal error" );
	}
}


/*
=============
Com_Freeze_f

Just freeze in place for a given number of seconds to test
error recovery
=============
*/
static void Com_Freeze_f( void ) {
	float s;
	int start, now;

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "freeze <seconds>\n" );
		return;
	}
	s = atof( Cmd_Argv( 1 ) );

	start = Com_Milliseconds();

	while ( 1 ) {
		now = Com_Milliseconds();
		if ( ( now - start ) * 0.001 > s ) {
			break;
		}
	}
}

/*
=================
Com_Crash_f

A way to force a bus error for development reasons
=================
*/
static void Com_Crash_f( void ) {
	* ( volatile int * ) 0 = 0x12345678;
}

/*
==================
Com_Setenv_f

For controlling environment variables
==================
*/
void Com_Setenv_f(void)
{
	int argc = Cmd_Argc();
	char *arg1 = Cmd_Argv(1);

	if(argc > 2)
	{
		char *arg2 = Cmd_ArgsFrom(2);
		
		Sys_SetEnv(arg1, arg2);
	}
	else if(argc == 2)
	{
		char *env = getenv(arg1);
		
		if(env)
			Com_Printf("%s=%s\n", arg1, env);
		else
			Com_Printf("%s undefined\n", arg1);
        }
}

/*
==================
Com_ExecuteCfg

For controlling environment variables
==================
*/

void Com_ExecuteCfg(void)
{
	Cbuf_ExecuteText(EXEC_NOW, "exec realrtcwdefault.cfg\n");
	if ( FS_ReadFile( "language.cfg", NULL ) > 0 ) {
		Cbuf_ExecuteText(EXEC_APPEND, "exec language.cfg\n");
	} else if ( FS_ReadFile( "Language.cfg", NULL ) > 0 ) {
		Cbuf_ExecuteText(EXEC_APPEND, "exec Language.cfg\n");
	}
	Cbuf_Execute(); // Always execute after exec to prevent text buffer overflowing

	if(!Com_SafeMode())
	{
		// skip the wolfconfig.cfg and autoexec.cfg if "safe" is on the command line
		Cbuf_ExecuteText(EXEC_NOW, "exec " RRTCW_BINDINGS "\n");
		Cbuf_Execute();
		Cbuf_ExecuteText(EXEC_NOW, "exec " RRTCW_VARIABLES "\n");
		Cbuf_Execute();
		Cbuf_ExecuteText(EXEC_NOW, "exec autoexec.cfg\n");
		Cbuf_Execute();
	}
}

/*
==================
Com_GameRestart

Change to a new mod properly with cleaning up cvars before switching.
==================
*/

void Com_GameRestart(int checksumFeed, qboolean disconnect)
{
	// make sure no recursion can be triggered
	if(!com_gameRestarting && com_fullyInitialized)
	{
		com_gameRestarting = qtrue;
		com_gameClientRestarting = com_cl_running->integer;

		// Kill server if we have one
		if(com_sv_running->integer)
			SV_Shutdown("Game directory changed");

		if(com_gameClientRestarting)
		{
			if(disconnect)
				CL_Disconnect(qfalse);
				
			CL_Shutdown("Game directory changed", disconnect, qfalse);
		}

		FS_Restart(checksumFeed);
	
		// Clean out any user and VM created cvars
		Cvar_Restart(qtrue);
		Com_ExecuteCfg();

		if(disconnect)
		{
			// We don't want to change any network settings if gamedir
			// change was triggered by a connect to server because the
			// new network settings might make the connection fail.
			NET_Restart_f();
		}

		if(com_gameClientRestarting)
		{
			CL_Init();
			CL_StartHunkUsers(qfalse);
		}
		
		com_gameRestarting = qfalse;
		com_gameClientRestarting = qfalse;
	}
}

/*
==================
Com_GameRestart_f

Expose possibility to change current running mod to the user
==================
*/

void Com_GameRestart_f(void)
{
    // If an argument is provided, treat it as the new fs_game
    if (Cmd_Argc() > 1) {
        Cvar_Set("fs_game", Cmd_Argv(1));
    }

    // Always restart the game
    Com_GameRestart(0, qtrue);
}

#ifndef STANDALONE

qboolean CL_CDKeyValidate( const char *key, const char *checksum );

// TTimo: centralizing the cl_cdkey stuff after I discovered a buffer overflow problem with the dedicated server version
//   not sure it's necessary to have different defaults for regular and dedicated, but I don't want to take the risk
#ifndef DEDICATED
char cl_cdkey[34] = "                                ";
#else
char cl_cdkey[34] = "123456789";
#endif

/*
=================
Com_ReadCDKey
=================
*/
void Com_ReadCDKey( const char *filename ) {
	fileHandle_t f;
	char buffer[33];
	char fbuffer[MAX_OSPATH];

	Com_sprintf(fbuffer, sizeof(fbuffer), "%s/rtcwkey", filename);

	FS_SV_FOpenFileRead( fbuffer, &f );
	if ( !f ) {
		Com_Memset( cl_cdkey, '\0', 17 );
		return;
	}

	Com_Memset( buffer, 0, sizeof( buffer ) );

	FS_Read( buffer, 16, f );
	FS_FCloseFile( f );

	if ( CL_CDKeyValidate( buffer, NULL ) ) {
		Q_strncpyz( cl_cdkey, buffer, 17 );
	} else {
		Com_Memset( cl_cdkey, '\0', 17 );
	}
}

/*
=================
Com_AppendCDKey
=================
*/
void Com_AppendCDKey( const char *filename ) {
	fileHandle_t f;
	char buffer[33];
	char fbuffer[MAX_OSPATH];

	Com_sprintf(fbuffer, sizeof(fbuffer), "%s/rtcwkey", filename);

	FS_SV_FOpenFileRead( fbuffer, &f );
	if ( !f ) {
		Com_Memset( &cl_cdkey[16], '\0', 17 );
		return;
	}

	Com_Memset( buffer, 0, sizeof( buffer ) );

	FS_Read( buffer, 16, f );
	FS_FCloseFile( f );

	if ( CL_CDKeyValidate( buffer, NULL ) ) {
		strcat( &cl_cdkey[16], buffer );
	} else {
		Com_Memset( &cl_cdkey[16], '\0', 17 );
	}
}

#ifndef DEDICATED
/*
=================
Com_WriteCDKey
=================
*/
static void Com_WriteCDKey( const char *filename, const char *ikey ) {
	fileHandle_t f;
	char fbuffer[MAX_OSPATH];
	char key[17];
#ifndef _WIN32
	mode_t			savedumask;
#endif

	Com_sprintf(fbuffer, sizeof(fbuffer), "%s/rtcwkey", filename);


	Q_strncpyz( key, ikey, 17 );

	if ( !CL_CDKeyValidate( key, NULL ) ) {
		return;
	}

#ifndef _WIN32
	savedumask = umask(0077);
#endif
	f = FS_SV_FOpenFileWrite( fbuffer );
	if ( !f ) {
		Com_Printf ("Couldn't write CD key to %s.\n", fbuffer );
		goto out;
	}

	FS_Write( key, 16, f );

	FS_Printf( f, "\n// generated by RTCW, do not modify\r\n" );
	FS_Printf( f, "// Do not give this file to ANYONE.\r\n" );
#ifdef __APPLE__ // TTimo
	FS_Printf( f, "// Aspyr will NOT ask you to send this file to them.\r\n" );
#else
	FS_Printf( f, "// id Software and Activision will NOT ask you to send this file to them.\r\n" );
#endif
	FS_FCloseFile( f );
out:
#ifndef _WIN32
	umask(savedumask);
#else
	;
#endif
}
#endif

#endif // STANDALONE

static void Com_DetectAltivec(void)
{
	// Only detect if user hasn't forcibly disabled it.
	if (com_altivec->integer) {
		static qboolean altivec = qfalse;
		static qboolean detected = qfalse;
		if (!detected) {
			altivec = ( Sys_GetProcessorFeatures( ) & CF_ALTIVEC );
			detected = qtrue;
		}

		if (!altivec) {
			Cvar_Set( "com_altivec", "0" );  // we don't have it! Disable support!
		}
	}
}

void Com_SetRecommended( qboolean vidrestart ) {
	cvar_t *cv;
	qboolean goodVideo;
	qboolean goodCPU;
	qboolean lowMemory;
	// will use this for recommended settings as well.. do i outside the lower check so it gets done even with command line stuff
	cv = Cvar_Get( "r_highQualityVideo", "1", CVAR_ARCHIVE );
	goodVideo = ( cv && cv->integer );
	goodCPU = Sys_GetHighQualityCPU();
	lowMemory = Sys_LowPhysicalMemory();

	if ( goodVideo && goodCPU ) {
		Com_Printf( "Found high quality video and CPU\n" );
		Cbuf_AddText( "exec highVidhighCPU.cfg\n" );
	} else if ( goodVideo && !goodCPU ) {
		Cbuf_AddText( "exec highVidlowCPU.cfg\n" );
		Com_Printf( "Found high quality video and low quality CPU\n" );
	} else if ( !goodVideo && goodCPU ) {
		Cbuf_AddText( "exec lowVidhighCPU.cfg\n" );
		Com_Printf( "Found low quality video and high quality CPU\n" );
	} else {
		Cbuf_AddText( "exec lowVidlowCPU.cfg\n" );
		Com_Printf( "Found low quality video and low quality CPU\n" );
	}

// (SA) set the cvar so the menu will reflect this on first run
	Cvar_Set( "ui_glCustom", "999" );   // 'recommended'


	if ( lowMemory ) {
		Com_Printf( "Found minimum memory requirement\n" );
		Cvar_Set( "s_khz", "11" );
		if ( !goodVideo ) {
			Cvar_Set( "r_lowMemTextureSize", "256" );
			Cvar_Set( "r_lowMemTextureThreshold", "40.0" );
		}
	}
	if ( vidrestart ) {
		Cbuf_AddText( "vid_restart\n" );
	}
}

/*
=================
Com_DetectSSE
Find out whether we have SSE support for Q_ftol function
=================
*/

#if id386 || idx64

static void Com_DetectSSE(void)
{
#if !idx64
	cpuFeatures_t feat;
	
	feat = Sys_GetProcessorFeatures();

	if(feat & CF_SSE)
	{
		if(feat & CF_SSE2)
			Q_SnapVector = qsnapvectorsse;
		else
			Q_SnapVector = qsnapvectorx87;

		Q_ftol = qftolsse;
#endif
		Q_VMftol = qvmftolsse;

		Com_Printf("SSE instruction set enabled\n");
#if !idx64
	}
	else
	{
		Q_ftol = qftolx87;
		Q_VMftol = qvmftolx87;
		Q_SnapVector = qsnapvectorx87;

		Com_Printf("SSE instruction set not available\n");
	}
#endif
}

#else

#define Com_DetectSSE()

#endif


#ifdef USE_MULTI_SEGMENT

// High-Performance 64-bit state for our Permuted Congruential Generator (PCG)
static uint64_t g_pcgRandomState = 0x853c49e6748fea9bULL;
static uint64_t g_pcgRandomInc   = 0xda3e39cb94b95bdbULL;
static zoneLock_t g_randSpinlock   = 0;

/*
========================
Com_PCGRand32

Thread-safe, ultra-fast 32-bit random number generator
Bypasses slow, lock-heavy standard library rand() implementation
========================
*/
uint32_t Com_PCGRand32( void ) {
	uint64_t oldState;
	uint32_t xorshifted;
	uint32_t rot;

	ZONE_LOCK( &g_randSpinlock );

	oldState = g_pcgRandomState;
	// Advance internal state equation lineally
	g_pcgRandomState = oldState * 6364136223846793005ULL + g_pcgRandomInc;

	ZONE_UNLOCK( &g_randSpinlock );

	// Calculate output permutation (XSH RR match)
	xorshifted = (uint32_t)( ( ( oldState >> 18u ) ^ oldState ) >> 27u );
	rot = (uint32_t)( oldState >> 59u );
	
	return ( xorshifted >> rot ) | ( xorshifted << ( ( -rot ) & 31u ) );
}

#endif // USE_MULTI_SEGMENT

/*
=================
Com_InitRand
Seed the random number generator using high-entropy OS bytes if possible=================
*/
static void Com_InitRand( void ) {
	unsigned int seed1 = 0;
	unsigned int seed2 = 0;

#ifdef USE_MULTI_SEGMENT
	ZONE_LOCK_INIT( &g_randSpinlock );
#endif

	if ( Sys_RandomBytes( (byte *)&seed1, sizeof( seed1 ) ) && 
		 Sys_RandomBytes( (byte *)&seed2, sizeof( seed2 ) ) ) {
#ifdef USE_MULTI_SEGMENT
		g_pcgRandomState = ( (uint64_t)seed1 << 32 ) | seed2;
#else
		srand( seed1 );
#endif
	} else {
#ifdef USE_MULTI_SEGMENT
		g_pcgRandomState = (uint64_t)time( NULL ) ^ 0x543d9b4e3a2c1ULL;
#else
		srand( (unsigned int)time( NULL ) );
#endif
	}
}

/*
==================
Com_RandomBytes

Fills a destination byte array with high-precision randomization
==================
*/
void Com_RandomBytes( byte *string, int len ) {
	int i;

	if ( !string || len <= 0 ) {
		return;
	}

	if ( Sys_RandomBytes( string, len ) ) {
		return;
	}

	// Dynamic fallback sequence loop utilizing our lockless register shifter
	for ( i = 0; i < len; i++ ) {
#ifdef USE_MULTI_SEGMENT
		string[i] = (byte)( Com_PCGRand32() % 256 );
#else
		string[i] = (byte)( rand() % 256 );
#endif
	}
}

/*
=================
Com_Init
=================
*/
void Com_Init( char *commandLine ) {
	char    *s;
	int	qport;

	Com_Printf( "%s %s %s\n", Q3_VERSION, PLATFORM_STRING, PRODUCT_DATE );

	if ( setjmp( abortframe ) ) {
		Sys_Error( "Error during initialization" );
	}

	// Clear queues
	Com_Memset( &eventQueue[ 0 ], 0, MAX_QUEUED_EVENTS * sizeof( sysEvent_t ) );

	// Initialize Multi-threaded Job System
	Sys_InitJobSystem();

	// initialize the weak pseudo-random number generator for use later.
	Com_InitRand();

	// do this before anything else decides to push events
	Com_InitPushEvent();

	Cvar_Init();

	// prepare enough of the subsystems to handle
	// cvar and command buffer management
	Com_ParseCommandLine( commandLine );

//	Swap_Init();
	Cbuf_Init();

	Com_DetectSSE();

	// override anything from the config files with command line args
	Com_StartupVariable( NULL );

	Com_InitZoneMemory();
	Cmd_Init ();

	// get the developer cvar set as early as possible
	com_developer = Cvar_Get("developer", "0", CVAR_TEMP);

	// done early so bind command exists
	CL_InitKeyCommands();

	com_standalone = Cvar_Get("com_standalone", "0", CVAR_ROM);
	com_basegame = Cvar_Get("com_basegame", BASEGAME, CVAR_INIT);
	com_homepath = Cvar_Get("com_homepath", "", CVAR_INIT|CVAR_PROTECTED);

	FS_InitFilesystem();

	Com_InitJournaling();

	// Add some commands here already so users can use them from config files
	Cmd_AddCommand ("setenv", Com_Setenv_f);
	if (com_developer && com_developer->integer)
	{
		Cmd_AddCommand ("error", Com_Error_f);
		Cmd_AddCommand ("crash", Com_Crash_f);
		Cmd_AddCommand ("freeze", Com_Freeze_f);
 	}
	Cmd_AddCommand ("quit", Com_Quit_f);
	Cmd_AddCommand ("changeVectors", MSG_ReportChangeVectors_f );
	Cmd_AddCommand ("writeconfig", Com_WriteConfig_f );
	Cmd_SetCommandCompletionFunc( "writeconfig", Cmd_CompleteCfgName );
	Cmd_AddCommand("game_restart", Com_GameRestart_f);
#ifdef USE_MIMALLOC_ZONE
	Cmd_AddCommand("mi_stats", Com_MimallocStats_f);
#endif

	Com_ExecuteCfg();

	// override anything from the config files with command line args
	Com_StartupVariable( NULL );

	// get dedicated here for proper hunk megs initialization
#ifdef DEDICATED
	com_dedicated = Cvar_Get ("dedicated", "1", CVAR_INIT);
	Cvar_CheckRange( com_dedicated, 1, 2, qtrue );
#else
	com_dedicated = Cvar_Get( "dedicated", "0", CVAR_LATCH );
	Cvar_CheckRange( com_dedicated, 0, 2, qtrue );
#endif
	// allocate the stack based hunk allocator
	Com_InitHunkMemory();

	// if any archived cvars are modified after this, we will trigger a writing
	// of the config file
	cvar_modifiedFlags &= ~CVAR_ARCHIVE;

	//
	// init commands and vars
	//
	com_altivec = Cvar_Get ("com_altivec", "1", CVAR_ARCHIVE);
	com_maxfps = Cvar_Get( "com_maxfps", "76", CVAR_ARCHIVE );
	com_blood = Cvar_Get( "com_blood", "1", CVAR_ARCHIVE );
	com_atmosphericEffect = Cvar_Get("com_atmosphericEffect", "", CVAR_ROM);

	com_logfile = Cvar_Get( "logfile", "0", CVAR_TEMP );

	com_timescale = Cvar_Get( "timescale", "1", CVAR_CHEAT | CVAR_SYSTEMINFO );
	com_fixedtime = Cvar_Get( "fixedtime", "0", CVAR_CHEAT );
	com_showtrace = Cvar_Get( "com_showtrace", "0", CVAR_CHEAT );
	com_speeds = Cvar_Get( "com_speeds", "0", 0 );
	com_timedemo = Cvar_Get( "timedemo", "0", CVAR_CHEAT );
	com_cameraMode = Cvar_Get( "com_cameraMode", "0", CVAR_CHEAT );

	cl_paused = Cvar_Get( "cl_paused", "0", CVAR_ROM );
	sv_paused = Cvar_Get( "sv_paused", "0", CVAR_ROM );
	cl_packetdelay = Cvar_Get ("cl_packetdelay", "0", CVAR_CHEAT);
	sv_packetdelay = Cvar_Get ("sv_packetdelay", "0", CVAR_CHEAT);
	com_sv_running = Cvar_Get( "sv_running", "0", CVAR_ROM );
	com_cl_running = Cvar_Get( "cl_running", "0", CVAR_ROM );
	com_buildScript = Cvar_Get( "com_buildScript", "0", 0 );
	com_ansiColor = Cvar_Get( "com_ansiColor", "0", CVAR_ARCHIVE );

	com_unfocused = Cvar_Get( "com_unfocused", "0", CVAR_ROM );
	com_maxfpsUnfocused = Cvar_Get( "com_maxfpsUnfocused", "0", CVAR_ARCHIVE );
	com_minimized = Cvar_Get( "com_minimized", "0", CVAR_ROM );
	com_maxfpsMinimized = Cvar_Get( "com_maxfpsMinimized", "0", CVAR_ARCHIVE );
	com_abnormalExit = Cvar_Get( "com_abnormalExit", "0", CVAR_ROM );
	com_pause = Cvar_Get( "com_pause", "0", CVAR_ROM );
	com_busyWait = Cvar_Get("com_busyWait", "0", CVAR_ARCHIVE);
	Cvar_Get("com_errorMessage", "", CVAR_ROM | CVAR_NORESTART);

#ifdef CINEMATICS_INTRO
	com_introPlayed = Cvar_Get( "com_introplayed", "0", CVAR_ARCHIVE );
#endif
	com_recommendedSet = Cvar_Get( "com_recommendedSet", "0", CVAR_ARCHIVE );

	Cvar_Get( "savegame_loading", "0", CVAR_ROM );

	s = va("%s %s %s", Q3_VERSION, PLATFORM_STRING, PRODUCT_DATE );
	com_version = Cvar_Get ("version", s, CVAR_ROM | CVAR_SERVERINFO );
	com_gamename = Cvar_Get("com_gamename", GAMENAME_FOR_MASTER, CVAR_SERVERINFO | CVAR_INIT);
	com_protocol = Cvar_Get("com_protocol", va("%i", PROTOCOL_VERSION), CVAR_SERVERINFO | CVAR_INIT);
#ifdef LEGACY_PROTOCOL
	com_legacyprotocol = Cvar_Get("com_legacyprotocol", va("%i", PROTOCOL_LEGACY_VERSION), CVAR_INIT);

	// Keep for compatibility with old mods / mods that haven't updated yet.
	if(com_legacyprotocol->integer > 0)
		Cvar_Get("protocol", com_legacyprotocol->string, CVAR_ROM);
	else
#endif
		Cvar_Get("protocol", com_protocol->string, CVAR_ROM);

#ifndef DEDICATED
	con_autochat = Cvar_Get("con_autochat", "1", CVAR_ARCHIVE);
#endif

	com_hunkused = Cvar_Get( "com_hunkused", "0", 0 );

	Sys_Init();

	Sys_InitPIDFile( FS_GetCurrentGameDir() );

	// Pick a random port value
	Com_RandomBytes( (byte*)&qport, sizeof(int) );
	Netchan_Init( qport & 0xffff );

	VM_Init();
	SV_Init();

	com_dedicated->modified = qfalse;

#ifndef DEDICATED
	CL_Init();
#endif

	// set com_frameTime so that if a map is started on the
	// command line it will still be able to count on com_frameTime
	// being random enough for a serverid
	com_frameTime = Com_Milliseconds();

	// add + commands from command line
	if ( !Com_AddStartupCommands() ) {
		// if the user didn't give any commands, run default action
		if ( !com_dedicated->integer ) {
#ifdef CINEMATICS_LOGO
			//Cbuf_AddText ("cinematic " CINEMATICS_LOGO "\n");
#endif
#ifdef CINEMATICS_INTRO
			if ( !com_introPlayed->integer ) {
				//Cvar_Set( com_introPlayed->name, "1" );		//----(SA)	force this to get played every time (but leave cvar for override)
				Cbuf_AddText( "cinematic " CINEMATICS_INTRO " 3\n" );
				//Cvar_Set( "nextmap", "cinematic " CINEMATICS_INTRO );
			}
#endif
		}
	}

	// start in full screen ui mode
	Cvar_Set( "r_uiFullScreen", "1" );

	CL_StartHunkUsers( qfalse );

	if ( !com_recommendedSet->integer ) {
		Com_SetRecommended( qtrue );
		Cvar_Set( "com_recommendedSet", "1" );
	}

	com_fullyInitialized = qtrue;

	// always set the cvar, but only print the info if it makes sense.
	Com_DetectAltivec();
#if idppc
	Com_Printf ("Altivec support is %s\n", com_altivec->integer ? "enabled" : "disabled");
#endif

	com_pipefile = Cvar_Get( "com_pipefile", "", CVAR_ARCHIVE|CVAR_LATCH );
	if( com_pipefile->string[0] )
	{
		pipefile = FS_FCreateOpenPipeFile( com_pipefile->string );
	}

	Com_Printf ("--- Common Initialization Complete ---\n");
}

/*
===============
Com_ReadFromPipe

Read whatever is in com_pipefile, if anything, and execute it
===============
*/
void Com_ReadFromPipe( void )
{
	static char buf[MAX_STRING_CHARS];
	static int accu = 0;
	int read;

	if( !pipefile )
		return;

	while( ( read = FS_Read( buf + accu, sizeof( buf ) - accu - 1, pipefile ) ) > 0 )
	{
		char *brk = NULL;
		int i;

		for( i = accu; i < accu + read; ++i )
		{
			if( buf[ i ] == '\0' )
				buf[ i ] = '\n';
			if( buf[ i ] == '\n' || buf[ i ] == '\r' )
				brk = &buf[ i + 1 ];
		}
		buf[ accu + read ] = '\0';

		accu += read;

		if( brk )
		{
			char tmp = *brk;
			*brk = '\0';
			Cbuf_ExecuteText( EXEC_APPEND, buf );
			*brk = tmp;

			accu -= brk - buf;
			memmove( buf, brk, accu + 1 );
		}
		else if( accu >= sizeof( buf ) - 1 ) // full
		{
			Cbuf_ExecuteText( EXEC_APPEND, buf );
			accu = 0;
		}
	}
}

//==================================================================

void Com_WriteVariablesToFile( const char *filename ) {
	fileHandle_t f;

	f = FS_FOpenFileWrite( filename );
	if ( !f ) {
		Com_Printf( "Couldn't write %s.\n", filename );
		return;
	}

	FS_Printf( f, "// generated by RTCW, do not modify\n" );
	Cvar_WriteVariables( f );
	FS_FCloseFile( f );
}

void Com_WriteBindingsToFile( const char *filename ) {
	fileHandle_t f;

	f = FS_FOpenFileWrite( filename );
	if ( !f ) {
		Com_Printf( "Couldn't write %s.\n", filename );
		return;
	}

	FS_Printf( f, "// generated by RTCW, do not modify\n" );
	Key_WriteBindings( f );
	FS_FCloseFile( f );
}


/*
===============
Com_WriteConfiguration

Writes key bindings and archived cvars to config file if modified
===============
*/
void Com_WriteConfiguration( void ) {
	// if we are quiting without fully initializing, make sure
	// we don't write out anything
	if ( !com_fullyInitialized ) {
		return;
	}

	if ( !( cvar_modifiedFlags & CVAR_ARCHIVE ) ) {
		return;
	}
	cvar_modifiedFlags &= ~CVAR_ARCHIVE;

	Com_WriteBindingsToFile (RRTCW_BINDINGS);
	Com_WriteVariablesToFile (RRTCW_VARIABLES);

	// not needed for dedicated or standalone
#if !defined(DEDICATED) && !defined(STANDALONE)
	if(!com_standalone->integer)
	{
		const char *gamedir;
		gamedir = Cvar_VariableString( "fs_game" );
		if ( UI_usesUniqueCDKey() && gamedir[0] != 0 ) {
			Com_WriteCDKey( gamedir, &cl_cdkey[16] );
		} else {
			Com_WriteCDKey( BASEGAME, cl_cdkey );
		}
	}
#endif
}


/*
===============
Com_WriteConfig_f

Write the config file to a specific name
===============
*/
void Com_WriteConfig_f( void ) {
    char filename[MAX_QPATH];
    char base[MAX_QPATH];
    char cvarsName[MAX_QPATH];
    char bindsName[MAX_QPATH];

    if ( Cmd_Argc() != 2 ) {
        Com_Printf( "Usage: writeconfig <filename>\n" );
        return;
    }

    Q_strncpyz( filename, Cmd_Argv(1), sizeof(filename) );
    COM_DefaultExtension( filename, sizeof(filename), ".cfg" );

    if ( !COM_CompareExtension(filename, ".cfg") ) {
        Com_Printf("Com_WriteConfig_f: Only the \".cfg\" extension is supported!\n");
        return;
    }

    // strip .cfg to build two names
    Q_strncpyz( base, filename, sizeof(base) );
    base[strlen(base) - 4] = '\0';

    Com_sprintf( cvarsName, sizeof(cvarsName), "%s_cvars.cfg", base );
    Com_sprintf( bindsName, sizeof(bindsName), "%s_binds.cfg", base );

    Com_Printf( "Writing %s and %s.\n", cvarsName, bindsName );
    Com_WriteVariablesToFile( cvarsName );
    Com_WriteBindingsToFile( bindsName );
}

/*
================
Com_ModifyMsec
================
*/
int Com_ModifyMsec( int msec ) {
	int clampTime;

	//
	// modify time for debugging values
	//
	if ( com_fixedtime->integer ) {
		msec = com_fixedtime->integer;
	} else if ( com_timescale->value ) {
		msec *= com_timescale->value;
//	} else if (com_cameraMode->integer) {
//		msec *= com_timescale->value;
	}

	// don't let it scale below 1 msec
	if ( msec < 1 && com_timescale->value ) {
		msec = 1;
	}

	if ( com_dedicated->integer ) {
		// dedicated servers don't want to clamp for a much longer
		// period, because it would mess up all the client's views
		// of time.
		if (com_sv_running->integer && msec > 500)
			Com_Printf( "Hitch warning: %i msec frame time\n", msec );

		clampTime = 5000;
	} else
	if ( !com_sv_running->integer ) {
		// clients of remote servers do not want to clamp time, because
		// it would skew their view of the server's time temporarily
		clampTime = 5000;
	} else {
		// for local single player gaming
		// we may want to clamp the time to prevent players from
		// flying off edges when something hitches.
		clampTime = 200;
	}

	if ( msec > clampTime ) {
		msec = clampTime;
	}

	return msec;
}

/*
=================
Com_TimeVal
=================
*/

int Com_TimeVal(int minMsec)
{
	int timeVal;

	timeVal = Sys_Milliseconds() - com_frameTime;

	if ( timeVal >= minMsec ) {
		return 0;
	}
	
	return minMsec - timeVal;
}


/*
=================
Com_Frame
=================
*/
void Com_Frame( void ) {
	int msec, minMsec;
	int timeVal, timeValSV;
	static int lastTime = 0;
	static int bias = 0;

	int timeBeforeFirstEvents;
	int timeBeforeServer;
	int timeBeforeEvents;
	int timeBeforeClient;
	int timeAfter;

	if ( setjmp( abortframe ) ) {
		return;         // an ERR_DROP was thrown, gracefully intercept frame drop
	}

	timeBeforeFirstEvents = 0;
	timeBeforeServer = 0;
	timeBeforeEvents = 0;
	timeBeforeClient = 0;
	timeAfter = 0;

	// Automatically serialize modified CVAR states to storage tables
	Com_WriteConfiguration();

	if ( com_speeds->integer ) {
		timeBeforeFirstEvents = Sys_Milliseconds();
	}

	// Calculate target frame pacing budget
	if ( !com_timedemo->integer ) {
		if ( com_dedicated->integer ) {
			minMsec = SV_FrameMsec();
		} else {
			if ( com_minimized->integer && com_maxfpsMinimized->integer > 0 ) {
				minMsec = 1000 / com_maxfpsMinimized->integer;
			} else if ( com_unfocused->integer && com_maxfpsUnfocused->integer > 0 ) {
				minMsec = 1000 / com_maxfpsUnfocused->integer;
			} else if ( com_maxfps->integer > 0 ) {
				minMsec = 1000 / com_maxfps->integer;
			} else {
				minMsec = 1;
			}
			
			timeVal = com_frameTime - lastTime;
			bias += timeVal - minMsec;
			
			if ( bias > minMsec ) {
				bias = minMsec;
			}
			
			// Compensate for physical scheduling offsets dynamically
			minMsec -= bias;
		}
	} else {
		minMsec = 1;
	}

	// --- Dual-Stage Hybrid Frame Limiter Execution Block ---
	while ( 1 ) {
		if ( com_sv_running->integer ) {
			timeValSV = SV_SendQueuedPackets();
			timeVal = Com_TimeVal( minMsec );
			if ( timeValSV < timeVal ) {
				timeVal = timeValSV;
			}
		} else {
			timeVal = Com_TimeVal( minMsec );
		}

		// Frame budget achieved, break out instantly to render
		if ( timeVal <= 0 ) {
			break;
		}

		// STAGE 1: Coarse yielding if we are far ahead of our timeline (> 2ms)
		if ( timeVal > 2 && !com_busyWait->integer ) {
			NET_Sleep( 1 ); 
		} else {
			// STAGE 2: High-Precision active spin-wait loop once we enter the critical boundary zone
			NET_Sleep( 0 ); 
		}
	}

	// Update device controls and input streams safely
	IN_Frame();
	
	lastTime = com_frameTime;
	com_frameTime = Com_EventLoop();
	
	msec = com_frameTime - lastTime;

	Cbuf_Execute();

	if ( com_altivec->modified ) {
		Com_DetectAltivec();
		com_altivec->modified = qfalse;
	}

	msec = Com_ModifyMsec( msec );

	// --- Server Logic Step ---
	if ( com_speeds->integer ) {
		timeBeforeServer = Sys_Milliseconds();
	}

	SV_Frame( msec );

	if ( com_dedicated->modified ) {
		Cvar_Get( "dedicated", "0", 0 );
		com_dedicated->modified = qfalse;
		if ( !com_dedicated->integer ) {
			SV_Shutdown( "dedicated set to 0" );
			CL_FlushMemory();
		}
	}

#ifndef DEDICATED
	// --- Client & Visual Logic Step ---
	if ( com_speeds->integer ) {
		timeBeforeEvents = Sys_Milliseconds();
	}
	
	Com_EventLoop();
	Cbuf_Execute();

	if ( com_speeds->integer ) {
		timeBeforeClient = Sys_Milliseconds();
	}

	CL_Frame( msec );

	if ( com_speeds->integer ) {
		timeAfter = Sys_Milliseconds();
	}
#else
	if ( com_speeds->integer ) {
		timeAfter = Sys_Milliseconds();
		timeBeforeEvents = timeAfter;
		timeBeforeClient = timeAfter;
	}
#endif

	NET_FlushPacketQueue();

	// Timing Profiler diagnostics tracking
	if ( com_speeds->integer ) {
		int all, sv, ev, cl;

		all = timeAfter - timeBeforeServer;
		sv = timeBeforeEvents - timeBeforeServer;
		ev = timeBeforeServer - timeBeforeFirstEvents + timeBeforeClient - timeBeforeEvents;
		cl = timeAfter - timeBeforeClient;
		sv -= time_game;
		cl -= time_frontend + time_backend;

		Com_Printf( "frame:%i all:%3i sv:%3i ev:%3i cl:%3i gm:%3i rf:%3i bk:%3i\n",
					com_frameNumber, all, sv, ev, cl, time_game, time_frontend, time_backend );
	}

	if ( com_showtrace->integer ) {
		extern int c_traces, c_brush_traces, c_patch_traces;
		extern int c_pointcontents;

		Com_Printf( "%4i traces  (%ib %ip) %4i points\n", c_traces,
					c_brush_traces, c_patch_traces, c_pointcontents );
		c_traces = 0;
		c_brush_traces = 0;
		c_patch_traces = 0;
		c_pointcontents = 0;
	}

	Com_ReadFromPipe();

	com_frameNumber++;
}
/*
=================
Com_Shutdown
=================
*/
void Com_Shutdown( void ) {

	// write config file if anything changed
	Com_WriteConfiguration();

	if ( logfile ) {
		FS_FCloseFile( logfile );
		logfile = 0;
	}

	if ( com_journalFile ) {
		FS_FCloseFile( com_journalFile );
		com_journalFile = 0;
	}

	if( pipefile ) {
		FS_FCloseFile( pipefile );
		FS_HomeRemove( com_pipefile->string );
	}

	Sys_ShutdownJobSystem();

}


/*
===========================================
command line completion
===========================================
*/

/*
==================
Field_Clear
==================
*/
void Field_Clear( field_t *edit ) {
	memset( edit->buffer, 0, MAX_EDIT_LINE );
	edit->cursor = 0;
	edit->scroll = 0;
}

static const char *completionString;
static char shortestMatch[MAX_TOKEN_CHARS];
static int matchCount;
// field we are working on, passed to Field_AutoComplete(&g_consoleCommand for instance)
static field_t *completionField;

/*
===============
FindMatches

===============
*/
static void FindMatches( const char *s ) {
	int i;

	if ( Q_stricmpn( s, completionString, strlen( completionString ) ) ) {
		return;
	}
	matchCount++;
	if ( matchCount == 1 ) {
		Q_strncpyz( shortestMatch, s, sizeof( shortestMatch ) );
		return;
	}

	// cut shortestMatch to the amount common with s
	for ( i = 0 ; shortestMatch[i] ; i++ ) {
		if ( i >= strlen( s ) ) {
			shortestMatch[i] = 0;
			break;
		}

		if ( tolower( shortestMatch[i] ) != tolower( s[i] ) ) {
			shortestMatch[i] = 0;
		}
	}
}

/*
===============
PrintMatches

===============
*/
static void PrintMatches( const char *s ) {
	if ( !Q_stricmpn( s, shortestMatch, strlen( shortestMatch ) ) ) {
		Com_Printf( "    %s\n", s );
	}
}

/*
===============
PrintCvarMatches

===============
*/
static void PrintCvarMatches( const char *s ) {
	char value[ TRUNCATE_LENGTH ];

	if ( !Q_stricmpn( s, shortestMatch, strlen( shortestMatch ) ) ) {
		Com_TruncateLongString( value, Cvar_VariableString( s ) );
		Com_Printf( "    %s = \"%s\"\n", s, value );
	}
}

/*
===============
Field_FindFirstSeparator
===============
*/
static char *Field_FindFirstSeparator( char *s )
{
	int i;

	for( i = 0; i < strlen( s ); i++ )
	{
		if( s[ i ] == ';' )
			return &s[ i ];
	}

	return NULL;
}

/*
===============
Field_Complete
===============
*/
static qboolean Field_Complete( void )
{
	int completionOffset;

	if( matchCount == 0 )
		return qtrue;

	completionOffset = strlen( completionField->buffer ) - strlen( completionString );

	Q_strncpyz( &completionField->buffer[ completionOffset ], shortestMatch,
		sizeof( completionField->buffer ) - completionOffset );

	completionField->cursor = strlen( completionField->buffer );

	if( matchCount == 1 )
	{
		Q_strcat( completionField->buffer, sizeof( completionField->buffer ), " " );
		completionField->cursor++;
		return qtrue;
	}

	Com_Printf( "]%s\n", completionField->buffer );

	return qfalse;
}

#ifndef DEDICATED
/*
===============
Field_CompleteKeyname
===============
*/
void Field_CompleteKeyname( void )
{
	matchCount = 0;
	shortestMatch[ 0 ] = 0;

	Key_KeynameCompletion( FindMatches );

	if( !Field_Complete( ) )
		Key_KeynameCompletion( PrintMatches );
}
#endif

/*
===============
Field_CompleteFilename
===============
*/
void Field_CompleteFilename( const char *dir,
		const char *ext, qboolean stripExt, qboolean allowNonPureFilesOnDisk )
{
	matchCount = 0;
	shortestMatch[ 0 ] = 0;

	FS_FilenameCompletion( dir, ext, stripExt, FindMatches, allowNonPureFilesOnDisk );

	if( !Field_Complete( ) )
		FS_FilenameCompletion( dir, ext, stripExt, PrintMatches, allowNonPureFilesOnDisk );
}

/*
===============
Field_CompleteCommand
===============
*/
void Field_CompleteCommand( char *cmd,
		qboolean doCommands, qboolean doCvars )
{
	int		completionArgument = 0;

	// Skip leading whitespace and quotes
	cmd = Com_SkipCharset( cmd, " \"" );

	Cmd_TokenizeStringIgnoreQuotes( cmd );
	completionArgument = Cmd_Argc( );

	// If there is trailing whitespace on the cmd
	if( *( cmd + strlen( cmd ) - 1 ) == ' ' )
	{
		completionString = "";
		completionArgument++;
	}
	else
		completionString = Cmd_Argv( completionArgument - 1 );

#ifndef DEDICATED
	// add a '\' to the start of the buffer if it might be sent as chat otherwise
	if( con_autochat->integer && completionField->buffer[ 0 ] &&
			completionField->buffer[ 0 ] != '\\' )
	{
		if( completionField->buffer[ 0 ] != '/' )
		{
			// Buffer is full, refuse to complete
			if( strlen( completionField->buffer ) + 1 >=
				sizeof( completionField->buffer ) )
				return;

			memmove( &completionField->buffer[ 1 ],
				&completionField->buffer[ 0 ],
				strlen( completionField->buffer ) + 1 );
			completionField->cursor++;
		}

		completionField->buffer[ 0 ] = '\\';
	}
#endif

	if( completionArgument > 1 )
	{
		const char *baseCmd = Cmd_Argv( 0 );
		char *p;

#ifndef DEDICATED
		// This should always be true
		if( baseCmd[ 0 ] == '\\' || baseCmd[ 0 ] == '/' )
			baseCmd++;
#endif

		if( ( p = Field_FindFirstSeparator( cmd ) ) )
			Field_CompleteCommand( p + 1, qtrue, qtrue ); // Compound command
		else
			Cmd_CompleteArgument( baseCmd, cmd, completionArgument ); 
	}
	else
	{
		if( completionString[0] == '\\' || completionString[0] == '/' )
			completionString++;

		matchCount = 0;
		shortestMatch[ 0 ] = 0;

		if( strlen( completionString ) == 0 )
			return;

		if( doCommands )
			Cmd_CommandCompletion( FindMatches );

		if( doCvars )
			Cvar_CommandCompletion( FindMatches );

		if( !Field_Complete( ) )
		{
			// run through again, printing matches
			if( doCommands )
				Cmd_CommandCompletion( PrintMatches );

			if( doCvars )
				Cvar_CommandCompletion( PrintCvarMatches );
		}
	}
}

/*
===============
Field_AutoComplete

Perform Tab expansion
===============
*/
void Field_AutoComplete( field_t *field )
{
	completionField = field;

	Field_CompleteCommand( completionField->buffer, qtrue, qtrue );
}

/*



/*
==================
Com_IsVoipTarget

Returns non-zero if given clientNum is enabled in voipTargets, zero otherwise.
If clientNum is negative return if any bit is set.
==================
*/
qboolean Com_IsVoipTarget(uint8_t *voipTargets, int voipTargetsSize, int clientNum)
{
	int index;
	if(clientNum < 0)
	{
		for(index = 0; index < voipTargetsSize; index++)
		{
			if(voipTargets[index])
				return qtrue;
		}
		
		return qfalse;
	}

	index = clientNum >> 3;
	
	if(index < voipTargetsSize)
		return (voipTargets[index] & (1 << (clientNum & 0x07)));

	return qfalse;
}

