/*
 *  Portions Copyright (C) 2000-2001 Tim Angus
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*  To assertain which portions are licensed under the GPL and which are
 *  licensed by Id Software, Inc. please run a diff between the equivalent
 *  versions of the "Tremulous" modification and the unmodified "Quake3"
 *  game source code.
 */
                    
#include "cg_local.h"

void CG_Scanner( )
{
  int     i;
  vec3_t  origin;
  vec3_t  relOrigin;
  vec3_t  drawOrigin;
  vec3_t  up = { 0, 0, 1 };
  vec4_t  hIabove = { 0, 1, 0, 1 };
  vec4_t  hIbelow = { 0, 0.5, 0, 1 };
  vec4_t  aIabove = { 1, 0, 0, 1 };
  vec4_t  aIbelow = { 0.5, 0, 0, 1 };
  
  VectorCopy( cg.refdef.vieworg, origin );
  
  for( i = 0; i < cgIP.numHumanItems; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( cgIP.humanItemPositions[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < 1000 && ( relOrigin[ 2 ] < 0 ) )
    {
      RotatePointAroundVector( drawOrigin, up, relOrigin, -cg.refdefViewAngles[ 1 ]-90 );
      drawOrigin[ 0 ] /= ( 1000 / 180 );
      drawOrigin[ 1 ] /= ( 1000 / 40 );
      drawOrigin[ 2 ] /= ( 1000 / 180 );
      
      trap_R_SetColor( hIbelow );
      CG_DrawPic( 319 - drawOrigin[ 0 ], 360 + drawOrigin[ 1 ], 2, -drawOrigin[ 2 ], cgs.media.scannerLineShader );
      CG_DrawPic( 312 - drawOrigin[ 0 ], 356 + drawOrigin[ 1 ] - drawOrigin[ 2 ], 16, 8, cgs.media.scannerBlipShader );
      trap_R_SetColor( NULL );
    }
  }
  
  for( i = 0; i < cgIP.numDroidItems; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( cgIP.droidItemPositions[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < 1000 && ( relOrigin[ 2 ] < 0 ) )
    {
      RotatePointAroundVector( drawOrigin, up, relOrigin, -cg.refdefViewAngles[ 1 ]-90 );
      drawOrigin[ 0 ] /= ( 1000 / 180 );
      drawOrigin[ 1 ] /= ( 1000 / 40 );
      drawOrigin[ 2 ] /= ( 1000 / 180 );
      
      trap_R_SetColor( aIbelow );
      CG_DrawPic( 319 - drawOrigin[ 0 ], 360 + drawOrigin[ 1 ], 2, -drawOrigin[ 2 ], cgs.media.scannerLineShader );
      CG_DrawPic( 312 - drawOrigin[ 0 ], 356 + drawOrigin[ 1 ] - drawOrigin[ 2 ], 16, 8, cgs.media.scannerBlipShader );
      trap_R_SetColor( NULL );
    }
  }
  
  CG_DrawPic( 140, 320, 360, 80, cgs.media.scannerShader );
  
  for( i = 0; i < cgIP.numHumanItems; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( cgIP.humanItemPositions[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < 1000 && ( relOrigin[ 2 ] > 0 ) )
    {
      RotatePointAroundVector( drawOrigin, up, relOrigin, -cg.refdefViewAngles[ 1 ]-90 );
      drawOrigin[ 0 ] /= ( 1000 / 180 );
      drawOrigin[ 1 ] /= ( 1000 / 40 );
      drawOrigin[ 2 ] /= ( 1000 / 180 );
      
      trap_R_SetColor( hIabove );
      CG_DrawPic( 319 - drawOrigin[ 0 ], 360 + drawOrigin[ 1 ], 2, -drawOrigin[ 2 ], cgs.media.scannerLineShader );
      CG_DrawPic( 312 - drawOrigin[ 0 ], 356 + drawOrigin[ 1 ] - drawOrigin[ 2 ], 16, 8, cgs.media.scannerBlipShader );
      trap_R_SetColor( NULL );
    }
  }
  for( i = 0; i < cgIP.numDroidItems; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( cgIP.droidItemPositions[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < 1000 && ( relOrigin[ 2 ] > 0 ) )
    {
      RotatePointAroundVector( drawOrigin, up, relOrigin, -cg.refdefViewAngles[ 1 ]-90 );
      drawOrigin[ 0 ] /= ( 1000 / 180 );
      drawOrigin[ 1 ] /= ( 1000 / 40 );
      drawOrigin[ 2 ] /= ( 1000 / 180 );
      
      trap_R_SetColor( aIabove );
      CG_DrawPic( 319 - drawOrigin[ 0 ], 360 + drawOrigin[ 1 ], 2, -drawOrigin[ 2 ], cgs.media.scannerLineShader );
      CG_DrawPic( 312 - drawOrigin[ 0 ], 356 + drawOrigin[ 1 ] - drawOrigin[ 2 ], 16, 8, cgs.media.scannerBlipShader );
      trap_R_SetColor( NULL );
    }
  }
}
