// Copyright (C) 1999-2000 Id Software, Inc.
//

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
 
#include "g_local.h"

/*
==================
ScoreboardMessage

==================
*/
void ScoreboardMessage( gentity_t *ent )
{
  char    entry[ 1024 ];
  char    string[ 1400 ];
  int     stringlength;
  int     i, j;
  gclient_t *cl;
  int     numSorted;

  // send the latest information on all clients
  string[ 0 ] = 0;
  stringlength = 0;

  numSorted = level.numConnectedClients;

  for( i = 0; i < numSorted; i++ )
  {
    int   ping;

    cl = &level.clients[ level.sortedClients[ i ] ];

    if( cl->pers.connected == CON_CONNECTING )
      ping = -1;
    else
      ping = cl->ps.ping < 999 ? cl->ps.ping : 999;

    Com_sprintf( entry, sizeof( entry ),
      " %i %i %i %i", level.sortedClients[ i ],
      cl->ps.persistant[ PERS_SCORE ], ping, ( level.time - cl->pers.enterTime ) / 60000 );
    
    j = strlen( entry );
    
    if( stringlength + j > 1024 )
      break;
    
    strcpy( string + stringlength, entry );
    stringlength += j;
  }

  trap_SendServerCommand( ent-g_entities, va( "scores %i %i %i%s", i,
    level.alienKills, level.humanKills, string ) );
}


/*
==================
Cmd_Score_f

Request current scoreboard information
==================
*/
void Cmd_Score_f( gentity_t *ent )
{
  ScoreboardMessage( ent );
}



/*
==================
CheatsOk
==================
*/
qboolean CheatsOk( gentity_t *ent )
{
  if( !g_cheats.integer )
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"Cheats are not enabled on this server.\n\"" ) );
    return qfalse;
  }
  
  if( ent->health <= 0 )
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"You must be alive to use this command.\n\"" ) );
    return qfalse; 
  }

  return qtrue;
}


/*
==================
ConcatArgs
==================
*/
char *ConcatArgs( int start )
{
  int         i, c, tlen;
  static char line[ MAX_STRING_CHARS ];
  int         len;
  char        arg[ MAX_STRING_CHARS ];

  len = 0;
  c = trap_Argc( );
  
  for( i = start; i < c; i++ )
  {
    trap_Argv( i, arg, sizeof( arg ) );
    tlen = strlen( arg );
    
    if( len + tlen >= MAX_STRING_CHARS - 1 )
      break;

    memcpy( line + len, arg, tlen );
    len += tlen;
    
    if( i != c - 1 )
    {
      line[ len ] = ' ';
      len++;
    }
  }

  line[ len ] = 0;

  return line;
}


/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f( gentity_t *ent )
{
  char      *name;
  int       i;
  qboolean  give_all;
  gentity_t *it_ent;
  trace_t   trace;

  if( !CheatsOk( ent ) )
    return;

  name = ConcatArgs( 1 );

  if( Q_stricmp( name, "all" ) == 0 )
    give_all = qtrue;
  else
    give_all = qfalse;

  if( give_all || Q_stricmp( name, "health" ) == 0 )
  {
    ent->health = ent->client->ps.stats[ STAT_MAX_HEALTH ];
    if( !give_all )
      return;
  }

  if( give_all || Q_stricmp( name, "weapons" ) == 0 )
  {
    BG_packWeapon( ( 1 << WP_NUM_WEAPONS ) - 1 - ( 1 << WP_NONE ), ent->client->ps.stats );
    
    if( !give_all )
      return;
  }

  if( give_all || Q_stricmp( name, "ammo" ) == 0 )
  {
    for( i = 0; i < MAX_WEAPONS; i++ )
      BG_packAmmoArray( i, ent->client->ps.ammo, ent->client->ps.powerups, 999, 0, 0 );

    if( !give_all )
      return;
  }

  if( give_all || Q_stricmpn( name, "funds", 5 ) == 0 )
  {
    int credits = atoi( name + 6 );

    if( !credits )
      ent->client->ps.persistant[ PERS_CREDIT ]++;
    else
      ent->client->ps.persistant[ PERS_CREDIT ] += credits;

    if( !give_all )
      return;
  }
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f( gentity_t *ent )
{
  char  *msg;

  if( !CheatsOk( ent ) )
    return;

  ent->flags ^= FL_GODMODE;
  
  if( !( ent->flags & FL_GODMODE ) )
    msg = "godmode OFF\n";
  else
    msg = "godmode ON\n";

  trap_SendServerCommand( ent - g_entities, va( "print \"%s\"", msg ) );
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f( gentity_t *ent )
{
  char  *msg;

  if( !CheatsOk( ent ) )
    return;

  ent->flags ^= FL_NOTARGET;
  
  if( !( ent->flags & FL_NOTARGET ) )
    msg = "notarget OFF\n";
  else
    msg = "notarget ON\n";

  trap_SendServerCommand( ent - g_entities, va( "print \"%s\"", msg ) );
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( gentity_t *ent )
{
  char  *msg;

  if( !CheatsOk( ent ) )
    return;

  if( ent->client->noclip )
    msg = "noclip OFF\n";
  else
    msg = "noclip ON\n";

  ent->client->noclip = !ent->client->noclip;

  trap_SendServerCommand( ent - g_entities, va( "print \"%s\"", msg ) );
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_LevelShot_f( gentity_t *ent )
{
  if( !CheatsOk( ent ) )
    return;

  BeginIntermission( );
  trap_SendServerCommand( ent - g_entities, "clientLevelShot" );
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( gentity_t *ent )
{
  if( ent->client->sess.sessionTeam == TEAM_SPECTATOR )
    return;

  if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_NONE )
    return;
  
  if( ent->client->ps.stats[ STAT_STATE ] & SS_INFESTING )
    return;
    
  if( ent->client->ps.stats[ STAT_STATE ] & SS_HOVELING )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Leave the hovel first (use your destroy key)\n\"" );
    return;
  }
    
  if( ent->health <= 0 )
    return;

  ent->flags &= ~FL_GODMODE;
  ent->client->ps.stats[ STAT_HEALTH ] = ent->health = 0;
  player_die( ent, ent, ent, 100000, MOD_SUICIDE );
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f( gentity_t *ent )
{
  int     oldTeam;
  char    s[MAX_TOKEN_CHARS];

  //TA: rip out the q3a team system :)

  oldTeam = ent->client->pers.teamSelection;

  trap_Argv( 1, s, sizeof( s ) );

  if( !strlen( s ) )
  {
    trap_SendServerCommand( ent-g_entities, va("print \"team: %i\n\"", ent->client->pers.teamSelection ) );
    return;
  }

  if( !Q_stricmp( s, "0" ) || !Q_stricmp( s, "spectate" ) )
    ent->client->pers.teamSelection = PTE_NONE;
  else if( !Q_stricmp( s, "1" ) || !Q_stricmp( s, "aliens" ) )
    ent->client->pers.teamSelection = PTE_ALIENS;
  else if( !Q_stricmp( s, "2" ) || !Q_stricmp( s, "humans" ) )
    ent->client->pers.teamSelection = PTE_HUMANS;

  if( oldTeam != ent->client->pers.teamSelection )
  {
    level.bankCredits[ ent->client->ps.clientNum ] = 0;
    ent->client->ps.persistant[ PERS_CREDIT ] = 0;
    ent->client->pers.classSelection = PCL_NONE;
    ClientSpawn( ent, NULL, NULL, NULL );
  }

  //update ClientInfo
  ClientUserinfoChanged( ent->client->ps.clientNum );
  
  //FIXME: put some team change broadcast code here.
}


/*
==================
G_Say
==================
*/
static void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message )
{
  if( !other )
    return;
  
  if( !other->inuse )
    return;
  
  if( !other->client )
    return;
  
  if( other->client->pers.connected != CON_CONNECTED )
    return;

  if( mode == SAY_TEAM && !OnSameTeam( ent, other ) )
    return;

  trap_SendServerCommand( other-g_entities, va( "%s \"%s%c%c%s\"",
    mode == SAY_TEAM ? "tchat" : "chat",
    name, Q_COLOR_ESCAPE, color, message ) );
}

#define EC    "\x19"

void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText )
{
  int     j;
  gentity_t *other;
  int     color;
  char    name[ 64 ];
  // don't let text be too long for malicious reasons
  char    text[ MAX_SAY_TEXT ];
  char    location[ 64 ];

  switch( mode )
  {
    default:
    case SAY_ALL:
      G_LogPrintf( "say: %s: %s\n", ent->client->pers.netname, chatText );
      Com_sprintf( name, sizeof( name ), "%s%c%c"EC": ", ent->client->pers.netname,
                   Q_COLOR_ESCAPE, COLOR_WHITE );
      color = COLOR_GREEN;
      break;
      
    case SAY_TEAM:
      G_LogPrintf( "sayteam: %s: %s\n", ent->client->pers.netname, chatText );
      if( Team_GetLocationMsg( ent, location, sizeof( location ) ) )
        Com_sprintf( name, sizeof( name ), EC"(%s%c%c"EC") (%s)"EC": ", 
          ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, location );
      else
        Com_sprintf( name, sizeof( name ), EC"(%s%c%c"EC")"EC": ", 
          ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
      color = COLOR_CYAN;
      break;
      
    case SAY_TELL:
      if( target && 
          target->client->ps.stats[ STAT_PTEAM ] == ent->client->ps.stats[ STAT_PTEAM ] &&
          Team_GetLocationMsg( ent, location, sizeof( location ) ) )
        Com_sprintf( name, sizeof( name ), EC"[%s%c%c"EC"] (%s)"EC": ",
          ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, location );
      else
        Com_sprintf( name, sizeof( name ), EC"[%s%c%c"EC"]"EC": ",
          ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
      color = COLOR_MAGENTA;
      break;
  }

  Q_strncpyz( text, chatText, sizeof( text ) );

  if( target )
  {
    G_SayTo( ent, target, mode, color, name, text );
    return;
  }

  // echo the text to the console
  if( g_dedicated.integer )
    G_Printf( "%s%s\n", name, text);

  // send it to all the apropriate clients
  for( j = 0; j < level.maxclients; j++ )
  {
    other = &g_entities[ j ];
    G_SayTo( ent, other, mode, color, name, text );
  }
}


/*
==================
Cmd_Say_f
==================
*/
static void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 )
{
  char    *p;

  if( trap_Argc( ) < 2 && !arg0 )
    return;

  if( arg0 )
    p = ConcatArgs( 0 );
  else
    p = ConcatArgs( 1 );

  G_Say( ent, NULL, mode, p );
}

/*
==================
Cmd_Tell_f
==================
*/
static void Cmd_Tell_f( gentity_t *ent )
{
  int     targetNum;
  gentity_t *target;
  char    *p;
  char    arg[MAX_TOKEN_CHARS];

  if( trap_Argc( ) < 2 )
    return;

  trap_Argv( 1, arg, sizeof( arg ) );
  targetNum = atoi( arg );
  
  if( targetNum < 0 || targetNum >= level.maxclients )
    return;

  target = &g_entities[ targetNum ];
  if( !target || !target->inuse || !target->client )
    return;

  p = ConcatArgs( 2 );

  G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, p );
  G_Say( ent, target, SAY_TELL, p );
  // don't tell to the player self if it was already directed to this player
  // also don't send the chat back to a bot
  if( ent != target && !( ent->r.svFlags & SVF_BOT ) )
    G_Say( ent, ent, SAY_TELL, p );
}

/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f( gentity_t *ent )
{
  trap_SendServerCommand( ent-g_entities, va( "print \"%s\n\"", vtos( ent->s.origin ) ) );
}

/*
==================
Cmd_CallVote_f
==================
*/
void Cmd_CallVote_f( gentity_t *ent )
{
  int   i;
  char  arg1[ MAX_STRING_TOKENS ];
  char  arg2[ MAX_STRING_TOKENS ];

  if( !g_allowVote.integer )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Voting not allowed here.\n\"" );
    return;
  }

  if( level.voteTime )
  {
    trap_SendServerCommand( ent-g_entities, "print \"A vote is already in progress.\n\"" );
    return;
  }
  
  if( ent->client->pers.voteCount >= MAX_VOTE_COUNT )
  {
    trap_SendServerCommand( ent-g_entities, "print \"You have called the maximum number of votes.\n\"" );
    return;
  }
  
  if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_NONE )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Not allowed to call a vote as spectator.\n\"" );
    return;
  }

  // make sure it is a valid command to vote on
  trap_Argv( 1, arg1, sizeof( arg1 ) );
  trap_Argv( 2, arg2, sizeof( arg2 ) );

  if( strchr( arg1, ';' ) || strchr( arg2, ';' ) )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
    return;
  }

  if( !Q_stricmp( arg1, "map_restart" ) ) { }
  else if( !Q_stricmp( arg1, "nextmap" ) ) { }
  else if( !Q_stricmp( arg1, "map" ) ) { }
  else if( !Q_stricmp( arg1, "kick" ) ) { }
  else if( !Q_stricmp( arg1, "clientkick" ) ) { }
  else if( !Q_stricmp( arg1, "timelimit" ) ) { }
  else
  {
    trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
    trap_SendServerCommand( ent-g_entities, "print \"Vote commands are: map_restart, nextmap, map <mapname>, "
                                            "kick <player>, clientkick <clientnum>, "
                                            "timelimit <time>.\n\"" );
    return;
  }

  // if there is still a vote to be executed
  if( level.voteExecuteTime )
  {
    level.voteExecuteTime = 0;
    trap_SendConsoleCommand( EXEC_APPEND, va( "%s\n", level.voteString ) );
  }

  if( !Q_stricmp( arg1, "map" ) )
  {
    // special case for map changes, we want to reset the nextmap setting
    // this allows a player to change maps, but not upset the map rotation
    char  s[ MAX_STRING_CHARS ];

    trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof( s ) );
    
    if( *s )
      Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s; set nextmap \"%s\"", arg1, arg2, s );
    else
      Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );

    Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
  }
  else if( !Q_stricmp( arg1, "nextmap" ) )
  {
    char  s[ MAX_STRING_CHARS ];

    trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof( s ) );
    
    if( !*s )
    {
      trap_SendServerCommand( ent-g_entities, "print \"nextmap not set.\n\"" );
      return;
    }
    
    Com_sprintf( level.voteString, sizeof( level.voteString ), "vstr nextmap" );
    Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
  }
  else
  {
    Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
    Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
  }

  trap_SendServerCommand( -1, va( "print \"%s called a vote.\n\"", ent->client->pers.netname ) );

  // start the voting, the caller autoamtically votes yes
  level.voteTime = level.time;
  level.voteYes = 1;
  level.voteNo = 0;

  for( i = 0 ; i < level.maxclients ; i++ )
    level.clients[i].ps.eFlags &= ~EF_VOTED;

  ent->client->ps.eFlags |= EF_VOTED;

  trap_SetConfigstring( CS_VOTE_TIME, va( "%i", level.voteTime ) );
  trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );
  trap_SetConfigstring( CS_VOTE_YES, va( "%i", level.voteYes ) );
  trap_SetConfigstring( CS_VOTE_NO, va( "%i", level.voteNo ) );
}

/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f( gentity_t *ent )
{
  char msg[ 64 ];

  if( !level.voteTime )
  {
    trap_SendServerCommand( ent-g_entities, "print \"No vote in progress.\n\"" );
    return;
  }
  
  if( ent->client->ps.eFlags & EF_VOTED )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Vote already cast.\n\"" );
    return;
  }

  if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_NONE )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Not allowed to vote as spectator.\n\"" );
    return;
  }

  trap_SendServerCommand( ent-g_entities, "print \"Vote cast.\n\"" );

  ent->client->ps.eFlags |= EF_VOTED;

  trap_Argv( 1, msg, sizeof( msg ) );

  if( msg[ 0 ] == 'y' || msg[ 1 ] == 'Y' || msg[ 1 ] == '1' )
  {
    level.voteYes++;
    trap_SetConfigstring( CS_VOTE_YES, va( "%i", level.voteYes ) );
  }
  else
  {
    level.voteNo++;
    trap_SetConfigstring( CS_VOTE_NO, va( "%i", level.voteNo ) );
  }

  // a majority will be determined in G_CheckVote, which will also account
  // for players entering or leaving
}

/*
==================
Cmd_CallTeamVote_f
==================
*/
void Cmd_CallTeamVote_f( gentity_t *ent )
{
  int   i, team, cs_offset;
  char  arg1[ MAX_STRING_TOKENS ];
  char  arg2[ MAX_STRING_TOKENS ];

  team = ent->client->ps.stats[ STAT_PTEAM ];
  
  if( team == PTE_HUMANS )
    cs_offset = 0;
  else if( team == PTE_ALIENS )
    cs_offset = 1;
  else
    return;

  if( !g_allowVote.integer )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Voting not allowed here.\n\"" );
    return;
  }

  if( level.teamVoteTime[ cs_offset ] )
  {
    trap_SendServerCommand( ent-g_entities, "print \"A team vote is already in progress.\n\"" );
    return;
  }
  
  if( ent->client->pers.teamVoteCount >= MAX_VOTE_COUNT )
  {
    trap_SendServerCommand( ent-g_entities, "print \"You have called the maximum number of team votes.\n\"" );
    return;
  }
  
  if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_NONE )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Not allowed to call a vote as spectator.\n\"" );
    return;
  }

  // make sure it is a valid command to vote on
  trap_Argv( 1, arg1, sizeof( arg1 ) );
  trap_Argv( 2, arg2, sizeof( arg2 ) );
  
  if( strchr( arg1, ';' ) || strchr( arg2, ';' ) )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Invalid team vote string.\n\"" );
    return;
  }

  if( !Q_stricmp( arg1, "teamkick" ) )
  {
    char netname[ MAX_NETNAME ], kickee[ MAX_NETNAME ];

    Q_strncpyz( kickee, arg2, sizeof( kickee ) );
    Q_CleanStr( kickee );
    
    for( i = 0; i < level.maxclients; i++ )
    {
      if( level.clients[ i ].pers.connected == CON_DISCONNECTED )
        continue;
      
      if( level.clients[ i ].ps.stats[ STAT_PTEAM ] != team )
        continue;
      
      Q_strncpyz( netname, level.clients[ i ].pers.netname, sizeof( netname ) );
      Q_CleanStr( netname );
      
      if( !Q_stricmp( netname, kickee ) )
        break;
    }
    
    if( i >= level.maxclients )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"%s is not a valid player on your team.\n\"", arg2 ) );
      return;
    }
  }
  else
  {
    trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
    trap_SendServerCommand( ent-g_entities, "print \"Team vote commands are: teamkick <player>.\n\"" );
    return;
  }

  Com_sprintf( level.teamVoteString[ cs_offset ],
               sizeof( level.teamVoteString[ cs_offset ] ), "kick \"%s\"", arg2 );

  for( i = 0 ; i < level.maxclients ; i++ )
  {
    if( level.clients[ i ].pers.connected == CON_DISCONNECTED )
      continue;
    
    if( level.clients[ i ].ps.stats[ STAT_PTEAM ] == team )
      trap_SendServerCommand( i, va("print \"%s called a team vote.\n\"", ent->client->pers.netname ) );
  }

  // start the voting, the caller autoamtically votes yes
  level.teamVoteTime[ cs_offset ] = level.time;
  level.teamVoteYes[ cs_offset ] = 1;
  level.teamVoteNo[ cs_offset ] = 0;

  for( i = 0 ; i < level.maxclients ; i++ )
  {
    if( level.clients[ i ].ps.stats[ STAT_PTEAM ] == team )
      level.clients[ i ].ps.eFlags &= ~EF_TEAMVOTED;
  }
  
  ent->client->ps.eFlags |= EF_TEAMVOTED;

  trap_SetConfigstring( CS_TEAMVOTE_TIME + cs_offset, va( "%i", level.teamVoteTime[ cs_offset ] ) );
  trap_SetConfigstring( CS_TEAMVOTE_STRING + cs_offset, level.teamVoteString[ cs_offset ] );
  trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va( "%i", level.teamVoteYes[ cs_offset ] ) );
  trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va( "%i", level.teamVoteNo[ cs_offset ] ) );
}


/*
==================
Cmd_TeamVote_f
==================
*/
void Cmd_TeamVote_f( gentity_t *ent )
{
  int     team, cs_offset;
  char    msg[ 64 ];

  team = ent->client->ps.stats[ STAT_PTEAM ];
  if( team == PTE_HUMANS )
    cs_offset = 0;
  else if( team == PTE_ALIENS )
    cs_offset = 1;
  else
    return;

  if( !level.teamVoteTime[ cs_offset ] )
  {
    trap_SendServerCommand( ent-g_entities, "print \"No team vote in progress.\n\"" );
    return;
  }
  
  if( ent->client->ps.eFlags & EF_TEAMVOTED )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Team vote already cast.\n\"" );
    return;
  }
  
  if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_NONE )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Not allowed to vote as spectator.\n\"" );
    return;
  }

  trap_SendServerCommand( ent-g_entities, "print \"Team vote cast.\n\"" );

  ent->client->ps.eFlags |= EF_TEAMVOTED;

  trap_Argv( 1, msg, sizeof( msg ) );

  if( msg[ 0 ] == 'y' || msg[ 1 ] == 'Y' || msg[ 1 ] == '1' )
  {
    level.teamVoteYes[ cs_offset ]++;
    trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va( "%i", level.teamVoteYes[ cs_offset ] ) );
  }
  else
  {
    level.teamVoteNo[ cs_offset ]++;
    trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va( "%i", level.teamVoteNo[ cs_offset ] ) ); 
  }

  // a majority will be determined in TeamCheckVote, which will also account
  // for players entering or leaving
}


/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f( gentity_t *ent )
{
  vec3_t  origin, angles;
  char    buffer[ MAX_TOKEN_CHARS ];
  int     i;

  if( !g_cheats.integer )
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"Cheats are not enabled on this server.\n\"" ) );
    return;
  }
  
  if( trap_Argc( ) != 5 )
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"usage: setviewpos x y z yaw\n\"" ) );
    return;
  }

  VectorClear( angles );
  
  for( i = 0 ; i < 3 ; i++ )
  {
    trap_Argv( i + 1, buffer, sizeof( buffer ) );
    origin[ i ] = atof( buffer );
  }

  trap_Argv( 4, buffer, sizeof( buffer ) );
  angles[ YAW ] = atof( buffer );

  TeleportPlayer( ent, origin, angles );
}

#define EVOLVE_TRACE_HEIGHT 128.0f

/*
=================
Cmd_Class_f
=================
*/
void Cmd_Class_f( gentity_t *ent )
{
  char      s[ MAX_TOKEN_CHARS ];
  qboolean  dontSpawn = qfalse;
  int       clientNum;
  gentity_t *spawn;
  vec3_t    spawn_origin, spawn_angles;
  vec3_t    up = { 0.0f, 0.0f, 1.0f };
  int       length = 4096;
  int       i;
  trace_t   tr, tr2;
  vec3_t    infestOrigin, infestAngles;
  int       allowedClasses[ PCL_NUM_CLASSES ];
  int       numClasses = 0;
  
  int       numLevels;
  vec3_t    fromMins, fromMaxs, toMins, toMaxs;
  vec3_t    temp;

  clientNum = ent->client - level.clients;
  trap_Argv( 1, s, sizeof( s ) );

  if( BG_FindStagesForClass( PCL_A_B_LEV1, g_alienStage.integer ) )
  {
    allowedClasses[ 0 ] = PCL_A_B_BASE;
    allowedClasses[ 1 ] = PCL_A_B_LEV1;
    allowedClasses[ 2 ] = PCL_A_O_BASE;
    numClasses = 3;
  }
  else
  {
    allowedClasses[ 0 ] = PCL_A_B_BASE;
    allowedClasses[ 1 ] = PCL_A_O_BASE;
    numClasses = 2;
  }
  
  if( ent->client->pers.teamSelection == PTE_ALIENS &&
      !( ent->client->ps.stats[ STAT_STATE ] & SS_INFESTING ) &&
      !( ent->client->ps.stats[ STAT_STATE ] & SS_HOVELING ) &&
      !( ent->client->ps.stats[ STAT_STATE ] & SS_WALLCLIMBING ) &&
      !( ent->client->ps.stats[ STAT_STATE ] & SS_WALLCLIMBINGCEILING ) )
  {
    //if we are not currently spectating, we are attempting evolution
    if( ent->client->ps.stats[ STAT_PCLASS ] != PCL_NONE )
    {
      //evolve now
      ent->client->pers.classSelection = BG_FindClassNumForName( s );

      if( ent->client->pers.classSelection == PCL_NONE )
      {
        trap_SendServerCommand( ent-g_entities, va( "print \"Unknown class\n\"" ) );
        return;
      }
      
      numLevels = BG_ClassCanEvolveFromTo( ent->client->ps.stats[ STAT_PCLASS ],
                                           ent->client->pers.classSelection,
                                           (short)ent->client->ps.persistant[ PERS_CREDIT ], 0 );

      BG_FindBBoxForClass( ent->client->ps.stats[ STAT_PCLASS ],
                           fromMins, fromMaxs, NULL, NULL, NULL );
      BG_FindBBoxForClass( ent->client->pers.classSelection,
                           toMins, toMaxs, NULL, NULL, NULL );

      VectorCopy( ent->s.pos.trBase, infestOrigin );
      
      infestOrigin[ 2 ] += ( fabs( toMins[ 2 ] ) - fabs( fromMins[ 2 ] ) ) + 1.0f;
      VectorCopy( infestOrigin, temp );
      temp[ 2 ] += EVOLVE_TRACE_HEIGHT;

      //compute a place up in the air to start the real trace
      trap_Trace( &tr, infestOrigin, toMins, toMaxs, temp, ent->s.number, MASK_SHOT );
      VectorCopy( infestOrigin, temp );
      temp[ 2 ] += ( EVOLVE_TRACE_HEIGHT * tr.fraction ) - 1.0f;
      
      //trace down to the ground so that we can evolve on slopes
      trap_Trace( &tr, temp, toMins, toMaxs, infestOrigin, ent->s.number, MASK_SHOT );
      VectorCopy( tr.endpos, infestOrigin );
      
      //make REALLY sure
      trap_Trace( &tr2, ent->s.pos.trBase, NULL, NULL, infestOrigin, ent->s.number, MASK_SHOT );
      
      //check there is room to evolve
      if( !tr.startsolid && tr2.fraction == 1.0f )
      {
        //...check we can evolve to that class
        if( numLevels >= 0 && BG_FindStagesForClass( ent->client->pers.classSelection, g_alienStage.integer ) )
        {
          //remove credit
          ent->client->ps.persistant[ PERS_CREDIT ] -= (short)numLevels;
       
          ClientUserinfoChanged( clientNum );
          VectorCopy( infestOrigin, ent->s.pos.trBase );
          ClientSpawn( ent, ent, ent->s.pos.trBase, ent->s.apos.trBase );
          return;
        }
        else
        {
          ent->client->pers.classSelection = PCL_NONE;
          trap_SendServerCommand( ent-g_entities,
               va( "print \"You cannot evolve from your current class\n\"" ) );
          return;
        }
      }
      else
      {
        ent->client->pers.classSelection = PCL_NONE;
        G_TriggerMenu( clientNum, MN_A_NOEROOM );
        return;
      }
    }
    else
    {
      //spawning from an egg
      ent->client->pers.classSelection =
        ent->client->ps.stats[ STAT_PCLASS ] = BG_FindClassNumForName( s );

      if( ent->client->pers.classSelection != PCL_NONE )
      {
        for( i = 0; i < numClasses; i++ )
        {
          if( allowedClasses[ i ] == ent->client->pers.classSelection &&
              BG_FindStagesForClass( ent->client->pers.classSelection, g_alienStage.integer ) )
          {
            G_PushSpawnQueue( &level.alienSpawnQueue, clientNum );
            return;
          }
        }

        ent->client->pers.classSelection = PCL_NONE;
        trap_SendServerCommand( ent-g_entities, va( "print \"You cannot spawn as this class\n\"" ) );
      }
      else
      {
        trap_SendServerCommand( ent-g_entities, va( "print \"Unknown class\n\"" ) );
        return;
      }
    }
  }
  else if( ent->client->pers.teamSelection == PTE_HUMANS )
  {
    //humans cannot use this command whilst alive
    if( ent->client->ps.stats[ STAT_PCLASS ] != PCL_NONE )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"You must be dead to use the class command\n\"" ) );
      return;
    }

    ent->client->pers.classSelection =
      ent->client->ps.stats[ STAT_PCLASS ] = PCL_H_BASE;

    //set the item to spawn with
    if( !Q_stricmp( s, BG_FindNameForWeapon( WP_MACHINEGUN ) ) )
      ent->client->pers.humanItemSelection = WP_MACHINEGUN;
    else if( !Q_stricmp( s, BG_FindNameForWeapon( WP_HBUILD ) ) )
      ent->client->pers.humanItemSelection = WP_HBUILD;
    else
    {
      ent->client->pers.classSelection = PCL_NONE;
      trap_SendServerCommand( ent-g_entities, va( "print \"Unknown starting item\n\"" ) );
      return;
    }

    G_PushSpawnQueue( &level.humanSpawnQueue, clientNum );
  }
  else if( ent->client->pers.teamSelection == PTE_NONE )
  {
    //can't use this command unless on a team
    ent->client->pers.classSelection = PCL_NONE;
    ent->client->sess.sessionTeam = TEAM_FREE;
    ClientSpawn( ent, NULL, NULL, NULL );
    trap_SendServerCommand( ent-g_entities, va( "print \"Join a team first\n\"" ) );
  }
}


/*
=================
Cmd_Destroy_f
=================
*/
void Cmd_Destroy_f( gentity_t *ent, qboolean deconstruct )
{
  vec3_t      forward, end;
  trace_t     tr;
  gentity_t   *traceEnt;

  if( ent->client->ps.stats[ STAT_STATE ] & SS_HOVELING )
    G_Damage( ent->client->hovel, ent, ent, forward, ent->s.origin, 10000, 0, MOD_SUICIDE );
  
  if( !( ent->client->ps.stats[ STAT_STATE ] & SS_INFESTING ) )
  {
    AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );
    VectorMA( ent->client->ps.origin, 100, forward, end );

    trap_Trace( &tr, ent->client->ps.origin, NULL, NULL, end, ent->s.number, MASK_PLAYERSOLID );
    traceEnt = &g_entities[ tr.entityNum ];

    if( tr.fraction < 1.0f &&
        ( traceEnt->s.eType == ET_BUILDABLE ) &&
        ( traceEnt->biteam == ent->client->pers.teamSelection ) &&
        ( ( ent->client->ps.weapon >= WP_ABUILD ) &&
          ( ent->client->ps.weapon <= WP_HBUILD ) ) )
    {
      if( ent->client->ps.stats[ STAT_MISC ] > 0 )
      {
        G_AddEvent( ent, EV_BUILD_DELAY, ent->client->ps.clientNum );
        return;
      }

      if( !deconstruct )
        G_Damage( traceEnt, ent, ent, forward, tr.endpos, 10000, 0, MOD_SUICIDE );
      else
        G_FreeEntity( traceEnt );
    
      ent->client->ps.stats[ STAT_MISC ] +=
        BG_FindBuildDelayForWeapon( ent->s.weapon ) >> 1;
    }
  }
}


/*
=================
Cmd_ActivateItem_f

Activate an item
=================
*/
void Cmd_ActivateItem_f( gentity_t *ent )
{
  char  s[ MAX_TOKEN_CHARS ];
  int   upgrade;

  trap_Argv( 1, s, sizeof( s ) );
  upgrade = BG_FindUpgradeNumForName( s );

  if( ent->client->pers.teamSelection != PTE_HUMANS )
    return;
    
  if( BG_gotItem( upgrade, ent->client->ps.stats ) )
    BG_activateItem( upgrade, ent->client->ps.stats );
  else
    trap_SendServerCommand( ent-g_entities, va( "print \"You don't have the %s\n\"", s ) );
}


/*
=================
Cmd_DeActivateItem_f

Deactivate an item
=================
*/
void Cmd_DeActivateItem_f( gentity_t *ent )
{
  char  s[ MAX_TOKEN_CHARS ];
  int   upgrade;

  trap_Argv( 1, s, sizeof( s ) );
  upgrade = BG_FindUpgradeNumForName( s );

  if( ent->client->pers.teamSelection != PTE_HUMANS )
    return;
    
  if( BG_gotItem( upgrade, ent->client->ps.stats ) )
    BG_deactivateItem( upgrade, ent->client->ps.stats );
  else
    trap_SendServerCommand( ent-g_entities, va( "print \"You don't have the %s\n\"", s ) );
}


/*
=================
Cmd_ToggleItem_f
=================
*/
void Cmd_ToggleItem_f( gentity_t *ent )
{
  char  s[ MAX_TOKEN_CHARS ];
  int   upgrade;

  trap_Argv( 1, s, sizeof( s ) );
  upgrade = BG_FindUpgradeNumForName( s );

  if( ent->client->pers.teamSelection != PTE_HUMANS )
    return;
    
  if( BG_gotItem( upgrade, ent->client->ps.stats ) )
  {
    if( BG_activated( upgrade, ent->client->ps.stats ) )
      BG_deactivateItem( upgrade, ent->client->ps.stats );
    else
      BG_activateItem( upgrade, ent->client->ps.stats );
  }
  else
    trap_SendServerCommand( ent-g_entities, va( "print \"You don't have the %s\n\"", s ) );
}

/*
=================
Cmd_Buy_f
=================
*/
void Cmd_Buy_f( gentity_t *ent )
{
  char      s[ MAX_TOKEN_CHARS ];
  int       i;
  int       weapon, upgrade, numItems = 0;
  int       quan, clips, maxClips;

  for( i = UP_NONE; i < UP_NUM_UPGRADES; i++ )
  {
    if( BG_gotItem( i, ent->client->ps.stats ) )
      numItems++;
  }

  for( i = WP_NONE; i < WP_NUM_WEAPONS; i++ )
  {
    if( BG_gotWeapon( i, ent->client->ps.stats ) )
      numItems++;
  }

  trap_Argv( 1, s, sizeof( s ) );
  
  //aliens don't buy stuff
  if( ent->client->pers.teamSelection != PTE_HUMANS )
    return;

  //no armoury nearby
  if( !G_BuildableRange( ent->client->ps.origin, 100, BA_H_ARMOURY ) )
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"You must be near an armoury\n\"" ) );
    return;
  }

  weapon = BG_FindWeaponNumForName( s );
  upgrade = BG_FindUpgradeNumForName( s );
    
  if( weapon != WP_NONE )
  {
    //already got this?
    if( BG_gotWeapon( weapon, ent->client->ps.stats ) )
    {
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_ITEMHELD );
      return;
    }
      
    //can afford this?
    if( BG_FindPriceForWeapon( weapon ) > (short)ent->client->ps.persistant[ PERS_CREDIT ] )
    {
      G_LogPrintf( "Client %d buying weapon %d, value %d, credit %d\n",
                   ent->client->ps.clientNum,
                   weapon,
                   BG_FindPriceForWeapon( weapon ),
                   (short)ent->client->ps.persistant[ PERS_CREDIT ] );

      G_TriggerMenu( ent->client->ps.clientNum, MN_H_NOFUNDS );
      return;
    }
    
    //have space to carry this?
    if( BG_FindSlotsForWeapon( weapon ) & ent->client->ps.stats[ STAT_SLOTS ] )
    {
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_NOSLOTS );
      return;
    }
    
    if( BG_FindTeamForWeapon( weapon ) != WUT_HUMANS )
    {
      //shouldn't need a fancy dialog
      trap_SendServerCommand( ent-g_entities, va( "print \"You can't buy alien items\n\"" ) );
      return;
    }
    
    //are we /allowed/ to buy this?
    if( !BG_FindPurchasableForWeapon( weapon ) )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"You can't buy this item\n\"" ) );
      return;
    }
    
    //are we /allowed/ to buy this?
    if( !BG_FindStagesForWeapon( weapon, g_humanStage.integer ) )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"You can't buy this item\n\"" ) );
      return;
    }
    
    //add to inventory
    BG_packWeapon( weapon, ent->client->ps.stats );
    BG_FindAmmoForWeapon( weapon, &quan, &clips, &maxClips );
    
    if( BG_FindUsesEnergyForWeapon( weapon ) &&
        BG_gotItem( UP_BATTPACK, ent->client->ps.stats ) )
      quan = (int)( (float)quan * BATTPACK_MODIFIER );
    
    BG_packAmmoArray( weapon, ent->client->ps.ammo, ent->client->ps.powerups,
                      quan, clips, maxClips );

    //force a weapon change
    ent->client->ps.pm_flags |= PMF_WEAPON_SWITCH;
    trap_SendServerCommand( ent-g_entities, va( "weaponswitch %d", weapon ) );

    //set build delay/pounce etc to 0
    ent->client->ps.stats[ STAT_MISC ] = 0;
    
    //subtract from funds
    ent->client->ps.persistant[ PERS_CREDIT ] -= (short)BG_FindPriceForWeapon( weapon );
  }
  else if( upgrade != UP_NONE )
  {
    weapon_t  weaponAmmo;
    
    //already got this?
    if( BG_gotItem( upgrade, ent->client->ps.stats ) )
    {
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_ITEMHELD );
      return;
    }
    
    //can afford this?
    if( BG_FindPriceForUpgrade( upgrade ) > (short)ent->client->ps.persistant[ PERS_CREDIT ] )
    {
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_NOFUNDS );
      G_LogPrintf( "Client %d buying upgrade %d, value %d, credit %d\n",
                   ent->client->ps.clientNum,
                   weapon,
                   BG_FindPriceForWeapon( upgrade ),
                   (short)ent->client->ps.persistant[ PERS_CREDIT ] );

      return;
    }
    
    //have space to carry this?
    if( BG_FindSlotsForUpgrade( upgrade ) & ent->client->ps.stats[ STAT_SLOTS ] )
    {
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_NOSLOTS );
      return;
    }
    
    if( BG_FindTeamForUpgrade( upgrade ) != WUT_HUMANS )
    {
      //shouldn't need a fancy dialog
      trap_SendServerCommand( ent-g_entities, va( "print \"You can't buy alien items\n\"" ) );
      return;
    }
    
    //are we /allowed/ to buy this?
    if( !BG_FindStagesForUpgrade( upgrade, g_humanStage.integer ) )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"You can't buy this item\n\"" ) );
      return;
    }
    
    if( upgrade == UP_AMMO )
    {
      for( i = WP_NONE; i < WP_NUM_WEAPONS; i++ )
      {
        if( BG_gotWeapon( i, ent->client->ps.stats ) &&
            !BG_FindUsesEnergyForWeapon( i ) &&
            !BG_FindInfinteAmmoForWeapon( i ) )
        {
          BG_FindAmmoForWeapon( i, &quan, &clips, &maxClips );
          BG_packAmmoArray( i, ent->client->ps.ammo, ent->client->ps.powerups,
                            quan, clips, maxClips );
        }
      }
    }
    else
    {
      //add to inventory
      BG_packItem( upgrade, ent->client->ps.stats );
    }
    
    //subtract from funds
    ent->client->ps.persistant[ PERS_CREDIT ] -= (short)BG_FindPriceForUpgrade( upgrade );
  }
  else
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"Unknown item\n\"" ) );
  }
  
  //if the buyer previously had no items at all, force a new selection
  if( numItems == 0 )
    G_AddEvent( ent, EV_NEXT_WEAPON, ent->client->ps.clientNum );
  
  if( trap_Argc( ) >= 2 )
  {
    trap_Argv( 2, s, sizeof( s ) );
    
    //retrigger the armoury menu
    if( !Q_stricmp( s, "retrigger" ) )
      ent->client->retriggerArmouryMenu = level.framenum + RAM_FRAMES;
  }
  
  //update ClientInfo
  ClientUserinfoChanged( ent->client->ps.clientNum );
}


/*
=================
Cmd_Sell_f
=================
*/
void Cmd_Sell_f( gentity_t *ent )
{
  char      s[ MAX_TOKEN_CHARS ];
  int       i;
  int       weapon, upgrade;
  int       quan, clips, maxClips;

  trap_Argv( 1, s, sizeof( s ) );

  //aliens don't sell stuff
  if( ent->client->pers.teamSelection != PTE_HUMANS )
    return;

  //no armoury nearby
  if( !G_BuildableRange( ent->client->ps.origin, 100, BA_H_ARMOURY ) )
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"You must be near an armoury\n\"" ) );
    return;
  }

  weapon = BG_FindWeaponNumForName( s );
  upgrade = BG_FindUpgradeNumForName( s );
  
  if( weapon != WP_NONE )
  {
    //are we /allowed/ to sell this?
    if( !BG_FindPurchasableForWeapon( weapon ) )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"You can't sell this item\n\"" ) );
      return;
    }
    
    //remove weapon if carried
    if( BG_gotWeapon( weapon, ent->client->ps.stats ) )
    {
      BG_removeWeapon( weapon, ent->client->ps.stats );

      //add to funds
      ent->client->ps.persistant[ PERS_CREDIT ] += (short)BG_FindPriceForWeapon( weapon );
    }

    //if we have this weapon selected, force a new selection
    if( weapon == ent->client->ps.weapon )
    {
      //force a weapon change
      ent->client->ps.pm_flags |= PMF_WEAPON_SWITCH;
      trap_SendServerCommand( ent-g_entities, va( "weaponswitch %d", WP_BLASTER ) );
    }
  }
  else if( upgrade != UP_NONE )
  {
    //remove upgrade if carried
    if( BG_gotItem( upgrade, ent->client->ps.stats ) )
    {
      BG_removeItem( upgrade, ent->client->ps.stats );

      //add to funds
      ent->client->ps.persistant[ PERS_CREDIT ] += (short)BG_FindPriceForUpgrade( upgrade );
    }
    
    //if we have this upgrade selected, force a new selection
    if( upgrade == ent->client->pers.cmd.weapon - 32 )
      G_AddEvent( ent, EV_NEXT_WEAPON, ent->client->ps.clientNum );
  }
  else if( !Q_stricmp( s, "weapons" ) )
  {
    for( i = WP_NONE + 1; i < WP_NUM_WEAPONS; i++ )
    {
      if( BG_gotWeapon( i, ent->client->ps.stats ) && i != WP_BLASTER )
      {
        BG_removeWeapon( i, ent->client->ps.stats );

        //add to funds
        ent->client->ps.persistant[ PERS_CREDIT ] += (short)BG_FindPriceForWeapon( i );
      }
      
      //if we have this weapon selected, force a new selection
      if( i == ent->client->ps.weapon )
      {
        //force a weapon change
        ent->client->ps.pm_flags |= PMF_WEAPON_SWITCH;
        trap_SendServerCommand( ent-g_entities, va( "weaponswitch %d", WP_BLASTER ) );
      }
    }
  }
  else if( !Q_stricmp( s, "upgrades" ) )
  {
    for( i = UP_NONE + 1; i < UP_NUM_UPGRADES; i++ )
    {
      //remove upgrade if carried
      if( BG_gotItem( i, ent->client->ps.stats ) )
      {
        BG_removeItem( i, ent->client->ps.stats );

        //add to funds
        ent->client->ps.persistant[ PERS_CREDIT ] += (short)BG_FindPriceForUpgrade( i );
      }
      
      //if we have this upgrade selected, force a new selection
      if( i == ent->client->pers.cmd.weapon - 32 )
        G_AddEvent( ent, EV_NEXT_WEAPON, ent->client->ps.clientNum );
    }
  }
  else
    trap_SendServerCommand( ent-g_entities, va( "print \"Unknown item\n\"" ) );
  
  if( trap_Argc( ) >= 2 )
  {
    trap_Argv( 2, s, sizeof( s ) );
    
    //retrigger the armoury menu
    if( !Q_stricmp( s, "retrigger" ) )
      ent->client->retriggerArmouryMenu = level.framenum + RAM_FRAMES;
  }
  
  //update ClientInfo
  ClientUserinfoChanged( ent->client->ps.clientNum );
}


/*
=================
Cmd_Build_f
=================
*/
void Cmd_Build_f( gentity_t *ent )
{
  char          s[ MAX_TOKEN_CHARS ];
  char          s1[ MAX_TOKEN_CHARS ];
  buildable_t   buildable;
  weapon_t      weapon;
  float         dist;
  vec3_t        origin;
  pTeam_t       team;

  trap_Argv( 1, s, sizeof( s ) );

  buildable = BG_FindBuildNumForName( s );
  team = ent->client->ps.stats[ STAT_PTEAM ];
  
  if( buildable != BA_NONE &&
      ( ( 1 << ent->client->ps.weapon ) & BG_FindBuildWeaponForBuildable( buildable ) ) &&
      !( ent->client->ps.stats[ STAT_STATE ] & SS_INFESTING ) &&
      !( ent->client->ps.stats[ STAT_STATE ] & SS_HOVELING ) &&
      ( ( team == PTE_ALIENS && BG_FindStagesForBuildable( buildable, g_alienStage.integer ) ) ||
        ( team == PTE_HUMANS && BG_FindStagesForBuildable( buildable, g_humanStage.integer ) ) ) )
  {
    dist = BG_FindBuildDistForClass( ent->client->ps.stats[ STAT_PCLASS ] );
    
    //these are the errors displayed when the builder first selects something to use
    switch( G_itemFits( ent, buildable, dist, origin ) )
    {
      case IBE_NONE:
      case IBE_RPLWARN:
      case IBE_RPTWARN:
      case IBE_SPWNWARN:
      case IBE_NOROOM:
      case IBE_NORMAL:
      case IBE_HOVELEXIT:
        ent->client->ps.stats[ STAT_BUILDABLE ] = ( buildable | SB_VALID_TOGGLEBIT );
        break;

      case IBE_NOASSERT:
        G_TriggerMenu( ent->client->ps.clientNum, MN_A_NOASSERT );
        break;

      case IBE_NOOVERMIND:
        G_TriggerMenu( ent->client->ps.clientNum, MN_A_NOOVMND );
        break;

      case IBE_OVERMIND:
        G_TriggerMenu( ent->client->ps.clientNum, MN_A_OVERMIND );
        break;

      case IBE_REACTOR:
        G_TriggerMenu( ent->client->ps.clientNum, MN_H_REACTOR );
        break;

      case IBE_REPEATER:
        G_TriggerMenu( ent->client->ps.clientNum, MN_H_REPEATER );
        break;

      case IBE_NOPOWER:
        G_TriggerMenu( ent->client->ps.clientNum, MN_H_NOPOWER );
        break;
        
      case IBE_NOCREEP:
        G_TriggerMenu( ent->client->ps.clientNum, MN_A_NOCREEP );
        break;
        
      case IBE_NODCC:
        G_TriggerMenu( ent->client->ps.clientNum, MN_H_NODCC );
        break;
    }
  }
  else
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"Cannot build this item\n\"" ) );

    G_LogPrintf( "Client %d tried to build %d using weapon %d\n",
                 ent->client->ps.clientNum,
                 buildable,
                 ( 1 << ent->client->ps.weapon ) & BG_FindBuildWeaponForBuildable( buildable ) );
  }
}


//TA: so we can print to the console from anywhere
/*
=================
Cmd_Echo_f
=================
*/
void Cmd_Echo_f( gentity_t *ent )
{
  char  s[ MAX_TOKEN_CHARS ];
  
  trap_Argv( 1, s, sizeof( s ) );

  trap_SendServerCommand( ent-g_entities, va( "print \"%s\n\"", s ) );
}


/*
=================
Cmd_Boost_f
=================
*/
void Cmd_Boost_f( gentity_t *ent )
{
  if( BG_gotItem( UP_JETPACK, ent->client->ps.stats ) &&
      BG_activated( UP_JETPACK, ent->client->ps.stats ) )
    return;

  if( ent->client->pers.cmd.buttons & BUTTON_WALKING )
    return;
  
  if( ( ent->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS ) &&
      ( ent->client->ps.stats[ STAT_STAMINA ] > 0 ) )
    ent->client->ps.stats[ STAT_STATE ] |= SS_SPEEDBOOST;
}

/*
=================
Cmd_Test_f
=================
*/
void Cmd_Test_f( gentity_t *ent )
{
  if( !CheatsOk( ent ) )
    return;

/*  ent->client->ps.stats[ STAT_STATE ] |= SS_POISONCLOUDED;
  ent->client->lastPoisonCloudedTime = level.time;
  ent->client->lastPoisonCloudedClient = ent;
  trap_SendServerCommand( ent->client->ps.clientNum, "poisoncloud" );*/
  
  ent->client->ps.stats[ STAT_STATE ] |= SS_POISONED;
  ent->client->lastPoisonTime = level.time;
  ent->client->lastPoisonClient = ent;
}

/*
=================
ClientCommand
=================
*/
void ClientCommand( int clientNum )
{
  gentity_t *ent;
  char      cmd[ MAX_TOKEN_CHARS ];

  ent = g_entities + clientNum;
  if( !ent->client )
    return;   // not fully in game yet

  trap_Argv( 0, cmd, sizeof( cmd ) );

  if( Q_stricmp( cmd, "say" ) == 0 )
  {
    Cmd_Say_f( ent, SAY_ALL, qfalse );
    return;
  }
  
  if( Q_stricmp( cmd, "say_team" ) == 0 )
  {
    Cmd_Say_f( ent, SAY_TEAM, qfalse );
    return;
  }
  
  if( Q_stricmp( cmd, "tell" ) == 0 )
  {
    Cmd_Tell_f( ent );
    return;
  }
  
  if( Q_stricmp( cmd, "score" ) == 0 )
  {
    Cmd_Score_f( ent );
    return;
  }

  // ignore all other commands when at intermission
  if( level.intermissiontime )
  {
    Cmd_Say_f( ent, qfalse, qtrue );
    return;
  }

  if( Q_stricmp( cmd, "give" ) == 0 )
    Cmd_Give_f( ent );
  else if( Q_stricmp( cmd, "god" ) == 0 )
    Cmd_God_f( ent );
  else if( Q_stricmp( cmd, "notarget" ) == 0 )
    Cmd_Notarget_f( ent );
  else if( Q_stricmp( cmd, "noclip" ) == 0 )
    Cmd_Noclip_f( ent );
  else if( Q_stricmp( cmd, "kill" ) == 0 )
    Cmd_Kill_f( ent );
  else if( Q_stricmp( cmd, "levelshot" ) == 0 )
    Cmd_LevelShot_f( ent );
  else if( Q_stricmp( cmd, "team" ) == 0 )
    Cmd_Team_f( ent );
  else if( Q_stricmp( cmd, "class" ) == 0 )
    Cmd_Class_f( ent );
  else if( Q_stricmp( cmd, "build" ) == 0 )
    Cmd_Build_f( ent );
  else if( Q_stricmp( cmd, "buy" ) == 0 )
    Cmd_Buy_f( ent );
  else if( Q_stricmp( cmd, "sell" ) == 0 )
    Cmd_Sell_f( ent );
  else if( Q_stricmp( cmd, "itemact" ) == 0 )
    Cmd_ActivateItem_f( ent );
  else if( Q_stricmp( cmd, "itemdeact" ) == 0 )
    Cmd_DeActivateItem_f( ent );
  else if( Q_stricmp( cmd, "itemtoggle" ) == 0 )
    Cmd_ToggleItem_f( ent );
  else if( Q_stricmp( cmd, "destroy" ) == 0 )
    Cmd_Destroy_f( ent, qfalse );
  else if( Q_stricmp( cmd, "deconstruct" ) == 0 )
    Cmd_Destroy_f( ent, qtrue );
  else if( Q_stricmp( cmd, "echo" ) == 0 )
    Cmd_Echo_f( ent );
  else if( Q_stricmp( cmd, "boost" ) == 0 )
    Cmd_Boost_f( ent );
  else if( Q_stricmp( cmd, "where" ) == 0 )
    Cmd_Where_f( ent );
  else if( Q_stricmp( cmd, "callvote" ) == 0 )
    Cmd_CallVote_f( ent );
  else if( Q_stricmp( cmd, "vote" ) == 0 )
    Cmd_Vote_f( ent );
  else if( Q_stricmp( cmd, "callteamvote" ) == 0 )
    Cmd_CallTeamVote_f( ent );
  else if( Q_stricmp( cmd, "teamvote" ) == 0 )
    Cmd_TeamVote_f( ent );
  else if( Q_stricmp( cmd, "setviewpos" ) == 0 )
    Cmd_SetViewpos_f( ent );
  else if( Q_stricmp( cmd, "test" ) == 0 )
    Cmd_Test_f( ent );
  else
    trap_SendServerCommand( clientNum, va( "print \"unknown cmd %s\n\"", cmd ) );
}
