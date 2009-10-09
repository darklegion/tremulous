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

// NULL for everyone
void QDECL PrintMsg( gentity_t *ent, const char *fmt, ... )
{
  char    msg[ 1024 ];
  va_list argptr;
  char    *p;

  va_start( argptr,fmt );

  if( Q_vsnprintf( msg, sizeof( msg ), fmt, argptr ) > sizeof( msg ) )
    G_Error ( "PrintMsg overrun" );

  va_end( argptr );

  // double quotes are bad
  while( ( p = strchr( msg, '"' ) ) != NULL )
    *p = '\'';

  trap_SendServerCommand( ( ( ent == NULL ) ? -1 : ent-g_entities ), va( "print \"%s\"", msg ) );
}

/*
================
G_TeamFromString

Return the team referenced by a string
================
*/
team_t G_TeamFromString( char *str )
{
  switch( tolower( *str ) )
  {
    case '0': case 's': return TEAM_NONE;
    case '1': case 'a': return TEAM_ALIENS;
    case '2': case 'h': return TEAM_HUMANS;
    default: return NUM_TEAMS;
  }
}

/*
==============
OnSameTeam
==============
*/
qboolean OnSameTeam( gentity_t *ent1, gentity_t *ent2 )
{
  if( !ent1->client || !ent2->client )
    return qfalse;

  if( ent1->client->pers.teamSelection == ent2->client->pers.teamSelection )
    return qtrue;

  return qfalse;
}

/*
==================
G_ClientListForTeam
==================
*/
static clientList_t G_ClientListForTeam( team_t team )
{
  int           i;
  clientList_t  clientList;

  Com_Memset( &clientList, 0, sizeof( clientList_t ) );

  for( i = 0; i < g_maxclients.integer; i++ )
  {
    gentity_t *ent = g_entities + i;
    if( ent->client->pers.connected != CON_CONNECTED )
      continue;

    if( ent->inuse && ( ent->client->ps.stats[ STAT_TEAM ] == team ) )
      Com_ClientListAdd( &clientList, ent->client->ps.clientNum );
  }

  return clientList;
}

/*
==================
G_UpdateTeamConfigStrings
==================
*/
static void G_UpdateTeamConfigStrings( void )
{
  clientList_t alienTeam = G_ClientListForTeam( TEAM_ALIENS );
  clientList_t humanTeam = G_ClientListForTeam( TEAM_HUMANS );

  if( level.intermissiontime )
  {
    // No restrictions once the game has ended
    Com_Memset( &alienTeam, 0, sizeof( clientList_t ) );
    Com_Memset( &humanTeam, 0, sizeof( clientList_t ) );
  }

  trap_SetConfigstringRestrictions( CS_VOTE_TIME + TEAM_ALIENS,   &humanTeam );
  trap_SetConfigstringRestrictions( CS_VOTE_STRING + TEAM_ALIENS, &humanTeam );
  trap_SetConfigstringRestrictions( CS_VOTE_YES + TEAM_ALIENS,    &humanTeam );
  trap_SetConfigstringRestrictions( CS_VOTE_NO + TEAM_ALIENS,     &humanTeam );

  trap_SetConfigstringRestrictions( CS_VOTE_TIME + TEAM_HUMANS,   &alienTeam );
  trap_SetConfigstringRestrictions( CS_VOTE_STRING + TEAM_HUMANS, &alienTeam );
  trap_SetConfigstringRestrictions( CS_VOTE_YES + TEAM_HUMANS,    &alienTeam );
  trap_SetConfigstringRestrictions( CS_VOTE_NO + TEAM_HUMANS,     &alienTeam );

  trap_SetConfigstringRestrictions( CS_ALIEN_STAGES, &humanTeam );
  trap_SetConfigstringRestrictions( CS_HUMAN_STAGES, &alienTeam );
}

/*
==================
G_LeaveTeam
==================
*/
void G_LeaveTeam( gentity_t *self )
{
  team_t    team = self->client->pers.teamSelection;
  gentity_t *ent;
  int       i;

  if( team == TEAM_ALIENS )
    G_RemoveFromSpawnQueue( &level.alienSpawnQueue, self->client->ps.clientNum );
  else if( team == TEAM_HUMANS )
    G_RemoveFromSpawnQueue( &level.humanSpawnQueue, self->client->ps.clientNum );
  else
  {
    if( self->client->sess.spectatorState == SPECTATOR_FOLLOW )
      G_StopFollowing( self );
    return;
  }

  // stop any following clients
  G_StopFromFollowing( self );

  G_Vote( self, team, qfalse );
  self->suicideTime = 0;

  for( i = 0; i < level.num_entities; i++ )
  {
    ent = &g_entities[ i ];
    if( !ent->inuse )
      continue;

    if( ent->client && ent->client->pers.connected == CON_CONNECTED )
    {
      // cure poison
      if( ent->client->ps.stats[ STAT_STATE ] & SS_POISONED &&
          ent->client->lastPoisonClient == self )
        ent->client->ps.stats[ STAT_STATE ] &= ~SS_POISONED;
    }
    else if( ent->s.eType == ET_MISSILE && ent->r.ownerNum == self->s.number )
      G_FreeEntity( ent );
  }
}

/*
=================
G_ChangeTeam
=================
*/
void G_ChangeTeam( gentity_t *ent, team_t newTeam )
{
  team_t  oldTeam = ent->client->pers.teamSelection;

  if( oldTeam == newTeam )
    return;

  G_LeaveTeam( ent );
  ent->client->pers.teamSelection = newTeam;
  ent->client->pers.classSelection = PCL_NONE;
  ClientSpawn( ent, NULL, NULL, NULL );
  ent->client->pers.joinedATeam = qtrue;
  ent->client->pers.teamChangeTime = level.time;

  if( oldTeam == TEAM_HUMANS && newTeam == TEAM_ALIENS )
  {
    // Convert from human to alien credits
    ent->client->pers.credit =
      (int)( ent->client->pers.credit *
             ALIEN_MAX_CREDITS / HUMAN_MAX_CREDITS + 0.5f );
  }
  else if( oldTeam == TEAM_ALIENS && newTeam == TEAM_HUMANS )
  {
    // Convert from alien to human credits
    ent->client->pers.credit =
      (int)( ent->client->pers.credit *
             HUMAN_MAX_CREDITS / ALIEN_MAX_CREDITS + 0.5f );
  }

  // Copy credits to ps for the client
  ent->client->ps.persistant[ PERS_CREDIT ] = ent->client->pers.credit;

  ClientUserinfoChanged( ent->client->ps.clientNum );

  G_UpdateTeamConfigStrings( );

  if( oldTeam != TEAM_NONE && newTeam != TEAM_NONE )
  {
    G_LogPrintf(
      "team: %i %i %i: %s" S_COLOR_WHITE " left the %ss and joined the %ss\n",
       ent->s.number, newTeam, oldTeam, ent->client->pers.netname,
       BG_TeamName( oldTeam ), BG_TeamName( newTeam ) );
  }
  else if( newTeam == TEAM_NONE )
  {
    G_LogPrintf( "team: %i %i %i: %s" S_COLOR_WHITE " left the %ss\n",
      ent->s.number, newTeam, oldTeam, ent->client->pers.netname,
      BG_TeamName( oldTeam ) );
  }
  else
  {
    G_LogPrintf( "team: %i %i %i: %s" S_COLOR_WHITE " joined the %ss\n",
      ent->s.number, newTeam, oldTeam, ent->client->pers.netname,
      BG_TeamName( newTeam ) );
  }
}

/*
===========
Team_GetLocation

Report a location for the player. Uses placed nearby target_location entities
============
*/
gentity_t *Team_GetLocation( gentity_t *ent )
{
  gentity_t   *eloc, *best;
  float       bestlen, len;
  vec3_t      origin;

  best = NULL;
  bestlen = 3.0f * 8192.0f * 8192.0f;

  VectorCopy( ent->r.currentOrigin, origin );

  for( eloc = level.locationHead; eloc; eloc = eloc->nextTrain )
  {
    len = ( origin[ 0 ] - eloc->r.currentOrigin[ 0 ] ) * ( origin[ 0 ] - eloc->r.currentOrigin[ 0 ] )
        + ( origin[ 1 ] - eloc->r.currentOrigin[ 1 ] ) * ( origin[ 1 ] - eloc->r.currentOrigin[ 1 ] )
        + ( origin[ 2 ] - eloc->r.currentOrigin[ 2 ] ) * ( origin[ 2 ] - eloc->r.currentOrigin[ 2 ] );

    if( len > bestlen )
      continue;

    if( !trap_InPVS( origin, eloc->r.currentOrigin ) )
      continue;

    bestlen = len;
    best = eloc;
  }

  return best;
}


/*
===========
Team_GetLocationMsg

Report a location message for the player. Uses placed nearby target_location entities
============
*/
qboolean Team_GetLocationMsg( gentity_t *ent, char *loc, int loclen )
{
  gentity_t *best;

  best = Team_GetLocation( ent );

  if( !best )
    return qfalse;

  if( best->count )
  {
    if( best->count < 0 )
      best->count = 0;

    if( best->count > 7 )
      best->count = 7;

    Com_sprintf( loc, loclen, "%c%c%s" S_COLOR_WHITE, Q_COLOR_ESCAPE, best->count + '0', best->message );
  }
  else
    Com_sprintf( loc, loclen, "%s", best->message );

  return qtrue;
}


/*---------------------------------------------------------------------------*/

static int QDECL SortClients( const void *a, const void *b )
{
  return *(int *)a - *(int *)b;
}


/*
==================
TeamplayLocationsMessage

Format:
  clientNum location health armor weapon misc

==================
*/
void TeamplayInfoMessage( gentity_t *ent )
{
  char      entry[ 1024 ];
  char      string[ 8192 ];
  int       stringlength;
  int       i, j;
  gentity_t *player;
  int       cnt;
  int       h, a = 0;
  int       clients[ TEAM_MAXOVERLAY ];

  if( ! ent->client->pers.teamInfo )
    return;

  // figure out what client should be on the display
  // we are limited to 8, but we want to use the top eight players
  // but in client order (so they don't keep changing position on the overlay)
  for( i = 0, cnt = 0; i < g_maxclients.integer && cnt < TEAM_MAXOVERLAY; i++ )
  {
    player = g_entities + level.sortedClients[ i ];

    if( player->inuse && player->client->ps.stats[ STAT_TEAM ] ==
        ent->client->ps.stats[ STAT_TEAM ] )
      clients[ cnt++ ] = level.sortedClients[ i ];
  }

  // We have the top eight players, sort them by clientNum
  qsort( clients, cnt, sizeof( clients[ 0 ] ), SortClients );

  // send the latest information on all clients
  string[ 0 ] = 0;
  stringlength = 0;

  for( i = 0, cnt = 0; i < g_maxclients.integer && cnt < TEAM_MAXOVERLAY; i++)
  {
    player = g_entities + i;

    if( player->inuse && player->client->ps.stats[ STAT_TEAM ] ==
        ent->client->ps.stats[ STAT_TEAM ] )
    {
      h = player->client->ps.stats[ STAT_HEALTH ];

      if( h < 0 )
        h = 0;

      Com_sprintf( entry, sizeof( entry ),
        " %i %i %i %i %i",
        i, player->client->pers.location, h, a,
        player->client->ps.weapon );

      j = strlen( entry );

      if( stringlength + j > sizeof( string ) )
        break;

      strcpy( string + stringlength, entry );
      stringlength += j;
      cnt++;
    }
  }

  trap_SendServerCommand( ent - g_entities, va( "tinfo %i %s", cnt, string ) );
}

void CheckTeamStatus( void )
{
  int i;
  gentity_t *loc, *ent;

  if( level.time - level.lastTeamLocationTime > TEAM_LOCATION_UPDATE_TIME )
  {
    level.lastTeamLocationTime = level.time;

    for( i = 0; i < g_maxclients.integer; i++ )
    {
      ent = g_entities + i;
      if( ent->client->pers.connected != CON_CONNECTED )
        continue;

      if( ent->inuse && ( ent->client->ps.stats[ STAT_TEAM ] == TEAM_HUMANS ||
                          ent->client->ps.stats[ STAT_TEAM ] == TEAM_ALIENS ) )
      {

        loc = Team_GetLocation( ent );

        if( loc )
          ent->client->pers.location = loc->s.generic1;
        else
          ent->client->pers.location = 0;
      }
    }

    for( i = 0; i < g_maxclients.integer; i++ )
    {
      ent = g_entities + i;
      if( ent->client->pers.connected != CON_CONNECTED )
        continue;

      if( ent->inuse && ( ent->client->ps.stats[ STAT_TEAM ] == TEAM_HUMANS ||
                          ent->client->ps.stats[ STAT_TEAM ] == TEAM_ALIENS ) )
        TeamplayInfoMessage( ent );
    }
  }
}
