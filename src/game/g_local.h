/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2013 Darklegion Development

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

// g_local.h -- local definitions for game module

#include "../qcommon/q_shared.h"
#include "bg_public.h"
#include "g_public.h"

typedef struct gentity_s gentity_t;
typedef struct gclient_s gclient_t;

#include "g_admin.h"

//==================================================================

#define INFINITE      1000000

#define FRAMETIME     100         // msec

#define INTERMISSION_DELAY_TIME 1000

// gentity->flags
#define FL_GODMODE        0x00000010
#define FL_NOTARGET       0x00000020
#define FL_TEAMSLAVE      0x00000400  // not the first on the team
#define FL_NO_KNOCKBACK   0x00000800
#define FL_DROPPED_ITEM   0x00001000
#define FL_NO_BOTS        0x00002000  // spawn point not for bot use
#define FL_NO_HUMANS      0x00004000  // spawn point just for bots
#define FL_FORCE_GESTURE  0x00008000  // spawn point just for bots

// movers are things like doors, plats, buttons, etc
typedef enum
{
  MOVER_POS1,
  MOVER_POS2,
  MOVER_1TO2,
  MOVER_2TO1,

  ROTATOR_POS1,
  ROTATOR_POS2,
  ROTATOR_1TO2,
  ROTATOR_2TO1,

  MODEL_POS1,
  MODEL_POS2,
  MODEL_1TO2,
  MODEL_2TO1
} moverState_t;

#define SP_PODIUM_MODEL   "models/mapobjects/podium/podium4.md3"

typedef struct gitem_s
{
  int  ammo; // ammo held
  int  clips; // clips held
} gitem_t;

//============================================================================

struct gentity_s
{
  entityState_t     s;        // communicated by server to clients
  entityShared_t    r;        // shared by both the server system and game

  // DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
  // EXPECTS THE FIELDS IN THAT ORDER!
  //================================

  struct gclient_s  *client;        // NULL if not a client
  gitem_t    item;

  qboolean          inuse;

  char              *classname;     // set in QuakeEd
  int               spawnflags;     // set in QuakeEd

  qboolean          neverFree;      // if true, FreeEntity will only unlink
                                    // bodyque uses this

  int               flags;          // FL_* variables

  char              *model;
  char              *model2;
  int               freetime;       // level.time when the object was freed

  int               eventTime;      // events will be cleared EVENT_VALID_MSEC after set
  qboolean          freeAfterEvent;
  qboolean          unlinkAfterEvent;

  qboolean          physicsObject;  // if true, it can be pushed by movers and fall off edges
                                    // all game items are physicsObjects,
  float             physicsBounce;  // 1.0 = continuous bounce, 0.0 = no bounce
  int               clipmask;       // brushes with this content value will be collided against
                                    // when moving.  items and corpses do not collide against
                                    // players, for instance

  // movers
  moverState_t      moverState;
  int               soundPos1;
  int               sound1to2;
  int               sound2to1;
  int               soundPos2;
  int               soundLoop;
  gentity_t         *parent;
  gentity_t         *nextTrain;
  vec3_t            pos1, pos2;
  float             rotatorAngle;
  gentity_t         *clipBrush;     // clipping brush for model doors

  char              *message;

  int               timestamp;      // body queue sinking, etc

  char              *target;
  char              *targetname;
  char              *team;
  char              *targetShaderName;
  char              *targetShaderNewName;
  gentity_t         *target_ent;

  float             speed;
  float             lastSpeed;      // used by trains that have been restarted
  vec3_t            movedir;

  // acceleration evaluation
  qboolean          evaluateAcceleration;
  vec3_t            oldVelocity;
  vec3_t            acceleration;
  vec3_t            oldAccel;
  vec3_t            jerk;

  int               nextthink;
  void              (*think)( gentity_t *self );
  void              (*reached)( gentity_t *self );  // movers call this when hitting endpoint
  void              (*blocked)( gentity_t *self, gentity_t *other );
  void              (*touch)( gentity_t *self, gentity_t *other, trace_t *trace );
  void              (*use)( gentity_t *self, gentity_t *other, gentity_t *activator );
  void              (*pain)( gentity_t *self, gentity_t *attacker, int damage );
  void              (*die)( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod );

  int               pain_debounce_time;
  int               last_move_time;

  int               health;
  int               lastHealth; // currently only used for overmind

  qboolean          takedamage;

  int               damage;
  int               splashDamage; // quad will increase this without increasing radius
  int               splashRadius;
  int               methodOfDeath;
  int               splashMethodOfDeath;

  int               count;

  gentity_t         *chain;
  gentity_t         *enemy;
  gentity_t         *activator;
  gentity_t         *teamchain;   // next entity in team
  gentity_t         *teammaster;  // master of the team

  int               watertype;
  int               waterlevel;

  int               noise_index;

  // timing variables
  float             wait;
  float             random;

  team_t            stageTeam;
  stage_t           stageStage;

  team_t            buildableTeam;      // buildable item team
  gentity_t         *parentNode;        // for creep and defence/spawn dependencies
  gentity_t         *rangeMarker;
  qboolean          active;             // for power repeater, but could be useful elsewhere
  qboolean          powered;            // for human buildables
  struct namelog_s  *builtBy;           // person who built this
  int               dcc;                // number of controlling dccs
  qboolean          spawned;            // whether or not this buildable has finished spawning
  int               shrunkTime;         // time when a barricade shrunk or zero
  int               buildTime;          // when this buildable was built
  int               animTime;           // last animation change
  int               time1000;           // timer evaluated every second
  qboolean          deconstruct;        // deconstruct if no BP left
  int               deconstructTime;    // time at which structure marked
  int               overmindAttackTimer;
  int               overmindDyingTimer;
  int               overmindSpawnsTimer;
  int               nextPhysicsTime;    // buildables don't need to check what they're sitting on
                                        // every single frame.. so only do it periodically
  int               clientSpawnTime;    // the time until this spawn can spawn a client

  int               credits[ MAX_CLIENTS ];     // human credits for each client
  int               killedBy;                   // clientNum of killer

  vec3_t            turretAim;          // aim vector for turrets
  vec3_t            turretAimRate;      // track turn speed for norfenturrets
  int               turretSpinupTime;   // spinup delay for norfenturrets

  vec4_t            animation;          // animated map objects

  qboolean          nonSegModel;        // this entity uses a nonsegmented player model

  buildable_t       bTriggers[ BA_NUM_BUILDABLES ]; // which buildables are triggers
  class_t           cTriggers[ PCL_NUM_CLASSES ];   // which classes are triggers
  weapon_t          wTriggers[ WP_NUM_WEAPONS ];    // which weapons are triggers
  upgrade_t         uTriggers[ UP_NUM_UPGRADES ];   // which upgrades are triggers

  int               triggerGravity;                 // gravity for this trigger

  int               suicideTime;                    // when the client will suicide

  int               lastDamageTime;
  int               nextRegenTime;

  qboolean          pointAgainstWorld;              // don't use the bbox for map collisions

  int               buildPointZone;                 // index for zone
  int               usesBuildPointZone;             // does it use a zone?
};

typedef enum
{
  CON_DISCONNECTED,
  CON_CONNECTING,
  CON_CONNECTED
} clientConnected_t;

// client data that stays across multiple levels or tournament restarts
// this is achieved by writing all the data to cvar strings at game shutdown
// time and reading them back at connection time.  Anything added here
// MUST be dealt with in G_InitSessionData() / G_ReadSessionData() / G_WriteSessionData()
typedef struct
{
  int               spectatorTime;    // for determining next-in-line to play
  spectatorState_t  spectatorState;
  int               spectatorClient;  // for chasecam and follow mode
  team_t            restartTeam; //for !restart keepteams and !restart switchteams
  clientList_t      ignoreList;
} clientSession_t;

// namelog
#define MAX_NAMELOG_NAMES 5
#define MAX_NAMELOG_ADDRS 5
typedef struct namelog_s
{
  struct namelog_s  *next;
  char              name[ MAX_NAMELOG_NAMES ][ MAX_NAME_LENGTH ];
  addr_t            ip[ MAX_NAMELOG_ADDRS ];
  char              guid[ 33 ];
  qboolean          guidless;
  int               slot;
  qboolean          banned;

  int               nameOffset;
  int               nameChangeTime;
  int               nameChanges;
  int               voteCount;

  qboolean          muted;
  qboolean          denyBuild;

  int               score;
  int               credits;
  team_t            team;

  int               id;
} namelog_t;

// client data that stays across multiple respawns, but is cleared
// on each level change or team change at ClientBegin()
typedef struct
{
  clientConnected_t   connected;
  usercmd_t           cmd;                // we would lose angles if not persistant
  qboolean            localClient;        // true if "ip" info key is "localhost"
  qboolean            stickySpec;         // don't stop spectating a player after they get killed
  qboolean            pmoveFixed;         //
  char                netname[ MAX_NAME_LENGTH ];
  int                 enterTime;          // level.time the client entered the game
  int                 location;           // player locations
  int                 teamInfo;           // level.time of team overlay update (disabled = 0)
  float               flySpeed;           // for spectator/noclip moves
  qboolean            disableBlueprintErrors; // should the buildable blueprint never be hidden from the players?
  int                 buildableRangeMarkerMask;

  class_t             classSelection;     // player class (copied to ent->client->ps.stats[ STAT_CLASS ] once spawned)
  float               evolveHealthFraction;
  weapon_t            humanItemSelection; // humans have a starting item
  team_t              teamSelection;      // player team (copied to ps.stats[ STAT_TEAM ])

  int                 teamChangeTime;     // level.time of last team change
  namelog_t           *namelog;
  g_admin_admin_t     *admin;

  int                 secondsAlive;       // time player has been alive in seconds
  qboolean            hasHealed;          // has healed a player (basi regen aura) in the last 10sec (for score use)

  // used to save persistant[] values while in SPECTATOR_FOLLOW mode
  int                 credit;

  int                 voted;
  int                 vote;

  // flood protection
  int                 floodDemerits;
  int                 floodTime;

  vec3_t              lastDeathLocation;
  int                 alternateProtocol;
  char                guid[ 33 ];
  qboolean            guidless;
  addr_t              ip;
  char                voice[ MAX_VOICE_NAME_LEN ];
  qboolean            useUnlagged;

  // level.time when teamoverlay info changed so we know to tell other players
  int                 infoChangeTime;
} clientPersistant_t;

#define MAX_UNLAGGED_MARKERS 256
typedef struct unlagged_s {
  vec3_t      origin;
  vec3_t      mins;
  vec3_t      maxs;
  qboolean    used;
} unlagged_t;

#define MAX_TRAMPLE_BUILDABLES_TRACKED 20
// this structure is cleared on each ClientSpawn(),
// except for 'client->pers' and 'client->sess'
struct gclient_s
{
  // ps MUST be the first element, because the server expects it
  playerState_t       ps;       // communicated by server to clients

  // exported into pmove, but not communicated to clients
  pmoveExt_t          pmext;

  // the rest of the structure is private to game
  clientPersistant_t  pers;
  clientSession_t     sess;

  qboolean            readyToExit;    // wishes to leave the intermission

  qboolean            noclip;
  int                 cliprcontents;  // the backup layer of ent->r.contents for when noclipping

  int                 lastCmdTime;    // level.time of last usercmd_t, for EF_CONNECTION
                                      // we can't just use pers.lastCommand.time, because
                                      // of the g_sycronousclients case
  int                 buttons;
  int                 oldbuttons;
  int                 latched_buttons;

  vec3_t              oldOrigin;

  // sum up damage over an entire frame, so
  // shotgun blasts give a single big kick
  int                 damage_armor;     // damage absorbed by armor
  int                 damage_blood;     // damage taken out of health
  int                 damage_knockback; // impact damage
  vec3_t              damage_from;      // origin for vector calculation
  qboolean            damage_fromWorld; // if true, don't use the damage_from vector

  //
  int                 lastkilled_client;// last client that this client killed
  int                 lasthurt_client;  // last client that damaged this client
  int                 lasthurt_mod;     // type of damage the client did

  // timers
  int                 respawnTime;      // can respawn when time > this
  int                 inactivityTime;   // kick players when time > this
  qboolean            inactivityWarning;// qtrue if the five seoond warning has been given
  int                 rewardTime;       // clear the EF_AWARD_IMPRESSIVE, etc when time > this
  int                 boostedTime;      // last time we touched a booster

  int                 airOutTime;

  qboolean            fireHeld;         // used for hook
  qboolean            fire2Held;        // used for alt fire
  gentity_t           *hook;            // grapple hook if out

  int                 switchTeamTime;   // time the player switched teams

  int                 time100;          // timer for 100ms interval events
  int                 time1000;         // timer for one second interval events
  int                 time10000;        // timer for ten second interval events

  char                *areabits;

  int                 lastPoisonTime;
  int                 poisonImmunityTime;
  gentity_t           *lastPoisonClient;
  int                 lastPoisonCloudedTime;
  int                 grabExpiryTime;
  int                 lastLockTime;
  int                 lastSlowTime;
  int                 lastMedKitTime;
  int                 medKitHealthToRestore;
  int                 medKitIncrementTime;
  int                 lastCreepSlowTime;    // time until creep can be removed

  qboolean            charging;

  int                 lastFlameBall;        // s.number of the last flame ball fired

  unlagged_t          unlaggedHist[ MAX_UNLAGGED_MARKERS ];
  unlagged_t          unlaggedBackup;
  unlagged_t          unlaggedCalc;
  int                 unlaggedTime;

  float               voiceEnthusiasm;
  char                lastVoiceCmd[ MAX_VOICE_CMD_LEN ];

  int                 lcannonStartTime;
  int                 trampleBuildablesHitPos;
  int                 trampleBuildablesHit[ MAX_TRAMPLE_BUILDABLES_TRACKED ];

  int                 lastCrushTime;        // Tyrant crush
  int                 lastDropTime;         // Weapon drop with /drop
};

typedef struct spawnQueue_s
{
  int clients[ MAX_CLIENTS ];

  int front, back;
} spawnQueue_t;

#define QUEUE_PLUS1(x)  (((x)+1)%MAX_CLIENTS)
#define QUEUE_MINUS1(x) (((x)+MAX_CLIENTS-1)%MAX_CLIENTS)

void      G_InitSpawnQueue( spawnQueue_t *sq );
int       G_GetSpawnQueueLength( spawnQueue_t *sq );
int       G_PopSpawnQueue( spawnQueue_t *sq );
int       G_PeekSpawnQueue( spawnQueue_t *sq );
qboolean  G_SearchSpawnQueue( spawnQueue_t *sq, int clientNum );
qboolean  G_PushSpawnQueue( spawnQueue_t *sq, int clientNum );
qboolean  G_RemoveFromSpawnQueue( spawnQueue_t *sq, int clientNum );
int       G_GetPosInSpawnQueue( spawnQueue_t *sq, int clientNum );
void      G_PrintSpawnQueue( spawnQueue_t *sq );


#define MAX_DAMAGE_REGION_TEXT    8192
#define MAX_DAMAGE_REGIONS 16

// build point zone
typedef struct
{
  int active;

  int totalBuildPoints;
  int queuedBuildPoints;
  int nextQueueTime;
} buildPointZone_t;

// store locational damage regions
typedef struct damageRegion_s
{
  char      name[ 32 ];
  float     area, modifier, minHeight, maxHeight;
  int       minAngle, maxAngle;
  qboolean  crouch;
} damageRegion_t;

//status of the warning of certain events
typedef enum
{
  TW_NOT = 0,
  TW_IMMINENT,
  TW_PASSED
} timeWarning_t;

// fate of a buildable
typedef enum
{
  BF_CONSTRUCT,
  BF_DECONSTRUCT,
  BF_REPLACE,
  BF_DESTROY,
  BF_TEAMKILL,
  BF_UNPOWER,
  BF_AUTO
} buildFate_t;

// data needed to revert a change in layout
typedef struct
{
  int          time;
  buildFate_t  fate;
  namelog_t    *actor;
  namelog_t    *builtBy;
  buildable_t  modelindex;
  qboolean     deconstruct;
  int          deconstructTime;
  vec3_t       origin;
  vec3_t       angles;
  vec3_t       origin2;
  vec3_t       angles2;
  buildable_t  powerSource;
  int          powerValue;
} buildLog_t;

//
// this structure is cleared as each map is entered
//
#define MAX_SPAWN_VARS      64
#define MAX_SPAWN_VARS_CHARS  4096
#define MAX_BUILDLOG          128
#define MAX_PLAYER_MODEL      256

typedef struct
{
  struct gclient_s  *clients;   // [maxclients]

  struct gentity_s  *gentities;
  int               gentitySize;
  int               num_entities;   // MAX_CLIENTS <= num_entities <= ENTITYNUM_MAX_NORMAL

  int               warmupTime;     // restart match at this time

  fileHandle_t      logFile;

  // store latched cvars here that we want to get at often
  int               maxclients;

  int               framenum;
  int               time;                         // in msec
  int               previousTime;                 // so movers can back up when blocked
  int               frameMsec;                    // trap_Milliseconds() at end frame

  int               startTime;                    // level.time the map was started

  int               teamScores[ NUM_TEAMS ];
  int               lastTeamLocationTime;         // last time of client team location update

  qboolean          newSession;                   // don't use any old session data, because
                                                  // we changed gametype

  qboolean          restarted;                    // waiting for a map_restart to fire

  int               numConnectedClients;
  int               numNonSpectatorClients;       // includes connecting clients
  int               numPlayingClients;            // connected, non-spectators
  int               sortedClients[MAX_CLIENTS];   // sorted by score

  int               snd_fry;                      // sound index for standing in lava

  int               warmupModificationCount;      // for detecting if g_warmup is changed

  // voting state
  int               voteThreshold[ NUM_TEAMS ];   // need at least this percent to pass
  char              voteString[ NUM_TEAMS ][ MAX_STRING_CHARS ];
  char              voteDisplayString[ NUM_TEAMS ][ MAX_STRING_CHARS ];
  int               voteTime[ NUM_TEAMS ];        // level.time vote was called
  int               voteExecuteTime[ NUM_TEAMS ]; // time the vote is executed
  int               voteDelay[ NUM_TEAMS ];       // it doesn't make sense to always delay vote execution
  int               voteYes[ NUM_TEAMS ];
  int               voteNo[ NUM_TEAMS ];
  int               numVotingClients[ NUM_TEAMS ];// set by CalculateRanks

  // spawn variables
  qboolean          spawning;                     // the G_Spawn*() functions are valid
  int               numSpawnVars;
  char              *spawnVars[ MAX_SPAWN_VARS ][ 2 ];  // key / value pairs
  int               numSpawnVarChars;
  char              spawnVarChars[ MAX_SPAWN_VARS_CHARS ];

  // intermission state
  qboolean          exited;
  int               intermissionQueued;           // intermission was qualified, but
                                                  // wait INTERMISSION_DELAY_TIME before
                                                  // actually going there so the last
                                                  // frag can be watched.  Disable future
                                                  // kills during this delay
  int               intermissiontime;             // time the intermission was started
  char              *changemap;
  qboolean          readyToExit;                  // at least one client wants to exit
  int               exitTime;
  vec3_t            intermission_origin;          // also used for spectator spawns
  vec3_t            intermission_angle;

  gentity_t         *locationHead;                // head of the location list

  int               numAlienSpawns;
  int               numHumanSpawns;

  int               numAlienClients;
  int               numHumanClients;

  float             averageNumAlienClients;
  int               numAlienSamples;
  float             averageNumHumanClients;
  int               numHumanSamples;

  int               numAlienClientsAlive;
  int               numHumanClientsAlive;

  int               alienBuildPoints;
  int               alienBuildPointQueue;
  int               alienNextQueueTime;
  int               humanBuildPoints;
  int               humanBuildPointQueue;
  int               humanNextQueueTime;

  buildPointZone_t  *buildPointZones;

  gentity_t         *markedBuildables[ MAX_GENTITIES ];
  int               numBuildablesForRemoval;

  int               alienKills;
  int               humanKills;

  qboolean          overmindMuted;

  int               humanBaseAttackTimer;

  team_t            lastWin;

  int               suddenDeathBeginTime;
  timeWarning_t     suddenDeathWarning;
  timeWarning_t     timelimitWarning;

  spawnQueue_t      alienSpawnQueue;
  spawnQueue_t      humanSpawnQueue;

  int               alienStage2Time;
  int               alienStage3Time;
  int               humanStage2Time;
  int               humanStage3Time;

  qboolean          uncondAlienWin;
  qboolean          uncondHumanWin;
  qboolean          alienTeamLocked;
  qboolean          humanTeamLocked;
  int               pausedTime;

  int unlaggedIndex;
  int unlaggedTimes[ MAX_UNLAGGED_MARKERS ];

  char              layout[ MAX_QPATH ];

  team_t            surrenderTeam;
  int               lastTeamImbalancedTime;
  int               numTeamImbalanceWarnings;

  voice_t           *voices;

  emoticon_t        emoticons[ MAX_EMOTICONS ];
  int               emoticonCount;
  
  char              *playerModel[ MAX_PLAYER_MODEL ];
  int               playerModelCount;

  namelog_t         *namelogs;

  buildLog_t        buildLog[ MAX_BUILDLOG ];
  int               buildId;
  int               numBuildLogs;
} level_locals_t;

#define CMD_CHEAT         0x0001
#define CMD_CHEAT_TEAM    0x0002 // is a cheat when used on a team
#define CMD_MESSAGE       0x0004 // sends message to others (skip when muted)
#define CMD_TEAM          0x0008 // must be on a team
#define CMD_SPEC          0x0010 // must be a spectator
#define CMD_ALIEN         0x0020
#define CMD_HUMAN         0x0040
#define CMD_ALIVE         0x0080
#define CMD_INTERMISSION  0x0100 // valid during intermission

typedef struct
{
  char *cmdName;
  int  cmdFlags;
  void ( *cmdHandler )( gentity_t *ent );
} commands_t;

//
// g_spawn.c
//
qboolean  G_SpawnString( const char *key, const char *defaultString, char **out );
// spawn string returns a temporary reference, you must CopyString() if you want to keep it
qboolean  G_SpawnFloat( const char *key, const char *defaultString, float *out );
qboolean  G_SpawnInt( const char *key, const char *defaultString, int *out );
qboolean  G_SpawnVector( const char *key, const char *defaultString, float *out );
void      G_SpawnEntitiesFromString( void );
char      *G_NewString( const char *string );

//
// g_cmds.c
//

#define DECOLOR_OFF '\16'
#define DECOLOR_ON  '\17'

void      G_StopFollowing( gentity_t *ent );
void      G_StopFromFollowing( gentity_t *ent );
void      G_FollowLockView( gentity_t *ent );
qboolean  G_FollowNewClient( gentity_t *ent, int dir );
void      G_ToggleFollow( gentity_t *ent );
int       G_ClientNumberFromString( char *s, char *err, int len );
int       G_ClientNumbersFromString( char *s, int *plist, int max );
char      *ConcatArgs( int start );
char      *ConcatArgsPrintable( int start );
void      G_Say( gentity_t *ent, saymode_t mode, const char *chatText );
void      G_DecolorString( const char *in, char *out, int len );
void      G_UnEscapeString( char *in, char *out, int len );
void      G_SanitiseString( char *in, char *out, int len );
void      Cmd_PrivateMessage_f( gentity_t *ent );
void      Cmd_ListMaps_f( gentity_t *ent );
void      Cmd_Test_f( gentity_t *ent );
void      Cmd_AdminMessage_f( gentity_t *ent );
int       G_FloodLimited( gentity_t *ent );
void      G_ListCommands( gentity_t *ent );
void      G_LoadCensors( void );
void      G_CensorString( char *out, const char *in, int len, gentity_t *ent );

//
// g_physics.c
//
void G_Physics( gentity_t *ent, int msec );

//
// g_buildable.c
//

#define MAX_ALIEN_BBOX  25

typedef enum
{
  IBE_NONE,

  IBE_NOOVERMIND,
  IBE_ONEOVERMIND,
  IBE_NOALIENBP,
  IBE_SPWNWARN, // not currently used
  IBE_NOCREEP,

  IBE_ONEREACTOR,
  IBE_NOPOWERHERE,
  IBE_TNODEWARN, // not currently used
  IBE_RPTNOREAC,
  IBE_RPTPOWERHERE,
  IBE_NOHUMANBP,
  IBE_NODCC,

  IBE_NORMAL, // too steep
  IBE_NOROOM,
  IBE_PERMISSION,
  IBE_LASTSPAWN,

  IBE_MAXERRORS
} itemBuildError_t;

gentity_t         *G_CheckSpawnPoint( int spawnNum, const vec3_t origin,
                                      const vec3_t normal, buildable_t spawn,
                                      vec3_t spawnOrigin );

buildable_t       G_IsPowered( vec3_t origin );
qboolean          G_IsDCCBuilt( void );
int               G_FindDCC( gentity_t *self );
gentity_t         *G_Reactor( void );
gentity_t         *G_Overmind( void );
qboolean          G_FindCreep( gentity_t *self );

void              G_BuildableThink( gentity_t *ent, int msec );
qboolean          G_BuildableRange( vec3_t origin, float r, buildable_t buildable );
void              G_ClearDeconMarks( void );
itemBuildError_t  G_CanBuild( gentity_t *ent, buildable_t buildable, int distance,
                              vec3_t origin, vec3_t normal, int *groundEntNum );
qboolean          G_BuildIfValid( gentity_t *ent, buildable_t buildable );
void              G_SetBuildableAnim( gentity_t *ent, buildableAnimNumber_t anim, qboolean force );
void              G_SetIdleBuildableAnim( gentity_t *ent, buildableAnimNumber_t anim );
void              G_SpawnBuildable(gentity_t *ent, buildable_t buildable);
void              FinishSpawningBuildable( gentity_t *ent );
void              G_LayoutSave( char *name );
int               G_LayoutList( const char *map, char *list, int len );
void              G_LayoutSelect( void );
void              G_LayoutLoad( char *lstr );
void              G_BaseSelfDestruct( team_t team );
int               G_NextQueueTime( int queuedBP, int totalBP, int queueBaseRate );
void              G_QueueBuildPoints( gentity_t *self );
int               G_GetBuildPoints( const vec3_t pos, team_t team );
int               G_GetMarkedBuildPoints( const vec3_t pos, team_t team );
qboolean          G_FindPower( gentity_t *self, qboolean searchUnspawned );
gentity_t         *G_PowerEntityForPoint( const vec3_t origin );
gentity_t         *G_PowerEntityForEntity( gentity_t *ent );
gentity_t         *G_RepeaterEntityForPoint( vec3_t origin );
gentity_t         *G_InPowerZone( gentity_t *self );
buildLog_t        *G_BuildLogNew( gentity_t *actor, buildFate_t fate );
void              G_BuildLogSet( buildLog_t *log, gentity_t *ent );
void              G_BuildLogAuto( gentity_t *actor, gentity_t *buildable, buildFate_t fate );
void              G_BuildLogRevert( int id );
void              G_RemoveRangeMarkerFrom( gentity_t *self );
void              G_UpdateBuildableRangeMarkers( void );

//
// g_utils.c
//
//addr_t in g_admin.h for g_admin_ban_t
qboolean    G_AddressParse( const char *str, addr_t *addr );
qboolean    G_AddressCompare( const addr_t *a, const addr_t *b );

int         G_ParticleSystemIndex( const char *name );
int         G_ShaderIndex( const char *name );
int         G_ModelIndex( const char *name );
int         G_SoundIndex( const char *name );
void        G_KillBox (gentity_t *ent);
gentity_t   *G_Find (gentity_t *from, int fieldofs, const char *match);
gentity_t   *G_PickTarget (char *targetname);
void        G_UseTargets (gentity_t *ent, gentity_t *activator);
void        G_SetMovedir ( vec3_t angles, vec3_t movedir);

void        G_InitGentity( gentity_t *e );
gentity_t   *G_Spawn( void );
gentity_t   *G_TempEntity( const vec3_t origin, int event );
void        G_Sound( gentity_t *ent, int channel, int soundIndex );
void        G_FreeEntity( gentity_t *e );
void        G_RemoveEntity( gentity_t *ent );
qboolean    G_EntitiesFree( void );

void        G_TouchTriggers( gentity_t *ent );

char        *vtos( const vec3_t v );

float       vectoyaw( const vec3_t vec );

void        G_AddPredictableEvent( gentity_t *ent, int event, int eventParm );
void        G_AddEvent( gentity_t *ent, int event, int eventParm );
void        G_BroadcastEvent( int event, int eventParm );
void        G_SetOrigin( gentity_t *ent, const vec3_t origin );
void        AddRemap(const char *oldShader, const char *newShader, float timeOffset);
const char  *BuildShaderStateConfig( void );


qboolean    G_ClientIsLagging( gclient_t *client );

void        G_TriggerMenu( int clientNum, dynMenu_t menu );
void        G_TriggerMenuArgs( int clientNum, dynMenu_t menu, int arg );
void        G_CloseMenus( int clientNum );

qboolean    G_Visible( gentity_t *ent1, gentity_t *ent2, int contents );
gentity_t   *G_ClosestEnt( vec3_t origin, gentity_t **entities, int numEntities );

//
// g_combat.c
//
qboolean  CanDamage( gentity_t *targ, vec3_t origin );
void      G_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
                    vec3_t dir, vec3_t point, int damage, int dflags, int mod );
void      G_SelectiveDamage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker, vec3_t dir,
                             vec3_t point, int damage, int dflags, int mod, int team );
qboolean  G_RadiusDamage( vec3_t origin, gentity_t *attacker, float damage, float radius,
                          gentity_t *ignore, int mod );
qboolean  G_SelectiveRadiusDamage( vec3_t origin, gentity_t *attacker, float damage, float radius,
                                   gentity_t *ignore, int mod, int team );
float     G_RewardAttackers( gentity_t *self );
void      AddScore( gentity_t *ent, int score );
void      G_LogDestruction( gentity_t *self, gentity_t *actor, int mod );

void      G_InitDamageLocations( void );

// damage flags
#define DAMAGE_RADIUS         0x00000001  // damage was indirect
#define DAMAGE_NO_ARMOR       0x00000002  // armour does not protect from this damage
#define DAMAGE_NO_KNOCKBACK   0x00000004  // do not affect velocity, just view angles
#define DAMAGE_NO_PROTECTION  0x00000008  // kills everything except godmode
#define DAMAGE_NO_LOCDAMAGE   0x00000010  // do not apply locational damage

//
// g_missile.c
//
void      G_BounceMissile( gentity_t *ent, trace_t *trace );
void      G_RunMissile( gentity_t *ent );

gentity_t *fire_flamer( gentity_t *self, vec3_t start, vec3_t aimdir );
gentity_t *fire_blaster( gentity_t *self, vec3_t start, vec3_t dir );
gentity_t *fire_pulseRifle( gentity_t *self, vec3_t start, vec3_t dir );
gentity_t *fire_luciferCannon( gentity_t *self, vec3_t start, vec3_t dir, int damage, int radius, int speed );
gentity_t *fire_lockblob( gentity_t *self, vec3_t start, vec3_t dir );
gentity_t *fire_paraLockBlob( gentity_t *self, vec3_t start, vec3_t dir );
gentity_t *fire_slowBlob( gentity_t *self, vec3_t start, vec3_t dir );
gentity_t *fire_bounceBall( gentity_t *self, vec3_t start, vec3_t dir );
gentity_t *fire_hive( gentity_t *self, vec3_t start, vec3_t dir );
gentity_t *launch_grenade( gentity_t *self, vec3_t start, vec3_t dir );


//
// g_mover.c
//
void G_RunMover( gentity_t *ent );
void Touch_DoorTrigger( gentity_t *ent, gentity_t *other, trace_t *trace );
void manualTriggerSpectator( gentity_t *trigger, gentity_t *player );

//
// g_trigger.c
//
void trigger_teleporter_touch( gentity_t *self, gentity_t *other, trace_t *trace );
void G_Checktrigger_stages( team_t team, stage_t stage );


//
// g_misc.c
//
void TeleportPlayer( gentity_t *player, vec3_t origin, vec3_t angles, float speed );

//
// g_playermodel.c
//
void G_InitPlayerModel(void);
qboolean G_IsValidPlayerModel(const char *model);
void G_FreePlayerModel(void);
void G_GetPlayerModelSkins( const char *modelname, char skins[][ 64 ], int maxskins, int *numskins );
char *GetSkin( char *modelname, char *wish );

//
// g_weapon.c
//

typedef struct zap_s
{
  qboolean      used;

  gentity_t     *creator;
  gentity_t     *targets[ LEVEL2_AREAZAP_MAX_TARGETS ];
  int           numTargets;
  float         distances[ LEVEL2_AREAZAP_MAX_TARGETS ];

  int           timeToLive;

  gentity_t     *effectChannel;
} zap_t;

#define MAX_ZAPS MAX_CLIENTS
extern zap_t zaps[ MAX_ZAPS ];

void      G_ForceWeaponChange( gentity_t *ent, weapon_t weapon );
void      G_GiveClientMaxAmmo( gentity_t *ent, qboolean buyingEnergyAmmo );
void      CalcMuzzlePoint( gentity_t *ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint );
void      SnapVectorTowards( vec3_t v, vec3_t to );
qboolean  CheckVenomAttack( gentity_t *ent );
void      CheckGrabAttack( gentity_t *ent );
qboolean  CheckPounceAttack( gentity_t *ent );
void      CheckCkitRepair( gentity_t *ent );
void      G_ChargeAttack( gentity_t *ent, gentity_t *victim );
void      G_CrushAttack( gentity_t *ent, gentity_t *victim );
void      G_UpdateZaps( int msec );
void      G_ClearPlayerZapEffects( gentity_t *player );


//
// g_client.c
//
void      G_AddCreditToClient( gclient_t *client, short credit, qboolean cap );
void      G_SetClientViewAngle( gentity_t *ent, const vec3_t angle );
gentity_t *G_SelectTremulousSpawnPoint( team_t team, vec3_t preference, vec3_t origin, vec3_t angles );
gentity_t *G_SelectSpawnPoint( vec3_t avoidPoint, vec3_t origin, vec3_t angles );
gentity_t *G_SelectAlienLockSpawnPoint( vec3_t origin, vec3_t angles );
gentity_t *G_SelectHumanLockSpawnPoint( vec3_t origin, vec3_t angles );
void      respawn( gentity_t *ent );
void      BeginIntermission( void );
void      ClientSpawn( gentity_t *ent, gentity_t *spawn, const vec3_t origin, const vec3_t angles );
void      player_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod );
qboolean  SpotWouldTelefrag( gentity_t *spot );

//
// g_svcmds.c
//
qboolean  ConsoleCommand( void );
void      G_RegisterCommands( void );
void      G_UnregisterCommands( void );

//
// g_weapon.c
//
void FireWeapon( gentity_t *ent );
void FireWeapon2( gentity_t *ent );
void FireWeapon3( gentity_t *ent );

//
// g_weapondrop.c
//
gentity_t *LaunchWeapon( gentity_t *client, weapon_t weap, vec3_t origin, vec3_t velocity );
gentity_t *G_DropWeapon( gentity_t *ent, weapon_t w, float angle );
void G_RunWeaponDrop(gentity_t *ent);

//
// g_main.c
//
void ScoreboardMessage( gentity_t *client );
void MoveClientToIntermission( gentity_t *client );
void G_MapConfigs( const char *mapname );
void CalculateRanks( void );
void FindIntermissionPoint( void );
void G_RunThink( gentity_t *ent );
void G_AdminMessage( gentity_t *ent, const char *string );
void QDECL G_LogPrintf( const char *fmt, ... ) __attribute__ ((format (printf, 1, 2)));
void SendScoreboardMessageToAllClients( void );
void QDECL G_Printf( const char *fmt, ... ) __attribute__ ((format (printf, 1, 2)));
void QDECL G_Error( const char *fmt, ... ) __attribute__ ((noreturn, format (printf, 1, 2)));
void G_Vote( gentity_t *ent, team_t team, qboolean voting );
void G_ExecuteVote( team_t team );
void G_CheckVote( team_t team );
void LogExit( const char *string );
int  G_TimeTilSuddenDeath( void );

//
// g_client.c
//
const char *ClientConnect( int clientNum, qboolean firstTime );
char *ClientUserinfoChanged( int clientNum, qboolean forceName );
void ClientDisconnect( int clientNum );
void ClientBegin( int clientNum );
void ClientCommand( int clientNum );

//
// g_active.c
//
void G_UnlaggedStore( void );
void G_UnlaggedClear( gentity_t *ent );
void G_UnlaggedCalc( int time, gentity_t *skipEnt );
void G_UnlaggedOn( gentity_t *attacker, vec3_t muzzle, float range );
void G_UnlaggedOff( void );
void ClientThink( int clientNum );
void ClientEndFrame( gentity_t *ent );
void G_RunClient( gentity_t *ent );

//
// g_team.c
//
team_t    G_TeamFromString( char *str );
void      G_TeamCommand( team_t team, const char *cmd );
qboolean  OnSameTeam( gentity_t *ent1, gentity_t *ent2 );
void      G_LeaveTeam( gentity_t *self );
void      G_ChangeTeam( gentity_t *ent, team_t newTeam );
gentity_t *Team_GetLocation( gentity_t *ent );
void      TeamplayInfoMessage( gentity_t *ent );
void      CheckTeamStatus( void );
void      G_UpdateTeamConfigStrings( void );

//
// g_session.c
//
void G_ReadSessionData( gclient_t *client );
void G_InitSessionData( gclient_t *client, char *userinfo );
void G_WriteSessionData( void );

//
// g_maprotation.c
//
void      G_PrintRotations( void );
void      G_AdvanceMapRotation( int depth );
qboolean  G_StartMapRotation( char *name, qboolean advance,
                              qboolean putOnStack, qboolean reset_index, int depth );
void      G_StopMapRotation( void );
qboolean  G_MapRotationActive( void );
void      G_InitMapRotations( void );
void      G_ShutdownMapRotations( void );
qboolean  G_MapExists( const char *name );
qboolean  G_LayoutExists( const char *map, const char *layout );
void      G_ClearRotationStack( void );

//
// g_namelog.c
//

void G_namelog_connect( gclient_t *client );
void G_namelog_disconnect( gclient_t *client );
void G_namelog_restore( gclient_t *client );
void G_namelog_update_score( gclient_t *client );
void G_namelog_update_name( gclient_t *client );
void G_namelog_cleanup( void );

//some maxs
#define MAX_FILEPATH      144

extern  level_locals_t  level;
extern  gentity_t       g_entities[ MAX_GENTITIES ];

#define FOFS(x) ((size_t)&(((gentity_t *)0)->x))

extern  vmCvar_t  g_dedicated;
extern  vmCvar_t  g_cheats;
extern  vmCvar_t  g_maxclients;     // allow this many total, including spectators
extern  vmCvar_t  g_maxGameClients;   // allow this many active
extern  vmCvar_t  g_restarted;
extern  vmCvar_t  g_lockTeamsAtStart;
extern  vmCvar_t  g_minNameChangePeriod;
extern  vmCvar_t  g_maxNameChanges;

extern  vmCvar_t  g_timelimit;
extern  vmCvar_t  g_suddenDeathTime;
extern  vmCvar_t  g_friendlyFire;
extern  vmCvar_t  g_friendlyBuildableFire;
extern  vmCvar_t  g_dretchPunt;
extern  vmCvar_t  g_password;
extern  vmCvar_t  g_needpass;
extern  vmCvar_t  g_gravity;
extern  vmCvar_t  g_speed;
extern  vmCvar_t  g_knockback;
extern  vmCvar_t  g_inactivity;
extern  vmCvar_t  g_debugMove;
extern  vmCvar_t  g_debugDamage;
extern  vmCvar_t  g_synchronousClients;
extern  vmCvar_t  g_motd;
extern  vmCvar_t  g_warmup;
extern  vmCvar_t  g_doWarmup;
extern  vmCvar_t  g_allowVote;
extern  vmCvar_t  g_voteLimit;
extern  vmCvar_t  g_suddenDeathVotePercent;
extern  vmCvar_t  g_suddenDeathVoteDelay;
extern  vmCvar_t  g_teamForceBalance;
extern  vmCvar_t  g_smoothClients;
extern  vmCvar_t  pmove_fixed;
extern  vmCvar_t  pmove_msec;

extern  vmCvar_t  g_alienBuildPoints;
extern  vmCvar_t  g_alienBuildQueueTime;
extern  vmCvar_t  g_humanBuildPoints;
extern  vmCvar_t  g_humanBuildQueueTime;
extern  vmCvar_t  g_humanRepeaterBuildPoints;
extern  vmCvar_t  g_humanRepeaterBuildQueueTime;
extern  vmCvar_t  g_humanRepeaterMaxZones;
extern  vmCvar_t  g_humanStage;
extern  vmCvar_t  g_humanCredits;
extern  vmCvar_t  g_humanMaxStage;
extern  vmCvar_t  g_humanStage2Threshold;
extern  vmCvar_t  g_humanStage3Threshold;
extern  vmCvar_t  g_alienStage;
extern  vmCvar_t  g_alienCredits;
extern  vmCvar_t  g_alienMaxStage;
extern  vmCvar_t  g_alienStage2Threshold;
extern  vmCvar_t  g_alienStage3Threshold;
extern  vmCvar_t  g_teamImbalanceWarnings;
extern  vmCvar_t  g_freeFundPeriod;

extern  vmCvar_t  g_unlagged;

extern  vmCvar_t  g_disabledEquipment;
extern  vmCvar_t  g_disabledClasses;
extern  vmCvar_t  g_disabledBuildables;

extern  vmCvar_t  g_markDeconstruct;

extern  vmCvar_t  g_debugMapRotation;
extern  vmCvar_t  g_currentMapRotation;
extern  vmCvar_t  g_mapRotationNodes;
extern  vmCvar_t  g_mapRotationStack;
extern  vmCvar_t  g_nextMap;
extern  vmCvar_t  g_initialMapRotation;
extern  vmCvar_t  g_sayAreaRange;

extern  vmCvar_t  g_debugVoices;
extern  vmCvar_t  g_voiceChats;

extern  vmCvar_t  g_floodMaxDemerits;
extern  vmCvar_t  g_floodMinTime;

extern  vmCvar_t  g_shove;

extern  vmCvar_t  g_mapConfigs;

extern  vmCvar_t  g_nextLayout;
extern  vmCvar_t  g_layouts[ 9 ];
extern  vmCvar_t  g_layoutAuto;

extern  vmCvar_t  g_emoticonsAllowedInNames;

extern  vmCvar_t  g_admin;
extern  vmCvar_t  g_adminTempBan;
extern  vmCvar_t  g_adminMaxBan;

extern  vmCvar_t  g_privateMessages;
extern  vmCvar_t  g_specChat;
extern  vmCvar_t  g_publicAdminMessages;
extern  vmCvar_t  g_allowTeamOverlay;

extern  vmCvar_t  g_censorship;

void      trap_Print( const char *fmt );
void      trap_Error( const char *fmt ) __attribute__((noreturn));
int       trap_Milliseconds( void );
int       trap_RealTime( qtime_t *qtime );
int       trap_Argc( void );
void      trap_Argv( int n, char *buffer, int bufferLength );
void      trap_Args( char *buffer, int bufferLength );
int       trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, enum FS_Mode mode );
void      trap_FS_Read( void *buffer, int len, fileHandle_t f );
void      trap_FS_Write( const void *buffer, int len, fileHandle_t f );
void      trap_FS_FCloseFile( fileHandle_t f );
int       trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize );
int       trap_FS_Seek( fileHandle_t f, long offset, enum FS_Mode origin ); // fsOrigin_t
void      trap_SendConsoleCommand( int exec_when, const char *text );
void      trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags );
void      trap_Cvar_Update( vmCvar_t *cvar );
void      trap_Cvar_Set( const char *var_name, const char *value );
int       trap_Cvar_VariableIntegerValue( const char *var_name );
void      trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void      trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t,
                               playerState_t *gameClients, int sizeofGameClient );
void      trap_DropClient( int clientNum, const char *reason );
void      trap_SendServerCommand( int clientNum, const char *text );
void      trap_SetConfigstring( int num, const char *string );
void      trap_GetConfigstring( int num, char *buffer, int bufferSize );
void      trap_SetConfigstringRestrictions( int num, const clientList_t *clientList );
void      trap_GetUserinfo( int num, char *buffer, int bufferSize );
void      trap_SetUserinfo( int num, const char *buffer );
void      trap_GetServerinfo( char *buffer, int bufferSize );
void      trap_SetBrushModel( gentity_t *ent, const char *name );
void      trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs,
                      const vec3_t end, int passEntityNum, int contentmask );
int       trap_PointContents( const vec3_t point, int passEntityNum );
qboolean  trap_InPVS( const vec3_t p1, const vec3_t p2 );
qboolean  trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 );
void      trap_AdjustAreaPortalState( gentity_t *ent, qboolean open );
qboolean  trap_AreasConnected( int area1, int area2 );
void      trap_LinkEntity( gentity_t *ent );
void      trap_UnlinkEntity( gentity_t *ent );
int       trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *entityList, int maxcount );
qboolean  trap_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
void      trap_GetUsercmd( int clientNum, usercmd_t *cmd );
qboolean  trap_GetEntityToken( char *buffer, int bufferSize );

void      trap_SnapVector( float *v );

void      trap_AddCommand( const char *cmdName );
void      trap_RemoveCommand( const char *cmdName );
int       trap_FS_GetFilteredFiles( const char *path, const char *extension, const char *filter, char *listbuf, int bufsize );
