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

// this file holds commands that can be executed by the server console, but not remote clients

#include "g_local.h"

/*
===================
Svcmd_EntityList_f
===================
*/
void  Svcmd_EntityList_f( void )
{
  int       e;
  gentity_t *check;

  check = g_entities;

  for( e = 0; e < level.num_entities; e++, check++ )
  {
    if( !check->inuse )
      continue;

    G_Printf( "%3i:", e );

    switch( check->s.eType )
    {
      case ET_GENERAL:
        G_Printf( "ET_GENERAL          " );
        break;
      case ET_PLAYER:
        G_Printf( "ET_PLAYER           " );
        break;
      case ET_ITEM:
        G_Printf( "ET_ITEM             " );
        break;
      case ET_BUILDABLE:
        G_Printf( "ET_BUILDABLE        " );
        break;
      case ET_MISSILE:
        G_Printf( "ET_MISSILE          " );
        break;
      case ET_MOVER:
        G_Printf( "ET_MOVER            " );
        break;
      case ET_BEAM:
        G_Printf( "ET_BEAM             " );
        break;
      case ET_PORTAL:
        G_Printf( "ET_PORTAL           " );
        break;
      case ET_SPEAKER:
        G_Printf( "ET_SPEAKER          " );
        break;
      case ET_PUSH_TRIGGER:
        G_Printf( "ET_PUSH_TRIGGER     " );
        break;
      case ET_TELEPORT_TRIGGER:
        G_Printf( "ET_TELEPORT_TRIGGER " );
        break;
      case ET_INVISIBLE:
        G_Printf( "ET_INVISIBLE        " );
        break;
      case ET_GRAPPLE:
        G_Printf( "ET_GRAPPLE          " );
        break;
      default:
        G_Printf( "%3i                 ", check->s.eType );
        break;
    }

    if( check->classname )
      G_Printf( "%s", check->classname );

    G_Printf( "\n" );
  }
}

gclient_t *ClientForString( const char *s )
{
  gclient_t *cl;
  int       i;
  int       idnum;

  // numeric values are just slot numbers
  if( s[ 0 ] >= '0' && s[ 0 ] <= '9' )
  {
    idnum = atoi( s );

    if( idnum < 0 || idnum >= level.maxclients )
    {
      Com_Printf( "Bad client slot: %i\n", idnum );
      return NULL;
    }

    cl = &level.clients[ idnum ];

    if( cl->pers.connected == CON_DISCONNECTED )
    {
      G_Printf( "Client %i is not connected\n", idnum );
      return NULL;
    }

    return cl;
  }

  // check for a name match
  for( i = 0; i < level.maxclients; i++ )
  {
    cl = &level.clients[ i ];
    if( cl->pers.connected == CON_DISCONNECTED )
      continue;

    if( !Q_stricmp( cl->pers.netname, s ) )
      return cl;
  }

  G_Printf( "User %s is not on the server\n", s );

  return NULL;
}

/*
===================
Svcmd_ForceTeam_f

forceteam <player> <team>
===================
*/
void  Svcmd_ForceTeam_f( void )
{
  gclient_t *cl;
  char      str[ MAX_TOKEN_CHARS ];

  // find the player
  trap_Argv( 1, str, sizeof( str ) );
  cl = ClientForString( str );

  if( !cl )
    return;

  // set the team
  trap_Argv( 2, str, sizeof( str ) );
  /*SetTeam( &g_entities[cl - level.clients], str );*/
  //FIXME: tremulise this
}

/*
===================
Svcmd_LayoutSave_f

layoutsave <name>
===================
*/
void  Svcmd_LayoutSave_f( void )
{
  char str[ MAX_QPATH ];
  char str2[ MAX_QPATH - 4 ];
  char *s;
  int i = 0;

  if( trap_Argc( ) != 2 )
  {
    G_Printf( "usage: layoutsave LAYOUTNAME\n" );
    return;
  }
  trap_Argv( 1, str, sizeof( str ) );

  // sanitize name
  s = &str[ 0 ];
  while( *s && i < sizeof( str2 ) - 1 )
  {
    if( ( *s >= '0' && *s <= '9' ) ||
      ( *s >= 'a' && *s <= 'z' ) ||
      ( *s >= 'A' && *s <= 'Z' ) || *s == '-' || *s == '_' )
    {
      str2[ i++ ] = *s;
      str2[ i ] = '\0';
    }
    s++;
  }

  if( !str2[ 0 ] )
  {
    G_Printf("layoutsave: invalid name \"%s\"\n", str );
    return;
  }

  G_LayoutSave( str2 );
}

char  *ConcatArgs( int start );

/*
===================
Svcmd_LayoutLoad_f

layoutload [<name> [<name2> [<name3 [...]]]]

This is just a silly alias for doing:
 set g_layouts "name name2 name3"
 map_restart
===================
*/
void  Svcmd_LayoutLoad_f( void )
{
  char layouts[ MAX_CVAR_VALUE_STRING ];
  char *s;

  s = ConcatArgs( 1 );
  Q_strncpyz( layouts, s, sizeof( layouts ) );
  trap_Cvar_Set( "g_layouts", layouts ); 
  trap_SendConsoleCommand( EXEC_APPEND, "map_restart\n" );
  level.restarted = qtrue;
}

static void Svcmd_AdmitDefeat_f( void )
{
  int  team;
  char teamNum[ 2 ];

  if( trap_Argc( ) != 2 )
  {
    G_Printf("admitdefeat: must provide a team\n");
    return;
  }
  trap_Argv( 1, teamNum, sizeof( teamNum ) );
  team = atoi( teamNum );
  if( team == TEAM_ALIENS || teamNum[ 0 ] == 'a' )
  {
    level.surrenderTeam = TEAM_ALIENS;
    G_BaseSelfDestruct( TEAM_ALIENS );
    G_TeamCommand( TEAM_ALIENS, "cp \"Hivemind Link Broken\" 1");
    trap_SendServerCommand( -1, "print \"Alien team has admitted defeat\n\"" );
  }
  else if( team == TEAM_HUMANS || teamNum[ 0 ] == 'h' )
  {
    level.surrenderTeam = TEAM_HUMANS;
    G_BaseSelfDestruct( TEAM_HUMANS );
    G_TeamCommand( TEAM_HUMANS, "cp \"Life Support Terminated\" 1");
    trap_SendServerCommand( -1, "print \"Human team has admitted defeat\n\"" );
  }
  else
  {
    G_Printf("admitdefeat: invalid team\n");
  } 
}

/*
=================
ConsoleCommand

=================
*/
qboolean  ConsoleCommand( void )
{
  char cmd[ MAX_TOKEN_CHARS ];

  trap_Argv( 0, cmd, sizeof( cmd ) );

  if( Q_stricmp( cmd, "entitylist" ) == 0 )
  {
    Svcmd_EntityList_f( );
    return qtrue;
  }

  if( Q_stricmp( cmd, "forceteam" ) == 0 )
  {
    Svcmd_ForceTeam_f( );
    return qtrue;
  }

  if( Q_stricmp( cmd, "mapRotation" ) == 0 )
  {
    char *rotationName = ConcatArgs( 1 );

    if( !G_StartMapRotation( rotationName, qfalse ) )
      G_Printf( "Can't find map rotation %s\n", rotationName );

    return qtrue;
  }

  if( Q_stricmp( cmd, "stopMapRotation" ) == 0 )
  {
    G_StopMapRotation( );

    return qtrue;
  }

  if( Q_stricmp( cmd, "advanceMapRotation" ) == 0 )
  {
    G_AdvanceMapRotation( );

    return qtrue;
  }

  if( Q_stricmp( cmd, "alienWin" ) == 0 )
  {
    G_BaseSelfDestruct( TEAM_HUMANS );
    return qtrue;
  }

  if( Q_stricmp( cmd, "humanWin" ) == 0 )
  {
    G_BaseSelfDestruct( TEAM_ALIENS );
    return qtrue;
  }

  if( !Q_stricmp( cmd, "layoutsave" ) )
  {
    Svcmd_LayoutSave_f( );
    return qtrue;
  }
  
  if( !Q_stricmp( cmd, "layoutload" ) )
  {
    Svcmd_LayoutLoad_f( );
    return qtrue;
  }
  
  if( !Q_stricmp( cmd, "admitdefeat" ) )
  {
    Svcmd_AdmitDefeat_f( );
    return qtrue;
  }

  if( !Q_stricmp( cmd, "evacuation" ) )
  {
    trap_SendServerCommand( -1, "print \"Evacuation ordered\n\"" );
    level.lastWin = TEAM_NONE;
    trap_SetConfigstring( CS_WINNER, "Evacuation" );
    LogExit( "Evacuation." );
    return qtrue;
  }
  
  // see if this is a a admin command
  if( G_admin_cmd_check( NULL, qfalse ) )
    return qtrue;

  if( g_dedicated.integer )
  {
    if( Q_stricmp( cmd, "say" ) == 0 )
    {
      trap_SendServerCommand( -1, va( "print \"server: %s\n\"", ConcatArgs( 1 ) ) );
      return qtrue;
    }
    else if( !Q_stricmp( cmd, "chat" ) )
    {
      trap_SendServerCommand( -1, va( "chat \"%s\" -1 0", ConcatArgs( 1 ) ) );
      G_Printf( "chat: %s\n", ConcatArgs( 1 ) );
      return qtrue;
    }
    else if( !Q_stricmp( cmd, "cp" ) )
    {
      trap_SendServerCommand( -1, va( "cp \"%s\"", ConcatArgs( 1 ) ) );
      G_Printf( "cp: %s\n", ConcatArgs( 1 ) );
      return qtrue;
    }
    else if( !Q_stricmp( cmd, "m" ) )
    {
      Cmd_PrivateMessage_f( NULL );
      return qtrue;
    }
    else if( !Q_stricmp( cmd, "a" ) )
    {
      Cmd_AdminMessage_f( NULL );
      return qtrue;
    }
    G_Printf( "unknown command: %s\n", cmd );
    return qtrue;
  }

  return qfalse;
}

