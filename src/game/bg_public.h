// Copyright (C) 1999-2000 Id Software, Inc.
//
// bg_public.h -- definitions shared by both the server game and client game modules

/*
 *  Portions Copyright (C) 2000-2001 Tim Angus
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*  To assertain which portions are licensed under the LGPL and which are
 *  licensed by Id Software, Inc. please run a diff between the equivalent
 *  versions of the "Tremulous" modification and the unmodified "Quake3"
 *  game source code.
 */
                  
// because games can change separately from the main system version, we need a
// second version that must match between game and cgame
#define GAME_VERSION    "baseq3-1"

#define DEFAULT_GRAVITY   800
#define GIB_HEALTH      -40
#define ARMOR_PROTECTION  0.66

#define MAX_ITEMS     256

#define RANK_TIED_FLAG    0x4000

#define DEFAULT_SHOTGUN_SPREAD  700
#define DEFAULT_SHOTGUN_COUNT 11

#define ITEM_RADIUS     15    // item sizes are needed for client side pickup detection

#define LIGHTNING_RANGE   768

#define SCORE_NOT_PRESENT -9999 // for the CS_SCORES[12] when only one player is present

#define VOTE_TIME     30000 // 30 seconds before vote times out

#define MINS_Z        -24
#define DEFAULT_VIEWHEIGHT  26
#define CROUCH_VIEWHEIGHT 12
#define DEAD_VIEWHEIGHT   -16

//
// config strings are a general means of communicating variable length strings
// from the server to all connected clients.
//

// CS_SERVERINFO and CS_SYSTEMINFO are defined in q_shared.h
#define CS_MUSIC            2
#define CS_MESSAGE          3   // from the map worldspawn's message field
#define CS_MOTD             4   // g_motd string for server message of the day
#define CS_WARMUP           5   // server time when the match will be restarted
#define CS_SCORES1          6
#define CS_SCORES2          7
#define CS_VOTE_TIME        8
#define CS_VOTE_STRING      9
#define CS_VOTE_YES         10
#define CS_VOTE_NO          11

#define CS_TEAMVOTE_TIME    12
#define CS_TEAMVOTE_STRING  14
#define CS_TEAMVOTE_YES     16
#define CS_TEAMVOTE_NO      18

#define CS_GAME_VERSION     20
#define CS_LEVEL_START_TIME 21    // so the timer only shows the current level
#define CS_INTERMISSION     22    // when 1, fraglimit/timelimit has been hit and intermission will start in a second or two
#define CS_FLAGSTATUS       23    // string indicating flag status in CTF
#define CS_SHADERSTATE      24
#define CS_BOTINFO          25

#define CS_ITEMS            27    // string of 0's and 1's that tell which items are present

//TA: extra stuff:
#define CS_ABPOINTS         28
#define CS_HBPOINTS         29

#define CS_MODELS           32
#define CS_SOUNDS           (CS_MODELS+MAX_MODELS)
#define CS_PLAYERS          (CS_SOUNDS+MAX_SOUNDS)
#define MAX_PRECACHES       32
#define CS_LOCATIONS        (CS_PLAYERS+MAX_CLIENTS+MAX_PRECACHES)

#define CS_MAX              (CS_LOCATIONS+MAX_LOCATIONS)

#if (CS_MAX) > MAX_CONFIGSTRINGS
#error overflow: (CS_MAX) > MAX_CONFIGSTRINGS
#endif

typedef enum {
  GT_FFA,       // free for all
  GT_TOURNAMENT,    // one on one tournament
  GT_SINGLE_PLAYER, // single player ffa

  //-- team games go after this --

  GT_TEAM,      // team deathmatch
  GT_CTF,       // capture the flag
  GT_1FCTF,
  GT_OBELISK,
  GT_HARVESTER,

  GT_MAX_GAME_TYPE
} gametype_t;

typedef enum { GENDER_MALE, GENDER_FEMALE, GENDER_NEUTER } gender_t;

/*
===================================================================================

PMOVE MODULE

The pmove code takes a player_state_t and a usercmd_t and generates a new player_state_t
and some other output data.  Used for local prediction on the client game and true
movement on the server game.
===================================================================================
*/

typedef enum {
  PM_NORMAL,    // can accelerate and turn
  PM_NOCLIP,    // noclip movement
  PM_SPECTATOR, // still run into walls
  PM_DEAD,    // no acceleration or turning, but free falling
  PM_FREEZE,    // stuck in place with no control
  PM_INTERMISSION, // no movement or status bar
  PM_SPINTERMISSION // no movement or status bar
} pmtype_t;

typedef enum {
  WEAPON_READY,
  WEAPON_RAISING,
  WEAPON_DROPPING,
  WEAPON_FIRING,
  WEAPON_RELOADING
} weaponstate_t;

//TA: clip-size defines
#define CS_MG         30   //clip-size
#define CS_CG         500
#define CS_BFG        100 
#define CS_FLAMER     400

//TA: bitmasks to get ammo, clips and maxclips out of ammo array
#define BM_AMMO       0x3F
#define BM_CLIPS      0xC0

//TA: bitmasks to get weapons out of weapons store
#define BM_SWB        0x0000FFFF
#define BM_SW2B       0xFFFF0000


// pmove->pm_flags
#define PMF_DUCKED      1
#define PMF_JUMP_HELD   2
#define PMF_BACKWARDS_JUMP  8   // go into backwards land
#define PMF_BACKWARDS_RUN 16    // coast down to backwards run
#define PMF_TIME_LAND   32    // pm_time is time before rejump
#define PMF_TIME_KNOCKBACK  64    // pm_time is an air-accelerate only time
#define PMF_TIME_WATERJUMP  256   // pm_time is waterjump
#define PMF_RESPAWNED   512   // clear after attack and jump buttons come up
#define PMF_USE_ITEM_HELD 1024
#define PMF_GRAPPLE_PULL  2048  // pull towards grapple location
#define PMF_FOLLOW      4096  // spectate following another player
#define PMF_SCOREBOARD    8192  // spectate as a scoreboard
#define PMF_INVULEXPAND   16384 // invulnerability sphere set to full size


#define PMF_ALL_TIMES (PMF_TIME_WATERJUMP|PMF_TIME_LAND|PMF_TIME_KNOCKBACK)

#define MAXTOUCH  32
typedef struct {
  // state (in / out)
  playerState_t *ps;

  // command (in)
  usercmd_t cmd;
  int     tracemask;      // collide against these types of surfaces
  int     debugLevel;     // if set, diagnostic output will be printed
  qboolean  noFootsteps;    // if the game is setup for no footsteps by the server
  qboolean  gauntletHit;    // true if a gauntlet attack would actually hit something

  int       framecount;
  
  // results (out)
  int     numtouch;
  int     touchents[MAXTOUCH];

  vec3_t    mins, maxs;     // bounding box size

  int     watertype;
  int     waterlevel;

  float   xyspeed;
  
  // for fixed msec Pmove
  int     pmove_fixed;
  int     pmove_msec;
      
  // callbacks to test the world
  // these will be different functions during game and cgame
  void    (*trace)( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask );
  int     (*pointcontents)( const vec3_t point, int passEntityNum );
} pmove_t;

// if a full pmove isn't done on the client, you can just update the angles
void PM_UpdateViewAngles( playerState_t *ps, const usercmd_t *cmd );
void Pmove (pmove_t *pmove);

//===================================================================================


// player_state->stats[] indexes
typedef enum {
  STAT_HEALTH,
  STAT_ITEMS,
  STAT_ACTIVEITEMS,
  STAT_WEAPONS,         // 16 bit fields
  STAT_WEAPONS2,        //TA: another 16 bits to push the max weapon count up
  STAT_ARMOR,
  STAT_DEAD_YAW,          // look this direction when dead (FIXME: get rid of?)
  STAT_CLIENTS_READY,       // bit mask of clients wishing to exit the intermission (FIXME: configstring?)
  STAT_MAX_HEALTH, // health / armor limit, changable by handicap
  STAT_PCLASS,    //TA: player class (for droids AND humans)
  STAT_PTEAM,     //TA: player team
  STAT_STAMINA,   //TA: stamina (human only)
  STAT_STATE      //TA: client states e.g. wall climbing
} statIndex_t;

#define SCA_WALLCLIMBER         1
#define SCA_TAKESFALLDAMAGE     2
#define SCA_CANZOOM             4
#define SCA_CANJUMP             8
#define SCA_NOWEAPONDRIFT       16
#define SCA_FOVWARPS            32

#define SS_WALLCLIMBING         1
#define SS_GPISROTVEC           2
#define SS_CREEPSLOWED          4
#define SS_WALLTRANSIDING       8
#define SS_SPEEDBOOST           16
#define SS_INFESTING            32


// player_state->persistant[] indexes
// these fields are the only part of player_state that isn't
// cleared on respawn
typedef enum {
  PERS_SCORE,           // !!! MUST NOT CHANGE, SERVER AND GAME BOTH REFERENCE !!!
  PERS_HITS,            // total points damage inflicted so damage beeps can sound on change
  PERS_RANK,
  PERS_TEAM,
  PERS_SPAWN_COUNT,       // incremented every respawn
  PERS_PLAYEREVENTS,        // 16 bits that can be flipped for events
  PERS_REWARD,          // a reward_t
  PERS_ATTACKER,          // clientnum of last damage inflicter
  PERS_KILLED,          // count of the number of times you died
  // these were added for single player awards tracking
  PERS_IMPRESSIVE_COUNT,
  PERS_EXCELLENT_COUNT,
  PERS_GAUNTLET_FRAG_COUNT,
  PERS_ACCURACY_SHOTS,
  PERS_ACCURACY_HITS,
  //TA: FIXME: /\ get rid of award counts to make some room
  
  //TA: extra gubbins
  PERS_POINTS,
  PERS_TOTALPOINTS
} persEnum_t;


// entityState_t->eFlags
#define EF_DEAD             0x00000001    // don't draw a foe marker over players with EF_DEAD
#define EF_TELEPORT_BIT     0x00000004    // toggled every time the origin abruptly changes
#define EF_AWARD_EXCELLENT  0x00000008    // draw an excellent sprite
#define EF_BOUNCE           0x00000010    // for missiles
#define EF_BOUNCE_HALF      0x00000020    // for missiles
#define EF_AWARD_GAUNTLET   0x00000040    // draw a gauntlet sprite
#define EF_NODRAW           0x00000080    // may have an event, but no model (unspawned items)
#define EF_FIRING           0x00000100    // for lightning gun
#define EF_MOVER_STOP       0x00000400    // will push otherwise
#define EF_TALK             0x00001000    // draw a talk balloon
#define EF_CONNECTION       0x00002000    // draw a connection trouble sprite
#define EF_VOTED            0x00004000    // already cast a vote
#define EF_TEAMVOTED        0x00008000    // already cast a vote
#define EF_AWARD_IMPRESSIVE 0x00010000    // draw an impressive sprite

typedef enum {
  PW_NONE,

  PW_QUAD,
  PW_BATTLESUIT,
  PW_HASTE,
  PW_INVIS,
  PW_REGEN,
  PW_FLIGHT,

  PW_REDFLAG,
  PW_BLUEFLAG,
  PW_BALL,

  PW_NUM_POWERUPS
} powerup_t;

typedef enum {
  HI_NONE,

  HI_TELEPORTER,
  HI_MEDKIT,

  HI_NUM_HOLDABLE
} holdable_t;

typedef enum {
  WP_NONE,

  WP_GAUNTLET,
  WP_MACHINEGUN,
  WP_CHAINGUN,
  WP_SHOTGUN,
  WP_GRENADE_LAUNCHER,
  WP_ROCKET_LAUNCHER,
  WP_LIGHTNING,
  WP_RAILGUN,
  WP_FLAMER,
  WP_PLASMAGUN,
  WP_BFG,
  WP_GRAPPLING_HOOK,
  WP_VENOM,
  WP_HBUILD,
  WP_ABUILD,
  WP_SCANNER,
  WP_GGRENADE,

  WP_NUM_WEAPONS
} weapon_t;

typedef enum {
  UP_NONE,

  UP_TORCH,
  UP_NVG,
  UP_CHESTARMOUR,
  UP_LIMBARMOUR,
  UP_HELMET,
  UP_ANTITOXIN,
  UP_BATTPACK,
  UP_JETPACK,
  UP_THREATHELMET,
  UP_BATTLESUIT,
  UP_IMPANTKIT,

  UP_NUM_UPGRADES
} upgrade_t;

typedef enum {
  BA_NONE,

  BA_D_SPAWN,
  BA_D_DEF1,
  BA_H_SPAWN,
  BA_H_DEF1,
  BA_H_MCU,

  BA_NUM_BUILDABLES
} buildable_t;

typedef enum
{
  BIT_NONE,
  
  BIT_DROIDS,
  BIT_HUMANS,

  BIT_NUM_TEAMS
} buildableTeam_t;

// reward sounds (stored in ps->persistant[PERS_PLAYEREVENTS])
#define PLAYEREVENT_DENIEDREWARD    0x0001
#define PLAYEREVENT_GAUNTLETREWARD    0x0002
#define PLAYEREVENT_HOLYSHIT      0x0004

// entityState_t->event values
// entity events are for effects that take place reletive
// to an existing entities origin.  Very network efficient.

// two bits at the top of the entityState->event field
// will be incremented with each change in the event so
// that an identical event started twice in a row can
// be distinguished.  And off the value with ~EV_EVENT_BITS
// to retrieve the actual event number
#define EV_EVENT_BIT1   0x00000100
#define EV_EVENT_BIT2   0x00000200
#define EV_EVENT_BITS   (EV_EVENT_BIT1|EV_EVENT_BIT2)

typedef enum {
  EV_NONE,

  EV_FOOTSTEP,
  EV_FOOTSTEP_METAL,
  EV_FOOTSTEP_SQUELCH,
  EV_FOOTSPLASH,
  EV_FOOTWADE,
  EV_SWIM,

  EV_STEP_4,
  EV_STEP_8,
  EV_STEP_12,
  EV_STEP_16,

  EV_FALL_SHORT,
  EV_FALL_MEDIUM,
  EV_FALL_FAR,

  EV_JUMP_PAD,      // boing sound at origin, jump sound on player

  EV_JUMP,
  EV_WATER_TOUCH, // foot touches
  EV_WATER_LEAVE, // foot leaves
  EV_WATER_UNDER, // head touches
  EV_WATER_CLEAR, // head leaves

  EV_ITEM_PICKUP,     // normal item pickups are predictable
  EV_GLOBAL_ITEM_PICKUP,  // powerup / team sounds are broadcast to everyone

  EV_NOAMMO,
  EV_CHANGE_WEAPON,
  EV_FIRE_WEAPON,

  EV_USE_ITEM0,
  EV_USE_ITEM1,
  EV_USE_ITEM2,
  EV_USE_ITEM3,
  EV_USE_ITEM4,
  EV_USE_ITEM5,
  EV_USE_ITEM6,
  EV_USE_ITEM7,
  EV_USE_ITEM8,
  EV_USE_ITEM9,
  EV_USE_ITEM10,
  EV_USE_ITEM11,
  EV_USE_ITEM12,
  EV_USE_ITEM13,
  EV_USE_ITEM14,
  EV_USE_ITEM15,

  EV_ITEM_RESPAWN,
  EV_PLAYER_RESPAWN, //TA: for fovwarp effects
  EV_ITEM_GROW, //droid items that grow
  EV_ITEM_POP,
  EV_PLAYER_TELEPORT_IN,
  EV_PLAYER_TELEPORT_OUT,

  EV_GRENADE_BOUNCE,    // eventParm will be the soundindex

  EV_GENERAL_SOUND,
  EV_GLOBAL_SOUND,    // no attenuation

  EV_BULLET_HIT_FLESH,
  EV_BULLET_HIT_WALL,

  EV_MISSILE_HIT,
  EV_MISSILE_MISS,
  EV_MISSILE_MISS_METAL,
  EV_ITEM_EXPLOSION, //TA: human item explosions
  EV_RAILTRAIL,
  EV_SHOTGUN,
  EV_BULLET,        // otherEntity is the shooter

  EV_PAIN,
  EV_DEATH1,
  EV_DEATH2,
  EV_DEATH3,
  EV_OBITUARY,

  EV_POWERUP_QUAD,
  EV_POWERUP_BATTLESUIT,
  EV_POWERUP_REGEN,

  EV_GIB_PLAYER,      // gib a previously living player
  EV_GIB_DROID,       //TA: generic green gib for droids
  EV_ITEM_RECEDE,     //TA: sent when creep should recede

  EV_DEBUG_LINE,
  EV_STOPLOOPINGSOUND,
  EV_TAUNT,
  EV_TAUNT_YES,
  EV_TAUNT_NO,
  EV_TAUNT_FOLLOWME,
  EV_TAUNT_GETFLAG,
  EV_TAUNT_GUARDBASE,
  EV_TAUNT_PATROL,

  EV_MENU             //TA: menu event

} entity_event_t;

typedef enum
{
  MN_TEAM,
  MN_DROID,
  MN_HUMAN,
  MN_ABUILD,
  MN_HBUILD,
  MN_MCU,
  MN_INFEST
} dynMenu_t;

// animations
typedef enum {
  BOTH_DEATH1,
  BOTH_DEAD1,
  BOTH_DEATH2,
  BOTH_DEAD2,
  BOTH_DEATH3,
  BOTH_DEAD3,

  TORSO_GESTURE,

  TORSO_ATTACK,
  TORSO_ATTACK2,

  TORSO_DROP,
  TORSO_RAISE,

  TORSO_STAND,
  TORSO_STAND2,

  LEGS_WALKCR,
  LEGS_WALK,
  LEGS_RUN,
  LEGS_BACK,
  LEGS_SWIM,

  LEGS_JUMP,
  LEGS_LAND,

  LEGS_JUMPB,
  LEGS_LANDB,

  LEGS_IDLE,
  LEGS_IDLECR,

  LEGS_TURN,

#ifdef NEW_ANIMS
  TORSO_GETFLAG,
  TORSO_GUARDBASE,
  TORSO_PATROL,
  TORSO_FOLLOWME,
  TORSO_AFFIRMATIVE,
  TORSO_NEGATIVE,
#endif

  MAX_ANIMATIONS,

  LEGS_BACKCR,
  LEGS_BACKWALK,
  FLAG_RUN,
  FLAG_STAND,
  FLAG_STAND2RUN,

  MAX_TOTALANIMATIONS
} animNumber_t;


typedef struct animation_s {
  int   firstFrame;
  int   numFrames;
  int   loopFrames;     // 0 to numFrames
  int   frameLerp;      // msec between frames
  int   initialLerp;    // msec to get to first frame
  int   reversed;     // true if animation is reversed
  int   flipflop;     // true if animation should flipflop back to base
} animation_t;


// flip the togglebit every time an animation
// changes so a restart of the same anim can be detected
#define ANIM_TOGGLEBIT    128
#define ANIM_WALLCLIMBING 64


typedef enum {
  TEAM_FREE,
  TEAM_HUMANS,
  TEAM_DROIDS,
  TEAM_SPECTATOR,

  TEAM_NUM_TEAMS
} team_t;

// Time between location updates
#define TEAM_LOCATION_UPDATE_TIME   1000

// How many players on the overlay
#define TEAM_MAXOVERLAY   32

//FIXME: switch to enums at some point
//TA: player classes
typedef enum {
  PCL_NONE,
  PCL_D_B_BASE,
  PCL_D_O_BASE,
  PCL_D_D_BASE,
  PCL_H_BASE,

  PCL_NUM_CLASSES
} pClass_t;


//TA: player teams
typedef enum {
  PTE_NONE,
  PTE_DROIDS,
  PTE_HUMANS,

  PTE_NUM_TEAMS
} pTeam_t;


// means of death
typedef enum {
  MOD_UNKNOWN,
  MOD_SHOTGUN,
  MOD_GAUNTLET,
  MOD_MACHINEGUN,
  MOD_CHAINGUN,
  MOD_GRENADE,
  MOD_GRENADE_SPLASH,
  MOD_ROCKET,
  MOD_ROCKET_SPLASH,
  MOD_FLAMER,
  MOD_FLAMER_SPLASH,
  MOD_RAILGUN,
  MOD_LIGHTNING,
  MOD_BFG,
  MOD_BFG_SPLASH,
  MOD_WATER,
  MOD_SLIME,
  MOD_LAVA,
  MOD_CRUSH,
  MOD_TELEFRAG,
  MOD_FALLING,
  MOD_SUICIDE,
  MOD_TARGET_LASER,
  MOD_TRIGGER_HURT,
  MOD_GRAPPLE,
  MOD_VENOM,
  MOD_HSPAWN,
  MOD_ASPAWN
} meansOfDeath_t;


//---------------------------------------------------------

// gitem_t->type
typedef enum {
  IT_BAD,
  IT_WEAPON,        // EFX: rotate + upscale + minlight
  IT_BUILDABLE,  //TA: gitem_t->type for buildable items (spawns etc.)
  IT_UPGRADE,    //TA: gitem_t->type for human upgrades
  IT_AMMO,        // EFX: rotate
  IT_ARMOR,       // EFX: rotate + minlight
  IT_HEALTH,        // EFX: static external sphere + rotating internal
  IT_POWERUP,       // instant on, timer based
              // EFX: rotate + external ring that rotates
  IT_HOLDABLE,      // single use, holdable item
              // EFX: rotate + bob
  IT_TEAM
} itemType_t;

#define MAX_ITEM_MODELS 4

typedef struct gitem_s {
  char    *classname; // spawning name
  char    *pickup_sound;
  char    *world_model[MAX_ITEM_MODELS];

  char    *icon;
  char    *pickup_name; // for printing on pickup

  int     quantity;   // for ammo how much, or duration of powerup
  itemType_t  giType;     // IT_* flags

  int     giTag;

  char    *precaches;   // string of all models and images this item will use
  char    *sounds;    // string of all sounds this item will use
} gitem_t;

//TA: player class record
typedef struct
{
  int     classNum;

  char    *className;
  
  char    *modelName;
  char    *skinName;
  
  vec3_t  mins;
  vec3_t  maxs;
  vec3_t  crouchMaxs;
  vec3_t  deadMins;
  vec3_t  deadMaxs;
  
  int     viewheight;
  int     crouchViewheight;
  
  int     health;
  int     armor;
  
  int     abilities;
  
  int     fov;
  float   bob;
  int     steptime;
  float   speed;
  float   sticky;

  int     children[ 3 ];
  int     timeToEvolve;
} classAttributes_t;

//TA: buildable item record
typedef struct
{
  int       buildNum;

  char      *buildName;
  char      *entityName;

  vec3_t    mins;
  vec3_t    maxs;

  int       health;
  
  int       damage;
  int       splashDamage;
  int       splashRadius;

  int       meansOfDeath;

  int       team;

  int       spawnEvent;

  int       nextthink;

  qboolean  creepTest;
} buildableAttributes_t;      

// included in both the game dll and the client
extern  gitem_t bg_itemlist[];
extern  int   bg_numItems;

gitem_t *BG_FindItem( const char *pickupName );
gitem_t *BG_FindItemForWeapon( weapon_t weapon );
gitem_t *BG_FindItemForBuildable( buildable_t buildable );
gitem_t *BG_FindItemForUpgrade( upgrade_t upgrade );
gitem_t *BG_FindItemForPowerup( powerup_t pw );
gitem_t *BG_FindItemForHoldable( holdable_t pw );

//TA:
int       BG_FindBuildNumForName( char *name );
char      *BG_FindNameForBuildable( int bclass );
char      *BG_FindEntityNameForBuildable( int bclass );
void      BG_FindBBoxForBuildable( int bclass, vec3_t mins, vec3_t maxs );
int       BG_FindHealthForBuildable( int bclass );
int       BG_FindDamageForBuildable( int bclass );
int       BG_FindSplashDamageForBuildable( int bclass );
int       BG_FindSplashRadiusForBuildable( int bclass );
int       BG_FindMODForBuildable( int bclass );
int       BG_FindTeamForBuildable( int bclass );
int       BG_FindEventForBuildable( int bclass );
int       BG_FindNextThinkForBuildable( int bclass );
int       BG_FindCreepTestForBuildable( int bclass );

int       BG_FindClassNumForName( char *name );
char      *BG_FindNameForClassNum( int pclass );
char      *BG_FindModelNameForClass( int pclass );
void      BG_FindBBoxForClass( int pclass, vec3_t mins, vec3_t maxs, vec3_t cmaxs, vec3_t dmins, vec3_t dmaxs );
void      BG_FindViewheightForClass( int pclass, int *viewheight, int *cViewheight );
int       BG_FindHealthForClass( int pclass );
int       BG_FindArmorForClass( int pclass );
int       BG_FindFovForClass( int pclass );
float     BG_FindBobForClass( int pclass );
float     BG_FindSpeedForClass( int pclass );
float     BG_FindStickyForClass( int pclass );
int       BG_FindSteptimeForClass( int pclass );
qboolean  BG_ClassHasAbility( int pclass, int ability );
qboolean  BG_ClassCanEvolveFromTo( int fclass, int tclass );
int       BG_FindEvolveTimeForClass( int pclass );
#define ITEM_INDEX(x) ((x)-bg_itemlist)

qboolean  BG_CanItemBeGrabbed( int gametype, const entityState_t *ent, const playerState_t *ps );


// g_dmflags->integer flags
#define DF_NO_FALLING     8
#define DF_FIXED_FOV      16
#define DF_NO_FOOTSTEPS     32

// content masks
#define MASK_ALL        (-1)
#define MASK_SOLID        (CONTENTS_SOLID)
#define MASK_PLAYERSOLID    (CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BODY)
#define MASK_DEADSOLID      (CONTENTS_SOLID|CONTENTS_PLAYERCLIP)
#define MASK_WATER        (CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME)
#define MASK_OPAQUE       (CONTENTS_SOLID|CONTENTS_SLIME|CONTENTS_LAVA)
#define MASK_SHOT       (CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_CORPSE)


//
// entityState_t->eType
//
typedef enum {
  ET_GENERAL,
  ET_PLAYER,
  ET_ITEM,
  
  ET_BUILDABLE, //TA: buildable type
  ET_CREEP, //TA: creep type
  
  ET_MISSILE,
  ET_MOVER,
  ET_BEAM,
  ET_PORTAL,
  ET_SPEAKER,
  ET_PUSH_TRIGGER,
  ET_TELEPORT_TRIGGER,
  ET_INVISIBLE,
  ET_GRAPPLE,       // grapple hooked on wall

  ET_TORCH,         //TA: torch type
  ET_CORPSE,

  ET_EVENTS       // any of the EV_* events can be added freestanding
              // by setting eType to ET_EVENTS + eventNum
              // this avoids having to set eFlags and eventNum
} entityType_t;

void  BG_EvaluateTrajectory( const trajectory_t *tr, int atTime, vec3_t result );
void  BG_EvaluateTrajectoryDelta( const trajectory_t *tr, int atTime, vec3_t result );

void  BG_AddPredictableEventToPlayerstate( int newEvent, int eventParm, playerState_t *ps );

void  BG_TouchJumpPad( playerState_t *ps, entityState_t *jumppad );

void  BG_PlayerStateToEntityState( playerState_t *ps, entityState_t *s, qboolean snap );
void  BG_PlayerStateToEntityStateExtraPolate( playerState_t *ps, entityState_t *s, int time, qboolean snap );

qboolean  BG_PlayerTouchesItem( playerState_t *ps, entityState_t *item, int atTime );

//TA: extra bits: (which I apparently dont need)
/*void      BG_unpackAmmoArray( int weapon, int ammo[ ], int ammo2[ ], int *quan, int *clips, int *maxclips );
void      BG_packAmmoArray( int weapon, int ammo[ ], int ammo2[ ], int quan, int clips, int maxclips );
qboolean  BG_infiniteAmmo( int weapon );
void      BG_packWeapon( int weapon, int stats[ ] );
qboolean  BG_gotWeapon( int weapon, int stats[ ] );*/

#define CREEP_BASESIZE    120

#define ARENAS_PER_TIER   4
#define MAX_ARENAS      1024
#define MAX_ARENAS_TEXT   8192

#define MAX_BOTS      1024
#define MAX_BOTS_TEXT   8192

//TA: conceptually should live in q_shared.h
void    AxisToAngles( vec3_t axis[3], vec3_t angles);
float   arccos( float x );
#define Vector2Set(v, x, y) ((v)[0]=(x), (v)[1]=(y))

