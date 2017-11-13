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
#ifndef _CM_PUBLIC_H_
#define _CM_PUBLIC_H_

#include "q_platform.h"
#include "qfiles.h"

SO_PUBLIC void CM_LoadMap(const char *name, bool clientload, int *checksum);
void CM_ClearMap(void);
SO_PUBLIC clipHandle_t CM_InlineModel(int index);  // 0 = world, 1 + are bmodels

// FIXME: Split this into TempBoxModel and TempCapsuleModel
SO_PUBLIC clipHandle_t CM_TempBoxModel(const vec3_t mins, const vec3_t maxs, int capsule);

void CM_ModelBounds(clipHandle_t model, vec3_t mins, vec3_t maxs);

int CM_NumClusters(void);
SO_PUBLIC int CM_NumInlineModels(void);
char *CM_EntityString(void);

// returns an ORed contents mask
SO_PUBLIC int CM_PointContents(const vec3_t p, clipHandle_t model);
SO_PUBLIC int CM_TransformedPointContents(const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles);

// FIXME: Split this into BoxTrace and CapsuleTrace
SO_PUBLIC void CM_BoxTrace(trace_t *results, const vec3_t start, const vec3_t end, vec3_t mins, vec3_t maxs, clipHandle_t model,
    int brushmask, traceType_t type);

SO_PUBLIC void CM_TransformedBoxTrace(trace_t *results, const vec3_t start, const vec3_t end, vec3_t mins, vec3_t maxs,
    clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles, traceType_t type);
SO_PUBLIC void CM_BiSphereTrace(trace_t *results, const vec3_t start, const vec3_t end, float startRad, float endRad, clipHandle_t model, int mask);
SO_PUBLIC void CM_TransformedBiSphereTrace(trace_t *results, const vec3_t start, const vec3_t end, float startRad, float endRad,
    clipHandle_t model, int mask, const vec3_t origin);

byte *CM_ClusterPVS(int cluster);

int CM_PointLeafnum(const vec3_t p);

// only returns non-solid leafs
// overflow if return listsize and if *lastLeaf != list[listsize-1]
int CM_BoxLeafnums(const vec3_t mins, const vec3_t maxs, int *list, int listsize, int *lastLeaf);

int CM_LeafCluster(int leafnum);
int CM_LeafArea(int leafnum);

void CM_AdjustAreaPortalState(int area1, int area2, bool open);
SO_PUBLIC bool CM_AreasConnected(int area1, int area2);

int CM_WriteAreaBits(byte *buffer, int area);

// cm_patch.c
void CM_DrawDebugSurface(void (*drawPoly)(int color, int numPoints, float *points));

#endif
