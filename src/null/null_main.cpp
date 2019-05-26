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
// null_main.c -- null system driver to aid porting efforts

#include <cerrno>
#include <cstdio>

#include "qcommon/qcommon.h"

int			sys_curtime;


//===================================================================


void Sys_Error (char *error, ...) {
	va_list		argptr;

	printf ("Sys_Error: ");	
	va_start (argptr,error);
	vprintf (error,argptr);
	va_end (argptr);
	printf ("\n");

	exit (1);
}

void Sys_Quit (void) {
	exit (0);
}

char *Sys_GetClipboardData( void ) {
	return NULL;
}

int		Sys_Milliseconds (void) {
	return 0;
}

FILE	*Sys_FOpen(const char *ospath, const char *mode) {
	return fopen( ospath, mode );
}

void	Sys_Mkdir (char *path) {
}

bool Sys_OpenWithDefault( const char *path )
{
    return false;
}

void	Sys_Init (void) {
}


void main (int argc, char **argv) {
	Com_Init (argc, argv);

	while (1) {
		Com_Frame( );
	}
}
