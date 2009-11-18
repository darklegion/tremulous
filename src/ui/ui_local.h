/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2009 Darklegion Development
 
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

#ifndef UI_LOCAL_H
#define UI_LOCAL_H

#include "../qcommon/q_shared.h"
#include "../renderer/tr_types.h"
#include "ui_public.h"
#include "../client/keycodes.h"
#include "../game/bg_public.h"
#include "ui_shared.h"

//
// ui_main.c
//
void UI_Report( void );
void UI_Load( void );
void UI_LoadMenus( const char *menuFile, qboolean reset );
int UI_AdjustTimeByGame( int time );
void UI_ClearScores( void );
void UI_LoadArenas( void );
void UI_ServerInfo( void );
void UI_UpdateNews( qboolean );

void UI_RegisterCvars( void );
void UI_UpdateCvars( void );
void UI_DrawConnectScreen( qboolean overlay );

// new ui stuff
#define MAX_MAPS 128
#define MAX_PINGREQUESTS 32
#define MAX_ADDRESSLENGTH 64
#define MAX_DISPLAY_SERVERS 2048
#define MAX_SERVERSTATUS_LINES 128
#define MAX_SERVERSTATUS_TEXT 1024
#define MAX_NEWS_LINES 50
#define MAX_NEWS_LINEWIDTH 85
#define MAX_FOUNDPLAYER_SERVERS 16
#define MAX_MODS 64
#define MAX_DEMOS 256
#define MAX_MOVIES 256
#define MAX_HELP_INFOPANES 32
#define MAX_RESOLUTIONS 32

typedef struct
{
  const char *mapName;
  const char *mapLoadName;
  const char *imageName;
  int cinematic;
  qhandle_t levelShot;
}
mapInfo;

typedef struct serverFilter_s
{
  const char *description;
  const char *basedir;
}
serverFilter_t;

typedef struct
{
  char  adrstr[MAX_ADDRESSLENGTH];
  int    start;
}
pinglist_t;


typedef struct serverStatus_s
{
  pinglist_t pingList[MAX_PINGREQUESTS];
  int    numqueriedservers;
  int    currentping;
  int    nextpingtime;
  int    maxservers;
  int    refreshtime;
  int    numServers;
  int    sortKey;
  int    sortDir;
  qboolean sorted;
  int    lastCount;
  qboolean refreshActive;
  int    currentServer;
  int    displayServers[MAX_DISPLAY_SERVERS];
  int    numDisplayServers;
  int    numPlayersOnServers;
  int    nextDisplayRefresh;
  int    nextSortTime;
  qhandle_t currentServerPreview;
  int    currentServerCinematic;
  int    motdLen;
  int    motdWidth;
  int    motdPaintX;
  int    motdPaintX2;
  int    motdOffset;
  int    motdTime;
  char  motd[MAX_STRING_CHARS];
}
serverStatus_t;


typedef struct
{
  char    adrstr[MAX_ADDRESSLENGTH];
  char    name[MAX_ADDRESSLENGTH];
  int      startTime;
  int      serverNum;
  qboolean  valid;
}
pendingServer_t;

typedef struct
{
  int num;
  pendingServer_t server[MAX_SERVERSTATUSREQUESTS];
}
pendingServerStatus_t;

typedef struct
{
  char address[MAX_ADDRESSLENGTH];
  char *lines[MAX_SERVERSTATUS_LINES][4];
  char text[MAX_SERVERSTATUS_TEXT];
  char pings[MAX_CLIENTS * 3];
  int numLines;
}
serverStatusInfo_t;

typedef struct
{
  char text[MAX_NEWS_LINES][MAX_NEWS_LINEWIDTH];
  int numLines;
  qboolean refreshActive;
  int refreshtime;
}
newsInfo_t;

typedef struct
{
  const char *modName;
  const char *modDescr;
}
modInfo_t;

typedef enum
{
  INFOTYPE_TEXT,
  INFOTYPE_BUILDABLE,
  INFOTYPE_CLASS,
  INFOTYPE_WEAPON,
  INFOTYPE_UPGRADE
} infoType_t;

typedef struct
{
  const char    *text;
  const char    *cmd;
  infoType_t    type;
  union
  {
    const char  *text;
    buildable_t buildable;
    class_t     pclass;
    weapon_t    weapon;
    upgrade_t   upgrade;
  } v;
}
menuItem_t;

typedef struct
{
  int w;
  int h;
}
resolution_t;

typedef struct
{
  displayContextDef_t uiDC;

  int playerCount;
  int myTeamCount;
  int teamPlayerIndex;
  int playerRefresh;
  int playerIndex;
  int playerNumber;
  int myPlayerIndex;
  int ignoreIndex;
  char playerNames[MAX_CLIENTS][MAX_NAME_LENGTH];
  char rawPlayerNames[MAX_CLIENTS][MAX_NAME_LENGTH];
  char teamNames[MAX_CLIENTS][MAX_NAME_LENGTH];
  char rawTeamNames[MAX_CLIENTS][MAX_NAME_LENGTH];
  int clientNums[MAX_CLIENTS];
  int teamClientNums[MAX_CLIENTS];
  clientList_t ignoreList[MAX_CLIENTS];

  int mapCount;
  mapInfo mapList[MAX_MAPS];

  modInfo_t modList[MAX_MODS];
  int modCount;
  int modIndex;

  const char *demoList[MAX_DEMOS];
  int demoCount;
  int demoIndex;

  const char *movieList[MAX_MOVIES];
  int movieCount;
  int movieIndex;
  int previewMovie;

  menuItem_t  teamList[ 4 ];
  int         teamCount;
  int         teamIndex;

  menuItem_t  alienClassList[ 3 ];
  int         alienClassCount;
  int         alienClassIndex;

  menuItem_t  humanItemList[ 3 ];
  int         humanItemCount;
  int         humanItemIndex;

  menuItem_t  humanArmouryBuyList[ 32 ];
  int         humanArmouryBuyCount;
  int         humanArmouryBuyIndex;

  menuItem_t  humanArmourySellList[ 32 ];
  int         humanArmourySellCount;
  int         humanArmourySellIndex;

  menuItem_t  alienUpgradeList[ 16 ];
  int         alienUpgradeCount;
  int         alienUpgradeIndex;

  menuItem_t  alienBuildList[ 32 ];
  int         alienBuildCount;
  int         alienBuildIndex;

  menuItem_t  humanBuildList[ 32 ];
  int         humanBuildCount;
  int         humanBuildIndex;

  menuItem_t  helpList[ MAX_HELP_INFOPANES ];
  int         helpCount;
  int         helpIndex;

  int         weapons;
  int         upgrades;

  serverStatus_t serverStatus;

  // for showing the game news window
  newsInfo_t newsInfo;

  // for the showing the status of a server
  char serverStatusAddress[MAX_ADDRESSLENGTH];
  serverStatusInfo_t serverStatusInfo;
  int nextServerStatusRefresh;

  // to retrieve the status of server to find a player
  pendingServerStatus_t pendingServerStatus;
  char findPlayerName[MAX_STRING_CHARS];
  char foundPlayerServerAddresses[MAX_FOUNDPLAYER_SERVERS][MAX_ADDRESSLENGTH];
  char foundPlayerServerNames[MAX_FOUNDPLAYER_SERVERS][MAX_ADDRESSLENGTH];
  int currentFoundPlayerServer;
  int numFoundPlayerServers;
  int nextFindPlayerRefresh;

  resolution_t  resolutions[ MAX_RESOLUTIONS ];
  int           numResolutions;
  int           resolutionIndex;

  qboolean inGameLoad;

  qboolean  chatTeam;
}
uiInfo_t;

extern uiInfo_t uiInfo;


qboolean UI_ConsoleCommand( int realTime );
char      *UI_Cvar_VariableString( const char *var_name );
void      UI_SetColor( const float *rgba );
void      UI_AdjustFrom640( float *x, float *y, float *w, float *h );
void      UI_Refresh( int time );
void      UI_DrawHandlePic( float x, float y, float w, float h, qhandle_t hShader );
void      UI_FillRect( float x, float y, float width, float height, const float *color );

//
// ui_syscalls.c
//
void      trap_Print( const char *string );
void      trap_Error( const char *string );
int       trap_Milliseconds( void );
void      trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
void      trap_Cvar_Update( vmCvar_t *vmCvar );
void      trap_Cvar_Set( const char *var_name, const char *value );
float     trap_Cvar_VariableValue( const char *var_name );
void      trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void      trap_Cvar_SetValue( const char *var_name, float value );
void      trap_Cvar_Reset( const char *name );
void      trap_Cvar_Create( const char *var_name, const char *var_value, int flags );
void      trap_Cvar_InfoStringBuffer( int bit, char *buffer, int bufsize );
int       trap_Argc( void );
void      trap_Argv( int n, char *buffer, int bufferLength );
void      trap_Cmd_ExecuteText( int exec_when, const char *text );  // don't use EXEC_NOW!
int       trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void      trap_FS_Read( void *buffer, int len, fileHandle_t f );
void      trap_FS_Write( const void *buffer, int len, fileHandle_t f );
void      trap_FS_FCloseFile( fileHandle_t f );
int       trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize );
int       trap_FS_Seek( fileHandle_t f, long offset, int origin ); // fsOrigin_t
qhandle_t trap_R_RegisterModel( const char *name );
qhandle_t trap_R_RegisterSkin( const char *name );
qhandle_t trap_R_RegisterShaderNoMip( const char *name );
void      trap_R_ClearScene( void );
void      trap_R_AddRefEntityToScene( const refEntity_t *re );
void      trap_R_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts );
void      trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b );
void      trap_R_RenderScene( const refdef_t *fd );
void      trap_R_SetColor( const float *rgba );
void      trap_R_SetClipRegion( const float *region );
void      trap_R_DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader );
void      trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs );
void      trap_UpdateScreen( void );
int       trap_CM_LerpTag( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, float frac, const char *tagName );
void      trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum );
sfxHandle_t    trap_S_RegisterSound( const char *sample, qboolean compressed );
void      trap_Key_KeynumToStringBuf( int keynum, char *buf, int buflen );
void      trap_Key_GetBindingBuf( int keynum, char *buf, int buflen );
void      trap_Key_SetBinding( int keynum, const char *binding );
qboolean  trap_Key_IsDown( int keynum );
qboolean  trap_Key_GetOverstrikeMode( void );
void      trap_Key_SetOverstrikeMode( qboolean state );
void      trap_Key_ClearStates( void );
int       trap_Key_GetCatcher( void );
void      trap_Key_SetCatcher( int catcher );
void      trap_GetClipboardData( char *buf, int bufsize );
void      trap_GetClientState( uiClientState_t *state );
void      trap_GetGlconfig( glconfig_t *glconfig );
int       trap_GetConfigString( int index, char* buff, int buffsize );
int       trap_LAN_GetServerCount( int source );
void      trap_LAN_GetServerAddressString( int source, int n, char *buf, int buflen );
void      trap_LAN_GetServerInfo( int source, int n, char *buf, int buflen );
int       trap_LAN_GetServerPing( int source, int n );
int       trap_LAN_GetPingQueueCount( void );
void      trap_LAN_ClearPing( int n );
void      trap_LAN_GetPing( int n, char *buf, int buflen, int *pingtime );
void      trap_LAN_GetPingInfo( int n, char *buf, int buflen );
void      trap_LAN_LoadCachedServers( void );
void      trap_LAN_SaveCachedServers( void );
void      trap_LAN_MarkServerVisible( int source, int n, qboolean visible );
int       trap_LAN_ServerIsVisible( int source, int n );
qboolean  trap_LAN_UpdateVisiblePings( int source );
int       trap_LAN_AddServer( int source, const char *name, const char *addr );
void      trap_LAN_RemoveServer( int source, const char *addr );
void      trap_LAN_ResetPings( int n );
int       trap_LAN_ServerStatus( const char *serverAddress, char *serverStatus, int maxLen );
qboolean  trap_GetNews( qboolean force );
int       trap_LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 );
int       trap_MemoryRemaining( void );
void      trap_R_RegisterFont( const char *pFontname, int pointSize, fontInfo_t *font );
void      trap_S_StopBackgroundTrack( void );
void      trap_S_StartBackgroundTrack( const char *intro, const char *loop );
int       trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits );
e_status  trap_CIN_StopCinematic( int handle );
e_status  trap_CIN_RunCinematic ( int handle );
void      trap_CIN_DrawCinematic ( int handle );
void      trap_CIN_SetExtents ( int handle, int x, int y, int w, int h );
int       trap_RealTime( qtime_t *qtime );
void      trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset );

void      trap_SetPbClStatus( int status );

#endif
