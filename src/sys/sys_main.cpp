/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2013 Darklegion Development
Copyright (C) 2015-2019 GrangerHub

This file is part of Tremulous.

Tremulous is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 3 of the License,
or (at your option) any later version.

Tremulous is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tremulous; if not, see <https://www.gnu.org/licenses/>

===========================================================================
*/

#include "sys_local.h"

#include <setjmp.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef WIN32
#include <windows.h>
#endif

#include <cctype>
#include <cerrno>
#include <climits>
#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstring>
#include <iostream>

#include "lua.hpp"
#include "sol.hpp"
#ifndef DEDICATED
#ifdef USE_LOCAL_HEADERS
# include "SDL.h"
# include "SDL_cpuinfo.h"
#else
# include <SDL.h>
# include <SDL_cpuinfo.h>
#endif
#endif

#include "qcommon/files.h"
#include "qcommon/q_shared.h"
#include "qcommon/qcommon.h"
#include "qcommon/vm.h"
#ifndef DEDICATED
#include "script/bind.h"
#include "script/client.h"
#include "script/http_client.h"
#endif
#include "script/cmd.h"
#include "script/cvar.h"
#include "script/rapidjson.h"
#include "script/nettle.h"

#include "dialog.h"
#include "sys_loadlib.h"

sol::state lua;

static char binaryPath[ MAX_OSPATH ] = { 0 };
static char installPath[ MAX_OSPATH ] = { 0 };

/*
=================
Sys_SetBinaryPath
=================
*/
void Sys_SetBinaryPath(const char *path)
{
    Q_strncpyz(binaryPath, path, sizeof(binaryPath));
}

/*
=================
Sys_BinaryPath
=================
*/
char *Sys_BinaryPath(void)
{
    return binaryPath;
}

/*
=================
Sys_SetDefaultInstallPath
=================
*/
void Sys_SetDefaultInstallPath(const char *path)
{
    Q_strncpyz(installPath, path, sizeof(installPath));
}

/*
=================
Sys_DefaultInstallPath
=================
*/
char *Sys_DefaultInstallPath(void)
{
    return installPath;
}

/*
=================
Sys_DefaultAppPath
=================
*/
char *Sys_DefaultAppPath(void)
{
    return Sys_BinaryPath();
}

/*
=================
Sys_In_Restart_f

Restart the input subsystem
=================
*/
void Sys_In_Restart_f( void )
{
    IN_Restart( );
}

/*
=================
Sys_ConsoleInput

Handle new console input
=================
*/
char *Sys_ConsoleInput(void)
{
    return CON_Input( );
}

/*
==================
Sys_GetClipboardData
==================
*/
char *Sys_GetClipboardData(void)
{
    char *data = NULL;
#ifndef DEDICATED
    char *cliptext;

    if ( ( cliptext = SDL_GetClipboardText() ) != NULL ) {
        if ( cliptext[0] != '\0' ) {
            size_t bufsize = strlen( cliptext ) + 1;

            data = (char*)Z_Malloc( bufsize );
            Q_strncpyz( data, cliptext, bufsize );

            // find first listed char and set to '\0'
            strtok( data, "\n\r\b" );
        }
        SDL_free( cliptext );
    }
#endif
    return data;
}

#ifdef DEDICATED
# define PID_FILENAME PRODUCT_NAME "_server.pid"
#else
# define PID_FILENAME PRODUCT_NAME ".pid"
#endif

/*
=================
Sys_PIDFileName
=================
*/
static std::string Sys_PIDFileName( void )
{
    const char *homePath = Cvar_VariableString( "fs_homepath" );
    std::string pidfile;

    if( *homePath != '\0' )
    {
        pidfile += homePath;
        pidfile += "/";
        pidfile += PID_FILENAME;
    }

    return pidfile;
}

/*
=================
Sys_WritePIDFile

Return true if there is an existing stale PID file
=================
*/
bool Sys_WritePIDFile( void )
{
    const char *pidFile = Sys_PIDFileName( ).c_str();
    FILE *f;
    bool  stale = false;

    if( pidFile == NULL )
        return false;

    // First, check if the pid file is already there
    if( ( f = fopen( pidFile, "r" ) ) != NULL )
    {
        char  pidBuffer[ 64 ] = { 0 };
        int   pid;

        pid = fread( pidBuffer, sizeof( char ), sizeof( pidBuffer ) - 1, f );
        fclose( f );

        if(pid > 0)
        {
            pid = atoi( pidBuffer );
            if( !Sys_PIDIsRunning( pid ) )
                stale = true;
        }
        else
            stale = true;
    }

    if( ( f = fopen( pidFile, "w" ) ) != NULL )
    {
        fprintf( f, "%d", Sys_PID( ) );
        fclose( f );
    }
    else
        Com_Printf( S_COLOR_YELLOW "Couldn't write %s.\n", pidFile );

    return stale;
}

/*
=================
Sys_Exit

Single exit point (regular exit or in case of error)
=================
*/
static __attribute__ ((noreturn)) void Sys_Exit( int exitCode )
{
    CON_Shutdown( );

#ifndef DEDICATED
    SDL_Quit( );
#endif

    if( exitCode < 2 )
    {
        // Normal exit
        const char *pidFile = Sys_PIDFileName( ).c_str();
        if( pidFile != NULL )
            remove( pidFile );
    }

    NET_Shutdown( );

    Sys_PlatformExit( );

    exit( exitCode );
}

/*
=================
Sys_Quit
=================
*/
void Sys_Quit( void )
{
    Sys_Exit( 0 );
}

/*
=================
Sys_GetProcessorFeatures
=================
*/
cpuFeatures_t Sys_GetProcessorFeatures( void )
{
    cpuFeatures_t features = CF_NONE;

#ifndef DEDICATED
    if( SDL_HasRDTSC( ) )      features |= CF_RDTSC;
    if( SDL_Has3DNow( ) )      features |= CF_3DNOW;
    if( SDL_HasMMX( ) )        features |= CF_MMX;
    if( SDL_HasSSE( ) )        features |= CF_SSE;
    if( SDL_HasSSE2( ) )       features |= CF_SSE2;
    if( SDL_HasAltiVec( ) )    features |= CF_ALTIVEC;
#endif

    return features;
}

void Sys_Script_f( void )
{
    std::string args = Cmd_Args();
    lua.script(args);
}

void Sys_ScriptFile_f( void )
{
    std::string args = Cmd_Args();
    lua.script_file(args);
}
/*
=================
Sys_Init
=================
*/
void Sys_Init(void)
{
    Cmd_AddCommand( "in_restart", Sys_In_Restart_f );
    Cmd_AddCommand( "script", Sys_Script_f );
    Cmd_AddCommand( "script_file", Sys_ScriptFile_f );
    Cvar_Set( "arch", OS_STRING " " ARCH_STRING );
    Cvar_Set( "username", "UnnamedPlayer" );
}

/*
=================
Sys_AnsiColorPrint
Transform Q3 colour codes to ANSI escape sequences
=================
*/
// FIXME -bbq This could be more extensible
void Sys_AnsiColorPrint( const char *msg )
{
    static char buffer[ MAXPRINTMSG ];
    int         length = 0;
    static int  q3ToAnsi[ 8 ] =
    {
        7, // COLOR_BLACK
        31, // COLOR_RED
        32, // COLOR_GREEN
        33, // COLOR_YELLOW
        34, // COLOR_BLUE
        36, // COLOR_CYAN
        35, // COLOR_MAGENTA
        0   // COLOR_WHITE
    };

    while( *msg )
    {
        if( Q_IsColorString( msg ) || *msg == '\n' )
        {
            // First empty the buffer
            if( length > 0 )
            {
                buffer[ length ] = '\0';
                fputs( buffer, stderr );
                length = 0;
            }

            if( *msg == '\n' )
            {
                // Issue a reset and then the newline
                fputs( "\033[0m\n", stderr );
                msg++;
            }
            else
            {
                // Print the color code (reset first to clear potential inverse (black))
                Com_sprintf( buffer, sizeof( buffer ), "\033[0m\033[%dm",
                        q3ToAnsi[ ColorIndex( *( msg + 1 ) ) ] );
                fputs( buffer, stderr );
                msg += 2;
            }
        }
        else
        {
            if( length >= MAXPRINTMSG - 1 )
                break;

            buffer[ length ] = *msg;
            length++;
            msg++;
        }
    }

    // Empty anything still left in the buffer
    if( length > 0 )
    {
        buffer[ length ] = '\0';
        fputs( buffer, stderr );
    }
}

/*
=================
Sys_Print
=================
*/
void Sys_Print( const char *msg )
{
    CON_LogWrite( msg );
    CON_Print( msg );
}

/*
=================
Sys_Error
=================
*/
void Sys_Error( const char *error, ... )
{
    va_list argptr;
    char    string[1024];

    va_start (argptr,error);
    Q_vsnprintf (string, sizeof(string), error, argptr);
    va_end (argptr);

    Sys_ErrorDialog( string );

    Sys_Exit( 3 );
}

/*
============
Sys_FileTime

returns -1 if not present
============
*/
int Sys_FileTime( char *path )
{
    struct stat buf;

    if (stat (path,&buf) == -1)
        return -1;

    return buf.st_mtime;
}

/*
=================
Sys_UnloadDll
=================
*/
void Sys_UnloadDll( void *dllHandle )
{
    if( !dllHandle )
    {
        Com_Printf("Sys_UnloadDll(NULL)\n");
        return;
    }

    Sys_UnloadLibrary(dllHandle);
}

/*
=================
Sys_LoadDll

First try to load library name from system library path,
from executable path, then fs_basepath.
=================
*/
void *Sys_LoadDll(const char *name, bool useSystemLib)
{
    void *dllhandle;

    if (!Sys_DllExtension(name))
    {
        Com_Printf("Refusing to load library \"%s\": Extension not allowed.\n", name);
        return nullptr;
    }

    if(useSystemLib)
        Com_Printf("Trying to load \"%s\"...\n", name);

    if(!useSystemLib || !(dllhandle = Sys_LoadLibrary(name)))
    {
        const char *topDir;
        char libPath[MAX_OSPATH];

        topDir = Sys_BinaryPath();

        if(!*topDir)
            topDir = ".";

        Com_Printf("Trying to load \"%s\" from \"%s\"...\n", name, topDir);

        int len = Com_sprintf(libPath, sizeof(libPath), "%s%c%s", topDir, PATH_SEP, name);
        if(len < sizeof(libPath))
        {
            Com_Printf("Trying to load \"%s\" from \"%s\"...\n", name, topDir);
            dllhandle = Sys_LoadLibrary(libPath);
        }
        else
        {
            Com_Printf("Skipping trying to load \"%s\" from \"%s\", file name is too long.\n", name, topDir);
        }

        if (!dllhandle)
        {
            const char *basePath = Cvar_VariableString("fs_basepath");

            if(!basePath || !*basePath)
                basePath = ".";

            if(FS_FilenameCompare(topDir, basePath))
            {
                Com_Printf("Trying to load \"%s\" from \"%s\"...\n", name, basePath);
                len = Com_sprintf(libPath, sizeof(libPath), "%s%c%s", basePath, PATH_SEP, name);
                if(len < sizeof(libPath))
                {
                    Com_Printf("Trying to load \"%s\" from \"%s\"...\n", name, basePath);
                    dllhandle = Sys_LoadLibrary(libPath);
                }
                else
                {
                    Com_Printf("Skipping trying to load \"%s\" from \"%s\", file name is too long.\n", name, basePath);
                }
            }

            if(!dllhandle)
                Com_Printf("Loading \"%s\" failed\n", name);
        }
    }

    return dllhandle;
}

/*
=================
Sys_LoadGameDll

Used to load a development dll instead of a virtual machine
=================
*/
using Entry = void (*)(intptr_t (*syscallptr)(intptr_t, ...));
using EntryPoint = intptr_t (QDECL *)(int, ...);
using SysCalls = intptr_t (*)(intptr_t, ...);

void *Sys_LoadGameDll(const char *name, EntryPoint* entryPoint, SysCalls systemcalls)
{
    void *libHandle;

    assert(name);

    if (!Sys_DllExtension(name))
    {
        Com_Printf("Refusing to load library \"%s\": Extension not allowed.\n", name);
        return nullptr;
    }

    Com_Printf( "Loading DLL file: %s\n", name);
    libHandle = Sys_LoadLibrary(name);

    if(!libHandle)
    {
        Com_Printf("Sys_LoadGameDll(%s) failed:\n\"%s\"\n", name, Sys_LibraryError());
        return NULL;
    }

    Entry entry = (Entry)Sys_LoadFunction( libHandle, "dllEntry" );
    *entryPoint = (EntryPoint)Sys_LoadFunction( libHandle, "vmMain" );

    if ( !*entryPoint || !entry )
    {
        Com_Printf ( "Sys_LoadGameDll(%s) failed to find vmMain function:\n\"%s\" !\n", name, Sys_LibraryError( ) );
        Sys_UnloadLibrary(libHandle);
        return NULL;
    }

    Com_Printf ( "Sys_LoadGameDll(%s) found vmMain function at %p\n", name, *entryPoint );
    entry( systemcalls );

    return libHandle;
}

/*
=================
Sys_ParseArgs
=================
*/
void Sys_ParseArgs( int argc, char **argv )
{
    if( argc == 2 )
    {
        if( !strcmp( argv[1], "--version" ) ||
                !strcmp( argv[1], "-v" ) )
        {
            const char* date = __DATE__;
#ifdef DEDICATED
            fprintf( stdout, Q3_VERSION " dedicated server (%s)\n", date );
#else
            fprintf( stdout, Q3_VERSION " client (%s)\n", date );
#endif
            Sys_Exit( 0 );
        }
    }
}

/*
=================
Sys_SigHandler
=================
*/
void Sys_SigHandler( int signal )
{
    static bool signalcaught = false;

    if( signalcaught )
    {
        std::cerr << "DOUBLE SIGNAL FAULT: Received signal "
            << signal << std::endl;
    }
    else
    {
        char const* msg = va("Received signal %d", signal);

        signalcaught = true;
        VM_Forced_Unload_Start();
#ifndef DEDICATED
        CL_Shutdown(va("Received signal %d", signal), true, true);
#endif
        SV_Shutdown(msg);
        VM_Forced_Unload_Done();
    }

    if( signal == SIGTERM || signal == SIGINT )
        Sys_Exit( 1 );

    Sys_Exit( 2 );
}

#ifndef DEFAULT_BASEDIR
# ifdef __APPLE__
#  define DEFAULT_BASEDIR Sys_StripAppBundle(Sys_BinaryPath())
# else
#  define DEFAULT_BASEDIR Sys_BinaryPath()
# endif
#endif

#ifdef __APPLE__
/*
=================
Sys_StripAppBundle

Discovers if passed dir is suffixed with the directory structure of a Mac OS X
.app bundle. If it is, the .app directory structure is stripped off the end and
the result is returned. If not, dir is returned untouched.
=================
*/
const char *Sys_StripAppBundle( const char *dir )
{
	static char cwd[MAX_OSPATH];

	Q_strncpyz(cwd, dir, sizeof(cwd));
	if(strcmp(Sys_Basename(cwd), "MacOS"))
		return dir;
	Q_strncpyz(cwd, Sys_Dirname(cwd), sizeof(cwd));
	if(strcmp(Sys_Basename(cwd), "Contents"))
		return dir;
	Q_strncpyz(cwd, Sys_Dirname(cwd), sizeof(cwd));
	if(!strstr(Sys_Basename(cwd), ".app"))
		return dir;
	Q_strncpyz(cwd, Sys_Dirname(cwd), sizeof(cwd));
	return cwd;
}
#endif

#ifndef DEDICATED

void SDLVersionCheck()
{
#if !SDL_VERSION_ATLEAST(MINSDL_MAJOR,MINSDL_MINOR,MINSDL_PATCH)
#error A more recent version of SDL is required
#endif
    SDL_version ver;
    SDL_GetVersion( &ver );
#define MINSDL_VERSION XSTRING(MINSDL_MAJOR) "." \
    XSTRING(MINSDL_MINOR) "." \
    XSTRING(MINSDL_PATCH)
    if( SDL_VERSIONNUM(ver.major, ver.minor, ver.patch)
            < SDL_VERSIONNUM(MINSDL_MAJOR, MINSDL_MINOR, MINSDL_PATCH) )
    {
        Sys_Dialog( DT_ERROR, va( "SDL version " MINSDL_VERSION " or greater is required, "
                    "but only version %d.%d.%d was found. You may be able to obtain a more recent copy "
                    "from http://www.libsdl.org/.", ver.major, ver.minor, ver.patch ), "SDL Library Too Old" );
        Sys_Exit( 1 );
    }
}
#endif


/*
=================
main
=================
*/
int main( int argc, char **argv )
{
#ifndef DEDICATED
    SDLVersionCheck();
#endif
    Sys_PlatformInit( );

    // Set the initial time base
    Sys_Milliseconds( );

#ifdef __APPLE__
    // This is passed if we are launched by double-clicking
    if ( argc >= 2 )
        if ( Q_strncmp( argv[1], "-psn", 4 ) == 0 )
            argc = 1;
#endif

    Sys_ParseArgs( argc, argv );
    Sys_SetBinaryPath( Sys_Dirname( argv[ 0 ] ) );
    Sys_SetDefaultInstallPath( DEFAULT_BASEDIR );

    // Concatenate the command line for passing to Com_Init
    char args[MAX_STRING_CHARS];
    args[0] = '\0';

    for( int i = 1; i < argc; i++ )
    {
        const bool ws = strchr(argv[i], ' ') ? true : false;

        if (ws) Q_strcat(args, sizeof(args), "\"");
        Q_strcat(args, sizeof(args), argv[i]);
        if (ws) Q_strcat(args, sizeof(args), "\"");
        Q_strcat(args, sizeof(args), " " );
    }

    CON_Init( );
    Com_Init( args );
    NET_Init( );

    lua.open_libraries
    (
     sol::lib::base,
     sol::lib::package,
#if !defined(SOL_LUAJIT) // Not with LuaJIT.
     sol::lib::coroutine,
#endif
     sol::lib::string,
     sol::lib::table,
     sol::lib::math,
     sol::lib::bit32,
     sol::lib::io,
     sol::lib::os,
     sol::lib::debug,
     sol::lib::utf8 // Only with Lua 5.3; ommiting ifdef on purpose. -bbq
#if defined(SOL_LUAJIT) // Only with LuaJIT.
     ,sol::lib::ffi,
     sol::lib::jit
#endif
    );

    script::cvar::init(std::move(lua));
    script::cmd::init(std::move(lua));
    script::rapidjson::init(std::move(lua));
    script::nettle::init(std::move(lua));

#ifndef DEDICATED
    script::client::init(std::move(lua));
    script::keybind::init(std::move(lua));
    script::http_client::init(std::move(lua));
#endif

    for ( ;; )
    {
        try
        { 
            Com_Frame( );
        } 
        catch (sol::error& e)
        {
            Com_Printf(S_COLOR_YELLOW "%s\n", e.what());
        }
    }

    return 0;
}
