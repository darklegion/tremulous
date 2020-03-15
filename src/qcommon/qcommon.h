/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2013 Darklegion Development
Copyright (C) 2015-2019 GrangerHub

This file is part of Tremulous.

Tremulous is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 3 of the License,
or (at your option) any later version.

Tremulous is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tremulous; if not, see <https://www.gnu.org/licenses/>

===========================================================================
*/
// qcommon.h -- definitions common between client and server, but not game.or ref modules
#ifndef _QCOMMON_H_
#define _QCOMMON_H_

#include <stdbool.h>

#include "cm_public.h"

//Ignore __attribute__ on non-gcc platforms
#ifndef __GNUC__
#ifndef __attribute__
#define __attribute__(x)
#endif
#endif

struct netadr_t;
struct msg_t;

/*
==============================================================

PROTOCOL

==============================================================
*/

#define	PROTOCOL_VERSION	71

// maintain a list of compatible protocols for demo playing
// NOTE: that stuff only works with two digits protocols
extern int demo_protocols[];

// override on command line, config files etc.
#ifndef MASTER_SERVER_NAME
#define MASTER_SERVER_NAME	"master.tremulous.net"
#endif

#define	PORT_MASTER			30700
#define	PORT_SERVER			30720
#define	ALT1PORT_MASTER			30700
#define	ALT1PORT_SERVER			30721
#define	ALT2PORT_MASTER			30710
#define	ALT2PORT_SERVER			30722
#define	NUM_SERVER_PORTS	4		// broadcast scan this many ports after
									// PORT_SERVER so a single machine can
									// run multiple servers


// the svc_strings[] array in cl_parse.c should mirror this
//
// server to client
//
enum svc_ops_e {
	svc_bad,
	svc_nop,
	svc_gamestate,
	svc_configstring,			// [short] [string] only in gamestate messages
	svc_baseline,				// only in gamestate messages
	svc_serverCommand,			// [string] to be executed by client game module
	svc_download,				// [short] size [size bytes]
	svc_snapshot,
	svc_EOF,

// new commands, supported only by ioquake3 protocol but not legacy
	svc_voipSpeex,     // not wrapped in USE_VOIP, so this value is reserved.
	svc_voipOpus,      //
};


//
// client to server
//
enum clc_ops_e {
	clc_bad,
	clc_nop, 		
	clc_move,				// [[usercmd_t]
	clc_moveNoDelta,		// [[usercmd_t]
	clc_clientCommand,		// [string] message
	clc_EOF,

// new commands, supported only by ioquake3 protocol but not legacy
	clc_voipSpeex,   // not wrapped in USE_VOIP, so this value is reserved.
	clc_voipOpus,    //
};

//#include "cvar.h"

typedef struct cvar_s cvar_t;

/*
==============================================================

Edit fields and command line history/completion

==============================================================
*/

#define	MAX_EDIT_LINE	256
typedef struct {
	int		cursor;
	int		scroll;
	int		widthInChars;
	char	buffer[MAX_EDIT_LINE];
} field_t;

void Field_Clear( field_t *edit );
void Field_AutoComplete( field_t *edit );
void Field_CompleteKeyname( void );
void Field_CompleteFilename( const char *dir, const char *ext, bool stripExt, bool allowNonPureFilesOnDisk );
void Field_CompleteCommand( char *cmd, bool doCommands, bool doCvars );
void Field_CompletePlayerName( const char **names, int count );
void Field_CompleteList( char *listJson );

/*
==============================================================

MISC

==============================================================
*/

// centralized and cleaned, that's the max string you can send to a Com_Printf / Com_DPrintf (above gets truncated)
#define	MAXPRINTMSG	4096


typedef enum {
	// SE_NONE must be zero
	SE_NONE = 0,		// evTime is still valid
	SE_KEY,			// evValue is a key code, evValue2 is the down flag
	SE_CHAR,		// evValue is an ascii char
	SE_MOUSE,		// evValue and evValue2 are relative signed x / y moves
	SE_JOYSTICK_AXIS,	// evValue is an axis number and evValue2 is the current state (-127 to 127)
	SE_CONSOLE		// evPtr is a char*
} sysEventType_t;

typedef struct {
	int				evTime;
	sysEventType_t	evType;
	int				evValue, evValue2;
	int				evPtrLength;	// bytes of data pointed to by evPtr, for journaling
	void			*evPtr;			// this must be manually freed if not NULL
} sysEvent_t;

void		Com_QueueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr );
int			Com_EventLoop( void );
sysEvent_t	Com_GetSystemEvent( void );

char		*CopyString( const char *in );
void		Info_Print( const char *s );

void		Com_BeginRedirect (char *buffer, int buffersize, void (*flush)(char *));
void		Com_EndRedirect( void );

//#ifndef __Q_SHARED_H
void 		QDECL Com_Printf( const char *fmt, ... ) __attribute__ ((format (printf, 1, 2)));
void 		QDECL Com_Error( int code, const char *fmt, ... ) __attribute__ ((noreturn, format(printf, 2, 3)));
//#endif
void 		QDECL Com_DPrintf( const char *fmt, ... ) __attribute__ ((format (printf, 1, 2)));
void        Engine_Exit(const char* p ) __attribute__ ((noreturn));
void 		Com_Quit_f( void ) __attribute__ ((noreturn));
void		Com_GameRestart(int checksumFeed, bool disconnect);

int			Com_Milliseconds( void );	// will be journaled properly
char		*Com_MD5File(const char *filename, int length, const char *prefix, int prefix_len);
int			Com_Filter(const char* filter, char *name, int casesensitive);
int			Com_FilterPath(const char *filter, char *name, int casesensitive);
int			Com_RealTime(qtime_t *qtime);
bool	Com_SafeMode( void );
void		Com_RunAndTimeServerPacket(struct netadr_t *evFrom, struct msg_t *buf);

bool	Com_IsVoipTarget(uint8_t *voipTargets, int voipTargetsSize, int clientNum);

void		Com_StartupVariable( const char *match );
// checks for and removes command line "+set var arg" constructs
// if match is NULL, all set commands will be executed, otherwise
// only a set with the exact name.  Only used during startup.

bool		Com_PlayerNameToFieldString( char *str, int length, const char *name );
bool		Com_FieldStringToPlayerName( char *name, int length, const char *rawname );
int QDECL	Com_strCompare( const void *a, const void *b );


extern	cvar_t	*com_developer;
extern	cvar_t	*com_dedicated;
extern	cvar_t	*com_speeds;
extern	cvar_t	*com_timescale;
extern	cvar_t	*com_sv_running;
extern	cvar_t	*com_cl_running;
extern	cvar_t	*com_version;
extern	cvar_t	*com_buildScript;		// for building release pak files
extern	cvar_t	*com_journal;
extern	cvar_t	*com_cameraMode;
extern	cvar_t	*com_ansiColor;
extern	cvar_t	*com_unfocused;
extern	cvar_t	*com_maxfpsUnfocused;
extern	cvar_t	*com_minimized;
extern	cvar_t	*com_maxfpsMinimized;
extern	cvar_t	*com_altivec;
extern	cvar_t	*com_homepath;

// both client and server must agree to pause
extern	cvar_t	*cl_paused;
extern	cvar_t	*sv_paused;

extern	cvar_t	*cl_packetdelay;
extern	cvar_t	*sv_packetdelay;

extern	cvar_t	*com_gamename;

// com_speeds times
extern	int		time_game;
extern	int		time_frontend;
extern	int		time_backend;		// renderer backend time

extern	int		com_frameTime;

extern	bool	com_errorEntered;
extern	bool	com_fullyInitialized;

extern	fileHandle_t	com_journalFile;
extern	fileHandle_t	com_journalDataFile;

typedef enum {
	TAG_FREE,
	TAG_GENERAL,
	TAG_BOTLIB,
	TAG_RENDERER,
	TAG_SMALL,
	TAG_STATIC
} memtag_t;

/*

--- low memory ----
server vm
server clipmap
---mark---
renderer initialization (shaders, etc)
UI vm
cgame vm
renderer map
renderer models

---free---

temp file loading
--- high memory ---

*/

#if !defined(NDEBUG) && !defined(BSPC)
	#define ZONE_DEBUG
#endif

#ifdef ZONE_DEBUG
#define Z_TagMalloc(size, tag)			Z_TagMallocDebug(size, tag, #size, __FILE__, __LINE__)
#define Z_Malloc(size)					Z_MallocDebug(size, #size, __FILE__, __LINE__)
#define S_Malloc(size)					S_MallocDebug(size, #size, __FILE__, __LINE__)
void *Z_TagMallocDebug( int size, int tag, const char *label, const char *file, int line );	// NOT 0 filled memory
void *Z_MallocDebug( int size, const char *label, const char *file, int line );			// returns 0 filled memory
void *S_MallocDebug( int size, const char *label, const char *file, int line );			// returns 0 filled memory
#else
void *Z_TagMalloc( int size, int tag );	// NOT 0 filled memory
void *Z_Malloc( int size );			// returns 0 filled memory
void *S_Malloc( int size );			// NOT 0 filled memory only for small allocations
#endif
void Z_Free( void *ptr );
void Z_FreeTags( int tag );
int Z_AvailableMemory( void );
void Z_LogHeap( void );

void Hunk_Clear( void );
void Hunk_ClearToMark( void );
void Hunk_SetMark( void );
bool Hunk_CheckMark( void );
void Hunk_ClearTempMemory( void );
void *Hunk_AllocateTempMemory( int size );
void Hunk_FreeTempMemory( void *buf );
int	Hunk_MemoryRemaining( void );
void Hunk_Log( void);

void Com_TouchMemory( void );

// commandLine should not include the executable name (argv[0])
void Com_Init( char *commandLine );
void Com_Frame( void );
void Com_Shutdown( void );


/*
==============================================================

CLIENT / SERVER SYSTEMS

==============================================================
*/

//
// client interface
//
void CL_InitKeyCommands( void );
// the keyboard binding interface must be setup before execing
// config files, but the rest of client startup will happen later

void CL_Init( void );
void CL_Disconnect( bool showMainMenu );
void CL_Shutdown(const char *finalmsg, bool disconnect, bool quit);
void CL_Frame( int msec );
bool CL_GameCommand( void );
void CL_KeyEvent (int key, bool down, unsigned time);

void CL_CharEvent( int key );
// char events are for field typing, not game control

void CL_MouseEvent( int dx, int dy, int time );

void CL_JoystickEvent( int axis, int value, int time );

void CL_PacketEvent( struct netadr_t from, struct msg_t *msg );

void CL_ConsolePrint( const char *text );

void CL_MapLoading( void );
// do a screen update before starting to load a map
// when the server is going to load a new map, the entire hunk
// will be cleared, so the client must shutdown cgame, ui, and
// the renderer

void	CL_ForwardCommandToServer( const char *string );
// adds the current command line as a clc_clientCommand to the client message.
// things like godmode, noclip, etc, are commands directed to the server,
// so when they are typed in at the console, they will need to be forwarded.

void CL_FlushMemory( void );
// dump all memory on an error

void CL_ShutdownAll(bool shutdownRef);
// shutdown client

void CL_StartHunkUsers( bool rendererOnly );
// start all the client stuff using the hunk

void Key_KeynameCompletion( void(*callback)(const char *s) );
// for keyname autocompletion

void Key_WriteBindings( fileHandle_t f );
// for writing the config files

void S_ClearSoundBuffer( void );
// call before filesystem access

void SCR_DebugGraph (float value);	// FIXME: move logging to common?

//
// server interface
//
void SV_Init( void );
void SV_Shutdown( const char *finalmsg );
void SV_Frame( int msec );
void SV_PacketEvent( struct netadr_t from, struct msg_t *msg );
int SV_FrameMsec(void);
bool SV_GameCommand( void );
int SV_SendQueuedPackets(void);

//
// UI interface
//
bool UI_GameCommand( void );

/*
==============================================================

NON-PORTABLE SYSTEM SERVICES

==============================================================
*/

bool Parse_AddGlobalDefine(char *string);
int Parse_LoadSourceHandle(const char *filename);
bool Parse_FreeSourceHandle(int handle);
bool Parse_ReadTokenHandle(int handle, pc_token_t *pc_token);
bool Parse_SourceFileAndLine(int handle, char *filename, int *line);

// flags for sv_allowDownload and cl_allowDownload
#define DLF_ENABLE 1
#define DLF_NO_REDIRECT 2
#define DLF_NO_UDP 4
#define DLF_NO_DISCONNECT 8

/*
==============================================================

Bucket Selection System

==============================================================
*/

unsigned int Com_Bucket_Create_Bucket(void);
void         Com_Bucket_Delete_Bucket(unsigned int bucket_handle);
void         Com_Bucket_Destroy_All_Buckets(void);
void         Com_Bucket_Add_Item_To_Bucket(
	unsigned int bucket_handle, void* item);
void         Com_Bucket_Remove_Item_From_Bucket(
	unsigned int bucket_handle, void* item);
void*        Com_Bucket_Select_A_Random_Item(unsigned int bucket_handle);
void         Com_Bucket_Select_A_Specific_Item(
	unsigned int bucket_handle, void* item);

#endif // _QCOMMON_H_
