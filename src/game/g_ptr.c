// g_ptr.c -- post timeout restoration handling

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

static connectionRecord_t connections[ MAX_CLIENTS ];

/*
===============
G_CheckForUniquePTRC

Callback to detect ptrc clashes
===============
*/
static qboolean G_CheckForUniquePTRC( int code )
{
  int i;
  
  if( code == 0 )
    return qfalse;

  for( i = 0; i < MAX_CLIENTS; i++ )
  {
    if( connections[ i ].ptrCode == code )
      return qfalse;
  }
  
  return qtrue;
}

/*
===============
G_UpdatePTRConnection

Update the data in a connection record
===============
*/
void G_UpdatePTRConnection( gclient_t *client )
{
  if( client && client->pers.connection )
  {
    client->pers.connection->clientTeam = client->ps.stats[ STAT_PTEAM ];
    client->pers.connection->clientCredit = client->ps.persistant[ PERS_CREDIT ];
  }
}

/*
===============
G_GenerateNewConnection

Generates a new connection
===============
*/
connectionRecord_t *G_GenerateNewConnection( gclient_t *client )
{
  int     code = 0;
  int     i;
  qtime_t time;

  // this should be really random
  trap_RealTime( &time );
  srand( time.tm_sec + time.tm_min + time.tm_hour );
  
  // there is a very very small possibility that this
  // will loop infinitely
  do
  {
    code = rand( );
  } while( !G_CheckForUniquePTRC( code ) );

  for( i = 0; i < MAX_CLIENTS; i++ )
  {
    //found an unused slot
    if( !connections[ i ].ptrCode )
    {
      connections[ i ].ptrCode = code;
      connections[ i ].clientNum = client->ps.clientNum;
      client->pers.connection = &connections[ i ];
      G_UpdatePTRConnection( client );

      return &connections[ i ];
    }
  }

  return NULL;
}

/*
===============
G_VerifyPTRC

Check a PTR code for validity
===============
*/
qboolean G_VerifyPTRC( int code )
{
  int i;
  
  if( code == 0 )
    return qfalse;
  
  for( i = 0; i < MAX_CLIENTS; i++ )
  {
    if( connections[ i ].ptrCode == code )
      return qtrue;
  }
  
  return qfalse;
}

/*
===============
G_FindConnectionForCode

Finds a connection for a given code
===============
*/
connectionRecord_t *G_FindConnectionForCode( int code )
{
  int i;
  
  if( code == 0 )
    return NULL;

  for( i = 0; i < MAX_CLIENTS; i++ )
  {
    if( connections[ i ].ptrCode == code )
      return &connections[ i ];
  }

  return NULL;
}

/*
===============
G_DeletePTRConnection

Finds a connection and deletes it
===============
*/
void G_DeletePTRConnection( connectionRecord_t *connection )
{
  if( connection )
    memset( connection, 0, sizeof( connectionRecord_t ) );
}

/*
===============
G_ResetPTRConnections

Invalidate any existing codes
===============
*/
void G_ResetPTRConnections( void )
{
  memset( connections, 0, sizeof( connectionRecord_t ) * MAX_CLIENTS );
}
