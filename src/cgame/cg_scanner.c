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

#define RANGE 1000.0f

#define XPOS  0.0f
#define YPOS  0.0f
#define WIDTH 640.0f
#define HEIGHT 480.0f

#define STALKWIDTH  2.0f
#define BLIPX 16.0f
#define BLIPY 8.0f

static void CG_DrawBlips( vec3_t origin, vec4_t colour )
{
  vec3_t  drawOrigin;
  vec3_t  up = { 0, 0, 1 };

  RotatePointAroundVector( drawOrigin, up, origin, -cg.refdefViewAngles[ 1 ] - 90 );
  drawOrigin[ 0 ] /= ( 2 * RANGE / WIDTH );
  drawOrigin[ 1 ] /= ( 2 * RANGE / HEIGHT );
  drawOrigin[ 2 ] /= ( 2 * RANGE / WIDTH );

  trap_R_SetColor( colour );

  if( drawOrigin[ 2 ] > 0 )
    CG_DrawPic( XPOS + ( WIDTH / 2 ) - ( STALKWIDTH / 2 ) - drawOrigin[ 0 ],
                YPOS + ( HEIGHT / 2 ) + drawOrigin[ 1 ] - drawOrigin[ 2 ],
                STALKWIDTH, drawOrigin[ 2 ], cgs.media.scannerLineShader );
  else
    CG_DrawPic( XPOS + ( WIDTH / 2 ) - ( STALKWIDTH / 2 ) - drawOrigin[ 0 ],
                YPOS + ( HEIGHT / 2 ) + drawOrigin[ 1 ],
                STALKWIDTH, -drawOrigin[ 2 ], cgs.media.scannerLineShader );
  
  CG_DrawPic( XPOS + ( WIDTH / 2 ) - ( BLIPX / 2 ) - drawOrigin[ 0 ],
              YPOS + ( HEIGHT / 2 ) - ( BLIPY / 2 ) + drawOrigin[ 1 ] - drawOrigin[ 2 ],
              BLIPX, BLIPY, cgs.media.scannerBlipShader );
  trap_R_SetColor( NULL );
}

/*
Global Scanner draw
*/
void CG_Scanner( )
{
  int     i;
  vec3_t  origin;
  vec3_t  relOrigin;
  vec4_t  hIabove = { 0, 1, 0, 1 };
  vec4_t  hIbelow = { 0, 0.5, 0, 1 };
  vec4_t  aIabove = { 1, 0, 0, 1 };
  vec4_t  aIbelow = { 0.5, 0, 0, 1 };
  
  VectorCopy( cg.refdef.vieworg, origin );
  
  //draw human items below scanner plane
  for( i = 0; i < cgIP.numHumanItems; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( cgIP.humanItemPositions[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < RANGE && ( relOrigin[ 2 ] < 0 ) )
      CG_DrawBlips( relOrigin, hIbelow );
  }
  
  //draw alien items below scanner plane
  for( i = 0; i < cgIP.numDroidItems; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( cgIP.droidItemPositions[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < RANGE && ( relOrigin[ 2 ] < 0 ) )
      CG_DrawBlips( relOrigin, aIbelow );
  }
  
  /*CG_DrawPic( XPOS, YPOS, WIDTH, HEIGHT, cgs.media.scannerShader );*/
  
  //draw human items above scanner plane
  for( i = 0; i < cgIP.numHumanItems; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( cgIP.humanItemPositions[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < RANGE && ( relOrigin[ 2 ] > 0 ) )
      CG_DrawBlips( relOrigin, hIabove );
  }

  //draw alien items above scanner plane
  for( i = 0; i < cgIP.numDroidItems; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( cgIP.droidItemPositions[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < RANGE && ( relOrigin[ 2 ] > 0 ) )
      CG_DrawBlips( relOrigin, aIabove );
  }
}
