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
=======================================================================

  SESSION DATA

Session data is the only data that stays persistant across level loads
and tournament restarts.
=======================================================================
*/

/*
================
G_WriteClientSessionData

Called on game shutdown
================
*/
void G_WriteClientSessionData( gclient_t *client ) {
	const char	*s;
	const char	*var;

	s = va("%i %i %i %i %i %i %i", 
		client->sess.sessionTeam,
		client->sess.spectatorTime,
		client->sess.spectatorState,
		client->sess.spectatorClient,
		client->sess.wins,
		client->sess.losses,
    client->sess.teamLeader
		);

	var = va( "session%i", client - level.clients );

	trap_Cvar_Set( var, s );
}

/*
================
G_ReadSessionData

Called on a reconnect
================
*/
void G_ReadSessionData( gclient_t *client ) {
	char	s[MAX_STRING_CHARS];
	const char	*var;

  // bk001205 - format
  int teamLeader;
  int spectatorState;
  int sessionTeam;

	var = va( "session%i", client - level.clients );
	trap_Cvar_VariableStringBuffer( var, s, sizeof(s) );

	sscanf( s, "%i %i %i %i %i %i %i", 
		&sessionTeam,
		&client->sess.spectatorTime,
		&spectatorState,
		&client->sess.spectatorClient,
		&client->sess.wins,
		&client->sess.losses,
    &teamLeader
		);

  // bk001205 - format issues
  client->sess.sessionTeam = (team_t)sessionTeam;
  client->sess.spectatorState = (spectatorState_t)spectatorState;
  client->sess.teamLeader = (qboolean)teamLeader;
}


/*
================
G_InitSessionData

Called on a first-time connect
================
*/
void G_InitSessionData( gclient_t *client, char *userinfo ) {
	clientSession_t	*sess;
	const char		*value;

	sess = &client->sess;

	// initial team determination
  value = Info_ValueForKey( userinfo, "team" );
  if( value[ 0 ] == 's' )
  {
    // a willing spectator, not a waiting-in-line
    sess->sessionTeam = TEAM_SPECTATOR;
  }
  else
  {
    if( g_maxGameClients.integer > 0 && 
      level.numNonSpectatorClients >= g_maxGameClients.integer )
      sess->sessionTeam = TEAM_SPECTATOR;
    else
      sess->sessionTeam = TEAM_FREE;
  }

	sess->spectatorState = SPECTATOR_FREE;
	sess->spectatorTime = level.time;

	G_WriteClientSessionData( client );
}


/*
==================
G_WriteSessionData

==================
*/
void G_WriteSessionData( void ) {
	int		i;

  //TA: ?
	trap_Cvar_Set( "session", va( "%i", 0 ) );

	for( i = 0 ; i < level.maxclients ; i++ )
  {
		if( level.clients[ i ].pers.connected == CON_CONNECTED )
			G_WriteClientSessionData( &level.clients[ i ] );
	}
}
