// Copyright (C) 1999-2000 Id Software, Inc.
//
//
// g_mem.c
//
//TA: hack to provide dynanmic allocation clientside

/*
 *  Portions Copyright (C) 2000-2001 Tim Angus
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*  To assertain which portions are licensed under the LGPL and which are
 *  licensed by Id Software, Inc. please run a diff between the equivalent
 *  versions of the "Tremulous" modification and the unmodified "Quake3"
 *  game source code.
 */
                    
#include "cg_local.h"


#define POOLSIZE	(256 * 1024)

static char		memoryPool[POOLSIZE];
static int		allocPoint;

void *CG_Alloc( int size ) {
	char	*p;

	/*if ( g_debugAlloc.integer ) {
		G_Printf( "CG_Alloc of %i bytes (%i left)\n", size, POOLSIZE - allocPoint - ( ( size + 31 ) & ~31 ) );
	}*/

	if ( allocPoint + size > POOLSIZE ) {
		CG_Error( "CG_Alloc: failed on allocation of %u bytes\n", size );
		return NULL;
	}

	p = &memoryPool[allocPoint];

	allocPoint += ( size + 31 ) & ~31;

	return p;
}

void CG_InitMemory( void ) {
	allocPoint = 0;
}

