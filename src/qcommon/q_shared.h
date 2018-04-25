/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2013 Darklegion Development
Copyright (C) 2015-2018 GrangerHub

This file is part of Tremulous.

Tremulous is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Tremulous is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tremulous; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
#ifndef __Q_SHARED_H
#define __Q_SHARED_H

// q_shared.h -- included first by ALL program modules.
// A user mod should never modify this file

#define PRODUCT_NAME              "tremulous"

#ifndef PRODUCT_VERSION
# define PRODUCT_VERSION          "1.3.0 alpha"
#endif

#define CLIENT_WINDOW_TITLE       "Tremulous " PRODUCT_VERSION
#define CLIENT_WINDOW_MIN_TITLE   "Tremulous"
#define Q3_VERSION                 PRODUCT_NAME " " PRODUCT_VERSION

#define GAMENAME_FOR_MASTER       "Tremulous"
#define HOMEPATH_NAME_UNIX        ".tremulous"
#define HOMEPATH_NAME_WIN         "Tremulous"
#define HOMEPATH_NAME_MACOSX      HOMEPATH_NAME_WIN

// Heartbeat for dpmaster protocol. You shouldn't change this unless you know what you're doing
#define HEARTBEAT_FOR_MASTER      GAMENAME_FOR_MASTER

#define MAX_MASTER_SERVERS      5 // number of supported master servers

#define DEMOEXT	"dm_"			// standard demo extension

#ifdef _MSC_VER

#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4032)
#pragma warning(disable : 4051)
#pragma warning(disable : 4057)		// slightly different base types
#pragma warning(disable : 4100)		// unreferenced formal parameter
#pragma warning(disable : 4115)
#pragma warning(disable : 4125)		// decimal digit terminates octal escape sequence
#pragma warning(disable : 4127)		// conditional expression is constant
#pragma warning(disable : 4136)
#pragma warning(disable : 4152)		// nonstandard extension, function/data pointer conversion in expression
//#pragma warning(disable : 4201)
//#pragma warning(disable : 4214)
#pragma warning(disable : 4244)
#pragma warning(disable : 4142)		// benign redefinition
//#pragma warning(disable : 4305)		// truncation from const double to float
//#pragma warning(disable : 4310)		// cast truncates constant value
//#pragma warning(disable:  4505) 	// unreferenced local function has been removed
#pragma warning(disable : 4514)
#pragma warning(disable : 4702)		// unreachable code
#pragma warning(disable : 4711)		// selected for automatic inline expansion
#pragma warning(disable : 4220)		// varargs matches remaining parameters
//#pragma intrinsic( memset, memcpy )
#endif

//Ignore __attribute__ on non-gcc platforms
#ifndef __GNUC__
#ifndef __attribute__
#define __attribute__(x)
#endif
#endif

#ifdef __GNUC__
#define UNUSED_VAR __attribute__((unused))
#else
#define UNUSED_VAR
#endif

/**********************************************************************
  VM Considerations

  The VM can not use the standard system headers because we aren't really
  using the compiler they were meant for.  We use bg_lib.h which contains
  prototypes for the functions we define for our own use in bg_lib.c.

  When writing mods, please add needed headers HERE, do not start including
  stuff like <stdio.h> in the various .c files that make up each of the VMs
  since you will be including system headers files can will have issues.

  Remember, if you use a C library function that is not defined in bg_lib.c,
  you will have to add your own version for support in the VM.

 **********************************************************************/

#ifdef Q3_VM

#include "game/bg_lib.h"

typedef int intptr_t;

#else

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>

#ifdef _MSC_VER
  #include <io.h>

  typedef __int64 int64_t;
  typedef __int32 int32_t;
  typedef __int16 int16_t;
  typedef __int8 int8_t;
  typedef unsigned __int64 uint64_t;
  typedef unsigned __int32 uint32_t;
  typedef unsigned __int16 uint16_t;
  typedef unsigned __int8 uint8_t;

  // vsnprintf is ISO/IEC 9899:1999
  // abstracting this to make it portable
  int Q_vsnprintf(char *str, size_t size, const char *format, va_list ap);
#else
  #include <stdint.h>

  #define Q_vsnprintf vsnprintf
  #define Q_snprintf snprintf
#endif

#endif

#include "mathlib.h"

#ifdef __cplusplus
extern "C" {
#endif


#include "qcommon/q_platform.h"

//=============================================================

typedef unsigned char byte;

typedef union {
	float f;
	int i;
	unsigned int ui;
} floatint_t;

typedef int		qhandle_t;
typedef int		sfxHandle_t;
typedef int		fileHandle_t;
typedef int		clipHandle_t;

#define PAD(base, alignment)	(((base)+(alignment)-1) & ~((alignment)-1))
#define PADLEN(base, alignment)	(PAD((base), (alignment)) - (base))

#define PADP(base, alignment)	((void *) PAD((intptr_t) (base), (alignment)))

#ifdef __GNUC__
#define QALIGN(x) __attribute__((aligned(x)))
#else
#define QALIGN(x)
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#define STRING(s)			#s
// expand constants before stringifying them
#define XSTRING(s)			STRING(s)

#define	MAX_QINT			0x7fffffff
#define	MIN_QINT			(-MAX_QINT-1)

#define ARRAY_LEN(x)			(sizeof(x) / sizeof(*(x)))
#define STRARRAY_LEN(x)			(ARRAY_LEN(x) - 1)

// angle indexes
#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

/* CVAR */

#define CVAR_ARCHIVE        0x0001  // set to cause it to be saved to vars.rc
// used for system variables, not for player
// specific configurations
#define CVAR_USERINFO       0x0002  // sent to server on connect or change
#define CVAR_SERVERINFO     0x0004  // sent in response to front end requests
#define CVAR_SYSTEMINFO     0x0008  // these cvars will be duplicated on all clients
#define CVAR_INIT           0x0010  // don't allow change from console at all,
// but can be set from the command line
#define CVAR_LATCH          0x0020  // will only change when C code next does
// a Cvar_Get(), so it can't be changed without proper initialization.
// modified will be set, even though the value hasn't changed yet
#define CVAR_ROM            0x0040  // display only, cannot be set by user at all
#define CVAR_USER_CREATED   0x0080  // created by a set command
#define CVAR_TEMP           0x0100  // can be set even when cheats are disabled, but is not archived
#define CVAR_CHEAT          0x0200  // can not be changed if cheats are disabled
#define CVAR_NORESTART      0x0400  // do not clear when a cvar_restart is issued

#define CVAR_SERVER_CREATED 0x0800  // cvar was created by a server the client connected to.
#define CVAR_VM_CREATED     0x1000  // cvar was created exclusively in one of the VMs.
#define CVAR_PROTECTED      0x2000  // prevent modifying this var from VMs or the server
#define CVAR_ALTERNATE_SYSTEMINFO 0x1000000
// These flags are only returned by the Cvar_Flags() function
#define CVAR_MODIFIED             0x40000000  // Cvar was modified
#define CVAR_NONEXISTENT          0x80000000  // Cvar doesn't exist.

#define MAX_CVAR_VALUE_STRING 256

typedef int cvarHandle_t;

// the modules that run in the virtual machine can't access the cvar_t directly,
// so they must ask for structured updates
typedef struct {
    cvarHandle_t handle;
    int modificationCount;
    float value;
    int integer;
    char string[MAX_CVAR_VALUE_STRING];
} vmCvar_t;


// the game guarantees that no string from the network will ever
// exceed MAX_STRING_CHARS
#define	MAX_STRING_CHARS	1024	// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS	1024	// max tokens resulting from Cmd_TokenizeString
#define	MAX_TOKEN_CHARS		1024	// max length of an individual token

#define	MAX_INFO_STRING		1024
#define	MAX_INFO_KEY		  1024
#define	MAX_INFO_VALUE		1024

#define	BIG_INFO_STRING		8192  // used for system info key only
#define	BIG_INFO_KEY		  8192
#define	BIG_INFO_VALUE		8192

#define	MAX_NEWS_STRING		10000

#define	MAX_QPATH			64		// max length of a quake game pathname
#ifdef PATH_MAX
#define MAX_OSPATH			PATH_MAX
#else
#define	MAX_OSPATH			256		// max length of a filesystem pathname
#endif

#define	MAX_NAME_LENGTH			32		// max length of a client name
#define	MAX_HOSTNAME_LENGTH	80		// max length of a host name

#define	MAX_SAY_TEXT	800

//
// these aren't needed by any of the VMs.  put in another header?
//
#define	MAX_MAP_AREA_BYTES		32		// bit vector of area visibility


// print levels from renderer (FIXME: set up for game / cgame?)
typedef enum {
	PRINT_ALL,
	PRINT_DEVELOPER,		// only print when "developer 1"
	PRINT_WARNING,
	PRINT_ERROR
} printParm_t;


#ifdef ERR_FATAL
#undef ERR_FATAL			// this is be defined in malloc.h
#endif

// parameters to the main Error routine
typedef enum {
    ERR_FATAL,					// exit the entire game with a popup window
    ERR_DROP,					// print to console and disconnect from game
    ERR_SERVERDISCONNECT,		// don't kill server
    ERR_DISCONNECT,				// client disconnected from the server
    ERR_RECONNECT
} errorParm_t;


// font rendering values used by ui and cgame
//
#define BLINK_DIVISOR			200
#define PULSE_DIVISOR			75

#if !defined(NDEBUG) && !defined(BSPC)
	#define HUNK_DEBUG
#endif

typedef enum {
	h_high,
	h_low,
	h_dontcare
} ha_pref;

#ifdef HUNK_DEBUG
#define Hunk_Alloc( size, preference )				Hunk_AllocDebug(size, preference, #size, __FILE__, __LINE__)
void *Hunk_AllocDebug( int size, ha_pref preference, const char *label, const char *file, int line );
#else
void *Hunk_Alloc( int size, ha_pref preference );
#endif

#define Com_Memset memset
#define Com_Memcpy memcpy

#define CIN_system	1
#define CIN_loop	2
#define	CIN_hold	4
#define CIN_silent	8
#define CIN_shader	16

float Com_Clamp( float min, float max, float value );

char	*COM_SkipPath( char *pathname );
const char	*COM_GetExtension( const char *name );
void	COM_StripExtension(const char *in, char *out, int destsize);
bool COM_CompareExtension(const char *in, const char *ext);
void	COM_DefaultExtension( char *path, int maxSize, const char *extension );

void	COM_BeginParseSession( const char *name );
int		COM_GetCurrentParseLine( void );
char	*COM_Parse( char **data_p );
char	*COM_ParseExt( char **data_p, bool allowLineBreak );
int		COM_Compress( char *data_p );
void	COM_ParseError( const char *format, ... ) __attribute__ ((format (printf, 1, 2)));
void	COM_ParseWarning( const char *format, ... ) __attribute__ ((format (printf, 1, 2)));

// data is an in/out parm, returns a parsed out token

void	COM_MatchToken( char**buf_p, char *match );

bool SkipBracedSection (char **program, int depth);
void SkipRestOfLine ( char **data );

void Parse1DMatrix (char **buf_p, int x, float *m);
void Parse2DMatrix (char **buf_p, int y, int x, float *m);
void Parse3DMatrix (char **buf_p, int z, int y, int x, float *m);
int Com_HexStrToInt( const char *str );

int QDECL Com_sprintf (char *dest, int size, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));

char *Com_SkipTokens( char *s, int numTokens, const char *sep );
char *Com_SkipCharset( char *s, const char *sep );

void Com_RandomBytes( byte *string, int len );

typedef struct
{
  unsigned int hi;
  unsigned int lo;
} clientList_t;

bool Com_ClientListContains( const clientList_t *list, int clientNum );
void Com_ClientListAdd( clientList_t *list, int clientNum );
void Com_ClientListRemove( clientList_t *list, int clientNum );
char *Com_ClientListString( const clientList_t *list );
void Com_ClientListParse( clientList_t *list, const char *s );

//=============================================

int Q_isprint( int c );
int Q_islower( int c );
int Q_isupper( int c );
int Q_isalpha( int c );
bool Q_isanumber( const char *s );
bool Q_isintegral( float f );

// portable case insensitive compare
int		Q_stricmp (const char *s1, const char *s2);
int		Q_strncmp (const char *s1, const char *s2, int n);
int		Q_stricmpn (const char *s1, const char *s2, int n);
char	*Q_strlwr( char *s1 );
char	*Q_strupr( char *s1 );
const char	*Q_stristr( const char *s, const char *find);

// buffer size safe library replacements
void	Q_strncpyz( char *dest, const char *src, int destsize );
void	Q_strcat( char *dest, int size, const char *src );

// strlen that discounts Quake color sequences
int Q_PrintStrlen( const char *string );
// removes color sequences from string
char *Q_CleanStr( char *string );
// parse "\n" into '\n'
void Q_ParseNewlines( char *dest, const char *src, int destsize );
// Count the number of char tocount encountered in string
int Q_CountChar(const char *string, char tocount);

//=============================================

const char	* QDECL va(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

#define TRUNCATE_LENGTH	64
void Com_TruncateLongString( char *buffer, const char *s );

//=============================================

//
// key / value info strings
//
const char *Info_ValueForKey( const char *s, const char *key );
void Info_RemoveKey( char *s, const char *key );
void Info_RemoveKey_Big( char *s, const char *key );
void Info_SetValueForKey( char *s, const char *key, const char *value );
void Info_SetValueForKey_Big( char *s, const char *key, const char *value );
bool Info_Validate( const char *s );
void Info_NextPair( const char **s, char *key, char *value );

// this is only here so the functions in q_shared.c and bg_*.c can link
SO_PUBLIC void QDECL Com_Error( int level, const char *error, ... ) __attribute__ ((noreturn, format(printf, 2, 3)));
SO_PUBLIC void QDECL Com_Printf( const char *msg, ... ) __attribute__ ((format (printf, 1, 2)));


/*
==============================================================

VoIP

==============================================================
*/

// if you change the count of flags be sure to also change VOIP_FLAGNUM
#define VOIP_SPATIAL		0x01		// spatialized voip message
#define VOIP_DIRECT		0x02		// non-spatialized voip message

// number of flags voip knows. You will have to bump protocol version number if you
// change this.
#define VOIP_FLAGCNT		2

/*
==============================================================

COLLISION DETECTION

==============================================================
*/

#include "qcommon/surfaceflags.h"			// shared with the q3map utility

// plane types are used to speed some tests
// 0-2 are axial planes
#define	PLANE_X			0
#define	PLANE_Y			1
#define	PLANE_Z			2
#define	PLANE_NON_AXIAL	3


/*
=================
PlaneTypeForNormal
=================
*/

#define PlaneTypeForNormal(x) (x[0] == 1.0 ? PLANE_X : (x[1] == 1.0 ? PLANE_Y : (x[2] == 1.0 ? PLANE_Z : PLANE_NON_AXIAL) ) )

// plane_t structure
// !!! if this is changed, it must be changed in asm code too !!!
typedef struct cplane_s {
	vec3_t	normal;
	float	dist;
	byte	type;			// for fast side tests: 0,1,2 = axial, 3 = nonaxial
	byte	signbits;		// signx + (signy<<1) + (signz<<2), used as lookup during collision
	byte	pad[2];
} cplane_t;

typedef enum {
	TT_NONE,

	TT_AABB,
	TT_CAPSULE,
	TT_BISPHERE,

	TT_NUM_TRACE_TYPES
} traceType_t;

// a trace is returned when a box is swept through the world
typedef struct {
	bool	allsolid;	// if true, plane is not valid
// FIXME: startsolid is supposed to be bool
	int /*bool*/	startsolid;	// if true, the initial point was in a solid area
	float		fraction;	// time completed, 1.0 = didn't hit anything
	vec3_t		endpos;		// final position
	cplane_t	plane;		// surface normal at impact, transformed to world space
	int			surfaceFlags;	// surface hit
	int			contents;	// contents on other side of surface hit
	int			entityNum;	// entity the contacted sirface is a part of
	float		lateralFraction; // fraction of collision tangetially to the trace direction
} trace_t;

// trace->entityNum can also be 0 to (MAX_GENTITIES-1)
// or ENTITYNUM_NONE, ENTITYNUM_WORLD

// markfragments are returned by R_MarkFragments()
typedef struct {
	int		firstPoint;
	int		numPoints;
} markFragment_t;

typedef struct {
	vec3_t		origin;
	vec3_t		axis[3];
} orientation_t;

//=====================================================================


// in order from highest priority to lowest
// if none of the catchers are active, bound key strings will be executed
#define KEYCATCH_CONSOLE		0x0001
#define	KEYCATCH_UI					0x0002
#define KEYCATCH_MESSAGE    0x0004
#define	KEYCATCH_CGAME			0x0008


// sound channels
// channel 0 never willingly overrides
// other channels will allways override a playing sound on that channel
typedef enum {
	CHAN_AUTO,
	CHAN_LOCAL,		// menu sounds, etc
	CHAN_WEAPON,
	CHAN_VOICE,
	CHAN_ITEM,
	CHAN_BODY,
	CHAN_LOCAL_SOUND,	// chat messages, etc
	CHAN_ANNOUNCER		// announcer voices, etc
} soundChannel_t;


/*
========================================================================

  ELEMENTS COMMUNICATED ACROSS THE NET

========================================================================
*/

#define	ANGLE2SHORT(x)	((int)((x)*65536/360) & 65535)
#define	SHORT2ANGLE(x)	((x)*(360.0/65536))

#define	SNAPFLAG_RATE_DELAYED	1
#define	SNAPFLAG_NOT_ACTIVE		2	// snapshot used during connection and for zombies
#define SNAPFLAG_SERVERCOUNT	4	// toggled every map_restart so transitions can be detected

//
// per-level limits
//
#define	MAX_CLIENTS			64		// absolute limit
#define MAX_LOCATIONS		64

#define	GENTITYNUM_BITS		10		// don't need to send any more
#define	MAX_GENTITIES		(1<<GENTITYNUM_BITS)
#define GENTITYNUM_MASK		(MAX_GENTITIES - 1)

// entitynums are communicated with GENTITY_BITS, so any reserved
// values that are going to be communcated over the net need to
// also be in this range
#define	ENTITYNUM_NONE		(MAX_GENTITIES-1)
#define	ENTITYNUM_WORLD		(MAX_GENTITIES-2)
#define	ENTITYNUM_MAX_NORMAL	(MAX_GENTITIES-2)


#define	MAX_MODELS									256		// these are sent over the net as 8 bits
#define	MAX_SOUNDS									256		// so they cannot be blindly increased
#define	MAX_GAME_SHADERS						64
#define	MAX_GAME_PARTICLE_SYSTEMS		64


#define	MAX_CONFIGSTRINGS	1024

// these are the only configstrings that the system reserves, all the
// other ones are strictly for servergame to clientgame communication
#define	CS_SERVERINFO		0		// an info string with all the serverinfo cvars
#define	CS_SYSTEMINFO		1		// an info string for server system to client system configuration (timescale, etc)

#define	RESERVED_CONFIGSTRINGS	2	// game can't modify below this, only the system can

#define	MAX_GAMESTATE_CHARS	16000
struct gameState_t {
	int			stringOffsets[MAX_CONFIGSTRINGS];
	char		stringData[MAX_GAMESTATE_CHARS];
	int			dataCount;
};

//=========================================================

// bit field limits
#define	MAX_STATS				16
#define	MAX_PERSISTANT			16
#define	MAX_MISC    			16
#define	MAX_WEAPONS				16

#define	MAX_PS_EVENTS			2

#define PS_PMOVEFRAMECOUNTBITS	6

// playerState_t is the information needed by both the client and server
// to predict player motion and actions
// nothing outside of pmove should modify these, or some degree of prediction error
// will occur

// you can't add anything to this without modifying the code in msg.c

// playerState_t is a full superset of entityState_t as it is used by players,
// so if a playerState_t is transmitted, the entityState_t can be fully derived
// from it.
struct playerState_t {
	int			commandTime;	// cmd->serverTime of last executed command
	int			pm_type;
	int			bobCycle;		// for view bobbing and footstep generation
	int			pm_flags;		// ducked, jump_held, etc
	int			pm_time;

	vec3_t		origin;
	vec3_t		velocity;
	int			weaponTime;
	int			gravity;
	int			speed;
	int			delta_angles[3];	// add to command angles to get view direction
									// changed by spawns, rotating objects, and teleporters

	int			groundEntityNum;// ENTITYNUM_NONE = in air

	int			legsTimer;		// don't change low priority animations until this runs out
	int			legsAnim;		// mask off ANIM_TOGGLEBIT

	int			torsoTimer;		// don't change low priority animations until this runs out
	int			torsoAnim;		// mask off ANIM_TOGGLEBIT

	int			tauntTimer;		// don't allow another taunt until this runs out

	int			weaponAnim;		// mask off ANIM_TOGGLEBIT

	int			movementDir;	// a number 0 to 7 that represents the relative angle
								// of movement to the view angle (axial and diagonals)
								// when at rest, the value will remain unchanged
								// used to twist the legs during strafing

	vec3_t		grapplePoint;	// location of grapple to pull towards if PMF_GRAPPLE_PULL

	int			eFlags;			// copied to entityState_t->eFlags

	int			eventSequence;	// pmove generated events
	int			events[MAX_PS_EVENTS];
	int			eventParms[MAX_PS_EVENTS];

	int			externalEvent;	// events set on player from another source
	int			externalEventParm;
	int			externalEventTime;

	int			clientNum;		// ranges from 0 to MAX_CLIENTS-1
	int			weapon;			// copied to entityState_t->weapon
	int			weaponstate;

	vec3_t		viewangles;		// for fixed views
	int			viewheight;

	// damage feedback
	int			damageEvent;	// when it changes, latch the other parms
	int			damageYaw;
	int			damagePitch;
	int			damageCount;

	int			stats[MAX_STATS];
	int			persistant[MAX_PERSISTANT];	// stats that aren't cleared on death
	int			misc[MAX_MISC];	// misc data
	int			ammo;			// ammo held
	int			clips;			// clips held

	int			generic1;
	int			loopSound;
	int			otherEntityNum;

	// not communicated over the net at all
	int			ping;			// server to game info for scoreboard
	int			pmove_framecount;
	int			jumppad_frame;
	int			entityEventSequence;
};


//====================================================================


//
// usercmd_t->button bits, many of which are generated by the client system,
// so they aren't game/cgame only definitions
//
#define	BUTTON_ATTACK		1
#define	BUTTON_TALK			2			// displays talk balloon and disables actions
#define BUTTON_USE_HOLDABLE 4           // activate upgrade
#define	BUTTON_GESTURE		8
#define	BUTTON_WALKING		16			// walking can't just be infered from MOVE_RUN
										// because a key pressed late in the frame will
										// only generate a small move value for that frame
										// walking will use different animations and
										// won't generate footsteps
#define BUTTON_ATTACK2	32
#define BUTTON_DODGE        64          // start a dodge or sprint motion
#define BUTTON_USE_EVOLVE   128         // use target or open evolve menu
#define BUTTON_SPRINT	256

#define	BUTTON_ANY			2048			// any key whatsoever

#define	MOVE_RUN			120			// if forwardmove or rightmove are >= MOVE_RUN,
										// then BUTTON_WALKING should be set

// usercmd_t is sent to the server each client frame
struct usercmd_t {
	int				serverTime;
	int				angles[3];
	int 			buttons;
	byte			weapon;           // weapon
	signed char	forwardmove, rightmove, upmove;
};

//===================================================================

// if entityState->solid == SOLID_BMODEL, modelindex is an inline model number
#define	SOLID_BMODEL	0xffffff

typedef enum {
	TR_STATIONARY,
	TR_INTERPOLATE,				// non-parametric, but interpolate between snapshots
	TR_LINEAR,
	TR_LINEAR_STOP,
	TR_SINE,					// value = base + sin( time / duration ) * delta
	TR_GRAVITY,
	TR_BUOYANCY
} trType_t;

typedef struct {
	trType_t	trType;
	int		trTime;
	int		trDuration;			// if non 0, trTime + trDuration = stop time
	vec3_t	trBase;
	vec3_t	trDelta;			// velocity, etc
} trajectory_t;

// entityState_t is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way
// Different eTypes may use the information in different ways
// The messages are delta compressed, so it doesn't really matter if
// the structure size is fairly large

struct entityState_t {
	int		number;			// entity index
	int		eType;			// entityType_t
	int		eFlags;

	trajectory_t	pos;	// for calculating position
	trajectory_t	apos;	// for calculating angles

	int		time;
	int		time2;

	vec3_t	origin;
	vec3_t	origin2;

	vec3_t	angles;
	vec3_t	angles2;

	int		otherEntityNum;	// shotgun sources, etc
	int		otherEntityNum2;

	int		groundEntityNum;	// ENTITYNUM_NONE = in air

	int		constantLight;	// r + (g<<8) + (b<<16) + (intensity<<24)
	int		loopSound;		// constantly loop this sound

	int		modelindex;
	int		modelindex2;
	int		clientNum;		// 0 to (MAX_CLIENTS - 1), for players and corpses
	int		frame;

	int		solid;			// for client side prediction, trap_linkentity sets this properly

	int		event;			// impulse events -- muzzle flashes, footsteps, etc
	int		eventParm;

	// for players
	int		misc;			// bit flags
	int		weapon;			// determines weapon and flash model, etc
	int		legsAnim;		// mask off ANIM_TOGGLEBIT
	int		torsoAnim;		// mask off ANIM_TOGGLEBIT
	int		weaponAnim;		// mask off ANIM_TOGGLEBIT

	int		generic1;
};

typedef enum {
	CA_UNINITIALIZED,
	CA_DISCONNECTED, 	// not talking to a server
	CA_AUTHORIZING,		// not used any more, was checking cd key
	CA_CONNECTING,		// sending request packets to the server
	CA_CHALLENGING,		// sending challenge packets to the server
	CA_CONNECTED,		// netchan_t established, getting gamestate
	CA_LOADING,			// only during cgame initialization, never during main loop
	CA_PRIMED,			// got gamestate, waiting for first frame
	CA_ACTIVE,			// game views should be displayed
	CA_CINEMATIC		// playing a cinematic or a static pic, not connected to a server
} connstate_t;

// font support

#define GLYPH_START 0
#define GLYPH_END 255
#define GLYPH_CHARSTART 32
#define GLYPH_CHAREND 127
#define GLYPHS_PER_FONT (GLYPH_END - GLYPH_START + 1)

typedef struct {
  int height;       // number of scan lines
  int top;          // top of glyph in buffer
  int bottom;       // bottom of glyph in buffer
  int pitch;        // width for copying
  int xSkip;        // x adjustment
  int imageWidth;   // width of actual image
  int imageHeight;  // height of actual image
  float s;          // x offset in image where glyph starts
  float t;          // y offset in image where glyph starts
  float s2;
  float t2;
  qhandle_t glyph;  // handle to the shader with the glyph
  char shaderName[32];
} glyphInfo_t;

typedef struct {
  glyphInfo_t glyphs [GLYPHS_PER_FONT];
  float glyphScale;
  char name[MAX_QPATH];
} fontInfo_t;

#define Square(x) ((x)*(x))

// real time
//=============================================


typedef struct qtime_s {
	int tm_sec;     /* seconds after the minute - [0,59] */
	int tm_min;     /* minutes after the hour - [0,59] */
	int tm_hour;    /* hours since midnight - [0,23] */
	int tm_mday;    /* day of the month - [1,31] */
	int tm_mon;     /* months since January - [0,11] */
	int tm_year;    /* years since 1900 */
	int tm_wday;    /* days since Sunday - [0,6] */
	int tm_yday;    /* days since January 1 - [0,365] */
	int tm_isdst;   /* daylight savings time flag */
} qtime_t;


// server browser sources
// AS_MPLAYER is no longer used
#define AS_GLOBAL           0
#define AS_MPLAYER          1
#define AS_LOCAL            2
#define AS_FAVORITES        3

enum demoState_t {
	DS_NONE,
	DS_PLAYBACK,
	DS_RECORDING,
	DS_NUM_DEMO_STATES
};


#define	MAX_GLOBAL_SERVERS				4096
#define	MAX_OTHER_SERVERS					128
#define MAX_PINGREQUESTS					32
#define MAX_SERVERSTATUSREQUESTS	16

#define MAX_EMOTICON_NAME_LEN 16
#define MAX_EMOTICONS 64

typedef struct
{
  char      name[ MAX_EMOTICON_NAME_LEN ];
#ifndef GAME
  int       width;
  qhandle_t shader;
#endif
} emoticon_t;

// flags for com_downloadPrompt
#define DLP_TYPE_MASK 0x0f
#define DLP_IGNORE    0x01 // don't download anything
#define DLP_CURL      0x02 // download via HTTP redirect
#define DLP_UDP       0x04 // download from server
#define DLP_SHOW      0x10 // prompt needs to be shown
#define DLP_PROMPTED  0x20 // prompt has been processed by client
#define DLP_STALE     0x40 // prompt is not being shown by UI VM

#define LERP( a, b, w ) ( ( a ) * ( 1.0f - ( w ) ) + ( b ) * ( w ) )
#define LUMA( red, green, blue ) ( 0.2126f * ( red ) + 0.7152f * ( green ) + 0.0722f * ( blue ) )

#ifdef __cplusplus
};
#endif
#endif	// __Q_SHARED_H
