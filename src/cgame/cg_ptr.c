// cg_ptr.c -- post timeout restoration handling

/*
 *  Portions Copyright (C) 2000-2001 Tim Angus
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the OSML - Open Source Modification License v1.0 as
 *  described in the file COPYING which is distributed with this source
 *  code.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "cg_local.h"

#define PTRC_FILE  "ptrc.cfg"

/*
===============
CG_ReadPTRCode

Read a PTR code from disk
===============
*/
int CG_ReadPTRCode( void )
{
  int           len;
  char          text[ 16 ];
  fileHandle_t  f;

  // load the file
  len = trap_FS_FOpenFile( PTRC_FILE, &f, FS_READ );
  if( len <= 0 )
    return 0;

  // should never happen - malformed write
  if( len >= sizeof( text ) - 1 )
    return 0;
  
  trap_FS_Read( text, len, f );
  text[ len ] = 0;
  trap_FS_FCloseFile( f );

  return atoi( text );
}

/*
===============
CG_WritePTRCode

Write a PTR code to disk
===============
*/
void CG_WritePTRCode( int code )
{
  char          text[ 16 ];
  fileHandle_t  f;

  Com_sprintf( text, 16, "%d", code );
  
  // open file
  if( trap_FS_FOpenFile( PTRC_FILE, &f, FS_WRITE ) < 0 )
    return;

  // write the code
  trap_FS_Write( text, strlen( text ), f );
  
  trap_FS_FCloseFile( f );
}
