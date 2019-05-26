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

#include "client.h"

#include "cl_updates.h"

/*
====================
GetClientState
====================
*/
static void GetClientState(uiClientState_t *state)
{
    state->connectPacketCount = clc.connectPacketCount;
    state->connState = clc.state;
    Q_strncpyz(state->servername, clc.servername, sizeof(state->servername));
    Q_strncpyz(state->updateInfoString, cls.updateInfoString, sizeof(state->updateInfoString));
    Q_strncpyz(state->messageString, clc.serverMessage, sizeof(state->messageString));
    state->clientNum = (clc.netchan.alternateProtocol == 2 ? cl.snap.alternatePs.clientNum : cl.snap.ps.clientNum);
}

/*
====================
LAN_LoadCachedServers
====================
*/
void LAN_LoadCachedServers(void)
{
    int size;
    fileHandle_t fileIn;
    cls.numglobalservers = cls.numfavoriteservers = 0;
    cls.numGlobalServerAddresses = 0;
    if (FS_SV_FOpenFileRead("servercache.dat", &fileIn))
    {
        FS_Read(&cls.numglobalservers, sizeof(int), fileIn);
        FS_Read(&cls.numfavoriteservers, sizeof(int), fileIn);
        FS_Read(&size, sizeof(int), fileIn);
        if (size == sizeof(cls.globalServers) + sizeof(cls.favoriteServers))
        {
            FS_Read(&cls.globalServers, sizeof(cls.globalServers), fileIn);
            FS_Read(&cls.favoriteServers, sizeof(cls.favoriteServers), fileIn);
        }
        else
        {
            cls.numglobalservers = cls.numfavoriteservers = 0;
            cls.numGlobalServerAddresses = 0;
        }
        FS_FCloseFile(fileIn);
    }
}

/*
====================
LAN_SaveServersToCache
====================
*/
void LAN_SaveServersToCache(void)
{
    fileHandle_t fileOut = FS_SV_FOpenFileWrite("servercache.dat");
    FS_Write(&cls.numglobalservers, sizeof(int), fileOut);
    FS_Write(&cls.numfavoriteservers, sizeof(int), fileOut);

    int size = sizeof(cls.globalServers) + sizeof(cls.favoriteServers);
    FS_Write(&size, sizeof(int), fileOut);
    FS_Write(&cls.globalServers, sizeof(cls.globalServers), fileOut);
    FS_Write(&cls.favoriteServers, sizeof(cls.favoriteServers), fileOut);
    FS_FCloseFile(fileOut);
}

/*
====================
GetNews
====================
*/
static bool GetNews(bool begin)
{
    bool finished = false;
    fileHandle_t fileIn;
    int readSize;

    if (begin)
    {  // if not already using curl, start the download
        if (!clc.downloadCURLM)
        {
            if (!CL_cURL_Init())
            {
                Cvar_Set("cl_newsString", "^1Error: Could not load cURL library");
                return true;
            }
            clc.activeCURLNotGameRelated = true;
            CL_cURL_BeginDownload("news.dat", "http://grangerhub.com/wp-content/uploads/clientnews.txt");
            return false;
        }
    }

    if (!clc.downloadCURLM && FS_SV_FOpenFileRead("news.dat", &fileIn))
    {
        readSize = FS_Read(clc.newsString, sizeof(clc.newsString), fileIn);
        FS_FCloseFile(fileIn);
        clc.newsString[readSize] = '\0';
        if (readSize > 0)
        {
            finished = true;
            clc.cURLUsed = false;
            CL_cURL_Shutdown();
            clc.activeCURLNotGameRelated = false;
        }
    }
    if (!finished) strcpy(clc.newsString, "Retrieving...");
    Cvar_Set("cl_newsString", clc.newsString);
    return finished;
    Cvar_Set("cl_newsString", "^1You must compile your client with CURL support to use this feature");
    return true;
}

/*
====================
LAN_ResetPings
====================
*/
static void LAN_ResetPings(int source)
{
    int count, i;
    serverInfo_t *servers = NULL;
    count = 0;

    switch (source)
    {
        case AS_LOCAL:
            servers = &cls.localServers[0];
            count = MAX_OTHER_SERVERS;
            break;
        case AS_MPLAYER:
        case AS_GLOBAL:
            servers = &cls.globalServers[0];
            count = MAX_GLOBAL_SERVERS;
            break;
        case AS_FAVORITES:
            servers = &cls.favoriteServers[0];
            count = MAX_OTHER_SERVERS;
            break;
    }
    if (servers)
    {
        for (i = 0; i < count; i++)
        {
            servers[i].ping = -1;
        }
    }
}

/*
====================
LAN_AddServer
====================
*/
static int LAN_AddServer(int source, const char *name, const char *address)
{
    int max, *count, i;
    netadr_t adr;
    serverInfo_t *servers = NULL;
    max = MAX_OTHER_SERVERS;
    count = NULL;

    switch (source)
    {
        case AS_LOCAL:
            count = &cls.numlocalservers;
            servers = &cls.localServers[0];
            break;
        case AS_MPLAYER:
        case AS_GLOBAL:
            max = MAX_GLOBAL_SERVERS;
            count = &cls.numglobalservers;
            servers = &cls.globalServers[0];
            break;
        case AS_FAVORITES:
            count = &cls.numfavoriteservers;
            servers = &cls.favoriteServers[0];
            break;
    }
    if (servers && *count < max)
    {
        NET_StringToAdr(address, &adr, NA_UNSPEC);
        for (i = 0; i < *count; i++)
        {
            if (NET_CompareAdr(servers[i].adr, adr))
            {
                break;
            }
        }
        if (i >= *count)
        {
            servers[*count].adr = adr;
            Q_strncpyz(servers[*count].hostName, name, sizeof(servers[*count].hostName));
            servers[*count].visible = true;
            (*count)++;
            return 1;
        }
        return 0;
    }
    return -1;
}

/*
====================
LAN_RemoveServer
====================
*/
static void LAN_RemoveServer(int source, const char *addr)
{
    int *count, i;
    serverInfo_t *servers = NULL;
    count = NULL;
    switch (source)
    {
        case AS_LOCAL:
            count = &cls.numlocalservers;
            servers = &cls.localServers[0];
            break;
        case AS_MPLAYER:
        case AS_GLOBAL:
            count = &cls.numglobalservers;
            servers = &cls.globalServers[0];
            break;
        case AS_FAVORITES:
            count = &cls.numfavoriteservers;
            servers = &cls.favoriteServers[0];
            break;
    }
    if (servers)
    {
        netadr_t comp;
        NET_StringToAdr(addr, &comp, NA_UNSPEC);
        for (i = 0; i < *count; i++)
        {
            if (NET_CompareAdr(comp, servers[i].adr))
            {
                int j = i;
                while (j < *count - 1)
                {
                    ::memcpy(&servers[j], &servers[j + 1], sizeof(servers[j]));
                    j++;
                }
                (*count)--;
                break;
            }
        }
    }
}

/*
====================
LAN_GetServerCount
====================
*/
static int LAN_GetServerCount(int source)
{
    switch (source)
    {
        case AS_LOCAL:
            return cls.numlocalservers;
            break;
        case AS_MPLAYER:
        case AS_GLOBAL:
            return cls.numglobalservers;
            break;
        case AS_FAVORITES:
            return cls.numfavoriteservers;
            break;
    }
    return 0;
}

/*
====================
LAN_GetLocalServerAddressString
====================
*/
static void LAN_GetServerAddressString(int source, int n, char *buf, int buflen)
{
    switch (source)
    {
        case AS_LOCAL:
            if (n >= 0 && n < MAX_OTHER_SERVERS)
            {
                Q_strncpyz(buf, NET_AdrToStringwPort(cls.localServers[n].adr), buflen);
                if (cls.localServers[n].adr.alternateProtocol != 0)
                    Q_strncpyz(buf + (int)strlen(buf), (cls.localServers[n].adr.alternateProtocol == 1 ? " -g" : " -1"),
                        buflen - (int)strlen(buf));
                return;
            }
            break;
        case AS_MPLAYER:
        case AS_GLOBAL:
            if (n >= 0 && n < MAX_GLOBAL_SERVERS)
            {
                Q_strncpyz(buf, NET_AdrToStringwPort(cls.globalServers[n].adr), buflen);
                if (cls.globalServers[n].adr.alternateProtocol != 0)
                    Q_strncpyz(buf + (int)strlen(buf),
                        (cls.globalServers[n].adr.alternateProtocol == 1 ? " -g" : " -1"), buflen - (int)strlen(buf));
                return;
            }
            break;
        case AS_FAVORITES:
            if (n >= 0 && n < MAX_OTHER_SERVERS)
            {
                Q_strncpyz(buf, NET_AdrToStringwPort(cls.favoriteServers[n].adr), buflen);
                if (cls.favoriteServers[n].adr.alternateProtocol != 0)
                    Q_strncpyz(buf + (int)strlen(buf),
                        (cls.favoriteServers[n].adr.alternateProtocol == 1 ? " -g" : " -1"), buflen - (int)strlen(buf));
                return;
            }
            break;
    }
    buf[0] = '\0';
}

/*
====================
LAN_GetServerInfo
====================
*/
static void LAN_GetServerInfo(int source, int n, char *buf, int buflen)
{
    char info[MAX_STRING_CHARS];
    serverInfo_t *server = NULL;
    info[0] = '\0';
    switch (source)
    {
        case AS_LOCAL:
            if (n >= 0 && n < MAX_OTHER_SERVERS)
            {
                server = &cls.localServers[n];
            }
            break;
        case AS_MPLAYER:
        case AS_GLOBAL:
            if (n >= 0 && n < MAX_GLOBAL_SERVERS)
            {
                server = &cls.globalServers[n];
            }
            break;
        case AS_FAVORITES:
            if (n >= 0 && n < MAX_OTHER_SERVERS)
            {
                server = &cls.favoriteServers[n];
            }
            break;
    }
    if (server && buf)
    {
        buf[0] = '\0';
        if (server->adr.alternateProtocol != 0)
        {
            char hn[MAX_HOSTNAME_LENGTH];
            Q_strncpyz(hn, server->hostName, sizeof(hn));
            Q_strcat(
                hn, sizeof(hn), (server->adr.alternateProtocol == 1 ? S_COLOR_WHITE " [GPP]" : S_COLOR_WHITE " [1.1]"));
            Info_SetValueForKey(info, "hostname", hn);
        }
        else
        {
            Info_SetValueForKey(info, "hostname", server->hostName);
        }
        Info_SetValueForKey(info, "mapname", server->mapName);
        Info_SetValueForKey(info, "label", server->label);
        Info_SetValueForKey(info, "clients", va("%i", server->clients));
        Info_SetValueForKey(info, "sv_maxclients", va("%i", server->maxClients));
        Info_SetValueForKey(info, "ping", va("%i", server->ping));
        Info_SetValueForKey(info, "minping", va("%i", server->minPing));
        Info_SetValueForKey(info, "maxping", va("%i", server->maxPing));
        Info_SetValueForKey(info, "game", server->game);
        Info_SetValueForKey(info, "gametype", va("%i", server->gameType));
        Info_SetValueForKey(info, "nettype", va("%i", server->netType));
        Info_SetValueForKey(info, "addr", NET_AdrToStringwPort(server->adr));
        Q_strncpyz(buf, info, buflen);
    }
    else
    {
        if (buf)
        {
            buf[0] = '\0';
        }
    }
}

/*
====================
LAN_GetServerPing
====================
*/
static int LAN_GetServerPing(int source, int n)
{
    serverInfo_t *server = NULL;
    switch (source)
    {
        case AS_LOCAL:
            if (n >= 0 && n < MAX_OTHER_SERVERS)
            {
                server = &cls.localServers[n];
            }
            break;
        case AS_MPLAYER:
        case AS_GLOBAL:
            if (n >= 0 && n < MAX_GLOBAL_SERVERS)
            {
                server = &cls.globalServers[n];
            }
            break;
        case AS_FAVORITES:
            if (n >= 0 && n < MAX_OTHER_SERVERS)
            {
                server = &cls.favoriteServers[n];
            }
            break;
    }
    if (server)
    {
        return server->ping;
    }
    return -1;
}

/*
====================
LAN_GetServerPtr
====================
*/
static serverInfo_t *LAN_GetServerPtr(int source, int n)
{
    switch (source)
    {
        case AS_LOCAL:
            if (n >= 0 && n < MAX_OTHER_SERVERS)
            {
                return &cls.localServers[n];
            }
            break;
        case AS_MPLAYER:
        case AS_GLOBAL:
            if (n >= 0 && n < MAX_GLOBAL_SERVERS)
            {
                return &cls.globalServers[n];
            }
            break;
        case AS_FAVORITES:
            if (n >= 0 && n < MAX_OTHER_SERVERS)
            {
                return &cls.favoriteServers[n];
            }
            break;
    }
    return NULL;
}

#define FEATURED_MAXPING 200
/*
====================
LAN_CompareServers
====================
*/
static int LAN_CompareServers(int source, int sortKey, int sortDir, int s1, int s2)
{
    int res;
    serverInfo_t *server1, *server2;

    server1 = LAN_GetServerPtr(source, s1);
    server2 = LAN_GetServerPtr(source, s2);
    if (!server1 || !server2)
    {
        return 0;
    }

    // featured servers on top
    if ((server1->label[0] && server1->ping <= FEATURED_MAXPING) ||
        (server2->label[0] && server2->ping <= FEATURED_MAXPING))
    {
        res = Q_stricmpn(server1->label, server2->label, MAX_FEATLABEL_CHARS);
        if (res) return -res;
    }

    res = 0;
    switch (sortKey)
    {
        case SORT_HOST:
        {
            char hostName1[MAX_HOSTNAME_LENGTH];
            char hostName2[MAX_HOSTNAME_LENGTH];
            char *p;
            int i;

            for (p = server1->hostName, i = 0; *p != '\0'; p++)
            {
                if (Q_isalpha(*p)) hostName1[i++] = *p;
            }
            hostName1[i] = '\0';

            for (p = server2->hostName, i = 0; *p != '\0'; p++)
            {
                if (Q_isalpha(*p)) hostName2[i++] = *p;
            }
            hostName2[i] = '\0';

            res = Q_stricmp(hostName1, hostName2);
        }
        break;

        case SORT_GAME:
            res = Q_stricmp(server1->game, server2->game);
            break;
        case SORT_MAP:
            res = Q_stricmp(server1->mapName, server2->mapName);
            break;
        case SORT_CLIENTS:
            if (server1->clients < server2->clients)
            {
                res = -1;
            }
            else if (server1->clients > server2->clients)
            {
                res = 1;
            }
            else
            {
                res = 0;
            }
            break;
        case SORT_PING:
            if (server1->ping < server2->ping)
            {
                res = -1;
            }
            else if (server1->ping > server2->ping)
            {
                res = 1;
            }
            else
            {
                res = 0;
            }
            break;
    }

    if (sortDir)
    {
        if (res < 0) return 1;
        if (res > 0) return -1;
        return 0;
    }
    return res;
}

/*
====================
LAN_GetPingQueueCount
====================
*/
static int LAN_GetPingQueueCount(void) { return (CL_GetPingQueueCount()); }
/*
====================
LAN_ClearPing
====================
*/
static void LAN_ClearPing(int n) { CL_ClearPing(n); }
/*
====================
LAN_GetPing
====================
*/
static void LAN_GetPing(int n, char *buf, int buflen, int *pingtime) { CL_GetPing(n, buf, buflen, pingtime); }
/*
====================
LAN_GetPingInfo
====================
*/
static void LAN_GetPingInfo(int n, char *buf, int buflen) { CL_GetPingInfo(n, buf, buflen); }
/*
====================
LAN_MarkServerVisible
====================
*/
static void LAN_MarkServerVisible(int source, int n, bool visible)
{
    if (n == -1)
    {
        int count = MAX_OTHER_SERVERS;
        serverInfo_t *server = NULL;
        switch (source)
        {
            case AS_LOCAL:
                server = &cls.localServers[0];
                break;
            case AS_MPLAYER:
            case AS_GLOBAL:
                server = &cls.globalServers[0];
                count = MAX_GLOBAL_SERVERS;
                break;
            case AS_FAVORITES:
                server = &cls.favoriteServers[0];
                break;
        }
        if (server)
        {
            for (n = 0; n < count; n++)
            {
                server[n].visible = visible;
            }
        }
    }
    else
    {
        switch (source)
        {
            case AS_LOCAL:
                if (n >= 0 && n < MAX_OTHER_SERVERS)
                {
                    cls.localServers[n].visible = visible;
                }
                break;
            case AS_MPLAYER:
            case AS_GLOBAL:
                if (n >= 0 && n < MAX_GLOBAL_SERVERS)
                {
                    cls.globalServers[n].visible = visible;
                }
                break;
            case AS_FAVORITES:
                if (n >= 0 && n < MAX_OTHER_SERVERS)
                {
                    cls.favoriteServers[n].visible = visible;
                }
                break;
        }
    }
}

/*
=======================
LAN_ServerIsVisible
=======================
*/
static bool LAN_ServerIsVisible(int source, int n)
{
    switch (source)
    {
        case AS_LOCAL:
            if (n >= 0 && n < MAX_OTHER_SERVERS)
            {
                return cls.localServers[n].visible;
            }
            break;
        case AS_MPLAYER:
        case AS_GLOBAL:
            if (n >= 0 && n < MAX_GLOBAL_SERVERS)
            {
                return cls.globalServers[n].visible;
            }
            break;
        case AS_FAVORITES:
            if (n >= 0 && n < MAX_OTHER_SERVERS)
            {
                return cls.favoriteServers[n].visible;
            }
            break;
    }
    return false;
}

/*
=======================
LAN_UpdateVisiblePings
=======================
*/
static bool LAN_UpdateVisiblePings(int source) { return CL_UpdateVisiblePings_f(source); }
/*
====================
LAN_GetServerStatus
====================
*/
static bool LAN_GetServerStatus(char *serverAddress, char *serverStatus, int maxLen)
{
    return CL_ServerStatus(serverAddress, serverStatus, maxLen);
}

/*
====================
CL_GetGlConfig
====================
*/
static void CL_GetGlconfig(glconfig_t *config) { *config = cls.glconfig; }
/*
====================
GetConfigString
====================
*/
static bool GetConfigString(int i, char *buf, int size)
{
    int offset;

    if (i < 0 || i >= MAX_CONFIGSTRINGS) return false;

    offset = cl.gameState.stringOffsets[i];
    if (!offset)
    {
        if (size) buf[0] = '\0';
        return false;
    }

    Q_strncpyz(buf, cl.gameState.stringData + offset, size);

    return true;
}

/*
====================
FloatAsInt
====================
*/
static int FloatAsInt(float f)
{
    floatint_t fi;
    fi.f = f;
    return fi.i;
}

static bool probingUI = false;

/*
====================
CL_UISystemCalls

The ui module is making a system call
====================
*/
intptr_t CL_UISystemCalls(intptr_t *args)
{
    if (cls.uiInterface == 2)
    {
        if (args[0] >= UI_R_SETCLIPREGION && args[0] < UI_MEMSET)
        {
            if (args[0] < UI_S_STOPBACKGROUNDTRACK - 1)
            {
                args[0] += 1;
            }
            else if (args[0] < UI_S_STOPBACKGROUNDTRACK + 4)
            {
                args[0] += UI_PARSE_ADD_GLOBAL_DEFINE - UI_S_STOPBACKGROUNDTRACK + 1;
            }
            else if (args[0] >= UI_PARSE_ADD_GLOBAL_DEFINE + 4)
            {
                args[0] += UI_GETNEWS - UI_PARSE_ADD_GLOBAL_DEFINE - 5;
                if (args[0] == UI_PARSE_SOURCE_FILE_AND_LINE || args[0] == UI_GETNEWS)
                    args[0] = UI_PARSE_SOURCE_FILE_AND_LINE - 1337 - args[0];
            }
            else
            {
                args[0] -= 4;
            }
        }

        switch (args[0])
        {
            case UI_LAN_GETSERVERCOUNT:
            case UI_LAN_GETSERVERADDRESSSTRING:
            case UI_LAN_GETSERVERINFO:
            case UI_LAN_MARKSERVERVISIBLE:
            case UI_LAN_UPDATEVISIBLEPINGS:
            case UI_LAN_RESETPINGS:
            case UI_LAN_ADDSERVER:
            case UI_LAN_REMOVESERVER:
            case UI_LAN_GETSERVERPING:
            case UI_LAN_SERVERISVISIBLE:
            case UI_LAN_COMPARESERVERS:
                if (args[1] == AS_GLOBAL)
                {
                    args[1] = AS_LOCAL;
                }
                else if (args[1] == AS_LOCAL)
                {
                    args[1] = AS_GLOBAL;
                }
        }
    }

    switch (args[0])
    {
        case UI_ERROR:
            if (probingUI)
            {
                cls.uiInterface = 2;
                return 0;
            }
            Com_Error(ERR_DROP, "%s", (const char *)VMA(1));
            return 0;

        case UI_PRINT:
            Com_Printf("%s", (const char *)VMA(1));
            return 0;

        case UI_MILLISECONDS:
            return Sys_Milliseconds();

        case UI_CVAR_REGISTER:
            Cvar_Register((vmCvar_t *)VMA(1), (const char *)VMA(2), (const char *)VMA(3), args[4]);
            return 0;

        case UI_CVAR_UPDATE:
            Cvar_Update((vmCvar_t *)VMA(1));
            return 0;

        case UI_CVAR_SET:
            Cvar_SetSafe((const char *)VMA(1), (const char *)VMA(2));
            return 0;

        case UI_CVAR_VARIABLEVALUE:
            return FloatAsInt(Cvar_VariableValue((const char *)VMA(1)));

        case UI_CVAR_VARIABLESTRINGBUFFER:
            Cvar_VariableStringBuffer((const char *)VMA(1), (char *)VMA(2), args[3]);
            return 0;

        case UI_CVAR_SETVALUE:
            Cvar_SetValueSafe((const char *)VMA(1), VMF(2));
            return 0;

        case UI_CVAR_RESET:
            Cvar_Reset((const char *)VMA(1));
            return 0;

        case UI_CVAR_CREATE:
            Cvar_Register(NULL, (const char *)VMA(1), (const char *)VMA(2), args[3]);
            return 0;

        case UI_CVAR_INFOSTRINGBUFFER:
            Cvar_InfoStringBuffer(args[1], (char *)VMA(2), args[3]);
            return 0;

        case UI_ARGC:
            return Cmd_Argc();

        case UI_ARGV:
            Cmd_ArgvBuffer(args[1], (char *)VMA(2), args[3]);
            return 0;

        case UI_CMD_EXECUTETEXT:
            if (args[1] == EXEC_NOW)
            {
                if (!strncmp((const char *)VMA(2), "snd_restart", 11)
                 || !strncmp((const char *)VMA(2), "vid_restart", 11)
                 || !strncmp((const char *)VMA(2), "quit", 5))
                {
                    Com_Printf(S_COLOR_YELLOW "turning EXEC_NOW '%.11s' into EXEC_INSERT\n", (const char *)VMA(2));
                    args[1] = EXEC_INSERT;
                }
            }

            //  TODO: Do this better
            if (!strncmp((const char *)VMA(2), "checkForUpdate", 14))
            {
                CL_GetLatestRelease();
                return 0;
            }

            Cbuf_ExecuteText(args[1], (const char *)VMA(2));
            return 0;

        case UI_FS_FOPENFILE:
            return FS_FOpenFileByMode((const char *)VMA(1), (fileHandle_t *)VMA(2), (FS_Mode)args[3]);

        case UI_FS_READ:
            FS_Read(VMA(1), args[2], args[3]);
            return 0;

        case UI_FS_WRITE:
            FS_Write(VMA(1), args[2], args[3]);
            return 0;

        case UI_FS_FCLOSEFILE:
            FS_FCloseFile(args[1]);
            return 0;

        case UI_FS_GETFILELIST:
            return FS_GetFileList((const char *)VMA(1), (const char *)VMA(2), (char *)VMA(3), args[4]);

        case UI_FS_SEEK:
            return FS_Seek((fileHandle_t)args[1], args[2], (FS_Origin)args[3]);

        case UI_R_REGISTERMODEL:
            return re.RegisterModel((const char *)VMA(1));

        case UI_R_REGISTERSKIN:
            return re.RegisterSkin((const char *)VMA(1));

        case UI_R_REGISTERSHADERNOMIP:
            return re.RegisterShaderNoMip((const char *)VMA(1));

        case UI_R_CLEARSCENE:
            re.ClearScene();
            return 0;

        case UI_R_ADDREFENTITYTOSCENE:
            re.AddRefEntityToScene((const refEntity_t *)VMA(1));
            return 0;

        case UI_R_ADDPOLYTOSCENE:
            re.AddPolyToScene(args[1], args[2], (const polyVert_t *)VMA(3), 1);
            return 0;

        case UI_R_ADDLIGHTTOSCENE:
            re.AddLightToScene((const float *)VMA(1), VMF(2), VMF(3), VMF(4), VMF(5));
            return 0;

        case UI_R_RENDERSCENE:
            re.RenderScene((const refdef_t *)VMA(1));
            return 0;

        case UI_R_SETCOLOR:
            re.SetColor((const float *)VMA(1));
            return 0;

        case UI_R_SETCLIPREGION:
            re.SetClipRegion((const float *)VMA(1));
            return 0;

        case UI_R_DRAWSTRETCHPIC:
            re.DrawStretchPic(VMF(1), VMF(2), VMF(3), VMF(4), VMF(5), VMF(6), VMF(7), VMF(8), args[9]);
            return 0;

        case UI_R_MODELBOUNDS:
            re.ModelBounds(args[1], (float *)VMA(2), (float *)VMA(3));
            return 0;

        case UI_UPDATESCREEN:
            SCR_UpdateScreen();
            return 0;

        case UI_CM_LERPTAG:
            re.LerpTag((orientation_t *)VMA(1), args[2], args[3], args[4], VMF(5), (const char *)VMA(6));
            return 0;

        case UI_S_REGISTERSOUND:
            return S_RegisterSound((const char *)VMA(1), (bool)args[2]);

        case UI_S_STARTLOCALSOUND:
            S_StartLocalSound(args[1], args[2]);
            return 0;

        case UI_KEY_KEYNUMTOSTRINGBUF:
            Key_KeynumToStringBuf(args[1], (char *)VMA(2), args[3]);
            return 0;

        case UI_KEY_GETBINDINGBUF:
            Key_GetBindingBuf(args[1], (char *)VMA(2), args[3]);
            return 0;

        case UI_KEY_SETBINDING:
            Key_SetBinding(args[1], (const char *)VMA(2));
            return 0;

        case UI_KEY_ISDOWN:
            return Key_IsDown(args[1]);

        case UI_KEY_GETOVERSTRIKEMODE:
            return Key_GetOverstrikeMode();

        case UI_KEY_SETOVERSTRIKEMODE:
            Key_SetOverstrikeMode((bool)args[1]);
            return 0;

        case UI_KEY_CLEARSTATES:
            Key_ClearStates();
            return 0;

        case UI_KEY_GETCATCHER:
            return Key_GetCatcher();

        case UI_KEY_SETCATCHER:
            // don't allow the ui module to toggle the console
            Key_SetCatcher((args[1] & ~KEYCATCH_CONSOLE) | (Key_GetCatcher() & KEYCATCH_CONSOLE));
            return 0;

        case UI_GETCLIPBOARDDATA:
            ((char *)VMA(1))[0] = '\0';
            return 0;

        case UI_GETCLIENTSTATE:
            GetClientState((uiClientState_t *)VMA(1));
            return 0;

        case UI_GETGLCONFIG:
            CL_GetGlconfig((glconfig_t *)VMA(1));
            return 0;

        case UI_GETCONFIGSTRING:
            return GetConfigString(args[1], (char *)VMA(2), args[3]);

        case UI_LAN_LOADCACHEDSERVERS:
            LAN_LoadCachedServers();
            return 0;

        case UI_LAN_SAVECACHEDSERVERS:
            LAN_SaveServersToCache();
            return 0;

        case UI_LAN_ADDSERVER:
            return LAN_AddServer(args[1], (const char *)VMA(2), (const char *)VMA(3));

        case UI_LAN_REMOVESERVER:
            LAN_RemoveServer(args[1], (const char *)VMA(2));
            return 0;

        case UI_LAN_GETPINGQUEUECOUNT:
            return LAN_GetPingQueueCount();

        case UI_LAN_CLEARPING:
            LAN_ClearPing(args[1]);
            return 0;

        case UI_LAN_GETPING:
            LAN_GetPing(args[1], (char *)VMA(2), args[3], (int *)VMA(4));
            return 0;

        case UI_LAN_GETPINGINFO:
            LAN_GetPingInfo(args[1], (char *)VMA(2), args[3]);
            return 0;

        case UI_LAN_GETSERVERCOUNT:
            return LAN_GetServerCount(args[1]);

        case UI_LAN_GETSERVERADDRESSSTRING:
            LAN_GetServerAddressString(args[1], args[2], (char *)VMA(3), args[4]);
            return 0;

        case UI_LAN_GETSERVERINFO:
            LAN_GetServerInfo(args[1], args[2], (char *)VMA(3), args[4]);
            return 0;

        case UI_LAN_GETSERVERPING:
            return LAN_GetServerPing(args[1], args[2]);

        case UI_LAN_MARKSERVERVISIBLE:
            LAN_MarkServerVisible(args[1], args[2], (bool)args[3]);
            return 0;

        case UI_LAN_SERVERISVISIBLE:
            return LAN_ServerIsVisible(args[1], args[2]);

        case UI_LAN_UPDATEVISIBLEPINGS:
            return LAN_UpdateVisiblePings(args[1]);

        case UI_LAN_RESETPINGS:
            LAN_ResetPings(args[1]);
            return 0;

        case UI_LAN_SERVERSTATUS:
            return LAN_GetServerStatus((char *)VMA(1), (char *)VMA(2), args[3]);

        case UI_GETNEWS:
            return GetNews((bool)args[1]);

        case UI_LAN_COMPARESERVERS:
            return LAN_CompareServers(args[1], args[2], args[3], args[4], args[5]);

        case UI_MEMORY_REMAINING:
            return Hunk_MemoryRemaining();

        case UI_SET_PBCLSTATUS:
            return 0;

        case UI_R_REGISTERFONT:
            re.RegisterFont((const char *)VMA(1), args[2], (fontInfo_t *)VMA(3));
            return 0;

        case UI_MEMSET:
            ::memset(VMA(1), args[2], args[3]);
            return 0;

        case UI_MEMCPY:
            ::memcpy(VMA(1), VMA(2), args[3]);
            return 0;

        case UI_STRNCPY:
            strncpy((char *)VMA(1), (const char *)VMA(2), args[3]);
            return args[1];

        case UI_SIN:
            return FloatAsInt(sin(VMF(1)));

        case UI_COS:
            return FloatAsInt(cos(VMF(1)));

        case UI_ATAN2:
            return FloatAsInt(atan2(VMF(1), VMF(2)));

        case UI_SQRT:
            return FloatAsInt(sqrt(VMF(1)));

        case UI_FLOOR:
            return FloatAsInt(floor(VMF(1)));

        case UI_CEIL:
            return FloatAsInt(ceil(VMF(1)));

        case UI_PARSE_ADD_GLOBAL_DEFINE:
            return Parse_AddGlobalDefine((char *)VMA(1));
        case UI_PARSE_LOAD_SOURCE:
            return Parse_LoadSourceHandle((char *)VMA(1));
        case UI_PARSE_FREE_SOURCE:
            return Parse_FreeSourceHandle(args[1]);
        case UI_PARSE_READ_TOKEN:
            return Parse_ReadTokenHandle(args[1], (pc_token_t *)VMA(2));
        case UI_PARSE_SOURCE_FILE_AND_LINE:
            return Parse_SourceFileAndLine(args[1], (char *)VMA(2), (int *)VMA(3));

        case UI_S_STOPBACKGROUNDTRACK:
            S_StopBackgroundTrack();
            return 0;
        case UI_S_STARTBACKGROUNDTRACK:
            S_StartBackgroundTrack((const char *)VMA(1), (const char *)VMA(2));
            return 0;

        case UI_REAL_TIME:
            return Com_RealTime((qtime_t *)VMA(1));

        case UI_CIN_PLAYCINEMATIC:
            return CIN_PlayCinematic((const char *)VMA(1), args[2], args[3], args[4], args[5], args[6]);

        case UI_CIN_STOPCINEMATIC:
            return CIN_StopCinematic(args[1]);

        case UI_CIN_RUNCINEMATIC:
            return CIN_RunCinematic(args[1]);

        case UI_CIN_DRAWCINEMATIC:
            CIN_DrawCinematic(args[1]);
            return 0;

        case UI_CIN_SETEXTENTS:
            CIN_SetExtents(args[1], args[2], args[3], args[4], args[5]);
            return 0;

        case UI_R_REMAP_SHADER:
            re.RemapShader((const char *)VMA(1), (const char *)VMA(2), (const char *)VMA(3));
            return 0;

        default:
            Com_Error(ERR_DROP, "Bad UI system trap: %ld", (long int)args[0]);
    }

    return 0;
}

/*
====================
CL_ShutdownUI
====================
*/
void CL_ShutdownUI(void)
{
    Key_SetCatcher(Key_GetCatcher() & ~KEYCATCH_UI);
    cls.uiStarted = false;
    if (!cls.ui)
    {
        return;
    }
    VM_Call(cls.ui, UI_SHUTDOWN);
    VM_Free(cls.ui);
    cls.ui = NULL;
}

/*
====================
CL_InitUI
====================
*/
void CL_InitUI(void)
{
    // load the dll or bytecode
    vmInterpret_t interpret = (vmInterpret_t)Cvar_VariableValue("vm_ui");
    if (cl_connectedToPureServer)
    {
        // if sv_pure is set we only allow qvms to be loaded
        if (interpret != VMI_COMPILED && interpret != VMI_BYTECODE) interpret = VMI_COMPILED;
    }

    cls.ui = VM_Create("ui", CL_UISystemCalls, interpret);
    if (!cls.ui)
    {
        Com_Printf("Failed to find a valid UI vm. The following paths were searched:\n");
        Cmd_ExecuteString("path /\n");
        Com_Error(ERR_RECONNECT, "VM_Create on UI failed");
    }

    // sanity check
    int v = VM_Call(cls.ui, UI_GETAPIVERSION);
    if (v != UI_API_VERSION)
    {
        // Free cls.ui now, so UI_SHUTDOWN doesn't get called later.
        VM_Free(cls.ui);
        cls.ui = NULL;

        cls.uiStarted = false;
        Com_Error(ERR_DROP, "User Interface is version %d, expected %d", v, UI_API_VERSION);
    }

    // Probe UI interface
    // Calls the GPP UI_CONSOLE_COMMAND (10), if GPP will return false 0. If a 1.1.0 qvm, will hit the error handler.
    Cmd_TokenizeString("");
    cls.uiInterface = 0;
    probingUI = true;
    if ( VM_Call(cls.ui, UI_CONSOLE_COMMAND, 0) < 0 )
        cls.uiInterface = 2;

    probingUI = false;

    if (clc.state >= CA_CONNECTED && clc.state <= CA_ACTIVE &&
        (clc.netchan.alternateProtocol == 2) != (cls.uiInterface == 2))
    {
        Com_Printf(S_COLOR_YELLOW "WARNING: %s protocol %i, but a ui module using the %s interface was found\n",
            (clc.demoplaying ? "Demo was recorded using" : "Server uses"),
            (clc.netchan.alternateProtocol == 0 ? PROTOCOL_VERSION : clc.netchan.alternateProtocol == 1 ? 70 : 69),
            (cls.uiInterface == 2 ? "1.1" : "non-1.1"));
    }

    // init for this gamestate
    VM_Call(cls.ui, UI_INIT, (clc.state >= CA_AUTHORIZING && clc.state < CA_ACTIVE));

    // show where the ui folder was loaded from
    Cmd_ExecuteString("which ui/\n");

    clc.newsString[0] = '\0';
}

/*
====================
UI_GameCommand

See if the current console command is claimed by the ui
====================
*/
bool UI_GameCommand(void)
{
    if (!cls.ui) return false;

    return (bool)VM_Call(cls.ui, UI_CONSOLE_COMMAND - (cls.uiInterface == 2 ? 2 : 0), cls.realtime);
}
