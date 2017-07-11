/*
===========================================================================
Copyright (C) 2000-2013 Darklegion Development

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


#include "cg_local.h"

static entityPos_t entityPositions;

#define HUMAN_SCANNER_UPDATE_PERIOD 700

/*
=============
CG_UpdateEntityPositions

Update this client's perception of entity positions
=============
*/
void CG_UpdateEntityPositions( void )
{
  centity_t *cent = NULL;
  int       i;

  if( cg.predictedPlayerState.stats[ STAT_TEAM ] == TEAM_HUMANS )
  {
    if( entityPositions.lastUpdateTime + HUMAN_SCANNER_UPDATE_PERIOD > cg.time )
      return;
  }

  VectorCopy( cg.refdef.vieworg, entityPositions.origin );
  VectorCopy( cg.refdefViewAngles, entityPositions.vangles );
  entityPositions.lastUpdateTime = cg.time;

  entityPositions.numAlienBuildables = 0;
  entityPositions.numHumanBuildables = 0;
  entityPositions.numAlienClients = 0;
  entityPositions.numHumanClients = 0;

  for( i = 0; i < cg.snap->numEntities; i++ )
  {
    cent = &cg_entities[ cg.snap->entities[ i ].number ];

    if( cent->currentState.eType == ET_BUILDABLE &&
     !( cent->currentState.eFlags & EF_DEAD ) )
    {
      // add to list of item positions (for creep)
      if( cent->currentState.modelindex2 == TEAM_ALIENS )
      {
        VectorCopy( cent->lerpOrigin, entityPositions.alienBuildablePos[
            entityPositions.numAlienBuildables ] );
        entityPositions.alienBuildableTimes[
            entityPositions.numAlienBuildables ] = cent->miscTime;

        if( entityPositions.numAlienBuildables < MAX_GENTITIES )
          entityPositions.numAlienBuildables++;
      }
      else if( cent->currentState.modelindex2 == TEAM_HUMANS )
      {
        VectorCopy( cent->lerpOrigin, entityPositions.humanBuildablePos[
            entityPositions.numHumanBuildables ] );

        if( entityPositions.numHumanBuildables < MAX_GENTITIES )
          entityPositions.numHumanBuildables++;
      }
    }
    else if( cent->currentState.eType == ET_PLAYER )
    {
      int team = cent->currentState.misc & 0x00FF;

      if( team == TEAM_ALIENS )
      {
        VectorCopy( cent->lerpOrigin, entityPositions.alienClientPos[
            entityPositions.numAlienClients ] );

        if( entityPositions.numAlienClients < MAX_CLIENTS )
          entityPositions.numAlienClients++;
      }
      else if( team == TEAM_HUMANS )
      {
        VectorCopy( cent->lerpOrigin, entityPositions.humanClientPos[
            entityPositions.numHumanClients ] );

        if( entityPositions.numHumanClients < MAX_CLIENTS )
          entityPositions.numHumanClients++;
      }
    }
  }
}

#define STALKWIDTH  (2.0f * cgDC.aspectScale)
#define BLIPX       (16.0f * cgDC.aspectScale)
#define BLIPY       8.0f
#define FAR_ALPHA   0.8f
#define NEAR_ALPHA  1.2f

/*
=============
CG_DrawBlips

Draw blips and stalks for the human scanner
=============
*/
static void CG_DrawBlips( rectDef_t *rect, vec3_t origin, vec4_t colour )
{
  vec3_t  drawOrigin;
  vec3_t  up = { 0, 0, 1 };
  float   alphaMod = 1.0f;
  float   timeFractionSinceRefresh = 1.0f -
    ( (float)( cg.time - entityPositions.lastUpdateTime ) /
      (float)HUMAN_SCANNER_UPDATE_PERIOD );
  vec4_t  localColour;

  Vector4Copy( colour, localColour );

  RotatePointAroundVector( drawOrigin, up, origin, -entityPositions.vangles[ 1 ] - 90 );
  drawOrigin[ 0 ] /= ( 2 * HELMET_RANGE / rect->w );
  drawOrigin[ 1 ] /= ( 2 * HELMET_RANGE / rect->h );
  drawOrigin[ 2 ] /= ( 2 * HELMET_RANGE / rect->w );

  alphaMod = FAR_ALPHA +
    ( ( drawOrigin[ 1 ] + ( rect->h / 2.0f ) ) / rect->h ) * ( NEAR_ALPHA - FAR_ALPHA );

  localColour[ 3 ] *= alphaMod;
  localColour[ 3 ] *= ( 0.5f + ( timeFractionSinceRefresh * 0.5f ) );

  if( localColour[ 3 ] > 1.0f )
    localColour[ 3 ] = 1.0f;
  else if( localColour[ 3 ] < 0.0f )
    localColour[ 3 ] = 0.0f;

  trap_R_SetColor( localColour );

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

#define BLIPX2  (24.0f * cgDC.aspectScale)
#define BLIPY2  24.0f

/*
=============
CG_DrawDir

Draw dot marking the direction to an enemy
=============
*/
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

  BG_GetClientNormal( ps, normal );

  AngleVectors( entityPositions.vangles, view, NULL, NULL );

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

  VectorCopy( entityPositions.origin, origin );

  //draw human buildables
  for( i = 0; i < entityPositions.numHumanBuildables; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( entityPositions.humanBuildablePos[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < ALIENSENSE_RANGE )
      CG_DrawDir( rect, relOrigin, buildable );
  }

  //draw human clients
  for( i = 0; i < entityPositions.numHumanClients; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( entityPositions.humanClientPos[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < ALIENSENSE_RANGE )
      CG_DrawDir( rect, relOrigin, client );
  }
}

/*
=============
CG_Scanner
=============
*/
void CG_Scanner( rectDef_t *rect, qhandle_t shader, vec4_t color )
{
  int     i;
  vec3_t  origin;
  vec3_t  relOrigin;
  vec4_t  hIabove;
  vec4_t  hIbelow;
  vec4_t  aIabove = { 1.0f, 0.0f, 0.0f, 0.75f };
  vec4_t  aIbelow = { 1.0f, 0.0f, 0.0f, 0.5f };

  Vector4Copy( color, hIabove );
  hIabove[ 3 ] *= 1.5f;
  Vector4Copy( color, hIbelow );

  VectorCopy( entityPositions.origin, origin );

  //draw human buildables below scanner plane
  for( i = 0; i < entityPositions.numHumanBuildables; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( entityPositions.humanBuildablePos[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < HELMET_RANGE && ( relOrigin[ 2 ] < 0 ) )
      CG_DrawBlips( rect, relOrigin, hIbelow );
  }

  //draw alien buildables below scanner plane
  for( i = 0; i < entityPositions.numAlienBuildables; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( entityPositions.alienBuildablePos[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < HELMET_RANGE && ( relOrigin[ 2 ] < 0 ) )
      CG_DrawBlips( rect, relOrigin, aIbelow );
  }

  //draw human clients below scanner plane
  for( i = 0; i < entityPositions.numHumanClients; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( entityPositions.humanClientPos[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < HELMET_RANGE && ( relOrigin[ 2 ] < 0 ) )
      CG_DrawBlips( rect, relOrigin, hIbelow );
  }

  //draw alien buildables below scanner plane
  for( i = 0; i < entityPositions.numAlienClients; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( entityPositions.alienClientPos[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < HELMET_RANGE && ( relOrigin[ 2 ] < 0 ) )
      CG_DrawBlips( rect, relOrigin, aIbelow );
  }

  if( !cg_disableScannerPlane.integer )
  {
    trap_R_SetColor( color );
    CG_DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
    trap_R_SetColor( NULL );
  }

  //draw human buildables above scanner plane
  for( i = 0; i < entityPositions.numHumanBuildables; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( entityPositions.humanBuildablePos[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < HELMET_RANGE && ( relOrigin[ 2 ] > 0 ) )
      CG_DrawBlips( rect, relOrigin, hIabove );
  }

  //draw alien buildables above scanner plane
  for( i = 0; i < entityPositions.numAlienBuildables; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( entityPositions.alienBuildablePos[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < HELMET_RANGE && ( relOrigin[ 2 ] > 0 ) )
      CG_DrawBlips( rect, relOrigin, aIabove );
  }

  //draw human clients above scanner plane
  for( i = 0; i < entityPositions.numHumanClients; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( entityPositions.humanClientPos[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < HELMET_RANGE && ( relOrigin[ 2 ] > 0 ) )
      CG_DrawBlips( rect, relOrigin, hIabove );
  }

  //draw alien clients above scanner plane
  for( i = 0; i < entityPositions.numAlienClients; i++ )
  {
    VectorClear( relOrigin );
    VectorSubtract( entityPositions.alienClientPos[ i ], origin, relOrigin );

    if( VectorLength( relOrigin ) < HELMET_RANGE && ( relOrigin[ 2 ] > 0 ) )
      CG_DrawBlips( rect, relOrigin, aIabove );
  }
}

void THZ_DrawScanner( rectDef_t *rect )
{
    vec4_t colorB = { 0.0f, 0.0f, 0.0f, 0.5f };
    vec4_t color  = { 1.0f, 1.0f, 1.0f, 0.2f };

    vec4_t aliencolor = { 1.0f, 0.0f, 0.0f, 0.8f };
    vec4_t humancolor = { 0.0f, 0.0f, 1.0f, 0.8f };
    vec4_t buildcolor = { 0.0f, 1.0f, 1.0f, 0.8f };
    vec4_t buildcolor2 = { 1.0f, 1.0f, 0.0f, 0.8f };

    vec3_t drawOrigin = { 0.0f, 0.0f, 0.0f };
    vec3_t origin     = { 0.0f, 0.0f, 0.0f };
    vec3_t relOrigin  = { 0.0f, 0.0f, 0.0f };

    static vec3_t up  = { 0.0f, 0.0f, 1.0f };

    int i;

    if( !thz_radar.integer )
        return;

    //CG_FillRect( rect->x, rect->y, rect->w, rect->h, colorB );

    // Draw cross
    CG_FillRect( rect->x + (rect->w/2),
                 rect->y, 
                 1,
                 rect->h,
                 color );
    CG_FillRect( rect->x,
                 rect->y+(rect->h/2),
                 rect->w,
                 1,
                 color );

    // update the player positions
    CG_UpdateEntityPositions( );

    // blips
    VectorCopy( entityPositions.origin, origin );

     // human buildables
    for( i = 0; i < entityPositions.numHumanBuildables; i++ )
    {
        VectorClear( relOrigin );
        VectorSubtract( entityPositions.humanBuildablePos[ i ], origin, relOrigin );

        if( VectorLength( relOrigin ) < thz_radarrange.integer )
        {
        RotatePointAroundVector( drawOrigin, up, relOrigin, -entityPositions.vangles[ 1 ] - 90 );

        drawOrigin[ 0 ] /= ((float)(1.25f * (float)thz_radarrange.integer) / (float)rect->w);
        drawOrigin[ 1 ] /= ((float)(1.25f * (float)thz_radarrange.integer) / (float)rect->h);
        drawOrigin[ 2 ] /= ((float)(1.25f * (float)thz_radarrange.integer) / (float)rect->w);

        CG_FillRect( rect->x + (rect->w / 2) + -drawOrigin[0] - 5,
                     rect->y + (rect->h / 2) + drawOrigin[1] - 5,
                     10, 10, buildcolor );
        }
    }

    // humans
    for( i = 0; i < entityPositions.numHumanClients; i++ )
    {
        VectorClear( relOrigin );
        VectorSubtract( entityPositions.humanClientPos[ i ], origin, relOrigin );

        if( VectorLength( relOrigin ) < thz_radarrange.integer )
        {
        RotatePointAroundVector( drawOrigin, up, relOrigin, -entityPositions.vangles[ 1 ] - 90 );

        drawOrigin[ 0 ] /= ((float)(1.25f * (float)thz_radarrange.integer) / (float)rect->w);
        drawOrigin[ 1 ] /= ((float)(1.25f * (float)thz_radarrange.integer) / (float)rect->h);
        drawOrigin[ 2 ] /= ((float)(1.25f * (float)thz_radarrange.integer) / (float)rect->w);

        CG_FillRect( rect->x + (rect->w / 2) + -drawOrigin[0] -3,
                     rect->y + (rect->h / 2) + drawOrigin[1] -3,
                     6, 6, humancolor );
        }
    }

    // alien structures
    for( i = 0; i < entityPositions.numAlienBuildables; i++ )
    {
        VectorClear( relOrigin );
        VectorSubtract( entityPositions.alienBuildablePos[ i ], origin, relOrigin );

        if( VectorLength( relOrigin ) < thz_radarrange.integer )
        {
        RotatePointAroundVector( drawOrigin, up, relOrigin, -entityPositions.vangles[ 1 ] - 90 );

        drawOrigin[ 0 ] /= ((float)(1.25f * (float)thz_radarrange.integer) / (float)rect->w);
        drawOrigin[ 1 ] /= ((float)(1.25f * (float)thz_radarrange.integer) / (float)rect->h);
        drawOrigin[ 2 ] /= ((float)(1.25f * (float)thz_radarrange.integer) / (float)rect->w);

        CG_FillRect( rect->x + (rect->w / 2) + -drawOrigin[0] - 5,
                     rect->y + (rect->h / 2) + drawOrigin[1] - 5,
                     10, 10, buildcolor2 );
        }
    }

    // aliens
    for( i = 0; i < entityPositions.numAlienClients; i++ )
    {
        VectorClear( relOrigin );
        VectorSubtract( entityPositions.alienClientPos[ i ], origin, relOrigin );

        if( VectorLength( relOrigin ) < thz_radarrange.integer )
        {
        RotatePointAroundVector( drawOrigin, up, relOrigin, -entityPositions.vangles[ 1 ] - 90 );

        drawOrigin[ 0 ] /= ((float)(1.25f * (float)thz_radarrange.integer) / (float)rect->w);
        drawOrigin[ 1 ] /= ((float)(1.25f * (float)thz_radarrange.integer) / (float)rect->h);
        drawOrigin[ 2 ] /= ((float)(1.25f * (float)thz_radarrange.integer) / (float)rect->w);

        CG_FillRect( rect->x + (rect->w / 2) + -drawOrigin[0] -3,
                     rect->y + (rect->h / 2) + drawOrigin[1] -3,
                     6, 6, aliencolor );
        }
    }
}


