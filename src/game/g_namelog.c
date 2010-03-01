/*
===========================================================================
Copyright (C) 2010 Darklegion Development

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

void G_namelog_cleanup( void )
{
  namelog_t *namelog, *n;

  for( namelog = level.namelogs; namelog; namelog = n )
  {
    n = namelog->next;
    BG_Free( namelog );
  }
}

void G_namelog_connect( gclient_t *client )
{
  namelog_t *n, *p = NULL;
  int       i;

  for( n = level.namelogs; n; p = n, n = n->next )
  {
    if( n->slot != -1 )
      continue;
    if( !Q_stricmp( client->pers.guid, n->guid ) )
      break;
  }
  if( !n )
  {
    n = BG_Alloc( sizeof( namelog_t ) );
    strcpy( n->guid, client->pers.guid );
    if( p )
      p->next = n;
    else
      level.namelogs = n;
  }
  client->pers.namelog = n;
  n->slot = client - level.clients;
  n->banned = qfalse;

  G_namelog_update_name( client );

  for( i = 0; i < MAX_NAMELOG_ADDRS && n->ip[ i ].str[ 0 ]; i++ )
    if( !strcmp( n->ip[ i ].str, client->pers.ip.str ) )
      return;
  if( i == MAX_NAMELOG_ADDRS )
    i--;
  memcpy( &n->ip[ i ], &client->pers.ip, sizeof( n->ip[ i ] ) );
}

void G_namelog_disconnect( gclient_t *client )
{
  client->pers.namelog->slot = -1;
  client->pers.namelog = NULL;
}

void G_namelog_update_score( gclient_t *client )
{
  namelog_t *n = client->pers.namelog;

  n->team = client->pers.teamSelection;
  n->score = client->ps.persistant[ PERS_SCORE ];
  n->credits = client->pers.credit;
}

void G_namelog_update_name( gclient_t *client )
{
  int       i;
  char      n1[ MAX_NAME_LENGTH ], n2[ MAX_NAME_LENGTH ];
  namelog_t *n = client->pers.namelog;

  G_SanitiseString( client->pers.netname, n1, sizeof( n1 ) );
  for( i = 0; i < MAX_NAMELOG_NAMES && n->name[ i ][ 0 ]; i++ )
  {
    G_SanitiseString( n->name[ i ], n2, sizeof( n2 ) );
    if( !strcmp( n1, n2 ) )
      return;
  }
  strcpy( n->name[ n->nameChanges % MAX_NAMELOG_NAMES ], client->pers.netname );
}

void G_namelog_restore( gclient_t *client )
{
  namelog_t *n = client->pers.namelog;

  G_ChangeTeam( g_entities + n->slot, n->team );
  client->ps.persistant[ PERS_SCORE ] = n->score;
  client->ps.persistant[ PERS_CREDIT ] = 0;
  G_AddCreditToClient( client, n->credits, qfalse );
}

