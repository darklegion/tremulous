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

#ifndef CM_LOCAL_H
#define CM_LOCAL_H 1

#include "cvar.h"
#include "q_shared.h"
#include "qcommon.h"
#include "cm_polylib.h"

#define	MAX_SUBMODELS			256
#define	BOX_MODEL_HANDLE		255
#define CAPSULE_MODEL_HANDLE	254

typedef struct {
	cplane_t	*plane;
	int			children[2];		// negative numbers are leafs
} cNode_t;

typedef struct {
	int			cluster;
	int			area;

	int			firstLeafBrush;
	int			numLeafBrushes;

	int			firstLeafSurface;
	int			numLeafSurfaces;
} cLeaf_t;

typedef struct cmodel_s {
	vec3_t		mins, maxs;
	cLeaf_t		leaf;			// submodels don't reference the main tree
} cmodel_t;

typedef struct cbrushedge_s
{
	vec3_t	p0;
	vec3_t	p1;
} cbrushedge_t;

typedef struct {
	cplane_t			*plane;
	int						planeNum;
	int						surfaceFlags;
	int						shaderNum;
	winding_t			*winding;
} cbrushside_t;

typedef struct {
	int			shaderNum;		// the shader that determined the contents
	int			contents;
	vec3_t		bounds[2];
	int			numsides;
	cbrushside_t	*sides;
	int			checkcount;		// to avoid repeated testings
	bool	    collided; // marker for optimisation
	cbrushedge_t	*edges;
	int						numEdges;
} cbrush_t;


typedef struct {
	int			checkcount;				// to avoid repeated testings
	int			surfaceFlags;
	int			contents;
	struct patchCollide_s	*pc;
} cPatch_t;


typedef struct {
	int			floodnum;
	int			floodvalid;
} cArea_t;

typedef struct {
	char		name[MAX_QPATH];

	int			numShaders;
	dshader_t	*shaders;

	int			numBrushSides;
	cbrushside_t *brushsides;

	int			numPlanes;
	cplane_t	*planes;

	int			numNodes;
	cNode_t		*nodes;

	int			numLeafs;
	cLeaf_t		*leafs;

	int			numLeafBrushes;
	int			*leafbrushes;

	int			numLeafSurfaces;
	int			*leafsurfaces;

	int			numSubModels;
	cmodel_t	*cmodels;

	int			numBrushes;
	cbrush_t	*brushes;

	int			numClusters;
	int			clusterBytes;
	byte		*visibility;
	bool	vised;			// if false, visibility is just a single cluster of ffs

	int			numEntityChars;
	char		*entityString;

	int			numAreas;
	cArea_t		*areas;
	int			*areaPortals;	// [ numAreas*numAreas ] reference counts

	int			numSurfaces;
	cPatch_t	**surfaces;			// non-patches will be NULL

	int			floodvalid;
	int			checkcount;					// incremented on each trace
} clipMap_t;


// keep 1/8 unit away to keep the position valid before network snapping
// and to avoid various numeric issues
#define	SURFACE_CLIP_EPSILON	(0.125)

extern	clipMap_t	cm;
extern	int			c_pointcontents;
extern	int			c_traces, c_brush_traces, c_patch_traces;
extern	cvar_t		*cm_noAreas;
extern	cvar_t		*cm_noCurves;
extern	cvar_t		*cm_playerCurveClip;

// cm_test.c

typedef struct
{
	float		startRadius;
	float		endRadius;
} biSphere_t;

// Used for oriented capsule collision detection
typedef struct
{
	float		radius;
	float		halfheight;
	vec3_t		offset;
} sphere_t;

typedef struct {
	traceType_t	type;
	vec3_t			start;
	vec3_t			end;
	vec3_t			size[2];	// size of the box being swept through the model
	vec3_t			offsets[8];	// [signbits][x] = either size[0][x] or size[1][x]
	float				maxOffset;	// longest corner length from origin
	vec3_t			extents;	// greatest of abs(size[0]) and abs(size[1])
	vec3_t			bounds[2];	// enclosing box of start and end surrounding by size
	vec3_t			modelOrigin;// origin of the model tracing through
	int					contents;	// ored contents of the model tracing through
	bool		isPoint;	// optimized case
	trace_t			trace;		// returned from trace call
	sphere_t		sphere;		// sphere for oriendted capsule collision
	biSphere_t	biSphere;
	bool		testLateralCollision; // whether or not to test for lateral collision
} traceWork_t;

typedef struct leafList_s {
	int		count;
	int		maxcount;
	bool	overflowed;
	int		*list;
	vec3_t	bounds[2];
	int		lastLeaf;		// for overflows where each leaf can't be stored individually
	void	(*storeLeafs)( struct leafList_s *ll, int nodenum );
} leafList_t;


int CM_BoxBrushes( const vec3_t mins, const vec3_t maxs, cbrush_t **list, int listsize );

void CM_StoreLeafs( leafList_t *ll, int nodenum );
void CM_StoreBrushes( leafList_t *ll, int nodenum );

void CM_BoxLeafnums_r( leafList_t *ll, int nodenum );

cmodel_t	*CM_ClipHandleToModel( clipHandle_t handle );
bool CM_BoundsIntersect( const vec3_t mins, const vec3_t maxs, const vec3_t mins2, const vec3_t maxs2 );
bool CM_BoundsIntersectPoint( const vec3_t mins, const vec3_t maxs, const vec3_t point );

// cm_patch.c

struct patchCollide_s	*CM_GeneratePatchCollide( int width, int height, vec3_t *points );
void CM_TraceThroughPatchCollide( traceWork_t *tw, const struct patchCollide_s *pc );
bool CM_PositionTestInPatchCollide( traceWork_t *tw, const struct patchCollide_s *pc );
void CM_ClearLevelPatches( void );

// cm_test.c
void	CM_FloodAreaConnections (void);

#endif
