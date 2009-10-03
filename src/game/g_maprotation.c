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

// g_maprotation.c -- the map rotation system

#include "g_local.h"

#define MAX_MAP_ROTATIONS       16
#define MAX_MAP_ROTATION_MAPS   64
#define MAX_MAP_COMMANDS        16

#define NOT_ROTATING            -1

typedef enum
{
  CV_ERR,
  CV_RANDOM,
  CV_NUMCLIENTS,
  CV_LASTWIN
} conditionVariable_t;

typedef enum
{
  CO_LT,
  CO_EQ,
  CO_GT
} conditionOp_t;

typedef enum
{
  CT_ERR,
  CT_MAP,
  CT_ROTATION
} conditionType_t;

typedef struct condition_s
{
  char                dest[ MAX_QPATH ];

  qboolean            unconditional;

  conditionVariable_t lhs;
  conditionOp_t       op;

  int                 numClients;
  team_t              lastWin;
} condition_t;

typedef struct map_s
{
  char  name[ MAX_QPATH ];

  char  postCmds[ MAX_MAP_COMMANDS ][ MAX_STRING_CHARS ];
  int   numCmds;

  char  layouts[ MAX_CVAR_VALUE_STRING ];
} map_t;

typedef enum
{
  NT_MAP,
  NT_CONDITION
} nodeType_t;

typedef struct node_s
{
  union
  {
    map_t       map;
    condition_t condition;
  } u;

  nodeType_t type;
} node_t;

typedef struct mapRotation_s
{
  char    name[ MAX_QPATH ];

  node_t  nodes[ MAX_MAP_ROTATION_MAPS ];
  int     numNodes;
  int     currentNode;
} mapRotation_t;

typedef struct mapRotations_s
{
  mapRotation_t   rotations[ MAX_MAP_ROTATIONS ];
  int             numRotations;
} mapRotations_t;

static mapRotations_t mapRotations;

/*
===============
G_MapExists

Check if a map exists
===============
*/
qboolean G_MapExists( char *name )
{
  return trap_FS_FOpenFile( va( "maps/%s.bsp", name ), NULL, FS_READ );
}

/*
===============
G_RotationExists

Check if a rotation exists
===============
*/
static qboolean G_RotationExists( char *name )
{
  int i;

  for( i = 0; i < mapRotations.numRotations; i++ )
  {
    if( Q_strncmp( mapRotations.rotations[ i ].name, name, MAX_QPATH ) == 0 )
      return qtrue;
  }

  return qfalse;
}

/*
===============
G_ParseCommandSection

Parse a map rotation command section
===============
*/
static qboolean G_ParseMapCommandSection( node_t *node, char **text_p )
{
  char  *token;
  map_t *map = &node->u.map;

  // read optional parameters
  while( 1 )
  {
    token = COM_Parse( text_p );

    if( !*token )
      break;

    if( !Q_stricmp( token, "" ) )
      return qfalse;

    if( !Q_stricmp( token, "}" ) )
      return qtrue; //reached the end of this command section

    if( !Q_stricmp( token, "layouts" ) )
    {
      token = COM_ParseExt( text_p, qfalse );
      map->layouts[ 0 ] = '\0';

      while( token[ 0 ] != 0 )
      {
        Q_strcat( map->layouts, sizeof( map->layouts ), token );
        Q_strcat( map->layouts, sizeof( map->layouts ), " " );
        token = COM_ParseExt( text_p, qfalse );
      }

      continue;
    }

    if( map->numCmds == MAX_MAP_COMMANDS )
    {
      G_Printf( S_COLOR_RED "ERROR: maximum number of map commands (%d) reached\n",
                MAX_MAP_COMMANDS );
      return qfalse;
    }

    Q_strncpyz( map->postCmds[ map->numCmds ], token, sizeof( map->postCmds[ 0 ] ) );
    Q_strcat( map->postCmds[ map->numCmds ], sizeof( map->postCmds[ 0 ] ), " " );

    token = COM_ParseExt( text_p, qfalse );

    while( token[ 0 ] != 0 )
    {
      Q_strcat( map->postCmds[ map->numCmds ], sizeof( map->postCmds[ 0 ] ), token );
      Q_strcat( map->postCmds[ map->numCmds ], sizeof( map->postCmds[ 0 ] ), " " );
      token = COM_ParseExt( text_p, qfalse );
    }
    map->numCmds++;
  }

  return qfalse;
}

/*
===============
G_ParseMapRotation

Parse a map rotation section
===============
*/
static qboolean G_ParseMapRotation( mapRotation_t *mr, char **text_p )
{
  char      *token;
  qboolean  mnSet = qfalse;
  node_t    *node = NULL;

  // read optional parameters
  while( 1 )
  {
    token = COM_Parse( text_p );

    if( !*token )
      break;

    if( !Q_stricmp( token, "" ) )
      return qfalse;

    node = &mr->nodes[ mr->numNodes ];

    if( !Q_stricmp( token, "{" ) )
    {
      if( !mnSet )
      {
        G_Printf( S_COLOR_RED "ERROR: map settings section with no name\n" );
        return qfalse;
      }

      if( !G_ParseMapCommandSection( node, text_p ) )
      {
        G_Printf( S_COLOR_RED "ERROR: failed to parse map command section\n" );
        return qfalse;
      }

      mnSet = qfalse;
      continue;
    }
    else if( !Q_stricmp( token, "goto" ) )
    {
      condition_t *condition = &node->u.condition;

      token = COM_Parse( text_p );

      if( !*token )
        break;

      condition->unconditional = qtrue;
      Q_strncpyz( condition->dest, token, sizeof( condition->dest ) );

      node->type = NT_CONDITION;
      mr->numNodes++;
      continue;
    }
    else if( !Q_stricmp( token, "if" ) )
    {
      condition_t *condition = &node->u.condition;

      token = COM_Parse( text_p );

      if( !*token )
        break;

      if( !Q_stricmp( token, "numClients" ) )
      {
        condition->lhs = CV_NUMCLIENTS;

        token = COM_Parse( text_p );

        if( !*token )
          break;

        if( !Q_stricmp( token, "<" ) )
          condition->op = CO_LT;
        else if( !Q_stricmp( token, ">" ) )
          condition->op = CO_GT;
        else if( !Q_stricmp( token, "=" ) )
          condition->op = CO_EQ;
        else
        {
          G_Printf( S_COLOR_RED "ERROR: invalid operator in expression: %s\n", token );
          return qfalse;
        }

        token = COM_Parse( text_p );

        if( !*token )
          break;

        condition->numClients = atoi( token );
      }
      else if( !Q_stricmp( token, "lastWin" ) )
      {
        condition->lhs = CV_LASTWIN;

        token = COM_Parse( text_p );

        if( !*token )
          break;

        if( !Q_stricmp( token, "aliens" ) )
          condition->lastWin = TEAM_ALIENS;
        else if( !Q_stricmp( token, "humans" ) )
          condition->lastWin = TEAM_HUMANS;
        else
        {
          G_Printf( S_COLOR_RED "ERROR: invalid right hand side in expression: %s\n", token );
          return qfalse;
        }
      }
      else if( !Q_stricmp( token, "random" ) )
        condition->lhs = CV_RANDOM;
      else
      {
        G_Printf( S_COLOR_RED "ERROR: invalid left hand side in expression: %s\n", token );
        return qfalse;
      }

      token = COM_Parse( text_p );

      if( !*token )
        break;

      condition->unconditional = qfalse;
      Q_strncpyz( condition->dest, token, sizeof( condition->dest ) );

      node->type = NT_CONDITION;
      mr->numNodes++;
      continue;
    }
    else if( !Q_stricmp( token, "}" ) )
      return qtrue; //reached the end of this map rotation

    if( mr->numNodes == MAX_MAP_ROTATION_MAPS )
    {
      G_Printf( S_COLOR_RED "ERROR: maximum number of maps in one rotation (%d) reached\n",
                MAX_MAP_ROTATION_MAPS );
      return qfalse;
    }

    Q_strncpyz( node->u.map.name, token, sizeof( node->u.map.name ) );
    mnSet = qtrue;

    node->type = NT_MAP;
    mr->numNodes++;
  }

  return qfalse;
}

/*
===============
G_ParseMapRotationFile

Load the map rotations from a map rotation file
===============
*/
static qboolean G_ParseMapRotationFile( const char *fileName )
{
  char          *text_p;
  int           i, j;
  int           len;
  char          *token;
  char          text[ 20000 ];
  char          mrName[ MAX_QPATH ];
  qboolean      mrNameSet = qfalse;
  fileHandle_t  f;

  // load the file
  len = trap_FS_FOpenFile( fileName, &f, FS_READ );
  if( len < 0 )
    return qfalse;

  if( len == 0 || len >= sizeof( text ) - 1 )
  {
    trap_FS_FCloseFile( f );
    G_Printf( S_COLOR_RED "ERROR: map rotation file %s is %s\n", fileName,
      len == 0 ? "empty" : "too long" );
    return qfalse;
  }

  trap_FS_Read( text, len, f );
  text[ len ] = 0;
  trap_FS_FCloseFile( f );

  // parse the text
  text_p = text;

  // read optional parameters
  while( 1 )
  {
    token = COM_Parse( &text_p );

    if( !*token )
      break;

    if( !Q_stricmp( token, "" ) )
      break;

    if( !Q_stricmp( token, "{" ) )
    {
      if( mrNameSet )
      {
        //check for name space clashes
        for( i = 0; i < mapRotations.numRotations; i++ )
        {
          if( !Q_stricmp( mapRotations.rotations[ i ].name, mrName ) )
          {
            G_Printf( S_COLOR_RED "ERROR: a map rotation is already named %s\n", mrName );
            return qfalse;
          }
        }

        if( mapRotations.numRotations == MAX_MAP_ROTATIONS )
        {
          G_Printf( S_COLOR_RED "ERROR: maximum number of map rotations (%d) reached\n",
                    MAX_MAP_ROTATIONS );
          return qfalse;
        }

        Q_strncpyz( mapRotations.rotations[ mapRotations.numRotations ].name, mrName, MAX_QPATH );

        if( !G_ParseMapRotation( &mapRotations.rotations[ mapRotations.numRotations ], &text_p ) )
        {
          G_Printf( S_COLOR_RED "ERROR: %s: failed to parse map rotation %s\n", fileName, mrName );
          return qfalse;
        }

        mapRotations.numRotations++;

        //start parsing map rotations again
        mrNameSet = qfalse;

        continue;
      }
      else
      {
        G_Printf( S_COLOR_RED "ERROR: unnamed map rotation\n" );
        return qfalse;
      }
    }

    if( !mrNameSet )
    {
      Q_strncpyz( mrName, token, sizeof( mrName ) );
      mrNameSet = qtrue;
    }
    else
    {
      G_Printf( S_COLOR_RED "ERROR: map rotation already named\n" );
      return qfalse;
    }
  }

  for( i = 0; i < mapRotations.numRotations; i++ )
  {
    for( j = 0; j < mapRotations.rotations[ i ].numNodes; j++ )
    {
      node_t *node = &mapRotations.rotations[ i ].nodes[ j ];

      switch( node->type )
      {
        case NT_MAP:
          if( !G_MapExists( node->u.map.name ) )
          {
            G_Printf( S_COLOR_RED "ERROR: map \"%s\" doesn't exist\n",
              node->u.map.name );
            return qfalse;
          }
          break;

        case NT_CONDITION:
          if( !G_MapExists( node->u.condition.dest ) &&
              !G_RotationExists( node->u.condition.dest ) )
          {
            G_Printf( S_COLOR_RED "ERROR: conditional destination \"%s\" doesn't exist\n",
              node->u.condition.dest );
            return qfalse;
          }
          break;
      }
    }
  }

  return qtrue;
}

/*
===============
G_PrintRotations

Print the parsed map rotations
===============
*/
void G_PrintRotations( void )
{
  int i, j, k;

  G_Printf( "Map rotations as parsed:\n\n" );

  for( i = 0; i < mapRotations.numRotations; i++ )
  {
    G_Printf( "rotation: %s\n{\n", mapRotations.rotations[ i ].name );

    for( j = 0; j < mapRotations.rotations[ i ].numNodes; j++ )
    {
      node_t *node = &mapRotations.rotations[ i ].nodes[ j ];

      switch( node->type )
      {
        case NT_MAP:
          G_Printf( "  map: %s\n", node->u.map.name );

          if( node->u.map.numCmds > 0 )
          {
            G_Printf( "  {\n" );

            for( k = 0; k < node->u.map.numCmds; k++ )
            {
              G_Printf( "    command: %s\n", node->u.map.postCmds[ k ] );
            }

            G_Printf( "  }\n" );
          }
          break;

        case NT_CONDITION:
          G_Printf( "  conditional: %s\n", node->u.condition.dest );
          break;
      }
    }

    G_Printf( "}\n" );
  }

  G_Printf( "Total memory used: %d bytes\n", sizeof( mapRotations ) );
}

/*
===============
G_GetCurrentNodeArray

Fill a static array with the current node of each rotation
===============
*/
static int *G_GetCurrentNodeArray( void )
{
  static int  currentNode[ MAX_MAP_ROTATIONS ];
  int         i = 0;
  char        text[ MAX_MAP_ROTATIONS * 2 ];
  char        *text_p, *token;

  Q_strncpyz( text, g_currentNode.string, sizeof( text ) );

  text_p = text;

  while( 1 )
  {
    token = COM_Parse( &text_p );

    if( !*token )
      break;

    if( !Q_stricmp( token, "" ) )
      break;

    currentNode[ i++ ] = atoi( token );
  }

  return currentNode;
}

/*
===============
G_SetCurrentNode

Set the current map in some rotation
===============
*/
static void G_SetCurrentNode( int currentNode, int rotation )
{
  char  text[ MAX_MAP_ROTATIONS * 2 ] = { 0 };
  int   *p = G_GetCurrentNodeArray( );
  int   i;

  p[ rotation ] = currentNode;

  for( i = 0; i < mapRotations.numRotations; i++ )
    Q_strcat( text, sizeof( text ), va( "%d ", p[ i ] ) );

  trap_Cvar_Set( "g_currentNode", text );
  trap_Cvar_Update( &g_currentNode );
}

/*
===============
G_GetCurrentNode

Return the current map in some rotation
===============
*/
static int G_GetCurrentNode( int rotation )
{
  int   *p = G_GetCurrentNodeArray( );

  return p[ rotation ];
}

/*
===============
G_IssueMapChange

Send commands to the server to actually change the map
===============
*/
static void G_IssueMapChange( int rotation )
{
  int     i;
  int     node = G_GetCurrentNode( rotation );
  char    cmd[ MAX_TOKEN_CHARS ];
  map_t   *map = &mapRotations.rotations[ rotation ].nodes[ node ].u.map;

  // allow a manually defined g_layouts setting to override the maprotation
  if( !g_layouts.string[ 0 ] && map->layouts[ 0 ] )
  {
    trap_Cvar_Set( "g_layouts", map->layouts );
  }

  trap_SendConsoleCommand( EXEC_APPEND, va( "map %s\n", map->name ) );

  // Load up map defaults if g_mapConfigs is set
  G_MapConfigs( map->name );

  for( i = 0; i < map->numCmds; i++ )
  {
    Q_strncpyz( cmd, map->postCmds[ i ], sizeof( cmd ) );
    Q_strcat( cmd, sizeof( cmd ), "\n" );
    trap_SendConsoleCommand( EXEC_APPEND, cmd );
  }
}

/*
===============
G_ResolveConditionDestination

Resolve the destination of some condition
===============
*/
static conditionType_t G_ResolveConditionDestination( int *n, char *name )
{
  int i;

  // Search the current rotation first...
  for( i = 0; i < mapRotations.rotations[ g_currentMapRotation.integer ].numNodes; i++ )
  {
    node_t *node = &mapRotations.rotations[ g_currentMapRotation.integer ].nodes[ i ];

    if( node->type == NT_MAP && !Q_stricmp( node->u.map.name, name ) )
    {
      *n = i;
      return CT_MAP;
    }
  }

  // ...then search the rotation names
  for( i = 0; i < mapRotations.numRotations; i++ )
  {
    if( !Q_stricmp( mapRotations.rotations[ i ].name, name ) )
    {
      *n = i;
      return CT_ROTATION;
    }
  }

  return CT_ERR;
}

/*
===============
G_EvaluateMapCondition

Evaluate a map condition
===============
*/
static qboolean G_EvaluateMapCondition( condition_t *condition )
{
  switch( condition->lhs )
  {
    case CV_RANDOM:
      return rand( ) & 1;
      break;

    case CV_NUMCLIENTS:
      switch( condition->op )
      {
        case CO_LT:
          return level.numConnectedClients < condition->numClients;
          break;

        case CO_GT:
          return level.numConnectedClients > condition->numClients;
          break;

        case CO_EQ:
          return level.numConnectedClients == condition->numClients;
          break;
      }
      break;

    case CV_LASTWIN:
      return level.lastWin == condition->lastWin;
      break;

    default:
    case CV_ERR:
      G_Printf( S_COLOR_RED "ERROR: malformed map switch condition\n" );
      break;
  }

  return qfalse;
}

/*
===============
G_AdvanceMapRotation

Increment the current map rotation
===============
*/
void G_AdvanceMapRotation( void )
{
  mapRotation_t *mr;
  node_t        *node;
  condition_t   *condition;
  int           currentRotation, currentNode, nextNode;
  int           n;

  if( ( currentRotation = g_currentMapRotation.integer ) == NOT_ROTATING )
    return;

  currentNode = G_GetCurrentNode( currentRotation );

  mr = &mapRotations.rotations[ currentRotation ];
  node = &mr->nodes[ currentNode ];
  nextNode = ( currentNode + 1 ) % mr->numNodes;

  if( node->type == NT_CONDITION )
  {
    condition = &node->u.condition;

    if( condition->unconditional || G_EvaluateMapCondition( condition ) )
    {
      switch( G_ResolveConditionDestination( &n, condition->dest ) )
      {
        case CT_MAP:
          nextNode = n;
          break;

        case CT_ROTATION:
          // Advance the current rotation so that if we come back to
          // it later, the next node in the rotation is evaluated
          G_SetCurrentNode( nextNode, currentRotation );
          G_StartMapRotation( condition->dest, qtrue );
          return;

        default:
        case CT_ERR:
          G_Printf( S_COLOR_YELLOW "WARNING: map switch destination could not be resolved: %s\n",
                    condition->dest );
          break;
      }
    }
  }

  G_SetCurrentNode( nextNode, currentRotation );
  G_IssueMapChange( currentRotation );
}

/*
===============
G_StartMapRotation

Switch to a new map rotation
===============
*/
qboolean G_StartMapRotation( char *name, qboolean changeMap )
{
  int i;

  for( i = 0; i < mapRotations.numRotations; i++ )
  {
    if( !Q_stricmp( mapRotations.rotations[ i ].name, name ) )
    {
      trap_Cvar_Set( "g_currentMapRotation", va( "%d", i ) );
      trap_Cvar_Update( &g_currentMapRotation );

      if( changeMap )
        G_IssueMapChange( i );
      break;
    }
  }

  if( i == mapRotations.numRotations )
    return qfalse;
  else
    return qtrue;
}

/*
===============
G_StopMapRotation

Stop the current map rotation
===============
*/
void G_StopMapRotation( void )
{
  trap_Cvar_Set( "g_currentMapRotation", va( "%d", NOT_ROTATING ) );
  trap_Cvar_Update( &g_currentMapRotation );
}

/*
===============
G_MapRotationActive

Test if any map rotation is currently active
===============
*/
qboolean G_MapRotationActive( void )
{
  return ( g_currentMapRotation.integer != NOT_ROTATING );
}

/*
===============
G_InitMapRotations

Load and intialise the map rotations
===============
*/
void G_InitMapRotations( void )
{
  const char  *fileName = "maprotation.cfg";

  // Load the file if it exists
  if( trap_FS_FOpenFile( fileName, NULL, FS_READ ) )
  {
    if( !G_ParseMapRotationFile( fileName ) )
      G_Printf( S_COLOR_RED "ERROR: failed to parse %s file\n", fileName );
  }
  else
    G_Printf( "%s file not found.\n", fileName );

  if( g_currentMapRotation.integer == NOT_ROTATING )
  {
    if( g_initialMapRotation.string[ 0 ] != 0 )
    {
      G_StartMapRotation( g_initialMapRotation.string, qfalse );

      trap_Cvar_Set( "g_initialMapRotation", "" );
      trap_Cvar_Update( &g_initialMapRotation );
    }
  }
}
