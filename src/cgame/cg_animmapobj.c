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

/*
===============
CG_RunAMOLerpFrame

Sets cg.snap, cg.oldFrame, and cg.backlerp
cg.time should be between oldFrameTime and frameTime after exit
===============
*/
static void CG_RunAMOLerpFrame( lerpFrame_t *lf )
{
  int     f, numFrames;
  animation_t *anim;

  // debugging tool to get no animations
  if ( cg_animSpeed.integer == 0 ) {
    lf->oldFrame = lf->frame = lf->backlerp = 0;
    return;
  }

  // if we have passed the current frame, move it to
  // oldFrame and calculate a new frame
  if ( cg.time >= lf->frameTime ) {
    lf->oldFrame = lf->frame;
    lf->oldFrameTime = lf->frameTime;

    // get the next frame based on the animation
    anim = lf->animation;
    if ( !anim->frameLerp ) {
      return;   // shouldn't happen
    }
    if ( cg.time < lf->animationTime ) {
      lf->frameTime = lf->animationTime;    // initial lerp
    } else {
      lf->frameTime = lf->oldFrameTime + anim->frameLerp;
    }
    f = ( lf->frameTime - lf->animationTime ) / anim->frameLerp;
    numFrames = anim->numFrames;
    if (anim->flipflop) {
      numFrames *= 2;
    }
    if ( f >= numFrames ) {
      f -= numFrames;
      if ( anim->loopFrames ) {
        f %= anim->loopFrames;
        f += anim->numFrames - anim->loopFrames;
      } else {
        f = numFrames - 1;
        // the animation is stuck at the end, so it
        // can immediately transition to another sequence
        lf->frameTime = cg.time;
      }
    }
    if ( anim->reversed ) {
      lf->frame = anim->firstFrame + anim->numFrames - 1 - f;
    }
    else if (anim->flipflop && f>=anim->numFrames) {
      lf->frame = anim->firstFrame + anim->numFrames - 1 - (f%anim->numFrames);
    }
    else {
      lf->frame = anim->firstFrame + f;
    }
    if ( cg.time > lf->frameTime ) {
      lf->frameTime = cg.time;
      if ( cg_debugAnim.integer ) {
        CG_Printf( "Clamp lf->frameTime\n");
      }
    }
  }

  if ( lf->frameTime > cg.time + 200 ) {
    lf->frameTime = cg.time;
  }

  if ( lf->oldFrameTime > cg.time ) {
    lf->oldFrameTime = cg.time;
  }
  // calculate current lerp value
  if ( lf->frameTime == lf->oldFrameTime ) {
    lf->backlerp = 0;
  } else {
    lf->backlerp = 1.0 - (float)( cg.time - lf->oldFrameTime ) / ( lf->frameTime - lf->oldFrameTime );
  }
}


/*
===============
CG_AMOAnimation
===============
*/
static void CG_AMOAnimation( centity_t *cent, int *old, int *now, float *backLerp )
{
  if( !( cent->currentState.eFlags & EF_MOVER_STOP ) )
    CG_RunAMOLerpFrame( &cent->lerpFrame );
  else
  {
    //FIXME: fiddle with params so that when anim is resumed the lerp doesn't freak
    //       cos of the time differential
/*    cent->lerpFrame.oldFrameTime = cg.time;
    cent->lerpFrame.frameTime = cg.time + 50;*/
  }

  *old      = cent->lerpFrame.oldFrame;
  *now      = cent->lerpFrame.frame;
  *backLerp = cent->lerpFrame.backlerp;
}


/*
==================
CG_animMapObj
==================
*/
void CG_animMapObj( centity_t *cent )
{
  refEntity_t     ent;
  entityState_t   *es;
  float           scale;
  animation_t     anim;

  es = &cent->currentState;

  // if set to invisible, skip
  if ( !es->modelindex || ( es->eFlags & EF_NODRAW ) )
    return;

  memset( &ent, 0, sizeof( ent ) );

  VectorCopy( es->angles, cent->lerpAngles );
  AnglesToAxis( cent->lerpAngles, ent.axis );

  ent.hModel = cgs.gameModels[ es->modelindex ];

  VectorCopy( cent->lerpOrigin, ent.origin);
  VectorCopy( cent->lerpOrigin, ent.oldorigin);

  ent.nonNormalizedAxes = qfalse;

  //scale the model
  if( es->angles2[ 0 ] )
  {
    scale = es->angles2[ 0 ];
    VectorScale( ent.axis[0], scale, ent.axis[0] );
    VectorScale( ent.axis[1], scale, ent.axis[1] );
    VectorScale( ent.axis[2], scale, ent.axis[2] );
    ent.nonNormalizedAxes = qtrue;
  } 

  //setup animation
  anim.firstFrame = es->powerups;
  anim.numFrames = es->weapon;
  anim.reversed = qfalse;
  anim.flipflop = qfalse;
  
  // if numFrames is negative the animation is reversed
  if( anim.numFrames < 0 )
  {
    anim.numFrames = -anim.numFrames;
    anim.reversed = qtrue;
  }
  
  anim.loopFrames = es->torsoAnim;

  if( !es->legsAnim )
  {
    anim.frameLerp = 1000;
    anim.initialLerp = 1000;
  }
  else
  {
    anim.frameLerp = 1000 / es->legsAnim;
    anim.initialLerp = 1000 / es->legsAnim;
  }

  cent->lerpFrame.animation = &anim;

  //run animation
  CG_AMOAnimation( cent, &ent.oldframe, &ent.frame, &ent.backlerp );

  // add to refresh list
  trap_R_AddRefEntityToScene(&ent);
}
