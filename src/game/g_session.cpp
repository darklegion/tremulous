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

#include "g_local.h"

/*
=======================================================================

  SESSION DATA

Session data is the only data that stays persistant across level loads
and tournament restarts.
=======================================================================
*/

/*
================
G_WriteClientSessionData

Called on game shutdown
================
*/
void G_WriteClientSessionData(gclient_t *client)
{
    const char *s;
    const char *var;

    s = va("%i %i %i %i %s", client->sess.spectatorTime, client->sess.spectatorState, client->sess.spectatorClient,
        client->sess.restartTeam, Com_ClientListString(&client->sess.ignoreList));

    var = va("session%i", (int)(client - level.clients));

    Cvar_Set(var, s);
}

/*
================
G_ReadSessionData

Called on a reconnect
================
*/
void G_ReadSessionData(gclient_t *client)
{
    char s[MAX_STRING_CHARS];
    const char *var;
    int spectatorState;
    int restartTeam;
    char ignorelist[17];

    var = va("session%i", (int)(client - level.clients));
    Cvar_VariableStringBuffer(var, s, sizeof(s));

    sscanf(s, "%i %i %i %i %16s", &client->sess.spectatorTime, &spectatorState, &client->sess.spectatorClient,
        &restartTeam, ignorelist);

    client->sess.spectatorState = (spectatorState_t)spectatorState;
    client->sess.restartTeam = (team_t)restartTeam;
    Com_ClientListParse(&client->sess.ignoreList, ignorelist);
}

/*
================
G_InitSessionData

Called on a first-time connect
================
*/
void G_InitSessionData(gclient_t *client, char *userinfo)
{
    clientSession_t *sess;
    const char *value;

    sess = &client->sess;

    // initial team determination
    value = Info_ValueForKey(userinfo, "team");
    if (value[0] == 's')
    {
        // a willing spectator, not a waiting-in-line
        sess->spectatorState = SPECTATOR_FREE;
    }
    else
    {
        if (g_maxGameClients.integer > 0 && level.numNonSpectatorClients >= g_maxGameClients.integer)
            sess->spectatorState = SPECTATOR_FREE;
        else
            sess->spectatorState = SPECTATOR_NOT;
    }

    sess->restartTeam = TEAM_NONE;
    sess->spectatorState = SPECTATOR_FREE;
    sess->spectatorTime = level.time;
    sess->spectatorClient = -1;
    memset(&sess->ignoreList, 0, sizeof(sess->ignoreList));

    G_WriteClientSessionData(client);
}

/*
==================
G_WriteSessionData

==================
*/
void G_WriteSessionData(void)
{
    int i;

    // FIXME: What's this for?
    Cvar_Set("session", va("%i", 0));

    for (i = 0; i < level.maxclients; i++)
    {
        if (level.clients[i].pers.connected == CON_CONNECTED)
            G_WriteClientSessionData(&level.clients[i]);
    }
}
