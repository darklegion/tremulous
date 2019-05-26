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

#include "sys_local.h"

#include "qcommon/cvar.h"
#include "qcommon/q_shared.h"
#include "qcommon/qcommon.h"

#include <stdio.h>

/*
==================
CON_Shutdown
==================
*/
void CON_Shutdown( void )
{
}

/*
==================
CON_Init
==================
*/
void CON_Init( void )
{
}

/*
==================
CON_Input
==================
*/
char *CON_Input( void )
{
	return NULL;
}

/*
==================
CON_Print
==================
*/
void CON_Print( const char *msg )
{
	if( com_ansiColor && com_ansiColor->integer )
		Sys_AnsiColorPrint( msg );
	else
		fputs( msg, stderr );
}
