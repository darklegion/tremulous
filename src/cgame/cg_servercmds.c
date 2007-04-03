/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2006 Tim Angus

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

// cg_servercmds.c -- reliably sequenced text commands sent by the server
// these are processed at snapshot transition time, so there will definately
// be a valid snapshot this frame


#include "cg_local.h"

/*
=================
CG_ParseScores

=================
*/
static void CG_ParseScores( void )
{
  int   i;

  cg.numScores = atoi( CG_Argv( 1 ) );

  if( cg.numScores > MAX_CLIENTS )
    cg.numScores = MAX_CLIENTS;

  cg.teamScores[ 0 ] = atoi( CG_Argv( 2 ) );
  cg.teamScores[ 1 ] = atoi( CG_Argv( 3 ) );

  memset( cg.scores, 0, sizeof( cg.scores ) );

  if( cg_debugRandom.integer )
    CG_Printf( "cg.numScores: %d\n", cg.numScores );

  for( i = 0; i < cg.numScores; i++ )
  {
    //
    cg.scores[ i ].client = atoi( CG_Argv( i * 6 + 4 ) );
    cg.scores[ i ].score = atoi( CG_Argv( i * 6 + 5 ) );
    cg.scores[ i ].ping = atoi( CG_Argv( i * 6 + 6 ) );
    cg.scores[ i ].time = atoi( CG_Argv( i * 6 + 7 ) );
    cg.scores[ i ].weapon = atoi( CG_Argv( i * 6 + 8 ) );
    cg.scores[ i ].upgrade = atoi( CG_Argv( i * 6 + 9 ) );

    if( cg.scores[ i ].client < 0 || cg.scores[ i ].client >= MAX_CLIENTS )
      cg.scores[ i ].client = 0;

    cgs.clientinfo[ cg.scores[ i ].client ].score = cg.scores[ i ].score;
    cgs.clientinfo[ cg.scores[ i ].client ].powerups = 0;

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
  cgs.timelimit = atoi( Info_ValueForKey( info, "timelimit" ) );
  cgs.maxclients = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
  cgs.markDeconstruct = atoi( Info_ValueForKey( info, "g_markDeconstruct" ) );
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
  sscanf( CG_ConfigString( CS_BUILDPOINTS ),
          "%d %d %d %d %d", &cgs.alienBuildPoints,
                            &cgs.alienBuildPointsTotal,
                            &cgs.humanBuildPoints,
                            &cgs.humanBuildPointsTotal,
                            &cgs.humanBuildPointsPowered );

  sscanf( CG_ConfigString( CS_STAGES ), "%d %d %d %d %d %d", &cgs.alienStage, &cgs.humanStage,
      &cgs.alienKills, &cgs.humanKills, &cgs.alienNextStageThreshold, &cgs.humanNextStageThreshold );
  sscanf( CG_ConfigString( CS_SPAWNS ), "%d %d", &cgs.numAlienSpawns, &cgs.numHumanSpawns );

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

    sscanf( str, "%d %d %d %d %d %d",
        &cgs.alienStage, &cgs.humanStage,
        &cgs.alienKills, &cgs.humanKills,
        &cgs.alienNextStageThreshold, &cgs.humanNextStageThreshold );

    if( cgs.alienStage != oldAlienStage )
      CG_AnnounceAlienStageTransistion( oldAlienStage, cgs.alienStage );

    if( cgs.humanStage != oldHumanStage )
      CG_AnnounceHumanStageTransistion( oldHumanStage, cgs.humanStage );
  }
  else if( num == CS_SPAWNS )
    sscanf( str, "%d %d", &cgs.numAlienSpawns, &cgs.numHumanSpawns );
  else if( num == CS_LEVEL_START_TIME )
    cgs.levelStartTime = atoi( str );
  else if( num == CS_VOTE_TIME )
  {
    cgs.voteTime = atoi( str );
    cgs.voteModified = qtrue;

    if( cgs.voteTime )
      trap_Cvar_Set( "ui_voteActive", "1" );
    else
      trap_Cvar_Set( "ui_voteActive", "0" );
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
    int cs_offset = num - CS_TEAMVOTE_TIME;

    cgs.teamVoteTime[ cs_offset ] = atoi( str );
    cgs.teamVoteModified[ cs_offset ] = qtrue;

    if( cs_offset == 0 )
    {
      if( cgs.teamVoteTime[ cs_offset ] )
        trap_Cvar_Set( "ui_humanTeamVoteActive", "1" );
      else
        trap_Cvar_Set( "ui_humanTeamVoteActive", "0" );
    }
    else if( cs_offset == 1 )
    {
      if( cgs.teamVoteTime[ cs_offset ] )
        trap_Cvar_Set( "ui_alienTeamVoteActive", "1" );
      else
        trap_Cvar_Set( "ui_alienTeamVoteActive", "0" );
    }
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
  {
    Q_strncpyz( cgs.teamVoteString[ num - CS_TEAMVOTE_STRING ], str,
      sizeof( cgs.teamVoteString[ num - CS_TEAMVOTE_STRING ] ) );
  }
  else if( num == CS_INTERMISSION )
    cg.intermissionStarted = atoi( str );
  else if( num >= CS_MODELS && num < CS_MODELS+MAX_MODELS )
    cgs.gameModels[ num - CS_MODELS ] = trap_R_RegisterModel( str );
  else if( num >= CS_SHADERS && num < CS_SHADERS+MAX_GAME_SHADERS )
    cgs.gameShaders[ num - CS_SHADERS ] = trap_R_RegisterShader( str );
  else if( num >= CS_PARTICLE_SYSTEMS && num < CS_PARTICLE_SYSTEMS+MAX_GAME_PARTICLE_SYSTEMS )
    cgs.gameParticleSystems[ num - CS_PARTICLE_SYSTEMS ] = CG_RegisterParticleSystem( (char *)str );
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
  else if( num == CS_WINNER )
  {
    trap_Cvar_Set( "ui_winner", str );
  }
  else if( num == CS_SHADERSTATE )
  {
    CG_ShaderStateChanged( );
  }
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

  CG_InitMarkPolys( );

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
    if( BG_InventoryContainsWeapon( i, cg.snap->ps.stats ) &&
        BG_FindPurchasableForWeapon( i ) )
      strcat( carriageCvar, va( "W%d ", i ) );
  }
  for( i = UP_NONE + 1; i < UP_NUM_UPGRADES; i++ )
  {
    if( BG_InventoryContainsUpgrade( i, cg.snap->ps.stats ) &&
        BG_FindPurchasableForUpgrade( i ) )
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
  const char *cmd = NULL;	// command to send
  const char *longMsg   = NULL;	// command parameter
  const char *shortMsg  = NULL;	// non-modal version of message
  CG_SetUIVars( );

  // string literals have static storage duration, this is safe,
  // cleaner and much more readable.
  switch( menu )
  {
    case MN_TEAM:
      cmd       = "menu tremulous_teamselect\n";
      break;

    case MN_A_CLASS:
      cmd       = "menu tremulous_alienclass\n";
      break;

    case MN_H_SPAWN:
      cmd       = "menu tremulous_humanitem\n";
      break;

    case MN_A_BUILD:
      cmd       = "menu tremulous_alienbuild\n";
      break;

    case MN_H_BUILD:
      cmd       = "menu tremulous_humanbuild\n";
      break;

    case MN_H_ARMOURY:
      cmd       = "menu tremulous_humanarmoury\n";
      break;

    case MN_A_TEAMFULL:
      longMsg   = "The alien team has too many players. Please wait until slots "
                  "become available or join the human team.";
      shortMsg  = "The alien team has too many players\n";
      cmd       = "menu tremulous_alien_dialog\n";
      break;

    case MN_H_TEAMFULL:
      longMsg   = "The human team has too many players. Please wait until slots "
                  "become available or join the alien team.";
      shortMsg  = "The human team has too many players\n";
      cmd       = "menu tremulous_human_dialog\n";
      break;

    case MN_A_TEAMCHANGEBUILDTIMER:
      longMsg   = "You cannot leave the Alien team until your build timer "
                  "has expired.";
      shortMsg  = "You cannot change teams until your build timer expires.\n";
      cmd       = "menu tremulous_alien_dialog\n";
      break;

    case MN_H_TEAMCHANGEBUILDTIMER:
      longMsg   = "You cannot leave the Human team until your build timer "
                  "has expired.";
      shortMsg  = "You cannot change teams until your build timer expires.\n";
      cmd       = "menu tremulous_human_dialog\n";
      break;

    //===============================

    case MN_H_NOROOM:
      longMsg   = "There is no room to build here. Move until the buildable turns "
                  "translucent green indicating a valid build location.";
      shortMsg  = "There is no room to build here\n";
      cmd       = "menu tremulous_human_dialog\n";
      break;

    case MN_H_NOPOWER:
      longMsg   = "There is no power remaining. Free up power by destroying "
                  "existing buildable objects.";
      shortMsg  = "There is no power remaining\n";
      cmd       = "menu tremulous_human_dialog\n";
      break;

    case MN_H_NOTPOWERED:
      longMsg   = "This buildable is not powered. Build a Reactor and/or Repeater "
                  "in order to power it.";
      shortMsg  = "This buildable is not powered\n";
      cmd       = "menu tremulous_human_dialog\n";
      break;

    case MN_H_NORMAL:
      longMsg   = "Cannot build on this surface. The surface is too steep or "
                  "unsuitable to build on. Please choose another site for this "
	                "structure.";
      shortMsg  = "Cannot build on this surface\n";
      cmd       = "menu tremulous_human_dialog\n";
      break;

    case MN_H_REACTOR:
      longMsg   = "There can only be one Reactor. Destroy the existing one if you "
                  "wish to move it.";
      shortMsg  = "There can only be one Reactor\n";
      cmd       = "menu tremulous_human_dialog\n";
      break;

    case MN_H_REPEATER:
      longMsg   = "There is no power here. If available, a Repeater may be used to "
                  "transmit power to this location.";
      shortMsg  = "There is no power here\n";
      cmd       = "menu tremulous_human_dialog\n";
      break;

    case MN_H_NODCC:
      longMsg   = "There is no Defense Computer. A Defense Computer is needed to "
                  "build this.";
      shortMsg  = "There is no Defense Computer\n";
      cmd       = "menu tremulous_human_dialog\n";
      break;

    case MN_H_TNODEWARN:
      longMsg   = "WARNING: This Telenode will not be powered. Build near a power "
                  "structure to prevent seeing this message again.";
      shortMsg  = "This Telenode will not be powered\n";
      cmd       = "menu tremulous_human_dialog\n";
      break;

    case MN_H_RPTWARN:
      longMsg   = "WARNING: This Repeater will not be powered as there is no parent "
                  "Reactor providing power. Build a Reactor.";
      shortMsg  = "This Repeater will not be powered\n";
      cmd       = "menu tremulous_human_dialog\n";
      break;

    case MN_H_RPTWARN2:
      longMsg   = "This area already has power. A Repeater is not required here.";
      shortMsg  = "This area already has power\n";
      cmd       = "menu tremulous_human_dialog\n";
      break;

    case MN_H_NOSLOTS:
      longMsg   = "You have no room to carry this. Please sell any conflicting "
                  "upgrades before purchasing this item.";
      shortMsg  = "You have no room to carry this\n";
      cmd       = "menu tremulous_human_dialog\n";
      break;

    case MN_H_NOFUNDS:
      longMsg   = "Insufficient funds. You do not have enough credits to perform "
                  "this action.";
      shortMsg  = "Insufficient funds\n";
      cmd       = "menu tremulous_human_dialog\n";
      break;

    case MN_H_ITEMHELD:
      longMsg   = "You already hold this item. It is not possible to carry multiple "
                  "items of the same type.";
      shortMsg  = "You already hold this item\n";
      cmd       = "menu tremulous_human_dialog\n";
      break;

    case MN_H_NOARMOURYHERE:
      longMsg   = "You must be near a powered Armoury in order to purchase "
                  "weapons, upgrades or non-energy ammunition.";
      shortMsg  = "You must be near a powered Armoury\n";
      cmd       = "menu tremulous_human_dialog\n";
      break;

    case MN_H_NOENERGYAMMOHERE:
      longMsg   = "You must be near an Armoury, Reactor or Repeater in order "
                  "to purchase energy ammunition.";
      shortMsg  = "You must be near an Armoury, Reactor or Repeater\n";
      cmd       = "menu tremulous_human_dialog\n";
      break;

    case MN_H_NOROOMBSUITON:
      longMsg   = "There is not enough room here to put on a Battle Suit. "
                  "Make sure you have enough head room to climb in.";
      shortMsg  = "Not enough room here to put on a Battle Suit\n";
      cmd       = "menu tremulous_human_dialog\n";
      break;

    case MN_H_NOROOMBSUITOFF:
      longMsg   = "There is not enough room here to take off your Battle Suit. "
                  "Make sure you have enough head room to climb out.";
      shortMsg  = "Not enough room here to take off your Battle Suit\n";
      cmd       = "menu tremulous_human_dialog\n";
      break;

    case MN_H_ARMOURYBUILDTIMER:
      longMsg   = "You are not allowed to buy or sell weapons until your "
                  "build timer has expired.";
      shortMsg  = "You can not buy or sell weapos until your build timer "
                  "expires\n";
      cmd       = "menu tremulous_human_dialog\n";
      break;


    //===============================

    case MN_A_NOROOM:
      longMsg   = "There is no room to build here. Move until the structure turns "
                  "translucent green indicating a valid build location.";
      shortMsg  = "There is no room to build here\n";
      cmd       = "menu tremulous_alien_dialog\n";
      break;

    case MN_A_NOCREEP:
      longMsg   = "There is no creep here. You must build near existing Eggs or "
                  "the Overmind. Alien structures will not support themselves.";
      shortMsg  = "There is no creep here\n";
      cmd       = "menu tremulous_alien_dialog\n";
      break;

    case MN_A_NOOVMND:
      longMsg   = "There is no Overmind. An Overmind must be built to control "
                  "the structure you tried to place";
      shortMsg  = "There is no Overmind\n";
      cmd       = "menu tremulous_alien_dialog\n";
      break;

    case MN_A_OVERMIND:
      longMsg   = "There can only be one Overmind. Destroy the existing one if you "
                  "wish to move it.";
      shortMsg  = "There can only be one Overmind\n";
      cmd       = "menu tremulous_alien_dialog\n";
      break;

    case MN_A_HOVEL:
      longMsg   = "There can only be one Hovel. Destroy the existing one if you "
                  "wish to move it.";
      shortMsg  = "There can only be one Hovel\n";
      cmd       = "menu tremulous_alien_dialog\n";
      break;

    case MN_A_NOASSERT:
      longMsg   = "The Overmind cannot control any more structures. Destroy existing "
                  "structures to build more.";
      shortMsg  = "The Overmind cannot control any more structures\n";
      cmd       = "menu tremulous_alien_dialog\n";
      break;

    case MN_A_SPWNWARN:
      longMsg   = "WARNING: This spawn will not be controlled by an Overmind. "
                  "Build an Overmind to prevent seeing this message again.";
      shortMsg  = "This spawn will not be controlled by an Overmind\n";
      cmd       = "menu tremulous_alien_dialog\n";
      break;

    case MN_A_NORMAL:
      longMsg   = "Cannot build on this surface. This surface is too steep or "
                  "unsuitable to build on. Please choose another site for this "
	                "structure.";
      shortMsg  = "Cannot build on this surface\n";
      cmd       = "menu tremulous_alien_dialog\n";
      break;

    case MN_A_NOEROOM:
      longMsg   = "There is no room to evolve here. Move away from walls or other "
                   "nearby objects and try again.";
      cmd       = "menu tremulous_alien_dialog\n";
      shortMsg  = "There is no room to evolve here\n";
      break;

    case MN_A_TOOCLOSE:
      longMsg   = "This location is too close to the enemy to evolve. Move away "
                  "until you are no longer aware of the enemy's presence and try "
	                "again.";
      shortMsg  = "This location is too close to the enemy to evolve\n";
      cmd       = "menu tremulous_alien_dialog\n";
      break;

    case MN_A_NOOVMND_EVOLVE:
      longMsg   = "There is no Overmind. An Overmind must be built to allow "
                  "you to upgrade.";
      shortMsg  = "There is no Overmind\n";
      cmd       = "menu tremulous_alien_dialog\n";
      break;

    case MN_A_EVOLVEBUILDTIMER:
      longMsg   = "You cannot Evolve until your build timer has expired.";
      shortMsg  = "You cannot Evolve until your build timer expires\n";
      cmd       = "menu tremulous_alien_dialog\n";
      break;

    case MN_A_HOVEL_OCCUPIED:
      longMsg   = "This Hovel is already occupied by another builder.";
      shortMsg  = "This Hovel is already occupied by another builder\n";
      cmd       = "menu tremulous_alien_dialog\n";
      break;

    case MN_A_HOVEL_BLOCKED:
      longMsg   = "The exit to this Hovel is currently blocked. Please wait until it "
                  "becomes clear then try again.";
      shortMsg  = "The exit to this Hovel is currently blocked\n";
      cmd       = "menu tremulous_alien_dialog\n";
      break;

    case MN_A_HOVEL_EXIT:
      longMsg   = "The exit to this Hovel would always be blocked. Please choose "
                  "a more suitable location.";
      shortMsg  = "The exit to this Hovel would always be blocked\n";
      cmd       = "menu tremulous_alien_dialog\n";
      break;

    case MN_A_INFEST:
      trap_Cvar_Set( "ui_currentClass", va( "%d %d",  cg.snap->ps.stats[ STAT_PCLASS ],
                                                      cg.snap->ps.persistant[ PERS_CREDIT ] ) );
      cmd       = "menu tremulous_alienupgrade\n";
      break;

    default:
      Com_Printf( "cgame: debug: no such menu %d\n", menu );
  }

	if( !cg_disableWarningDialogs.integer || !shortMsg )
  {
		// Player either wants dialog window or there's no short message
		if( cmd )
    {
			if( longMsg )
				trap_Cvar_Set( "ui_dialog", longMsg );

			trap_SendConsoleCommand( cmd );
		}
	}
  else
  {
		// There is short message and player wants it
		CG_Printf( shortMsg );
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
      Q_strncpyz( text, CG_Argv( 1 ), MAX_SAY_TEXT );
      if( Q_stricmpn( text, "[skipnotify]", 12 ) )
        trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
      CG_RemoveChatEscapeChar( text );
      CG_Printf( "%s\n", text );
    }

    return;
  }

  if( !strcmp( cmd, "tchat" ) )
  {
    Q_strncpyz( text, CG_Argv( 1 ), MAX_SAY_TEXT );
    if( Q_stricmpn( text, "[skipnotify]", 12 ) )
    {
      if( cg.snap->ps.stats[ STAT_PTEAM ] == PTE_ALIENS )
        trap_S_StartLocalSound( cgs.media.alienTalkSound, CHAN_LOCAL_SOUND );
      else if( cg.snap->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
        trap_S_StartLocalSound( cgs.media.humanTalkSound, CHAN_LOCAL_SOUND );
      else
        trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
    }
    CG_RemoveChatEscapeChar( text );
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

  // clientLevelShot is sent before taking a special screenshot for
  // the menu system during development
  if( !strcmp( cmd, "clientLevelShot" ) )
  {
    cg.levelShot = qtrue;
    return;
  }

  //the server has triggered a menu
  if( !strcmp( cmd, "servermenu" ) )
  {
    if( trap_Argc( ) == 2 && !cg.demoPlayback )
      CG_Menu( atoi( CG_Argv( 1 ) ) );

    return;
  }

  //the server thinks this client should close all menus
  if( !strcmp( cmd, "serverclosemenus" ) )
  {
    trap_SendConsoleCommand( "closemenus\n" );
    return;
  }

  //poison cloud effect needs to be reliable
  if( !strcmp( cmd, "poisoncloud" ) )
  {
    cg.poisonedTime = cg.time;

    if( CG_IsParticleSystemValid( &cg.poisonCloudPS ) )
    {
      cg.poisonCloudPS = CG_SpawnNewParticleSystem( cgs.media.poisonCloudPS );
      CG_SetAttachmentCent( &cg.poisonCloudPS->attachment, &cg.predictedPlayerEntity );
      CG_AttachToCent( &cg.poisonCloudPS->attachment );
    }

    return;
  }

  if( !strcmp( cmd, "weaponswitch" ) )
  {
    CG_Printf( "client weaponswitch\n" );
    if( trap_Argc( ) == 2 )
    {
      cg.weaponSelect = atoi( CG_Argv( 1 ) );
      cg.weaponSelectTime = cg.time;
    }

    return;
  }

  // server requests a ptrc
  if( !strcmp( cmd, "ptrcrequest" ) )
  {
    int   code = CG_ReadPTRCode( );

    trap_SendClientCommand( va( "ptrcverify %d", code ) );
    return;
  }

  // server issues a ptrc
  if( !strcmp( cmd, "ptrcissue" ) )
  {
    if( trap_Argc( ) == 2 )
    {
      int code = atoi( CG_Argv( 1 ) );

      CG_WritePTRCode( code );
    }

    return;
  }

  // reply to ptrcverify
  if( !strcmp( cmd, "ptrcconfirm" ) )
  {
    trap_SendConsoleCommand( "menu ptrc_popmenu\n" );

    return;
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
