/*
===========================================================================
Copyright (C) 2009-2011 Andrei Drexler, Richard Allen, James Canete
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

along with Tremulous; if not, see <https://www.gnu.org/licenses/>

===========================================================================
*/

#ifndef __TR_EXTRATYPES_H__
#define __TR_EXTRATYPES_H__

// tr_extratypes.h, for mods that want to extend tr_types.h without losing compatibility with original VMs

// extra refdef flags start at 0x0008
#define RDF_NOFOG		0x0008		// don't apply fog to polys added using RE_AddPolyToScene
#define RDF_EXTRA		0x0010		// Makro - refdefex_t to follow after refdef_t
#define RDF_SUNLIGHT    0x0020      // SmileTheory - render sunlight and shadows

typedef struct {
	float			blurFactor;
	float           sunDir[3];
	float           sunCol[3];
	float           sunAmbCol[3];
} refdefex_t;

#endif
