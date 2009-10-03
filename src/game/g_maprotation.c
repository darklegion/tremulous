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

#define MAX_MAP_ROTATIONS       64
#define MAX_MAP_ROTATION_MAPS   256

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
  DT_ERR,
  DT_MAP,
  DT_ROTATION
} destinationType_t;

typedef struct condition_s
{
  struct node_s       *target;

  conditionVariable_t lhs;
  conditionOp_t       op;

  int                 numClients;
  team_t              lastWin;
} condition_t;

typedef struct destination_s
{
  char  name[ MAX_QPATH ];

  char  postCommand[ MAX_STRING_CHARS ];
  char  layouts[ MAX_CVAR_VALUE_STRING ];
} destination_t;

typedef enum
{
  NT_DESTINATION,
  NT_CONDITION
} nodeType_t;

typedef struct node_s
{
  nodeType_t    type;

  union
  {
    destination_t destination;
    condition_t   condition;
  } u;

} node_t;

typedef struct mapRotation_s
{
  char    name[ MAX_QPATH ];

  node_t  *nodes[ MAX_MAP_ROTATION_MAPS ];
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
G_AllocateNode

Allocate memory for a node_t
===============
*/
static node_t *G_AllocateNode( void )
{
  node_t *node = BG_Alloc( sizeof( node_t ) );

  return node;
}

/*
===============
G_ParseMapCommandSection

Parse a map rotation command section
===============
*/
static qboolean G_ParseMapCommandSection( node_t *node, char **text_p )
{
  char          *token;
  destination_t *destination = &node->u.destination;
  int           commandLength = 0;

  // read optional parameters
  while( 1 )
  {
    token = COM_Parse( text_p );

    if( !*token )
      break;

    if( !Q_stricmp( token, "" ) )
      return qfalse;

    if( !Q_stricmp( token, "}" ) )
    {
      if( commandLength > 0 )
      {
        // Replace last ; with \n
        destination->postCommand[ commandLength - 1 ] = '\n';
      }

      return qtrue; //reached the end of this command section
    }

    if( !Q_stricmp( token, "layouts" ) )
    {
      token = COM_ParseExt( text_p, qfalse );
      destination->layouts[ 0 ] = '\0';

      while( token[ 0 ] != 0 )
      {
        Q_strcat( destination->layouts, sizeof( destination->layouts ), token );
        Q_strcat( destination->layouts, sizeof( destination->layouts ), " " );
        token = COM_ParseExt( text_p, qfalse );
      }

      continue;
    }

    // Parse the rest of the line into destination->postCommand
    Q_strcat( destination->postCommand, sizeof( destination->postCommand ), token );
    Q_strcat( destination->postCommand, sizeof( destination->postCommand ), " " );

    token = COM_ParseExt( text_p, qfalse );

    while( token[ 0 ] != 0 )
    {
      Q_strcat( destination->postCommand, sizeof( destination->postCommand ), token );
      Q_strcat( destination->postCommand, sizeof( destination->postCommand ), " " );
      token = COM_ParseExt( text_p, qfalse );
    }

    commandLength = strlen( destination->postCommand );
    destination->postCommand[ commandLength - 1 ] = ';';
  }

  return qfalse;
}

/*
===============
G_ParseNode

Parse a node
===============
*/
static qboolean G_ParseNode( node_t **node, char *token, char **text_p )
{
  if( !Q_stricmp( token, "if" ) )
  {
    condition_t *condition;

    (*node)->type = NT_CONDITION;
    condition = &(*node)->u.condition;

    token = COM_Parse( text_p );

    if( !*token )
      return qfalse;

    if( !Q_stricmp( token, "numClients" ) )
    {
      condition->lhs = CV_NUMCLIENTS;

      token = COM_Parse( text_p );

      if( !*token )
        return qfalse;

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
        return qfalse;

      condition->numClients = atoi( token );
    }
    else if( !Q_stricmp( token, "lastWin" ) )
    {
      condition->lhs = CV_LASTWIN;

      token = COM_Parse( text_p );

      if( !*token )
        return qfalse;

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
      return qfalse;

    condition->target = G_AllocateNode( );
    *node = condition->target;
    if( !G_ParseNode( node, token, text_p ) )
      return qfalse;
  }
  else
  {
    destination_t *destination;

    (*node)->type = NT_DESTINATION;
    destination = &(*node)->u.destination;

    Q_strncpyz( destination->name, token, sizeof( destination->name ) );
    destination->postCommand[ 0 ] = '\0';
  }

  return qtrue;
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
  node_t    *node = NULL;

  // read optional parameters
  while( 1 )
  {
    token = COM_Parse( text_p );

    if( !*token )
      break;

    if( !Q_stricmp( token, "" ) )
      return qfalse;

    if( !Q_stricmp( token, "{" ) )
    {
      if( node == NULL )
      {
        G_Printf( S_COLOR_RED "ERROR: map command section with no associated map\n" );
        return qfalse;
      }

      if( !G_ParseMapCommandSection( node, text_p ) )
      {
        G_Printf( S_COLOR_RED "ERROR: failed to parse map command section\n" );
        return qfalse;
      }

      continue;
    }
    else if( !Q_stricmp( token, "}" ) )
    {
      // Reached the end of this map rotation
      return qtrue;
    }

    if( mr->numNodes == MAX_MAP_ROTATION_MAPS )
    {
      G_Printf( S_COLOR_RED "ERROR: maximum number of maps in one rotation (%d) reached\n",
                MAX_MAP_ROTATION_MAPS );
      return qfalse;
    }

    node = G_AllocateNode( );
    mr->nodes[ mr->numNodes++ ] = node;

    if( !G_ParseNode( &node, token, text_p ) )
      return qfalse;
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
    mapRotation_t *mr = &mapRotations.rotations[ i ];
    int           destinationCount = 0;

    for( j = 0; j < mr->numNodes; j++ )
    {
      node_t        *node = mr->nodes[ j ];
      destination_t *destination;

      if( node->type == NT_DESTINATION )
        destinationCount++;
      else while( node->type == NT_CONDITION )
        node = node->u.condition.target;

      destination = &node->u.destination;

      if( !G_MapExists( destination->name ) &&
          !G_RotationExists( destination->name ) )
      {
        G_Printf( S_COLOR_RED "ERROR: conditional destination \"%s\" doesn't exist\n",
          destination->name );
        return qfalse;
      }
    }

    if( destinationCount == 0 )
    {
      G_Printf( S_COLOR_RED "ERROR: rotation \"%s\" needs at least one unconditional entry\n",
        mr->name );
      return qfalse;
    }
  }

  return qtrue;
}

/*
===============
G_PrintSpaces
===============
*/
static void G_PrintSpaces( int spaces )
{
  int i;

  for( i = 0; i < spaces; i++ )
    G_Printf( " " );
}

/*
===============
G_PrintRotations

Print the parsed map rotations
===============
*/
void G_PrintRotations( void )
{
  int i, j;
  int size = sizeof( mapRotations );

  G_Printf( "Map rotations as parsed:\n\n" );

  for( i = 0; i < mapRotations.numRotations; i++ )
  {
    mapRotation_t *mr = &mapRotations.rotations[ i ];

    G_Printf( "rotation: %s\n{\n", mr->name );

    size += mr->numNodes * sizeof( node_t );

    for( j = 0; j < mr->numNodes; j++ )
    {
      node_t *node = mr->nodes[ j ];
      int indentation = 0;

      while( node->type == NT_CONDITION )
      {
        G_PrintSpaces( indentation );
        G_Printf( "  condition\n" );
        node = node->u.condition.target;

        size += sizeof( node_t );

        indentation += 2;
      }

      G_PrintSpaces( indentation );
      G_Printf( "  %s\n", node->u.destination.name );

      if( strlen( node->u.destination.postCommand ) > 0 )
      {
        G_Printf( "    command: %s", node->u.destination.postCommand );
      }
    }

    G_Printf( "}\n" );
  }

  G_Printf( "Total memory used: %d bytes\n", size );
}

/*
===============
G_CurrentNodeIndexArray

Fill a static array with the current node of each rotation
===============
*/
static int *G_CurrentNodeIndexArray( void )
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
G_SetCurrentNodeByIndex

Set the current map in some rotation
===============
*/
static void G_SetCurrentNodeByIndex( int currentNode, int rotation )
{
  char  text[ MAX_MAP_ROTATIONS * 2 ] = { 0 };
  int   *p = G_CurrentNodeIndexArray( );
  int   i;

  p[ rotation ] = currentNode;

  for( i = 0; i < mapRotations.numRotations; i++ )
    Q_strcat( text, sizeof( text ), va( "%d ", p[ i ] ) );

  trap_Cvar_Set( "g_currentNode", text );
  trap_Cvar_Update( &g_currentNode );
}

/*
===============
G_CurrentNodeIndex

Return the current node index in some rotation
===============
*/
static int G_CurrentNodeIndex( int rotation )
{
  int   *p = G_CurrentNodeIndexArray( );

  return p[ rotation ];
}

/*
===============
G_NodeByIndex

Return a node in a rotation by its index
===============
*/
static node_t *G_NodeByIndex( int index, int rotation )
{
  return mapRotations.rotations[ rotation ].nodes[ index ];
}

/*
===============
G_IssueMapChange

Send commands to the server to actually change the map
===============
*/
static void G_IssueMapChange( int index, int rotation )
{
  node_t        *node = mapRotations.rotations[ rotation ].nodes[ index ];
  destination_t *destination = &node->u.destination;

  // allow a manually defined g_layouts setting to override the maprotation
  if( !g_layouts.string[ 0 ] && destination->layouts[ 0 ] )
  {
    trap_Cvar_Set( "g_layouts", destination->layouts );
  }

  trap_SendConsoleCommand( EXEC_APPEND, va( "map %s\n", destination->name ) );

  // Load up map defaults if g_mapConfigs is set
  G_MapConfigs( destination->name );

  if( strlen( destination->postCommand ) > 0 )
    trap_SendConsoleCommand( EXEC_APPEND, destination->postCommand );
}

/*
===============
G_GotoDestination

Resolve the destination of some condition
===============
*/
static void G_GotoDestination( int currentRotation, char *name )
{
  int i;

  G_Printf( "G_GotoDestination( %s )\n", name );

  // Search the rotation names...
  if( G_StartMapRotation( name, qtrue ) )
    return;

  // ...then try maps in the current rotation
  for( i = 0; i < mapRotations.rotations[ currentRotation ].numNodes; i++ )
  {
    node_t *node = mapRotations.rotations[ currentRotation ].nodes[ i ];

    if( node->type == NT_DESTINATION && !Q_stricmp( node->u.destination.name, name ) )
    {
      G_IssueMapChange( i, currentRotation );
      return;
    }
  }
}

/*
===============
G_EvaluateMapCondition

Evaluate a map condition
===============
*/
static qboolean G_EvaluateMapCondition( condition_t **condition )
{
  qboolean    result = qfalse;
  condition_t *localCondition = *condition;

  switch( localCondition->lhs )
  {
    case CV_RANDOM:
      result = rand( ) & 1;
      break;

    case CV_NUMCLIENTS:
      switch( localCondition->op )
      {
        case CO_LT:
          result = level.numConnectedClients < localCondition->numClients;
          break;

        case CO_GT:
          result = level.numConnectedClients > localCondition->numClients;
          break;

        case CO_EQ:
          result = level.numConnectedClients == localCondition->numClients;
          break;
      }
      break;

    case CV_LASTWIN:
      result = level.lastWin == localCondition->lastWin;
      break;

    default:
    case CV_ERR:
      G_Printf( S_COLOR_RED "ERROR: malformed map switch localCondition\n" );
      break;
  }

  if( localCondition->target->type == NT_CONDITION )
  {
    *condition = &localCondition->target->u.condition;

    return result && G_EvaluateMapCondition( condition );
  }

  return result;
}

/*
===============
G_NodeIndexAfter
===============
*/
static int G_NodeIndexAfter( int currentNode, int rotation )
{
  mapRotation_t *mr = &mapRotations.rotations[ rotation ];

  return ( currentNode + 1 ) % mr->numNodes;
}

/*
===============
G_AdvanceMapRotation

Increment the current map rotation
===============
*/
void G_AdvanceMapRotation( void )
{
  node_t        *node;
  condition_t   *condition;
  int           rotation, nodeIndex;

  if( ( rotation = g_currentMapRotation.integer ) == NOT_ROTATING )
    return;

  nodeIndex = G_CurrentNodeIndex( rotation );
  node = G_NodeByIndex( nodeIndex, rotation );

  while( node->type == NT_CONDITION )
  {
    condition = &node->u.condition;

    if( G_EvaluateMapCondition( &condition ) )
    {
      node = condition->target;
    }
    else
    {
      nodeIndex = G_NodeIndexAfter( nodeIndex, rotation );
      node = G_NodeByIndex( nodeIndex, rotation );
    }
  }

  G_GotoDestination( rotation, node->u.destination.name );
  G_SetCurrentNodeByIndex(
    G_NodeIndexAfter( nodeIndex, rotation ), rotation );
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
      {
        G_IssueMapChange( 0, i );
        G_SetCurrentNodeByIndex( G_NodeIndexAfter(
          G_CurrentNodeIndex( i ), i ), i );
      }
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

Load and initialise the map rotations
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

/*
===============
G_FreeNode

Free up memory used by a node
===============
*/
void G_FreeNode( node_t *node )
{
  if( node->type == NT_CONDITION )
    G_FreeNode( node->u.condition.target );

  BG_Free( node );
}

/*
===============
G_ShutdownMapRotations

Free up memory used by map rotations
===============
*/
void G_ShutdownMapRotations( void )
{
  int i, j;

  for( i = 0; i < mapRotations.numRotations; i++ )
  {
    mapRotation_t *mr = &mapRotations.rotations[ i ];

    for( j = 0; j < mr->numNodes; j++ )
    {
      node_t *node = mr->nodes[ j ];

      G_FreeNode( node );
    }
  }
}
