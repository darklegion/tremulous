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

#include "../../ui/menudef.h"     // for the voice chats

/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage( gentity_t *ent ) {
  char    entry[1024];
  char    string[1400];
  int     stringlength;
  int     i, j;
  gclient_t *cl;
  int     numSorted;
  int     scoreFlags;

  // send the latest information on all clients
  string[0] = 0;
  stringlength = 0;
  scoreFlags = 0;

  numSorted = level.numConnectedClients;

  for (i=0 ; i < numSorted ; i++) {
    int   ping;

    cl = &level.clients[level.sortedClients[i]];

    if ( cl->pers.connected == CON_CONNECTING ) {
      ping = -1;
    } else {
      ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
    }
    Com_sprintf (entry, sizeof(entry),
      " %i %i %i %i %i %i", level.sortedClients[i],
      cl->ps.persistant[PERS_SCORE], ping, (level.time - cl->pers.enterTime)/60000,
      scoreFlags, g_entities[level.sortedClients[i]].s.powerups);
    j = strlen(entry);
    if (stringlength + j > 1024)
      break;
    strcpy (string + stringlength, entry);
    stringlength += j;
  }

  trap_SendServerCommand( ent-g_entities, va("scores %i %i %i%s", i,
    level.teamScores[TEAM_HUMANS], level.teamScores[TEAM_ALIENS],
    string ) );
}


/*
==================
Cmd_Score_f

Request current scoreboard information
==================
*/
void Cmd_Score_f( gentity_t *ent ) {
  DeathmatchScoreboardMessage( ent );
}



/*
==================
CheatsOk
==================
*/
qboolean  CheatsOk( gentity_t *ent ) {
  if ( !g_cheats.integer ) {
    trap_SendServerCommand( ent-g_entities, va("print \"Cheats are not enabled on this server.\n\""));
    return qfalse;
  }
  if ( ent->health <= 0 ) {
    trap_SendServerCommand( ent-g_entities, va("print \"You must be alive to use this command.\n\""));
    return qfalse;
  }
  return qtrue;
}


/*
==================
ConcatArgs
==================
*/
char  *ConcatArgs( int start ) {
  int   i, c, tlen;
  static char line[MAX_STRING_CHARS];
  int   len;
  char  arg[MAX_STRING_CHARS];

  len = 0;
  c = trap_Argc();
  for ( i = start ; i < c ; i++ ) {
    trap_Argv( i, arg, sizeof( arg ) );
    tlen = strlen( arg );
    if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
      break;
    }
    memcpy( line + len, arg, tlen );
    len += tlen;
    if ( i != c - 1 ) {
      line[len] = ' ';
      len++;
    }
  }

  line[len] = 0;

  return line;
}

/*
==================
SanitizeString

Remove case and control characters
==================
*/
void SanitizeString( char *in, char *out ) {
  while ( *in ) {
    if ( *in == 27 ) {
      in += 2;    // skip color code
      continue;
    }
    if ( *in < 32 ) {
      in++;
      continue;
    }
    *out++ = tolower( *in++ );
  }

  *out = 0;
}

/*
==================
ClientNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
int ClientNumberFromString( gentity_t *to, char *s ) {
  gclient_t *cl;
  int     idnum;
  char    s2[MAX_STRING_CHARS];
  char    n2[MAX_STRING_CHARS];

  // numeric values are just slot numbers
  if (s[0] >= '0' && s[0] <= '9') {
    idnum = atoi( s );
    if ( idnum < 0 || idnum >= level.maxclients ) {
      trap_SendServerCommand( to-g_entities, va("print \"Bad client slot: %i\n\"", idnum));
      return -1;
    }

    cl = &level.clients[idnum];
    if ( cl->pers.connected != CON_CONNECTED ) {
      trap_SendServerCommand( to-g_entities, va("print \"Client %i is not active\n\"", idnum));
      return -1;
    }
    return idnum;
  }

  // check for a name match
  SanitizeString( s, s2 );
  for ( idnum=0,cl=level.clients ; idnum < level.maxclients ; idnum++,cl++ ) {
    if ( cl->pers.connected != CON_CONNECTED ) {
      continue;
    }
    SanitizeString( cl->pers.netname, n2 );
    if ( !strcmp( n2, s2 ) ) {
      return idnum;
    }
  }

  trap_SendServerCommand( to-g_entities, va("print \"User %s is not on the server\n\"", s));
  return -1;
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (gentity_t *ent)
{
  char    *name;
  gitem_t   *it;
  int     i;
  qboolean  give_all;
  gentity_t   *it_ent;
  trace_t   trace;

  if ( !CheatsOk( ent ) ) {
    return;
  }

  name = ConcatArgs( 1 );

  if (Q_stricmp(name, "all") == 0)
    give_all = qtrue;
  else
    give_all = qfalse;

  if (give_all || Q_stricmp( name, "health") == 0)
  {
    ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
    if (!give_all)
      return;
  }

  if (give_all || Q_stricmp(name, "weapons") == 0)
  {
    BG_packWeapon( (1 << WP_NUM_WEAPONS) - 1 - ( 1 << WP_GRAPPLING_HOOK ) - ( 1 << WP_NONE ), ent->client->ps.stats );
    if (!give_all)
      return;
  }

  if (give_all || Q_stricmp(name, "ammo") == 0)
  {
    for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
      BG_packAmmoArray( i, ent->client->ps.ammo, ent->client->ps.powerups, 999, 0, 0 );
    }
    if (!give_all)
      return;
  }

  if (give_all || Q_stricmp(name, "armor") == 0)
  {
    ent->client->ps.stats[STAT_ARMOR] = 200;

    if (!give_all)
      return;
  }

  if( give_all || Q_stricmp( name, "funds" ) == 0 )
  {
    ent->client->ps.stats[ STAT_CREDIT ] += 100;

    if( !give_all )
      return;
  }

  // spawn a specific item right on the player
/*  if ( !give_all ) {
    it = BG_FindItem (name);
    if (!it) {
      return;
    }

    it_ent = G_Spawn();
    VectorCopy( ent->r.currentOrigin, it_ent->s.origin );
    it_ent->classname = it->classname;
    G_SpawnItem (it_ent, it);
    FinishSpawningItem(it_ent );
    memset( &trace, 0, sizeof( trace ) );
    Touch_Item (it_ent, ent, &trace);
    if (it_ent->inuse) {
      G_FreeEntity( it_ent );
    }
  }*/
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (gentity_t *ent)
{
  char  *msg;

  if ( !CheatsOk( ent ) ) {
    return;
  }

  ent->flags ^= FL_GODMODE;
  if (!(ent->flags & FL_GODMODE) )
    msg = "godmode OFF\n";
  else
    msg = "godmode ON\n";

  trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f( gentity_t *ent ) {
  char  *msg;

  if ( !CheatsOk( ent ) ) {
    return;
  }

  ent->flags ^= FL_NOTARGET;
  if (!(ent->flags & FL_NOTARGET) )
    msg = "notarget OFF\n";
  else
    msg = "notarget ON\n";

  trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( gentity_t *ent ) {
  char  *msg;

  if ( !CheatsOk( ent ) ) {
    return;
  }

  if ( ent->client->noclip ) {
    msg = "noclip OFF\n";
  } else {
    msg = "noclip ON\n";
  }
  ent->client->noclip = !ent->client->noclip;

  trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
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
void Cmd_LevelShot_f( gentity_t *ent ) {
  if ( !CheatsOk( ent ) ) {
    return;
  }

  // doesn't work in single player
  if ( g_gametype.integer != 0 ) {
    trap_SendServerCommand( ent-g_entities,
      "print \"Must be in g_gametype 0 for levelshot\n\"" );
    return;
  }

  BeginIntermission();
  trap_SendServerCommand( ent-g_entities, "clientLevelShot" );
}


void Cmd_TeamTask_f( gentity_t *ent ) {
  char userinfo[MAX_INFO_STRING];
  char    arg[MAX_TOKEN_CHARS];
  int task;
  int client = ent->client - level.clients;

  if ( trap_Argc() != 2 ) {
    return;
  }
  trap_Argv( 1, arg, sizeof( arg ) );
  task = atoi( arg );

  trap_GetUserinfo(client, userinfo, sizeof(userinfo));
  Info_SetValueForKey(userinfo, "teamtask", va("%d", task));
  trap_SetUserinfo(client, userinfo);
  ClientUserinfoChanged(client);
}


/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( gentity_t *ent ) {
  if( ent->client->sess.sessionTeam == TEAM_SPECTATOR )
    return;
  
  if( ent->client->ps.stats[ STAT_STATE ] & SS_INFESTING )
    return;
    
  if( ent->client->ps.stats[ STAT_STATE ] & SS_HOVELING )
    return;
    
  if (ent->health <= 0)
    return;

  ent->flags &= ~FL_GODMODE;
  ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
  player_die (ent, ent, ent, 100000, MOD_SUICIDE);
}

/*
=================
BroadCastTeamChange

Let everyone know about a team change
=================
*/
void BroadcastTeamChange( gclient_t *client, int oldTeam )
{
  if ( client->sess.sessionTeam == TEAM_HUMANS ) {
    trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the humans.\n\"",
      client->pers.netname) );
  } else if ( client->sess.sessionTeam == TEAM_ALIENS ) {
    trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the aliens.\n\"",
    client->pers.netname));
  } else if ( client->sess.sessionTeam == TEAM_SPECTATOR && oldTeam != TEAM_SPECTATOR ) {
    trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the spectators.\n\"",
    client->pers.netname));
  } else if ( client->sess.sessionTeam == TEAM_FREE ) {
    trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the battle.\n\"",
    client->pers.netname));
  }
}

/*
=================
SetTeam
=================
*/
void SetTeam( gentity_t *ent, char *s ) {
  int         team, oldTeam;
  gclient_t     *client;
  int         clientNum;
  spectatorState_t  specState;
  int         specClient;

  //
  // see what change is requested
  //
  client = ent->client;

  clientNum = client - level.clients;
  specClient = 0;

  specState = SPECTATOR_NOT;
  if ( !Q_stricmp( s, "scoreboard" ) || !Q_stricmp( s, "score" )  ) {
    team = TEAM_SPECTATOR;
    specState = SPECTATOR_SCOREBOARD;
  } else if ( !Q_stricmp( s, "follow1" ) ) {
    team = TEAM_SPECTATOR;
    specState = SPECTATOR_FOLLOW;
    specClient = -1;
  } else if ( !Q_stricmp( s, "follow2" ) ) {
    team = TEAM_SPECTATOR;
    specState = SPECTATOR_FOLLOW;
    specClient = -2;
  } else if ( !Q_stricmp( s, "spectator" ) || !Q_stricmp( s, "s" ) ) {
    team = TEAM_SPECTATOR;
    specState = SPECTATOR_FREE;
  } else if ( g_gametype.integer >= GT_TEAM ) {
    // if running a team game, assign player to one of the teams
    specState = SPECTATOR_NOT;
    if ( !Q_stricmp( s, "humans" ) || !Q_stricmp( s, "h" ) ) {
      team = TEAM_HUMANS;
    } else if ( !Q_stricmp( s, "aliens" ) || !Q_stricmp( s, "d" ) ) {
      team = TEAM_ALIENS;
    } else {
      // pick the team with the least number of players
      team = PickTeam( clientNum );
    }

    if ( g_teamForceBalance.integer ) {
      int   counts[TEAM_NUM_TEAMS];

      counts[TEAM_ALIENS] = TeamCount( ent->client->ps.clientNum, TEAM_ALIENS );
      counts[TEAM_HUMANS] = TeamCount( ent->client->ps.clientNum, TEAM_HUMANS );

      // We allow a spread of two
      if ( team == TEAM_HUMANS && counts[TEAM_HUMANS] - counts[TEAM_ALIENS] > 1 ) {
        trap_SendServerCommand( ent->client->ps.clientNum,
          "cp \"Humans team has too many players.\n\"" );
        return; // ignore the request
      }
      if ( team == TEAM_ALIENS && counts[TEAM_ALIENS] - counts[TEAM_HUMANS] > 1 ) {
        trap_SendServerCommand( ent->client->ps.clientNum,
          "cp \"Aliens team has too many players.\n\"" );
        return; // ignore the request
      }

      // It's ok, the team we are switching to has less or same number of players
    }

  } else {
    // force them to spectators if there aren't any spots free
    team = TEAM_FREE;
  }

  // override decision if limiting the players
  if ( (g_gametype.integer == GT_TOURNAMENT)
    && level.numNonSpectatorClients >= 2 ) {
    team = TEAM_SPECTATOR;
  } else if ( g_maxGameClients.integer > 0 &&
    level.numNonSpectatorClients >= g_maxGameClients.integer ) {
    team = TEAM_SPECTATOR;
  }

  //
  // decide if we will allow the change
  //
  oldTeam = client->sess.sessionTeam;
  if ( team == oldTeam && team != TEAM_SPECTATOR ) {
    return;
  }

  //
  // execute the team change
  //

  // he starts at 'base'
  client->pers.teamState.state = TEAM_BEGIN;
  if ( oldTeam != TEAM_SPECTATOR ) {
    // Kill him (makes sure he loses flags, etc)
    ent->flags &= ~FL_GODMODE;
    ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
    player_die (ent, ent, ent, 100000, MOD_SUICIDE);

  }
  // they go to the end of the line for tournements
  if ( team == TEAM_SPECTATOR ) {
    client->sess.spectatorTime = level.time;
  }

  client->sess.sessionTeam = team;
  client->sess.spectatorState = specState;
  client->sess.spectatorClient = specClient;

  BroadcastTeamChange( client, oldTeam );

  // get and distribute relevent paramters
  ClientUserinfoChanged( clientNum );

  ClientBegin( clientNum );
}

/*
=================
StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
void StopFollowing( gentity_t *ent ) {
  ent->client->ps.persistant[ PERS_TEAM ] = TEAM_SPECTATOR;
  ent->client->sess.sessionTeam = TEAM_SPECTATOR;
  ent->client->sess.spectatorState = SPECTATOR_FREE;
  ent->client->ps.pm_flags &= ~PMF_FOLLOW;
  ent->r.svFlags &= ~SVF_BOT;
  ent->client->ps.clientNum = ent - g_entities;
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f( gentity_t *ent ) {
  int     oldTeam;
  char    s[MAX_TOKEN_CHARS];

  //TA: rip out the q3a team system :)

  oldTeam = ent->client->pers.pteam;

  trap_Argv( 1, s, sizeof( s ) );

  if( !strlen( s ) )
  {
    trap_SendServerCommand( ent-g_entities, va("print \"team: %i\n\"", ent->client->pers.pteam ) );
    return;
  }

  if( !Q_stricmp( s, "0" ) || !Q_stricmp( s, "spectate" ) )
    ent->client->pers.pteam = PTE_NONE;
  else if( !Q_stricmp( s, "1" ) || !Q_stricmp( s, "aliens" ) )
    ent->client->pers.pteam = PTE_ALIENS;
  else if( !Q_stricmp( s, "2" ) || !Q_stricmp( s, "humans" ) )
    ent->client->pers.pteam = PTE_HUMANS;

  if( oldTeam != ent->client->pers.pteam )
  {
    ent->client->pers.pclass = 0;
    ClientSpawn( ent, NULL );
  }

  //FIXME: put some team change broadcast code here.
}


/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f( gentity_t *ent ) {
  int   i;
  char  arg[MAX_TOKEN_CHARS];

  if ( trap_Argc() != 2 ) {
    if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
      StopFollowing( ent );
    }
    return;
  }

  trap_Argv( 1, arg, sizeof( arg ) );
  i = ClientNumberFromString( ent, arg );
  if ( i == -1 ) {
    return;
  }

  // can't follow self
  if ( &level.clients[ i ] == ent->client ) {
    return;
  }

  // can't follow another spectator
  if ( level.clients[ i ].sess.sessionTeam == TEAM_SPECTATOR ) {
    return;
  }

  // if they are playing a tournement game, count as a loss
  if ( ( g_gametype.integer == GT_TOURNAMENT )
      && ent->client->sess.sessionTeam == TEAM_FREE ) {
    ent->client->sess.losses++;
  }

  // first set them to spectator
  if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
    SetTeam( ent, "spectator" );
  }

  ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
  ent->client->sess.spectatorClient = i;
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_FollowCycle_f( gentity_t *ent, int dir ) {
  int   clientnum;
  int   original;

  // if they are playing a tournement game, count as a loss
  if ( ( g_gametype.integer == GT_TOURNAMENT )
      && ent->client->sess.sessionTeam == TEAM_FREE ) {
    ent->client->sess.losses++;
  }
  // first set them to spectator
  if ( ent->client->sess.spectatorState == SPECTATOR_NOT ) {
    SetTeam( ent, "spectator" );
  }

  if ( dir != 1 && dir != -1 ) {
    G_Error( "Cmd_FollowCycle_f: bad dir %i", dir );
  }

  clientnum = ent->client->sess.spectatorClient;
  original = clientnum;
  do {
    clientnum += dir;
    if ( clientnum >= level.maxclients ) {
      clientnum = 0;
    }
    if ( clientnum < 0 ) {
      clientnum = level.maxclients - 1;
    }

    // can only follow connected clients
    if ( level.clients[ clientnum ].pers.connected != CON_CONNECTED ) {
      continue;
    }

    // can't follow another spectator
    if ( level.clients[ clientnum ].sess.sessionTeam == TEAM_SPECTATOR ) {
      continue;
    }

    // this is good, we can use it
    ent->client->sess.spectatorClient = clientnum;
    ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
    return;
  } while ( clientnum != original );

  // leave it where it was
}


/*
==================
G_Say
==================
*/
static void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message ) {
  if (!other) {
    return;
  }
  if (!other->inuse) {
    return;
  }
  if (!other->client) {
    return;
  }
  if ( other->client->pers.connected != CON_CONNECTED ) {
    return;
  }
  if ( mode == SAY_TEAM  && !OnSameTeam(ent, other) ) {
    return;
  }
  // no chatting to players in tournements
  if ( ( g_gametype.integer == GT_TOURNAMENT )
    && other->client->sess.sessionTeam == TEAM_FREE
    && ent->client->sess.sessionTeam != TEAM_FREE ) {
    return;
  }

  trap_SendServerCommand( other-g_entities, va("%s \"%s%c%c%s\"",
    mode == SAY_TEAM ? "tchat" : "chat",
    name, Q_COLOR_ESCAPE, color, message));
}

#define EC    "\x19"

void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText ) {
  int     j;
  gentity_t *other;
  int     color;
  char    name[64];
  // don't let text be too long for malicious reasons
  char    text[MAX_SAY_TEXT];
  char    location[64];

  if ( g_gametype.integer < GT_TEAM && mode == SAY_TEAM ) {
    mode = SAY_ALL;
  }

  switch ( mode ) {
  default:
  case SAY_ALL:
    G_LogPrintf( "say: %s: %s\n", ent->client->pers.netname, chatText );
    Com_sprintf (name, sizeof(name), "%s%c%c"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
    color = COLOR_GREEN;
    break;
  case SAY_TEAM:
    G_LogPrintf( "sayteam: %s: %s\n", ent->client->pers.netname, chatText );
    if (Team_GetLocationMsg(ent, location, sizeof(location)))
      Com_sprintf (name, sizeof(name), EC"(%s%c%c"EC") (%s)"EC": ", 
        ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, location);
    else
      Com_sprintf (name, sizeof(name), EC"(%s%c%c"EC")"EC": ", 
        ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
    color = COLOR_CYAN;
    break;
  case SAY_TELL:
    if (target && g_gametype.integer >= GT_TEAM &&
      target->client->sess.sessionTeam == ent->client->sess.sessionTeam &&
      Team_GetLocationMsg(ent, location, sizeof(location)))
      Com_sprintf (name, sizeof(name), EC"[%s%c%c"EC"] (%s)"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, location );
    else
      Com_sprintf (name, sizeof(name), EC"[%s%c%c"EC"]"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
    color = COLOR_MAGENTA;
    break;
  }

  Q_strncpyz( text, chatText, sizeof(text) );

  if ( target ) {
    G_SayTo( ent, target, mode, color, name, text );
    return;
  }

  // echo the text to the console
  if ( g_dedicated.integer ) {
    G_Printf( "%s%s\n", name, text);
  }

  // send it to all the apropriate clients
  for (j = 0; j < level.maxclients; j++) {
    other = &g_entities[j];
    G_SayTo( ent, other, mode, color, name, text );
  }
}


/*
==================
Cmd_Say_f
==================
*/
static void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 ) {
  char    *p;

  if ( trap_Argc () < 2 && !arg0 ) {
    return;
  }

  if (arg0)
  {
    p = ConcatArgs( 0 );
  }
  else
  {
    p = ConcatArgs( 1 );
  }

  G_Say( ent, NULL, mode, p );
}

/*
==================
Cmd_Tell_f
==================
*/
static void Cmd_Tell_f( gentity_t *ent ) {
  int     targetNum;
  gentity_t *target;
  char    *p;
  char    arg[MAX_TOKEN_CHARS];

  if ( trap_Argc () < 2 ) {
    return;
  }

  trap_Argv( 1, arg, sizeof( arg ) );
  targetNum = atoi( arg );
  if ( targetNum < 0 || targetNum >= level.maxclients ) {
    return;
  }

  target = &g_entities[targetNum];
  if ( !target || !target->inuse || !target->client ) {
    return;
  }

  p = ConcatArgs( 2 );

  G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, p );
  G_Say( ent, target, SAY_TELL, p );
  // don't tell to the player self if it was already directed to this player
  // also don't send the chat back to a bot
  if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
    G_Say( ent, ent, SAY_TELL, p );
  }
}


static void G_VoiceTo( gentity_t *ent, gentity_t *other, int mode, const char *id, qboolean voiceonly ) {
  int color;
  char *cmd;

  if (!other) {
    return;
  }
  if (!other->inuse) {
    return;
  }
  if (!other->client) {
    return;
  }
  if ( mode == SAY_TEAM && !OnSameTeam(ent, other) ) {
    return;
  }
  // no chatting to players in tournements
  if ( (g_gametype.integer == GT_TOURNAMENT )) {
    return;
  }

  if (mode == SAY_TEAM) {
    color = COLOR_CYAN;
    cmd = "vtchat";
  }
  else if (mode == SAY_TELL) {
    color = COLOR_MAGENTA;
    cmd = "vtell";
  }
  else {
    color = COLOR_GREEN;
    cmd = "vchat";
  }

  trap_SendServerCommand( other-g_entities, va("%s %d %d %d %s", cmd, voiceonly, ent->s.number, color, id));
}


void G_Voice( gentity_t *ent, gentity_t *target, int mode, const char *id, qboolean voiceonly ) {
  int     j;
  gentity_t *other;

  if ( g_gametype.integer < GT_TEAM && mode == SAY_TEAM ) {
    mode = SAY_ALL;
  }

  if ( target ) {
    G_VoiceTo( ent, target, mode, id, voiceonly );
    return;
  }

  // echo the text to the console
  if ( g_dedicated.integer ) {
    G_Printf( "voice: %s %s\n", ent->client->pers.netname, id);
  }

  // send it to all the apropriate clients
  for (j = 0; j < level.maxclients; j++) {
    other = &g_entities[j];
    G_VoiceTo( ent, other, mode, id, voiceonly );
  }
}


/*
==================
Cmd_Voice_f
==================
*/
static void Cmd_Voice_f( gentity_t *ent, int mode, qboolean arg0, qboolean voiceonly ) {
  char    *p;

  if ( trap_Argc () < 2 && !arg0 ) {
    return;
  }

  if (arg0)
  {
    p = ConcatArgs( 0 );
  }
  else
  {
    p = ConcatArgs( 1 );
  }

  G_Voice( ent, NULL, mode, p, voiceonly );
}


/*
==================
Cmd_VoiceTell_f
==================
*/
static void Cmd_VoiceTell_f( gentity_t *ent, qboolean voiceonly ) {
  int     targetNum;
  gentity_t *target;
  char    *id;
  char    arg[MAX_TOKEN_CHARS];

  if ( trap_Argc () < 2 ) {
    return;
  }

  trap_Argv( 1, arg, sizeof( arg ) );
  targetNum = atoi( arg );
  if ( targetNum < 0 || targetNum >= level.maxclients ) {
    return;
  }

  target = &g_entities[targetNum];
  if ( !target || !target->inuse || !target->client ) {
    return;
  }

  id = ConcatArgs( 2 );

  G_LogPrintf( "vtell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, id );
  G_Voice( ent, target, SAY_TELL, id, voiceonly );
  // don't tell to the player self if it was already directed to this player
  // also don't send the chat back to a bot
  if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
    G_Voice( ent, ent, SAY_TELL, id, voiceonly );
  }
}


/*
==================
Cmd_VoiceTaunt_f
==================
*/
static void Cmd_VoiceTaunt_f( gentity_t *ent ) {
  gentity_t *who;
  int i;

  if (!ent->client) {
    return;
  }

  // insult someone who just killed you
  if (ent->enemy && ent->enemy->client && ent->enemy->client->lastkilled_client == ent->s.number) {
    // i am a dead corpse
    if (!(ent->enemy->r.svFlags & SVF_BOT)) {
      //G_Voice( ent, ent->enemy, SAY_TELL, VOICECHAT_DEATHINSULT, qfalse );
    }
    if (!(ent->r.svFlags & SVF_BOT)) {
      //G_Voice( ent, ent,        SAY_TELL, VOICECHAT_DEATHINSULT, qfalse );
    }
    ent->enemy = NULL;
    return;
  }
  // insult someone you just killed
  if (ent->client->lastkilled_client >= 0 && ent->client->lastkilled_client != ent->s.number) {
    who = g_entities + ent->client->lastkilled_client;
    if (who->client) {
      // who is the person I just killed
      if (who->client->lasthurt_mod == MOD_GAUNTLET) {
        if (!(who->r.svFlags & SVF_BOT)) {
          //G_Voice( ent, who, SAY_TELL, VOICECHAT_KILLGAUNTLET, qfalse );  // and I killed them with a gauntlet
        }
        if (!(ent->r.svFlags & SVF_BOT)) {
          //G_Voice( ent, ent, SAY_TELL, VOICECHAT_KILLGAUNTLET, qfalse );
        }
      } else {
        if (!(who->r.svFlags & SVF_BOT)) {
          //G_Voice( ent, who, SAY_TELL, VOICECHAT_KILLINSULT, qfalse );  // and I killed them with something else
        }
        if (!(ent->r.svFlags & SVF_BOT)) {
          //G_Voice( ent, ent, SAY_TELL, VOICECHAT_KILLINSULT, qfalse );
        }
      }
      ent->client->lastkilled_client = -1;
      return;
    }
  }

  if (g_gametype.integer >= GT_TEAM) {
    // praise a team mate who just got a reward
    for(i = 0; i < MAX_CLIENTS; i++) {
      who = g_entities + i;
      if (who->client && who != ent && who->client->sess.sessionTeam == ent->client->sess.sessionTeam) {
        if (who->client->rewardTime > level.time) {
          if (!(who->r.svFlags & SVF_BOT)) {
            //G_Voice( ent, who, SAY_TELL, VOICECHAT_PRAISE, qfalse );
          }
          if (!(ent->r.svFlags & SVF_BOT)) {
            //G_Voice( ent, ent, SAY_TELL, VOICECHAT_PRAISE, qfalse );
          }
          return;
        }
      }
    }
  }

  // just say something
  //G_Voice( ent, NULL, SAY_ALL, VOICECHAT_TAUNT, qfalse );
}


static char *gc_orders[] = {
  "hold your position",
  "hold this position",
  "come here",
  "cover me",
  "guard location",
  "search and destroy",
  "report"
};

void Cmd_GameCommand_f( gentity_t *ent ) {
  int   player;
  int   order;
  char  str[MAX_TOKEN_CHARS];

  trap_Argv( 1, str, sizeof( str ) );
  player = atoi( str );
  trap_Argv( 2, str, sizeof( str ) );
  order = atoi( str );

  if ( player < 0 || player >= MAX_CLIENTS ) {
    return;
  }
  if ( order < 0 || order > sizeof(gc_orders)/sizeof(char *) ) {
    return;
  }
  G_Say( ent, &g_entities[player], SAY_TELL, gc_orders[order] );
  G_Say( ent, ent, SAY_TELL, gc_orders[order] );
}

/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f( gentity_t *ent ) {
  trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos( ent->s.origin ) ) );
}

static const char *gameNames[] = {
  "Free For All",
  "Tournament",
  "Single Player",
  "Team Deathmatch",
  "Capture the Flag",
  "One Flag CTF",
  "Overload",
  "Harvester"
};

/*
==================
Cmd_CallVote_f
==================
*/
void Cmd_CallVote_f( gentity_t *ent ) {
  int   i;
  char  arg1[MAX_STRING_TOKENS];
  char  arg2[MAX_STRING_TOKENS];

  if ( !g_allowVote.integer ) {
    trap_SendServerCommand( ent-g_entities, "print \"Voting not allowed here.\n\"" );
    return;
  }

  if ( level.voteTime ) {
    trap_SendServerCommand( ent-g_entities, "print \"A vote is already in progress.\n\"" );
    return;
  }
  if ( ent->client->pers.voteCount >= MAX_VOTE_COUNT ) {
    trap_SendServerCommand( ent-g_entities, "print \"You have called the maximum number of votes.\n\"" );
    return;
  }
  if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
    trap_SendServerCommand( ent-g_entities, "print \"Not allowed to call a vote as spectator.\n\"" );
    return;
  }

  // make sure it is a valid command to vote on
  trap_Argv( 1, arg1, sizeof( arg1 ) );
  trap_Argv( 2, arg2, sizeof( arg2 ) );

  if( strchr( arg1, ';' ) || strchr( arg2, ';' ) ) {
    trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
    return;
  }

  if ( !Q_stricmp( arg1, "map_restart" ) ) {
  } else if ( !Q_stricmp( arg1, "nextmap" ) ) {
  } else if ( !Q_stricmp( arg1, "map" ) ) {
  } else if ( !Q_stricmp( arg1, "g_gametype" ) ) {
  } else if ( !Q_stricmp( arg1, "kick" ) ) {
  } else if ( !Q_stricmp( arg1, "clientkick" ) ) {
  } else if ( !Q_stricmp( arg1, "g_doWarmup" ) ) {
  } else if ( !Q_stricmp( arg1, "timelimit" ) ) {
  } else if ( !Q_stricmp( arg1, "fraglimit" ) ) {
  } else {
    trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
    trap_SendServerCommand( ent-g_entities, "print \"Vote commands are: map_restart, nextmap, map <mapname>, g_gametype <n>, kick <player>, clientkick <clientnum>, g_doWarmup, timelimit <time>, fraglimit <frags>.\n\"" );
    return;
  }

  // if there is still a vote to be executed
  if ( level.voteExecuteTime ) {
    level.voteExecuteTime = 0;
    trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.voteString ) );
  }

  // special case for g_gametype, check for bad values
  if ( !Q_stricmp( arg1, "g_gametype" ) ) {
    i = atoi( arg2 );
    if( i == GT_SINGLE_PLAYER || i < GT_FFA || i >= GT_MAX_GAME_TYPE) {
      trap_SendServerCommand( ent-g_entities, "print \"Invalid gametype.\n\"" );
      return;
    }

    Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %d", arg1, i );
    Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %s", arg1, gameNames[i] );
  } else if ( !Q_stricmp( arg1, "map" ) ) {
    // special case for map changes, we want to reset the nextmap setting
    // this allows a player to change maps, but not upset the map rotation
    char  s[MAX_STRING_CHARS];

    trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
    if (*s) {
      Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s; set nextmap \"%s\"", arg1, arg2, s );
    } else {
      Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );
    }
    Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
  } else if ( !Q_stricmp( arg1, "nextmap" ) ) {
    char  s[MAX_STRING_CHARS];

    trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
    if (!*s) {
      trap_SendServerCommand( ent-g_entities, "print \"nextmap not set.\n\"" );
      return;
    }
    Com_sprintf( level.voteString, sizeof( level.voteString ), "vstr nextmap");
    Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
  } else {
    Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
    Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
  }

  trap_SendServerCommand( -1, va("print \"%s called a vote.\n\"", ent->client->pers.netname ) );

  // start the voting, the caller autoamtically votes yes
  level.voteTime = level.time;
  level.voteYes = 1;
  level.voteNo = 0;

  for ( i = 0 ; i < level.maxclients ; i++ ) {
    level.clients[i].ps.eFlags &= ~EF_VOTED;
  }
  ent->client->ps.eFlags |= EF_VOTED;

  trap_SetConfigstring( CS_VOTE_TIME, va("%i", level.voteTime ) );
  trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );
  trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
  trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
}

/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f( gentity_t *ent ) {
  char    msg[64];

  if ( !level.voteTime ) {
    trap_SendServerCommand( ent-g_entities, "print \"No vote in progress.\n\"" );
    return;
  }
  if ( ent->client->ps.eFlags & EF_VOTED ) {
    trap_SendServerCommand( ent-g_entities, "print \"Vote already cast.\n\"" );
    return;
  }

  if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
    trap_SendServerCommand( ent-g_entities, "print \"Not allowed to vote as spectator.\n\"" );
    return;
  }

  trap_SendServerCommand( ent-g_entities, "print \"Vote cast.\n\"" );

  ent->client->ps.eFlags |= EF_VOTED;

  trap_Argv( 1, msg, sizeof( msg ) );

  if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
    level.voteYes++;
    trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
  } else {
    level.voteNo++;
    trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
  }

  // a majority will be determined in G_CheckVote, which will also account
  // for players entering or leaving
}

/*
==================
Cmd_CallTeamVote_f
==================
*/
void Cmd_CallTeamVote_f( gentity_t *ent ) {
  int   i, team, cs_offset;
  char  arg1[MAX_STRING_TOKENS];
  char  arg2[MAX_STRING_TOKENS];

  team = ent->client->sess.sessionTeam;
  if ( team == TEAM_HUMANS )
    cs_offset = 0;
  else if ( team == TEAM_ALIENS )
    cs_offset = 1;
  else
    return;

  if ( !g_allowVote.integer ) {
    trap_SendServerCommand( ent-g_entities, "print \"Voting not allowed here.\n\"" );
    return;
  }

  if ( level.teamVoteTime[cs_offset] ) {
    trap_SendServerCommand( ent-g_entities, "print \"A team vote is already in progress.\n\"" );
    return;
  }
  if ( ent->client->pers.teamVoteCount >= MAX_VOTE_COUNT ) {
    trap_SendServerCommand( ent-g_entities, "print \"You have called the maximum number of team votes.\n\"" );
    return;
  }
  if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
    trap_SendServerCommand( ent-g_entities, "print \"Not allowed to call a vote as spectator.\n\"" );
    return;
  }

  // make sure it is a valid command to vote on
  trap_Argv( 1, arg1, sizeof( arg1 ) );
  arg2[0] = '\0';
  for ( i = 2; i < trap_Argc(); i++ ) {
    if (i > 2)
      strcat(arg2, " ");
    trap_Argv( i, &arg2[strlen(arg2)], sizeof( arg2 ) - strlen(arg2) );
  }

  if( strchr( arg1, ';' ) || strchr( arg2, ';' ) ) {
    trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
    return;
  }


  if ( !Q_stricmp( arg1, "leader" ) ) {
    char netname[MAX_NETNAME], leader[MAX_NETNAME];

    if ( !arg2[0] ) {
      i = ent->client->ps.clientNum;
    }
    else {
      // numeric values are just slot numbers
      for (i = 0; i < 3; i++) {
        if ( !arg2[i] || arg2[i] < '0' || arg2[i] > '9' )
          break;
      }
      if ( i >= 3 || !arg2[i]) {
        i = atoi( arg2 );
        if ( i < 0 || i >= level.maxclients ) {
          trap_SendServerCommand( ent-g_entities, va("print \"Bad client slot: %i\n\"", i) );
          return;
        }

        if ( !g_entities[i].inuse ) {
          trap_SendServerCommand( ent-g_entities, va("print \"Client %i is not active\n\"", i) );
          return;
        }
      }
      else {
        Q_strncpyz(leader, arg2, sizeof(leader));
        Q_CleanStr(leader);
        for ( i = 0 ; i < level.maxclients ; i++ ) {
          if ( level.clients[i].pers.connected == CON_DISCONNECTED )
            continue;
          if (level.clients[i].sess.sessionTeam != team)
            continue;
          Q_strncpyz(netname, level.clients[i].pers.netname, sizeof(netname));
          Q_CleanStr(netname);
          if ( !Q_stricmp(netname, leader) ) {
            break;
          }
        }
        if ( i >= level.maxclients ) {
          trap_SendServerCommand( ent-g_entities, va("print \"%s is not a valid player on your team.\n\"", arg2) );
          return;
        }
      }
    }
    Com_sprintf(arg2, sizeof(arg2), "%d", i);
  } else {
    trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
    trap_SendServerCommand( ent-g_entities, "print \"Team vote commands are: leader <player>.\n\"" );
    return;
  }

  Com_sprintf( level.teamVoteString[cs_offset], sizeof( level.teamVoteString[cs_offset] ), "%s %s", arg1, arg2 );

  for ( i = 0 ; i < level.maxclients ; i++ ) {
    if ( level.clients[i].pers.connected == CON_DISCONNECTED )
      continue;
    if (level.clients[i].sess.sessionTeam == team)
      trap_SendServerCommand( i, va("print \"%s called a team vote.\n\"", ent->client->pers.netname ) );
  }

  // start the voting, the caller autoamtically votes yes
  level.teamVoteTime[cs_offset] = level.time;
  level.teamVoteYes[cs_offset] = 1;
  level.teamVoteNo[cs_offset] = 0;

  for ( i = 0 ; i < level.maxclients ; i++ ) {
    if (level.clients[i].sess.sessionTeam == team)
      level.clients[i].ps.eFlags &= ~EF_TEAMVOTED;
  }
  ent->client->ps.eFlags |= EF_TEAMVOTED;

  trap_SetConfigstring( CS_TEAMVOTE_TIME + cs_offset, va("%i", level.teamVoteTime[cs_offset] ) );
  trap_SetConfigstring( CS_TEAMVOTE_STRING + cs_offset, level.teamVoteString[cs_offset] );
  trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
  trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );
}


/*
==================
Cmd_TeamVote_f
==================
*/
void Cmd_TeamVote_f( gentity_t *ent ) {
  int     team, cs_offset;
  char    msg[64];

  team = ent->client->sess.sessionTeam;
  if ( team == TEAM_HUMANS )
    cs_offset = 0;
  else if ( team == TEAM_ALIENS )
    cs_offset = 1;
  else
    return;

  if ( !level.teamVoteTime[cs_offset] ) {
    trap_SendServerCommand( ent-g_entities, "print \"No team vote in progress.\n\"" );
    return;
  }
  if ( ent->client->ps.eFlags & EF_TEAMVOTED ) {
    trap_SendServerCommand( ent-g_entities, "print \"Team vote already cast.\n\"" );
    return;
  }
  if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
    trap_SendServerCommand( ent-g_entities, "print \"Not allowed to vote as spectator.\n\"" );
    return;
  }

  trap_SendServerCommand( ent-g_entities, "print \"Team vote cast.\n\"" );

  ent->client->ps.eFlags |= EF_TEAMVOTED;

  trap_Argv( 1, msg, sizeof( msg ) );

  if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
    level.teamVoteYes[cs_offset]++;
    trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
  } else {
    level.teamVoteNo[cs_offset]++;
    trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) ); 
  }

  // a majority will be determined in TeamCheckVote, which will also account
  // for players entering or leaving
}


/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f( gentity_t *ent ) {
  vec3_t    origin, angles;
  char    buffer[MAX_TOKEN_CHARS];
  int     i;

  if ( !g_cheats.integer ) {
    trap_SendServerCommand( ent-g_entities, va("print \"Cheats are not enabled on this server.\n\""));
    return;
  }
  if ( trap_Argc() != 5 ) {
    trap_SendServerCommand( ent-g_entities, va("print \"usage: setviewpos x y z yaw\n\""));
    return;
  }

  VectorClear( angles );
  for ( i = 0 ; i < 3 ; i++ ) {
    trap_Argv( i + 1, buffer, sizeof( buffer ) );
    origin[i] = atof( buffer );
  }

  trap_Argv( 4, buffer, sizeof( buffer ) );
  angles[YAW] = atof( buffer );

  TeleportPlayer( ent, origin, angles );
}


/*
=================
Cmd_Stats_f
=================
*/
void Cmd_Stats_f( gentity_t *ent ) {
/*
  int max, n, i;

  max = trap_AAS_PointReachabilityAreaIndex( NULL );

  n = 0;
  for ( i = 0; i < max; i++ ) {
    if ( ent->client->areabits[i >> 3] & (1 << (i & 7)) )
      n++;
  }

  //trap_SendServerCommand( ent-g_entities, va("print \"visited %d of %d areas\n\"", n, max));
  trap_SendServerCommand( ent-g_entities, va("print \"%d%% level coverage\n\"", n * 100 / max));
*/
}


/*
=================
Cmd_TorchOff_f
=================
*/
void Cmd_TorchOff_f( gentity_t *ent )
{
  G_FreeEntity( ent->client->torch );
  trap_LinkEntity( ent->client->torch );
  ent->client->torch = NULL;
}

#define NUM_AC  3

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
  gentity_t *body, *victim;
  vec3_t    distance;
  int       length = 4096;
  int       i;
  trace_t   tr;
  vec3_t    infestOrigin, infestAngles;
  int       allowedClasses[ NUM_AC ] = {  PCL_A_B_BASE,
                                          PCL_A_B_LEV1,
                                          PCL_A_O_BASE };

  clientNum = ent->client - level.clients;
  trap_Argv( 1, s, sizeof( s ) );

  if( ent->client->pers.pteam == PTE_ALIENS &&
      !( ent->client->ps.stats[ STAT_STATE ] & SS_INFESTING ) &&
      !( ent->client->ps.stats[ STAT_STATE ] & SS_HOVELING ) )
  {
    //if we are not currently spectating, we are attempting evolution
    if( ent->client->pers.pclass != PCL_NONE )
    {
      for ( i = 1, body = g_entities + i; i < level.num_entities; i++, body++ )
      {
        if( !Q_stricmp( body->classname, "humanCorpse" ) )
        {
          VectorSubtract( ent->s.pos.trBase, body->s.origin, distance );
          if( VectorLength( distance ) <= length )
          {
            length = VectorLength( distance );
            victim = body;
          }
        }
      }
      
      //if a human corpse is nearby...
      if( length <= 200 )
      {
        ent->client->pers.pclass = BG_FindClassNumForName( s );

        //...check we can evolve to that class
        if( !BG_ClassCanEvolveFromTo( ent->client->ps.stats[ STAT_PCLASS ],
                                      ent->client->pers.pclass ) )
        {
          trap_SendServerCommand( ent-g_entities, va("print \"You cannot evolve from your current class\n\"" ) );
          return;
        }

        if( ent->client->pers.pclass != PCL_NONE )
        {
          //prevent lerping
          ent->client->ps.eFlags ^= EF_TELEPORT_BIT;
    
          //evolve
          ent->client->ps.stats[ STAT_PCLASS ] = PCL_NONE;
          ent->client->sess.sessionTeam = TEAM_FREE;
          ClientUserinfoChanged( clientNum );
          ent->client->ps.stats[ STAT_STATE ] |= SS_INFESTING;
          ent->client->lastInfestTime = level.time;
          ent->client->infestBody = victim;

          VectorCopy( victim->s.pos.trBase, infestOrigin );
          infestOrigin[ 2 ] += 128;

          VectorCopy( victim->s.angles, infestAngles );

          infestAngles[ PITCH ] = 90;

          G_SetOrigin( ent, infestOrigin );
          VectorCopy( infestOrigin, ent->client->ps.origin );
          SetClientViewAngle( ent, infestAngles );
        }
        else
        {
          trap_SendServerCommand( ent-g_entities, va("print \"Unknown class\n\"" ) );
          return;
        }
      }
    }
    else
    {
      //spawing from a bioegg
      ent->client->pers.pclass = BG_FindClassNumForName( s );

      if( ent->client->pers.pclass != PCL_NONE )
      {
        for( i = 0; i < NUM_AC; i++ )
        {
          if( allowedClasses[ i ] == ent->client->pers.pclass )
          {
            ent->client->sess.sessionTeam = TEAM_FREE;
            ClientUserinfoChanged( clientNum );
            ClientSpawn( ent, NULL );
            return;
          }
        }

        ent->client->pers.pclass = PCL_NONE;
        trap_SendServerCommand( ent-g_entities, va("print \"You cannot spawn as this class\n\"" ) );
      }
      else
      {
        trap_SendServerCommand( ent-g_entities, va("print \"Unknown class\n\"" ) );
        return;
      }
    }
  }
  else if( ent->client->pers.pteam == PTE_HUMANS )
  {
    //humans cannot use this command whilst alive
    if( ent->client->pers.pclass != PCL_NONE )
    {
      trap_SendServerCommand( ent-g_entities, va("print \"You must be dead to use the class command\n\"" ) );
      return;
    }

    ent->client->pers.pclass = PCL_H_BASE;

    //set the item to spawn with
    if( !Q_stricmp( s, "rifle" ) )
      ent->client->pers.pitem = WP_MACHINEGUN;
    else if( !Q_stricmp( s, "ckit" ) )
      ent->client->pers.pitem = WP_HBUILD;
    else
    {
      trap_SendServerCommand( ent-g_entities, va("print \"Unknown starting item\n\"" ) );
      return;
    }

    ent->client->sess.sessionTeam = TEAM_FREE;
    ClientUserinfoChanged( clientNum );
    ClientSpawn( ent, NULL );
  }
  else if( ent->client->pers.pteam == PTE_NONE )
  {
    //can't use this command unless on a team
    ent->client->pers.pclass = PCL_NONE;
    ent->client->sess.sessionTeam = TEAM_FREE;
    ClientSpawn( ent, NULL );
    trap_SendServerCommand( ent-g_entities, va("print \"Join a team first\n\"" ) );
  }
}


/*
=================
Cmd_Destroy_f
=================
*/
void Cmd_Destroy_f( gentity_t *ent )
{
  vec3_t      forward, end;
  trace_t     tr;
  gentity_t   *traceEnt;

  if( !( ent->client->ps.stats[ STAT_STATE ] & SS_INFESTING ) &&
      !( ent->client->ps.stats[ STAT_STATE ] & SS_HOVELING ) )
  {
    AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );
    VectorMA( ent->client->ps.origin, 50, forward, end );

    trap_Trace( &tr, ent->client->ps.origin, NULL, NULL, end, ent->s.number, MASK_PLAYERSOLID );
    traceEnt = &g_entities[ tr.entityNum ];

    if( tr.fraction < 1.0 &&
        ( traceEnt->s.eType == ET_BUILDABLE ) &&
        ( traceEnt->biteam == ent->client->pers.pteam ) &&
        ( ( ent->client->ps.weapon >= WP_ABUILD ) &&
          ( ent->client->ps.weapon <= WP_HBUILD ) ) )
      G_Damage( traceEnt, ent, ent, forward, tr.endpos, 10000, 0, MOD_SUICIDE );
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

  if( ent->client->pers.pteam != PTE_HUMANS )
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

  if( ent->client->pers.pteam != PTE_HUMANS )
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

  if( ent->client->pers.pteam != PTE_HUMANS )
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
  vec3_t    distance;
  int       i;
  gentity_t *mcuEntity;
  qboolean  nearMCU = qfalse;
  int       weapon, upgrade;
  int       quan, clips, maxClips;

  trap_Argv( 1, s, sizeof( s ) );
  
  //aliens don't buy stuff
  if( ent->client->pers.pteam != PTE_HUMANS )
    return;

  for ( i = 1, mcuEntity = g_entities + i; i < level.num_entities; i++, mcuEntity++ )
  {
    if( mcuEntity->s.eType != ET_BUILDABLE )
      continue;
      
    if( mcuEntity->s.modelindex == BA_H_MCU )
    {
      VectorSubtract( ent->s.pos.trBase, mcuEntity->s.origin, distance );
      if( VectorLength( distance ) <= 100 )
        nearMCU = qtrue;
    }
  }

  //no MCU nearby
  if( !nearMCU )
  {
    trap_SendServerCommand( ent-g_entities, va("print \"You must be near an MCU\n\"" ) );
    return;
  }

  weapon = BG_FindWeaponNumForName( s );
  upgrade = BG_FindUpgradeNumForName( s );
    
  if( weapon != WP_NONE )
  {
    //already got this?
    if( BG_gotWeapon( weapon, ent->client->ps.stats ) )
    {
      G_AddPredictableEvent( ent, EV_MENU, MN_H_ITEMHELD );
      return;
    }
      
    //can afford this?
    if( BG_FindPriceForWeapon( weapon ) > ent->client->ps.stats[ STAT_CREDIT ] )
    {
      G_AddPredictableEvent( ent, EV_MENU, MN_H_NOFUNDS );
      return;
    }
    
    //have space to carry this?
    if( BG_FindSlotsForWeapon( weapon ) & ent->client->ps.stats[ STAT_SLOTS ] )
    {
      G_AddPredictableEvent( ent, EV_MENU, MN_H_NOSLOTS );
      return;
    }
    
    if( BG_FindTeamForWeapon( weapon ) != WUT_HUMANS )
    {
      //shouldn't need a fancy dialog
      trap_SendServerCommand( ent-g_entities, va("print \"You can't buy alien items\n\"" ) );
      return;
    }
    
    //add to inventory
    BG_packWeapon( weapon, ent->client->ps.stats );
    BG_FindAmmoForWeapon( weapon, &quan, &clips, &maxClips );
    BG_packAmmoArray( weapon, ent->client->ps.ammo, ent->client->ps.powerups,
                      quan, clips, maxClips );
    ent->client->ps.weapon = weapon;
    
    //subtract from funds
    ent->client->ps.stats[ STAT_CREDIT ] -= BG_FindPriceForWeapon( weapon );
  }
  else if( upgrade != UP_NONE )
  {
    //already got this?
    if( BG_gotItem( upgrade, ent->client->ps.stats ) )
    {
      G_AddPredictableEvent( ent, EV_MENU, MN_H_ITEMHELD );
      return;
    }
    
    //can afford this?
    if( BG_FindPriceForUpgrade( upgrade ) > ent->client->ps.stats[ STAT_CREDIT ] )
    {
      G_AddPredictableEvent( ent, EV_MENU, MN_H_NOFUNDS );
      return;
    }
    
    //have space to carry this?
    if( BG_FindSlotsForUpgrade( upgrade ) & ent->client->ps.stats[ STAT_SLOTS ] )
    {
      G_AddPredictableEvent( ent, EV_MENU, MN_H_NOSLOTS );
      return;
    }
    
    //add to inventory
    BG_packItem( upgrade, ent->client->ps.stats );
    
    //subtract from funds
    ent->client->ps.stats[ STAT_CREDIT ] -= BG_FindPriceForUpgrade( upgrade );
  }
  else
  {
    trap_SendServerCommand( ent-g_entities, va("print \"Unknown item\n\"" ) );
  }
  
}


/*
=================
Cmd_Sell_f
=================
*/
void Cmd_Sell_f( gentity_t *ent )
{
  char      s[ MAX_TOKEN_CHARS ];
  vec3_t    distance;
  int       i;
  gentity_t *mcuEntity;
  qboolean  nearMCU = qfalse;
  int       weapon, upgrade;
  int       quan, clips, maxClips;

  trap_Argv( 1, s, sizeof( s ) );

  //aliens don't sell stuff
  if( ent->client->pers.pteam != PTE_HUMANS )
    return;
    
  for ( i = 1, mcuEntity = g_entities + i; i < level.num_entities; i++, mcuEntity++ )
  {
    if( mcuEntity->s.eType != ET_BUILDABLE )
      continue;
      
    if( mcuEntity->s.modelindex == BA_H_MCU )
    {
      VectorSubtract( ent->s.pos.trBase, mcuEntity->s.origin, distance );
      if( VectorLength( distance ) <= 100 )
        nearMCU = qtrue;
    }
  }

  //no MCU nearby
  if( !nearMCU )
  {
    trap_SendServerCommand( ent-g_entities, va("print \"You must be near an MCU\n\"" ) );
    return;
  }

  weapon = BG_FindWeaponNumForName( s );
  upgrade = BG_FindUpgradeNumForName( s );
  
  if( weapon != WP_NONE )
  {
    //remove weapon if carried
    if( BG_gotWeapon( weapon, ent->client->ps.stats ) )
    {
      BG_removeWeapon( weapon, ent->client->ps.stats );

      //add to funds
      ent->client->ps.stats[ STAT_CREDIT ] += BG_FindPriceForWeapon( weapon );
    }
  }
  else if( upgrade != UP_NONE )
  {
    //remove upgrade if carried
    if( BG_gotItem( upgrade, ent->client->ps.stats ) )
    {
      BG_removeItem( upgrade, ent->client->ps.stats );

      //add to funds
      ent->client->ps.stats[ STAT_CREDIT ] += BG_FindPriceForUpgrade( upgrade );
    }
  }
  else
  {
    trap_SendServerCommand( ent-g_entities, va("print \"Unknown item\n\"" ) );
  }
}


/*
=================
Cmd_Deposit_f
=================
*/
void Cmd_Deposit_f( gentity_t *ent )
{
  char      s[ MAX_TOKEN_CHARS ];
  int       amount;
  vec3_t    distance;
  int       i;
  gentity_t *bankEntity;
  qboolean  nearBank = qfalse;

  trap_Argv( 1, s, sizeof( s ) );

  //aliens don't sell stuff
  if( ent->client->pers.pteam != PTE_HUMANS )
    return;
    
  for ( i = 1, bankEntity = g_entities + i; i < level.num_entities; i++, bankEntity++ )
  {
    if( bankEntity->s.eType != ET_BUILDABLE )
      continue;
      
    if( bankEntity->s.modelindex == BA_H_BANK )
    {
      VectorSubtract( ent->s.pos.trBase, bankEntity->s.origin, distance );
      if( VectorLength( distance ) <= 100 )
        nearBank = qtrue;
    }
  }

  if( !Q_stricmp( s, "all" ) )
    amount = ent->client->ps.stats[ STAT_CREDIT ];
  else
    amount = atoi( s );

  //no Bank nearby
  if( !nearBank )
  {
    trap_SendServerCommand( ent-g_entities, va("print \"You must be near an Bank\n\"" ) );
    return;
  }

  if( amount <= ent->client->ps.stats[ STAT_CREDIT ] )
  {
    ent->client->ps.stats[ STAT_CREDIT ] -= amount;
    level.bankCredits[ ent->client->ps.clientNum ] += amount;
  }
  else
    G_AddPredictableEvent( ent, EV_MENU, MN_H_NOFUNDS );
}


/*
=================
Cmd_Withdraw_f
=================
*/
void Cmd_Withdraw_f( gentity_t *ent )
{
  char      s[ MAX_TOKEN_CHARS ];
  int       amount;
  vec3_t    distance;
  int       i;
  gentity_t *bankEntity;
  qboolean  nearBank = qfalse;

  trap_Argv( 1, s, sizeof( s ) );
  
  //aliens don't sell stuff
  if( ent->client->pers.pteam != PTE_HUMANS )
    return;
    
  for ( i = 1, bankEntity = g_entities + i; i < level.num_entities; i++, bankEntity++ )
  {
    if( bankEntity->s.eType != ET_BUILDABLE )
      continue;
      
    if( bankEntity->s.modelindex == BA_H_BANK )
    {
      VectorSubtract( ent->s.pos.trBase, bankEntity->s.origin, distance );
      if( VectorLength( distance ) <= 100 )
        nearBank = qtrue;
    }
  }

  if( !Q_stricmp( s, "all" ) )
    amount = level.bankCredits[ ent->client->ps.clientNum ];
  else
    amount = atoi( s );

  //no Bank nearby
  if( !nearBank )
  {
    trap_SendServerCommand( ent-g_entities, va("print \"You must be near an Bank\n\"" ) );
    return;
  }

  if( amount <= level.bankCredits[ ent->client->ps.clientNum ] )
  {
    ent->client->ps.stats[ STAT_CREDIT ] += amount;
    level.bankCredits[ ent->client->ps.clientNum ] -= amount;
  }
  else
    G_AddPredictableEvent( ent, EV_MENU, MN_H_NOFUNDS );
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

  trap_Argv( 1, s, sizeof( s ) );

  buildable = BG_FindBuildNumForName( s );
  
  if( buildable != BA_NONE &&
      ( 1 << ent->client->ps.weapon ) & BG_FindBuildWeaponForBuildable( buildable ) &&
      !( ent->client->ps.stats[ STAT_STATE ] & SS_INFESTING ) &&
      !( ent->client->ps.stats[ STAT_STATE ] & SS_HOVELING ) )
  {
    dist = BG_FindBuildDistForClass( ent->client->ps.stats[ STAT_PCLASS ] );
    
    switch( G_itemFits( ent, buildable, dist, origin ) )
    {
      case IBE_NONE:
      case IBE_RPLWARN:
      case IBE_RPTWARN:
      case IBE_SPWNWARN:
      case IBE_NOROOM:
      case IBE_NORMAL:
        ent->client->ps.stats[ STAT_BUILDABLE ] = ( buildable | SB_VALID_TOGGLEBIT );
        break;

      case IBE_NOASSERT:
        G_AddPredictableEvent( ent, EV_MENU, MN_D_NOASSERT );
        break;

      case IBE_NOHIVEMIND:
        G_AddPredictableEvent( ent, EV_MENU, MN_D_NOHVMND );
        break;

      case IBE_HIVEMIND:
        G_AddPredictableEvent( ent, EV_MENU, MN_D_HIVEMIND );
        break;

      case IBE_REACTOR:
        G_AddPredictableEvent( ent, EV_MENU, MN_H_REACTOR );
        break;

      case IBE_REPEATER:
        G_AddPredictableEvent( ent, EV_MENU, MN_H_REPEATER );
        break;

      case IBE_NOPOWER:
        G_AddPredictableEvent( ent, EV_MENU, MN_H_NOPOWER );
        break;
    }
  }
  else
    trap_SendServerCommand( ent-g_entities, va("print \"Cannot build this item\n\"" ) );
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

  trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", s ) );
}


/*
=================
Cmd_Boost_f
=================
*/
void Cmd_Boost_f( gentity_t *ent )
{
  if( ( ent->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS ) &&
      ( ent->client->ps.stats[ STAT_STAMINA ] > 0 ) )
    ent->client->ps.stats[ STAT_STATE ] |= SS_SPEEDBOOST;
}

/*
=================
ClientCommand
=================
*/
void ClientCommand( int clientNum ) {
  gentity_t *ent;
  char  cmd[MAX_TOKEN_CHARS];

  ent = g_entities + clientNum;
  if ( !ent->client ) {
    return;   // not fully in game yet
  }

  trap_Argv( 0, cmd, sizeof( cmd ) );

  if (Q_stricmp (cmd, "say") == 0) {
    Cmd_Say_f (ent, SAY_ALL, qfalse);
    return;
  }
  if (Q_stricmp (cmd, "say_team") == 0) {
    Cmd_Say_f (ent, SAY_TEAM, qfalse);
    return;
  }
  if (Q_stricmp (cmd, "tell") == 0) {
    Cmd_Tell_f ( ent );
    return;
  }
  if (Q_stricmp (cmd, "vsay") == 0) {
    Cmd_Voice_f (ent, SAY_ALL, qfalse, qfalse);
    return;
  }
  if (Q_stricmp (cmd, "vsay_team") == 0) {
    Cmd_Voice_f (ent, SAY_TEAM, qfalse, qfalse);
    return;
  }
  if (Q_stricmp (cmd, "vtell") == 0) {
    Cmd_VoiceTell_f ( ent, qfalse );
    return;
  }
  if (Q_stricmp (cmd, "vosay") == 0) {
    Cmd_Voice_f (ent, SAY_ALL, qfalse, qtrue);
    return;
  }
  if (Q_stricmp (cmd, "vosay_team") == 0) {
    Cmd_Voice_f (ent, SAY_TEAM, qfalse, qtrue);
    return;
  }
  if (Q_stricmp (cmd, "votell") == 0) {
    Cmd_VoiceTell_f ( ent, qtrue );
    return;
  }
  if (Q_stricmp (cmd, "vtaunt") == 0) {
    Cmd_VoiceTaunt_f ( ent );
    return;
  }
  if (Q_stricmp (cmd, "score") == 0) {
    Cmd_Score_f (ent);
    return;
  }

  // ignore all other commands when at intermission
  if (level.intermissiontime) {
    Cmd_Say_f (ent, qfalse, qtrue);
    return;
  }

  if (Q_stricmp (cmd, "give") == 0)
    Cmd_Give_f (ent);
  else if (Q_stricmp (cmd, "god") == 0)
    Cmd_God_f (ent);
  else if (Q_stricmp (cmd, "notarget") == 0)
    Cmd_Notarget_f (ent);
  else if (Q_stricmp (cmd, "noclip") == 0)
    Cmd_Noclip_f (ent);
  else if (Q_stricmp (cmd, "kill") == 0)
    Cmd_Kill_f (ent);
  else if (Q_stricmp (cmd, "teamtask") == 0)
    Cmd_TeamTask_f (ent);
  else if (Q_stricmp (cmd, "levelshot") == 0)
    Cmd_LevelShot_f (ent);
  else if (Q_stricmp (cmd, "follow") == 0)
    Cmd_Follow_f (ent);
  else if (Q_stricmp (cmd, "follownext") == 0)
    Cmd_FollowCycle_f (ent, 1);
  else if (Q_stricmp (cmd, "followprev") == 0)
    Cmd_FollowCycle_f (ent, -1);
  else if (Q_stricmp (cmd, "team") == 0)
    Cmd_Team_f (ent);
  else if (Q_stricmp (cmd, "class") == 0)
    Cmd_Class_f( ent );
  else if (Q_stricmp (cmd, "build") == 0)
    Cmd_Build_f( ent );
  else if (Q_stricmp (cmd, "buy") == 0)
    Cmd_Buy_f( ent );
  else if (Q_stricmp (cmd, "sell") == 0)
    Cmd_Sell_f( ent );
  else if (Q_stricmp (cmd, "deposit") == 0)
    Cmd_Deposit_f( ent );
  else if (Q_stricmp (cmd, "withdraw") == 0)
    Cmd_Withdraw_f( ent );
  else if (Q_stricmp (cmd, "itemact") == 0)
    Cmd_ActivateItem_f( ent );
  else if (Q_stricmp (cmd, "itemdeact") == 0)
    Cmd_DeActivateItem_f( ent );
  else if (Q_stricmp (cmd, "itemtoggle") == 0)
    Cmd_ToggleItem_f( ent );
  else if (Q_stricmp (cmd, "destroy") == 0)
    Cmd_Destroy_f( ent );
  else if (Q_stricmp (cmd, "echo") == 0)
    Cmd_Echo_f( ent );
  else if (Q_stricmp (cmd, "boost") == 0)
    Cmd_Boost_f( ent );
  else if (Q_stricmp (cmd, "where") == 0)
    Cmd_Where_f (ent);
  else if (Q_stricmp (cmd, "callvote") == 0)
    Cmd_CallVote_f (ent);
  else if (Q_stricmp (cmd, "vote") == 0)
    Cmd_Vote_f (ent);
  else if (Q_stricmp (cmd, "callteamvote") == 0)
    Cmd_CallTeamVote_f (ent);
  else if (Q_stricmp (cmd, "teamvote") == 0)
    Cmd_TeamVote_f (ent);
  else if (Q_stricmp (cmd, "gc") == 0)
    Cmd_GameCommand_f( ent );
  else if (Q_stricmp (cmd, "setviewpos") == 0)
    Cmd_SetViewpos_f( ent );
  else if (Q_stricmp (cmd, "stats") == 0)
    Cmd_Stats_f( ent );
  else
    trap_SendServerCommand( clientNum, va("print \"unknown cmd %s\n\"", cmd ) );
}
