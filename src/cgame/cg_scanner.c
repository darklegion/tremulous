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

#define STALKWIDTH  2.0f
#define BLIPX       16.0f
#define BLIPY       8.0f

static void CG_DrawBlips( rectDef_t *rect, vec3_t origin, vec4_t colour )
{
  vec3_t  drawOrigin;
  vec3_t  up = { 0, 0, 1 };

  RotatePointAroundVector( drawOrigin, up, origin, -cg.refdefViewAngles[ 1 ] - 90 );
  drawOrigin[ 0 ] /= ( 2 * HELMET_RANGE / rect->w );
  drawOrigin[ 1 ] /= ( 2 * HELMET_RANGE / rect->h );
  drawOrigin[ 2 ] /= ( 2 * HELMET_RANGE / rect->w );

  trap_R_SetColor( colour );

  if( drawOrigin[ 2 ] > 0 )
    CG_DrawPic( rect->x + ( rect->w / 2 ) - ( STALKWIDTH / 2 ) - drawOrigin[ 0 ],
                rect->y + ( rect->h / 2 ) + drawOrigin[ 1 ] - drawOrigin[ 2 ],
                STALKWIDTH, drawOrigin[ 2 ], cgs.media.scannerLineShader );
  else
    CG_DrawPic( rect->x + ( rect->w / 2 ) - ( STALKWIDTH / 2 ) - drawOrigin[ 0 ],
                rect->y + ( rect->h / 2 ) + drawOrigin[ 1 ],
                STALKWIDTH, -drawOrigin[ 2 ], cgs.media.scannerLineShader );
  
  CG_DrawPic( rect->x + ( rect->w / 2 ) - ( BLIPX / 2 ) - drawOrigin[ 0 ],
              rect->y + ( rect->h / 2 ) - ( BLIPY / 2 ) + drawOrigin[ 1 ] - drawOrigin[ 2 ],
              BLIPX, BLIPY, cgs.media.scannerBlipShader );
  trap_R_SetColor( NULL );
}

#define BLIPX2  24.0f
#define BLIPY2  24.0f

static void CG_DrawDir( rectDef_t *rect, vec3_t origin, vec4_t colour )
{
  vec3_t  drawOrigin;
  vec3_t  noZOrigin;
  vec3_t  normal, antinormal, normalDiff;
  vec3_t  view, noZview;
  vec3_t  up  = { 0.0f, 0.0f,   1.0f };
  vec3_t  top = { 0.0f, -1.0f,  0.0f };
  float   angle;
  playerState_t *ps = &cg.snap->ps;

  if( ps->stats[ STAT_STATE ] & SS_WALLCLIMBING )
  {
    if( ps->stats[ STAT_STATE ] & SS_WALLCLIMBINGCEILING )
      VectorSet( normal, 0.0f, 0.0f, -1.0f );
    else
      VectorCopy( ps->grapplePoint, normal );
  }
  else
    VectorSet( normal, 0.0f, 0.0f, 1.0f );

  AngleVectors( cg.refdefViewAngles, view, NULL, NULL );
  
  ProjectPointOnPlane( noZOrigin, origin, normal );
  ProjectPointOnPlane( noZview, view, normal );
  VectorNormalize( noZOrigin );
  VectorNormalize( noZview );

  //calculate the angle between the images of the blip and the view
  angle = RAD2DEG( acos( DotProduct( noZOrigin, noZview ) ) );
  CrossProduct( noZOrigin, noZview, antinormal );
  VectorNormalize( antinormal );

  //decide which way to rotate
  VectorSubtract( normal, antinormal, normalDiff );
  if( VectorLength( normalDiff ) < 1.0f )
    angle = 360.0f - angle;

  RotatePointAroundVector( drawOrigin, up, top, angle );

  trap_R_SetColor( colour );
  CG_DrawPic( rect->x + ( rect->w / 2 ) - ( BLIPX2 / 2 ) - drawOrigin[ 0 ] * ( rect->w / 2 ),
              rect->y + ( rect->h / 2 ) - ( BLIPY2 / 2 ) + drawOrigin[ 1 ] * ( rect->h / 2 ),
              BLIPX2, BLIPY2, cgs.media.scannerBlipShader );
  trap_R_SetColor( NULL );
}

/*
=============
CG_AlienSense
=============
*/
void CG_AlienSense( rectDef_t *rect )
{
  int     i;
  vec3_t  origin;
  vec3_t  relOrigin;
  vec4_t  buildable = { 1.0f, 0.0f, 0.0f, 0.7f };
  vec4_t  client    = { 0.0f, 0.0f, 1.0f, 0.7f };
  
  VectorCopy( cg.refdef.vieworg, origin );
  
  //draw human buildables
  for( i = 0; i < cg.ep.numHumanBuildables; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( cg.ep.humanBuildablePos[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < ALIENSENSE_RANGE )
      CG_DrawDir( rect, relOrigin, buildable );
  }
  
  //draw human clients
  for( i = 0; i < cg.ep.numHumanClients; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( cg.ep.humanClientPos[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < ALIENSENSE_RANGE )
      CG_DrawDir( rect, relOrigin, client );
  }
}

/*
=============
CG_Scanner
=============
*/
void CG_Scanner( rectDef_t *rect, qhandle_t shader )
{
  int     i;
  vec3_t  origin;
  vec3_t  relOrigin;
  vec4_t  hIabove = { 0.0f, 1.0f, 0.0f, 1.0f };
  vec4_t  hIbelow = { 0.0f, 0.5f, 0.0f, 1.0f };
  vec4_t  aIabove = { 1.0f, 0.0f, 0.0f, 1.0f };
  vec4_t  aIbelow = { 0.5f, 0.0f, 0.0f, 1.0f };
  
  VectorCopy( cg.refdef.vieworg, origin );
  
  //draw human buildables below scanner plane
  for( i = 0; i < cg.ep.numHumanBuildables; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( cg.ep.humanBuildablePos[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < HELMET_RANGE && ( relOrigin[ 2 ] < 0 ) )
      CG_DrawBlips( rect, relOrigin, hIbelow );
  }
  
  //draw alien buildables below scanner plane
  for( i = 0; i < cg.ep.numAlienBuildables; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( cg.ep.alienBuildablePos[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < HELMET_RANGE && ( relOrigin[ 2 ] < 0 ) )
      CG_DrawBlips( rect, relOrigin, aIbelow );
  }
  
  //draw human clients below scanner plane
  for( i = 0; i < cg.ep.numHumanClients; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( cg.ep.humanClientPos[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < HELMET_RANGE && ( relOrigin[ 2 ] < 0 ) )
      CG_DrawBlips( rect, relOrigin, hIbelow );
  }
  
  //draw alien buildables below scanner plane
  for( i = 0; i < cg.ep.numAlienClients; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( cg.ep.alienClientPos[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < HELMET_RANGE && ( relOrigin[ 2 ] < 0 ) )
      CG_DrawBlips( rect, relOrigin, aIbelow );
  }
  
  CG_DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
  
  //draw human buildables above scanner plane
  for( i = 0; i < cg.ep.numHumanBuildables; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( cg.ep.humanBuildablePos[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < HELMET_RANGE && ( relOrigin[ 2 ] > 0 ) )
      CG_DrawBlips( rect, relOrigin, hIabove );
  }

  //draw alien buildables above scanner plane
  for( i = 0; i < cg.ep.numAlienBuildables; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( cg.ep.alienBuildablePos[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < HELMET_RANGE && ( relOrigin[ 2 ] > 0 ) )
      CG_DrawBlips( rect, relOrigin, aIabove );
  }
  
  //draw human clients above scanner plane
  for( i = 0; i < cg.ep.numHumanClients; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( cg.ep.humanClientPos[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < HELMET_RANGE && ( relOrigin[ 2 ] > 0 ) )
      CG_DrawBlips( rect, relOrigin, hIabove );
  }

  //draw alien clients above scanner plane
  for( i = 0; i < cg.ep.numAlienClients; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( cg.ep.alienClientPos[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < HELMET_RANGE && ( relOrigin[ 2 ] > 0 ) )
      CG_DrawBlips( rect, relOrigin, aIabove );
  }
}
