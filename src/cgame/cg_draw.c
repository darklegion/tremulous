/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2006 Tim Angus

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

// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay


#include "cg_local.h"
#include "../ui/ui_shared.h"

menuDef_t *menuScoreboard = NULL;

int drawTeamOverlayModificationCount = -1;

int   sortedTeamPlayers[ TEAM_MAXOVERLAY ];
int   numSortedTeamPlayers;

static void CG_AlignText( rectDef_t *rect, const char *text, float scale,
                          float w, float h,
                          int align, int valign,
                          float *x, float *y )
{
  float tx, ty;

  if( scale > 0.0f )
  {
    w = UI_Text_Width( text, scale, 0 );
    h = UI_Text_Height( text, scale, 0 );
  }

  switch( align )
  {
    default:
    case ALIGN_LEFT:
      tx = 0.0f;
      break;

    case ALIGN_RIGHT:
      tx = rect->w - w;
      break;

    case ALIGN_CENTER:
      tx = ( rect->w - w ) / 2.0f;
      break;
  }

  switch( valign )
  {
    default:
    case VALIGN_BOTTOM:
      ty = rect->h;
      break;

    case VALIGN_TOP:
      ty = h;
      break;

    case VALIGN_CENTER:
      ty = h + ( ( rect->h - h ) / 2.0f );
      break;
  }

  if( x )
    *x = rect->x + tx;

  if( y )
    *y = rect->y + ty;
}

/*
==============
CG_DrawFieldPadded

Draws large numbers for status bar
==============
*/
static void CG_DrawFieldPadded( int x, int y, int width, int cw, int ch, int value )
{
  char  num[ 16 ], *ptr;
  int   l, orgL;
  int   frame;
  int   charWidth, charHeight;

  if( !( charWidth = cw ) )
    charWidth = CHAR_WIDTH;

  if( !( charHeight = ch ) )
    charHeight = CHAR_HEIGHT;

  if( width < 1 )
    return;

  // draw number string
  if( width > 4 )
    width = 4;

  switch( width )
  {
    case 1:
      value = value > 9 ? 9 : value;
      value = value < 0 ? 0 : value;
      break;
    case 2:
      value = value > 99 ? 99 : value;
      value = value < -9 ? -9 : value;
      break;
    case 3:
      value = value > 999 ? 999 : value;
      value = value < -99 ? -99 : value;
      break;
    case 4:
      value = value > 9999 ? 9999 : value;
      value = value < -999 ? -999 : value;
      break;
  }

  Com_sprintf( num, sizeof( num ), "%d", value );
  l = strlen( num );

  if( l > width )
    l = width;

  orgL = l;

  x += ( 2.0f * cgDC.aspectScale );

  ptr = num;
  while( *ptr && l )
  {
    if( width > orgL )
    {
      CG_DrawPic( x,y, charWidth, charHeight, cgs.media.numberShaders[ 0 ] );
      width--;
      x += charWidth;
      continue;
    }

    if( *ptr == '-' )
      frame = STAT_MINUS;
    else
      frame = *ptr - '0';

    CG_DrawPic( x,y, charWidth, charHeight, cgs.media.numberShaders[ frame ] );
    x += charWidth;
    ptr++;
    l--;
  }
}

/*
==============
CG_DrawField

Draws large numbers for status bar
==============
*/
void CG_DrawField( float x, float y, int width, float cw, float ch, int value )
{
  char  num[ 16 ], *ptr;
  int   l;
  int   frame;
  float charWidth, charHeight;

  if( !( charWidth = cw ) )
    charWidth = CHAR_WIDTH;

  if( !( charHeight = ch ) )
    charHeight = CHAR_HEIGHT;

  if( width < 1 )
    return;

  // draw number string
  if( width > 4 )
    width = 4;

  switch( width )
  {
    case 1:
      value = value > 9 ? 9 : value;
      value = value < 0 ? 0 : value;
      break;
    case 2:
      value = value > 99 ? 99 : value;
      value = value < -9 ? -9 : value;
      break;
    case 3:
      value = value > 999 ? 999 : value;
      value = value < -99 ? -99 : value;
      break;
    case 4:
      value = value > 9999 ? 9999 : value;
      value = value < -999 ? -999 : value;
      break;
  }

  Com_sprintf( num, sizeof( num ), "%d", value );
  l = strlen( num );

  if( l > width )
    l = width;

  x += ( 2.0f * cgDC.aspectScale ) + charWidth * ( width - l );

  ptr = num;
  while( *ptr && l )
  {
    if( *ptr == '-' )
      frame = STAT_MINUS;
    else
      frame = *ptr -'0';

    CG_DrawPic( x,y, charWidth, charHeight, cgs.media.numberShaders[ frame ] );
    x += charWidth;
    ptr++;
    l--;
  }
}

static void CG_DrawProgressBar( rectDef_t *rect, vec4_t color, float scale,
                                int align, int textalign, int textStyle,
                                int special, float progress )
{
  float   rimWidth = rect->h / 20.0f;
  float   doneWidth, leftWidth;
  float   tx, ty;
  char    textBuffer[ 8 ];

  if( rimWidth < 0.6f )
    rimWidth = 0.6f;

  if( special >= 0.0f )
    rimWidth = special;

  if( progress < 0.0f )
    progress = 0.0f;
  else if( progress > 1.0f )
    progress = 1.0f;

  doneWidth = ( rect->w - 2 * rimWidth ) * progress;
  leftWidth = ( rect->w - 2 * rimWidth ) - doneWidth;

  trap_R_SetColor( color );

  //draw rim and bar
  if( align == ALIGN_RIGHT )
  {
    CG_DrawPic( rect->x, rect->y, rimWidth, rect->h, cgs.media.whiteShader );
    CG_DrawPic( rect->x + rimWidth, rect->y,
      leftWidth, rimWidth, cgs.media.whiteShader );
    CG_DrawPic( rect->x + rimWidth, rect->y + rect->h - rimWidth,
      leftWidth, rimWidth, cgs.media.whiteShader );
    CG_DrawPic( rect->x + rimWidth + leftWidth, rect->y,
      rimWidth + doneWidth, rect->h, cgs.media.whiteShader );
  }
  else
  {
    CG_DrawPic( rect->x, rect->y, rimWidth + doneWidth, rect->h, cgs.media.whiteShader );
    CG_DrawPic( rimWidth + rect->x + doneWidth, rect->y,
      leftWidth, rimWidth, cgs.media.whiteShader );
    CG_DrawPic( rimWidth + rect->x + doneWidth, rect->y + rect->h - rimWidth,
      leftWidth, rimWidth, cgs.media.whiteShader );
    CG_DrawPic( rect->x + rect->w - rimWidth, rect->y, rimWidth, rect->h, cgs.media.whiteShader );
  }

  trap_R_SetColor( NULL );

  //draw text
  if( scale > 0.0 )
  {
    Com_sprintf( textBuffer, sizeof( textBuffer ), "%d%%", (int)( progress * 100 ) );
    CG_AlignText( rect, textBuffer, scale, 0.0f, 0.0f, textalign, VALIGN_CENTER, &tx, &ty );

    UI_Text_Paint( tx, ty, scale, color, textBuffer, 0, 0, textStyle );
  }
}

//=============== TA: was cg_newdraw.c

#define NO_CREDITS_TIME 2000

static void CG_DrawPlayerCreditsValue( rectDef_t *rect, vec4_t color, qboolean padding )
{
  int           value;
  playerState_t *ps;
  centity_t     *cent;

  cent = &cg_entities[ cg.snap->ps.clientNum ];
  ps = &cg.snap->ps;

  //if the build timer pie is showing don't show this
  if( ( cent->currentState.weapon == WP_ABUILD ||
      cent->currentState.weapon == WP_ABUILD2 ) && ps->stats[ STAT_MISC ] )
    return;

  value = ps->persistant[ PERS_CREDIT ];
  if( value > -1 )
  {
    if( cg.predictedPlayerState.stats[ STAT_TEAM ] == TEAM_ALIENS )
    {
      if( !BG_AlienCanEvolve( cg.predictedPlayerState.stats[ STAT_CLASS ],
                              value, cgs.alienStage ) &&
          cg.time - cg.lastEvolveAttempt <= NO_CREDITS_TIME &&
          ( ( cg.time - cg.lastEvolveAttempt ) / 300 ) & 1 )
      {
        color[ 3 ] = 0.0f;
      }

      value /= ALIEN_CREDITS_PER_FRAG;
    }

    trap_R_SetColor( color );

    if( padding )
      CG_DrawFieldPadded( rect->x, rect->y, 4, rect->w / 4, rect->h, value );
    else
      CG_DrawField( rect->x, rect->y, 1, rect->w, rect->h, value );

    trap_R_SetColor( NULL );
  }
}

static void CG_DrawPlayerBankValue( rectDef_t *rect, vec4_t color, qboolean padding )
{
  int           value;
  playerState_t *ps;

  ps = &cg.snap->ps;

  value = ps->persistant[ PERS_BANK ];
  if( value > -1 )
  {
    trap_R_SetColor( color );

    if( padding )
      CG_DrawFieldPadded( rect->x, rect->y, 4, rect->w / 4, rect->h, value );
    else
      CG_DrawField( rect->x, rect->y, 1, rect->w, rect->h, value );

    trap_R_SetColor( NULL );
  }
}

#define HH_MIN_ALPHA  0.2f
#define HH_MAX_ALPHA  0.8f
#define HH_ALPHA_DIFF (HH_MAX_ALPHA-HH_MIN_ALPHA)

#define AH_MIN_ALPHA  0.2f
#define AH_MAX_ALPHA  0.8f
#define AH_ALPHA_DIFF (AH_MAX_ALPHA-AH_MIN_ALPHA)

/*
==============
CG_DrawPlayerStamina1
==============
*/
static void CG_DrawPlayerStamina1( rectDef_t *rect, vec4_t color, qhandle_t shader )
{
  playerState_t *ps = &cg.snap->ps;
  float         stamina = ps->stats[ STAT_STAMINA ];
  float         maxStaminaBy3 = (float)MAX_STAMINA / 3.0f;
  float         progress;

  stamina -= ( 2 * (int)maxStaminaBy3 );
  progress = stamina / maxStaminaBy3;

  if( progress > 1.0f )
    progress  = 1.0f;
  else if( progress < 0.0f )
    progress = 0.0f;

  color[ 3 ] = HH_MIN_ALPHA + ( progress * HH_ALPHA_DIFF );

  trap_R_SetColor( color );
  CG_DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
  trap_R_SetColor( NULL );
}

/*
==============
CG_DrawPlayerStamina2
==============
*/
static void CG_DrawPlayerStamina2( rectDef_t *rect, vec4_t color, qhandle_t shader )
{
  playerState_t *ps = &cg.snap->ps;
  float         stamina = ps->stats[ STAT_STAMINA ];
  float         maxStaminaBy3 = (float)MAX_STAMINA / 3.0f;
  float         progress;

  stamina -= (int)maxStaminaBy3;
  progress = stamina / maxStaminaBy3;

  if( progress > 1.0f )
    progress  = 1.0f;
  else if( progress < 0.0f )
    progress = 0.0f;

  color[ 3 ] = HH_MIN_ALPHA + ( progress * HH_ALPHA_DIFF );

  trap_R_SetColor( color );
  CG_DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
  trap_R_SetColor( NULL );
}

/*
==============
CG_DrawPlayerStamina3
==============
*/
static void CG_DrawPlayerStamina3( rectDef_t *rect, vec4_t color, qhandle_t shader )
{
  playerState_t *ps = &cg.snap->ps;
  float         stamina = ps->stats[ STAT_STAMINA ];
  float         maxStaminaBy3 = (float)MAX_STAMINA / 3.0f;
  float         progress;

  progress = stamina / maxStaminaBy3;

  if( progress > 1.0f )
    progress  = 1.0f;
  else if( progress < 0.0f )
    progress = 0.0f;

  color[ 3 ] = HH_MIN_ALPHA + ( progress * HH_ALPHA_DIFF );

  trap_R_SetColor( color );
  CG_DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
  trap_R_SetColor( NULL );
}

/*
==============
CG_DrawPlayerStamina4
==============
*/
static void CG_DrawPlayerStamina4( rectDef_t *rect, vec4_t color, qhandle_t shader )
{
  playerState_t *ps = &cg.snap->ps;
  float         stamina = ps->stats[ STAT_STAMINA ];
  float         progress;

  stamina += (float)MAX_STAMINA;
  progress = stamina / (float)MAX_STAMINA;

  if( progress > 1.0f )
    progress  = 1.0f;
  else if( progress < 0.0f )
    progress = 0.0f;

  color[ 3 ] = HH_MIN_ALPHA + ( progress * HH_ALPHA_DIFF );

  trap_R_SetColor( color );
  CG_DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
  trap_R_SetColor( NULL );
}

/*
==============
CG_DrawPlayerStaminaBolt
==============
*/
static void CG_DrawPlayerStaminaBolt( rectDef_t *rect, vec4_t color, qhandle_t shader )
{
  playerState_t *ps = &cg.snap->ps;
  float         stamina = ps->stats[ STAT_STAMINA ];

  if( stamina < 0 )
    color[ 3 ] = HH_MIN_ALPHA;
  else if( cg.predictedPlayerState.stats[ STAT_STATE ] & SS_SPEEDBOOST )
    color[ 3 ] = HH_MIN_ALPHA + HH_MAX_ALPHA *
                 ( 0.5f + sin( cg.time / 200.0f ) / 2 );
  else
    color[ 3 ] = HH_MAX_ALPHA;

  trap_R_SetColor( color );
  CG_DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
  trap_R_SetColor( NULL );
}

/*
==============
CG_DrawPlayerClipsRing
==============
*/
static void CG_DrawPlayerClipsRing( rectDef_t *rect, vec4_t color, qhandle_t shader )
{
  playerState_t *ps = &cg.snap->ps;
  centity_t     *cent;
  float         buildTime = ps->stats[ STAT_MISC ];
  float         progress;
  float         maxDelay;
  weapon_t      weapon;

  cent = &cg_entities[ cg.snap->ps.clientNum ];
  weapon = BG_GetPlayerWeapon( ps );

  switch( weapon )
  {
    case WP_ABUILD:
    case WP_ABUILD2:
    case WP_HBUILD:
      if( buildTime > MAXIMUM_BUILD_TIME )
        buildTime = MAXIMUM_BUILD_TIME;
      progress = ( MAXIMUM_BUILD_TIME - buildTime ) / MAXIMUM_BUILD_TIME;

      color[ 3 ] = HH_MIN_ALPHA + ( progress * HH_ALPHA_DIFF );
      break;

    default:
      if( ps->weaponstate == WEAPON_RELOADING )
      {
        maxDelay = (float)BG_Weapon( cent->currentState.weapon )->reloadTime;
        progress = ( maxDelay - (float)ps->weaponTime ) / maxDelay;

        color[ 3 ] = HH_MIN_ALPHA + ( progress * HH_ALPHA_DIFF );
      }
      break;
  }

  trap_R_SetColor( color );
  CG_DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
  trap_R_SetColor( NULL );
}

/*
==============
CG_DrawPlayerBuildTimerRing
==============
*/
static void CG_DrawPlayerBuildTimerRing( rectDef_t *rect, vec4_t color, qhandle_t shader )
{
  playerState_t *ps = &cg.snap->ps;
  centity_t     *cent;
  float         buildTime = ps->stats[ STAT_MISC ];
  float         progress;

  cent = &cg_entities[ cg.snap->ps.clientNum ];

  if( buildTime > MAXIMUM_BUILD_TIME )
    buildTime = MAXIMUM_BUILD_TIME;

  progress = ( MAXIMUM_BUILD_TIME - buildTime ) / MAXIMUM_BUILD_TIME;

  color[ 3 ] = AH_MIN_ALPHA + ( progress * AH_ALPHA_DIFF );

  trap_R_SetColor( color );
  CG_DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
  trap_R_SetColor( NULL );
}

/*
==============
CG_DrawPlayerBoosted
==============
*/
static void CG_DrawPlayerBoosted( rectDef_t *rect, vec4_t color, qhandle_t shader )
{
  if( cg.snap->ps.stats[ STAT_STATE ] & SS_BOOSTED )
    color[ 3 ] = AH_MAX_ALPHA;
  else
    color[ 3 ] = AH_MIN_ALPHA;

  trap_R_SetColor( color );
  CG_DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
  trap_R_SetColor( NULL );
}

/*
==============
CG_DrawPlayerBoosterBolt
==============
*/
static void CG_DrawPlayerBoosterBolt( rectDef_t *rect, vec4_t color, qhandle_t shader )
{
  vec4_t localColor;

  Vector4Copy( color, localColor );

  // Flash bolts when the boost is almost out
  if( ( cg.snap->ps.stats[ STAT_STATE ] & SS_BOOSTED ) &&
      ( cg.snap->ps.stats[ STAT_STATE ] & SS_BOOSTEDWARNING ) )
    localColor[ 3 ] += ( 1.0f - localColor[ 3 ] ) *
                       ( 0.5f + sin( cg.time / 100.0f ) / 2 );

  trap_R_SetColor( localColor );
  CG_DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
  trap_R_SetColor( NULL );
}

/*
==============
CG_DrawPlayerPoisonBarbs
==============
*/
static void CG_DrawPlayerPoisonBarbs( rectDef_t *rect, vec4_t color, qhandle_t shader )
{
  playerState_t *ps = &cg.snap->ps;
  int           x = rect->x;
  int           y = rect->y;
  int           width = rect->w;
  int           height = rect->h;
  qboolean      vertical;
  int           iconsize, numBarbs, i;

  numBarbs = ps->ammo;

  if( height > width )
  {
    vertical = qtrue;
    iconsize = width;
  }
  else if( height <= width )
  {
    vertical = qfalse;
    iconsize = height * cgDC.aspectScale;
  }

  if( color[ 3 ] != 0.0 )
    trap_R_SetColor( color );

  for( i = 0; i < numBarbs; i ++ )
  {
    if( vertical )
      y += iconsize;
    else
      x += iconsize;

    CG_DrawPic( x, y, iconsize, iconsize, shader );
  }

  trap_R_SetColor( NULL );
}

/*
==============
CG_DrawPlayerWallclimbing
==============
*/
static void CG_DrawPlayerWallclimbing( rectDef_t *rect, vec4_t color, qhandle_t shader )
{
  playerState_t *ps = &cg.snap->ps;
  qboolean      ww = ps->stats[ STAT_STATE ] & SS_WALLCLIMBING;

  if( ww )
    color[ 3 ] = AH_MAX_ALPHA;
  else
    color[ 3 ] = AH_MIN_ALPHA;

  trap_R_SetColor( color );
  CG_DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
  trap_R_SetColor( NULL );
}

static void CG_DrawPlayerAmmoValue( rectDef_t *rect, vec4_t color )
{
  int           value;
  centity_t     *cent;
  playerState_t *ps;
  weapon_t      weapon;

  cent = &cg_entities[ cg.snap->ps.clientNum ];
  ps = &cg.snap->ps;
  weapon = BG_GetPlayerWeapon( ps );

  if( weapon )
  {
    switch( weapon )
    {
      case WP_ABUILD:
      case WP_ABUILD2:
        value = cgs.alienBuildPoints;
        break;

      case WP_HBUILD:
        value = cgs.humanBuildPoints;
        break;

      default:
        value = ps->ammo;
        break;
    }

    if( value > 999 )
      value = 999;

    if( value > -1 )
    {
      trap_R_SetColor( color );
      CG_DrawField( rect->x, rect->y, 4, rect->w / 4, rect->h, value );
      trap_R_SetColor( NULL );
    }
  }
}


/*
==============
CG_DrawAlienSense
==============
*/
static void CG_DrawAlienSense( rectDef_t *rect )
{
  if( BG_ClassHasAbility( cg.snap->ps.stats[ STAT_CLASS ], SCA_ALIENSENSE ) )
    CG_AlienSense( rect );
}


/*
==============
CG_DrawHumanScanner
==============
*/
static void CG_DrawHumanScanner( rectDef_t *rect, qhandle_t shader, vec4_t color )
{
  if( BG_InventoryContainsUpgrade( UP_HELMET, cg.snap->ps.stats ) )
    CG_Scanner( rect, shader, color );
}


/*
==============
CG_DrawUsableBuildable
==============
*/
static void CG_DrawUsableBuildable( rectDef_t *rect, qhandle_t shader, vec4_t color )
{
  vec3_t        view, point;
  trace_t       trace;
  entityState_t *es;

  AngleVectors( cg.refdefViewAngles, view, NULL, NULL );
  VectorMA( cg.refdef.vieworg, 64, view, point );
  CG_Trace( &trace, cg.refdef.vieworg, NULL, NULL,
            point, cg.predictedPlayerState.clientNum, MASK_SHOT );

  es = &cg_entities[ trace.entityNum ].currentState;

  if( es->eType == ET_BUILDABLE && BG_Buildable( es->modelindex )->usable &&
      cg.predictedPlayerState.stats[ STAT_TEAM ] == BG_Buildable( es->modelindex )->team )
  {
    //hack to prevent showing the usable buildable when you aren't carrying an energy weapon
    if( ( es->modelindex == BA_H_REACTOR || es->modelindex == BA_H_REPEATER ) &&
        ( !BG_Weapon( cg.snap->ps.weapon )->usesEnergy ||
          BG_Weapon( cg.snap->ps.weapon )->infiniteAmmo ) )
      return;

    trap_R_SetColor( color );
    CG_DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
    trap_R_SetColor( NULL );
  }
}


#define BUILD_DELAY_TIME  2000

static void CG_DrawPlayerBuildTimer( rectDef_t *rect, vec4_t color )
{
  int           index;
  centity_t     *cent;
  playerState_t *ps;
  weapon_t      weapon;

  cent = &cg_entities[ cg.snap->ps.clientNum ];
  ps = &cg.snap->ps;
  weapon = BG_GetPlayerWeapon( ps );

  if( ( weapon != WP_ABUILD && weapon != WP_ABUILD2 && weapon != WP_HBUILD ) ||
      ps->stats[ STAT_MISC ] <= 0 )
    return;

  index = 8 * ( ps->stats[ STAT_MISC ] - 1 ) / MAXIMUM_BUILD_TIME;
  if( index > 7 )
    index = 7;
  else if( index < 0 )
    index = 0;

  if( cg.time - cg.lastBuildAttempt <= BUILD_DELAY_TIME &&
      ( ( cg.time - cg.lastBuildAttempt ) / 300 ) % 2 )
  {
    color[ 0 ] = 1.0f;
    color[ 1 ] = color[ 2 ] = 0.0f;
    color[ 3 ] = 1.0f;
  }

  trap_R_SetColor( color );
  CG_DrawPic( rect->x, rect->y, rect->w, rect->h,
              cgs.media.buildWeaponTimerPie[ index ] );
  trap_R_SetColor( NULL );
}

static void CG_DrawPlayerClipsValue( rectDef_t *rect, vec4_t color )
{
  int           value;
  centity_t     *cent;
  playerState_t *ps;
  weapon_t      weapon;

  cent = &cg_entities[ cg.snap->ps.clientNum ];
  ps = &cg.snap->ps;
  weapon = BG_GetPlayerWeapon( ps );

  switch( weapon )
  {
    case WP_ABUILD:
    case WP_ABUILD2:
    case WP_HBUILD:
    case 0:
      break;

    default:
      value = ps->clips;

      if( value > -1 )
      {
        trap_R_SetColor( color );
        CG_DrawField( rect->x, rect->y, 4, rect->w / 4, rect->h, value );
        trap_R_SetColor( NULL );
      }
      break;
  }
}

static void CG_DrawPlayerHealthValue( rectDef_t *rect, vec4_t color )
{
  playerState_t *ps;
  int value;

  ps = &cg.snap->ps;

  value = ps->stats[ STAT_HEALTH ];

  trap_R_SetColor( color );
  CG_DrawField( rect->x, rect->y, 4, rect->w / 4, rect->h, value );
  trap_R_SetColor( NULL );
}

/*
==============
CG_DrawPlayerHealthCross
==============
*/
static void CG_DrawPlayerHealthCross( rectDef_t *rect, vec4_t ref_color )
{
  qhandle_t shader;
  vec4_t color;
  float ref_alpha;
  
  // Pick the current icon
  shader = cgs.media.healthCross;
  if( cg.snap->ps.stats[ STAT_STATE ] & SS_HEALING_3X )
    shader = cgs.media.healthCross3X;
  else if( cg.snap->ps.stats[ STAT_STATE ] & SS_HEALING_2X )
  {
    if( cg.snap->ps.stats[ STAT_TEAM ] == TEAM_ALIENS )
      shader = cgs.media.healthCross2X;
    else
      shader = cgs.media.healthCrossMedkit;
  }
  else if( cg.snap->ps.stats[ STAT_STATE ] & SS_POISONED )
    shader = cgs.media.healthCrossPoisoned;

  // Pick the alpha value
  Vector4Copy( ref_color, color );
  if( cg.snap->ps.stats[ STAT_TEAM ] == TEAM_HUMANS &&
      cg.snap->ps.stats[ STAT_HEALTH ] < 10 )
  {
    color[ 0 ] = 1.0f;
    color[ 1 ] = color[ 2 ] = 0.0f;
  }
  ref_alpha = ref_color[ 3 ];
  if( cg.snap->ps.stats[ STAT_STATE ] & SS_HEALING_ACTIVE )
    ref_alpha = 1.0f;
    
  // Don't fade from nothing
  if( !cg.lastHealthCross )
    cg.lastHealthCross = shader;
  
  // Fade the icon during transition
  if( cg.lastHealthCross != shader )
  {
    cg.healthCrossFade += cg.frametime / 500.f;
    if( cg.healthCrossFade > 1.f )
    {
      cg.healthCrossFade = 0.f;
      cg.lastHealthCross = shader;
    }
    else
    {
      // Fading between two icons
      color[ 3 ] = ref_alpha * cg.healthCrossFade;
      trap_R_SetColor( color );
      CG_DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
      color[ 3 ] = ref_alpha * ( 1.f - cg.healthCrossFade );
      trap_R_SetColor( color );
      CG_DrawPic( rect->x, rect->y, rect->w, rect->h, cg.lastHealthCross );
      trap_R_SetColor( NULL );
      return;
    }
  }

  // Not fading, draw a single icon
  color[ 3 ] = ref_alpha;
  trap_R_SetColor( color );
  CG_DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
  trap_R_SetColor( NULL );
}

static float CG_ChargeProgress( void )
{
  float progress;
  int min = 0, max = 0;

  if( cg.snap->ps.weapon == WP_ALEVEL3 )
  {
    min = LEVEL3_POUNCE_TIME_MIN;
    max = LEVEL3_POUNCE_TIME;
  }
  else if( cg.snap->ps.weapon == WP_ALEVEL3_UPG )
  {
    min = LEVEL3_POUNCE_TIME_MIN;
    max = LEVEL3_POUNCE_TIME_UPG;
  }
  else if( cg.snap->ps.weapon == WP_ALEVEL4 )
  {
    if( cg.predictedPlayerState.stats[ STAT_STATE ] & SS_CHARGING )
    {
      min = 0;
      max = LEVEL4_TRAMPLE_DURATION;
    }
    else
    {
      min = LEVEL4_TRAMPLE_CHARGE_MIN;
      max = LEVEL4_TRAMPLE_CHARGE_MAX;
    }
  }
  else if( cg.snap->ps.weapon == WP_LUCIFER_CANNON )
  {
    min = LCANNON_CHARGE_TIME_MIN;
    max = LCANNON_CHARGE_TIME_MAX;
  }

  if( max - min <= 0.0f )
    return 0.0f;

  progress = ( (float)cg.predictedPlayerState.stats[ STAT_MISC ] - min ) /
             ( max - min );

  if( progress > 1.0f )
    return 1.0f;

  if( progress < 0.0f )
    return 0.0f;

  return progress;
}

#define CHARGE_BAR_FADE_RATE 0.002f

static void CG_DrawPlayerChargeBarBG( rectDef_t *rect, vec4_t ref_color,
                                      qhandle_t shader )
{
  vec4_t color;
  
  if( !cg_drawChargeBar.integer || cg.chargeMeterAlpha <= 0.0f )
    return;

  color[ 0 ] = ref_color[ 0 ];
  color[ 1 ] = ref_color[ 1 ];
  color[ 2 ] = ref_color[ 2 ];
  color[ 3 ] = ref_color[ 3 ] * cg.chargeMeterAlpha;

  // Draw meter background
  trap_R_SetColor( color );
  CG_DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
  trap_R_SetColor( NULL );
}

// FIXME: This should come from the element info
#define CHARGE_BAR_CAP_SIZE 3

static void CG_DrawPlayerChargeBar( rectDef_t *rect, vec4_t ref_color,
                                    qhandle_t shader )
{
  vec4_t color;
  float x, y, width, height, cap_size, progress;
  
  if( !cg_drawChargeBar.integer )
    return;
  
  // Get progress proportion and pump fade
  progress = CG_ChargeProgress();
  if( progress <= 0.0f )
  {
    cg.chargeMeterAlpha -= CHARGE_BAR_FADE_RATE * cg.frametime;
    if( cg.chargeMeterAlpha <= 0.0f )
    {
      cg.chargeMeterAlpha = 0.0f;
      return;
    }
  }
  else
  {
    cg.chargeMeterValue = progress;
    cg.chargeMeterAlpha += CHARGE_BAR_FADE_RATE * cg.frametime;
    if( cg.chargeMeterAlpha > 1.0f )
      cg.chargeMeterAlpha = 1.0f;
  }

  color[ 0 ] = ref_color[ 0 ];
  color[ 1 ] = ref_color[ 1 ];
  color[ 2 ] = ref_color[ 2 ];
  color[ 3 ] = ref_color[ 3 ] * cg.chargeMeterAlpha;
  
  // Flash red for Lucifer Cannon warning
  if( cg.snap->ps.weapon == WP_LUCIFER_CANNON &&
      cg.snap->ps.stats[ STAT_MISC ] >= LCANNON_CHARGE_TIME_WARN &&
      ( cg.time & 128 ) )
  {
    color[ 0 ] = 1.0f;
    color[ 1 ] = 0.0f;
    color[ 2 ] = 0.0f;
  }

  x = rect->x;
  y = rect->y;
  
  // Horizontal charge bar
  if( rect->w >= rect->h )
  {
    width = ( rect->w - CHARGE_BAR_CAP_SIZE * 2 ) * cg.chargeMeterValue;
    height = rect->h;
    CG_AdjustFrom640( &x, &y, &width, &height );
    cap_size = CHARGE_BAR_CAP_SIZE * cgs.screenXScale;
  
    // Draw the meter
    trap_R_SetColor( color );
    trap_R_DrawStretchPic( x, y, cap_size, height, 0, 0, 1, 1, shader );
    trap_R_DrawStretchPic( x + width + cap_size, y, cap_size, height,
                           1, 0, 0, 1, shader );
    trap_R_DrawStretchPic( x + cap_size, y, width, height, 1, 0, 1, 1, shader );
    trap_R_SetColor( NULL );
  }
  
  // Vertical charge bar
  else
  {
    y += rect->h;
    width = rect->w;
    height = ( rect->h - CHARGE_BAR_CAP_SIZE * 2 ) * cg.chargeMeterValue;
    CG_AdjustFrom640( &x, &y, &width, &height );
    cap_size = CHARGE_BAR_CAP_SIZE * cgs.screenYScale;
  
    // Draw the meter
    trap_R_SetColor( color );
    trap_R_DrawStretchPic( x, y - cap_size, width, cap_size,
                           0, 1, 1, 0, shader );
    trap_R_DrawStretchPic( x, y - height - cap_size * 2, width,
                           cap_size, 0, 0, 1, 1, shader );
    trap_R_DrawStretchPic( x, y - height - cap_size, width, height,
                           0, 1, 1, 1, shader );
    trap_R_SetColor( NULL );
  }
}

static void CG_DrawProgressLabel( rectDef_t *rect, float text_x, float text_y, vec4_t color,
                                  float scale, int textalign, int textvalign,
                                  const char *s, float fraction )
{
  vec4_t white = { 1.0f, 1.0f, 1.0f, 1.0f };
  float tx, ty;

  CG_AlignText( rect, s, scale, 0.0f, 0.0f, textalign, textvalign, &tx, &ty );

  if( fraction < 1.0f )
    UI_Text_Paint( text_x + tx, text_y + ty, scale, white,
      s, 0, 0, ITEM_TEXTSTYLE_NORMAL );
  else
    UI_Text_Paint( text_x + tx, text_y + ty, scale, color,
      s, 0, 0, ITEM_TEXTSTYLE_NEON );
}

static void CG_DrawMediaProgress( rectDef_t *rect, vec4_t color, float scale,
                                  int align, int textalign, int textStyle, int special )
{
  CG_DrawProgressBar( rect, color, scale, align, textalign, textStyle, special, cg.mediaFraction );
}

static void CG_DrawMediaProgressLabel( rectDef_t *rect, float text_x, float text_y,
                                       vec4_t color, float scale, int textalign, int textvalign )
{
  CG_DrawProgressLabel( rect, text_x, text_y, color, scale, textalign, textvalign,
                        "Map and Textures", cg.mediaFraction );
}

static void CG_DrawBuildablesProgress( rectDef_t *rect, vec4_t color, float scale,
                                       int align, int textalign, int textStyle, int special )
{
  CG_DrawProgressBar( rect, color, scale, align, textalign, textStyle, special, cg.buildablesFraction );
}

static void CG_DrawBuildablesProgressLabel( rectDef_t *rect, float text_x, float text_y,
                                            vec4_t color, float scale, int textalign, int textvalign )
{
  CG_DrawProgressLabel( rect, text_x, text_y, color, scale, textalign, textvalign,
                        "Buildable Models", cg.buildablesFraction );
}

static void CG_DrawCharModelProgress( rectDef_t *rect, vec4_t color, float scale,
                                      int align, int textalign, int textStyle, int special )
{
  CG_DrawProgressBar( rect, color, scale, align, textalign, textStyle, special, cg.charModelFraction );
}

static void CG_DrawCharModelProgressLabel( rectDef_t *rect, float text_x, float text_y,
                                           vec4_t color, float scale, int textalign, int textvalign )
{
  CG_DrawProgressLabel( rect, text_x, text_y, color, scale, textalign, textvalign,
                        "Character Models", cg.charModelFraction );
}

static void CG_DrawOverallProgress( rectDef_t *rect, vec4_t color, float scale,
                                    int align, int textalign, int textStyle, int special )
{
  float total;

  total = ( cg.charModelFraction + cg.buildablesFraction + cg.mediaFraction ) / 3.0f;
  CG_DrawProgressBar( rect, color, scale, align, textalign, textStyle, special, total );
}

static void CG_DrawLevelShot( rectDef_t *rect )
{
  const char  *s;
  const char  *info;
  qhandle_t   levelshot;
  qhandle_t   detail;

  info = CG_ConfigString( CS_SERVERINFO );
  s = Info_ValueForKey( info, "mapname" );
  levelshot = trap_R_RegisterShaderNoMip( va( "levelshots/%s.tga", s ) );

  if( !levelshot )
    levelshot = trap_R_RegisterShaderNoMip( "gfx/2d/load_screen" );

  trap_R_SetColor( NULL );
  CG_DrawPic( rect->x, rect->y, rect->w, rect->h, levelshot );

  // blend a detail texture over it
  detail = trap_R_RegisterShader( "gfx/misc/detail" );
  CG_DrawPic( rect->x, rect->y, rect->w, rect->h, detail );
}

static void CG_DrawLevelName( rectDef_t *rect, float text_x, float text_y,
                              vec4_t color, float scale,
                              int textalign, int textvalign, int textStyle )
{
  const char  *s;

  s = CG_ConfigString( CS_MESSAGE );

  UI_DrawTextBlock( rect, text_x, text_y, color, scale, textalign, textvalign, textStyle, s );
}

static void CG_DrawMOTD( rectDef_t *rect, float text_x, float text_y,
                         vec4_t color, float scale,
                         int textalign, int textvalign, int textStyle )
{
  const char  *s;

  s = CG_ConfigString( CS_MOTD );

  UI_DrawTextBlock( rect, text_x, text_y, color, scale, textalign, textvalign, textStyle, s );
}

static void CG_DrawHostname( rectDef_t *rect, float text_x, float text_y,
                             vec4_t color, float scale,
                             int textalign, int textvalign, int textStyle )
{
  char buffer[ 1024 ];
  const char  *info;

  info = CG_ConfigString( CS_SERVERINFO );

  Q_strncpyz( buffer, Info_ValueForKey( info, "sv_hostname" ), 1024 );
  Q_CleanStr( buffer );

  UI_DrawTextBlock( rect, text_x, text_y, color, scale, textalign, textvalign, textStyle, buffer );
}

/*
==============
CG_DrawDemoPlayback
==============
*/
static void CG_DrawDemoPlayback( rectDef_t *rect, vec4_t color, qhandle_t shader )
{
  if( !cg_drawDemoState.integer )
    return;

  if( trap_GetDemoState( ) != DS_PLAYBACK )
    return;

  trap_R_SetColor( color );
  CG_DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
  trap_R_SetColor( NULL );
}

/*
==============
CG_DrawDemoRecording
==============
*/
static void CG_DrawDemoRecording( rectDef_t *rect, vec4_t color, qhandle_t shader )
{
  if( !cg_drawDemoState.integer )
    return;

  if( trap_GetDemoState( ) != DS_RECORDING )
    return;

  trap_R_SetColor( color );
  CG_DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
  trap_R_SetColor( NULL );
}

/*
======================
CG_UpdateMediaFraction

======================
*/
void CG_UpdateMediaFraction( float newFract )
{
  cg.mediaFraction = newFract;

  trap_UpdateScreen( );
}

/*
====================
CG_DrawLoadingScreen

Draw all the status / pacifier stuff during level loading
====================
*/
void CG_DrawLoadingScreen( void )
{
  Menu_Paint( Menus_FindByName( "Loading" ), qtrue );
}

float CG_GetValue( int ownerDraw )
{
  centity_t *cent;
  playerState_t *ps;
  weapon_t weapon;

  cent = &cg_entities[ cg.snap->ps.clientNum ];
  ps = &cg.snap->ps;
  weapon = BG_GetPlayerWeapon( ps );

  switch( ownerDraw )
  {
    case CG_PLAYER_AMMO_VALUE:
      if( weapon )
        return ps->ammo;
      break;
    case CG_PLAYER_CLIPS_VALUE:
      if( weapon )
        return ps->clips;
      break;
    case CG_PLAYER_HEALTH:
      return ps->stats[ STAT_HEALTH ];
      break;
    default:
      break;
  }

  return -1;
}

const char *CG_GetKillerText( )
{
  const char *s = "";
  if( cg.killerName[ 0 ] )
    s = va( "Fragged by %s", cg.killerName );

  return s;
}


static void CG_DrawKiller( rectDef_t *rect, float scale, vec4_t color,
                           qhandle_t shader, int textStyle )
{
  // fragged by ... line
 if( cg.killerName[ 0 ] )
 {
   int x = rect->x + rect->w / 2;
   UI_Text_Paint( x - UI_Text_Width( CG_GetKillerText( ), scale, 0 ) / 2,
     rect->y + rect->h, scale, color, CG_GetKillerText( ), 0, 0, textStyle );
  }
}


static void CG_DrawTeamSpectators( rectDef_t *rect, float scale, int textvalign, vec4_t color, qhandle_t shader )
{
  float y;

  if( cg.spectatorLen )
  {
    float maxX;

    if( cg.spectatorWidth == -1 )
    {
      cg.spectatorWidth = 0;
      cg.spectatorPaintX = rect->x + 1;
      cg.spectatorPaintX2 = -1;
    }

    if( cg.spectatorOffset > cg.spectatorLen )
    {
      cg.spectatorOffset = 0;
      cg.spectatorPaintX = rect->x + 1;
      cg.spectatorPaintX2 = -1;
    }

    if( cg.time > cg.spectatorTime )
    {
      cg.spectatorTime = cg.time + 10;

      if( cg.spectatorPaintX <= rect->x + 2 )
      {
        if( cg.spectatorOffset < cg.spectatorLen )
        {
          // skip colour directives
          if( Q_IsColorString( &cg.spectatorList[ cg.spectatorOffset ] ) )
            cg.spectatorOffset += 2;
          else
          {
            cg.spectatorPaintX += UI_Text_Width( &cg.spectatorList[ cg.spectatorOffset ], scale, 1 ) - 1;
            cg.spectatorOffset++;
          }
        }
        else
        {
          cg.spectatorOffset = 0;

          if( cg.spectatorPaintX2 >= 0 )
            cg.spectatorPaintX = cg.spectatorPaintX2;
          else
            cg.spectatorPaintX = rect->x + rect->w - 2;

          cg.spectatorPaintX2 = -1;
        }
      }
      else
      {
        cg.spectatorPaintX--;

        if( cg.spectatorPaintX2 >= 0 )
          cg.spectatorPaintX2--;
      }
    }

    maxX = rect->x + rect->w - 2;
    CG_AlignText( rect, NULL, 0.0f, 0.0f, UI_Text_EmHeight( scale ),
                  ALIGN_LEFT, textvalign, NULL, &y );

    UI_Text_Paint_Limit( &maxX, cg.spectatorPaintX, y, scale, color,
                         &cg.spectatorList[ cg.spectatorOffset ], 0, 0 );

    if( cg.spectatorPaintX2 >= 0 )
    {
      float maxX2 = rect->x + rect->w - 2;
      UI_Text_Paint_Limit( &maxX2, cg.spectatorPaintX2, y, scale,
                           color, cg.spectatorList, 0, cg.spectatorOffset );
    }

    if( cg.spectatorOffset && maxX > 0 )
    {
      // if we have an offset ( we are skipping the first part of the string ) and we fit the string
      if( cg.spectatorPaintX2 == -1 )
        cg.spectatorPaintX2 = rect->x + rect->w - 2;
    }
    else
      cg.spectatorPaintX2 = -1;
  }
}

/*
==================
CG_DrawTeamLabel
==================
*/
static void CG_DrawTeamLabel( rectDef_t *rect, team_t team, float text_x, float text_y,
    vec4_t color, float scale, int textalign, int textvalign, int textStyle )
{
  char  *t;
  char  stage[ MAX_TOKEN_CHARS ];
  char  *s;
  float tx, ty;

  stage[ 0 ] = '\0';

  switch( team )
  {
    case TEAM_ALIENS:
      t = "Aliens";
      if( cg.intermissionStarted )
        Com_sprintf( stage, MAX_TOKEN_CHARS, "(Stage %d)", cgs.alienStage + 1 );
      break;

    case TEAM_HUMANS:
      t = "Humans";
      if( cg.intermissionStarted )
        Com_sprintf( stage, MAX_TOKEN_CHARS, "(Stage %d)", cgs.humanStage + 1 );
      break;

    default:
      t = "";
      break;
  }

  switch( textalign )
  {
    default:
    case ALIGN_LEFT:
      s = va( "%s %s", t, stage );
      break;

    case ALIGN_RIGHT:
      s = va( "%s %s", stage, t );
      break;
  }

  CG_AlignText( rect, s, scale, 0.0f, 0.0f, textalign, textvalign, &tx, &ty );
  UI_Text_Paint( text_x + tx, text_y + ty, scale, color, s, 0, 0, textStyle );
}

/*
==================
CG_DrawStageReport
==================
*/
static void CG_DrawStageReport( rectDef_t *rect, float text_x, float text_y,
    vec4_t color, float scale, int textalign, int textvalign, int textStyle )
{
  char  s[ MAX_TOKEN_CHARS ];
  float tx, ty;

  if( cg.intermissionStarted )
    return;

  if( cg.snap->ps.stats[ STAT_TEAM ] == TEAM_NONE )
    return;

  if( cg.snap->ps.stats[ STAT_TEAM ] == TEAM_ALIENS )
  {
    int kills = ceil( (float)(cgs.alienNextStageThreshold - cgs.alienCredits) / ALIEN_CREDITS_PER_FRAG );
    if( kills < 0 )
      kills = 0;

    if( cgs.alienNextStageThreshold < 0 )
      Com_sprintf( s, MAX_TOKEN_CHARS, "Stage %d", cgs.alienStage + 1 );
    else if( kills == 1 )
      Com_sprintf( s, MAX_TOKEN_CHARS, "Stage %d, 1 kill for next stage",
          cgs.alienStage + 1 );
    else
      Com_sprintf( s, MAX_TOKEN_CHARS, "Stage %d, %d kills for next stage",
          cgs.alienStage + 1, kills );
  }
  else if( cg.snap->ps.stats[ STAT_TEAM ] == TEAM_HUMANS )
  {
    int credits = cgs.humanNextStageThreshold - cgs.humanCredits;

    if( credits < 0 )
      credits = 0;

    if( cgs.humanNextStageThreshold < 0 )
      Com_sprintf( s, MAX_TOKEN_CHARS, "Stage %d", cgs.humanStage + 1 );
    else if( credits == 1 )
      Com_sprintf( s, MAX_TOKEN_CHARS, "Stage %d, 1 credit for next stage",
          cgs.humanStage + 1 );
    else
      Com_sprintf( s, MAX_TOKEN_CHARS, "Stage %d, %d credits for next stage",
          cgs.humanStage + 1, credits );
  }

  CG_AlignText( rect, s, scale, 0.0f, 0.0f, textalign, textvalign, &tx, &ty );

  UI_Text_Paint( text_x + tx, text_y + ty, scale, color, s, 0, 0, textStyle );
}

/*
==================
CG_DrawFPS
==================
*/
#define FPS_FRAMES  20
#define FPS_STRING  "fps"
static void CG_DrawFPS( rectDef_t *rect, float text_x, float text_y,
                        float scale, vec4_t color,
                        int textalign, int textvalign, int textStyle,
                        qboolean scalableText )
{
  char        *s;
  float       tx, ty;
  float       w, h, totalWidth;
  int         strLength;
  static int  previousTimes[ FPS_FRAMES ];
  static int  index;
  int         i, total;
  int         fps;
  static int  previous;
  int         t, frameTime;

  if( !cg_drawFPS.integer )
    return;

  // don't use serverTime, because that will be drifting to
  // correct for internet lag changes, timescales, timedemos, etc
  t = trap_Milliseconds( );
  frameTime = t - previous;
  previous = t;

  previousTimes[ index % FPS_FRAMES ] = frameTime;
  index++;

  if( index > FPS_FRAMES )
  {
    // average multiple frames together to smooth changes out a bit
    total = 0;

    for( i = 0 ; i < FPS_FRAMES ; i++ )
      total += previousTimes[ i ];

    if( !total )
      total = 1;

    fps = 1000 * FPS_FRAMES / total;

    s = va( "%d", fps );
    w = UI_Text_Width( "0", scale, 0 );
    h = UI_Text_Height( "0", scale, 0 );
    strLength = CG_DrawStrlen( s );
    totalWidth = UI_Text_Width( FPS_STRING, scale, 0 ) + w * strLength;

    CG_AlignText( rect, s, 0.0f, totalWidth, h, textalign, textvalign, &tx, &ty );

    if( scalableText )
    {
      for( i = 0; i < strLength; i++ )
      {
        char c[ 2 ];

        c[ 0 ] = s[ i ];
        c[ 1 ] = '\0';

        UI_Text_Paint( text_x + tx + i * w, text_y + ty, scale, color, c, 0, 0, textStyle );
      }

      UI_Text_Paint( text_x + tx + i * w, text_y + ty, scale, color, FPS_STRING, 0, 0, textStyle );
    }
    else
    {
      trap_R_SetColor( color );
      CG_DrawField( rect->x, rect->y, 3, rect->w / 3, rect->h, fps );
      trap_R_SetColor( NULL );
    }
  }
}


/*
=================
CG_DrawTimerMins
=================
*/
static void CG_DrawTimerMins( rectDef_t *rect, vec4_t color )
{
  int     mins, seconds;
  int     msec;

  if( !cg_drawTimer.integer )
    return;

  msec = cg.time - cgs.levelStartTime;

  seconds = msec / 1000;
  mins = seconds / 60;
  seconds -= mins * 60;

  trap_R_SetColor( color );
  CG_DrawField( rect->x, rect->y, 3, rect->w / 3, rect->h, mins );
  trap_R_SetColor( NULL );
}


/*
=================
CG_DrawTimerSecs
=================
*/
static void CG_DrawTimerSecs( rectDef_t *rect, vec4_t color )
{
  int     mins, seconds;
  int     msec;

  if( !cg_drawTimer.integer )
    return;

  msec = cg.time - cgs.levelStartTime;

  seconds = msec / 1000;
  mins = seconds / 60;
  seconds -= mins * 60;

  trap_R_SetColor( color );
  CG_DrawFieldPadded( rect->x, rect->y, 2, rect->w / 2, rect->h, seconds );
  trap_R_SetColor( NULL );
}


/*
=================
CG_DrawTimer
=================
*/
static void CG_DrawTimer( rectDef_t *rect, float text_x, float text_y,
                          float scale, vec4_t color,
                          int textalign, int textvalign, int textStyle )
{
  char    *s;
  float   tx, ty;
  int     i, strLength;
  float   w, h, totalWidth;
  int     mins, seconds, tens;
  int     msec;

  if( !cg_drawTimer.integer )
    return;

  msec = cg.time - cgs.levelStartTime;

  seconds = msec / 1000;
  mins = seconds / 60;
  seconds -= mins * 60;
  tens = seconds / 10;
  seconds -= tens * 10;

  s = va( "%d:%d%d", mins, tens, seconds );
  w = UI_Text_Width( "0", scale, 0 );
  h = UI_Text_Height( "0", scale, 0 );
  strLength = CG_DrawStrlen( s );
  totalWidth = w * strLength;

  CG_AlignText( rect, s, 0.0f, totalWidth, h, textalign, textvalign, &tx, &ty );

  for( i = 0; i < strLength; i++ )
  {
    char c[ 2 ];

    c[ 0 ] = s[ i ];
    c[ 1 ] = '\0';

    UI_Text_Paint( text_x + tx + i * w, text_y + ty, scale, color, c, 0, 0, textStyle );
  }
}

/*
=================
CG_DrawClock
=================
*/
static void CG_DrawClock( rectDef_t *rect, float text_x, float text_y,
                          float scale, vec4_t color,
                          int textalign, int textvalign, int textStyle )
{
  char    *s;
  float   tx, ty;
  int     i, strLength;
  float   w, h, totalWidth;
  qtime_t qt;
  int     t;

  if( !cg_drawClock.integer )
    return;

  t = trap_RealTime( &qt );

  if( cg_drawClock.integer == 2 )
  {
    s = va( "%02d%s%02d", qt.tm_hour, ( qt.tm_sec % 2 ) ? ":" : " ",
      qt.tm_min );
  }
  else
  {
    char *pm = "am";
    int h = qt.tm_hour;

    if( h == 0 )
      h = 12;
    else if( h == 12 )
      pm = "pm";
    else if( h > 12 )
    {
      h -= 12;
      pm = "pm";
    }

    s = va( "%d%s%02d%s", h, ( qt.tm_sec % 2 ) ? ":" : " ", qt.tm_min, pm );
  }
  w = UI_Text_Width( "0", scale, 0 );
  h = UI_Text_Height( "0", scale, 0 );
  strLength = CG_DrawStrlen( s );
  totalWidth = w * strLength;

  CG_AlignText( rect, s, 0.0f, totalWidth, h, textalign, textvalign, &tx, &ty );

  for( i = 0; i < strLength; i++ )
  {
    char c[ 2 ];

    c[ 0 ] = s[ i ];
    c[ 1 ] = '\0';

    UI_Text_Paint( text_x + tx + i * w, text_y + ty, scale, color, c, 0, 0, textStyle );
  }
}

/*
==================
CG_DrawSnapshot
==================
*/
static void CG_DrawSnapshot( rectDef_t *rect, float text_x, float text_y,
                             float scale, vec4_t color,
                             int textalign, int textvalign, int textStyle )
{
  char    *s;
  float   tx, ty;

  if( !cg_drawSnapshot.integer )
    return;

  s = va( "time:%d snap:%d cmd:%d", cg.snap->serverTime,
    cg.latestSnapshotNum, cgs.serverCommandSequence );

  CG_AlignText( rect, s, scale, 0.0f, 0.0f, textalign, textvalign, &tx, &ty );

  UI_Text_Paint( text_x + tx, text_y + ty, scale, color, s, 0, 0, textStyle );
}

/*
===============================================================================

LAGOMETER

===============================================================================
*/

#define LAG_SAMPLES   128

typedef struct
{
  int frameSamples[ LAG_SAMPLES ];
  int frameCount;
  int snapshotFlags[ LAG_SAMPLES ];
  int snapshotSamples[ LAG_SAMPLES ];
  int snapshotCount;
} lagometer_t;

lagometer_t   lagometer;

/*
==============
CG_AddLagometerFrameInfo

Adds the current interpolate / extrapolate bar for this frame
==============
*/
void CG_AddLagometerFrameInfo( void )
{
  int     offset;

  offset = cg.time - cg.latestSnapshotTime;
  lagometer.frameSamples[ lagometer.frameCount & ( LAG_SAMPLES - 1 ) ] = offset;
  lagometer.frameCount++;
}

/*
==============
CG_AddLagometerSnapshotInfo

Each time a snapshot is received, log its ping time and
the number of snapshots that were dropped before it.

Pass NULL for a dropped packet.
==============
*/
#define PING_FRAMES 40
void CG_AddLagometerSnapshotInfo( snapshot_t *snap )
{
  static int  previousPings[ PING_FRAMES ];
  static int  index;
  int         i;

  // dropped packet
  if( !snap )
  {
    lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1 ) ] = -1;
    lagometer.snapshotCount++;
    return;
  }

  // add this snapshot's info
  lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1 ) ] = snap->ping;
  lagometer.snapshotFlags[ lagometer.snapshotCount & ( LAG_SAMPLES - 1 ) ] = snap->snapFlags;
  lagometer.snapshotCount++;

  cg.ping = 0;
  if( cg.snap )
  {
    previousPings[ index++ ] = cg.snap->ping;
    index = index % PING_FRAMES;
  
    for( i = 0; i < PING_FRAMES; i++ )
    {
      cg.ping += previousPings[ i ];
    }

    cg.ping /= PING_FRAMES;
  }
}

/*
==============
CG_DrawDisconnect

Should we draw something differnet for long lag vs no packets?
==============
*/
static void CG_DrawDisconnect( void )
{
  float       x, y;
  int         cmdNum;
  usercmd_t   cmd;
  const char  *s;
  int         w;
  vec4_t      color = { 1.0f, 1.0f, 1.0f, 1.0f };

  // draw the phone jack if we are completely past our buffers
  cmdNum = trap_GetCurrentCmdNumber( ) - CMD_BACKUP + 1;
  trap_GetUserCmd( cmdNum, &cmd );

  // special check for map_restart
  if( cmd.serverTime <= cg.snap->ps.commandTime || cmd.serverTime > cg.time )
    return;

  // also add text in center of screen
  s = "Connection Interrupted";
  w = UI_Text_Width( s, 0.7f, 0 );
  UI_Text_Paint( 320 - w / 2, 100, 0.7f, color, s, 0, 0, ITEM_TEXTSTYLE_SHADOWED );

  // blink the icon
  if( ( cg.time >> 9 ) & 1 )
    return;

  x = 640 - 48;
  y = 480 - 48;

  CG_DrawPic( x, y, 48, 48, trap_R_RegisterShader( "gfx/2d/net.tga" ) );
}

#define MAX_LAGOMETER_PING  900
#define MAX_LAGOMETER_RANGE 300


/*
==============
CG_DrawLagometer
==============
*/
static void CG_DrawLagometer( rectDef_t *rect, float text_x, float text_y,
    float scale, vec4_t textColor )
{
  int     a, x, y, i;
  float   v;
  float   ax, ay, aw, ah, mid, range;
  int     color;
  vec4_t  adjustedColor;
  float   vscale;
  vec4_t  white = { 1.0f, 1.0f, 1.0f, 1.0f };

  if( cg.snap->ps.pm_type == PM_INTERMISSION )
    return;

  if( !cg_lagometer.integer )
    return;

  if( cg.demoPlayback )
    return;

  Vector4Copy( textColor, adjustedColor );
  adjustedColor[ 3 ] = 0.25f;

  trap_R_SetColor( adjustedColor );
  CG_DrawPic( rect->x, rect->y, rect->w, rect->h, cgs.media.whiteShader );
  trap_R_SetColor( NULL );

  //
  // draw the graph
  //
  ax = x = rect->x;
  ay = y = rect->y;
  aw = rect->w;
  ah = rect->h;

  trap_R_SetColor( NULL );

  CG_AdjustFrom640( &ax, &ay, &aw, &ah );

  color = -1;
  range = ah / 3;
  mid = ay + range;

  vscale = range / MAX_LAGOMETER_RANGE;

  // draw the frame interpoalte / extrapolate graph
  for( a = 0 ; a < aw ; a++ )
  {
    i = ( lagometer.frameCount - 1 - a ) & ( LAG_SAMPLES - 1 );
    v = lagometer.frameSamples[ i ];
    v *= vscale;

    if( v > 0 )
    {
      if( color != 1 )
      {
        color = 1;
        trap_R_SetColor( g_color_table[ ColorIndex( COLOR_YELLOW ) ] );
      }

      if( v > range )
        v = range;

      trap_R_DrawStretchPic( ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
    }
    else if( v < 0 )
    {
      if( color != 2 )
      {
        color = 2;
        trap_R_SetColor( g_color_table[ ColorIndex( COLOR_BLUE ) ] );
      }

      v = -v;
      if( v > range )
        v = range;

      trap_R_DrawStretchPic( ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
    }
  }

  // draw the snapshot latency / drop graph
  range = ah / 2;
  vscale = range / MAX_LAGOMETER_PING;

  for( a = 0 ; a < aw ; a++ )
  {
    i = ( lagometer.snapshotCount - 1 - a ) & ( LAG_SAMPLES - 1 );
    v = lagometer.snapshotSamples[ i ];

    if( v > 0 )
    {
      if( lagometer.snapshotFlags[ i ] & SNAPFLAG_RATE_DELAYED )
      {
        if( color != 5 )
        {
          color = 5;  // YELLOW for rate delay
          trap_R_SetColor( g_color_table[ ColorIndex( COLOR_YELLOW ) ] );
        }
      }
      else
      {
        if( color != 3 )
        {
          color = 3;

          trap_R_SetColor( g_color_table[ ColorIndex( COLOR_GREEN ) ] );
        }
      }

      v = v * vscale;

      if( v > range )
        v = range;

      trap_R_DrawStretchPic( ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
    }
    else if( v < 0 )
    {
      if( color != 4 )
      {
        color = 4;    // RED for dropped snapshots
        trap_R_SetColor( g_color_table[ ColorIndex( COLOR_RED ) ] );
      }

      trap_R_DrawStretchPic( ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, cgs.media.whiteShader );
    }
  }

  trap_R_SetColor( NULL );

  if( cg_nopredict.integer || cg_synchronousClients.integer )
    UI_Text_Paint( ax, ay, 0.5, white, "snc", 0, 0, ITEM_TEXTSTYLE_NORMAL );
  else
  {
    char        *s;

    s = va( "%d", cg.ping );
    ax = rect->x + ( rect->w / 2.0f ) - ( UI_Text_Width( s, scale, 0 ) / 2.0f ) + text_x;
    ay = rect->y + ( rect->h / 2.0f ) + ( UI_Text_Height( s, scale, 0 ) / 2.0f ) + text_y;

    Vector4Copy( textColor, adjustedColor );
    adjustedColor[ 3 ] = 0.5f;
    UI_Text_Paint( ax, ay, scale, adjustedColor, s, 0, 0, ITEM_TEXTSTYLE_NORMAL );
  }

  CG_DrawDisconnect( );
}

/*
===================
CG_DrawConsole
===================
*/
static void CG_DrawConsole( rectDef_t *rect, float text_x, float text_y, vec4_t color,
                            float scale, int textalign, int textvalign, int textStyle )
{
  UI_DrawTextBlock( rect, text_x, text_y, color, scale, textalign, textvalign, textStyle, cg.consoleText );
}

/*
===================
CG_DrawTutorial
===================
*/
static void CG_DrawTutorial( rectDef_t *rect, float text_x, float text_y, vec4_t color,
                            float scale, int textalign, int textvalign, int textStyle )
{
  if( !cg_tutorial.integer )
    return;

  UI_DrawTextBlock( rect, text_x, text_y, color, scale, textalign, textvalign, textStyle, CG_TutorialText( ) );
}

/*
===================
CG_DrawWeaponIcon
===================
*/
void CG_DrawWeaponIcon( rectDef_t *rect, vec4_t color )
{
  int           maxAmmo;
  centity_t     *cent;
  playerState_t *ps;
  weapon_t      weapon;

  cent = &cg_entities[ cg.snap->ps.clientNum ];
  ps = &cg.snap->ps;
  weapon = BG_GetPlayerWeapon( ps );

  maxAmmo = BG_Weapon( weapon )->maxAmmo;

  // don't display if dead
  if( cg.predictedPlayerState.stats[ STAT_HEALTH ] <= 0 )
    return;

  if( weapon == 0 )
    return;

  CG_RegisterWeapon( weapon );

  if( ps->clips == 0 && !BG_Weapon( weapon )->infiniteAmmo )
  {
    float ammoPercent = (float)ps->ammo / (float)maxAmmo;

    if( ammoPercent < 0.33f )
    {
      color[ 0 ] = 1.0f;
      color[ 1 ] = color[ 2 ] = 0.0f;
    }
  }

  if( cg.predictedPlayerState.stats[ STAT_TEAM ] == TEAM_ALIENS &&
      !BG_AlienCanEvolve( cg.predictedPlayerState.stats[ STAT_CLASS ],
                          ps->persistant[ PERS_CREDIT ], cgs.alienStage ) )
  {
    if( cg.time - cg.lastEvolveAttempt <= NO_CREDITS_TIME )
    {
      if( ( ( cg.time - cg.lastEvolveAttempt ) / 300 ) % 2 )
        color[ 3 ] = 0.0f;
    }
  }

  trap_R_SetColor( color );
  CG_DrawPic( rect->x, rect->y, rect->w, rect->h,
              cg_weapons[ weapon ].weaponIcon );
  trap_R_SetColor( NULL );
}



/*
================================================================================

CROSSHAIR

================================================================================
*/



/*
=================
CG_DrawCrosshair
=================
*/
static void CG_DrawCrosshair( rectDef_t *rect, vec4_t color )
{
  float         w, h;
  qhandle_t     hShader;
  float         x, y;
  weaponInfo_t  *wi;
  weapon_t      weapon;
  
  weapon = BG_GetPlayerWeapon( &cg.snap->ps );

  if( cg_drawCrosshair.integer == CROSSHAIR_ALWAYSOFF )
    return;

  if( cg_drawCrosshair.integer == CROSSHAIR_RANGEDONLY &&
      !BG_Weapon( weapon )->longRanged )
    return;

  if( ( cg.snap->ps.persistant[ PERS_SPECSTATE ] != SPECTATOR_NOT ) ||
      ( cg.snap->ps.stats[ STAT_STATE ] & SS_INFESTING ) ||
      ( cg.snap->ps.stats[ STAT_STATE ] & SS_HOVELING ) )
    return;

  if( cg.renderingThirdPerson )
    return;

  wi = &cg_weapons[ weapon ];

  w = h = wi->crossHairSize * cg_crosshairSize.value;
  w *= cgDC.aspectScale;
  
  //FIXME: this still ignores the width/height of the rect, but at least it's
  //neater than cg_crosshairX/cg_crosshairY
  x = rect->x + ( rect->w / 2 ) - ( w / 2 );
  y = rect->y + ( rect->h / 2 ) - ( h / 2 );

  hShader = wi->crossHair;
  
  //aiming at a friendly player/buildable, dim the crosshair
  if( cg.time == cg.crosshairClientTime || cg.crosshairBuildable >= 0 )
  {
    int i;
    for( i = 0; i < 3; i++ )
      color[i] *= .5f;

  }

  if( hShader != 0 )
  {

    trap_R_SetColor( color );
    CG_DrawPic( x, y, w, h, hShader );
    trap_R_SetColor( NULL );
  }
}



/*
=================
CG_ScanForCrosshairEntity
=================
*/
static void CG_ScanForCrosshairEntity( void )
{
  trace_t   trace;
  vec3_t    start, end;
  int       content;
  team_t    team;

  VectorCopy( cg.refdef.vieworg, start );
  VectorMA( start, 131072, cg.refdef.viewaxis[ 0 ], end );

  CG_Trace( &trace, start, vec3_origin, vec3_origin, end,
    cg.snap->ps.clientNum, CONTENTS_SOLID|CONTENTS_BODY );

  // if the player is in fog, don't show it
  content = trap_CM_PointContents( trace.endpos, 0 );
  if( content & CONTENTS_FOG )
    return;

  if( trace.entityNum >= MAX_CLIENTS )
  {
    entityState_t *s = &cg_entities[ trace.entityNum ].currentState;
    if( s->eType == ET_BUILDABLE && BG_Buildable( s->modelindex )->team ==
        cg.snap->ps.stats[ STAT_TEAM ] )
      cg.crosshairBuildable = trace.entityNum;
    else
      cg.crosshairBuildable = -1;

    return;
  }

  team = cgs.clientinfo[ trace.entityNum ].team;

  if( cg.snap->ps.persistant[ PERS_SPECSTATE ] == SPECTATOR_NOT )
  {
    //only display team names of those on the same team as this player
    if( team != cg.snap->ps.stats[ STAT_TEAM ] )
      return;
  }

  // update the fade timer
  cg.crosshairClientNum = trace.entityNum;
  cg.crosshairClientTime = cg.time;
}


/*
=====================
CG_DrawLocation
=====================
*/
static void CG_DrawLocation( rectDef_t *rect, float scale, int textalign, vec4_t color )
{
  const char    *location;
  centity_t     *locent;
  float         maxX;
  float         tx = rect->x, ty = rect->y;
  maxX = rect->x + rect->w;

  locent = CG_GetLocation( &cg_entities[ cg.clientNum ] );
  if( locent )
    location = CG_ConfigString( CS_LOCATIONS + locent->currentState.generic1 );
  else
    location = CG_ConfigString( CS_LOCATIONS );

  if( UI_Text_Width( location, scale, 0 ) < rect->w ) 
    CG_AlignText( rect, location, scale, 0.0f, 0.0f, textalign, VALIGN_CENTER, &tx, &ty );

  UI_Text_Paint_Limit( &maxX, tx, ty, scale, color, location, 0, 0 );
  trap_R_SetColor( NULL );
}

/*
=====================
CG_DrawCrosshairNames
=====================
*/
static void CG_DrawCrosshairNames( rectDef_t *rect, float scale, int textStyle )
{
  float   *color;
  char    *name;
  float   w, x;

  if( !cg_drawCrosshairNames.integer )
    return;

  if( cg.renderingThirdPerson )
    return;

  // scan the known entities to see if the crosshair is sighted on one
  CG_ScanForCrosshairEntity( );

  // draw the name of the player being looked at
  color = CG_FadeColor( cg.crosshairClientTime, CROSSHAIR_CLIENT_TIMEOUT );
  if( !color )
  {
    trap_R_SetColor( NULL );
    return;
  }

  name = cgs.clientinfo[ cg.crosshairClientNum ].name;
  w = UI_Text_Width( name, scale, 0 );
  x = rect->x + rect->w / 2;
  UI_Text_Paint( x - w / 2, rect->y + rect->h, scale, color, name, 0, 0, textStyle );
  trap_R_SetColor( NULL );
}

/*
===============
CG_OwnerDraw

Draw an owner drawn item
===============
*/
void CG_OwnerDraw( float x, float y, float w, float h, float text_x,
                   float text_y, int ownerDraw, int ownerDrawFlags,
                   int align, int textalign, int textvalign, float special,
                   float scale, vec4_t color,
                   qhandle_t shader, int textStyle )
{
  rectDef_t rect;

  if( cg_drawStatus.integer == 0 )
    return;

  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;

  switch( ownerDraw )
  {
    case CG_PLAYER_CREDITS_VALUE:
      CG_DrawPlayerCreditsValue( &rect, color, qtrue );
      break;
    case CG_PLAYER_BANK_VALUE:
      CG_DrawPlayerBankValue( &rect, color, qtrue );
      break;
    case CG_PLAYER_CREDITS_VALUE_NOPAD:
      CG_DrawPlayerCreditsValue( &rect, color, qfalse );
      break;
    case CG_PLAYER_BANK_VALUE_NOPAD:
      CG_DrawPlayerBankValue( &rect, color, qfalse );
      break;
    case CG_PLAYER_STAMINA_1:
      CG_DrawPlayerStamina1( &rect, color, shader );
      break;
    case CG_PLAYER_STAMINA_2:
      CG_DrawPlayerStamina2( &rect, color, shader );
      break;
    case CG_PLAYER_STAMINA_3:
      CG_DrawPlayerStamina3( &rect, color, shader );
      break;
    case CG_PLAYER_STAMINA_4:
      CG_DrawPlayerStamina4( &rect, color, shader );
      break;
    case CG_PLAYER_STAMINA_BOLT:
      CG_DrawPlayerStaminaBolt( &rect, color, shader );
      break;
    case CG_PLAYER_AMMO_VALUE:
      CG_DrawPlayerAmmoValue( &rect, color );
      break;
    case CG_PLAYER_CLIPS_VALUE:
      CG_DrawPlayerClipsValue( &rect, color );
      break;
    case CG_PLAYER_BUILD_TIMER:
      CG_DrawPlayerBuildTimer( &rect, color );
      break;
    case CG_PLAYER_HEALTH:
      CG_DrawPlayerHealthValue( &rect, color );
      break;
    case CG_PLAYER_HEALTH_CROSS:
      CG_DrawPlayerHealthCross( &rect, color );
      break;
    case CG_PLAYER_CHARGE_BAR_BG:
      CG_DrawPlayerChargeBarBG( &rect, color, shader );
      break;
    case CG_PLAYER_CHARGE_BAR:
      CG_DrawPlayerChargeBar( &rect, color, shader );
      break;
    case CG_PLAYER_CLIPS_RING:
      CG_DrawPlayerClipsRing( &rect, color, shader );
      break;
    case CG_PLAYER_BUILD_TIMER_RING:
      CG_DrawPlayerBuildTimerRing( &rect, color, shader );
      break;
    case CG_PLAYER_WALLCLIMBING:
      CG_DrawPlayerWallclimbing( &rect, color, shader );
      break;
    case CG_PLAYER_BOOSTED:
      CG_DrawPlayerBoosted( &rect, color, shader );
      break;
    case CG_PLAYER_BOOST_BOLT:
      CG_DrawPlayerBoosterBolt( &rect, color, shader );
      break;
    case CG_PLAYER_POISON_BARBS:
      CG_DrawPlayerPoisonBarbs( &rect, color, shader );
      break;
    case CG_PLAYER_ALIEN_SENSE:
      CG_DrawAlienSense( &rect );
      break;
    case CG_PLAYER_HUMAN_SCANNER:
      CG_DrawHumanScanner( &rect, shader, color );
      break;
    case CG_PLAYER_USABLE_BUILDABLE:
      CG_DrawUsableBuildable( &rect, shader, color );
      break;
    case CG_KILLER:
      CG_DrawKiller( &rect, scale, color, shader, textStyle );
      break;
    case CG_PLAYER_SELECT:
      CG_DrawItemSelect( &rect, color );
      break;
    case CG_PLAYER_WEAPONICON:
      CG_DrawWeaponIcon( &rect, color );
      break;
    case CG_PLAYER_SELECTTEXT:
      CG_DrawItemSelectText( &rect, scale, textStyle );
      break;
    case CG_SPECTATORS:
      CG_DrawTeamSpectators( &rect, scale, textvalign, color, shader );
      break;
    case CG_PLAYER_LOCATION:
      CG_DrawLocation( &rect, scale, textalign, color );
      break;
    case CG_PLAYER_CROSSHAIRNAMES:
      CG_DrawCrosshairNames( &rect, scale, textStyle );
      break;
    case CG_PLAYER_CROSSHAIR:
      CG_DrawCrosshair( &rect, color );
      break;
    case CG_STAGE_REPORT_TEXT:
      CG_DrawStageReport( &rect, text_x, text_y, color, scale, textalign, textvalign, textStyle );
      break;
    case CG_ALIENS_SCORE_LABEL:
      CG_DrawTeamLabel( &rect, TEAM_ALIENS, text_x, text_y, color, scale, textalign, textvalign, textStyle );
      break;
    case CG_HUMANS_SCORE_LABEL:
      CG_DrawTeamLabel( &rect, TEAM_HUMANS, text_x, text_y, color, scale, textalign, textvalign, textStyle );
      break;

    //loading screen
    case CG_LOAD_LEVELSHOT:
      CG_DrawLevelShot( &rect );
      break;
    case CG_LOAD_MEDIA:
      CG_DrawMediaProgress( &rect, color, scale, align, textalign, textStyle, special );
      break;
    case CG_LOAD_MEDIA_LABEL:
      CG_DrawMediaProgressLabel( &rect, text_x, text_y, color, scale, textalign, textvalign );
      break;
    case CG_LOAD_BUILDABLES:
      CG_DrawBuildablesProgress( &rect, color, scale, align, textalign, textStyle, special );
      break;
    case CG_LOAD_BUILDABLES_LABEL:
      CG_DrawBuildablesProgressLabel( &rect, text_x, text_y, color, scale, textalign, textvalign );
      break;
    case CG_LOAD_CHARMODEL:
      CG_DrawCharModelProgress( &rect, color, scale, align, textalign, textStyle, special );
      break;
    case CG_LOAD_CHARMODEL_LABEL:
      CG_DrawCharModelProgressLabel( &rect, text_x, text_y, color, scale, textalign, textvalign );
      break;
    case CG_LOAD_OVERALL:
      CG_DrawOverallProgress( &rect, color, scale, align, textalign, textStyle, special );
      break;
    case CG_LOAD_LEVELNAME:
      CG_DrawLevelName( &rect, text_x, text_y, color, scale, textalign, textvalign, textStyle );
      break;
    case CG_LOAD_MOTD:
      CG_DrawMOTD( &rect, text_x, text_y, color, scale, textalign, textvalign, textStyle );
      break;
    case CG_LOAD_HOSTNAME:
      CG_DrawHostname( &rect, text_x, text_y, color, scale, textalign, textvalign, textStyle );
      break;

    case CG_FPS:
      CG_DrawFPS( &rect, text_x, text_y, scale, color, textalign, textvalign, textStyle, qtrue );
      break;
    case CG_FPS_FIXED:
      CG_DrawFPS( &rect, text_x, text_y, scale, color, textalign, textvalign, textStyle, qfalse );
      break;
    case CG_TIMER:
      CG_DrawTimer( &rect, text_x, text_y, scale, color, textalign, textvalign, textStyle );
      break;
    case CG_CLOCK:
      CG_DrawClock( &rect, text_x, text_y, scale, color, textalign, textvalign, textStyle );
      break;
    case CG_TIMER_MINS:
      CG_DrawTimerMins( &rect, color );
      break;
    case CG_TIMER_SECS:
      CG_DrawTimerSecs( &rect, color );
      break;
    case CG_SNAPSHOT:
      CG_DrawSnapshot( &rect, text_x, text_y, scale, color, textalign, textvalign, textStyle );
      break;
    case CG_LAGOMETER:
      CG_DrawLagometer( &rect, text_x, text_y, scale, color );
      break;

    case CG_DEMO_PLAYBACK:
      CG_DrawDemoPlayback( &rect, color, shader );
      break;
    case CG_DEMO_RECORDING:
      CG_DrawDemoRecording( &rect, color, shader );
      break;

    case CG_CONSOLE:
      CG_DrawConsole( &rect, text_x, text_y, color, scale, textalign, textvalign, textStyle );
      break;

    case CG_TUTORIAL:
      CG_DrawTutorial( &rect, text_x, text_y, color, scale, textalign, textvalign, textStyle );
      break;

    default:
      break;
  }
}

void CG_MouseEvent( int x, int y )
{
  int n;

  if( ( cg.predictedPlayerState.pm_type == PM_NORMAL ||
        cg.predictedPlayerState.pm_type == PM_SPECTATOR ) &&
        cg.showScores == qfalse )
  {
    trap_Key_SetCatcher( 0 );
    return;
  }

  cgs.cursorX += x;
  if( cgs.cursorX < 0 )
    cgs.cursorX = 0;
  else if( cgs.cursorX > 640 )
    cgs.cursorX = 640;

  cgs.cursorY += y;
  if( cgs.cursorY < 0 )
    cgs.cursorY = 0;
  else if( cgs.cursorY > 480 )
    cgs.cursorY = 480;

  n = Display_CursorType( cgs.cursorX, cgs.cursorY );
  cgs.activeCursor = 0;
  if( n == CURSOR_ARROW )
    cgs.activeCursor = cgs.media.selectCursor;
  else if( n == CURSOR_SIZER )
    cgs.activeCursor = cgs.media.sizeCursor;

  if( cgs.capturedItem )
    Display_MouseMove( cgs.capturedItem, x, y );
  else
    Display_MouseMove( NULL, cgs.cursorX, cgs.cursorY );
}

/*
==================
CG_HideTeamMenus
==================

*/
void CG_HideTeamMenu( void )
{
  Menus_CloseByName( "teamMenu" );
  Menus_CloseByName( "getMenu" );
}

/*
==================
CG_ShowTeamMenus
==================

*/
void CG_ShowTeamMenu( void )
{
  Menus_ActivateByName( "teamMenu" );
}

/*
==================
CG_EventHandling
==================
 type 0 - no event handling
      1 - team menu
      2 - hud editor

*/
void CG_EventHandling( int type )
{
  cgs.eventHandling = type;

  if( type == CGAME_EVENT_NONE )
    CG_HideTeamMenu( );
}



void CG_KeyEvent( int key, qboolean down )
{
  if( !down )
    return;

  if( cg.predictedPlayerState.pm_type == PM_NORMAL ||
      ( cg.predictedPlayerState.pm_type == PM_SPECTATOR &&
        cg.showScores == qfalse ) )
  {
    CG_EventHandling( CGAME_EVENT_NONE );
    trap_Key_SetCatcher( 0 );
    return;
  }

  Display_HandleKey( key, down, cgs.cursorX, cgs.cursorY );

  if( cgs.capturedItem )
    cgs.capturedItem = NULL;
  else
  {
    if( key == K_MOUSE2 && down )
      cgs.capturedItem = Display_CaptureItem( cgs.cursorX, cgs.cursorY );
  }
}

int CG_ClientNumFromName( const char *p )
{
  int i;

  for( i = 0; i < cgs.maxclients; i++ )
  {
    if( cgs.clientinfo[ i ].infoValid &&
        Q_stricmp( cgs.clientinfo[ i ].name, p ) == 0 )
      return i;
  }

  return -1;
}

void CG_RunMenuScript( char **args )
{
}
//END TA UI


/*
================
CG_DrawLighting

================
*/
static void CG_DrawLighting( void )
{
  centity_t   *cent;

  cent = &cg_entities[ cg.snap->ps.clientNum ];

  //fade to black if stamina is low
  if( ( cg.snap->ps.stats[ STAT_STAMINA ] < -800 ) &&
      ( cg.snap->ps.stats[ STAT_TEAM ] == TEAM_HUMANS ) )
  {
    vec4_t black = { 0, 0, 0, 0 };
    black[ 3 ] = 1.0 - ( (float)( cg.snap->ps.stats[ STAT_STAMINA ] + 1000 ) / 200.0f );
    trap_R_SetColor( black );
    CG_DrawPic( 0, 0, 640, 480, cgs.media.whiteShader );
    trap_R_SetColor( NULL );
  }
}

/*
===============================================================================

CENTER PRINTING

===============================================================================
*/


/*
==============
CG_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void CG_CenterPrint( const char *str, int y, int charWidth )
{
  char  *s;

  Q_strncpyz( cg.centerPrint, str, sizeof( cg.centerPrint ) );

  cg.centerPrintTime = cg.time;
  cg.centerPrintY = y;
  cg.centerPrintCharWidth = charWidth;

  // count the number of lines for centering
  cg.centerPrintLines = 1;
  s = cg.centerPrint;
  while( *s )
  {
    if( *s == '\n' )
      cg.centerPrintLines++;

    s++;
  }
}


/*
===================
CG_DrawCenterString
===================
*/
static void CG_DrawCenterString( void )
{
  char  *start;
  int   l;
  int   x, y, w;
  int h;
  float *color;

  if( !cg.centerPrintTime )
    return;

  color = CG_FadeColor( cg.centerPrintTime, 1000 * cg_centertime.value );
  if( !color )
    return;

  trap_R_SetColor( color );

  start = cg.centerPrint;

  y = cg.centerPrintY - cg.centerPrintLines * BIGCHAR_HEIGHT / 2;

  while( 1 )
  {
    char linebuffer[ 1024 ];

    for( l = 0; l < 50; l++ )
    {
      if( !start[ l ] || start[ l ] == '\n' )
        break;

      linebuffer[ l ] = start[ l ];
    }

    linebuffer[ l ] = 0;

    w = UI_Text_Width( linebuffer, 0.5, 0 );
    h = UI_Text_Height( linebuffer, 0.5, 0 );
    x = ( SCREEN_WIDTH - w ) / 2;
    UI_Text_Paint( x, y + h, 0.5, color, linebuffer, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE );
    y += h + 6;

    while( *start && ( *start != '\n' ) )
      start++;

    if( !*start )
      break;

    start++;
  }

  trap_R_SetColor( NULL );
}





//==============================================================================

//FIXME: both vote notes are hardcoded, change to ownerdrawn?

/*
=================
CG_DrawVote
=================
*/
static void CG_DrawVote( void )
{
  char    *s;
  int     sec;
  vec4_t  white = { 1.0f, 1.0f, 1.0f, 1.0f };
  char    yeskey[ 32 ], nokey[ 32 ];

  if( !cgs.voteTime )
    return;

  // play a talk beep whenever it is modified
  if( cgs.voteModified )
  {
    cgs.voteModified = qfalse;
    trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
  }

  sec = ( VOTE_TIME - ( cg.time - cgs.voteTime ) ) / 1000;

  if( sec < 0 )
    sec = 0;
  Q_strncpyz( yeskey, CG_KeyBinding( "vote yes" ), sizeof( yeskey ) ); 
  Q_strncpyz( nokey, CG_KeyBinding( "vote no" ), sizeof( nokey ) ); 
  s = va( "VOTE(%i): \"%s\"  [%s]Yes:%i [%s]No:%i", sec, cgs.voteString,
    yeskey, cgs.voteYes, nokey, cgs.voteNo );
  UI_Text_Paint( 8, 340, 0.3f, white, s, 0, 0, ITEM_TEXTSTYLE_NORMAL );
}

/*
=================
CG_DrawTeamVote
=================
*/
static void CG_DrawTeamVote( void )
{
  char    *s;
  int     sec, cs_offset;
  vec4_t  white = { 1.0f, 1.0f, 1.0f, 1.0f };
  char    yeskey[ 32 ], nokey[ 32 ];

  if( cg.predictedPlayerState.stats[ STAT_TEAM ] == TEAM_HUMANS )
    cs_offset = 0;
  else if( cg.predictedPlayerState.stats[ STAT_TEAM ] == TEAM_ALIENS )
    cs_offset = 1;
  else
    return;

  if( !cgs.teamVoteTime[ cs_offset ] )
    return;

  // play a talk beep whenever it is modified
  if ( cgs.teamVoteModified[ cs_offset ] )
  {
    cgs.teamVoteModified[ cs_offset ] = qfalse;
    trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
  }

  sec = ( VOTE_TIME - ( cg.time - cgs.teamVoteTime[ cs_offset ] ) ) / 1000;

  if( sec < 0 )
    sec = 0;

  Q_strncpyz( yeskey, CG_KeyBinding( "teamvote yes" ), sizeof( yeskey ) ); 
  Q_strncpyz( nokey, CG_KeyBinding( "teamvote no" ), sizeof( nokey ) ); 
  s = va( "TEAMVOTE(%i): \"%s\"  [%s]Yes:%i   [%s]No:%i", sec,
          cgs.teamVoteString[ cs_offset ],
          yeskey, cgs.teamVoteYes[cs_offset],
          nokey, cgs.teamVoteNo[ cs_offset ] );

  UI_Text_Paint( 8, 360, 0.3f, white, s, 0, 0, ITEM_TEXTSTYLE_NORMAL );
}


static qboolean CG_DrawScoreboard( void )
{
  static qboolean firstTime = qtrue;
  float fade, *fadeColor;

  if( menuScoreboard )
    menuScoreboard->window.flags &= ~WINDOW_FORCED;

  if( cg_paused.integer )
  {
    cg.deferredPlayerLoading = 0;
    firstTime = qtrue;
    return qfalse;
  }

  if( cg.showScores ||
      cg.predictedPlayerState.pm_type == PM_INTERMISSION )
  {
    fade = 1.0;
    fadeColor = colorWhite;
  }
  else
  {
    cg.deferredPlayerLoading = 0;
    cg.killerName[ 0 ] = 0;
    firstTime = qtrue;
    return qfalse;
  }


  if( menuScoreboard == NULL )
    menuScoreboard = Menus_FindByName( "teamscore_menu" );

  if( menuScoreboard )
  {
    if( firstTime )
    {
      CG_SetScoreSelection( menuScoreboard );
      firstTime = qfalse;
    }

    Menu_Paint( menuScoreboard, qtrue );
  }

  return qtrue;
}

/*
=================
CG_DrawIntermission
=================
*/
static void CG_DrawIntermission( void )
{
  if( cg_drawStatus.integer )
    Menu_Paint( Menus_FindByName( "default_hud" ), qtrue );

  cg.scoreFadeTime = cg.time;
  cg.scoreBoardShowing = CG_DrawScoreboard( );
}

#define FOLLOWING_STRING "following "

/*
=================
CG_DrawFollow
=================
*/
static qboolean CG_DrawFollow( void )
{
  float       w;
  vec4_t      color;
  char        buffer[ MAX_STRING_CHARS ];

  if( cg.snap->ps.clientNum == cg.clientNum )
    return qfalse;

  color[ 0 ] = 1;
  color[ 1 ] = 1;
  color[ 2 ] = 1;
  color[ 3 ] = 1;

  strcpy( buffer, FOLLOWING_STRING );
  strcat( buffer, cgs.clientinfo[ cg.snap->ps.clientNum ].name );

  w = UI_Text_Width( buffer, 0.7f, 0 );
  UI_Text_Paint( 320 - w / 2, 400, 0.7f, color, buffer, 0, 0, ITEM_TEXTSTYLE_SHADOWED );

  return qtrue;
}

/*
=================
CG_DrawQueue
=================
*/
static qboolean CG_DrawQueue( void )
{
  float       w;
  vec4_t      color;
  int         position, remainder;
  char        *ordinal, buffer[ MAX_STRING_CHARS ];

  if( !( cg.snap->ps.pm_flags & PMF_QUEUED ) )
    return qfalse;

  color[ 0 ] = 1;
  color[ 1 ] = 1;
  color[ 2 ] = 1;
  color[ 3 ] = 1;

  position = cg.snap->ps.persistant[ PERS_QUEUEPOS ] + 1;
  if( position < 1 )
    return qfalse;
  remainder = position % 10;
  ordinal = "th";
  if( remainder == 1 )
    ordinal = "st";
  else if( remainder == 2 )
    ordinal = "nd";
  else if( remainder == 3 )
    ordinal = "rd";
  Com_sprintf( buffer, MAX_STRING_CHARS, "You are %d%s in the spawn queue",
               position, ordinal );

  w = UI_Text_Width( buffer, 0.7f, 0 );
  UI_Text_Paint( 320 - w / 2, 360, 0.7f, color, buffer, 0, 0, ITEM_TEXTSTYLE_SHADOWED );

  if( cg.snap->ps.persistant[ PERS_SPAWNS ] == 0 )
    Com_sprintf( buffer, MAX_STRING_CHARS, "There are no spawns remaining" );
  else if( cg.snap->ps.persistant[ PERS_SPAWNS ] == 1 )
    Com_sprintf( buffer, MAX_STRING_CHARS, "There is 1 spawn remaining" );
  else
    Com_sprintf( buffer, MAX_STRING_CHARS, "There are %d spawns remaining",
                 cg.snap->ps.persistant[ PERS_SPAWNS ] );

  w = UI_Text_Width( buffer, 0.7f, 0 );
  UI_Text_Paint( 320 - w / 2, 400, 0.7f, color, buffer, 0, 0, ITEM_TEXTSTYLE_SHADOWED );

  return qtrue;
}

//==================================================================================

#define SPECTATOR_STRING "SPECTATOR"
/*
=================
CG_Draw2D
=================
*/
static void CG_Draw2D( void )
{
  vec4_t    color;
  float     w;
  menuDef_t *menu = NULL, *defaultMenu;

  color[ 0 ] = color[ 1 ] = color[ 2 ] = color[ 3 ] = 1.0f;

  // if we are taking a levelshot for the menu, don't draw anything
  if( cg.levelShot )
    return;

  if( cg_draw2D.integer == 0 )
    return;

  if( cg.snap->ps.pm_type == PM_INTERMISSION )
  {
    CG_DrawIntermission( );
    return;
  }

  CG_DrawLighting( );

  defaultMenu = Menus_FindByName( "default_hud" );

  if( cg.snap->ps.persistant[ PERS_SPECSTATE ] != SPECTATOR_NOT )
  {
    w = UI_Text_Width( SPECTATOR_STRING, 0.7f, 0 );
    UI_Text_Paint( 320 - w / 2, 440, 0.7f, color, SPECTATOR_STRING, 0, 0, ITEM_TEXTSTYLE_SHADOWED );
  }
  else
    menu = Menus_FindByName( BG_ClassConfig( cg.predictedPlayerState.stats[ STAT_CLASS ] )->hudName );

  if( !( cg.snap->ps.stats[ STAT_STATE ] & SS_INFESTING ) &&
      !( cg.snap->ps.stats[ STAT_STATE ] & SS_HOVELING ) && menu &&
      ( cg.snap->ps.stats[ STAT_HEALTH ] > 0 ) )
  {
    CG_DrawBuildableStatus( );
    if( cg_drawStatus.integer )
      Menu_Paint( menu, qtrue );

  }
  else if( cg_drawStatus.integer )
    Menu_Paint( defaultMenu, qtrue );

  CG_DrawVote( );
  CG_DrawTeamVote( );
  CG_DrawFollow( );
  CG_DrawQueue( );

  // don't draw center string if scoreboard is up
  cg.scoreBoardShowing = CG_DrawScoreboard( );

  if( !cg.scoreBoardShowing )
    CG_DrawCenterString( );
}

/*
===============
CG_ScalePainBlendTCs
===============
*/
static void CG_ScalePainBlendTCs( float* s1, float *t1, float *s2, float *t2 )
{
  *s1 -= 0.5f;
  *t1 -= 0.5f;
  *s2 -= 0.5f;
  *t2 -= 0.5f;

  *s1 *= cg_painBlendZoom.value;
  *t1 *= cg_painBlendZoom.value;
  *s2 *= cg_painBlendZoom.value;
  *t2 *= cg_painBlendZoom.value;

  *s1 += 0.5f;
  *t1 += 0.5f;
  *s2 += 0.5f;
  *t2 += 0.5f;
}

#define PAINBLEND_BORDER    0.15f

/*
===============
CG_PainBlend
===============
*/
static void CG_PainBlend( void )
{
  vec4_t      color;
  int         damage;
  float       damageAsFracOfMax;
  qhandle_t   shader = cgs.media.viewBloodShader;
  float       x, y, w, h;
  float       s1, t1, s2, t2;

  if( cg.snap->ps.persistant[ PERS_SPECSTATE ] != SPECTATOR_NOT || cg.intermissionStarted )
    return;

  damage = cg.lastHealth - cg.snap->ps.stats[ STAT_HEALTH ];

  if( damage < 0 )
    damage = 0;

  damageAsFracOfMax = (float)damage / cg.snap->ps.stats[ STAT_MAX_HEALTH ];
  cg.lastHealth = cg.snap->ps.stats[ STAT_HEALTH ];

  cg.painBlendValue += damageAsFracOfMax * cg_painBlendScale.value;

  if( cg.painBlendValue > 0.0f )
  {
    cg.painBlendValue -= ( cg.frametime / 1000.0f ) *
      cg_painBlendDownRate.value;
  }

  if( cg.painBlendValue > 1.0f )
    cg.painBlendValue = 1.0f;
  else if( cg.painBlendValue <= 0.0f )
  {
    cg.painBlendValue = 0.0f;
    return;
  }

  if( cg.snap->ps.stats[ STAT_TEAM ] == TEAM_ALIENS )
    VectorSet( color, 0.43f, 0.8f, 0.37f );
  else if( cg.snap->ps.stats[ STAT_TEAM ] == TEAM_HUMANS )
    VectorSet( color, 0.8f, 0.0f, 0.0f );

  if( cg.painBlendValue > cg.painBlendTarget )
  {
    cg.painBlendTarget += ( cg.frametime / 1000.0f ) *
      cg_painBlendUpRate.value;
  }
  else if( cg.painBlendValue < cg.painBlendTarget )
    cg.painBlendTarget = cg.painBlendValue;

  if( cg.painBlendTarget > cg_painBlendMax.value )
    cg.painBlendTarget = cg_painBlendMax.value;

  color[ 3 ] = cg.painBlendTarget;

  trap_R_SetColor( color );

  //left
  x = 0.0f; y = 0.0f;
  w = PAINBLEND_BORDER * 640.0f; h = 480.0f;
  CG_AdjustFrom640( &x, &y, &w, &h );
  s1 = 0.0f; t1 = 0.0f;
  s2 = PAINBLEND_BORDER; t2 = 1.0f;
  CG_ScalePainBlendTCs( &s1, &t1, &s2, &t2 );
  trap_R_DrawStretchPic( x, y, w, h, s1, t1, s2, t2, shader );

  //right
  x = 640.0f - ( PAINBLEND_BORDER * 640.0f ); y = 0.0f;
  w = PAINBLEND_BORDER * 640.0f; h = 480.0f;
  CG_AdjustFrom640( &x, &y, &w, &h );
  s1 = 1.0f - PAINBLEND_BORDER; t1 = 0.0f;
  s2 = 1.0f; t2 =1.0f;
  CG_ScalePainBlendTCs( &s1, &t1, &s2, &t2 );
  trap_R_DrawStretchPic( x, y, w, h, s1, t1, s2, t2, shader );

  //top
  x = PAINBLEND_BORDER * 640.0f; y = 0.0f;
  w = 640.0f - ( 2 * PAINBLEND_BORDER * 640.0f ); h = PAINBLEND_BORDER * 480.0f;
  CG_AdjustFrom640( &x, &y, &w, &h );
  s1 = PAINBLEND_BORDER; t1 = 0.0f;
  s2 = 1.0f - PAINBLEND_BORDER; t2 = PAINBLEND_BORDER;
  CG_ScalePainBlendTCs( &s1, &t1, &s2, &t2 );
  trap_R_DrawStretchPic( x, y, w, h, s1, t1, s2, t2, shader );

  //bottom
  x = PAINBLEND_BORDER * 640.0f; y = 480.0f - ( PAINBLEND_BORDER * 480.0f );
  w = 640.0f - ( 2 * PAINBLEND_BORDER * 640.0f ); h = PAINBLEND_BORDER * 480.0f;
  CG_AdjustFrom640( &x, &y, &w, &h );
  s1 = PAINBLEND_BORDER; t1 = 1.0f - PAINBLEND_BORDER;
  s2 = 1.0f - PAINBLEND_BORDER; t2 = 1.0f;
  CG_ScalePainBlendTCs( &s1, &t1, &s2, &t2 );
  trap_R_DrawStretchPic( x, y, w, h, s1, t1, s2, t2, shader );

  trap_R_SetColor( NULL );
}

/*
=====================
CG_ResetPainBlend
=====================
*/
void CG_ResetPainBlend( void )
{
  cg.painBlendValue = 0.0f;
  cg.painBlendTarget = 0.0f;
  cg.lastHealth = cg.snap->ps.stats[ STAT_HEALTH ];
}

/*
=====================
CG_DrawActive

Perform all drawing needed to completely fill the screen
=====================
*/
void CG_DrawActive( stereoFrame_t stereoView )
{
  float   separation;
  vec3_t    baseOrg;

  // optionally draw the info screen instead
  if( !cg.snap )
    return;

  switch ( stereoView )
  {
    case STEREO_CENTER:
      separation = 0;
      break;
    case STEREO_LEFT:
      separation = -cg_stereoSeparation.value / 2;
      break;
    case STEREO_RIGHT:
      separation = cg_stereoSeparation.value / 2;
      break;
    default:
      separation = 0;
      CG_Error( "CG_DrawActive: Undefined stereoView" );
  }

  // clear around the rendered view if sized down
  CG_TileClear( );

  // offset vieworg appropriately if we're doing stereo separation
  VectorCopy( cg.refdef.vieworg, baseOrg );

  if( separation != 0 )
    VectorMA( cg.refdef.vieworg, -separation, cg.refdef.viewaxis[ 1 ],
              cg.refdef.vieworg );

  // draw 3D view
  trap_R_RenderScene( &cg.refdef );

  // restore original viewpoint if running stereo
  if( separation != 0 )
    VectorCopy( baseOrg, cg.refdef.vieworg );

  // first person blend blobs, done after AnglesToAxis
  if( !cg.renderingThirdPerson )
    CG_PainBlend( );

  // draw status bar and other floating elements
  CG_Draw2D( );
}

