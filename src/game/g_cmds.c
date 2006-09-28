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

#include "g_local.h"

/*
==================
G_SanitiseName

Remove case and control characters from a player name
==================
*/
void G_SanitiseName( char *in, char *out )
{
  qboolean skip = qtrue;
  int spaces = 0;

  while( *in )
  {
    // strip leading white space
    if( *in == ' ' )
    {
      if( skip )
      {
        in++;
        continue;
      }
      spaces++;
    }
    else
    {
      spaces = 0;
      skip = qfalse;
    }
    
    if( *in == 27 || *in == '^' )
    {
      in += 2;    // skip color code
      continue;
    }

    if( *in < 32 )
    {
      in++;
      continue;
    }

    *out++ = tolower( *in++ );
  }
  out -= spaces; 
  *out = 0;
}

/*
==================
G_ClientNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
int G_ClientNumberFromString( gentity_t *to, char *s )
{
  gclient_t *cl;
  int       idnum;
  char      s2[ MAX_STRING_CHARS ];
  char      n2[ MAX_STRING_CHARS ];

  // numeric values are just slot numbers
  if( s[ 0 ] >= '0' && s[ 0 ] <= '9' )
  {
    idnum = atoi( s );

    if( idnum < 0 || idnum >= level.maxclients )
    {
      trap_SendServerCommand( to - g_entities, va( "print \"Bad client slot: %i\n\"", idnum ) );
      return -1;
    }

    cl = &level.clients[ idnum ];

    if( cl->pers.connected != CON_CONNECTED )
    {
      trap_SendServerCommand( to - g_entities, va( "print \"Client %i is not active\n\"", idnum ) );
      return -1;
    }

    return idnum;
  }

  // check for a name match
  G_SanitiseName( s, s2 );

  for( idnum = 0, cl = level.clients; idnum < level.maxclients; idnum++, cl++ )
  {
    if( cl->pers.connected != CON_CONNECTED )
      continue;

    G_SanitiseName( cl->pers.netname, n2 );

    if( !strcmp( n2, s2 ) )
      return idnum;
  }

  trap_SendServerCommand( to - g_entities, va( "print \"User %s is not on the server\n\"", s ) );
  return -1;
}


/*
==================
G_MatchOnePlayer

This is a companion function to G_ClientNumbersFromString()

returns qtrue if the int array plist only has one client id, false otherwise
In the case of false, err will be populated with an error message.
==================
*/
qboolean G_MatchOnePlayer( int *plist, char *err, int len )
{
  gclient_t *cl;
  int *p;
  char line[ MAX_NAME_LENGTH + 10 ] = {""};

  err[ 0 ] = '\0';
  if( plist[ 0 ] == -1 )
  {
    Q_strcat( err, len, "no connected player by that name or slot #" );
    return qfalse;
  }
  if( plist[ 1 ] != -1 )
  {
    Q_strcat( err, len, "more than one player name matches. "
            "be more specific or use the slot #:\n" );
    for( p = plist; *p != -1; p++ )
    {
      cl = &level.clients[ *p ];
      if( cl->pers.connected == CON_CONNECTED )
      {
        Com_sprintf( line, sizeof( line ), "%2i - %s^7\n",
          *p, cl->pers.netname );
        if( strlen( err ) + strlen( line ) > len )
          break;
        Q_strcat( err, len, line );
      }
    }
    return qfalse;
  }
  return qtrue;
}

/*
==================
G_ClientNumbersFromString

Sets plist to an array of integers that represent client numbers that have
names that are a partial match for s. List is terminated by a -1.

Returns number of matching clientids.
==================
*/
int G_ClientNumbersFromString( char *s, int *plist )
{
  gclient_t *p;
  int i, found = 0;
  char n2[ MAX_NAME_LENGTH ] = {""}; 
  char s2[ MAX_NAME_LENGTH ] = {""}; 
  qboolean is_slot = qtrue;

  *plist = -1;

  // if a number is provided, it might be a slot #
  for( i = 0; i < (int)strlen( s ); i++ )
  {
    if( s[i] < '0' || s[i] > '9' )
    {
      is_slot = qfalse;
      break;
    }
  }

  if( is_slot ) {
    i = atoi( s );
    if( i >= 0 && i < level.maxclients ) {
      p = &level.clients[ i ];
      if( p->pers.connected == CON_CONNECTED ||
        p->pers.connected == CON_CONNECTING ) 
      {
        *plist++ = i;
        *plist = -1;
        return 1;
      }
    }
    // we must assume that if only a number is provided, it is a clientNum
    return 0;
  }
  
  // now look for name matches
  G_SanitiseName( s, s2 );
  if( strlen( s2 ) < 1 )
    return 0;
  for( i = 0; i < level.maxclients; i++ )
  {
    p = &level.clients[ i ];
    if(p->pers.connected != CON_CONNECTED
      && p->pers.connected != CON_CONNECTING)
    {
      continue;
    }
    G_SanitiseName( p->pers.netname, n2 );
    if( strstr( n2, s2 ) )
    {
      *plist++ = i;
      found++;
    }
  }
  *plist = -1;
  return found;
}

/*
==================
ScoreboardMessage

==================
*/
void ScoreboardMessage( gentity_t *ent )
{
  char      entry[ 1024 ];
  char      string[ 1400 ];
  int       stringlength;
  int       i, j;
  gclient_t *cl;
  int       numSorted;
  weapon_t  weapon = WP_NONE;
  upgrade_t upgrade = UP_NONE;

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

    if( cl->ps.stats[ STAT_HEALTH ] > 0 )
    {
      weapon = cl->ps.weapon;

      if( BG_InventoryContainsUpgrade( UP_BATTLESUIT, cl->ps.stats ) )
        upgrade = UP_BATTLESUIT;
      else if( BG_InventoryContainsUpgrade( UP_JETPACK, cl->ps.stats ) )
        upgrade = UP_JETPACK;
      else if( BG_InventoryContainsUpgrade( UP_BATTPACK, cl->ps.stats ) )
        upgrade = UP_BATTPACK;
      else if( BG_InventoryContainsUpgrade( UP_HELMET, cl->ps.stats ) )
        upgrade = UP_HELMET;
      else if( BG_InventoryContainsUpgrade( UP_LIGHTARMOUR, cl->ps.stats ) )
        upgrade = UP_LIGHTARMOUR;
      else
        upgrade = UP_NONE;
    }
    else
    {
      weapon = WP_NONE;
      upgrade = UP_NONE;
    }

    Com_sprintf( entry, sizeof( entry ),
      " %d %d %d %d %d %d", level.sortedClients[ i ], cl->ps.persistant[ PERS_SCORE ],
      ping, ( level.time - cl->pers.enterTime ) / 60000, weapon, upgrade );

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
    trap_SendServerCommand( ent-g_entities, va( "print \"Cheats are not enabled on this server\n\"" ) );
    return qfalse;
  }

  if( ent->health <= 0 )
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"You must be alive to use this command\n\"" ) );
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
  qboolean  give_all;

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

  if( give_all || Q_stricmpn( name, "funds", 5 ) == 0 )
  {
    int credits = atoi( name + 6 );

    if( !credits )
      G_AddCreditToClient( ent->client, 1, qtrue );
    else
      G_AddCreditToClient( ent->client, credits, qtrue );

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

  if( g_cheats.integer )
  {
    ent->flags &= ~FL_GODMODE;
    ent->client->ps.stats[ STAT_HEALTH ] = ent->health = 0;
    player_die( ent, ent, ent, 100000, MOD_SUICIDE );
  }
  else
  {
    if( ent->suicideTime == 0 )
    {
      trap_SendServerCommand( ent-g_entities, "print \"You will suicide in 20 seconds\n\"" );
      ent->suicideTime = level.time + 20000;
    }
    else if( ent->suicideTime > level.time )
    {
      trap_SendServerCommand( ent-g_entities, "print \"Suicide cancelled\n\"" );
      ent->suicideTime = 0;
    }
  }
}

/*
=================
G_ChangeTeam
=================
*/
void G_ChangeTeam( gentity_t *ent, pTeam_t newTeam )
{
  pTeam_t oldTeam = ent->client->pers.teamSelection;
 
  if( oldTeam == newTeam )
    return;

  ent->client->pers.teamSelection = newTeam;

  if( oldTeam == PTE_ALIENS )
    G_RemoveFromSpawnQueue( &level.alienSpawnQueue, ent->client->ps.clientNum );
  else if( oldTeam == PTE_HUMANS )
    G_RemoveFromSpawnQueue( &level.humanSpawnQueue, ent->client->ps.clientNum );

  // under certain circumstances, clients can keep their kills and credits
  // when switching teams
  if( G_admin_permission( ent, ADMF_TEAMCHANGEFREE ) ||
    ( ( oldTeam == PTE_HUMANS || oldTeam == PTE_ALIENS )
    && ( level.time - ent->client->pers.teamChangeTime ) > 60000 ) )
  {
    if( oldTeam == PTE_NONE )
    {
      // ps.persistant[] from a spectator cannot be trusted
      ent->client->ps.persistant[ PERS_SCORE ] = ent->client->pers.savedScore;
      ent->client->ps.persistant[ PERS_CREDIT ] = ent->client->pers.savedCredit;
    }
    else if( oldTeam == PTE_ALIENS )
    {
      // always save in human credtis
      ent->client->ps.persistant[ PERS_CREDIT ] *=
        (float)FREEKILL_HUMAN / FREEKILL_ALIEN;
    }

    if( newTeam == PTE_NONE )
    {
      // save values before the client enters the spectator team and their
      // ps.persistant[] values become trashed
      ent->client->pers.savedScore = ent->client->ps.persistant[ PERS_SCORE ];
      ent->client->pers.savedCredit = ent->client->ps.persistant[ PERS_CREDIT ];
    }
    else if( newTeam == PTE_ALIENS )
    {
      // convert to alien currency
      ent->client->ps.persistant[ PERS_CREDIT ] *=
        (float)FREEKILL_ALIEN / FREEKILL_HUMAN;
    }
  }
  else
  {
    ent->client->ps.persistant[ PERS_CREDIT ] = 0;
    ent->client->ps.persistant[ PERS_SCORE ] = 0;
    ent->client->pers.savedScore = 0;
    ent->client->pers.savedCredit = 0;
  }

  ent->client->pers.classSelection = PCL_NONE;
  ClientSpawn( ent, NULL, NULL, NULL );

  ent->client->pers.joinedATeam = qtrue;
  ent->client->pers.teamChangeTime = level.time;

  //update ClientInfo
  ClientUserinfoChanged( ent->client->ps.clientNum );
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f( gentity_t *ent )
{
  pTeam_t team;
  char    s[ MAX_TOKEN_CHARS ];
  qboolean force = G_admin_permission(ent, ADMF_FORCETEAMCHANGE);

  trap_Argv( 1, s, sizeof( s ) );

  if( !strlen( s ) )
  {
    trap_SendServerCommand( ent-g_entities, va("print \"team: %i\n\"", ent->client->pers.teamSelection ) );
    return;
  }

  if( !Q_stricmp( s, "spectate" ) )
    team = PTE_NONE;
  else if( !Q_stricmp( s, "aliens" ) )
  {
    if( level.alienTeamLocked )
    {
      trap_SendServerCommand( ent-g_entities,
        va( "print \"Alien team has been ^1LOCKED\n\"", s ) );
      return; 
    }
    else if( level.humanTeamLocked )
    {
      // if only one team has been locked, let people join the other
      // regardless of balance
      force = qtrue;
    }

    if( !force && g_teamForceBalance.integer
      && ( ( level.numAlienClients > level.numHumanClients ) ||
        ( ent->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS &&
          level.numAlienClients >= level.numHumanClients ) ) )
    {
      G_TriggerMenu( ent - g_entities, MN_A_TEAMFULL );
      return;
    }

    team = PTE_ALIENS;
  }
  else if( !Q_stricmp( s, "humans" ) )
  {
    if( level.humanTeamLocked )
    {
      trap_SendServerCommand( ent-g_entities,
        va( "print \"Human team has been ^1LOCKED\n\"", s ) );
      return; 
    }
    else if( level.alienTeamLocked )
    {
      // if only one team has been locked, let people join the other
      // regardless of balance
      force = qtrue;
    }

    if( !force && g_teamForceBalance.integer &&
      ( ( level.numHumanClients > level.numAlienClients ) ||
        ( ent->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS &&
          level.numHumanClients >= level.numAlienClients ) ) )
    {
      G_TriggerMenu( ent - g_entities, MN_H_TEAMFULL );
      return;
    }

    team = PTE_HUMANS;
  }
  else if( !Q_stricmp( s, "auto" ) )
  {
    if( level.humanTeamLocked && level.alienTeamLocked )
      team = PTE_NONE;
    else if( level.numHumanClients > level.numAlienClients )
      team = PTE_ALIENS;
    else if( level.numHumanClients < level.numAlienClients )
      team = PTE_HUMANS;
    else
      team = PTE_ALIENS + ( rand( ) % 2 );

    if( team == PTE_ALIENS && level.alienTeamLocked )
      team = PTE_HUMANS;
    else if( team == PTE_HUMANS && level.humanTeamLocked )
      team = PTE_ALIENS;
  }
  else
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"Unknown team: %s\n\"", s ) );
    return;
  }

  G_ChangeTeam( ent, team );

  if( team == PTE_ALIENS )
    trap_SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE " joined the aliens\n\"", ent->client->pers.netname ) );
  else if( team == PTE_HUMANS )
    trap_SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE " joined the humans\n\"", ent->client->pers.netname ) );
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
  {
    if( other->client->ps.stats[ STAT_PTEAM ] != PTE_NONE )
      return; 

    if( !G_admin_permission( other, ADMF_SPEC_ALLCHAT ) )
      return;

    // specs with ADMF_SPEC_ALLCHAT flag can see team chat
  }

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
  char    prefix[ 5 ];
  char    name[ 64 ];
  // don't let text be too long for malicious reasons
  char    text[ MAX_SAY_TEXT ];
  char    location[ 64 ];

  if (g_chatTeamPrefix.integer)
    switch( ent->client->pers.teamSelection)
    {
      default:
      case PTE_NONE:
        Com_sprintf( prefix, sizeof( prefix ), "[S] ");
        break;

      case PTE_ALIENS:
        Com_sprintf( prefix, sizeof( prefix ), "[A] ");
        break;

      case PTE_HUMANS:
        Com_sprintf( prefix, sizeof( prefix ), "[H] ");
    }
  else
    Com_sprintf( prefix, sizeof( prefix ), "");

  switch( mode )
  {
    default:
    case SAY_ALL:
      G_LogPrintf( "say: %s: %s\n", ent->client->pers.netname, chatText );
      Com_sprintf( name, sizeof( name ), "%s%s%c%c"EC": ", prefix,
                   ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
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
  
  if( g_adminParseSay.integer )
  {
    G_admin_cmd_check ( ent, qtrue );
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

  if( ent->client->pers.muted )
  {
    return;
  }

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
  if( ent != target )
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
    trap_SendServerCommand( ent-g_entities, "print \"Voting not allowed here\n\"" );
    return;
  }

  if( level.voteTime )
  {
    trap_SendServerCommand( ent-g_entities, "print \"A vote is already in progress\n\"" );
    return;
  }

  if( g_voteLimit.integer > 0
    && ent->client->pers.voteCount >= g_voteLimit.integer 
    && !G_admin_permission( ent, ADMF_NO_VOTE_LIMIT ) )
  {
    trap_SendServerCommand( ent-g_entities, va(
      "print \"You have already called the maxium number of votes (%d)\n\"",
      g_voteLimit.integer ) );
    return;
  }

  if( ent->client->sess.sessionTeam == TEAM_SPECTATOR )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Not allowed to call a vote as spectator\n\"" );
    return;
  }

  // make sure it is a valid command to vote on
  trap_Argv( 1, arg1, sizeof( arg1 ) );
  trap_Argv( 2, arg2, sizeof( arg2 ) );

  if( strchr( arg1, ';' ) || strchr( arg2, ';' ) )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string\n\"" );
    return;
  }

  // if there is still a vote to be executed
  if( level.voteExecuteTime )
  {
    level.voteExecuteTime = 0;
    trap_SendConsoleCommand( EXEC_APPEND, va( "%s\n", level.voteString ) );
  }

  if( !Q_stricmp( arg1, "kick" ) )
  {
    int clientNum;
    int clientNums[ MAX_CLIENTS ] = { -1 };
    
    if( G_ClientNumbersFromString( arg2, clientNums ) == 1 )
    {
      // there was one partial name match name was clientNum
      clientNum = clientNums[ 0 ]; 
    }
    else
    {
      // look for an exact name match before bailing out
      clientNum = G_ClientNumberFromString( ent, arg2 );
      if( clientNum == -1 )
      {
        trap_SendServerCommand( ent-g_entities,
          "print \"callvote: invalid player\n\"" );
        return;
      }
    }
    Q_strncpyz( arg1, "clientkick", sizeof( arg1 ) );
    Q_strncpyz( arg2, va( "%d", clientNum ), sizeof( arg2 ) );
  }

  if( !Q_stricmp( arg1, "clientkick" ) )
  {
    char  kickee[ MAX_NETNAME ];
    int   clientNum = 0;

    //check arg2 is a number
    for( i = 0; arg2[ i ]; i++ )
    {
      if( arg2[ i ] < '0' || arg2[ i ] > '9' )
      {
        clientNum = -1;
        break;
      }
    }

    if( clientNum >= 0 && clientNum < level.maxclients )
    {
      clientNum = atoi( arg2 );
      if( G_admin_permission( &g_entities[ clientNum ], ADMF_IMMUNITY ) )
      {
        trap_SendServerCommand( ent-g_entities,
          "print \"callvote: admin is immune from vote kick\n\"" );
        return;
      }

      if( level.clients[ clientNum ].pers.connected != CON_DISCONNECTED )
      {
        Q_strncpyz( kickee, level.clients[ clientNum ].pers.netname,
          sizeof( kickee ) );
        Q_CleanStr( kickee );
        if( g_admin.string[ 0 ] ) 
        {
          // !kick will add a temp ban and a descriptive drop message
          Com_sprintf( level.voteString, sizeof( level.voteString ),
            "!kick %d vote kick", clientNum );
        }
        else
        {
          Com_sprintf( level.voteString, sizeof( level.voteString ),
            "clientkick %d", clientNum );
        }
        Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
          "Kick player \'%s\'", kickee );
      }
      else
        return;
    }
    else
      return;
  }
  else if( !Q_stricmp( arg1, "map_restart" ) )
  {
    Com_sprintf( level.voteString, sizeof( level.voteString ), "%s", arg1 );
    Com_sprintf( level.voteDisplayString,
        sizeof( level.voteDisplayString ), "Restart current map" );
  }
  else if( !Q_stricmp( arg1, "map" ) )
  {
    Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );
    Com_sprintf( level.voteDisplayString,
        sizeof( level.voteDisplayString ), "Change to map \'%s\'", arg2 );
  }
  else if( !Q_stricmp( arg1, "nextmap" ) )
  {
    Com_sprintf( level.voteString, sizeof( level.voteString ), "advanceMapRotation" );
    Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
        "Skip to next map in rotation" );
  }
  else
  {
    trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string\n\"" );
    trap_SendServerCommand( ent-g_entities, "print \"Valid vote commands are: map_restart, nextmap, "
                                            "map <mapname>, kick <player>, clientkick <clientnum>\n\"" );
    return;
  }

  trap_SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE
        " called a vote\n\"", ent->client->pers.netname ) );

  ent->client->pers.voteCount++;

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
    trap_SendServerCommand( ent-g_entities, "print \"No vote in progress\n\"" );
    return;
  }

  if( ent->client->ps.eFlags & EF_VOTED )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Vote already cast\n\"" );
    return;
  }

  if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_NONE )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Not allowed to vote as spectator\n\"" );
    return;
  }

  trap_SendServerCommand( ent-g_entities, "print \"Vote cast\n\"" );

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
    trap_SendServerCommand( ent-g_entities, "print \"Voting not allowed here\n\"" );
    return;
  }

  if( level.teamVoteTime[ cs_offset ] )
  {
    trap_SendServerCommand( ent-g_entities, "print \"A team vote is already in progress\n\"" );
    return;
  }

  if( g_voteLimit.integer > 0
    && ent->client->pers.voteCount >= g_voteLimit.integer 
    && !G_admin_permission( ent, ADMF_NO_VOTE_LIMIT ) )
  {
    trap_SendServerCommand( ent-g_entities, va(
      "print \"You have already called the maxium number of votes (%d)\n\"",
      g_voteLimit.integer ) );
    return;
  }

  if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_NONE )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Not allowed to call a vote as spectator\n\"" );
    return;
  }

  // make sure it is a valid command to vote on
  trap_Argv( 1, arg1, sizeof( arg1 ) );
  trap_Argv( 2, arg2, sizeof( arg2 ) );

  if( strchr( arg1, ';' ) || strchr( arg2, ';' ) )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Invalid team vote string\n\"" );
    return;
  }

  if( !Q_stricmp( arg1, "teamkick" ) )
  {
    int clientNum;
    int clientNums[ MAX_CLIENTS ] = { -1 };
    
    if( G_ClientNumbersFromString( arg2, clientNums ) == 1 )
    {
      // there was one partial name match or name was clientNum
      clientNum = clientNums[ 0 ]; 
    }
    else
    {
      // look for an exact name match before bailing out
      clientNum = G_ClientNumberFromString( ent, arg2 );
      if( clientNum == -1 )
      {
        trap_SendServerCommand( ent-g_entities,
          "print \"callvote: invalid player\n\"" );
        return;
      }
    }
    Q_strncpyz( arg1, "teamclientkick", sizeof( arg1 ) );
    Q_strncpyz( arg2, va( "%d", clientNum ), sizeof( arg2 ) );
  }
  
  if( !Q_stricmp( arg1, "teamclientkick" ) )
  {
    int   clientNum = 0;
    char  kickee[ MAX_NETNAME ];

    //check arg2 is a number
    for( i = 0; arg2[ i ]; i++ )
    {
      if( arg2[ i ] < '0' || arg2[ i ] > '9' )
      {
        clientNum = -1;
        break;
      }
    }

    if( clientNum >= 0 && clientNum < level.maxclients )
    {
      clientNum = atoi( arg2 );

      for( i = 0; i < level.maxclients; i++ )
      {
        if( level.clients[ i ].pers.connected == CON_DISCONNECTED )
          continue;

        if( level.clients[ i ].ps.stats[ STAT_PTEAM ] != team )
          continue;

        if( level.clients[ i ].ps.clientNum == clientNum )
          break;
      }

      if( i >= level.maxclients )
        clientNum = -1;
    }

    if( clientNum < 0 )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"client %s "
            S_COLOR_WHITE "is not a valid player on your team\n\"", arg2 ) );
      return;
    }

    if( G_admin_permission( &g_entities[ clientNum ], ADMF_IMMUNITY ) )
    {
      trap_SendServerCommand( ent-g_entities,
        "print \"callteamvote: admin is immune from vote kick\n\"" );
      return;
    }

    Q_strncpyz( kickee, level.clients[ clientNum ].pers.netname,
      sizeof( kickee ) );
    Q_CleanStr( kickee );

    if( g_admin.string[ 0 ] )
    {
      // !kick will add a temp ban and a descriptive drop message
      Com_sprintf( level.teamVoteString[ cs_offset ],
          sizeof( level.teamVoteString[ cs_offset ] ),
          "!kick %d team vote kick", clientNum );
    }
    else
    {
      Com_sprintf( level.teamVoteString[ cs_offset ],
          sizeof( level.teamVoteString[ cs_offset ] ),
          "clientkick %d", clientNum );
    }
    Com_sprintf( level.teamVoteDisplayString[ cs_offset ],
        sizeof( level.teamVoteDisplayString[ cs_offset ] ),
        "Kick player \'%s\'", kickee );
  }
  else
  {
    trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string\n\"" );
    trap_SendServerCommand( ent-g_entities, "print \"Valid team vote commands are: teamkick <player>, "
                                            "teamclientkick <client>\n\"" );
    return;
  }
  ent->client->pers.voteCount++;

  for( i = 0 ; i < level.maxclients ; i++ )
  {
    if( level.clients[ i ].pers.connected == CON_DISCONNECTED )
      continue;

    if( level.clients[ i ].ps.stats[ STAT_PTEAM ] == team )
      trap_SendServerCommand( i, va("print \"%s " S_COLOR_WHITE
            "called a team vote\n\"", ent->client->pers.netname ) );
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
  trap_SetConfigstring( CS_TEAMVOTE_STRING + cs_offset, level.teamVoteDisplayString[ cs_offset ] );
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
    trap_SendServerCommand( ent-g_entities, "print \"No team vote in progress\n\"" );
    return;
  }

  if( ent->client->ps.eFlags & EF_TEAMVOTED )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Team vote already cast\n\"" );
    return;
  }

  if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_NONE )
  {
    trap_SendServerCommand( ent-g_entities, "print \"Not allowed to vote as spectator\n\"" );
    return;
  }

  trap_SendServerCommand( ent-g_entities, "print \"Team vote cast\n\"" );

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
    trap_SendServerCommand( ent-g_entities, va( "print \"Cheats are not enabled on this server\n\"" ) );
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
#define AS_OVER_RT3         ((ALIENSENSE_RANGE*0.5f)/M_ROOT3)

/*
=================
Cmd_Class_f
=================
*/
void Cmd_Class_f( gentity_t *ent )
{
  char      s[ MAX_TOKEN_CHARS ];
  int       clientNum;
  int       i;
  trace_t   tr, tr2;
  vec3_t    infestOrigin;
  int       allowedClasses[ PCL_NUM_CLASSES ];
  int       numClasses = 0;
  pClass_t  currentClass = ent->client->ps.stats[ STAT_PCLASS ];
  pClass_t  newClass;

  int       numLevels;
  vec3_t    fromMins, fromMaxs, toMins, toMaxs;
  vec3_t    temp;

  int       entityList[ MAX_GENTITIES ];
  vec3_t    range = { AS_OVER_RT3, AS_OVER_RT3, AS_OVER_RT3 };
  vec3_t    mins, maxs;
  int       num;
  gentity_t *other;

  if( ent->client->ps.stats[ STAT_HEALTH ] <= 0 )
    return;

  clientNum = ent->client - level.clients;
  trap_Argv( 1, s, sizeof( s ) );

  if( BG_ClassIsAllowed( PCL_ALIEN_BUILDER0 ) )
    allowedClasses[ numClasses++ ] = PCL_ALIEN_BUILDER0;

  if( BG_ClassIsAllowed( PCL_ALIEN_BUILDER0_UPG ) &&
      BG_FindStagesForClass( PCL_ALIEN_BUILDER0_UPG, g_alienStage.integer ) )
    allowedClasses[ numClasses++ ] = PCL_ALIEN_BUILDER0_UPG;

  if( BG_ClassIsAllowed( PCL_ALIEN_LEVEL0 ) )
    allowedClasses[ numClasses++ ] = PCL_ALIEN_LEVEL0;

  if( ent->client->pers.teamSelection == PTE_ALIENS &&
      !( ent->client->ps.stats[ STAT_STATE ] & SS_INFESTING ) &&
      !( ent->client->ps.stats[ STAT_STATE ] & SS_HOVELING ) )
  {
    newClass = BG_FindClassNumForName( s );
    if( newClass == PCL_NONE )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"Unknown class\n\"" ) );
      return;
    }

    //if we are not currently spectating, we are attempting evolution
    if( currentClass != PCL_NONE )
    {
      if( ( ent->client->ps.stats[ STAT_STATE ] & SS_WALLCLIMBING ) ||
          ( ent->client->ps.stats[ STAT_STATE ] & SS_WALLCLIMBINGCEILING ) )
      {
        trap_SendServerCommand( ent-g_entities, va( "print \"You cannot evolve while wallwalking\n\"" ) );
        return;
      }

      //check there are no humans nearby
      VectorAdd( ent->client->ps.origin, range, maxs );
      VectorSubtract( ent->client->ps.origin, range, mins );

      num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
      for( i = 0; i < num; i++ )
      {
        other = &g_entities[ entityList[ i ] ];

        if( ( other->client && other->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS ) ||
            ( other->s.eType == ET_BUILDABLE && other->biteam == BIT_HUMANS ) )
        {
          G_TriggerMenu( clientNum, MN_A_TOOCLOSE );
          return;
        }
      }

      if( !level.overmindPresent )
      {
        G_TriggerMenu( clientNum, MN_A_NOOVMND_EVOLVE );
        return;
      }

      //guard against selling the HBUILD weapons exploit
      if( ( currentClass == PCL_ALIEN_BUILDER0 ||
            currentClass == PCL_ALIEN_BUILDER0_UPG ) &&
          ent->client->ps.stats[ STAT_MISC ] > 0 )
      {
        trap_SendServerCommand( ent-g_entities,
            va( "print \"You cannot evolve until build timer expires\n\"" ) );
        return;
      }

      numLevels = BG_ClassCanEvolveFromTo( currentClass,
                                           newClass,
                                           (short)ent->client->ps.persistant[ PERS_CREDIT ], 0 );

      BG_FindBBoxForClass( currentClass,
                           fromMins, fromMaxs, NULL, NULL, NULL );
      BG_FindBBoxForClass( newClass,
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
        if( numLevels >= 0 &&
            BG_FindStagesForClass( newClass, g_alienStage.integer ) &&
            BG_ClassIsAllowed( newClass ) )
        {
          ent->client->pers.evolveHealthFraction = (float)ent->client->ps.stats[ STAT_HEALTH ] /
            (float)BG_FindHealthForClass( currentClass );

          if( ent->client->pers.evolveHealthFraction < 0.0f )
            ent->client->pers.evolveHealthFraction = 0.0f;
          else if( ent->client->pers.evolveHealthFraction > 1.0f )
            ent->client->pers.evolveHealthFraction = 1.0f;

          //remove credit
          G_AddCreditToClient( ent->client, -(short)numLevels, qtrue );
          ent->client->pers.classSelection = newClass;
          ClientUserinfoChanged( clientNum );
          VectorCopy( infestOrigin, ent->s.pos.trBase );
          ClientSpawn( ent, ent, ent->s.pos.trBase, ent->s.apos.trBase );
          return;
        }
        else
        {
          trap_SendServerCommand( ent-g_entities,
               va( "print \"You cannot evolve from your current class\n\"" ) );
          return;
        }
      }
      else
      {
        G_TriggerMenu( clientNum, MN_A_NOEROOM );
        return;
      }
    }
    else
    {
      //spawning from an egg
      for( i = 0; i < numClasses; i++ )
      {
        if( allowedClasses[ i ] == newClass &&
            BG_FindStagesForClass( newClass, g_alienStage.integer ) &&
            BG_ClassIsAllowed( newClass ) )
        {
          ent->client->pers.classSelection =
            ent->client->ps.stats[ STAT_PCLASS ] = newClass;
          G_PushSpawnQueue( &level.alienSpawnQueue, clientNum );
          return;
        }
      }
      trap_SendServerCommand( ent-g_entities, va( "print \"You cannot spawn as this class\n\"" ) );
      return;
    }
  }
  else if( ent->client->pers.teamSelection == PTE_HUMANS )
  {
    //humans cannot use this command whilst alive
    if( ent->client->pers.classSelection != PCL_NONE )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"You must be dead to use the class command\n\"" ) );
      return;
    }

    ent->client->pers.classSelection =
      ent->client->ps.stats[ STAT_PCLASS ] = PCL_HUMAN;

    //set the item to spawn with
    if( !Q_stricmp( s, BG_FindNameForWeapon( WP_MACHINEGUN ) ) && BG_WeaponIsAllowed( WP_MACHINEGUN ) )
      ent->client->pers.humanItemSelection = WP_MACHINEGUN;
    else if( !Q_stricmp( s, BG_FindNameForWeapon( WP_HBUILD ) ) && BG_WeaponIsAllowed( WP_HBUILD ) )
      ent->client->pers.humanItemSelection = WP_HBUILD;
    else if( !Q_stricmp( s, BG_FindNameForWeapon( WP_HBUILD2 ) ) && BG_WeaponIsAllowed( WP_HBUILD2 ) &&
        BG_FindStagesForWeapon( WP_HBUILD2, g_humanStage.integer ) )
      ent->client->pers.humanItemSelection = WP_HBUILD2;
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
      // Don't allow destruction of hovel with granger inside
      if( traceEnt->s.modelindex == BA_A_HOVEL && traceEnt->active )
        return;

      // Don't allow destruction of buildables that cannot be rebuilt
      if( g_suddenDeathTime.integer && ( level.time - level.startTime >=
          g_suddenDeathTime.integer * 60000 ) &&
          BG_FindBuildPointsForBuildable( traceEnt->s.modelindex ) )
      {
        return;
      }

      if( ent->client->ps.stats[ STAT_MISC ] > 0 )
      {
        G_AddEvent( ent, EV_BUILD_DELAY, ent->client->ps.clientNum );
        return;
      }

      if( !deconstruct && CheatsOk( ent ) )
        G_Damage( traceEnt, ent, ent, forward, tr.endpos, 10000, 0, MOD_SUICIDE );
      else
        G_FreeEntity( traceEnt );

      ent->client->ps.stats[ STAT_MISC ] +=
        BG_FindBuildDelayForWeapon( ent->s.weapon ) >> 2;
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
  int   upgrade, weapon;

  trap_Argv( 1, s, sizeof( s ) );
  upgrade = BG_FindUpgradeNumForName( s );
  weapon = BG_FindWeaponNumForName( s );

  if( ent->client->pers.teamSelection != PTE_HUMANS )
    return;

  if( ent->client->pers.classSelection == PCL_NONE )
    return;

  if( upgrade != UP_NONE && BG_InventoryContainsUpgrade( upgrade, ent->client->ps.stats ) )
    BG_ActivateUpgrade( upgrade, ent->client->ps.stats );
  else if( weapon != WP_NONE && BG_InventoryContainsWeapon( weapon, ent->client->ps.stats ) )
    G_ForceWeaponChange( ent, weapon );
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

  if( ent->client->pers.classSelection == PCL_NONE )
    return;

  if( BG_InventoryContainsUpgrade( upgrade, ent->client->ps.stats ) )
    BG_DeactivateUpgrade( upgrade, ent->client->ps.stats );
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
  int   upgrade, weapon, i;

  trap_Argv( 1, s, sizeof( s ) );
  upgrade = BG_FindUpgradeNumForName( s );
  weapon = BG_FindWeaponNumForName( s );

  if( ent->client->pers.teamSelection != PTE_HUMANS )
    return;

  if( weapon != WP_NONE )
  {
    //special case to allow switching between
    //the blaster and the primary weapon

    if( ent->client->ps.weapon != WP_BLASTER )
      weapon = WP_BLASTER;
    else
    {
      //find a held weapon which isn't the blaster
      for( i = WP_NONE + 1; i < WP_NUM_WEAPONS; i++ )
      {
        if( i == WP_BLASTER )
          continue;

        if( BG_InventoryContainsWeapon( i, ent->client->ps.stats ) )
        {
          weapon = i;
          break;
        }
      }

      if( i == WP_NUM_WEAPONS )
        weapon = WP_BLASTER;
    }

    G_ForceWeaponChange( ent, weapon );
  }
  else if( BG_InventoryContainsUpgrade( upgrade, ent->client->ps.stats ) )
  {
    if( BG_UpgradeIsActive( upgrade, ent->client->ps.stats ) )
      BG_DeactivateUpgrade( upgrade, ent->client->ps.stats );
    else
      BG_ActivateUpgrade( upgrade, ent->client->ps.stats );
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
  int       maxAmmo, maxClips;
  qboolean  buyingEnergyAmmo = qfalse;

  for( i = UP_NONE; i < UP_NUM_UPGRADES; i++ )
  {
    if( BG_InventoryContainsUpgrade( i, ent->client->ps.stats ) )
      numItems++;
  }

  for( i = WP_NONE; i < WP_NUM_WEAPONS; i++ )
  {
    if( BG_InventoryContainsWeapon( i, ent->client->ps.stats ) )
      numItems++;
  }

  trap_Argv( 1, s, sizeof( s ) );

  //aliens don't buy stuff
  if( ent->client->pers.teamSelection != PTE_HUMANS )
    return;

  weapon = BG_FindWeaponNumForName( s );
  upgrade = BG_FindUpgradeNumForName( s );

  //special case to keep norf happy
  if( weapon == WP_NONE && upgrade == UP_AMMO )
    buyingEnergyAmmo = BG_FindUsesEnergyForWeapon( ent->client->ps.weapon );

  if( buyingEnergyAmmo )
  {
    //no armoury nearby
    if( ( !G_BuildableRange( ent->client->ps.origin, 100, BA_H_REACTOR ) &&
        !G_BuildableRange( ent->client->ps.origin, 100, BA_H_REPEATER ) ) )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"You must be near a reactor or repeater\n\"" ) );
      return;
    }
  }
  else
  {
    //no armoury nearby
    if( !G_BuildableRange( ent->client->ps.origin, 100, BA_H_ARMOURY ) )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"You must be near a powered armoury\n\"" ) );
      return;
    }
  }

  if( weapon != WP_NONE )
  {
    //already got this?
    if( BG_InventoryContainsWeapon( weapon, ent->client->ps.stats ) )
    {
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_ITEMHELD );
      return;
    }

    //can afford this?
    if( BG_FindPriceForWeapon( weapon ) > (short)ent->client->ps.persistant[ PERS_CREDIT ] )
    {
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
    if( !BG_FindStagesForWeapon( weapon, g_humanStage.integer ) || !BG_WeaponIsAllowed( weapon ) )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"You can't buy this item\n\"" ) );
      return;
    }

    //add to inventory
    BG_AddWeaponToInventory( weapon, ent->client->ps.stats );
    BG_FindAmmoForWeapon( weapon, &maxAmmo, &maxClips );

    if( BG_FindUsesEnergyForWeapon( weapon ) &&
        BG_InventoryContainsUpgrade( UP_BATTPACK, ent->client->ps.stats ) )
      maxAmmo = (int)( (float)maxAmmo * BATTPACK_MODIFIER );

    BG_PackAmmoArray( weapon, ent->client->ps.ammo, ent->client->ps.powerups,
                      maxAmmo, maxClips );

    G_ForceWeaponChange( ent, weapon );

    //set build delay/pounce etc to 0
    ent->client->ps.stats[ STAT_MISC ] = 0;

    //subtract from funds
    G_AddCreditToClient( ent->client, -(short)BG_FindPriceForWeapon( weapon ), qfalse );
  }
  else if( upgrade != UP_NONE )
  {
    //already got this?
    if( BG_InventoryContainsUpgrade( upgrade, ent->client->ps.stats ) )
    {
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_ITEMHELD );
      return;
    }

    //can afford this?
    if( BG_FindPriceForUpgrade( upgrade ) > (short)ent->client->ps.persistant[ PERS_CREDIT ] )
    {
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_NOFUNDS );
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
    if( !BG_FindPurchasableForUpgrade( upgrade ) )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"You can't buy this item\n\"" ) );
      return;
    }

    //are we /allowed/ to buy this?
    if( !BG_FindStagesForUpgrade( upgrade, g_humanStage.integer ) || !BG_UpgradeIsAllowed( upgrade ) )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"You can't buy this item\n\"" ) );
      return;
    }

    if( upgrade == UP_AMMO )
      G_GiveClientMaxAmmo( ent, buyingEnergyAmmo );
    else
    {
      //add to inventory
      BG_AddUpgradeToInventory( upgrade, ent->client->ps.stats );
    }

    if( upgrade == UP_BATTPACK )
      G_GiveClientMaxAmmo( ent, qtrue );

    //subtract from funds
    G_AddCreditToClient( ent->client, -(short)BG_FindPriceForUpgrade( upgrade ), qfalse );
  }
  else
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"Unknown item\n\"" ) );
  }

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

  trap_Argv( 1, s, sizeof( s ) );

  //aliens don't sell stuff
  if( ent->client->pers.teamSelection != PTE_HUMANS )
    return;

  //no armoury nearby
  if( !G_BuildableRange( ent->client->ps.origin, 100, BA_H_ARMOURY ) )
  {
    trap_SendServerCommand( ent-g_entities, va( "print \"You must be near a powered armoury\n\"" ) );
    return;
  }

  weapon = BG_FindWeaponNumForName( s );
  upgrade = BG_FindUpgradeNumForName( s );

  if( weapon != WP_NONE )
  {
    //are we /allowed/ to sell this?
    if( !BG_FindPurchasableForWeapon( weapon ) )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"You can't sell this weapon\n\"" ) );
      return;
    }

    //remove weapon if carried
    if( BG_InventoryContainsWeapon( weapon, ent->client->ps.stats ) )
    {
      //guard against selling the HBUILD weapons exploit
      if( ( weapon == WP_HBUILD || weapon == WP_HBUILD2 ) &&
          ent->client->ps.stats[ STAT_MISC ] > 0 )
      {
        trap_SendServerCommand( ent-g_entities, va( "print \"Cannot sell until build timer expires\n\"" ) );
        return;
      }

      BG_RemoveWeaponFromInventory( weapon, ent->client->ps.stats );

      //add to funds
      G_AddCreditToClient( ent->client, (short)BG_FindPriceForWeapon( weapon ), qfalse );
    }

    //if we have this weapon selected, force a new selection
    if( weapon == ent->client->ps.weapon )
      G_ForceWeaponChange( ent, WP_NONE );
  }
  else if( upgrade != UP_NONE )
  {
    //are we /allowed/ to sell this?
    if( !BG_FindPurchasableForUpgrade( upgrade ) )
    {
      trap_SendServerCommand( ent-g_entities, va( "print \"You can't sell this item\n\"" ) );
      return;
    }
    //remove upgrade if carried
    if( BG_InventoryContainsUpgrade( upgrade, ent->client->ps.stats ) )
    {
      BG_RemoveUpgradeFromInventory( upgrade, ent->client->ps.stats );

      if( upgrade == UP_BATTPACK )
        G_GiveClientMaxAmmo( ent, qtrue );

      //add to funds
      G_AddCreditToClient( ent->client, (short)BG_FindPriceForUpgrade( upgrade ), qfalse );
    }
  }
  else if( !Q_stricmp( s, "weapons" ) )
  {
    for( i = WP_NONE + 1; i < WP_NUM_WEAPONS; i++ )
    {
      //guard against selling the HBUILD weapons exploit
      if( ( i == WP_HBUILD || i == WP_HBUILD2 ) &&
          ent->client->ps.stats[ STAT_MISC ] > 0 )
      {
        trap_SendServerCommand( ent-g_entities, va( "print \"Cannot sell until build timer expires\n\"" ) );
        continue;
      }

      if( BG_InventoryContainsWeapon( i, ent->client->ps.stats ) &&
          BG_FindPurchasableForWeapon( i ) )
      {
        BG_RemoveWeaponFromInventory( i, ent->client->ps.stats );

        //add to funds
        G_AddCreditToClient( ent->client, (short)BG_FindPriceForWeapon( i ), qfalse );
      }

      //if we have this weapon selected, force a new selection
      if( i == ent->client->ps.weapon )
        G_ForceWeaponChange( ent, WP_NONE );
    }
  }
  else if( !Q_stricmp( s, "upgrades" ) )
  {
    for( i = UP_NONE + 1; i < UP_NUM_UPGRADES; i++ )
    {
      //remove upgrade if carried
      if( BG_InventoryContainsUpgrade( i, ent->client->ps.stats ) &&
          BG_FindPurchasableForUpgrade( i ) )
      {
        BG_RemoveUpgradeFromInventory( i, ent->client->ps.stats );

        if( i == UP_BATTPACK )
        {
          int j;

          //remove energy
          for( j = WP_NONE; j < WP_NUM_WEAPONS; j++ )
          {
            if( BG_InventoryContainsWeapon( j, ent->client->ps.stats ) &&
                BG_FindUsesEnergyForWeapon( j ) &&
                !BG_FindInfinteAmmoForWeapon( j ) )
            {
              BG_PackAmmoArray( j, ent->client->ps.ammo, ent->client->ps.powerups, 0, 0 );
            }
          }
        }

        //add to funds
        G_AddCreditToClient( ent->client, (short)BG_FindPriceForUpgrade( i ), qfalse );
      }
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
  buildable_t   buildable;
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
      BG_BuildableIsAllowed( buildable ) &&
      ( ( team == PTE_ALIENS && BG_FindStagesForBuildable( buildable, g_alienStage.integer ) ) ||
        ( team == PTE_HUMANS && BG_FindStagesForBuildable( buildable, g_humanStage.integer ) ) ) )
  {
    dist = BG_FindBuildDistForClass( ent->client->ps.stats[ STAT_PCLASS ] );

    //these are the errors displayed when the builder first selects something to use
    switch( G_itemFits( ent, buildable, dist, origin ) )
    {
      case IBE_NONE:
      case IBE_TNODEWARN:
      case IBE_RPTWARN:
      case IBE_RPTWARN2:
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

      default:
        break;
    }
  }
  else
    trap_SendServerCommand( ent-g_entities, va( "print \"Cannot build this item\n\"" ) );
}


/*
=================
Cmd_Boost_f
=================
*/
void Cmd_Boost_f( gentity_t *ent )
{
  if( BG_InventoryContainsUpgrade( UP_JETPACK, ent->client->ps.stats ) &&
      BG_UpgradeIsActive( UP_JETPACK, ent->client->ps.stats ) )
    return;

  if( ent->client->pers.cmd.buttons & BUTTON_WALKING )
    return;

  if( ( ent->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS ) &&
      ( ent->client->ps.stats[ STAT_STAMINA ] > 0 ) )
    ent->client->ps.stats[ STAT_STATE ] |= SS_SPEEDBOOST;
}

/*
=================
Cmd_Reload_f
=================
*/
void Cmd_Reload_f( gentity_t *ent )
{
  if( ent->client->ps.weaponstate != WEAPON_RELOADING )
    ent->client->ps.pm_flags |= PMF_WEAPON_RELOAD;
}

/*
=================
G_StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
void G_StopFollowing( gentity_t *ent )
{
  ent->client->ps.persistant[ PERS_TEAM ] = TEAM_SPECTATOR;
  ent->client->sess.sessionTeam = TEAM_SPECTATOR;
  ent->client->sess.spectatorState = SPECTATOR_FREE;
  ent->client->sess.spectatorClient = -1;
  ent->client->ps.pm_flags &= ~PMF_FOLLOW;
  ent->client->ps.stats[ STAT_PTEAM ] = PTE_NONE;

  ent->client->ps.stats[ STAT_STATE ] &= ~SS_WALLCLIMBING;
  ent->client->ps.stats[ STAT_STATE ] &= ~SS_WALLCLIMBINGCEILING;
  ent->client->ps.eFlags &= ~EF_WALLCLIMB;
  ent->client->ps.viewangles[ PITCH ] = 0.0f;

  ent->client->ps.clientNum = ent - g_entities;

  CalculateRanks( );
}

/*
=================
G_FollowNewClient

This was a really nice, elegant function. Then I fucked it up.
=================
*/
qboolean G_FollowNewClient( gentity_t *ent, int dir )
{
  int       clientnum = ent->client->sess.spectatorClient;
  int       original = clientnum;
  qboolean  selectAny = qfalse;

  if( dir > 1 )
    dir = 1;
  else if( dir < -1 )
    dir = -1;
  else if( dir == 0 )
    return qtrue;

  if( ent->client->sess.sessionTeam != TEAM_SPECTATOR )
    return qfalse;

  // select any if no target exists
  if( clientnum < 0 || clientnum >= level.maxclients )
  {
    clientnum = original = 0;
    selectAny = qtrue;
  }

  do
  {
    clientnum += dir;

    if( clientnum >= level.maxclients )
      clientnum = 0;

    if( clientnum < 0 )
      clientnum = level.maxclients - 1;

    // avoid selecting existing follow target
    if( clientnum == original && !selectAny )
      continue; //effectively break;

    // can't follow self
    if( &level.clients[ clientnum ] == ent->client )
      continue;

    // can only follow connected clients
    if( level.clients[ clientnum ].pers.connected != CON_CONNECTED )
      continue;

    // can't follow another spectator
    if( level.clients[ clientnum ].sess.sessionTeam == TEAM_SPECTATOR )
      continue;

    // this is good, we can use it
    ent->client->sess.spectatorClient = clientnum;
    ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
    return qtrue;

  } while( clientnum != original );

  return qfalse;
}

/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f( gentity_t *ent, qboolean toggle )
{
  int   i;
  char  arg[ MAX_TOKEN_CHARS ];

  if( trap_Argc( ) != 2 || toggle )
  {
    if( ent->client->sess.spectatorState == SPECTATOR_FOLLOW )
      G_StopFollowing( ent );
    else if( ent->client->sess.spectatorState == SPECTATOR_FREE )
      G_FollowNewClient( ent, 1 );
  }
  else if( ent->client->sess.spectatorState == SPECTATOR_FREE )
  {
    trap_Argv( 1, arg, sizeof( arg ) );
    i = G_ClientNumberFromString( ent, arg );

    if( i == -1 )
      return;

    // can't follow self
    if( &level.clients[ i ] == ent->client )
      return;

    // can't follow another spectator
    if( level.clients[ i ].sess.sessionTeam == TEAM_SPECTATOR )
      return;

    // first set them to spectator
    if( ent->client->sess.sessionTeam != TEAM_SPECTATOR )
      return;

    ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
    ent->client->sess.spectatorClient = i;
  }
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_FollowCycle_f( gentity_t *ent, int dir )
{
  // won't work unless spectating
  if( ent->client->sess.spectatorState == SPECTATOR_NOT )
    return;

  if( dir != 1 && dir != -1 )
    G_Error( "Cmd_FollowCycle_f: bad dir %i", dir );

  G_FollowNewClient( ent, dir );
}

/*
=================
Cmd_PTRCVerify_f

Check a PTR code is valid
=================
*/
void Cmd_PTRCVerify_f( gentity_t *ent )
{
  connectionRecord_t  *connection;
  char                s[ MAX_TOKEN_CHARS ] = { 0 };
  int                 code;

  trap_Argv( 1, s, sizeof( s ) );

  if( !strlen( s ) )
    return;

  code = atoi( s );

  if( G_VerifyPTRC( code ) )
  {
    connection = G_FindConnectionForCode( code );

    // valid code
    if( connection->clientTeam != PTE_NONE )
      trap_SendServerCommand( ent->client->ps.clientNum, "ptrcconfirm" );

    // restore mapping
    ent->client->pers.connection = connection;
  }
  else
  {
    // invalid code -- generate a new one
    connection = G_GenerateNewConnection( ent->client );

    if( connection )
    {
      trap_SendServerCommand( ent->client->ps.clientNum,
        va( "ptrcissue %d", connection->ptrCode ) );
    }
  }
}

/*
=================
Cmd_PTRCRestore_f

Restore against a PTR code
=================
*/
void Cmd_PTRCRestore_f( gentity_t *ent )
{
  char                s[ MAX_TOKEN_CHARS ] = { 0 };
  int                 code;
  connectionRecord_t  *connection;

  trap_Argv( 1, s, sizeof( s ) );

  if( !strlen( s ) )
    return;

  code = atoi( s );

  if( G_VerifyPTRC( code ) )
  {
    if( ent->client->pers.joinedATeam )
    {
      trap_SendServerCommand( ent - g_entities,
        "print \"You cannot use a PTR code after joining a team\n\"" );
    }
    else
    {
      // valid code
      connection = G_FindConnectionForCode( code );

      if( connection )
      {
        // set the correct team
        G_ChangeTeam( ent, connection->clientTeam );

        // set the correct credit
        ent->client->ps.persistant[ PERS_CREDIT ] = 0;
        G_AddCreditToClient( ent->client, connection->clientCredit, qtrue );
      }
    }
  }
  else
  {
    trap_SendServerCommand( ent - g_entities,
      va( "print \"\"%d\" is not a valid PTR code\n\"", code ) );
  }
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

/*  ent->client->ps.stats[ STAT_STATE ] |= SS_POISONED;
  ent->client->lastPoisonTime = level.time;
  ent->client->lastPoisonClient = ent;*/
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

  if( G_admin_cmd_check( ent, qfalse ) )
    return;

  // ignore all other commands when at intermission
  if( level.intermissiontime )
    return;

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
  else if( Q_stricmp( cmd, "reload" ) == 0 )
    Cmd_Reload_f( ent );
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
  else if( Q_stricmp( cmd, "follow" ) == 0 )
    Cmd_Follow_f( ent, qfalse );
  else if( Q_stricmp (cmd, "follownext") == 0)
    Cmd_FollowCycle_f( ent, 1 );
  else if( Q_stricmp( cmd, "followprev" ) == 0 )
    Cmd_FollowCycle_f( ent, -1 );
  else if( Q_stricmp( cmd, "teamvote" ) == 0 )
    Cmd_TeamVote_f( ent );
  else if( Q_stricmp( cmd, "setviewpos" ) == 0 )
    Cmd_SetViewpos_f( ent );
  else if( Q_stricmp( cmd, "ptrcverify" ) == 0 )
    Cmd_PTRCVerify_f( ent );
  else if( Q_stricmp( cmd, "ptrcrestore" ) == 0 )
    Cmd_PTRCRestore_f( ent );
  else if( Q_stricmp( cmd, "test" ) == 0 )
    Cmd_Test_f( ent );
  else
    trap_SendServerCommand( clientNum, va( "print \"unknown cmd %s\n\"", cmd ) );
}

int G_SayArgc()
{
  int c = 1;
  char *s;

  s = ConcatArgs( 0 );
  if( !*s )
     return 0;
  while( *s )
  {
    if( *s == ' ' )
    {
      s++; 
      if( *s != ' ' )
      {
        c++;
        continue;
      }
      while( *s && *s == ' ' )
        s++;
      c++;
    }
    s++;
  }
  return c;
}

qboolean G_SayArgv( int n, char *buffer, int bufferLength )
{
  int bc = 1;
  int c = 0;
  char *s;

  if( bufferLength < 1 )
    return qfalse;
  if(n < 0)
    return qfalse;
  *buffer = '\0';
  s = ConcatArgs( 0 );
  while( *s )
  {
    if( c == n )
    {
      while( *s && ( bc < bufferLength ) )
      {
        if( *s == ' ' )
        {
          *buffer = '\0';
          return qtrue;
        }
        *buffer = *s;
        buffer++;
        s++;
        bc++;
      }
      *buffer = '\0';
      return qtrue;
    }
    if( *s == ' ' )
    {
      s++;
      if( *s != ' ' )
      {
        c++;
        continue;
      }
      while( *s && *s == ' ' )
        s++;
      c++;
    }
    s++;
  }
  return qfalse;
}

char *G_SayConcatArgs(int start)
{
  char *s;
  int c = 0;

  s = ConcatArgs( 0 );
  while( *s ) {
    if( c == start )
      return s;
    if( *s == ' ' )
    {
      s++;
      if( *s != ' ' )
      {
        c++;
        continue;
      }
      while( *s && *s == ' ' )
        s++;
      c++;
    }
    s++;
  }
  return s;
}

void G_DecolorString( char *in, char *out )
{   
  while( *in ) {
    if( *in == 27 || *in == '^' ) {
      in++;
      if( *in )
        in++;
      continue;
    }
    *out++ = *in++;
  }
  *out = '\0';
}
