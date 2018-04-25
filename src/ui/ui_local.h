/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2013 Darklegion Development
Copyright (C) 2015-2018 GrangerHub

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

#include "client/keycodes.h"
#include "game/bg_public.h"
#include "qcommon/cmd.h"
#include "qcommon/cvar.h"
#include "qcommon/files.h"
#include "qcommon/q_shared.h"
#include "renderercommon/tr_public.h"
#include "renderercommon/tr_types.h"
#include "sys/sys_shared.h"

#include "ui_public.h"
#include "ui_shared.h"

//
// ui_main.c
//
void UI_Report(void);
void UI_Load(void);
void UI_LoadMenus(const char *menuFile, bool reset);
int UI_AdjustTimeByGame(int time);
void UI_ClearScores(void);
void UI_LoadArenas(void);
void UI_ServerInfo(void);
void UI_UpdateNews(bool);
void UI_UpdateGithubRelease(void);

void UI_RegisterCvars(void);
void UI_UpdateCvars(void);
void UI_DrawConnectScreen(void);

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

struct mapInfo {
    const char *mapName;
    const char *mapLoadName;
    const char *imageName;
    int cinematic;
    qhandle_t levelShot;
};

struct serverFilter_t {
    const char *description;
    const char *basedir;
};

struct pinglist_t {
    char adrstr[MAX_ADDRESSLENGTH];
    int start;
};

struct serverStatus_t {
    pinglist_t pingList[MAX_PINGREQUESTS];
    int numqueriedservers;
    int currentping;
    int nextpingtime;
    int maxservers;
    int refreshtime;
    int numServers;
    int sortKey;
    int sortDir;
    bool sorted;
    int lastCount;
    bool refreshActive;
    int currentServer;
    int displayServers[MAX_DISPLAY_SERVERS];
    int numDisplayServers;
    int numPlayersOnServers;
    int nextDisplayRefresh;
    int nextSortTime;
    qhandle_t currentServerPreview;
    int currentServerCinematic;
    int motdLen;
    int motdWidth;
    int motdPaintX;
    int motdPaintX2;
    int motdOffset;
    int motdTime;
    char motd[MAX_STRING_CHARS];
};

struct pendingServer_t {
    char adrstr[MAX_ADDRESSLENGTH];
    char name[MAX_ADDRESSLENGTH];
    int startTime;
    int serverNum;
    bool valid;
};

struct pendingServerStatus_t {
    int num;
    pendingServer_t server[MAX_SERVERSTATUSREQUESTS];
};

struct serverStatusInfo_t {
    char address[MAX_ADDRESSLENGTH];
    const char *lines[MAX_SERVERSTATUS_LINES][4];
    char text[MAX_SERVERSTATUS_TEXT];
    char pings[MAX_CLIENTS * 3];
    int numLines;
};

struct newsInfo_t {
    char text[MAX_NEWS_LINES][MAX_NEWS_LINEWIDTH];
    int numLines;
    bool refreshActive;
    int refreshtime;
};

struct githubRelease_t {
    char text[MAX_NEWS_LINES][MAX_NEWS_LINEWIDTH];
    int numLines;
    bool refreshActive;
    int nextTime;
};

struct modInfo_t {
    const char *modName;
    const char *modDescr;
};

enum infoType_t {
    INFOTYPE_TEXT,
    INFOTYPE_BUILDABLE,
    INFOTYPE_CLASS,
    INFOTYPE_WEAPON,
    INFOTYPE_UPGRADE,
    INFOTYPE_VOICECMD
};

struct menuItem_t {
    const char *text;
    const char *cmd;
    infoType_t type;
    union {
        const char *text;
        buildable_t buildable;
        class_t pclass;
        weapon_t weapon;
        upgrade_t upgrade;
    } v;
};

struct resolution_t {
    int w;
    int h;
};

struct uiInfo_t {
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

    menuItem_t teamList[4];
    int teamCount;
    int teamIndex;

    menuItem_t alienClassList[3];
    int alienClassCount;
    int alienClassIndex;

    menuItem_t humanItemList[3];
    int humanItemCount;
    int humanItemIndex;

    menuItem_t humanArmouryBuyList[32];
    int humanArmouryBuyCount;
    int humanArmouryBuyIndex;

    menuItem_t humanArmourySellList[32];
    int humanArmourySellCount;
    int humanArmourySellIndex;

    menuItem_t alienUpgradeList[16];
    int alienUpgradeCount;
    int alienUpgradeIndex;

    menuItem_t alienBuildList[32];
    int alienBuildCount;
    int alienBuildIndex;

    menuItem_t humanBuildList[32];
    int humanBuildCount;
    int humanBuildIndex;

    menuItem_t voiceCmdList[32];
    int voiceCmdCount;
    int voiceCmdIndex;

    menuItem_t helpList[MAX_HELP_INFOPANES];
    int helpCount;
    int helpIndex;

    int weapons;
    int upgrades;

    serverStatus_t serverStatus;

    // for showing the game news window
    newsInfo_t newsInfo;

    githubRelease_t githubRelease;

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

    resolution_t resolutions[MAX_RESOLUTIONS];
    int numResolutions;
    int resolutionIndex;

    voice_t *voices;

    bool inGameLoad;

    bool chatTeam;
    bool voiceCmd;
};

extern uiInfo_t uiInfo;
extern refexport_t* re;


char *UI_Cvar_VariableString(const char *var_name);
bool UI_ConsoleCommand(int realTime);
void UI_SetColor(const float *rgba);
void UI_AdjustFrom640(float *x, float *y, float *w, float *h);
void UI_Refresh(int time);
void UI_DrawHandlePic(float x, float y, float w, float h, qhandle_t hShader);
void UI_FillRect(float x, float y, float width, float height, const float *color);

#endif
