/*
===========================================================================
Copyright (C) 2015-2017 GrangerHub
Copyright (C) 2000-2013 Darklegion Development
Copyright (C) 1999-2005 Id Software, Inc.

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
#ifndef SV_GAME_H
#define SV_GAME_H

struct gentity_t;
struct sharedEntity_t;
struct playerState_t;

SO_PUBLIC void SV_LocateGameData( sharedEntity_t *gEnts, int numGEntities,
        int sizeofGEntity_t, playerState_t *clients, int sizeofGameClient );
SO_PUBLIC void SV_GameDropClient(int clientNum, const char *reason);
SO_PUBLIC void SV_GameSendServerCommand(int clientNum, const char *text);
SO_PUBLIC void SV_SendClientGameState(int clientNum);
SO_PUBLIC void SV_GetServerinfo(char *buffer, int bufferSize);
SO_PUBLIC void SV_SetBrushModel(sharedEntity_t *ent, const char *name);
SO_PUBLIC bool SV_inPVSIgnorePortals(const vec3_t p1, const vec3_t p2);
SO_PUBLIC void SV_AdjustAreaPortalState(sharedEntity_t *ent, bool open);
SO_PUBLIC bool SV_EntityContact(vec3_t mins, vec3_t maxs, const sharedEntity_t *ent, traceType_t type);
SO_PUBLIC void SV_GetUsercmd(int clientNum, usercmd_t *cmd);
SO_PUBLIC bool SV_GetEntityToken(char *buffer, int bufferSize);

//SO_PUBLIC void SV_SendClientGameState2(int clientNum)
//  Only exists in Scribble3
//SO_PUBLIC void G_TraceWrapper(trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask)
//  Only exists in Scribble3

#endif
