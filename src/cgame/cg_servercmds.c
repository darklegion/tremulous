// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_servercmds.c -- reliably sequenced text commands sent by the server
// these are processed at snapshot transition time, so there will definately
// be a valid snapshot this frame

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

/*
=================
CG_ParseScores

=================
*/
static void CG_ParseScores( void )
{
  int   i, powerups;

  cg.numScores = atoi( CG_Argv( 1 ) );
  
  if( cg.numScores > MAX_CLIENTS )
    cg.numScores = MAX_CLIENTS;

  cg.teamScores[ 0 ] = atoi( CG_Argv( 2 ) );
  cg.teamScores[ 1 ] = atoi( CG_Argv( 3 ) );

  memset( cg.scores, 0, sizeof( cg.scores ) );
  
  for( i = 0; i < cg.numScores; i++ )
  {
    //
    cg.scores[ i ].client = atoi( CG_Argv( i * 4 + 4 ) );
    cg.scores[ i ].score = atoi( CG_Argv( i * 4 + 5 ) );
    cg.scores[ i ].ping = atoi( CG_Argv( i * 4 + 6 ) );
    cg.scores[ i ].time = atoi( CG_Argv( i * 4 + 7 ) );

    if( cg.scores[ i ].client < 0 || cg.scores[ i ].client >= MAX_CLIENTS )
      cg.scores[ i ].client = 0;
    
    cgs.clientinfo[ cg.scores[ i ].client ].score = cg.scores[ i ].score;
    cgs.clientinfo[ cg.scores[ i ].client ].powerups = powerups;

    cg.scores[ i ].team = cgs.clientinfo[ cg.scores[ i ].client ].team;
  } 
}

/*
=================
CG_ParseTeamInfo

=================
*/
static void CG_ParseTeamInfo( void )
{
  int   i;
  int   client;

  numSortedTeamPlayers = atoi( CG_Argv( 1 ) );

  for( i = 0; i < numSortedTeamPlayers; i++ )
  {
    client = atoi( CG_Argv( i * 6 + 2 ) );

    sortedTeamPlayers[ i ] = client;

    cgs.clientinfo[ client ].location = atoi( CG_Argv( i * 6 + 3 ) );
    cgs.clientinfo[ client ].health = atoi( CG_Argv( i * 6 + 4 ) );
    cgs.clientinfo[ client ].armor = atoi( CG_Argv( i * 6 + 5 ) );
    cgs.clientinfo[ client ].curWeapon = atoi( CG_Argv( i * 6 + 6 ) );
    cgs.clientinfo[ client ].powerups = atoi( CG_Argv( i * 6 + 7 ) );
  }
}


/*
================
CG_ParseServerinfo

This is called explicitly when the gamestate is first received,
and whenever the server updates any serverinfo flagged cvars
================
*/
void CG_ParseServerinfo( void )
{
  const char  *info;
  char  *mapname;

  info = CG_ConfigString( CS_SERVERINFO );
  cgs.dmflags = atoi( Info_ValueForKey( info, "dmflags" ) );
  cgs.teamflags = atoi( Info_ValueForKey( info, "teamflags" ) );
  cgs.timelimit = atoi( Info_ValueForKey( info, "timelimit" ) );
  cgs.maxclients = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
  mapname = Info_ValueForKey( info, "mapname" );
  Com_sprintf( cgs.mapname, sizeof( cgs.mapname ), "maps/%s.bsp", mapname );
}

/*
==================
CG_ParseWarmup
==================
*/
static void CG_ParseWarmup( void )
{
  const char  *info;
  int         warmup;

  info = CG_ConfigString( CS_WARMUP );

  warmup = atoi( info );
  cg.warmupCount = -1;

  if( warmup == 0 && cg.warmup )
  {
  }

  cg.warmup = warmup;
}

/*
================
CG_SetConfigValues

Called on load to set the initial values from configure strings
================
*/
void CG_SetConfigValues( void )
{
  const char *s;

  cgs.scores1 = atoi( CG_ConfigString( CS_SCORES1 ) );
  cgs.scores2 = atoi( CG_ConfigString( CS_SCORES2 ) );

  sscanf( CG_ConfigString( CS_BUILDPOINTS ),
          "%d %d %d %d %d", &cgs.alienBuildPoints,
                            &cgs.alienBuildPointsTotal,
                            &cgs.humanBuildPoints,
                            &cgs.humanBuildPointsTotal,
                            &cgs.humanBuildPointsPowered );

  sscanf( CG_ConfigString( CS_STAGES ), "%d %d", &cgs.alienStage, &cgs.humanStage );
  
  cgs.levelStartTime = atoi( CG_ConfigString( CS_LEVEL_START_TIME ) );
  cg.warmup = atoi( CG_ConfigString( CS_WARMUP ) );
}


/*
=====================
CG_ShaderStateChanged
=====================
*/
void CG_ShaderStateChanged( void )
{
  char        originalShader[ MAX_QPATH ];
  char        newShader[ MAX_QPATH ];
  char        timeOffset[ 16 ];
  const char  *o;
  char        *n, *t;

  o = CG_ConfigString( CS_SHADERSTATE );
  
  while( o && *o )
  {
    n = strstr( o, "=" );
    
    if( n && *n )
    {
      strncpy( originalShader, o, n - o );
      originalShader[ n - o ] = 0;
      n++;
      t = strstr( n, ":" );
      
      if( t && *t )
      {
        strncpy( newShader, n, t - n );
        newShader[ t - n ] = 0;
      }
      else
        break;
      
      t++;
      o = strstr( t, "@" );
      
      if( o )
      {
        strncpy( timeOffset, t, o - t );
        timeOffset[ o - t ] = 0;
        o++;
        trap_R_RemapShader( originalShader, newShader, timeOffset );
      }
    }
    else
      break;
  }
}

/*
================
CG_AnnounceAlienStageTransistion
================
*/
static void CG_AnnounceAlienStageTransistion( stage_t from, stage_t to )
{
  if( cg.predictedPlayerState.stats[ STAT_PTEAM ] != PTE_ALIENS )
    return;

  trap_S_StartLocalSound( cgs.media.alienStageTransition, CHAN_ANNOUNCER );
  CG_CenterPrint( "We have evolved!", 200, GIANTCHAR_WIDTH * 4 );
}

/*
================
CG_AnnounceHumanStageTransistion
================
*/
static void CG_AnnounceHumanStageTransistion( stage_t from, stage_t to )
{
  if( cg.predictedPlayerState.stats[ STAT_PTEAM ] != PTE_HUMANS )
    return;

  trap_S_StartLocalSound( cgs.media.humanStageTransition, CHAN_ANNOUNCER );
  CG_CenterPrint( "Reinforcements have arrived!", 200, GIANTCHAR_WIDTH * 4 );
}

/*
================
CG_ConfigStringModified

================
*/
static void CG_ConfigStringModified( void )
{
  const char  *str;
  int         num;

  num = atoi( CG_Argv( 1 ) );

  // get the gamestate from the client system, which will have the
  // new configstring already integrated
  trap_GetGameState( &cgs.gameState );

  // look up the individual string that was modified
  str = CG_ConfigString( num );

  // do something with it if necessary
  if( num == CS_MUSIC )
    CG_StartMusic( );
  else if( num == CS_SERVERINFO )
    CG_ParseServerinfo( );
  else if( num == CS_WARMUP )
    CG_ParseWarmup( );
  else if( num == CS_SCORES1 )
    cgs.scores1 = atoi( str );
  else if( num == CS_SCORES2 )
    cgs.scores2 = atoi( str );
  else if( num == CS_BUILDPOINTS )
    sscanf( str, "%d %d %d %d %d", &cgs.alienBuildPoints,
                                   &cgs.alienBuildPointsTotal,
                                   &cgs.humanBuildPoints,
                                   &cgs.humanBuildPointsTotal,
                                   &cgs.humanBuildPointsPowered );
  else if( num == CS_STAGES )
  {
    stage_t oldAlienStage = cgs.alienStage;
    stage_t oldHumanStage = cgs.humanStage;
  
    sscanf( str, "%d %d", &cgs.alienStage, &cgs.humanStage );
    
    if( cgs.alienStage != oldAlienStage )
      CG_AnnounceAlienStageTransistion( oldAlienStage, cgs.alienStage );
    
    if( cgs.humanStage != oldHumanStage )
      CG_AnnounceHumanStageTransistion( oldHumanStage, cgs.humanStage );
  }
  else if( num == CS_LEVEL_START_TIME )
    cgs.levelStartTime = atoi( str );
  else if( num == CS_VOTE_TIME )
  {
    cgs.voteTime = atoi( str );
    cgs.voteModified = qtrue;
  }
  else if( num == CS_VOTE_YES )
  {
    cgs.voteYes = atoi( str );
    cgs.voteModified = qtrue;
  }
  else if( num == CS_VOTE_NO )
  {
    cgs.voteNo = atoi( str );
    cgs.voteModified = qtrue;
  }
  else if( num == CS_VOTE_STRING )
    Q_strncpyz( cgs.voteString, str, sizeof( cgs.voteString ) );
  else if( num >= CS_TEAMVOTE_TIME && num <= CS_TEAMVOTE_TIME + 1 )
  {
    cgs.teamVoteTime[ num - CS_TEAMVOTE_TIME ] = atoi( str );
    cgs.teamVoteModified[ num - CS_TEAMVOTE_TIME ] = qtrue;
  }
  else if( num >= CS_TEAMVOTE_YES && num <= CS_TEAMVOTE_YES + 1 )
  {
    cgs.teamVoteYes[ num - CS_TEAMVOTE_YES ] = atoi( str );
    cgs.teamVoteModified[ num - CS_TEAMVOTE_YES ] = qtrue;
  }
  else if( num >= CS_TEAMVOTE_NO && num <= CS_TEAMVOTE_NO + 1 )
  {
    cgs.teamVoteNo[ num - CS_TEAMVOTE_NO ] = atoi( str );
    cgs.teamVoteModified[ num - CS_TEAMVOTE_NO ] = qtrue;
  }
  else if( num >= CS_TEAMVOTE_STRING && num <= CS_TEAMVOTE_STRING + 1 )
    Q_strncpyz( cgs.teamVoteString[ num - CS_TEAMVOTE_STRING ], str, sizeof( cgs.teamVoteString ) );
  else if( num == CS_INTERMISSION )
    cg.intermissionStarted = atoi( str );
  else if( num >= CS_MODELS && num < CS_MODELS+MAX_MODELS )
    cgs.gameModels[ num - CS_MODELS ] = trap_R_RegisterModel( str );
  else if( num >= CS_SHADERS && num < CS_SHADERS+MAX_SHADERS )
    cgs.gameShaders[ num - CS_SHADERS ] = trap_R_RegisterShader( str );
  else if( num >= CS_SOUNDS && num < CS_SOUNDS+MAX_SOUNDS )
  {
    if( str[ 0 ] != '*' )
    {  // player specific sounds don't register here
      cgs.gameSounds[ num - CS_SOUNDS ] = trap_S_RegisterSound( str, qfalse );
    }
  }
  else if( num >= CS_PLAYERS && num < CS_PLAYERS+MAX_CLIENTS )
  {
    CG_NewClientInfo( num - CS_PLAYERS );
    CG_BuildSpectatorString( );
  }
  else if( num == CS_FLAGSTATUS )
  {
  }
  else if( num == CS_SHADERSTATE )
  {
    CG_ShaderStateChanged( );
  }
}


/*
=======================
CG_AddToTeamChat

=======================
*/
static void CG_AddToTeamChat( const char *str )
{
  int   len;
  char  *p, *ls;
  int   lastcolor;
  int   chatHeight;

  if( cg_teamChatHeight.integer < TEAMCHAT_HEIGHT )
    chatHeight = cg_teamChatHeight.integer;
  else
    chatHeight = TEAMCHAT_HEIGHT;

  if( chatHeight <= 0 || cg_teamChatTime.integer <= 0 )
  {
    // team chat disabled, dump into normal chat
    cgs.teamChatPos = cgs.teamLastChatPos = 0;
    return;
  }

  len = 0;

  p = cgs.teamChatMsgs[cgs.teamChatPos % chatHeight];
  *p = 0;

  lastcolor = '7';

  ls = NULL;
  while( *str )
  {
    if( len > TEAMCHAT_WIDTH - 1 )
    {
      if( ls )
      {
        str -= ( p - ls );
        str++;
        p -= ( p - ls );
      }
      
      *p = 0;

      cgs.teamChatMsgTimes[ cgs.teamChatPos % chatHeight ] = cg.time;

      cgs.teamChatPos++;
      p = cgs.teamChatMsgs[ cgs.teamChatPos % chatHeight ];
      *p = 0;
      *p++ = Q_COLOR_ESCAPE;
      *p++ = lastcolor;
      len = 0;
      ls = NULL;
    }

    if( Q_IsColorString( str ) )
    {
      *p++ = *str++;
      lastcolor = *str;
      *p++ = *str++;
      continue;
    }
    
    if( *str == ' ' )
      ls = p;
    
    *p++ = *str++;
    len++;
  }
  *p = 0;

  cgs.teamChatMsgTimes[ cgs.teamChatPos % chatHeight ] = cg.time;
  cgs.teamChatPos++;

  if( cgs.teamChatPos - cgs.teamLastChatPos > chatHeight )
    cgs.teamLastChatPos = cgs.teamChatPos - chatHeight;
}



/*
===============
CG_MapRestart

The server has issued a map_restart, so the next snapshot
is completely new and should not be interpolated to.

A tournement restart will clear everything, but doesn't
require a reload of all the media
===============
*/
static void CG_MapRestart( void )
{
  if( cg_showmiss.integer )
    CG_Printf( "CG_MapRestart\n" );

  CG_InitLocalEntities( );
  CG_InitMarkPolys( );

	// Ridah, trails
	CG_ClearTrails( );
	// done.

  // make sure the "3 frags left" warnings play again
  cg.fraglimitWarnings = 0;

  cg.timelimitWarnings = 0;

  cg.intermissionStarted = qfalse;

  cgs.voteTime = 0;

  cg.mapRestart = qtrue;

  CG_StartMusic( );

  trap_S_ClearLoopingSounds( qtrue );

  // we really should clear more parts of cg here and stop sounds

  // play the "fight" sound if this is a restart without warmup
  if( cg.warmup == 0 )
    CG_CenterPrint( "FIGHT!", 120, GIANTCHAR_WIDTH * 2 );

  trap_Cvar_Set( "cg_thirdPerson", "0" );
}

/*
=================
CG_RemoveChatEscapeChar
=================
*/
static void CG_RemoveChatEscapeChar( char *text )
{
  int i, l;

  l = 0;
  for( i = 0; text[ i ]; i++ )
  {
    if( text[ i ] == '\x19' )
      continue;
    
    text[ l++ ] = text[ i ];
  }

  text[ l ] = '\0';
}

/*
===============
CG_SetUIVars

Set some cvars used by the UI
===============
*/
static void CG_SetUIVars( void )
{
  int   i;
  char  carriageCvar[ MAX_TOKEN_CHARS ];

  *carriageCvar = 0;
  
  //determine what the player is carrying
  for( i = WP_NONE + 1; i < WP_NUM_WEAPONS; i++ )
  {
    if( BG_gotWeapon( i, cg.snap->ps.stats ) )
      strcat( carriageCvar, va( "W%d ", i ) );
  }
  for( i = UP_NONE + 1; i < UP_NUM_UPGRADES; i++ )
  {
    if( BG_gotItem( i, cg.snap->ps.stats ) )
      strcat( carriageCvar, va( "U%d ", i ) );
  }
  strcat( carriageCvar, "$" );
  
  trap_Cvar_Set( "ui_carriage", carriageCvar );
  
  trap_Cvar_Set( "ui_stages", va( "%d %d", cgs.alienStage, cgs.humanStage ) );
}
  

/*
==============
CG_Menu
==============
*/
void CG_Menu( int menu )
{
  CG_SetUIVars( );

  switch( menu )
  {
    case MN_TEAM:       trap_SendConsoleCommand( "menu tremulous_teamselect\n" );   break;
    case MN_A_CLASS:    trap_SendConsoleCommand( "menu tremulous_alienclass\n" );   break;
    case MN_H_SPAWN:    trap_SendConsoleCommand( "menu tremulous_humanitem\n" );    break;
    case MN_A_BUILD:    trap_SendConsoleCommand( "menu tremulous_alienbuild\n" );   break;
    case MN_H_BUILD:    trap_SendConsoleCommand( "menu tremulous_humanbuild\n" );   break;
    case MN_H_ARMOURY:  trap_SendConsoleCommand( "menu tremulous_humanarmoury\n" ); break;
    case MN_H_BANK:     trap_SendConsoleCommand( "menu tremulous_humanbank\n" );    break;
    case MN_A_OBANK:    trap_SendConsoleCommand( "menu tremulous_alienbank\n" );    break;
                        
    case MN_H_NOROOM:
      trap_Cvar_Set( "ui_dialog", "There is no room to build here. Move until the buildable turns "
                                  "translucent green indicating a valid build location." );
      trap_SendConsoleCommand( "menu tremulous_human_dialog\n" );
      break;
      
    case MN_H_NOPOWER:
      trap_Cvar_Set( "ui_dialog", "There is no power remaining. Free up power by destroying existing "
                                  "buildable objects." );
      trap_SendConsoleCommand( "menu tremulous_human_dialog\n" );
      break;
      
    case MN_H_REACTOR:
      trap_Cvar_Set( "ui_dialog", "There can only be one reactor. Destroy the existing one if you "
                                  "wish to move it." );
      trap_SendConsoleCommand( "menu tremulous_human_dialog\n" );
      break;
      
    case MN_H_REPEATER:
      trap_Cvar_Set( "ui_dialog", "There is no power here. If available, a Repeater may be used to "
                                  "transmit power to this location." );
      trap_SendConsoleCommand( "menu tremulous_human_dialog\n" );
      break;
      
    case MN_H_NODCC:
      trap_Cvar_Set( "ui_dialog", "There is no Defense Computer. A Defense Computer is needed to build "
                                  "this." );
      trap_SendConsoleCommand( "menu tremulous_human_dialog\n" );
      break;
      
    case MN_H_RPLWARN:
      trap_Cvar_Set( "ui_dialog", "WARNING: This replicator will not be powered. Build a reactor to "
                                  "prevent seeing this message again." );
      trap_SendConsoleCommand( "menu tremulous_human_dialog\n" );
      break;
      
    case MN_H_RPTWARN:
      trap_Cvar_Set( "ui_dialog", "WARNING: This repeater will not be powered as there is no parent "
                                  "reactor providing power. Build a reactor." );
      trap_SendConsoleCommand( "menu tremulous_human_dialog\n" );
      break;
      
    case MN_H_NOSLOTS:
      trap_Cvar_Set( "ui_dialog", "You have no room to carry this. Please sell any conflicting "
                                  "upgrades before purchasing this item." );
      trap_SendConsoleCommand( "menu tremulous_human_dialog\n" );
      break;
      
    case MN_H_NOFUNDS:
      trap_Cvar_Set( "ui_dialog", "Insufficient funds. You do not have enough credits to perform this "
                                  "action." );
      trap_SendConsoleCommand( "menu tremulous_human_dialog\n" );
      break;
      
    case MN_H_ITEMHELD:
      trap_Cvar_Set( "ui_dialog", "You already hold this item. It is not possible to carry multiple items "
                                  "of the same type." );
      trap_SendConsoleCommand( "menu tremulous_human_dialog\n" );
      break;
      
    case MN_A_NOROOM:
      trap_Cvar_Set( "ui_dialog", "There is no room to build here. Move until the structure turns "
                                  "translucent green indicating a valid build location." );
      trap_SendConsoleCommand( "menu tremulous_alien_dialog\n" );
      break;
      
    case MN_A_NOCREEP:
      trap_Cvar_Set( "ui_dialog", "There is no creep here. You must build near existing Eggs or "
                                  "the Overmind. Alien structures will not support themselves." );
      trap_SendConsoleCommand( "menu tremulous_alien_dialog\n" );
      break;
      
    case MN_A_NOOVMND:
      trap_Cvar_Set( "ui_dialog", "There is no Overmind. An Overmind must be built to control "
                                  "the structure you tried to place" );
      trap_SendConsoleCommand( "menu tremulous_alien_dialog\n" );
      break;
      
    case MN_A_NOEROOM:
      trap_Cvar_Set( "ui_dialog", "There is no room to evolve here. Move away from walls or other "
                                  "nearby objects and try again." );
      trap_SendConsoleCommand( "menu tremulous_alien_dialog\n" );
      break;
      
    case MN_A_OVERMIND:
      trap_Cvar_Set( "ui_dialog", "There can only be one Overmind. Destroy the existing one if you "
                                  "wish to move it." );
      trap_SendConsoleCommand( "menu tremulous_alien_dialog\n" );
      break;
      
    case MN_A_NOASSERT:
      trap_Cvar_Set( "ui_dialog", "The Overmind cannot control anymore structures. Destroy existing "
                                  "structures to build more." );
      trap_SendConsoleCommand( "menu tremulous_alien_dialog\n" );
      break;
      
    case MN_A_SPWNWARN:
      trap_Cvar_Set( "ui_dialog", "WARNING: This spawn will not be controlled by an Overmind. "
                                  "Build an Overmind to prevent seeing this message again." );
      trap_SendConsoleCommand( "menu tremulous_alien_dialog\n" );
      break;
      
    case MN_A_NORMAL:
      trap_Cvar_Set( "ui_dialog", "Cannot build on this surface. This surface is too steep or unsuitable "
                                  "to build on. Please choose another site for this structure." );
      trap_SendConsoleCommand( "menu tremulous_alien_dialog\n" );
      break;
      
    case MN_A_NOFUNDS:
      trap_Cvar_Set( "ui_dialog", "You do not possess sufficient organs to perform this action." );
      trap_SendConsoleCommand( "menu tremulous_alien_dialog\n" );
      break;
      
    case MN_A_HOVEL_OCCUPIED:
      trap_Cvar_Set( "ui_dialog", "This Hovel is occupied by another builder. Please find or build "
                                  "another." );
      trap_SendConsoleCommand( "menu tremulous_alien_dialog\n" );
      break;
      
    case MN_A_HOVEL_BLOCKED:
      trap_Cvar_Set( "ui_dialog", "The exit to this Hovel is currently blocked. Please wait until it "
                                  "becomes clear then try again." );
      trap_SendConsoleCommand( "menu tremulous_alien_dialog\n" );
      break;
      
    case MN_A_INFEST:
      trap_Cvar_Set( "ui_currentClass", va( "%d %d",  cg.snap->ps.stats[ STAT_PCLASS ],
                                                      cg.snap->ps.persistant[ PERS_CREDIT ] ) );
      trap_SendConsoleCommand( "menu tremulous_alienupgrade\n" );
      break;

    default:
      Com_Printf( "cgame: debug: no such menu %d\n", menu );
  }
}

/*
=================
CG_ServerCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
static void CG_ServerCommand( void )
{
  const char  *cmd;
  char        text[ MAX_SAY_TEXT ];

  cmd = CG_Argv( 0 );

  if( !cmd[ 0 ] )
  {
    // server claimed the command
    return;
  }

  if( !strcmp( cmd, "cp" ) )
  {
    CG_CenterPrint( CG_Argv( 1 ), SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
    return;
  }

  if( !strcmp( cmd, "cs" ) )
  {
    CG_ConfigStringModified( );
    return;
  }

  if( !strcmp( cmd, "print" ) )
  {
    CG_Printf( "%s", CG_Argv( 1 ) );
    return;
  }

  if( !strcmp( cmd, "chat" ) )
  {
    if( !cg_teamChatsOnly.integer )
    {
      trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
      Q_strncpyz( text, CG_Argv( 1 ), MAX_SAY_TEXT );
      CG_RemoveChatEscapeChar( text );
      CG_Printf( "%s\n", text );
    }
    
    return;
  }

  if( !strcmp( cmd, "tchat" ) )
  {
    trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
    Q_strncpyz( text, CG_Argv( 1 ), MAX_SAY_TEXT );
    CG_RemoveChatEscapeChar( text );
    CG_AddToTeamChat( text );
    CG_Printf( "%s\n", text );
    return;
  }
  
  if( !strcmp( cmd, "scores" ) )
  {
    CG_ParseScores( );
    return;
  }

  if( !strcmp( cmd, "tinfo" ) )
  {
    CG_ParseTeamInfo( );
    return;
  }

  if( !strcmp( cmd, "map_restart" ) )
  {
    CG_MapRestart( );
    return;
  }

  if( Q_stricmp( cmd, "remapShader" ) == 0 )
  {
    if( trap_Argc( ) == 4 )
      trap_R_RemapShader( CG_Argv( 1 ), CG_Argv( 2 ), CG_Argv( 3 ) );
  }

  // loaddeferred can be both a servercmd and a consolecmd
  if( !strcmp( cmd, "loaddefered" ) )
  {  // FIXME: spelled wrong, but not changing for demo
    CG_LoadDeferredPlayers( );
    return;
  }

  // clientLevelShot is sent before taking a special screenshot for
  // the menu system during development
  if( !strcmp( cmd, "clientLevelShot" ) )
  {
    cg.levelShot = qtrue;
    return;
  }

  //enable G_Printfs from the server to appear in the TA console
  if( !strcmp( cmd, "gprintf" ) )
  {
    if( trap_Argc( ) == 2 )
    {
      CG_TAUIConsole( CG_Argv( 1 ) );
      return;
    }
  }

  //the server has triggered a menu
  if( !strcmp( cmd, "servermenu" ) )
  {
    if( trap_Argc( ) == 2 )
    {
      CG_Menu( atoi( CG_Argv( 1 ) ) );
      return;
    }
  }
  
  CG_Printf( "Unknown client game command: %s\n", cmd );
}


/*
====================
CG_ExecuteNewServerCommands

Execute all of the server commands that were received along
with this this snapshot.
====================
*/
void CG_ExecuteNewServerCommands( int latestSequence )
{
  while( cgs.serverCommandSequence < latestSequence )
  {
    if( trap_GetServerCommand( ++cgs.serverCommandSequence ) )
      CG_ServerCommand( );
  }
}
