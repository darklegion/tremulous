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

// cl_cgame.c  -- client system interaction with client game

#include <setjmp.h>
#include "client.h"
#include "snd_public.h"

#ifdef USE_MUMBLE
#include "libmumblelink.h"
#endif

int cgInterface;

/*
====================
CL_GetGameState
====================
*/
static void CL_GetGameState( gameState_t *gs )
{
	*gs = cl.gameState;
}

/*
====================
CL_GetGlconfig
====================
*/
static void CL_GetGlconfig( glconfig_t *glconfig )
{
	*glconfig = cls.glconfig;
}


/*
====================
CL_GetUserCmd
====================
*/
static bool CL_GetUserCmd( int cmdNumber, usercmd_t *ucmd )
{
	// cmds[cmdNumber] is the last properly generated command

	// can't return anything that we haven't created yet
	if ( cmdNumber > cl.cmdNumber )
		Com_Error( ERR_DROP, "CL_GetUserCmd: %i >= %i", cmdNumber, cl.cmdNumber );

	// the usercmd has been overwritten in the wrapping
	// buffer because it is too far out of date
	if ( cmdNumber <= cl.cmdNumber - CMD_BACKUP )
		return false;

	*ucmd = cl.cmds[ cmdNumber & CMD_MASK ];

	return true;
}

static int CL_GetCurrentCmdNumber( void )
{
	return cl.cmdNumber;
}

/*
====================
CL_GetCurrentSnapshotNumber
====================
*/
static void CL_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime )
{
	*snapshotNumber = cl.snap.messageNum;
	*serverTime = cl.snap.serverTime;
}

/*
====================
CL_GetSnapshot
====================
*/
bool CL_GetSnapshot( int snapshotNumber, snapshot_t *snapshot )
{
	clSnapshot_t *clSnap;

	if ( snapshotNumber > cl.snap.messageNum ) {
		Com_Error( ERR_DROP, "CL_GetSnapshot: snapshotNumber > cl.snapshot.messageNum" );
	}

    // if the frame has fallen out of the circular buffer, we can't return it
	if ( cl.snap.messageNum - snapshotNumber >= PACKET_BACKUP )
		return false;

	// if the frame is not valid, we can't return it
	clSnap = &cl.snapshots[snapshotNumber & PACKET_MASK];
	if ( !clSnap->valid )
		return false;

    // if the entities in the frame have fallen out of their circular buffer,
    // we can't return it
	if ( cl.parseEntitiesNum - clSnap->parseEntitiesNum >= MAX_PARSE_ENTITIES )
		return false;

	// write the snapshot
	snapshot->snapFlags = clSnap->snapFlags;
	snapshot->ping = clSnap->ping;
	snapshot->serverTime = clSnap->serverTime;
	::memcpy( snapshot->areamask, clSnap->areamask, sizeof( snapshot->areamask ) );

	int count = clSnap->numEntities;
	if ( count > MAX_ENTITIES_IN_SNAPSHOT )
    {
		Com_DPrintf( "CL_GetSnapshot: truncated %i entities to %i\n", count, MAX_ENTITIES_IN_SNAPSHOT );
		count = MAX_ENTITIES_IN_SNAPSHOT;
	}

	if ( cgInterface == 2 )
    {
		alternateSnapshot_t *altSnapshot = (alternateSnapshot_t *)snapshot;
		altSnapshot->ps = clSnap->alternatePs;
		altSnapshot->serverCommandSequence = clSnap->serverCommandNum;
		altSnapshot->numEntities = count;

		for ( int i = 0 ; i < count ; i++ )
        {
			entityState_t *es = &cl.parseEntities[ ( clSnap->parseEntitiesNum + i ) & (MAX_PARSE_ENTITIES-1) ];
			::memcpy( &altSnapshot->entities[i], es, (size_t)&((entityState_t *)0)->weaponAnim );
			altSnapshot->entities[i].generic1 = es->generic1;
		}
	}
    else
    {
		snapshot->ps = clSnap->ps;
		snapshot->serverCommandSequence = clSnap->serverCommandNum;
		snapshot->numEntities = count;
		for ( int i = 0 ; i < count ; i++ )
        {
			snapshot->entities[i] = 
				cl.parseEntities[ ( clSnap->parseEntitiesNum + i ) & (MAX_PARSE_ENTITIES-1) ];
		}
	}

	// FIXME: configstring changes and server commands!!!

	return true;
}

/*
=====================
CL_SetUserCmdValue
=====================
*/
void CL_SetUserCmdValue( int userCmdValue, float sensitivityScale ) {
	cl.cgameUserCmdValue = userCmdValue;
	cl.cgameSensitivity = sensitivityScale;
}

/*
=====================
CL_AddCgameCommand
=====================
*/
void CL_AddCgameCommand( const char *cmdName ) {
	Cmd_AddCommand( cmdName, NULL );
}

/*
=====================
CL_ConfigstringModified
=====================
*/
void CL_ConfigstringModified( void )
{
	int idx = atoi( Cmd_Argv(1) );
	if ( idx < 0 || idx >= MAX_CONFIGSTRINGS )
		Com_Error( ERR_DROP, "CL_ConfigstringModified: bad index %i", idx );

	// get everything after "cs <num>"
	const char* s = Cmd_ArgsFrom(2);
	const char* old = cl.gameState.stringData + cl.gameState.stringOffsets[ idx ];
	if ( !strcmp(old, s) )
		return;

	// build the new gameState_t
	gameState_t	oldGs = cl.gameState;

	::memset( &cl.gameState, 0, sizeof( cl.gameState ) );

	// leave the first 0 for uninitialized strings
	cl.gameState.dataCount = 1;
		
    const char* dup;
	for ( int i = 0 ; i < MAX_CONFIGSTRINGS ; i++ )
    {
		if ( i == idx )
			dup = s;
        else
			dup = oldGs.stringData + oldGs.stringOffsets[ i ];

		if ( !dup[0] )
			continue;		// leave with the default empty string

		int len = strlen(dup);
		if ( len + 1 + cl.gameState.dataCount > MAX_GAMESTATE_CHARS )
			Com_Error(ERR_DROP, "MAX_GAMESTATE_CHARS exceeded");

		// append it to the gameState string buffer
		cl.gameState.stringOffsets[ i ] = cl.gameState.dataCount;
		::memcpy( cl.gameState.stringData + cl.gameState.dataCount, dup, len + 1 );
		cl.gameState.dataCount += len + 1;
	}

	if ( idx == CS_SYSTEMINFO )
    {
		// parse serverId and other cvars
		CL_SystemInfoChanged();
	}
}


/*
===================
CL_GetServerCommand

Set up argc/argv for the given command
===================
*/
static bool CL_GetServerCommand( int serverCommandNumber )
{
	const char *s;
	const char *cmd;
	static char bigConfigString[BIG_INFO_STRING];
	int argc;

	// if we have irretrievably lost a reliable command, drop the connection
	if ( serverCommandNumber <= clc.serverCommandSequence - MAX_RELIABLE_COMMANDS )
    {
		// when a demo record was started after the client got a whole bunch of
		// reliable commands then the client never got those first reliable commands
		if ( clc.demoplaying )
			return false;

		Com_Error( ERR_DROP, "CL_GetServerCommand: a reliable command was cycled out" );
		return false;
	}

	if ( serverCommandNumber > clc.serverCommandSequence )
    {
		Com_Error( ERR_DROP, "CL_GetServerCommand: requested a command not received" );
		return false;
	}

	s = clc.serverCommands[ serverCommandNumber & ( MAX_RELIABLE_COMMANDS - 1 ) ];
	clc.lastExecutedServerCommand = serverCommandNumber;

	Com_DPrintf( "serverCommand: %i : %s\n", serverCommandNumber, s );

rescan:
	Cmd_TokenizeString( s );
	cmd = Cmd_Argv(0);
	argc = Cmd_Argc();

	if ( !strcmp( cmd, "disconnect" ) )
    {
		// allow server to indicate why they were disconnected
		if ( argc >= 2 )
			Com_Error( ERR_SERVERDISCONNECT, "Server disconnected - %s", Cmd_Argv( 1 ) );
		else
			Com_Error( ERR_SERVERDISCONNECT, "Server disconnected" );
	}

	if ( !strcmp( cmd, "bcs0" ) )
    {
		Com_sprintf( bigConfigString, BIG_INFO_STRING, "cs %s \"%s", Cmd_Argv(1), Cmd_Argv(2) );
		return false;
	}

	if ( !strcmp( cmd, "bcs1" ) )
    {
		s = Cmd_Argv(2);
		if( strlen(bigConfigString) + strlen(s) >= BIG_INFO_STRING )
			Com_Error( ERR_DROP, "bcs exceeded BIG_INFO_STRING" );

		strcat( bigConfigString, s );
		return false;
	}

	if ( !strcmp( cmd, "bcs2" ) )
    {
		s = Cmd_Argv(2);
		if( strlen(bigConfigString) + strlen(s) + 1 >= BIG_INFO_STRING )
			Com_Error( ERR_DROP, "bcs exceeded BIG_INFO_STRING" );

		strcat( bigConfigString, s );
		strcat( bigConfigString, "\"" );
		s = bigConfigString;
		goto rescan;
	}

	if ( !strcmp( cmd, "cs" ) )
    {
		CL_ConfigstringModified();
        // reparse the string, because CL_ConfigstringModified may have done
        // another Cmd_TokenizeString()
		Cmd_TokenizeString( s );
		return true;
	}

	if ( !strcmp( cmd, "map_restart" ) )
    {
		// clear notify lines and outgoing commands before passing
		// the restart to the cgame
		Con_ClearNotify();
        // reparse the string, because Con_ClearNotify() may have done another
        // Cmd_TokenizeString()
		Cmd_TokenizeString( s );
		::memset( cl.cmds, 0, sizeof( cl.cmds ) );
		return true;
	}

	// we may want to put a "connect to other server" command here

	// cgame can now act on the command
	return true;
}


/*
====================
CL_CM_LoadMap

Just adds default parameters that cgame doesn't need to know about
====================
*/
void CL_CM_LoadMap( const char *mapname ) {
	int		checksum;

	CM_LoadMap( mapname, true, &checksum );
}

char * safe_strncpy(char *dest, const char *src, size_t n)
{
    char *ret = dest;
    while (n > 0 && src[0])
    {
	    *ret++ = *src++;
	    --n;
    }
    while (n > 0)
    {
	    *ret++ = '\0';
    	--n;
    }
    return dest;
}

/*
====================
CL_ShutdonwCGame

====================
*/
void CL_ShutdownCGame( void ) {
	Key_SetCatcher( Key_GetCatcher( ) & ~KEYCATCH_CGAME );
	cls.cgameStarted = false;
	if ( !cgvm ) {
		return;
	}
	VM_Call( cgvm, CG_SHUTDOWN );
	VM_Free( cgvm );
	cgvm = NULL;
}

static int	FloatAsInt( float f ) {
	floatint_t fi;
	fi.f = f;
	return fi.i;
}

static jmp_buf cgProbingJB;
static bool probingCG = false;

/*
====================
CL_CgameSystemCalls

The cgame module is making a system call
====================
*/
intptr_t CL_CgameSystemCalls( intptr_t *args )
{
	if( cgInterface == 2 && args[0] >= CG_R_SETCLIPREGION && args[0] < CG_MEMSET )
    {
		if( args[0] < CG_S_STOPBACKGROUNDTRACK - 1 )
			args[0] += 1;

        else if( args[0] < CG_S_STOPBACKGROUNDTRACK + 4 )
			args[0] += CG_PARSE_ADD_GLOBAL_DEFINE - CG_S_STOPBACKGROUNDTRACK + 1;

        else if( args[0] < CG_PARSE_ADD_GLOBAL_DEFINE + 4 )
			args[0] -= 4;

        else if( args[0] >= CG_PARSE_SOURCE_FILE_AND_LINE && args[0] <= CG_S_SOUNDDURATION )
			args[0] = CG_PARSE_SOURCE_FILE_AND_LINE - 1337 - args[0] ;
	}

	switch( args[0] )
    {
        case CG_PRINT:
            Com_Printf( "%s", (const char*)VMA(1) );
            return 0;
        case CG_ERROR:
            if( probingCG )
            {
                longjmp(cgProbingJB, 1);
            }
            Com_Error( ERR_DROP, "%s", (const char*)VMA(1) );
            return 0;
        case CG_MILLISECONDS:
            return Sys_Milliseconds();
        case CG_CVAR_REGISTER:
            Cvar_Register( (vmCvar_t*)VMA(1), (const char*)VMA(2), (const char*)VMA(3), args[4] ); 
            return 0;
        case CG_CVAR_UPDATE:
            Cvar_Update( (vmCvar_t*)VMA(1) );
            return 0;
        case CG_CVAR_SET:
            Cvar_SetSafe( (const char*)VMA(1), (const char*)VMA(2) );
            return 0;
        case CG_CVAR_VARIABLESTRINGBUFFER:
            Cvar_VariableStringBuffer( (const char*)VMA(1), (char*)VMA(2), args[3] );
            return 0;
        case CG_ARGC:
            return Cmd_Argc();
        case CG_ARGV:
            Cmd_ArgvBuffer( args[1], (char*)VMA(2), args[3] );
            return 0;
        case CG_ARGS:
            Cmd_ArgsBuffer( (char*)VMA(1), args[2] );
            return 0;
        case CG_LITERAL_ARGS:
            Cmd_LiteralArgsBuffer( (char*)VMA(1), args[2] );
            return 0;
        case CG_FS_FOPENFILE:
            return FS_FOpenFileByMode( (const char*)VMA(1), (fileHandle_t*)VMA(2), (FS_Mode)args[3] );
        case CG_FS_READ:
            FS_Read( VMA(1), args[2], args[3] );
            return 0;
        case CG_FS_WRITE:
            FS_Write( VMA(1), args[2], args[3] );
            return 0;
        case CG_FS_FCLOSEFILE:
            FS_FCloseFile( args[1] );
            return 0;
        case CG_FS_SEEK:
            return FS_Seek( (fileHandle_t)args[1], args[2], (FS_Origin)args[3] );
        case CG_FS_GETFILELIST:
            return FS_GetFileList( (const char*)VMA(1), (const char*)VMA(2), (char*)VMA(3), args[4] );
        case CG_SENDCONSOLECOMMAND:
            Cbuf_AddText( (const char*)VMA(1) );
            return 0;
        case CG_ADDCOMMAND:
            CL_AddCgameCommand( (const char*)VMA(1) );
            return 0;
        case CG_REMOVECOMMAND:
            Cmd_RemoveCommandSafe( (const char*)VMA(1) );
            return 0;
        case CG_SENDCLIENTCOMMAND:
            CL_AddReliableCommand((const char*)VMA(1), false);
            return 0;
        case CG_UPDATESCREEN:
            // this is used during lengthy level loading, so pump message loop
            //		Com_EventLoop();	// FIXME: if a server restarts here, BAD THINGS HAPPEN!
            // We can't call Com_EventLoop here, a restart will crash and this _does_ happen
            // if there is a map change while we are downloading at pk3.
            // ZOID
            SCR_UpdateScreen();
            return 0;
        case CG_CM_LOADMAP:
            CL_CM_LoadMap( (const char*)VMA(1) );
            return 0;
        case CG_CM_NUMINLINEMODELS:
            return CM_NumInlineModels();
        case CG_CM_INLINEMODEL:
            return CM_InlineModel( args[1] );
        case CG_CM_TEMPBOXMODEL:
            return CM_TempBoxModel( (const float*)VMA(1), (const float*)VMA(2), false );
        case CG_CM_TEMPCAPSULEMODEL:
            return CM_TempBoxModel( (const float*)VMA(1), (const float*)VMA(2), true );
        case CG_CM_POINTCONTENTS:
            return CM_PointContents( (const float*)VMA(1), args[2] );
        case CG_CM_TRANSFORMEDPOINTCONTENTS:
            return CM_TransformedPointContents( (const float*)VMA(1), (clipHandle_t)args[2],
                                                (const float*)VMA(3), (const float*)VMA(4) );
        case CG_CM_BOXTRACE:
            CM_BoxTrace( (trace_t*)VMA(1), (const float*)VMA(2), (const float*)VMA(3),
                         (float*)VMA(4), (float*)VMA(5), (clipHandle_t)args[6], args[7], TT_AABB );
            return 0;
        case CG_CM_CAPSULETRACE:
            CM_BoxTrace( (trace_t*)VMA(1), (const float*)VMA(2), (const float*)VMA(3),
                         (float*)VMA(4), (float*)VMA(5), (clipHandle_t)args[6], args[7], TT_CAPSULE );
            return 0;
        case CG_CM_TRANSFORMEDBOXTRACE:
            CM_TransformedBoxTrace( (trace_t*)VMA(1), (const float*)VMA(2), (const float*)VMA(3),
                                    (float*)VMA(4), (float*)VMA(5), (clipHandle_t)args[6], args[7],
                                    (const float*)VMA(8), (const float*)VMA(9), TT_AABB );
            return 0;
        case CG_CM_TRANSFORMEDCAPSULETRACE:
            CM_TransformedBoxTrace( (trace_t*)VMA(1), (const float*)VMA(2), (const float*)VMA(3),
                                    (float*)VMA(4), (float*)VMA(5), (clipHandle_t)args[6], args[7],
                                    (const float*)VMA(8), (const float*)VMA(9), TT_CAPSULE );
            return 0;
        case CG_CM_BISPHERETRACE:
            CM_BiSphereTrace( (trace_t*)VMA(1), (const float*)VMA(2), (const float*)VMA(3), VMF(4), VMF(5), (clipHandle_t)args[6], args[7] );
            return 0;
        case CG_CM_TRANSFORMEDBISPHERETRACE:
            CM_TransformedBiSphereTrace( (trace_t*)VMA(1), (const float*)VMA(2), (const float*)VMA(3),
                                         VMF(4), VMF(5), (clipHandle_t)args[6], args[7], (const float*)VMA(8) );
            return 0;
        case CG_CM_MARKFRAGMENTS:
            {
            float (&arg2)[3][3] = *reinterpret_cast<float (*)[3][3]>(VMA(2));
            return re.MarkFragments( args[1], arg2, (const float*)VMA(3), args[4], (float*)VMA(5), args[6], (markFragment_t*)VMA(7) );
            }
        case CG_S_STARTSOUND:
            S_StartSound( (float*)VMA(1), args[2], args[3], (sfxHandle_t)args[4] );
            return 0;
        case CG_S_STARTLOCALSOUND:
            S_StartLocalSound( (sfxHandle_t)args[1], args[2] );
            return 0;
        case CG_S_CLEARLOOPINGSOUNDS:
            S_ClearLoopingSounds( (bool)args[1] );
            return 0;
        case CG_S_ADDLOOPINGSOUND:
            S_AddLoopingSound( args[1], (const float*)VMA(2), (const float*)VMA(3), (sfxHandle_t)args[4] );
            return 0;
        case CG_S_ADDREALLOOPINGSOUND:
            S_AddRealLoopingSound( args[1], (const float*)VMA(2), (const float*)VMA(3), (sfxHandle_t)args[4] );
            return 0;
        case CG_S_STOPLOOPINGSOUND:
            S_StopLoopingSound( args[1] );
            return 0;
        case CG_S_UPDATEENTITYPOSITION:
            S_UpdateEntityPosition( args[1], (const float*)VMA(2) );
            return 0;
        case CG_S_RESPATIALIZE:
            {
            float (&arg3)[3][3] = *reinterpret_cast<float (*)[3][3]>(VMA(3));
            S_Respatialize( args[1], (const float*)VMA(2), arg3, args[4] );
            return 0;
            }
        case CG_S_REGISTERSOUND:
            return S_RegisterSound( (const char*)VMA(1), (bool)args[2] );
        case CG_S_SOUNDDURATION:
            return S_SoundDuration( args[1] );
        case CG_S_STARTBACKGROUNDTRACK:
            S_StartBackgroundTrack( (const char*)VMA(1), (const char*)VMA(2) );
            return 0;
        case CG_R_LOADWORLDMAP:
            re.LoadWorld( (const char*)VMA(1) );
            return 0; 
        case CG_R_REGISTERMODEL:
            return re.RegisterModel( (const char*)VMA(1) );
        case CG_R_REGISTERSKIN:
            return re.RegisterSkin( (const char*)VMA(1) );
        case CG_R_REGISTERSHADER:
            return re.RegisterShader( (const char*)VMA(1) );
        case CG_R_REGISTERSHADERNOMIP:
            return re.RegisterShaderNoMip( (const char*)VMA(1) );
        case CG_R_REGISTERFONT:
            re.RegisterFont( (const char*)VMA(1), args[2], (fontInfo_t*)VMA(3));
            return 0;
        case CG_R_CLEARSCENE:
            re.ClearScene();
            return 0;
        case CG_R_ADDREFENTITYTOSCENE:
            re.AddRefEntityToScene( (const refEntity_t*)VMA(1) );
            return 0;
        case CG_R_ADDPOLYTOSCENE:
            re.AddPolyToScene( (qhandle_t)args[1], args[2], (const polyVert_t*)VMA(3), 1 );
            return 0;
        case CG_R_ADDPOLYSTOSCENE:
            re.AddPolyToScene( (qhandle_t)args[1], args[2], (const polyVert_t*)VMA(3), args[4] );
            return 0;
        case CG_R_LIGHTFORPOINT:
            return re.LightForPoint( (float*)VMA(1), (float*)VMA(2), (float*)VMA(3), (float*)VMA(4) );
        case CG_R_ADDLIGHTTOSCENE:
            re.AddLightToScene( (const float*)VMA(1), VMF(2), VMF(3), VMF(4), VMF(5) );
            return 0;
        case CG_R_ADDADDITIVELIGHTTOSCENE:
            re.AddAdditiveLightToScene( (const float*)VMA(1), VMF(2), VMF(3), VMF(4), VMF(5) );
            return 0;
        case CG_R_RENDERSCENE:
            re.RenderScene( (const refdef_t*)VMA(1) );
            return 0;
        case CG_R_SETCOLOR:
            re.SetColor( (const float*)VMA(1) );
            return 0;
        case CG_R_SETCLIPREGION:
            re.SetClipRegion( (const float*)VMA(1) );
            return 0;
        case CG_R_DRAWSTRETCHPIC:
            re.DrawStretchPic( VMF(1), VMF(2), VMF(3), VMF(4), VMF(5), VMF(6), VMF(7), VMF(8), (qhandle_t)args[9] );
            return 0;
        case CG_R_MODELBOUNDS:
            re.ModelBounds( (qhandle_t)args[1], (float*)VMA(2), (float*)VMA(3) );
            return 0;
        case CG_R_LERPTAG:
            return re.LerpTag( (orientation_t*)VMA(1), args[2], args[3], args[4], VMF(5), (const char*)VMA(6) );
        case CG_GETGLCONFIG:
            CL_GetGlconfig( (glconfig_t*)VMA(1) );
            return 0;
        case CG_GETGAMESTATE:
            CL_GetGameState( (gameState_t*)VMA(1) );
            return 0;
        case CG_GETCURRENTSNAPSHOTNUMBER:
            CL_GetCurrentSnapshotNumber( (int*)VMA(1), (int*)VMA(2) );
            return 0;
        case CG_GETSNAPSHOT:
            return CL_GetSnapshot( args[1], (snapshot_t*)VMA(2) );
        case CG_GETSERVERCOMMAND:
            return CL_GetServerCommand( args[1] );
        case CG_GETCURRENTCMDNUMBER:
            return CL_GetCurrentCmdNumber();
        case CG_GETUSERCMD:
            return CL_GetUserCmd( args[1], (usercmd_t*)VMA(2) );
        case CG_SETUSERCMDVALUE:
            CL_SetUserCmdValue( args[1], VMF(2) );
            return 0;
        case CG_MEMORY_REMAINING:
            return Hunk_MemoryRemaining();
        case CG_KEY_ISDOWN:
            return Key_IsDown( args[1] );
        case CG_KEY_GETCATCHER:
            return Key_GetCatcher();
        case CG_KEY_SETCATCHER:
            // don't allow the cgame module to toggle the console
            Key_SetCatcher( ( args[1] & ~KEYCATCH_CONSOLE ) | ( Key_GetCatcher() & KEYCATCH_CONSOLE ) );
            return 0;
        case CG_KEY_GETKEY:
            return Key_GetKey( (const char*)VMA(1) );

        case CG_GETDEMOSTATE:
            return CL_DemoState( );
        case CG_GETDEMOPOS:
            return CL_DemoPos( );
        case CG_GETDEMONAME:
            CL_DemoName( (char*)VMA(1), args[2] );
            return 0;

        case CG_KEY_KEYNUMTOSTRINGBUF:
            Key_KeynumToStringBuf( args[1], (char*)VMA(2), args[3] );
            return 0;
        case CG_KEY_GETBINDINGBUF:
            Key_GetBindingBuf( args[1], (char*)VMA(2), args[3] );
            return 0;
        case CG_KEY_SETBINDING:
            Key_SetBinding( args[1], (const char*)VMA(2) );
            return 0;

        case CG_PARSE_ADD_GLOBAL_DEFINE:
            return Parse_AddGlobalDefine( (char*)VMA(1) );
        case CG_PARSE_LOAD_SOURCE:
            return Parse_LoadSourceHandle( (const char*)VMA(1) );
        case CG_PARSE_FREE_SOURCE:
            return Parse_FreeSourceHandle( args[1] );
        case CG_PARSE_READ_TOKEN:
            return Parse_ReadTokenHandle( args[1], (pc_token_t*)VMA(2) );
        case CG_PARSE_SOURCE_FILE_AND_LINE:
            return Parse_SourceFileAndLine( args[1], (char*)VMA(2), (int*)VMA(3) );

        case CG_KEY_SETOVERSTRIKEMODE:
            Key_SetOverstrikeMode( (bool)args[1] );
            return 0;

        case CG_KEY_GETOVERSTRIKEMODE:
            return Key_GetOverstrikeMode( );

        case CG_FIELD_COMPLETELIST:
            Field_CompleteList( (char*)VMA(1) );
            return 0;

        case CG_MEMSET:
            ::memset( VMA(1), args[2], args[3] );
            return 0;
        case CG_MEMCPY:
            ::memcpy( VMA(1), VMA(2), args[3] );
            return 0;
        case CG_STRNCPY:
            safe_strncpy( (char*)VMA(1), (const char*)VMA(2), args[3] );
            return args[1];
        case CG_SIN:
            return FloatAsInt( sin( VMF(1) ) );
        case CG_COS:
            return FloatAsInt( cos( VMF(1) ) );
        case CG_ATAN2:
            return FloatAsInt( atan2( VMF(1), VMF(2) ) );
        case CG_SQRT:
            return FloatAsInt( sqrt( VMF(1) ) );
        case CG_FLOOR:
            return FloatAsInt( floor( VMF(1) ) );
        case CG_CEIL:
            return FloatAsInt( ceil( VMF(1) ) );
        case CG_ACOS:
            return FloatAsInt( Q_acos( VMF(1) ) );

        case CG_S_STOPBACKGROUNDTRACK:
            S_StopBackgroundTrack();
            return 0;

        case CG_REAL_TIME:
            return Com_RealTime( (qtime_t*)VMA(1) );
        case CG_SNAPVECTOR:
            Q_SnapVector((float*)VMA(1));
            return 0;

        case CG_CIN_PLAYCINEMATIC:
            return CIN_PlayCinematic((const char*)VMA(1), args[2], args[3], args[4], args[5], args[6]);

        case CG_CIN_STOPCINEMATIC:
            return CIN_StopCinematic(args[1]);

        case CG_CIN_RUNCINEMATIC:
            return CIN_RunCinematic(args[1]);

        case CG_CIN_DRAWCINEMATIC:
            CIN_DrawCinematic(args[1]);
            return 0;

        case CG_CIN_SETEXTENTS:
            CIN_SetExtents(args[1], args[2], args[3], args[4], args[5]);
            return 0;

        case CG_R_REMAP_SHADER:
            re.RemapShader( (const char*)VMA(1), (const char*)VMA(2), (const char*)VMA(3) );
            return 0;

        case CG_GET_ENTITY_TOKEN:
            return re.GetEntityToken( (char*)VMA(1), args[2] );
        case CG_R_INPVS:
            return re.inPVS( (const float*)VMA(1), (const float*)VMA(2) );

        default:
            assert(0);
            Com_Error( ERR_DROP, "Bad cgame system trap: %ld", (long int) args[0] );
    }
	return 0;
}


/*
====================
CL_InitCGame

Should only be called by CL_StartHunkUsers
====================
*/
void CL_InitCGame( void ) {
	const char			*info;
	const char			*mapname;
	int					t1, t2;
	char				backup[ MAX_STRING_CHARS ];
	vmInterpret_t		interpret;

	t1 = Sys_Milliseconds();

	// find the current mapname
	info = cl.gameState.stringData + cl.gameState.stringOffsets[ CS_SERVERINFO ];
	mapname = Info_ValueForKey( info, "mapname" );
	Com_sprintf( cl.mapname, sizeof( cl.mapname ), "maps/%s.bsp", mapname );

	// load the dll or bytecode
	interpret = (vmInterpret_t)Cvar_VariableValue("vm_cgame");
	if(cl_connectedToPureServer)
	{
		// if sv_pure is set we only allow qvms to be loaded
		if(interpret != VMI_COMPILED && interpret != VMI_BYTECODE)
			interpret = VMI_COMPILED;
	}

	cgvm = VM_Create( "cgame", CL_CgameSystemCalls, interpret );
	if ( !cgvm ) {
		Com_Error( ERR_DROP, "VM_Create on cgame failed" );
	}
	clc.state = CA_LOADING;

	Cvar_VariableStringBuffer( "cl_voipSendTarget", backup, sizeof( backup ) );
	Cvar_Set( "cl_voipSendTarget", "" );
	cgInterface = 0;
	probingCG = true;
	if ( setjmp( cgProbingJB ) == 0 ) {
		VM_Call( cgvm, CG_VOIP_STRING );
	} else {
		VM_ClearCallLevel( cgvm );
		cgInterface = 2;
	}
	probingCG = false;
	Cvar_Set( "cl_voipSendTarget", backup );

	if ( ( clc.netchan.alternateProtocol == 2 ) != ( cgInterface == 2 ) ) {
		Com_Error( ERR_DROP, "%s protocol %i, but a cgame module using the %s interface was found",
		           ( clc.demoplaying ? "Demo was recorded using" : "Server uses" ),
		           ( clc.netchan.alternateProtocol == 0 ? PROTOCOL_VERSION : clc.netchan.alternateProtocol == 1 ? 70 : 69 ),
		           ( cgInterface == 2 ? "1.1" : "non-1.1" ) );
	}

	// init for this gamestate
	// use the lastExecutedServerCommand instead of the serverCommandSequence
	// otherwise server commands sent just before a gamestate are dropped
	VM_Call( cgvm, CG_INIT, clc.serverMessageSequence, clc.lastExecutedServerCommand, clc.clientNum );

	// reset any CVAR_CHEAT cvars registered by cgame
	if ( !clc.demoplaying && !cl_connectedToCheatServer )
		Cvar_SetCheatState();

	// we will send a usercmd this frame, which
	// will cause the server to send us the first snapshot
	clc.state = CA_PRIMED;

	t2 = Sys_Milliseconds();

	Com_Printf( "CL_InitCGame: %5.2f seconds\n", (t2-t1)/1000.0 );

	// have the renderer touch all its images, so they are present
	// on the card even if the driver does deferred loading
	re.EndRegistration();

	// make sure everything is paged in
	if (!Sys_LowPhysicalMemory()) {
		Com_TouchMemory();
	}

	// clear anything that got printed
	Con_ClearNotify ();
}


/*
====================
CL_GameCommand

See if the current console command is claimed by the cgame
====================
*/
bool CL_GameCommand( void )
{
	if ( !cgvm )
		return false;

	return (bool)VM_Call( cgvm, CG_CONSOLE_COMMAND );
}

/*
====================
CL_GameConsoleText
====================
*/
void CL_GameConsoleText( void )
{
	if ( !cgvm )
		return;

	VM_Call( cgvm, CG_CONSOLE_TEXT );
}

/*
=====================
CL_CGameRendering
=====================
*/
void CL_CGameRendering( stereoFrame_t stereo )
{
	VM_Call( cgvm, CG_DRAW_ACTIVE_FRAME, cl.serverTime, stereo, clc.demoplaying );
	VM_Debug( 0 );
}


/*
=================
CL_AdjustTimeDelta

Adjust the clients view of server time.

We attempt to have cl.serverTime exactly equal the server's view
of time plus the timeNudge, but with variable latencies over
the internet it will often need to drift a bit to match conditions.

Our ideal time would be to have the adjusted time approach, but not pass,
the very latest snapshot.

Adjustments are only made when a new snapshot arrives with a rational
latency, which keeps the adjustment process framerate independent and
prevents massive overadjustment during times of significant packet loss
or bursted delayed packets.
=================
*/

#define	RESET_TIME	500

void CL_AdjustTimeDelta( void ) {
	int		newDelta;
	int		deltaDelta;

	cl.newSnapshots = false;

	// the delta never drifts when replaying a demo
	if ( clc.demoplaying ) {
		return;
	}

	newDelta = cl.snap.serverTime - cls.realtime;
	deltaDelta = abs( newDelta - cl.serverTimeDelta );

	if ( deltaDelta > RESET_TIME ) {
		cl.serverTimeDelta = newDelta;
		cl.oldServerTime = cl.snap.serverTime;	// FIXME: is this a problem for cgame?
		cl.serverTime = cl.snap.serverTime;
		if ( cl_showTimeDelta->integer ) {
			Com_Printf( "<RESET> " );
		}
	} else if ( deltaDelta > 100 ) {
		// fast adjust, cut the difference in half
		if ( cl_showTimeDelta->integer ) {
			Com_Printf( "<FAST> " );
		}
		cl.serverTimeDelta = ( cl.serverTimeDelta + newDelta ) >> 1;
	} else {
		// slow drift adjust, only move 1 or 2 msec

		// if any of the frames between this and the previous snapshot
		// had to be extrapolated, nudge our sense of time back a little
		// the granularity of +1 / -2 is too high for timescale modified frametimes
		if ( com_timescale->value == 0 || com_timescale->value == 1 ) {
			if ( cl.extrapolatedSnapshot ) {
				cl.extrapolatedSnapshot = false;
				cl.serverTimeDelta -= 2;
			} else {
				// otherwise, move our sense of time forward to minimize total latency
				cl.serverTimeDelta++;
			}
		}
	}

	if ( cl_showTimeDelta->integer ) {
		Com_Printf( "%i ", cl.serverTimeDelta );
	}
}


/*
==================
CL_FirstSnapshot
==================
*/
void CL_FirstSnapshot( void ) {
	// ignore snapshots that don't have entities
	if ( cl.snap.snapFlags & SNAPFLAG_NOT_ACTIVE ) {
		return;
	}
	clc.state = CA_ACTIVE;

	// set the timedelta so we are exactly on this first frame
	cl.serverTimeDelta = cl.snap.serverTime - cls.realtime;
	cl.oldServerTime = cl.snap.serverTime;

	clc.timeDemoBaseTime = cl.snap.serverTime;

	// if this is the first frame of active play,
	// execute the contents of activeAction now
	// this is to allow scripting a timedemo to start right
	// after loading
	if ( cl_activeAction->string[0] ) {
		Cbuf_AddText( cl_activeAction->string );
		Cvar_Set( "activeAction", "" );
	}

#ifdef USE_MUMBLE
	if ((cl_useMumble->integer) && !mumble_islinked()) {
		int ret = mumble_link(CLIENT_WINDOW_TITLE);
		Com_Printf("Mumble: Linking to Mumble application %s\n", ret==0?"ok":"failed");
	}
#endif

#ifdef USE_VOIP
	if (!clc.voipCodecInitialized) {
		int i;
		int error;

		clc.opusEncoder = opus_encoder_create(48000, 1, OPUS_APPLICATION_VOIP, &error);

		if ( error ) {
			Com_DPrintf("VoIP: Error opus_encoder_create %d\n", error);
			return;
		}

		for (i = 0; i < MAX_CLIENTS; i++) {
			clc.opusDecoder[i] = opus_decoder_create(48000, 1, &error);
			if ( error ) {
				Com_DPrintf("VoIP: Error opus_decoder_create(%d) %d\n", i, error);
				return;
			}
			clc.voipIgnore[i] = false;
			clc.voipGain[i] = 1.0f;
		}
		clc.voipCodecInitialized = true;
		clc.voipMuteAll = false;
		Cmd_AddCommand ("voip", CL_Voip_f);
		Cvar_Set("cl_voipSendTarget", "spatial");
		::memset(clc.voipTargets, ~0, sizeof(clc.voipTargets));
	}
#endif
}

/*
==================
CL_SetCGameTime
==================
*/
void CL_SetCGameTime( void ) {
	// getting a valid frame message ends the connection process
	if ( clc.state != CA_ACTIVE ) {
		if ( clc.state != CA_PRIMED ) {
			return;
		}
		if ( clc.demoplaying ) {
			// we shouldn't get the first snapshot on the same frame
			// as the gamestate, because it causes a bad time skip
			if ( !clc.firstDemoFrameSkipped ) {
				clc.firstDemoFrameSkipped = true;
				return;
			}
			CL_ReadDemoMessage();
		}
		if ( cl.newSnapshots ) {
			cl.newSnapshots = false;
			CL_FirstSnapshot();
		}
		if ( clc.state != CA_ACTIVE ) {
			return;
		}
	}	

	// if we have gotten to this point, cl.snap is guaranteed to be valid
	if ( !cl.snap.valid ) {
		Com_Error( ERR_DROP, "CL_SetCGameTime: !cl.snap.valid" );
	}

	// allow pause in single player
	if ( sv_paused->integer && CL_CheckPaused() && com_sv_running->integer ) {
		// paused
		return;
	}

	if ( cl.snap.serverTime < cl.oldFrameServerTime ) {
		Com_Error( ERR_DROP, "cl.snap.serverTime < cl.oldFrameServerTime" );
	}
	cl.oldFrameServerTime = cl.snap.serverTime;


	// get our current view of time

	if ( clc.demoplaying && cl_freezeDemo->integer ) {
		// cl_freezeDemo is used to lock a demo in place for single frame advances

	} else {
		// cl_timeNudge is a user adjustable cvar that allows more
		// or less latency to be added in the interest of better 
		// smoothness or better responsiveness.
		int tn;
		
		tn = cl_timeNudge->integer;
		if (tn<-30) {
			tn = -30;
		} else if (tn>30) {
			tn = 30;
		}

		cl.serverTime = cls.realtime + cl.serverTimeDelta - tn;

		// guarantee that time will never flow backwards, even if
		// serverTimeDelta made an adjustment or cl_timeNudge was changed
		if ( cl.serverTime < cl.oldServerTime ) {
			cl.serverTime = cl.oldServerTime;
		}
		cl.oldServerTime = cl.serverTime;

		// note if we are almost past the latest frame (without timeNudge),
		// so we will try and adjust back a bit when the next snapshot arrives
		if ( cls.realtime + cl.serverTimeDelta >= cl.snap.serverTime - 5 ) {
			cl.extrapolatedSnapshot = true;
		}
	}

	// if we have gotten new snapshots, drift serverTimeDelta
	// don't do this every frame, or a period of packet loss would
	// make a huge adjustment
	if ( cl.newSnapshots ) {
		CL_AdjustTimeDelta();
	}

	if ( !clc.demoplaying ) {
		return;
	}

	// if we are playing a demo back, we can just keep reading
	// messages from the demo file until the cgame definately
	// has valid snapshots to interpolate between

	// a timedemo will always use a deterministic set of time samples
	// no matter what speed machine it is run on,
	// while a normal demo may have different time samples
	// each time it is played back
	if ( cl_timedemo->integer ) {
		int now = Sys_Milliseconds( );
		int frameDuration;

		if (!clc.timeDemoStart) {
			clc.timeDemoStart = clc.timeDemoLastFrame = now;
			clc.timeDemoMinDuration = INT_MAX;
			clc.timeDemoMaxDuration = 0;
		}

		frameDuration = now - clc.timeDemoLastFrame;
		clc.timeDemoLastFrame = now;

		// Ignore the first measurement as it'll always be 0
		if( clc.timeDemoFrames > 0 )
		{
			if( frameDuration > clc.timeDemoMaxDuration )
				clc.timeDemoMaxDuration = frameDuration;

			if( frameDuration < clc.timeDemoMinDuration )
				clc.timeDemoMinDuration = frameDuration;

			// 255 ms = about 4fps
			if( frameDuration > UCHAR_MAX )
				frameDuration = UCHAR_MAX;

			clc.timeDemoDurations[ ( clc.timeDemoFrames - 1 ) %
				MAX_TIMEDEMO_DURATIONS ] = frameDuration;
		}

		clc.timeDemoFrames++;
		cl.serverTime = clc.timeDemoBaseTime + clc.timeDemoFrames * 50;
	}

	while ( cl.serverTime >= cl.snap.serverTime ) {
		// feed another messag, which should change
		// the contents of cl.snap
		CL_ReadDemoMessage();
		if ( clc.state != CA_ACTIVE ) {
			return;		// end of demo
		}
	}

}



