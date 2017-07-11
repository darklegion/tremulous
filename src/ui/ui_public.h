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

#ifndef UI_PUBLIC_H
#define UI_PUBLIC_H

#define UI_API_VERSION  6

typedef struct
{
  connstate_t   connState;
  int       connectPacketCount;
  int       clientNum;
  char      servername[MAX_STRING_CHARS];
  char      updateInfoString[MAX_STRING_CHARS];
  char      messageString[MAX_STRING_CHARS];
}
uiClientState_t;

typedef enum
{
  UI_ERROR,
  UI_PRINT,
  UI_MILLISECONDS,
  UI_CVAR_SET,
  UI_CVAR_VARIABLEVALUE,
  UI_CVAR_VARIABLESTRINGBUFFER,
  UI_CVAR_SETVALUE,
  UI_CVAR_RESET,
  UI_CVAR_CREATE,
  UI_CVAR_INFOSTRINGBUFFER,
  UI_ARGC,
  UI_ARGV,
  UI_CMD_EXECUTETEXT,
  UI_FS_FOPENFILE,
  UI_FS_READ,
  UI_FS_WRITE,
  UI_FS_FCLOSEFILE,
  UI_FS_GETFILELIST,
  UI_R_REGISTERMODEL,
  UI_R_REGISTERSKIN,
  UI_R_REGISTERSHADERNOMIP,
  UI_R_CLEARSCENE,
  UI_R_ADDREFENTITYTOSCENE,
  UI_R_ADDPOLYTOSCENE,
  UI_R_ADDLIGHTTOSCENE,
  UI_R_RENDERSCENE,
  UI_R_SETCOLOR,
#ifndef MODULE_INTERFACE_11
  UI_R_SETCLIPREGION,
#endif
  UI_R_DRAWSTRETCHPIC,
  UI_UPDATESCREEN,
  UI_CM_LERPTAG,
  UI_CM_LOADMODEL,
  UI_S_REGISTERSOUND,
  UI_S_STARTLOCALSOUND,
  UI_KEY_KEYNUMTOSTRINGBUF,
  UI_KEY_GETBINDINGBUF,
  UI_KEY_SETBINDING,
  UI_KEY_ISDOWN,
  UI_KEY_GETOVERSTRIKEMODE,
  UI_KEY_SETOVERSTRIKEMODE,
  UI_KEY_CLEARSTATES,
  UI_KEY_GETCATCHER,
  UI_KEY_SETCATCHER,
  UI_GETCLIPBOARDDATA,
  UI_GETGLCONFIG,
  UI_GETCLIENTSTATE,
  UI_GETCONFIGSTRING,
  UI_LAN_GETPINGQUEUECOUNT,
  UI_LAN_CLEARPING,
  UI_LAN_GETPING,
  UI_LAN_GETPINGINFO,
  UI_CVAR_REGISTER,
  UI_CVAR_UPDATE,
  UI_MEMORY_REMAINING,
  UI_R_REGISTERFONT,
  UI_R_MODELBOUNDS,
#ifdef MODULE_INTERFACE_11
  UI_PARSE_ADD_GLOBAL_DEFINE,
  UI_PARSE_LOAD_SOURCE,
  UI_PARSE_FREE_SOURCE,
  UI_PARSE_READ_TOKEN,
  UI_PARSE_SOURCE_FILE_AND_LINE,
#endif
  UI_S_STOPBACKGROUNDTRACK,
  UI_S_STARTBACKGROUNDTRACK,
  UI_REAL_TIME,
  UI_LAN_GETSERVERCOUNT,
  UI_LAN_GETSERVERADDRESSSTRING,
  UI_LAN_GETSERVERINFO,
  UI_LAN_MARKSERVERVISIBLE,
  UI_LAN_UPDATEVISIBLEPINGS,
  UI_LAN_RESETPINGS,
  UI_LAN_LOADCACHEDSERVERS,
  UI_LAN_SAVECACHEDSERVERS,
  UI_LAN_ADDSERVER,
  UI_LAN_REMOVESERVER,
  UI_CIN_PLAYCINEMATIC,
  UI_CIN_STOPCINEMATIC,
  UI_CIN_RUNCINEMATIC,
  UI_CIN_DRAWCINEMATIC,
  UI_CIN_SETEXTENTS,
  UI_R_REMAP_SHADER,
  UI_LAN_SERVERSTATUS,
  UI_LAN_GETSERVERPING,
  UI_LAN_SERVERISVISIBLE,
  UI_LAN_COMPARESERVERS,
  // 1.32
  UI_FS_SEEK,
  UI_SET_PBCLSTATUS,

#ifndef MODULE_INTERFACE_11
  UI_PARSE_ADD_GLOBAL_DEFINE,
  UI_PARSE_LOAD_SOURCE,
  UI_PARSE_FREE_SOURCE,
  UI_PARSE_READ_TOKEN,
  UI_PARSE_SOURCE_FILE_AND_LINE,
  UI_GETNEWS,
#endif

  // XXX THERE IS ROOM FOR _1_ (or 2? Did i count from 0?)
  // SYSCALL BETWEEN UI_GETNEWS and UI_MEMSET
  //UI_RESERVED_SYSCALL = 99,

  UI_MEMSET = 100,
  UI_MEMCPY,
  UI_STRNCPY,
  UI_SIN,
  UI_COS,
  UI_ATAN2,
  UI_SQRT,
  UI_FLOOR,
  UI_CEIL,
} uiImport_t;

typedef enum
{
  UIMENU_NONE,
  UIMENU_MAIN,
  UIMENU_INGAME
}
uiMenuCommand_t;

typedef enum
{
  SORT_HOST,
  SORT_GAME,
  SORT_MAP,
  SORT_CLIENTS,
  SORT_PING
}
serverSortField_t;

typedef enum
{
  UI_GETAPIVERSION = 0, // system reserved

  UI_INIT,
  //  void  UI_Init( void );

  UI_SHUTDOWN,
  //  void  UI_Shutdown( void );

  UI_KEY_EVENT,
  //  void  UI_KeyEvent( int key );

  UI_MOUSE_EVENT,
  //  void  UI_MouseEvent( int dx, int dy );

#ifndef MODULE_INTERFACE_11
  UI_MOUSE_POSITION,
  //  int   UI_MousePosition( void );

  UI_SET_MOUSE_POSITION,
  //  void  UI_SetMousePosition( int x, int y );
#endif

  UI_REFRESH,
  //  void  UI_Refresh( int time );

  UI_IS_FULLSCREEN,
  //  qboolean UI_IsFullscreen( void );

  UI_SET_ACTIVE_MENU,
  //  void  UI_SetActiveMenu( uiMenuCommand_t menu );

  UI_CONSOLE_COMMAND,
  //  qboolean UI_ConsoleCommand( int realTime );

  UI_DRAW_CONNECT_SCREEN
  //  void  UI_DrawConnectScreen( qboolean overlay );

  // if !overlay, the background will be drawn, otherwise it will be
  // overlayed over whatever the cgame has drawn.
  // a GetClientState syscall will be made to get the current strings
}
uiExport_t;

#endif
