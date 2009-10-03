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
    client = atoi( CG_Argv( i * 5 + 2 ) );

    sortedTeamPlayers[ i ] = client;

    cgs.clientinfo[ client ].location = atoi( CG_Argv( i * 5 + 3 ) );
    cgs.clientinfo[ client ].health = atoi( CG_Argv( i * 5 + 4 ) );
    cgs.clientinfo[ client ].armor = atoi( CG_Argv( i * 5 + 5 ) );
    cgs.clientinfo[ client ].curWeapon = atoi( CG_Argv( i * 5 + 6 ) );
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
==============
CG_Menu
==============
*/
void CG_Menu( int menu, int arg )
{
  const char *cmd;              // command to send
  const char *longMsg  = NULL; // command parameter
  const char *shortMsg = NULL; // non-modal version of message
  const char *dialog;
  dialogType_t type = 0; // controls which cg_disable var will switch it off
  
  switch( cg.snap->ps.stats[ STAT_PTEAM ] )
  {
    case PTE_ALIENS:
      dialog = "menu tremulous_alien_dialog\n";
      break;
    case PTE_HUMANS:
      dialog = "menu tremulous_human_dialog\n";
      break;
    default:
      dialog = "menu tremulous_default_dialog\n";
  }
  cmd = dialog;

  switch( menu )
  {
    case MN_TEAM:
      cmd       = "menu tremulous_teamselect\n";
      type      = DT_INTERACTIVE;
      break;

    case MN_A_CLASS:
      cmd       = "menu tremulous_alienclass\n";
      type      = DT_INTERACTIVE;
      break;

    case MN_H_SPAWN:
      cmd       = "menu tremulous_humanitem\n";
      type      = DT_INTERACTIVE;
      break;

    case MN_A_BUILD:
      cmd       = "menu tremulous_alienbuild\n";
      type      = DT_INTERACTIVE;
      break;

    case MN_H_BUILD:
      cmd       = "menu tremulous_humanbuild\n";
      type      = DT_INTERACTIVE;
      break;

    case MN_H_ARMOURY:
      cmd       = "menu tremulous_humanarmoury\n";
      type      = DT_INTERACTIVE;
      break;

    case MN_H_UNKNOWNITEM:
      shortMsg  = "Unknown item";
      type      = DT_ARMOURYEVOLVE;
      break;

    case MN_A_TEAMFULL:
      longMsg   = "The alien team has too many players. Please wait until slots "
                  "become available or join the human team.";
      shortMsg  = "The alien team has too many players";
      type      = DT_COMMAND;
      break;

    case MN_H_TEAMFULL:
      longMsg   = "The human team has too many players. Please wait until slots "
                  "become available or join the alien team.";
      shortMsg  = "The human team has too many players";
      type      = DT_COMMAND;
      break;

    case MN_A_TEAMCHANGEBUILDTIMER:
      longMsg   = "You cannot leave the Alien team until your build timer "
                  "has expired.";
      shortMsg  = "You cannot change teams until your build timer expires.";
      type      = DT_COMMAND;
      break;

    case MN_H_TEAMCHANGEBUILDTIMER:
      longMsg   = "You cannot leave the Human team until your build timer "
                  "has expired.";
      shortMsg  = "You cannot change teams until your build timer expires.";
      type      = DT_COMMAND;
      break;

    //===============================

    // Since cheating commands have no default binds, they will often be done
    // via console. In light of this, perhaps opening a menu is 
    // counterintuitive
    case MN_CMD_CHEAT:
      //longMsg   = "This action is considered cheating. It can only be used "
      //            "in cheat mode, which is not enabled on this server.";
      shortMsg  = "Cheats are not enabled on this server";
      type      = DT_COMMAND;
      break;

    case MN_CMD_TEAM:
      //longMsg   = "You must be on a team to perform this action. Join the alien"
      //            "or human team and try again.";
      shortMsg  = "Join a team first";
      type      = DT_COMMAND;
      break;

    case MN_CMD_SPEC:
      //longMsg   = "You may not perform this action while on a team. Become a "
      //            "spectator before trying again.";
      shortMsg  = "You can only use this command when spectating";
      type      = DT_COMMAND;
      break;

    case MN_CMD_ALIEN:
      //longMsg   = "You must be on the alien team to perform this action.";
      shortMsg  = "Must be alien to use this command";
      type      = DT_COMMAND;
      break;

    case MN_CMD_HUMAN:
      //longMsg   = "You must be on the human team to perform this action.";
      shortMsg  = "Must be human to use this command";
      type      = DT_COMMAND;
      break;

    case MN_CMD_LIVING:
      //longMsg   = "You must be living to perform this action.";
      shortMsg  = "Must be living to use this command";
      type      = DT_COMMAND;
      break;


    //===============================
    
    case MN_B_NOROOM:
      longMsg   = "There is no room to build here. Move until the structure turns "
                  "translucent green indicating a valid build location.";
      shortMsg  = "There is no room to build here";
      type      = DT_BUILD;
      break;

    case MN_B_NORMAL:
      longMsg   = "Cannot build on this surface. The surface is too steep or "
                  "unsuitable to build on. Please choose another site for this "
                  "structure.";
      shortMsg  = "Cannot build on this surface";
      type      = DT_BUILD;
      break;

    case MN_B_CANNOT:
      longMsg   = NULL;
      shortMsg  = "You cannot build that structure";
      type      = DT_BUILD;
      break;

    // FIXME: MN_H_ and MN_A_?
    case MN_B_LASTSPAWN:
      longMsg   = "This action would remove your team's last spawn point, "
                  "which often quickly results in a loss. Try building more "
                  "spawns.";
      shortMsg  = "You may not deconstruct the last spawn";
      break;

    case MN_B_SUDDENDEATH:
      longMsg   = "Neither team has prevailed after a certain time and the "
                  "game has entered Sudden Death. During Sudden Death "
                  "building is not allowed.";
      shortMsg  = "Cannot build during Sudden Death";
      type      = DT_BUILD;
      break;

    case MN_B_REVOKED:
      longMsg   = "Your teammates have lost faith in your ability to build "
                  "for the team. You will not be allowed to build until your "
                  "team votes to reinstate your building rights.";
      shortMsg  = "Your building rights have been revoked";
      type      = DT_BUILD;
      break;

    case MN_B_SURRENDER:
      longMsg   = "Your team has decided to admit defeat and concede the game:"
                  "traitors and cowards are not allowed to build.";
                  // too harsh?
      shortMsg  = "Building is denied to traitorous cowards";
      break;

    //===============================

    case MN_H_NOBP:
      longMsg   = "There is no power remaining. Free up power by deconstructing "
                  "existing buildable objects.";
      shortMsg  = "There is no power remaining";
      type      = DT_BUILD;
      break;

    case MN_H_NOTPOWERED:
      longMsg   = "This buildable is not powered. Build a Reactor and/or Repeater "
                  "in order to power it.";
      shortMsg  = "This buildable is not powered";
      type      = DT_BUILD;
      break;

    case MN_H_ONEREACTOR:
      longMsg   = "There can only be one Reactor. Deconstruct the existing one if you "
                  "wish to move it.";
      shortMsg  = "There can only be one Reactor";
      type      = DT_BUILD;
      break;

    case MN_H_NOPOWERHERE:
      longMsg   = "There is no power here. If available, a Repeater may be used to "
                  "transmit power to this location.";
      shortMsg  = "There is no power here";
      type      = DT_BUILD;
      break;

    case MN_H_NODCC:
      longMsg   = "There is no Defense Computer. A Defense Computer is needed to "
                  "build this.";
      shortMsg  = "There is no Defense Computer";
      type      = DT_BUILD;
      break;

    case MN_H_RPTPOWERHERE:
      longMsg   = "This area already has power. A Repeater is not required here.";
      shortMsg  = "This area already has power";
      type      = DT_BUILD;
      break;

    case MN_H_NOSLOTS:
      longMsg   = "You have no room to carry this. Please sell any conflicting "
                  "upgrades before purchasing this item.";
      shortMsg  = "You have no room to carry this";
      type      = DT_ARMOURYEVOLVE;
      break;

    case MN_H_NOFUNDS:
      longMsg   = "Insufficient funds. You do not have enough credits to perform "
                  "this action.";
      shortMsg  = "Insufficient funds";
      type      = DT_ARMOURYEVOLVE;
      break;

    case MN_H_ITEMHELD:
      longMsg   = "You already hold this item. It is not possible to carry multiple "
                  "items of the same type.";
      shortMsg  = "You already hold this item";
      type      = DT_ARMOURYEVOLVE;
      break;

    case MN_H_NOARMOURYHERE:
      longMsg   = "You must be near a powered Armoury in order to purchase "
                  "weapons, upgrades or ammunition.";
      shortMsg  = "You must be near a powered Armoury";
      type      = DT_ARMOURYEVOLVE;
      break;

    case MN_H_NOENERGYAMMOHERE:
      longMsg   = "You must be near a Reactor or a powered Armoury or Repeater "
                  "in order to purchase energy ammunition.";
      shortMsg  = "You must be near a Reactor or a powered Armoury or Repeater";
      type      = DT_ARMOURYEVOLVE;
      break;

    case MN_H_NOROOMBSUITON:
      longMsg   = "There is not enough room here to put on a Battle Suit. "
                  "Make sure you have enough head room to climb in.";
      shortMsg  = "Not enough room here to put on a Battle Suit";
      type      = DT_ARMOURYEVOLVE;
      break;

    case MN_H_NOROOMBSUITOFF:
      longMsg   = "There is not enough room here to take off your Battle Suit. "
                  "Make sure you have enough head room to climb out.";
      shortMsg  = "Not enough room here to take off your Battle Suit";
      type      = DT_ARMOURYEVOLVE;
      break;

    case MN_H_ARMOURYBUILDTIMER:
      longMsg   = "You are not allowed to buy or sell weapons until your "
                  "build timer has expired.";
      shortMsg  = "You can not buy or sell weapons until your build timer "
                  "expires";
      type      = DT_ARMOURYEVOLVE;
      break;

    case MN_H_DEADTOCLASS:
      shortMsg  = "You must be dead to use the class command";
      type      = DT_COMMAND;
      break;

    case MN_H_UNKNOWNSPAWNITEM:
      shortMsg  = "Unknown starting item";
      type      = DT_COMMAND;
      break;

    //===============================

    case MN_A_NOCREEP:
      longMsg   = "There is no creep here. You must build near existing Eggs or "
                  "the Overmind. Alien structures will not support themselves.";
      shortMsg  = "There is no creep here";
      type      = DT_BUILD;
      break;

    case MN_A_NOOVMND:
      longMsg   = "There is no Overmind. An Overmind must be built to control "
                  "the structure you tried to place";
      shortMsg  = "There is no Overmind";
      type      = DT_BUILD;
      break;

    case MN_A_ONEOVERMIND:
      longMsg   = "There can only be one Overmind. Deconstruct the existing one if you "
                  "wish to move it.";
      shortMsg  = "There can only be one Overmind";
      type      = DT_BUILD;
      break;

    case MN_A_ONEHOVEL:
      longMsg   = "There can only be one Hovel. Deconstruct the existing one if you "
                  "wish to move it.";
      shortMsg  = "There can only be one Hovel";
      type      = DT_BUILD;
      break;

    case MN_A_NOBP:
      longMsg   = "The Overmind cannot control any more structures. Deconstruct existing "
                  "structures to build more.";
      shortMsg  = "The Overmind cannot control any more structures";
      type      = DT_BUILD;
      break;

    case MN_A_NOEROOM:
      longMsg   = "There is no room to evolve here. Move away from walls or other "
                   "nearby objects and try again.";
      shortMsg  = "There is no room to evolve here";
      type      = DT_ARMOURYEVOLVE;
      break;

    case MN_A_TOOCLOSE:
      longMsg   = "This location is too close to the enemy to evolve. Move away "
                  "until you are no longer aware of the enemy's presence and try "
                  "again.";
      shortMsg  = "This location is too close to the enemy to evolve";
      type      = DT_ARMOURYEVOLVE;
      break;

    case MN_A_NOOVMND_EVOLVE:
      longMsg   = "There is no Overmind. An Overmind must be built to allow "
                  "you to upgrade.";
      shortMsg  = "There is no Overmind";
      type      = DT_ARMOURYEVOLVE;
      break;

    case MN_A_EVOLVEBUILDTIMER:
      longMsg   = "You cannot Evolve until your build timer has expired.";
      shortMsg  = "You cannot Evolve until your build timer expires";
      type      = DT_ARMOURYEVOLVE;
      break;

    case MN_A_HOVEL_OCCUPIED:
      longMsg   = "This Hovel is already occupied by another builder.";
      shortMsg  = "This Hovel is already occupied by another builder";
      type      = DT_COMMAND;
      break;

    case MN_A_HOVEL_BLOCKED:
      longMsg   = "The exit to this Hovel is currently blocked. Please wait until it "
                  "becomes clear then try again.";
      shortMsg  = "The exit to this Hovel is currently blocked";
      type      = DT_COMMAND;
      break;

    case MN_A_HOVEL_EXIT:
      longMsg   = "The exit to this Hovel would always be blocked. Please choose "
                  "a more suitable location.";
      shortMsg  = "The exit to this Hovel would always be blocked";
      type      = DT_BUILD;
      break;

    case MN_A_INFEST:
      trap_Cvar_Set( "ui_currentClass", va( "%d %d",  cg.snap->ps.stats[ STAT_PCLASS ],
                                                      cg.snap->ps.persistant[ PERS_CREDIT ] ) );
      cmd       = "menu tremulous_alienupgrade\n";
      type      = DT_INTERACTIVE;
      break;

    case MN_A_CANTEVOLVE:
      shortMsg  = va( "You cannot evolve to %s", 
                      BG_FindHumanNameForClassNum( arg ) );
      type      = DT_ARMOURYEVOLVE;
      break;

    case MN_A_EVOLVEWALLWALK:
      shortMsg  = "You cannot evolve while wallwalking";
      type      = DT_ARMOURYEVOLVE;
      break;

    case MN_A_UNKNOWNCLASS:
      shortMsg  = "Unknown class";
      type      = DT_ARMOURYEVOLVE;
      break;
      
    case MN_A_CLASSNOTSPAWN:
      shortMsg  = va( "You cannot spawn as %s", 
                      BG_FindHumanNameForClassNum( arg ) );
      type      = DT_ARMOURYEVOLVE;
      break;
    
    case MN_A_CLASSNOTALLOWED:
      shortMsg  = va( "%s is not allowed",
                      BG_FindHumanNameForClassNum( arg ) );
      type      = DT_ARMOURYEVOLVE;
      break;

    case MN_A_CLASSNOTATSTAGE:
      shortMsg  = va( "%s is not allowed at Stage %d",
                      BG_FindHumanNameForClassNum( arg ),
                      cgs.alienStage + 1 );
      type      = DT_ARMOURYEVOLVE;
      break;

    default:
      Com_Printf( "cgame: debug: no such menu %d\n", menu );
  }
  
  if( type == DT_ARMOURYEVOLVE && cg_disableUpgradeDialogs.integer )
    return;

  if( type == DT_BUILD && cg_disableBuildDialogs.integer )
    return;

  if( type == DT_COMMAND && cg_disableCommandDialogs.integer )
    return;

  if( cmd != dialog )
  {
    trap_SendConsoleCommand( cmd );
  }
  else if( longMsg && cg_disableWarningDialogs.integer == 0 )
  {
    trap_Cvar_Set( "ui_dialog", longMsg );
    trap_SendConsoleCommand( cmd );
  }
  else if( shortMsg && cg_disableWarningDialogs.integer < 2 )
  {
    CG_Printf( "%s\n", shortMsg );
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
      CG_Menu( atoi( CG_Argv( 1 ) ), 0 );
    if( trap_Argc( ) == 3 && !cg.demoPlayback )
      CG_Menu( atoi( CG_Argv( 1 ) ), atoi( CG_Argv( 2 ) ) );
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
