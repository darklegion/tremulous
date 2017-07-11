/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2013 Darklegion Development

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
// sv_game.c -- interface to the game dll

#include "server.h"

// these functions must be used instead of pointer arithmetic, because
// the game allocates gentities with private information after the server shared part
int	SV_NumForGentity( sharedEntity_t *ent ) {
	int		num;

	num = ( (byte *)ent - (byte *)sv.gentities ) / sv.gentitySize;

	return num;
}

sharedEntity_t *SV_GentityNum( int num ) {
	sharedEntity_t *ent;

	ent = (sharedEntity_t *)((byte *)sv.gentities + sv.gentitySize*(num));

	return ent;
}

playerState_t *SV_GameClientNum( int num ) {
	playerState_t	*ps;

	ps = (playerState_t *)((byte *)sv.gameClients + sv.gameClientSize*(num));

	return ps;
}

svEntity_t	*SV_SvEntityForGentity( sharedEntity_t *gEnt ) {
	if ( !gEnt || gEnt->s.number < 0 || gEnt->s.number >= MAX_GENTITIES ) {
		Com_Error( ERR_DROP, "SV_SvEntityForGentity: bad gEnt" );
	}
	return &sv.svEntities[ gEnt->s.number ];
}

sharedEntity_t *SV_GEntityForSvEntity( svEntity_t *svEnt ) {
	int		num;

	num = svEnt - sv.svEntities;
	return SV_GentityNum( num );
}

/*
===============
SV_GameSendServerCommand

Sends a command string to a client
===============
*/
void SV_GameSendServerCommand( int clientNum, const char *text ) {
	if ( clientNum == -1 ) {
		SV_SendServerCommand( NULL, "%s", text );
	} else {
		if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
			return;
		}
		SV_SendServerCommand( svs.clients + clientNum, "%s", text );	
	}
}


/*
===============
SV_GameDropClient

Disconnects the client with a message
===============
*/
void SV_GameDropClient( int clientNum, const char *reason ) {
	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		return;
	}
	SV_DropClient( svs.clients + clientNum, reason );	
}


/*
=================
SV_SetBrushModel

sets mins and maxs for inline bmodels
=================
*/
void SV_SetBrushModel( sharedEntity_t *ent, const char *name ) {
	clipHandle_t	h;
	vec3_t			mins, maxs;

	if (!name) {
		Com_Error( ERR_DROP, "SV_SetBrushModel: NULL" );
	}

	if (name[0] != '*') {
		Com_Error( ERR_DROP, "SV_SetBrushModel: %s isn't a brush model", name );
	}


	ent->s.modelindex = atoi( name + 1 );

	h = CM_InlineModel( ent->s.modelindex );
	CM_ModelBounds( h, mins, maxs );
	VectorCopy (mins, ent->r.mins);
	VectorCopy (maxs, ent->r.maxs);
	ent->r.bmodel = qtrue;

	ent->r.contents = -1;		// we don't know exactly what is in the brushes

	SV_LinkEntity( ent );		// FIXME: remove
}



/*
=================
SV_inPVS

Also checks portalareas so that doors block sight
=================
*/
bool SV_inPVS (const vec3_t p1, const vec3_t p2)
{
	int		leafnum;
	int		cluster;
	int		area1, area2;
	byte	*mask;

	leafnum = CM_PointLeafnum (p1);
	cluster = CM_LeafCluster (leafnum);
	area1 = CM_LeafArea (leafnum);
	mask = CM_ClusterPVS (cluster);

	leafnum = CM_PointLeafnum (p2);
	cluster = CM_LeafCluster (leafnum);
	area2 = CM_LeafArea (leafnum);

	if ( mask && !(mask[cluster>>3] & (1<<(cluster&7))) )
		return false;

	if (!CM_AreasConnected (area1, area2))
		return false;		// a door blocks sight

	return true;
}


/*
=================
SV_inPVSIgnorePortals

Does NOT check portalareas
=================
*/
bool SV_inPVSIgnorePortals( const vec3_t p1, const vec3_t p2)
{
	int		leafnum;
	int		cluster;
	byte	*mask;

	leafnum = CM_PointLeafnum (p1);
	cluster = CM_LeafCluster (leafnum);
	mask = CM_ClusterPVS (cluster);

	leafnum = CM_PointLeafnum (p2);
	cluster = CM_LeafCluster (leafnum);

	if ( mask && (!(mask[cluster>>3] & (1<<(cluster&7)) ) ) )
		return false;

	return true;
}


/*
========================
SV_AdjustAreaPortalState
========================
*/
void SV_AdjustAreaPortalState( sharedEntity_t *ent, bool open ) {
	svEntity_t	*svEnt;

	svEnt = SV_SvEntityForGentity( ent );
	if ( svEnt->areanum2 == -1 ) {
		return;
	}
	CM_AdjustAreaPortalState( svEnt->areanum, svEnt->areanum2, open );
}


/*
==================
SV_EntityContact
==================
*/
bool SV_EntityContact( vec3_t mins, vec3_t maxs, const sharedEntity_t *gEnt, traceType_t type ) {
	const float	*origin, *angles;
	clipHandle_t	ch;
	trace_t			trace;

	// check for exact collision
	origin = gEnt->r.currentOrigin;
	angles = gEnt->r.currentAngles;

	ch = SV_ClipHandleForEntity( gEnt );
	CM_TransformedBoxTrace ( &trace, vec3_origin, vec3_origin, mins, maxs,
		ch, -1, origin, angles, type );

	return trace.startsolid;
}


/*
===============
SV_GetServerinfo

===============
*/
void SV_GetServerinfo( char *buffer, int bufferSize ) {
	if ( bufferSize < 1 ) {
		Com_Error( ERR_DROP, "SV_GetServerinfo: bufferSize == %i", bufferSize );
	}
	Q_strncpyz( buffer, Cvar_InfoString( CVAR_SERVERINFO ), bufferSize );
}

/*
===============
SV_LocateGameData

===============
*/
void SV_LocateGameData( sharedEntity_t *gEnts, int numGEntities, int sizeofGEntity_t,
					   playerState_t *clients, int sizeofGameClient ) {
	sv.gentities = gEnts;
	sv.gentitySize = sizeofGEntity_t;
	sv.num_entities = numGEntities;

	sv.gameClients = clients;
	sv.gameClientSize = sizeofGameClient;
}


/*
===============
SV_GetUsercmd

===============
*/
void SV_GetUsercmd( int clientNum, usercmd_t *cmd ) {
	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		Com_Error( ERR_DROP, "SV_GetUsercmd: bad clientNum:%i", clientNum );
	}
	*cmd = svs.clients[clientNum].lastUsercmd;
}

//==============================================

static int	FloatAsInt( float f ) {
	floatint_t fi;
	fi.f = f;
	return fi.i;
}

/*
====================
SV_GameSystemCalls

The module is making a system call
====================
*/
intptr_t SV_GameSystemCalls( intptr_t *args ) {
    switch( args[0] )
    {
        case G_PRINT:
            Com_Printf( "%s", (const char*)VMA(1) );
            return 0;
        case G_ERROR:
            Com_Error( ERR_DROP, "%s", (const char*)VMA(1) );
            return 0;
        case G_MILLISECONDS:
            return Sys_Milliseconds();
        case G_CVAR_REGISTER:
            Cvar_Register( (vmCvar_t*)VMA(1), (const char*)VMA(2), (const char*)VMA(3), args[4] ); 
            return 0;
        case G_CVAR_UPDATE:
            Cvar_Update( (vmCvar_t*)VMA(1) );
            return 0;
        case G_CVAR_SET:
            Cvar_SetSafe( (const char *)VMA(1), (const char *)VMA(2) );
            return 0;
        case G_CVAR_VARIABLE_INTEGER_VALUE:
            return Cvar_VariableIntegerValue( (const char *)VMA(1) );
        case G_CVAR_VARIABLE_STRING_BUFFER:
            Cvar_VariableStringBuffer( (const char*)VMA(1), (char*)VMA(2), args[3] );
            return 0;
        case G_ARGC:
            return Cmd_Argc();
        case G_ARGV:
            Cmd_ArgvBuffer( args[1], (char*)VMA(2), args[3] );
            return 0;
        case G_SEND_CONSOLE_COMMAND:
            Cbuf_ExecuteText( args[1], (const char*)VMA(2) );
            return 0;

        case G_FS_FOPEN_FILE:
            return FS_FOpenFileByMode( (const char*)VMA(1), (fileHandle_t*)VMA(2), (FS_Mode)args[3] );
        case G_FS_READ:
            FS_Read( VMA(1), args[2], args[3] );
            return 0;
        case G_FS_WRITE:
            FS_Write( VMA(1), args[2], args[3] );
            return 0;
        case G_FS_FCLOSE_FILE:
            FS_FCloseFile( args[1] );
            return 0;
        case G_FS_GETFILELIST:
            return FS_GetFileList( (const char*)VMA(1), (const char*)VMA(2), (char*)VMA(3), args[4] );
        case G_FS_GETFILTEREDFILES:
            return FS_GetFilteredFiles( (const char*)VMA(1), (const char*)VMA(2), (char*)VMA(3), (char*)VMA(4), args[5] );
        case G_FS_SEEK:
            return FS_Seek( args[1], args[2], (FS_Origin)args[3] );
        case G_LOCATE_GAME_DATA:
            SV_LocateGameData( (sharedEntity_t*)VMA(1), args[2], args[3], (playerState_t*)VMA(4), args[5] );
            return 0;
        case G_DROP_CLIENT:
            SV_GameDropClient( args[1], (const char*)VMA(2) );
            return 0;
        case G_SEND_SERVER_COMMAND:
            SV_GameSendServerCommand( args[1], (const char*)VMA(2) );
            return 0;
        case G_LINKENTITY:
            SV_LinkEntity( (sharedEntity_t*)VMA(1) );
            return 0;
        case G_UNLINKENTITY:
            SV_UnlinkEntity( (sharedEntity_t*)VMA(1) );
            return 0;
        case G_ENTITIES_IN_BOX:
            return SV_AreaEntities( (const vec_t*)VMA(1), (const vec_t*)VMA(2), (int*)VMA(3), args[4] );
        case G_ENTITY_CONTACT:
            return SV_EntityContact( (vec_t*)VMA(1), (vec_t*)VMA(2), (const sharedEntity_t*)VMA(3), TT_AABB );
        case G_ENTITY_CONTACTCAPSULE:
            return SV_EntityContact( (vec_t*)VMA(1), (vec_t*)VMA(2), (const sharedEntity_t*)VMA(3), TT_CAPSULE );
        case G_TRACE:
            SV_Trace( (trace_t*)VMA(1), (const vec_t*)VMA(2), (vec_t*)VMA(3), (vec_t*)VMA(4), (const vec_t*)VMA(5), args[6], args[7], TT_AABB );
            return 0;
        case G_TRACECAPSULE:
            SV_Trace( (trace_t*)VMA(1), (const vec_t*)VMA(2), (vec_t*)VMA(3), (vec_t*)VMA(4), (const vec_t*)VMA(5), args[6], args[7], TT_CAPSULE );
            return 0;
        case G_POINT_CONTENTS:
            return SV_PointContents( (const vec_t*)VMA(1), args[2] );
        case G_SET_BRUSH_MODEL:
            SV_SetBrushModel( (sharedEntity_t*)VMA(1), (const char*)VMA(2) );
            return 0;
        case G_IN_PVS:
            return SV_inPVS( (const vec_t*)VMA(1), (const vec_t*)VMA(2) );
        case G_IN_PVS_IGNORE_PORTALS:
            return SV_inPVSIgnorePortals( (const vec_t*)VMA(1), (const vec_t*)VMA(2) );

        case G_SET_CONFIGSTRING:
            SV_SetConfigstring( args[1], (const char*)VMA(2) );
            return 0;
        case G_GET_CONFIGSTRING:
            SV_GetConfigstring( args[1], (char*)VMA(2), args[3] );
            return 0;
        case G_SET_CONFIGSTRING_RESTRICTIONS:
            SV_SetConfigstringRestrictions( args[1], (clientList_t*)VMA(2) );
            return 0;
        case G_SET_USERINFO:
            SV_SetUserinfo( args[1], (const char*)VMA(2) );
            return 0;
        case G_GET_USERINFO:
            SV_GetUserinfo( args[1], (char*)VMA(2), args[3] );
            return 0;
        case G_GET_SERVERINFO:
            SV_GetServerinfo( (char*)VMA(1), args[2] );
            return 0;
        case G_ADJUST_AREA_PORTAL_STATE:
            SV_AdjustAreaPortalState( (sharedEntity_t*)VMA(1), (bool)args[2] );
            return 0;
        case G_AREAS_CONNECTED:
            return CM_AreasConnected( args[1], args[2] );

        case G_GET_USERCMD:
            SV_GetUsercmd( args[1], (usercmd_t*)VMA(2) );
            return 0;
        case G_GET_ENTITY_TOKEN:
            {
                const char	*s;

                s = COM_Parse( &sv.entityParsePoint );
                Q_strncpyz( (char*)VMA(1), s, args[2] );
                if ( !sv.entityParsePoint && !s[0] ) {
                    return false;
                } else {
                    return true;
                }
            }

        case G_REAL_TIME:
            return Com_RealTime( (qtime_t*)VMA(1) );
        case G_SNAPVECTOR:
            Q_SnapVector( (vec_t*)VMA(1) );
            return 0;

        case G_SEND_GAMESTAT:
            return 0;

	//====================================

        case G_PARSE_ADD_GLOBAL_DEFINE:
            return Parse_AddGlobalDefine( (char*)VMA(1) );
        case G_PARSE_LOAD_SOURCE:
            return Parse_LoadSourceHandle( (const char*)VMA(1) );
        case G_PARSE_FREE_SOURCE:
            return Parse_FreeSourceHandle( args[1] );
        case G_PARSE_READ_TOKEN:
            return Parse_ReadTokenHandle( args[1], (pc_token_t*)VMA(2) );
        case G_PARSE_SOURCE_FILE_AND_LINE:
            return Parse_SourceFileAndLine( args[1], (char*)VMA(2), (int*)VMA(3) );

        case G_ADDCOMMAND:
            Cmd_AddCommand( (const char*)VMA(1), NULL );
            return 0;
        case G_REMOVECOMMAND:
            Cmd_RemoveCommand( (const char*)VMA(1) );
            return 0;

        case TRAP_MEMSET:
            ::memset( VMA(1), args[2], args[3] );
            return 0;

        case TRAP_MEMCPY:
            ::memcpy( VMA(1), VMA(2), args[3] );
            return 0;

        case TRAP_STRNCPY:
            ::strncpy( (char*)VMA(1), (const char*)VMA(2), args[3] );
            return args[1];

        case TRAP_SIN:
            return FloatAsInt( sin( VMF(1) ) );

        case TRAP_COS:
            return FloatAsInt( cos( VMF(1) ) );

        case TRAP_ATAN2:
            return FloatAsInt( atan2( VMF(1), VMF(2) ) );

        case TRAP_SQRT:
            return FloatAsInt( sqrt( VMF(1) ) );

        case TRAP_MATRIXMULTIPLY:
        {
            // XXX C++ is made this annoying
            float (&in1)[3][3] = *reinterpret_cast<float (*)[3][3]>(VMA(1));
            float (&in2)[3][3] = *reinterpret_cast<float (*)[3][3]>(VMA(2));
            float (&in3)[3][3] = *reinterpret_cast<float (*)[3][3]>(VMA(3));
            MatrixMultiply( in1, in2, in3 );
            return 0;
        }

        case TRAP_ANGLEVECTORS:
            AngleVectors( (const vec_t*)VMA(1), (vec_t*)VMA(2), (vec_t*)VMA(3), (vec_t*)VMA(4) );
            return 0;

        case TRAP_PERPENDICULARVECTOR:
            PerpendicularVector( (vec_t*)VMA(1), (const vec_t*)VMA(2) );
            return 0;

        case TRAP_FLOOR:
            return FloatAsInt( floor( VMF(1) ) );

        case TRAP_CEIL:
            return FloatAsInt( ceil( VMF(1) ) );

	    default:
		    Com_Error( ERR_DROP, "Bad game system trap: %ld", (long int) args[0] );
	}
	return 0;
}

/*
===============
SV_ShutdownGameProgs

Called every time a map changes
===============
*/
void SV_ShutdownGameProgs( void ) {
	if ( !gvm ) {
		return;
	}
	VM_Call( gvm, GAME_SHUTDOWN, false );
	VM_Free( gvm );
	gvm = NULL;
}

/*
==================
SV_InitGameVM

Called for both a full init and a restart
==================
*/
static void SV_InitGameVM( bool restart ) {
	int		i;

	// start the entity parsing at the beginning
	sv.entityParsePoint = CM_EntityString();

	// clear all gentity pointers that might still be set from
	// a previous level
	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=522
	//   now done before GAME_INIT call
	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		svs.clients[i].gentity = NULL;
	}
	
	// use the current msec count for a random seed
	// init for this gamestate
	VM_Call (gvm, GAME_INIT, sv.time, Com_Milliseconds(), restart);
}



/*
===================
SV_RestartGameProgs

Called on a map_restart, but not on a normal map change
===================
*/
void SV_RestartGameProgs( void ) {
	if ( !gvm ) {
		return;
	}
	VM_Call( gvm, GAME_SHUTDOWN, true );

	// do a restart instead of a free
	gvm = VM_Restart(gvm, true);
	if ( !gvm ) {
		Com_Error( ERR_FATAL, "VM_Restart on game failed" );
	}

	SV_InitGameVM( true );
}


/*
===============
SV_InitGameProgs

Called on a normal map change, not on a map_restart
===============
*/
void SV_InitGameProgs( void ) {
	// load the dll or bytecode
	gvm = VM_Create( "game", SV_GameSystemCalls, (vmInterpret_t)Cvar_VariableValue( "vm_game" ) );
	if ( !gvm ) {
		Com_Error( ERR_FATAL, "VM_Create on game failed" );
	}

	SV_InitGameVM( false );
}


/*
====================
SV_GameCommand

See if the current console command is claimed by the game
====================
*/
bool SV_GameCommand( void ) {
	if ( sv.state != SS_GAME ) {
		return false;
	}

	return (bool)VM_Call( gvm, GAME_CONSOLE_COMMAND );
}

