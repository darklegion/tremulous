// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_main.c -- initialization and primary entry point for cgame

/*
 *  Portions Copyright (C) 2000-2001 Tim Angus
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the OSML - Open Source Modification License v1.0 as
 *  described in the file COPYING which is distributed with this source
 *  code.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
                    
#include "cg_local.h"

#include "../ui/ui_shared.h"
// display context for new ui stuff
displayContextDef_t cgDC;

int forceModelModificationCount = -1;

void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum );
void CG_Shutdown( void );

/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
int vmMain( int command, int arg0, int arg1, int arg2, int arg3,
                         int arg4, int arg5, int arg6, int arg7,
                         int arg8, int arg9, int arg10, int arg11 )
{
  switch( command )
  {
    case CG_INIT:
      CG_Init( arg0, arg1, arg2 );
      return 0;
      
    case CG_SHUTDOWN:
      CG_Shutdown( );
      return 0;
      
    case CG_CONSOLE_COMMAND:
      return CG_ConsoleCommand( );
      
    case CG_DRAW_ACTIVE_FRAME:
      CG_DrawActiveFrame( arg0, arg1, arg2 );
      return 0;
      
    case CG_CROSSHAIR_PLAYER:
      return CG_CrosshairPlayer( );
      
    case CG_LAST_ATTACKER:
      return CG_LastAttacker( );
      
    case CG_KEY_EVENT:
      CG_KeyEvent( arg0, arg1 );
      return 0;
      
    case CG_MOUSE_EVENT:
      cgDC.cursorx = cgs.cursorX;
      cgDC.cursory = cgs.cursorY;
      CG_MouseEvent( arg0, arg1 );
      return 0;
      
    case CG_EVENT_HANDLING:
      CG_EventHandling( arg0 );
      return 0;
      
    default:
      CG_Error( "vmMain: unknown command %i", command );
      break;
  }
  
  return -1;
}


cg_t        cg;
cgs_t       cgs;
centity_t   cg_entities[ MAX_GENTITIES ];

//TA: weapons limit expanded:
//weaponInfo_t    cg_weapons[MAX_WEAPONS];
weaponInfo_t    cg_weapons[ 32 ];
upgradeInfo_t   cg_upgrades[ 32 ];

buildableInfo_t cg_buildables[ BA_NUM_BUILDABLES ];

vmCvar_t  cg_teslaTrailTime;
vmCvar_t  cg_alienZapTime;
vmCvar_t  cg_railTrailTime;
vmCvar_t  cg_centertime;
vmCvar_t  cg_runpitch;
vmCvar_t  cg_runroll;
vmCvar_t  cg_bobup;
vmCvar_t  cg_bobpitch;
vmCvar_t  cg_bobroll;
vmCvar_t  cg_swingSpeed;
vmCvar_t  cg_shadows;
vmCvar_t  cg_gibs;
vmCvar_t  cg_drawTimer;
vmCvar_t  cg_drawFPS;
vmCvar_t  cg_drawSnapshot;
vmCvar_t  cg_draw3dIcons;
vmCvar_t  cg_drawIcons;
vmCvar_t  cg_drawAmmoWarning;
vmCvar_t  cg_drawCrosshair;
vmCvar_t  cg_drawCrosshairNames;
vmCvar_t  cg_drawRewards;
vmCvar_t  cg_crosshairX;
vmCvar_t  cg_crosshairY;
vmCvar_t  cg_draw2D;
vmCvar_t  cg_drawStatus;
vmCvar_t  cg_animSpeed;
vmCvar_t  cg_debugAnim;
vmCvar_t  cg_debugPosition;
vmCvar_t  cg_debugEvents;
vmCvar_t  cg_errorDecay;
vmCvar_t  cg_nopredict;
vmCvar_t  cg_noPlayerAnims;
vmCvar_t  cg_showmiss;
vmCvar_t  cg_footsteps;
vmCvar_t  cg_addMarks;
vmCvar_t  cg_brassTime;
vmCvar_t  cg_viewsize;
vmCvar_t  cg_drawGun;
vmCvar_t  cg_gun_frame;
vmCvar_t  cg_gun_x;
vmCvar_t  cg_gun_y;
vmCvar_t  cg_gun_z;
vmCvar_t  cg_tracerChance;
vmCvar_t  cg_tracerWidth;
vmCvar_t  cg_tracerLength;
vmCvar_t  cg_autoswitch;
vmCvar_t  cg_ignore;
vmCvar_t  cg_simpleItems;
vmCvar_t  cg_fov;
vmCvar_t  cg_zoomFov;
vmCvar_t  cg_thirdPerson;
vmCvar_t  cg_thirdPersonRange;
vmCvar_t  cg_thirdPersonAngle;
vmCvar_t  cg_stereoSeparation;
vmCvar_t  cg_lagometer;
vmCvar_t  cg_drawAttacker;
vmCvar_t  cg_synchronousClients;
vmCvar_t  cg_teamChatTime;
vmCvar_t  cg_teamChatHeight;
vmCvar_t  cg_stats;
vmCvar_t  cg_buildScript;
vmCvar_t  cg_forceModel;
vmCvar_t  cg_paused;
vmCvar_t  cg_blood;
vmCvar_t  cg_predictItems;
vmCvar_t  cg_deferPlayers;
vmCvar_t  cg_drawTeamOverlay;
vmCvar_t  cg_teamOverlayUserinfo;
vmCvar_t  cg_drawFriend;
vmCvar_t  cg_teamChatsOnly;
vmCvar_t  cg_noVoiceChats;
vmCvar_t  cg_noVoiceText;
vmCvar_t  cg_hudFiles;
vmCvar_t  cg_scorePlum;
vmCvar_t  cg_smoothClients;
vmCvar_t  pmove_fixed;
//vmCvar_t  cg_pmove_fixed;
vmCvar_t  pmove_msec;
vmCvar_t  cg_pmove_msec;
vmCvar_t  cg_cameraMode;
vmCvar_t  cg_cameraOrbit;
vmCvar_t  cg_cameraOrbitDelay;
vmCvar_t  cg_timescaleFadeEnd;
vmCvar_t  cg_timescaleFadeSpeed;
vmCvar_t  cg_timescale;
vmCvar_t  cg_smallFont;
vmCvar_t  cg_bigFont;
vmCvar_t  cg_noTaunt;
vmCvar_t  cg_noProjectileTrail;
vmCvar_t  cg_oldRail;
vmCvar_t  cg_oldRocket;
vmCvar_t  cg_oldPlasma;
vmCvar_t  cg_trueLightning;
vmCvar_t  cg_creepRes;
vmCvar_t  cg_drawSurfNormal;
vmCvar_t  cg_debugAlloc;
vmCvar_t  cg_wwSmoothTime;
vmCvar_t  cg_wwFollow;
vmCvar_t  cg_zsortLEs;
vmCvar_t  cg_consoleLatency;
vmCvar_t  cg_lightFlare;

//TA: hack to get class and carriage through to UI module
vmCvar_t  ui_currentClass;
vmCvar_t  ui_carriage;
vmCvar_t  ui_stages;
vmCvar_t  ui_dialog;
vmCvar_t  ui_loading;


typedef struct
{
  vmCvar_t  *vmCvar;
  char      *cvarName;
  char      *defaultString;
  int       cvarFlags;
} cvarTable_t;

static cvarTable_t cvarTable[ ] =
{
  { &cg_ignore, "cg_ignore", "0", 0 },  // used for debugging
  { &cg_autoswitch, "cg_autoswitch", "1", CVAR_ARCHIVE },
  { &cg_drawGun, "cg_drawGun", "1", CVAR_ARCHIVE },
  { &cg_zoomFov, "cg_zoomfov", "22.5", CVAR_ARCHIVE },
  { &cg_fov, "cg_fov", "90", CVAR_ARCHIVE },
  { &cg_viewsize, "cg_viewsize", "100", CVAR_ARCHIVE },
  { &cg_stereoSeparation, "cg_stereoSeparation", "0.4", CVAR_ARCHIVE  },
  { &cg_shadows, "cg_shadows", "1", CVAR_ARCHIVE  },
  { &cg_gibs, "cg_gibs", "1", CVAR_ARCHIVE  },
  { &cg_draw2D, "cg_draw2D", "1", CVAR_ARCHIVE  },
  { &cg_drawStatus, "cg_drawStatus", "1", CVAR_ARCHIVE  },
  { &cg_drawTimer, "cg_drawTimer", "1", CVAR_ARCHIVE  },
  { &cg_drawFPS, "cg_drawFPS", "1", CVAR_ARCHIVE  },
  { &cg_drawSnapshot, "cg_drawSnapshot", "0", CVAR_ARCHIVE  },
  { &cg_draw3dIcons, "cg_draw3dIcons", "1", CVAR_ARCHIVE  },
  { &cg_drawIcons, "cg_drawIcons", "1", CVAR_ARCHIVE  },
  { &cg_drawAmmoWarning, "cg_drawAmmoWarning", "1", CVAR_ARCHIVE  },
  { &cg_drawAttacker, "cg_drawAttacker", "1", CVAR_ARCHIVE  },
  { &cg_drawCrosshair, "cg_drawCrosshair", "4", CVAR_ARCHIVE },
  { &cg_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE },
  { &cg_drawRewards, "cg_drawRewards", "1", CVAR_ARCHIVE },
  { &cg_crosshairX, "cg_crosshairX", "0", CVAR_ARCHIVE },
  { &cg_crosshairY, "cg_crosshairY", "0", CVAR_ARCHIVE },
  { &cg_brassTime, "cg_brassTime", "2500", CVAR_ARCHIVE },
  { &cg_simpleItems, "cg_simpleItems", "0", CVAR_ARCHIVE },
  { &cg_addMarks, "cg_marks", "1", CVAR_ARCHIVE },
  { &cg_lagometer, "cg_lagometer", "0", CVAR_ARCHIVE },
  { &cg_teslaTrailTime, "cg_teslaTrailTime", "600", CVAR_ARCHIVE  },
  { &cg_alienZapTime, "cg_alienZapTime", "500", CVAR_ARCHIVE  },
  { &cg_railTrailTime, "cg_railTrailTime", "400", CVAR_ARCHIVE  },
  { &cg_gun_x, "cg_gunX", "0", CVAR_CHEAT },
  { &cg_gun_y, "cg_gunY", "0", CVAR_CHEAT },
  { &cg_gun_z, "cg_gunZ", "0", CVAR_CHEAT },
  { &cg_centertime, "cg_centertime", "3", CVAR_CHEAT },
  { &cg_runpitch, "cg_runpitch", "0.002", CVAR_ARCHIVE},
  { &cg_runroll, "cg_runroll", "0.005", CVAR_ARCHIVE },
  { &cg_bobup , "cg_bobup", "0.005", CVAR_CHEAT },
  { &cg_bobpitch, "cg_bobpitch", "0.002", CVAR_ARCHIVE },
  { &cg_bobroll, "cg_bobroll", "0.002", CVAR_ARCHIVE },
  { &cg_swingSpeed, "cg_swingSpeed", "0.3", CVAR_CHEAT },
  { &cg_animSpeed, "cg_animspeed", "1", CVAR_CHEAT },
  { &cg_debugAnim, "cg_debuganim", "0", CVAR_CHEAT },
  { &cg_debugPosition, "cg_debugposition", "0", CVAR_CHEAT },
  { &cg_debugEvents, "cg_debugevents", "0", CVAR_CHEAT },
  { &cg_errorDecay, "cg_errordecay", "100", 0 },
  { &cg_nopredict, "cg_nopredict", "0", 0 },
  { &cg_noPlayerAnims, "cg_noplayeranims", "0", CVAR_CHEAT },
  { &cg_showmiss, "cg_showmiss", "0", 0 },
  { &cg_footsteps, "cg_footsteps", "1", CVAR_CHEAT },
  { &cg_tracerChance, "cg_tracerchance", "0.4", CVAR_CHEAT },
  { &cg_tracerWidth, "cg_tracerwidth", "1", CVAR_CHEAT },
  { &cg_tracerLength, "cg_tracerlength", "100", CVAR_CHEAT },
  { &cg_thirdPersonRange, "cg_thirdPersonRange", "40", CVAR_CHEAT },
  { &cg_thirdPersonAngle, "cg_thirdPersonAngle", "0", CVAR_CHEAT },
  { &cg_thirdPerson, "cg_thirdPerson", "0", CVAR_CHEAT },
  { &cg_teamChatTime, "cg_teamChatTime", "3000", CVAR_ARCHIVE  },
  { &cg_teamChatHeight, "cg_teamChatHeight", "0", CVAR_ARCHIVE  },
  { &cg_forceModel, "cg_forceModel", "0", CVAR_ARCHIVE  },
  { &cg_predictItems, "cg_predictItems", "1", CVAR_ARCHIVE },
  { &cg_deferPlayers, "cg_deferPlayers", "1", CVAR_ARCHIVE },
  { &cg_drawTeamOverlay, "cg_drawTeamOverlay", "0", CVAR_ARCHIVE },
  { &cg_teamOverlayUserinfo, "teamoverlay", "0", CVAR_ROM | CVAR_USERINFO },
  { &cg_stats, "cg_stats", "0", 0 },
  { &cg_drawFriend, "cg_drawFriend", "1", CVAR_ARCHIVE },
  { &cg_teamChatsOnly, "cg_teamChatsOnly", "0", CVAR_ARCHIVE },
  { &cg_noVoiceChats, "cg_noVoiceChats", "0", CVAR_ARCHIVE },
  { &cg_noVoiceText, "cg_noVoiceText", "0", CVAR_ARCHIVE },
  { &cg_creepRes, "cg_creepRes", "16", CVAR_ARCHIVE },
  { &cg_drawSurfNormal, "cg_drawSurfNormal", "0", CVAR_CHEAT },
  { &cg_debugAlloc, "cg_debugAlloc", "0", 0 },
  { &cg_wwSmoothTime, "cg_wwSmoothTime", "300", CVAR_ARCHIVE },
  { &cg_wwFollow, "cg_wwFollow", "1", CVAR_ARCHIVE|CVAR_USERINFO },
  { &cg_zsortLEs, "cg_zsortLEs", "1", CVAR_ARCHIVE },
  { &cg_consoleLatency, "cg_consoleLatency", "3000", CVAR_ARCHIVE },
  { &cg_lightFlare, "cg_lightFlare", "3", CVAR_ARCHIVE },
  
  { &ui_currentClass, "ui_currentClass", "0", 0 },
  { &ui_carriage, "ui_carriage", "", 0 },
  { &ui_stages, "ui_stages", "0 0", 0 },
  { &ui_dialog, "ui_dialog", "Text not set", 0 },
  { &ui_loading, "ui_loading", "0", 0 },

  // the following variables are created in other parts of the system,
  // but we also reference them here

  { &cg_buildScript, "com_buildScript", "0", 0 }, // force loading of all possible data amd error on failures
  { &cg_paused, "cl_paused", "0", CVAR_ROM },
  { &cg_blood, "com_blood", "1", CVAR_ARCHIVE },
  { &cg_synchronousClients, "g_synchronousClients", "0", 0 }, // communicated by systeminfo
  { &cg_cameraOrbit, "cg_cameraOrbit", "0", CVAR_CHEAT},
  { &cg_cameraOrbitDelay, "cg_cameraOrbitDelay", "50", CVAR_ARCHIVE},
  { &cg_timescaleFadeEnd, "cg_timescaleFadeEnd", "1", 0},
  { &cg_timescaleFadeSpeed, "cg_timescaleFadeSpeed", "0", 0},
  { &cg_timescale, "timescale", "1", 0},
  { &cg_scorePlum, "cg_scorePlums", "1", CVAR_USERINFO | CVAR_ARCHIVE},
  { &cg_smoothClients, "cg_smoothClients", "0", CVAR_USERINFO | CVAR_ARCHIVE},
  { &cg_cameraMode, "com_cameraMode", "0", CVAR_CHEAT},

  { &pmove_fixed, "pmove_fixed", "0", 0},
  { &pmove_msec, "pmove_msec", "8", 0},
  { &cg_noTaunt, "cg_noTaunt", "0", CVAR_ARCHIVE},
  { &cg_noProjectileTrail, "cg_noProjectileTrail", "0", CVAR_ARCHIVE},
  { &cg_smallFont, "ui_smallFont", "0.25", CVAR_ARCHIVE},
  { &cg_bigFont, "ui_bigFont", "0.4", CVAR_ARCHIVE},
  { &cg_oldRail, "cg_oldRail", "1", CVAR_ARCHIVE},
  { &cg_oldRocket, "cg_oldRocket", "1", CVAR_ARCHIVE},
  { &cg_oldPlasma, "cg_oldPlasma", "1", CVAR_ARCHIVE},
  { &cg_trueLightning, "cg_trueLightning", "0.0", CVAR_ARCHIVE}
//  { &cg_pmove_fixed, "cg_pmove_fixed", "0", CVAR_USERINFO | CVAR_ARCHIVE }
};

static int   cvarTableSize = sizeof( cvarTable ) / sizeof( cvarTable[0] );

/*
=================
CG_RegisterCvars
=================
*/
void CG_RegisterCvars( void )
{
  int         i;
  cvarTable_t *cv;
  char        var[ MAX_TOKEN_CHARS ];

  for( i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++ )
  {
    trap_Cvar_Register( cv->vmCvar, cv->cvarName,
      cv->defaultString, cv->cvarFlags );
  }

  //repress standard Q3 console
  trap_Cvar_Set( "con_notifytime", "-2" );
  
  // see if we are also running the server on this machine
  trap_Cvar_VariableStringBuffer( "sv_running", var, sizeof( var ) );
  cgs.localServer = atoi( var );
  forceModelModificationCount = cg_forceModel.modificationCount;

  trap_Cvar_Register( NULL, "model", DEFAULT_MODEL, CVAR_USERINFO | CVAR_ARCHIVE );
  trap_Cvar_Register( NULL, "headmodel", DEFAULT_MODEL, CVAR_USERINFO | CVAR_ARCHIVE );
  trap_Cvar_Register( NULL, "team_model", DEFAULT_TEAM_MODEL, CVAR_USERINFO | CVAR_ARCHIVE );
  trap_Cvar_Register( NULL, "team_headmodel", DEFAULT_TEAM_HEAD, CVAR_USERINFO | CVAR_ARCHIVE );
}


/*                                                                      
===================
CG_ForceModelChange
===================
*/
static void CG_ForceModelChange( void )
{
  int   i;

  for( i = 0; i < MAX_CLIENTS; i++ )
  {
    const char    *clientInfo;

    clientInfo = CG_ConfigString( CS_PLAYERS + i );
    
    if( !clientInfo[ 0 ] )
      continue;

    CG_NewClientInfo( i );
  }
}


/*
=================
CG_UpdateCvars
=================
*/
void CG_UpdateCvars( void )
{
  int         i;
  cvarTable_t *cv;

  for( i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++ )
    trap_Cvar_Update( cv->vmCvar );

  // check for modications here

  // if force model changed
  if( forceModelModificationCount != cg_forceModel.modificationCount )
  {
    forceModelModificationCount = cg_forceModel.modificationCount;
    CG_ForceModelChange( );
  }
}


int CG_CrosshairPlayer( void )
{
  if( cg.time > ( cg.crosshairClientTime + 1000 ) )
    return -1;

  return cg.crosshairClientNum;
}


int CG_LastAttacker( void )
{
  if( !cg.attackerTime )
    return -1;

  return cg.snap->ps.persistant[ PERS_ATTACKER ];
}

void CG_RemoveConsoleLine( void )
{
  int i, offset, totalLength;

  if( cg.numConsoleLines == 0 )
    return;

  offset = cg.consoleLines[ 0 ].length;
  totalLength = strlen( cg.consoleText ) - offset;
  
  //slide up consoleText
  for( i = 0; i <= totalLength; i++ )
    cg.consoleText[ i ] = cg.consoleText[ i + offset ];
  
  //pop up the first consoleLine
  for( i = 0; i < cg.numConsoleLines; i++ )
    cg.consoleLines[ i ] = cg.consoleLines[ i + 1 ];

  cg.numConsoleLines--;
}

//TA: team arena UI based console
void CG_TAUIConsole( const char *text )
{
  if( cg.numConsoleLines == MAX_CONSOLE_LINES )
    CG_RemoveConsoleLine( );

  if( cg.consoleValid )
  {
    strcat( cg.consoleText, text );
    cg.consoleLines[ cg.numConsoleLines ].time = cg.time;
    cg.consoleLines[ cg.numConsoleLines ].length = strlen( text );
    cg.numConsoleLines++;
  }
  
}

void QDECL CG_Printf( const char *msg, ... )
{
  va_list argptr;
  char    text[ 1024 ];

  va_start( argptr, msg );
  vsprintf( text, msg, argptr );
  va_end( argptr );

  CG_TAUIConsole( text );

  trap_Print( text );
}

void QDECL CG_Error( const char *msg, ... )
{
  va_list argptr;
  char    text[ 1024 ];

  va_start( argptr, msg );
  vsprintf( text, msg, argptr );
  va_end( argptr );

  trap_Error( text );
}

#ifndef CGAME_HARD_LINKED
// this is only here so the functions in q_shared.c and bg_*.c can link (FIXME)

void QDECL Com_Error( int level, const char *error, ... )
{
  va_list argptr;
  char    text[1024];

  va_start( argptr, error );
  vsprintf( text, error, argptr );
  va_end( argptr );

  CG_Error( "%s", text );
}

void QDECL Com_Printf( const char *msg, ... ) {
  va_list   argptr;
  char    text[1024];

  va_start (argptr, msg);
  vsprintf (text, msg, argptr);
  va_end (argptr);

  //TA: team arena UI based console
  if( cg.numConsoleLines == MAX_CONSOLE_LINES )
    CG_RemoveConsoleLine( );

  if( cg.consoleValid )
  {
    strcat( cg.consoleText, text );
    cg.consoleLines[ cg.numConsoleLines ].time = cg.time;
    cg.consoleLines[ cg.numConsoleLines ].length = strlen( text );
    cg.numConsoleLines++;
  }
  
  CG_Printf ("%s", text);
}

#endif



/*
================
CG_Argv
================
*/
const char *CG_Argv( int arg )
{
  static char buffer[ MAX_STRING_CHARS ];

  trap_Argv( arg, buffer, sizeof( buffer ) );

  return buffer;
}


//========================================================================

/*
=================
CG_RegisterSounds

called during a precache command
=================
*/
static void CG_RegisterSounds( void )
{
  int         i;
  char        name[ MAX_QPATH ];
  const char  *soundName;

  // voice commands

  cgs.media.alienStageTransition  = trap_S_RegisterSound( "sound/announcements/overmindevolved.wav", qtrue );
  cgs.media.humanStageTransition  = trap_S_RegisterSound( "sound/announcements/reinforcement.wav", qtrue );

  cgs.media.alienOvermindAttack   = trap_S_RegisterSound( "sound/announcements/overmindattack.wav", qtrue );
  cgs.media.alienOvermindDying    = trap_S_RegisterSound( "sound/announcements/overminddying.wav", qtrue );
  cgs.media.alienOvermindSpawns   = trap_S_RegisterSound( "sound/announcements/overmindspawns.wav", qtrue );
  
  cgs.media.tracerSound           = trap_S_RegisterSound( "sound/weapons/machinegun/buletby1.wav", qfalse );
  cgs.media.selectSound           = trap_S_RegisterSound( "sound/weapons/change.wav", qfalse );
  cgs.media.wearOffSound          = trap_S_RegisterSound( "sound/items/wearoff.wav", qfalse );
  cgs.media.useNothingSound       = trap_S_RegisterSound( "sound/items/use_nothing.wav", qfalse );
  cgs.media.gibSound              = trap_S_RegisterSound( "sound/player/gibsplt1.wav", qfalse );
  cgs.media.gibBounce1Sound       = trap_S_RegisterSound( "sound/player/gibimp1.wav", qfalse );
  cgs.media.gibBounce2Sound       = trap_S_RegisterSound( "sound/player/gibimp2.wav", qfalse );
  cgs.media.gibBounce3Sound       = trap_S_RegisterSound( "sound/player/gibimp3.wav", qfalse );

  cgs.media.teleInSound           = trap_S_RegisterSound( "sound/world/telein.wav", qfalse );
  cgs.media.teleOutSound          = trap_S_RegisterSound( "sound/world/teleout.wav", qfalse );
  cgs.media.respawnSound          = trap_S_RegisterSound( "sound/items/respawn1.wav", qfalse );

  cgs.media.noAmmoSound           = trap_S_RegisterSound( "sound/weapons/noammo.wav", qfalse );

  cgs.media.talkSound             = trap_S_RegisterSound( "sound/player/talk.wav", qfalse );
  cgs.media.landSound             = trap_S_RegisterSound( "sound/player/land1.wav", qfalse );

  cgs.media.hitSound              = trap_S_RegisterSound( "sound/feedback/hit.wav", qfalse );

  cgs.media.watrInSound           = trap_S_RegisterSound( "sound/player/watr_in.wav", qfalse );
  cgs.media.watrOutSound          = trap_S_RegisterSound( "sound/player/watr_out.wav", qfalse );
  cgs.media.watrUnSound           = trap_S_RegisterSound( "sound/player/watr_un.wav", qfalse );

  cgs.media.jumpPadSound          = trap_S_RegisterSound( "sound/world/jumppad.wav", qfalse );

  for( i = 0; i < 4; i++ )
  {
    Com_sprintf( name, sizeof( name ), "sound/player/footsteps/step%i.wav", i + 1 );
    cgs.media.footsteps[ FOOTSTEP_NORMAL ][ i ] = trap_S_RegisterSound( name, qfalse );

    Com_sprintf( name, sizeof( name ), "sound/player/footsteps/boot%i.wav", i + 1 );
    cgs.media.footsteps[ FOOTSTEP_BOOT ][ i ] = trap_S_RegisterSound( name, qfalse );

    Com_sprintf( name, sizeof( name ), "sound/player/footsteps/flesh%i.wav", i + 1 );
    cgs.media.footsteps[ FOOTSTEP_FLESH ][ i ] = trap_S_RegisterSound( name, qfalse );

    Com_sprintf( name, sizeof( name ), "sound/player/footsteps/mech%i.wav", i + 1 );
    cgs.media.footsteps[ FOOTSTEP_MECH ][ i ] = trap_S_RegisterSound( name, qfalse );

    Com_sprintf( name, sizeof( name ), "sound/player/footsteps/energy%i.wav", i + 1 );
    cgs.media.footsteps[ FOOTSTEP_ENERGY ][ i ] = trap_S_RegisterSound( name, qfalse );

    Com_sprintf( name, sizeof( name ), "sound/player/footsteps/splash%i.wav", i + 1 );
    cgs.media.footsteps[ FOOTSTEP_SPLASH ][ i ] = trap_S_RegisterSound( name, qfalse );

    Com_sprintf( name, sizeof( name ), "sound/player/footsteps/clank%i.wav", i + 1 );
    cgs.media.footsteps[ FOOTSTEP_METAL ][ i ] = trap_S_RegisterSound( name, qfalse );
  }

  for( i = 1 ; i < MAX_SOUNDS ; i++ )
  {
    soundName = CG_ConfigString( CS_SOUNDS + i );
    
    if( !soundName[ 0 ] )
      break;
    
    if( soundName[ 0 ] == '*' )
      continue; // custom sound

    cgs.gameSounds[ i ] = trap_S_RegisterSound( soundName, qfalse );
  }

  //TA:
  cgs.media.jetpackDescendSound     = trap_S_RegisterSound( "sound/upgrades/jetpack/low.wav", qfalse );
  cgs.media.jetpackIdleSound        = trap_S_RegisterSound( "sound/upgrades/jetpack/idle.wav", qfalse );
  cgs.media.jetpackAscendSound      = trap_S_RegisterSound( "sound/upgrades/jetpack/hi.wav", qfalse );

  cgs.media.humanBuildableExpl      = trap_S_RegisterSound( "sound/weapons/rocket/rocklx1a.wav", qfalse );
  
  // FIXME: only needed with item
  cgs.media.sfx_ric1                = trap_S_RegisterSound( "sound/weapons/machinegun/ric1.wav", qfalse );
  cgs.media.sfx_ric2                = trap_S_RegisterSound( "sound/weapons/machinegun/ric2.wav", qfalse );
  cgs.media.sfx_ric3                = trap_S_RegisterSound( "sound/weapons/machinegun/ric3.wav", qfalse );
  cgs.media.sfx_plasmaexp           = trap_S_RegisterSound( "sound/weapons/plasma/plasmx1a.wav", qfalse );
  cgs.media.sfx_flamerexp           = trap_S_RegisterSound( "sound/weapons/flamer/fireimpc.wav", qfalse );

  cgs.media.hgrenb1aSound           = trap_S_RegisterSound( "sound/weapons/grenade/hgrenb1a.wav", qfalse );
  cgs.media.hgrenb2aSound           = trap_S_RegisterSound( "sound/weapons/grenade/hgrenb2a.wav", qfalse );
  
  cgs.media.repeaterUseSound        = trap_S_RegisterSound( "sound/buildables/repeater/use.wav", qfalse );
  
  cgs.media.buildableRepairSound    = trap_S_RegisterSound( "sound/buildables/human/repair.wav", qfalse );
  cgs.media.buildableRepairedSound  = trap_S_RegisterSound( "sound/buildables/human/repaired.wav", qfalse );
}


//===================================================================================


/*
=================
CG_RegisterGraphics

This function may execute for a couple of minutes with a slow disk.
=================
*/
static void CG_RegisterGraphics( void )
{
  int         i;
  static char *sb_nums[ 11 ] =
  {
    "gfx/2d/numbers/zero_32b",
    "gfx/2d/numbers/one_32b",
    "gfx/2d/numbers/two_32b",
    "gfx/2d/numbers/three_32b",
    "gfx/2d/numbers/four_32b",
    "gfx/2d/numbers/five_32b",
    "gfx/2d/numbers/six_32b",
    "gfx/2d/numbers/seven_32b",
    "gfx/2d/numbers/eight_32b",
    "gfx/2d/numbers/nine_32b",
    "gfx/2d/numbers/minus_32b",
  };

  // clear any references to old media
  memset( &cg.refdef, 0, sizeof( cg.refdef ) );
  trap_R_ClearScene( );

  trap_R_LoadWorldMap( cgs.mapname );
  CG_UpdateMediaFraction( 0.66f );
    
  for( i = 0; i < 11; i++ )
    cgs.media.numberShaders[ i ] = trap_R_RegisterShader( sb_nums[ i ] );

  cgs.media.viewBloodShader           = trap_R_RegisterShader( "viewBloodBlend" );

  cgs.media.deferShader               = trap_R_RegisterShaderNoMip( "gfx/2d/defer.tga" );

  cgs.media.smokePuffShader           = trap_R_RegisterShader( "smokePuff" );
  cgs.media.smokePuffRageProShader    = trap_R_RegisterShader( "smokePuffRagePro" );
  cgs.media.shotgunSmokePuffShader    = trap_R_RegisterShader( "shotgunSmokePuff" );
  cgs.media.blasterShader             = trap_R_RegisterShader( "sprites/plasma1" );
  cgs.media.plasmaBallShader          = trap_R_RegisterShader( "sprites/plasma1" );
  cgs.media.bloodTrailShader          = trap_R_RegisterShader( "bloodTrail" );
  cgs.media.connectionShader          = trap_R_RegisterShader( "disconnected" );

  //TA: extra stuff
  cgs.media.explosionShader           = trap_R_RegisterShader( "grenadeExplosion" );
  cgs.media.greenBloodTrailShader     = trap_R_RegisterShader( "greenBloodTrail" );
  cgs.media.greenBloodExplosionShader = trap_R_RegisterShader( "greenBloodExplosion" );
  cgs.media.greenBloodMarkShader      = trap_R_RegisterShader( "greenBloodMark" );
  cgs.media.explosionTrailShader      = trap_R_RegisterShader( "explosionTrail" );
  cgs.media.poisonCloudShader         = trap_R_RegisterShader( "poisonCloud" );
  
  for( i = 0; i < 32; i++ )
    cgs.media.flameShader[ i ] = trap_R_RegisterShader( va( "fireball%d", i + 1 ) );
  
  cgs.media.scannerBlipShader         = trap_R_RegisterShader( "gfx/2d/blip" );
  cgs.media.scannerLineShader         = trap_R_RegisterShader( "white" );
  /*cgs.media.scannerShader = trap_R_RegisterShader( "gfx/2d/scanner" );*/
  
  cgs.media.waterBubbleShader         = trap_R_RegisterShader( "waterBubble" );

  cgs.media.tracerShader              = trap_R_RegisterShader( "gfx/misc/tracer" );
  cgs.media.selectShader              = trap_R_RegisterShader( "gfx/2d/select" );

  cgs.media.backTileShader            = trap_R_RegisterShader( "gfx/2d/backtile" );
  cgs.media.noammoShader              = trap_R_RegisterShader( "icons/noammo" );
  cgs.media.friendShader              = trap_R_RegisterShader( "sprites/foe" );
   

  //TA: building shaders
  cgs.media.greenBuildShader          = trap_R_RegisterShader("gfx/2d/greenbuild" );
  cgs.media.redBuildShader            = trap_R_RegisterShader("gfx/2d/redbuild" );
  cgs.media.noPowerShader             = trap_R_RegisterShader("gfx/2d/nopower" );
  cgs.media.humanSpawningShader       = trap_R_RegisterShader("models/buildables/replicator/rep_cyl" );

  cgs.media.machinegunBrassModel      = trap_R_RegisterModel( "models/weapons2/shells/m_shell.md3" );
  cgs.media.shotgunBrassModel         = trap_R_RegisterModel( "models/weapons2/shells/s_shell.md3" );

  cgs.media.gibAbdomen                = trap_R_RegisterModel( "models/gibs/abdomen.md3" );
  cgs.media.gibArm                    = trap_R_RegisterModel( "models/gibs/arm.md3" );
  cgs.media.gibChest                  = trap_R_RegisterModel( "models/gibs/chest.md3" );
  cgs.media.gibFist                   = trap_R_RegisterModel( "models/gibs/fist.md3" );
  cgs.media.gibFoot                   = trap_R_RegisterModel( "models/gibs/foot.md3" );
  cgs.media.gibForearm                = trap_R_RegisterModel( "models/gibs/forearm.md3" );
  cgs.media.gibIntestine              = trap_R_RegisterModel( "models/gibs/intestine.md3" );
  cgs.media.gibLeg                    = trap_R_RegisterModel( "models/gibs/leg.md3" );
  cgs.media.gibSkull                  = trap_R_RegisterModel( "models/gibs/skull.md3" );
  cgs.media.gibBrain                  = trap_R_RegisterModel( "models/gibs/brain.md3" );

  cgs.media.metalGib1                 = trap_R_RegisterModel( "models/fx/metal_gibs/m_gib1.md3" );
  cgs.media.metalGib2                 = trap_R_RegisterModel( "models/fx/metal_gibs/m_gib2.md3" );
  cgs.media.metalGib3                 = trap_R_RegisterModel( "models/fx/metal_gibs/m_gib3.md3" );
  cgs.media.metalGib4                 = trap_R_RegisterModel( "models/fx/metal_gibs/m_gib4.md3" );
  cgs.media.metalGib5                 = trap_R_RegisterModel( "models/fx/metal_gibs/m_gib5.md3" );
  cgs.media.metalGib6                 = trap_R_RegisterModel( "models/fx/metal_gibs/m_gib6.md3" );
  cgs.media.metalGib7                 = trap_R_RegisterModel( "models/fx/metal_gibs/m_gib7.md3" );
  cgs.media.metalGib8                 = trap_R_RegisterModel( "models/fx/metal_gibs/m_gib8.md3" );
  
  cgs.media.gibSpark1                 = trap_R_RegisterShader( "models/fx/metal_gibs/spark.jpg" );
  cgs.media.gibSpark2                 = trap_R_RegisterShader( "models/fx/metal_gibs/spark2.jpg" );
  
  cgs.media.smoke2                    = trap_R_RegisterModel( "models/weapons2/shells/s_shell.md3" );

  cgs.media.balloonShader             = trap_R_RegisterShader( "sprites/balloon3" );

  cgs.media.bloodExplosionShader      = trap_R_RegisterShader( "bloodExplosion" );

  cgs.media.bulletFlashModel          = trap_R_RegisterModel( "models/weaphits/bullet.md3" );
  cgs.media.ringFlashModel            = trap_R_RegisterModel( "models/weaphits/ring02.md3" );
  cgs.media.dishFlashModel            = trap_R_RegisterModel( "models/weaphits/boom01.md3" );
  cgs.media.teleportEffectModel       = trap_R_RegisterModel( "models/misc/telep.md3" );
  cgs.media.teleportEffectShader      = trap_R_RegisterShader( "teleportEffect" );

  CG_UpdateMediaFraction( 0.7f );

  memset( cg_weapons, 0, sizeof( cg_weapons ) );
  memset( cg_upgrades, 0, sizeof( cg_upgrades ) );

  // wall marks
  cgs.media.bulletMarkShader          = trap_R_RegisterShader( "gfx/damage/bullet_mrk" );
  cgs.media.burnMarkShader            = trap_R_RegisterShader( "gfx/damage/burn_med_mrk" );
  cgs.media.holeMarkShader            = trap_R_RegisterShader( "gfx/damage/hole_lg_mrk" );
  cgs.media.energyMarkShader          = trap_R_RegisterShader( "gfx/damage/plasma_mrk" );
  cgs.media.shadowMarkShader          = trap_R_RegisterShader( "markShadow" );
  cgs.media.wakeMarkShader            = trap_R_RegisterShader( "wake" );
  cgs.media.bloodMarkShader           = trap_R_RegisterShader( "bloodMark" );

  // register the inline models
  cgs.numInlineModels = trap_CM_NumInlineModels( );
  
  for( i = 1; i < cgs.numInlineModels; i++ )
  {
    char    name[ 10 ];
    vec3_t  mins, maxs;
    int     j;

    Com_sprintf( name, sizeof( name ), "*%i", i );
    
    cgs.inlineDrawModel[ i ] = trap_R_RegisterModel( name );
    trap_R_ModelBounds( cgs.inlineDrawModel[ i ], mins, maxs );
    
    for( j = 0 ; j < 3 ; j++ )
      cgs.inlineModelMidpoints[ i ][ j ] = mins[ j ] + 0.5 * ( maxs[ j ] - mins[ j ] );
  }

  // register all the server specified models
  for( i = 1; i < MAX_MODELS; i++ )
  {
    const char *modelName;

    modelName = CG_ConfigString( CS_MODELS+i );
    
    if( !modelName[ 0 ] )
      break;

    cgs.gameModels[ i ] = trap_R_RegisterModel( modelName );
  }

  CG_UpdateMediaFraction( 0.8f );

  // register all the server specified shaders
  for( i = 1; i < MAX_SHADERS; i++ )
  {
    const char *shaderName;

    shaderName = CG_ConfigString( CS_SHADERS+i );
    
    if( !shaderName[ 0 ] )
      break;

    cgs.gameShaders[ i ] = trap_R_RegisterShader( shaderName );
  }
}


/*                                                                      
=======================
CG_BuildSpectatorString

=======================
*/
void CG_BuildSpectatorString( void )
{
  int i;
  
  cg.spectatorList[ 0 ] = 0;
  
  for( i = 0; i < MAX_CLIENTS; i++ )
  {
    if( cgs.clientinfo[ i ].infoValid && cgs.clientinfo[ i ].team == PTE_NONE )
      Q_strcat( cg.spectatorList, sizeof( cg.spectatorList ), va( "%s     ", cgs.clientinfo[ i ].name ) );
  }
  
  i = strlen( cg.spectatorList );
  
  if( i != cg.spectatorLen )
  {
    cg.spectatorLen = i;
    cg.spectatorWidth = -1;
  }
}



/*
===================
CG_RegisterClients

===================
*/
static void CG_RegisterClients( void )
{
  char  buffer[ MAX_INFO_STRING ];
  char  *s;
  int   i;

  cg.charModelFraction = 0.0f;
  
  //precache all the models/sounds/etc
  for( i = PCL_NONE + 1; i < PCL_NUM_CLASSES;  i++ )
  {
    CG_PrecacheClientInfo( i, BG_FindModelNameForClass( i ),
                              BG_FindSkinNameForClass( i ) );
    
    cg.charModelFraction = (float)i / (float)PCL_NUM_CLASSES;
    trap_UpdateScreen( );
  }

  cgs.media.helmetModel        = trap_R_RegisterModel( "models/players/human_base/helmet.md3" );
  cgs.media.helmetSkin         = trap_R_RegisterSkin( "models/players/human_base/helmet_default.skin" );

  cgs.media.larmourLegsSkin    = trap_R_RegisterSkin( "models/players/sarge/lower_red.skin" );
  cgs.media.larmourTorsoSkin   = trap_R_RegisterSkin( "models/players/sarge/upper_red.skin" );
  
  cgs.media.jetpackModel       = trap_R_RegisterModel( "models/players/human_base/jetpack.md3" );
  cgs.media.jetpackFlashModel  = trap_R_RegisterModel( "models/players/human_base/jetpack_flash.md3" );
  
  cg.charModelFraction = 1.0f;
  trap_UpdateScreen( );
  
  //load all the clientinfos of clients already connected to the server
  for( i = 0; i < MAX_CLIENTS; i++ )
  {
    const char  *clientInfo;

    clientInfo = CG_ConfigString( CS_PLAYERS + i );
    if( !clientInfo[ 0 ] )
      continue;

    CG_NewClientInfo( i );
  }

  CG_BuildSpectatorString( );
}

//===========================================================================

/*
=================
CG_ConfigString
=================
*/
const char *CG_ConfigString( int index )
{
  if( index < 0 || index >= MAX_CONFIGSTRINGS )
    CG_Error( "CG_ConfigString: bad index: %i", index );

  return cgs.gameState.stringData + cgs.gameState.stringOffsets[ index ];
}

//==================================================================

/*
======================
CG_StartMusic

======================
*/
void CG_StartMusic( void )
{
  char  *s;
  char  parm1[ MAX_QPATH ], parm2[ MAX_QPATH ];

  // start the background music
  s = (char *)CG_ConfigString( CS_MUSIC );
  Q_strncpyz( parm1, COM_Parse( &s ), sizeof( parm1 ) );
  Q_strncpyz( parm2, COM_Parse( &s ), sizeof( parm2 ) );

  trap_S_StartBackgroundTrack( parm1, parm2 );
}

//TA: FIXME: preliminary integration of CG TA UI stuff
//
// ==============================
// new hud stuff ( mission pack )
// ==============================
//
char *CG_GetMenuBuffer( const char *filename )
{
  int           len;
  fileHandle_t  f;
  static char   buf[ MAX_MENUFILE ];

  len = trap_FS_FOpenFile( filename, &f, FS_READ );
  
  if( !f )
  {
    trap_Print( va( S_COLOR_RED "menu file not found: %s, using default\n", filename ) );
    return NULL;
  }
  
  if( len >= MAX_MENUFILE )
  {
    trap_Print( va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i",
                    filename, len, MAX_MENUFILE ) );
    trap_FS_FCloseFile( f );
    return NULL;
  }

  trap_FS_Read( buf, len, f );
  buf[len] = 0;
  trap_FS_FCloseFile( f );

  return buf;
}

qboolean CG_Asset_Parse( int handle )
{
  pc_token_t token;
  const char *tempStr;

  if( !trap_PC_ReadToken( handle, &token ) )
    return qfalse;
  
  if( Q_stricmp( token.string, "{" ) != 0 )
    return qfalse;
    
  while( 1 )
  {
    if( !trap_PC_ReadToken( handle, &token ) )
      return qfalse;

    if( Q_stricmp( token.string, "}" ) == 0 )
      return qtrue;

    // font
    if( Q_stricmp( token.string, "font" ) == 0 )
    {
      int pointSize;
      
      if( !PC_String_Parse( handle, &tempStr ) || !PC_Int_Parse( handle, &pointSize ) )
        return qfalse;
      
      cgDC.registerFont( tempStr, pointSize, &cgDC.Assets.textFont );
      continue;
    }

    // smallFont
    if( Q_stricmp( token.string, "smallFont" ) == 0 )
    {
      int pointSize;
      
      if( !PC_String_Parse( handle, &tempStr ) || !PC_Int_Parse( handle, &pointSize ) )
        return qfalse;
      
      cgDC.registerFont( tempStr, pointSize, &cgDC.Assets.smallFont );
      continue;
    }

    // font
    if( Q_stricmp( token.string, "bigfont" ) == 0 )
    {
      int pointSize;
      
      if( !PC_String_Parse( handle, &tempStr ) || !PC_Int_Parse( handle, &pointSize ) )
        return qfalse;

      cgDC.registerFont( tempStr, pointSize, &cgDC.Assets.bigFont );
      continue;
    }

    // gradientbar
    if( Q_stricmp( token.string, "gradientbar" ) == 0 )
    {
      if( !PC_String_Parse( handle, &tempStr ) )
        return qfalse;

      cgDC.Assets.gradientBar = trap_R_RegisterShaderNoMip( tempStr );
      continue;
    }

    // enterMenuSound
    if( Q_stricmp( token.string, "menuEnterSound" ) == 0 )
    {
      if( !PC_String_Parse( handle, &tempStr ) )
        return qfalse;

      cgDC.Assets.menuEnterSound = trap_S_RegisterSound( tempStr, qfalse );
      continue;
    }

    // exitMenuSound
    if( Q_stricmp( token.string, "menuExitSound" ) == 0 )
    {
      if( !PC_String_Parse( handle, &tempStr ) )
        return qfalse;

      cgDC.Assets.menuExitSound = trap_S_RegisterSound( tempStr, qfalse );
      continue;
    }

    // itemFocusSound
    if( Q_stricmp( token.string, "itemFocusSound" ) == 0 )
    {
      if( !PC_String_Parse( handle, &tempStr ) )
        return qfalse;
      
      cgDC.Assets.itemFocusSound = trap_S_RegisterSound( tempStr, qfalse );
      continue;
    }

    // menuBuzzSound
    if( Q_stricmp( token.string, "menuBuzzSound" ) == 0 )
    {
      if( !PC_String_Parse( handle, &tempStr ) )
        return qfalse;

      cgDC.Assets.menuBuzzSound = trap_S_RegisterSound( tempStr, qfalse );
      continue;
    }

    if( Q_stricmp( token.string, "cursor" ) == 0 )
    {
      if( !PC_String_Parse( handle, &cgDC.Assets.cursorStr ) )
        return qfalse;

      cgDC.Assets.cursor = trap_R_RegisterShaderNoMip( cgDC.Assets.cursorStr );
      continue;
    }

    if( Q_stricmp( token.string, "fadeClamp" ) == 0 )
    {
      if( !PC_Float_Parse( handle, &cgDC.Assets.fadeClamp ) )
        return qfalse;

      continue;
    }

    if( Q_stricmp( token.string, "fadeCycle" ) == 0 )
    {
      if( !PC_Int_Parse( handle, &cgDC.Assets.fadeCycle ) )
        return qfalse;

      continue;
    }

    if( Q_stricmp( token.string, "fadeAmount" ) == 0 )
    {
      if( !PC_Float_Parse( handle, &cgDC.Assets.fadeAmount ) )
        return qfalse;

      continue;
    }

    if( Q_stricmp( token.string, "shadowX" ) == 0 )
    {
      if( !PC_Float_Parse( handle, &cgDC.Assets.shadowX ) )
        return qfalse;
      
      continue;
    }

    if( Q_stricmp( token.string, "shadowY" ) == 0 )
    {
      if( !PC_Float_Parse( handle, &cgDC.Assets.shadowY ) )
        return qfalse;

      continue;
    }

    if( Q_stricmp( token.string, "shadowColor" ) == 0 )
    {
      if( !PC_Color_Parse( handle, &cgDC.Assets.shadowColor ) )
        return qfalse;

      cgDC.Assets.shadowFadeClamp = cgDC.Assets.shadowColor[ 3 ];
      continue;
    }
  }
  
  return qfalse; // bk001204 - why not?
}

void CG_ParseMenu( const char *menuFile )
{
  pc_token_t  token;
  int         handle;

  handle = trap_PC_LoadSource( menuFile );
  
  if( !handle )
    handle = trap_PC_LoadSource( "ui/testhud.menu" );
  
  if( !handle )
    return;

  while( 1 )
  {
    if( !trap_PC_ReadToken( handle, &token ) )
      break;

    //if ( Q_stricmp( token, "{" ) ) {
    //  Com_Printf( "Missing { in menu file\n" );
    //  break;
    //}

    //if ( menuCount == MAX_MENUS ) {
    //  Com_Printf( "Too many menus!\n" );
    //  break;
    //}

    if( token.string[ 0 ] == '}' )
      break;

    if( Q_stricmp( token.string, "assetGlobalDef" ) == 0 )
    {
      if( CG_Asset_Parse( handle ) )
        continue;
      else
        break; 
    }


    if( Q_stricmp( token.string, "menudef" ) == 0 )
    {
      // start a new menu
      Menu_New( handle );
    }
  }
  
  trap_PC_FreeSource( handle );
}

qboolean CG_Load_Menu( char **p )
{
  char *token;

  token = COM_ParseExt( p, qtrue );

  if( token[ 0 ] != '{' )
    return qfalse;

  while( 1 )
  {
    token = COM_ParseExt( p, qtrue );
    
    if( Q_stricmp( token, "}" ) == 0 )
      return qtrue;

    if( !token || token[ 0 ] == 0 )
      return qfalse;

    CG_ParseMenu( token ); 
  }
  return qfalse;
}



void CG_LoadMenus( const char *menuFile )
{
  char          *token;
  char          *p;
  int           len, start;
  fileHandle_t  f;
  static char   buf[ MAX_MENUDEFFILE ];

  start = trap_Milliseconds( );

  len = trap_FS_FOpenFile( menuFile, &f, FS_READ );
  
  if( !f )
  {
    trap_Error( va( S_COLOR_YELLOW "menu file not found: %s, using default\n", menuFile ) );
    len = trap_FS_FOpenFile( "ui/hud.txt", &f, FS_READ );
    
    if( !f )
      trap_Error( va( S_COLOR_RED "default menu file not found: ui/hud.txt, unable to continue!\n", menuFile ) );
  }

  if( len >= MAX_MENUDEFFILE )
  {
    trap_Error( va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i",
                menuFile, len, MAX_MENUDEFFILE ) );
    trap_FS_FCloseFile( f );
    return;
  }

  trap_FS_Read( buf, len, f );
  buf[ len ] = 0;
  trap_FS_FCloseFile( f );
  
  COM_Compress( buf );

  Menu_Reset( );

  p = buf;

  while( 1 )
  {
    token = COM_ParseExt( &p, qtrue );
    
    if( !token || token[ 0 ] == 0 || token[ 0 ] == '}' )
      break;

    if( Q_stricmp( token, "}" ) == 0 )
      break;

    if( Q_stricmp( token, "loadmenu" ) == 0 )
    {
      if( CG_Load_Menu( &p ) )
        continue;
      else
        break;
    }
  }

  Com_Printf( "UI menu load time = %d milli seconds\n", trap_Milliseconds( ) - start );
}



static qboolean CG_OwnerDrawHandleKey( int ownerDraw, int flags, float *special, int key )
{
  return qfalse;
}


static int CG_FeederCount( float feederID )
{
  int i, count = 0;
  
  if( feederID == FEEDER_ALIENTEAM_LIST )
  {
    for( i = 0; i < cg.numScores; i++ )
    {
      if( cg.scores[ i ].team == PTE_ALIENS )
        count++;
    }
  }
  else if( feederID == FEEDER_HUMANTEAM_LIST )
  {
    for( i = 0; i < cg.numScores; i++ )
    {
      if( cg.scores[ i ].team == PTE_HUMANS )
        count++;
    }
  }
  
  return count;
}


void CG_SetScoreSelection( void *p )
{
  menuDef_t     *menu = (menuDef_t*)p;
  playerState_t *ps = &cg.snap->ps;
  int           i, alien, human;
  int           feeder;
  
  alien = human = 0;
  
  for( i = 0; i < cg.numScores; i++ )
  {
    if( cg.scores[ i ].team == PTE_ALIENS )
      alien++;
    else if( cg.scores[ i ].team == PTE_HUMANS )
      human++;
    
    if( ps->clientNum == cg.scores[ i ].client )
      cg.selectedScore = i;
  }

  if( menu == NULL )
    // just interested in setting the selected score
    return;

  feeder = FEEDER_ALIENTEAM_LIST;
  i = alien;
  
  if( cg.scores[ cg.selectedScore ].team == PTE_HUMANS )
  {
    feeder = FEEDER_HUMANTEAM_LIST;
    i = human;
  }
  
  Menu_SetFeederSelection(menu, feeder, i, NULL);
}

// FIXME: might need to cache this info
static clientInfo_t * CG_InfoFromScoreIndex( int index, int team, int *scoreIndex )
{
  int i, count;
  count = 0;
  
  for( i = 0; i < cg.numScores; i++ )
  {
    if( cg.scores[ i ].team == team )
    {
      if( count == index )
      {
        *scoreIndex = i;
        return &cgs.clientinfo[ cg.scores[ i ].client ];
      }
      count++;
    }
  }
  
  *scoreIndex = index;
  return &cgs.clientinfo[ cg.scores[ index ].client ];
}

static const char *CG_FeederItemText( float feederID, int index, int column, qhandle_t *handle )
{
  gitem_t *item;
  int scoreIndex = 0;
  clientInfo_t *info = NULL;
  int team = -1;
  score_t *sp = NULL;

  *handle = -1;

  if( feederID == FEEDER_ALIENTEAM_LIST )
    team = PTE_ALIENS;
  else if( feederID == FEEDER_HUMANTEAM_LIST )
    team = PTE_HUMANS;

  info = CG_InfoFromScoreIndex( index, team, &scoreIndex );
  sp = &cg.scores[ scoreIndex ];

  if( info && info->infoValid )
  {
    switch( column )
    {
      case 0:
        if( atoi( CG_ConfigString( CS_CLIENTS_READY ) ) & ( 1 << sp->client ) )
          return "Ready";
        break;
        
      case 1:
        return info->name;
        break;
      
      case 2:
        return va( "%d", info->score );
        break;
        
      case 3:
        return va( "%4d", sp->time );
        break;
        
      case 4:
        if( sp->ping == -1 )
          return "connecting";
        
        return va( "%4d", sp->ping );
        break;
    }
  }

  return "";
}

static qhandle_t CG_FeederItemImage( float feederID, int index )
{
  return 0;
}

static void CG_FeederSelection( float feederID, int index )
{
  int i, count;
  int team = ( feederID == FEEDER_ALIENTEAM_LIST ) ? PTE_ALIENS : PTE_HUMANS;
  count = 0;
  
  for( i = 0; i < cg.numScores; i++ )
  {
    if( cg.scores[ i ].team == team )
    {
      if( index == count )
        cg.selectedScore = i;

      count++;
    }
  }
}

static float CG_Cvar_Get( const char *cvar )
{
  char buff[ 128 ];
  
  memset( buff, 0, sizeof( buff ) );
  trap_Cvar_VariableStringBuffer( cvar, buff, sizeof( buff ) );
  return atof( buff );
}

void CG_Text_PaintWithCursor( float x, float y, float scale, vec4_t color, const char *text,
                              int cursorPos, char cursor, int limit, int style )
{
  CG_Text_Paint( x, y, scale, color, text, 0, limit, style );
}

static int CG_OwnerDrawWidth( int ownerDraw, float scale ) 
{
  switch( ownerDraw )
  {
    case CG_KILLER:
      return CG_Text_Width( CG_GetKillerText( ), scale, 0 );
      break;
  }
  
  return 0;
}

static int CG_PlayCinematic( const char *name, float x, float y, float w, float h )
{
  return trap_CIN_PlayCinematic( name, x, y, w, h, CIN_loop );
}

static void CG_StopCinematic( int handle )
{
  trap_CIN_StopCinematic( handle );
}

static void CG_DrawCinematic( int handle, float x, float y, float w, float h )
{
  trap_CIN_SetExtents( handle, x, y, w, h );
  trap_CIN_DrawCinematic( handle );
}

static void CG_RunCinematicFrame( int handle )
{
  trap_CIN_RunCinematic( handle );
}

//TA: hack to prevent warning
static qboolean CG_OwnerDrawVisible( int parameter )
{
  return qfalse;
}

/*
=================
CG_LoadHudMenu
=================
*/
void CG_LoadHudMenu( )
{
  char        buff[ 1024 ];
  const char  *hudSet;

  cgDC.registerShaderNoMip  = &trap_R_RegisterShaderNoMip;
  cgDC.setColor             = &trap_R_SetColor;
  cgDC.drawHandlePic        = &CG_DrawPic;
  cgDC.drawStretchPic       = &trap_R_DrawStretchPic;
  cgDC.drawText             = &CG_Text_Paint;
  cgDC.textWidth            = &CG_Text_Width;
  cgDC.textHeight           = &CG_Text_Height;
  cgDC.registerModel        = &trap_R_RegisterModel;
  cgDC.modelBounds          = &trap_R_ModelBounds;
  cgDC.fillRect             = &CG_FillRect;
  cgDC.drawRect             = &CG_DrawRect;   
  cgDC.drawSides            = &CG_DrawSides;
  cgDC.drawTopBottom        = &CG_DrawTopBottom;
  cgDC.clearScene           = &trap_R_ClearScene;
  cgDC.addRefEntityToScene  = &trap_R_AddRefEntityToScene;
  cgDC.renderScene          = &trap_R_RenderScene;
  cgDC.registerFont         = &trap_R_RegisterFont;
  cgDC.ownerDrawItem        = &CG_OwnerDraw;
  cgDC.getValue             = &CG_GetValue;
  cgDC.ownerDrawVisible     = &CG_OwnerDrawVisible;
  cgDC.runScript            = &CG_RunMenuScript;
  cgDC.getTeamColor         = &CG_GetTeamColor;
  cgDC.setCVar              = trap_Cvar_Set;
  cgDC.getCVarString        = trap_Cvar_VariableStringBuffer;
  cgDC.getCVarValue         = CG_Cvar_Get;
  cgDC.drawTextWithCursor   = &CG_Text_PaintWithCursor;
  //cgDC.setOverstrikeMode    = &trap_Key_SetOverstrikeMode;
  //cgDC.getOverstrikeMode    = &trap_Key_GetOverstrikeMode;
  cgDC.startLocalSound      = &trap_S_StartLocalSound;
  cgDC.ownerDrawHandleKey   = &CG_OwnerDrawHandleKey;
  cgDC.feederCount          = &CG_FeederCount;
  cgDC.feederItemImage      = &CG_FeederItemImage;
  cgDC.feederItemText       = &CG_FeederItemText;
  cgDC.feederSelection      = &CG_FeederSelection;
  //cgDC.setBinding           = &trap_Key_SetBinding;
  //cgDC.getBindingBuf        = &trap_Key_GetBindingBuf;
  //cgDC.keynumToStringBuf    = &trap_Key_KeynumToStringBuf;
  //cgDC.executeText          = &trap_Cmd_ExecuteText;
  cgDC.Error                = &Com_Error; 
  cgDC.Print                = &Com_Printf; 
  cgDC.ownerDrawWidth       = &CG_OwnerDrawWidth;
  //cgDC.Pause                = &CG_Pause;
  cgDC.registerSound        = &trap_S_RegisterSound;
  cgDC.startBackgroundTrack = &trap_S_StartBackgroundTrack;
  cgDC.stopBackgroundTrack  = &trap_S_StopBackgroundTrack;
  cgDC.playCinematic        = &CG_PlayCinematic;
  cgDC.stopCinematic        = &CG_StopCinematic;
  cgDC.drawCinematic        = &CG_DrawCinematic;
  cgDC.runCinematicFrame    = &CG_RunCinematicFrame;
  
  Init_Display( &cgDC );

  Menu_Reset( );
  
  trap_Cvar_VariableStringBuffer( "cg_hudFiles", buff, sizeof( buff ) );
  hudSet = buff;
  
  if( hudSet[ 0 ] == '\0' )
    hudSet = "ui/hud.txt";

  CG_LoadMenus( hudSet );
}

void CG_AssetCache( void )
{
  cgDC.Assets.gradientBar         = trap_R_RegisterShaderNoMip( ASSET_GRADIENTBAR );
  cgDC.Assets.fxBasePic           = trap_R_RegisterShaderNoMip( ART_FX_BASE );
  cgDC.Assets.fxPic[ 0 ]          = trap_R_RegisterShaderNoMip( ART_FX_RED );
  cgDC.Assets.fxPic[ 1 ]          = trap_R_RegisterShaderNoMip( ART_FX_YELLOW );
  cgDC.Assets.fxPic[ 2 ]          = trap_R_RegisterShaderNoMip( ART_FX_GREEN );
  cgDC.Assets.fxPic[ 3 ]          = trap_R_RegisterShaderNoMip( ART_FX_TEAL );
  cgDC.Assets.fxPic[ 4 ]          = trap_R_RegisterShaderNoMip( ART_FX_BLUE );
  cgDC.Assets.fxPic[ 5 ]          = trap_R_RegisterShaderNoMip( ART_FX_CYAN );
  cgDC.Assets.fxPic[ 6 ]          = trap_R_RegisterShaderNoMip( ART_FX_WHITE );
  cgDC.Assets.scrollBar           = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR );
  cgDC.Assets.scrollBarArrowDown  = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWDOWN );
  cgDC.Assets.scrollBarArrowUp    = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWUP );
  cgDC.Assets.scrollBarArrowLeft  = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWLEFT );
  cgDC.Assets.scrollBarArrowRight = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWRIGHT );
  cgDC.Assets.scrollBarThumb      = trap_R_RegisterShaderNoMip( ASSET_SCROLL_THUMB );
  cgDC.Assets.sliderBar           = trap_R_RegisterShaderNoMip( ASSET_SLIDER_BAR );
  cgDC.Assets.sliderThumb         = trap_R_RegisterShaderNoMip( ASSET_SLIDER_THUMB );
}
//TA: FIXME: preliminary integration of CG TA UI stuff

/*
=================
CG_Init

Called after every level change or subsystem restart
Will perform callbacks to make the loading info screen update.
=================
*/
void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum )
{
  const char  *s;

  // clear everything
  memset( &cgs, 0, sizeof( cgs ) );
  memset( &cg, 0, sizeof( cg ) );
  memset( cg_entities, 0, sizeof( cg_entities ) );

  cg.clientNum = clientNum;

  cgs.processedSnapshotNum = serverMessageNum;
  cgs.serverCommandSequence = serverCommandSequence;

  // load a few needed things before we do any screen updates
  cgs.media.whiteShader     = trap_R_RegisterShader( "white" );
  cgs.media.charsetShader   = trap_R_RegisterShader( "gfx/2d/bigchars" );
  cgs.media.outlineShader   = trap_R_RegisterShader( "outline" );

  //inform UI to repress cursor whilst loading
  trap_Cvar_Set( "ui_loading", "1" );
  
  //TA: dyn memory
  CG_InitMemory( );

  CG_RegisterCvars( );

  CG_InitConsoleCommands( );

  //TA: moved up for LoadHudMenu
  String_Init( );

  //TA: FIXME: TA UI
  CG_AssetCache( );
  CG_LoadHudMenu( );      // load new hud stuff

  cg.weaponSelect = WP_NONE;

  // old servers

  // get the rendering configuration from the client system
  trap_GetGlconfig( &cgs.glconfig );
  cgs.screenXScale = cgs.glconfig.vidWidth / 640.0;
  cgs.screenYScale = cgs.glconfig.vidHeight / 480.0;

  // get the gamestate from the client system
  trap_GetGameState( &cgs.gameState );

  // check version
  s = CG_ConfigString( CS_GAME_VERSION );
  
  if( strcmp( s, GAME_VERSION ) )
    CG_Error( "Client/Server game mismatch: %s/%s", GAME_VERSION, s );

  s = CG_ConfigString( CS_LEVEL_START_TIME );
  cgs.levelStartTime = atoi( s );

  CG_ParseServerinfo( );

  // load the new map
  trap_CM_LoadMap( cgs.mapname );

  cg.loading = qtrue;   // force players to load instead of defer

  CG_RegisterSounds( );
  CG_UpdateMediaFraction( 0.33f );

  CG_RegisterGraphics( );
  CG_UpdateMediaFraction( 0.90f );
  
  CG_InitWeapons( );
  CG_UpdateMediaFraction( 0.95f );
  
  CG_InitUpgrades( );
  CG_UpdateMediaFraction( 1.0f );
  
  //TA:
  CG_InitBuildables( );

  CG_RegisterClients( );   // if low on memory, some clients will be deferred

  cg.loading = qfalse;  // future players will be deferred

  CG_InitLocalEntities( );

  CG_InitMarkPolys( );

  // remove the last loading update
  cg.infoScreenText[ 0 ] = 0;

  // Make sure we have update values (scores)
  CG_SetConfigValues( );

  CG_StartMusic( );

  CG_ShaderStateChanged( );

  trap_S_ClearLoopingSounds( qtrue );

  cg.consoleValid = qtrue;
  
  trap_Cvar_Set( "ui_loading", "0" );
}

/*
=================
CG_Shutdown

Called before every level change or subsystem restart
=================
*/
void CG_Shutdown( void )
{
  // some mods may need to do cleanup work here,
  // like closing files or archiving session data
}
