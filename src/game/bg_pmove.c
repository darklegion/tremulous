// Copyright (C) 1999-2000 Id Software, Inc.
//
// bg_pmove.c -- both games player movement code
// takes a playerstate and a usercmd as input and returns a modifed playerstate

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
                  
#include "q_shared.h"
#include "bg_public.h"
#include "bg_local.h"

pmove_t     *pm;
pml_t       pml;

// movement parameters
float pm_stopspeed = 100.0f;
float pm_duckScale = 0.25f;
float pm_swimScale = 0.50f;
float pm_wadeScale = 0.70f;

float pm_accelerate = 10.0f;
float pm_airaccelerate = 1.0f;
float pm_wateraccelerate = 4.0f;
float pm_flyaccelerate = 4.0f;

float pm_friction = 6.0f;
float pm_waterfriction = 1.0f;
float pm_flightfriction = 6.0f;
float pm_spectatorfriction = 5.0f;

int   c_pmove = 0;

/*
===============
PM_AddEvent

===============
*/
void PM_AddEvent( int newEvent ) {
  BG_AddPredictableEventToPlayerstate( newEvent, 0, pm->ps );
}

/*
===============
PM_AddTouchEnt
===============
*/
void PM_AddTouchEnt( int entityNum ) {
  int   i;

  if ( entityNum == ENTITYNUM_WORLD ) {
    return;
  }
  if ( pm->numtouch == MAXTOUCH ) {
    return;
  }

  // see if it is already added
  for ( i = 0 ; i < pm->numtouch ; i++ ) {
    if ( pm->touchents[ i ] == entityNum ) {
      return;
    }
  }

  // add it
  pm->touchents[pm->numtouch] = entityNum;
  pm->numtouch++;
}

/*
===================
PM_StartTorsoAnim
===================
*/
static void PM_StartTorsoAnim( int anim ) {
  if ( pm->ps->pm_type >= PM_DEAD ) {
    return;
  }
  pm->ps->torsoAnim = ( ( pm->ps->torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT )
    | anim;
}
static void PM_StartLegsAnim( int anim ) {
  if ( pm->ps->pm_type >= PM_DEAD ) {
    return;
  }
  if ( pm->ps->legsTimer > 0 ) {
    return;   // a high priority animation is running
  }
  pm->ps->legsAnim = ( ( pm->ps->legsAnim & ( ANIM_TOGGLEBIT | ANIM_WALLCLIMBING ) ) ^ ANIM_TOGGLEBIT )
    | anim;
}

static void PM_ContinueLegsAnim( int anim ) {
  if ( ( pm->ps->legsAnim & ~( ANIM_TOGGLEBIT | ANIM_WALLCLIMBING ) ) == anim ) {
    return;
  }
  if ( pm->ps->legsTimer > 0 ) {
    return;   // a high priority animation is running
  }
  PM_StartLegsAnim( anim );
}

static void PM_ContinueTorsoAnim( int anim ) {
  if ( ( pm->ps->torsoAnim & ~( ANIM_TOGGLEBIT | ANIM_WALLCLIMBING ) ) == anim ) {
    return;
  }
  if ( pm->ps->torsoTimer > 0 ) {
    return;   // a high priority animation is running
  }
  PM_StartTorsoAnim( anim );
}

static void PM_ForceLegsAnim( int anim ) {
  pm->ps->legsTimer = 0;
  PM_StartLegsAnim( anim );
}


/*
==================
PM_ClipVelocity

Slide off of the impacting surface
==================
*/
void PM_ClipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce ) {
  float backoff;
  float change;
  int   i;

  backoff = DotProduct (in, normal);

  //Com_Printf( "%1.0f ", backoff );

  if ( backoff < 0 ) {
    backoff *= overbounce;
  } else {
    backoff /= overbounce;
  }

  for ( i=0 ; i<3 ; i++ ) {
    change = normal[i]*backoff;
    //Com_Printf( "%1.0f ", change );
    out[i] = in[i] - change;
  }

  //Com_Printf( "   " );
}


/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
static void PM_Friction( void ) {
  vec3_t  vec;
  float *vel;
  float speed, newspeed, control;
  float drop;

  vel = pm->ps->velocity;

  //TA: make sure vertical velocity is NOT set to zero when wall climbing
  VectorCopy( vel, vec );
  if ( pml.walking &&
       !( pm->ps->stats[ STAT_STATE ] & SS_WALLCLIMBING ) ) {
    vec[2] = 0; // ignore slope movement
  }

  speed = VectorLength(vec);
  if (speed < 1) {
    vel[0] = 0;
    vel[1] = 0;   // allow sinking underwater
    // FIXME: still have z friction underwater?
    return;
  }

  drop = 0;

  // apply ground friction
  if ( pm->waterlevel <= 1 ) {
    if ( pml.walking && !(pml.groundTrace.surfaceFlags & SURF_SLICK) ) {
      // if getting knocked back, no friction
      if ( ! (pm->ps->pm_flags & PMF_TIME_KNOCKBACK) )
      {
        float sticky = BG_FindStickyForClass( pm->ps->stats[ STAT_PCLASS ] );
        
        control = speed < pm_stopspeed*sticky ? pm_stopspeed*sticky : speed;
        drop += control*pm_friction*pml.frametime;
      }
    }
  }

  // apply water friction even if just wading
  if ( pm->waterlevel ) {
    drop += speed*pm_waterfriction*pm->waterlevel*pml.frametime;
  }

  // apply flying friction
  if( BG_gotItem( UP_JETPACK, pm->ps->stats ) && BG_activated( UP_JETPACK, pm->ps->stats ) )
    drop += speed * pm_flightfriction * pml.frametime;

  if ( pm->ps->pm_type == PM_SPECTATOR ) {
    drop += speed*pm_spectatorfriction*pml.frametime;
  }

  // scale the velocity
  newspeed = speed - drop;
  if (newspeed < 0) {
    newspeed = 0;
  }
  newspeed /= speed;

  vel[0] = vel[0] * newspeed;
  vel[1] = vel[1] * newspeed;
  vel[2] = vel[2] * newspeed;
}


/*
==============
PM_Accelerate

Handles user intended acceleration
==============
*/
static void PM_Accelerate( vec3_t wishdir, float wishspeed, float accel ) {
#if 1
  // q2 style
  int     i;
  float   addspeed, accelspeed, currentspeed;

  currentspeed = DotProduct (pm->ps->velocity, wishdir);
  addspeed = wishspeed - currentspeed;
  if (addspeed <= 0) {
    return;
  }
  accelspeed = accel*pml.frametime*wishspeed;
  if (accelspeed > addspeed) {
    accelspeed = addspeed;
  }

  for (i=0 ; i<3 ; i++) {
    pm->ps->velocity[i] += accelspeed*wishdir[i];
  }
#else
  // proper way (avoids strafe jump maxspeed bug), but feels bad
  vec3_t    wishVelocity;
  vec3_t    pushDir;
  float   pushLen;
  float   canPush;

  VectorScale( wishdir, wishspeed, wishVelocity );
  VectorSubtract( wishVelocity, pm->ps->velocity, pushDir );
  pushLen = VectorNormalize( pushDir );

  canPush = accel*pml.frametime*wishspeed;
  if (canPush > pushLen) {
    canPush = pushLen;
  }

  VectorMA( pm->ps->velocity, canPush, pushDir, pm->ps->velocity );
#endif
}



/*
============
PM_CmdScale

Returns the scale factor to apply to cmd movements
This allows the clients to use axial -127 to 127 values for all directions
without getting a sqrt(2) distortion in speed.
============
*/
static float PM_CmdScale( usercmd_t *cmd ) {
  int   max;
  float total;
  float scale;
  float modifier = 1.0f;
  static int time;
  int   dTime;
  int   aForward, aRight;

  dTime = pm->cmd.serverTime - time;
  time = pm->cmd.serverTime;

  if( pm->ps->stats[ STAT_PTEAM ] == PTE_HUMANS && pm->ps->pm_type == PM_NORMAL &&
      !( BG_gotItem( UP_JETPACK, pm->ps->stats ) && BG_activated( UP_JETPACK, pm->ps->stats ) ) )
  {
    if( !( pm->ps->stats[ STAT_STATE ] & SS_SPEEDBOOST ) )
    {
      //if not sprinting
      modifier *= 0.8;
    }

    if( cmd->forwardmove < 0 )
    {
      //can't run backwards
      modifier *= 0.5;
    }
    else if( cmd->rightmove )
    {
      //can't move that fast sideways
      modifier *= 0.75;
    }

    //must have +ve stamina to jump
    if( pm->ps->stats[ STAT_STAMINA ] < 0 )
      cmd->upmove = 0;

    //slow down once stamina depletes
    if( pm->ps->stats[ STAT_STAMINA ] <= -500 )
      modifier *= (float)( pm->ps->stats[ STAT_STAMINA ] + 1000 ) / 500.0f;
  }

  if( pm->ps->pm_type == PM_GRABBED )
    modifier = 0.0f;

  if( !BG_ClassHasAbility( pm->ps->stats[ STAT_PCLASS ], SCA_CANJUMP ) )
    cmd->upmove = 0;

  max = abs( cmd->forwardmove );
  if ( abs( cmd->rightmove ) > max ) {
    max = abs( cmd->rightmove );
  }
  if ( abs( cmd->upmove ) > max ) {
    max = abs( cmd->upmove );
  }
  if ( !max ) {
    return 0;
  }

  total = sqrt( cmd->forwardmove * cmd->forwardmove
    + cmd->rightmove * cmd->rightmove + cmd->upmove * cmd->upmove );

  scale = (float)pm->ps->speed * max / ( 127.0 * total ) * modifier;

  return scale;
}


/*
================
PM_SetMovementDir

Determine the rotation of the legs reletive
to the facing dir
================
*/
static void PM_SetMovementDir( void ) {
  if ( pm->cmd.forwardmove || pm->cmd.rightmove ) {
    if ( pm->cmd.rightmove == 0 && pm->cmd.forwardmove > 0 ) {
      pm->ps->movementDir = 0;
    } else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove > 0 ) {
      pm->ps->movementDir = 1;
    } else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove == 0 ) {
      pm->ps->movementDir = 2;
    } else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove < 0 ) {
      pm->ps->movementDir = 3;
    } else if ( pm->cmd.rightmove == 0 && pm->cmd.forwardmove < 0 ) {
      pm->ps->movementDir = 4;
    } else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove < 0 ) {
      pm->ps->movementDir = 5;
    } else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove == 0 ) {
      pm->ps->movementDir = 6;
    } else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove > 0 ) {
      pm->ps->movementDir = 7;
    }
  } else {
    // if they aren't actively going directly sideways,
    // change the animation to the diagonal so they
    // don't stop too crooked
    if ( pm->ps->movementDir == 2 ) {
      pm->ps->movementDir = 1;
    } else if ( pm->ps->movementDir == 6 ) {
      pm->ps->movementDir = 7;
    }
  }
}


/*
=============
PM_CheckPounce
=============
*/
static qboolean PM_CheckPounce( void )
{
  vec3_t forward;
  
  if( pm->ps->weapon != WP_POUNCE && pm->ps->weapon != WP_POUNCE_UPG )
    return qfalse;

  if( pm->cmd.buttons & BUTTON_ATTACK2 )
  {
    pm->ps->pm_flags &= ~PMF_POUNCE;
    return qfalse;
  }

  if( pm->ps->pm_flags & PMF_POUNCE )
    return qfalse;

  if( pm->ps->stats[ STAT_MISC ] == 0 )
    return qfalse;

  pml.groundPlane = qfalse;   // jumping away
  pml.walking = qfalse;
  
  pm->ps->pm_flags |= PMF_POUNCE;
  
  pm->ps->groundEntityNum = ENTITYNUM_NONE;
  
  AngleVectors( pm->ps->viewangles, forward, NULL, NULL );
  VectorMA( pm->ps->velocity, pm->ps->stats[ STAT_MISC ], forward, pm->ps->velocity );
  pm->ps->velocity[ 2 ] += JUMP_VELOCITY / 2;
  
  PM_AddEvent( EV_JUMP );
  
  return qtrue;
}

/*
=============
PM_CheckJump
=============
*/
static qboolean PM_CheckJump( void )
{
  if( !BG_ClassHasAbility( pm->ps->stats[ STAT_PCLASS ], SCA_CANJUMP ) )
    return qfalse;

  //can't jump and pounce charge at the same time
  if( ( pm->ps->weapon == WP_POUNCE || pm->ps->weapon == WP_POUNCE_UPG ) && pm->ps->stats[ STAT_MISC ] > 0 )
    return qfalse;

  if( ( pm->ps->stats[ STAT_PTEAM ] == PTE_HUMANS ) &&
      ( pm->ps->stats[ STAT_STAMINA ] < 0 ) )
    return qfalse;

  if ( pm->ps->pm_flags & PMF_RESPAWNED ) {
    return qfalse;    // don't allow jump until all buttons are up
  }

  if ( pm->cmd.upmove < 10 ) {
    // not holding jump
    return qfalse;
  }

  //can't jump whilst grabbed
  if( pm->ps->pm_type == PM_GRABBED )
  {
    pm->cmd.upmove = 0;
    return qfalse;
  }

  // must wait for jump to be released
  if ( pm->ps->pm_flags & PMF_JUMP_HELD ) {
    // clear upmove so cmdscale doesn't lower running speed
    pm->cmd.upmove = 0;
    return qfalse;
  }

  pml.groundPlane = qfalse;   // jumping away
  pml.walking = qfalse;
  pm->ps->pm_flags |= PMF_JUMP_HELD;

  //TA: take some stamina off
  if( pm->ps->stats[ STAT_PTEAM ] == PTE_HUMANS )
    pm->ps->stats[ STAT_STAMINA ] -= 500;

  pm->ps->groundEntityNum = ENTITYNUM_NONE;
  
  //TA: jump away from wall
  if( pm->ps->stats[ STAT_STATE ] & SS_WALLCLIMBING )
  {
    vec3_t normal = { 0, 0, -1 };
    
    if( !( pm->ps->stats[ STAT_STATE ] & SS_WALLCLIMBINGCEILING ) )
      VectorCopy( pm->ps->grapplePoint, normal );
      
    VectorMA( pm->ps->velocity, JUMP_VELOCITY, normal, pm->ps->velocity );
  }
  else
    pm->ps->velocity[2] = JUMP_VELOCITY;
    
  PM_AddEvent( EV_JUMP );

  if ( pm->cmd.forwardmove >= 0 ) {
    PM_ForceLegsAnim( LEGS_JUMP );
    pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
  } else {
    PM_ForceLegsAnim( LEGS_JUMPB );
    pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
  }

  return qtrue;
}

/*
=============
PM_CheckWaterJump
=============
*/
static qboolean PM_CheckWaterJump( void ) {
  vec3_t  spot;
  int   cont;
  vec3_t  flatforward;

  if (pm->ps->pm_time) {
    return qfalse;
  }

  // check for water jump
  if ( pm->waterlevel != 2 ) {
    return qfalse;
  }

  flatforward[0] = pml.forward[0];
  flatforward[1] = pml.forward[1];
  flatforward[2] = 0;
  VectorNormalize (flatforward);

  VectorMA (pm->ps->origin, 30, flatforward, spot);
  spot[2] += 4;
  cont = pm->pointcontents (spot, pm->ps->clientNum );
  if ( !(cont & CONTENTS_SOLID) ) {
    return qfalse;
  }

  spot[2] += 16;
  cont = pm->pointcontents (spot, pm->ps->clientNum );
  if ( cont ) {
    return qfalse;
  }

  // jump out of water
  VectorScale (pml.forward, 200, pm->ps->velocity);
  pm->ps->velocity[2] = 350;

  pm->ps->pm_flags |= PMF_TIME_WATERJUMP;
  pm->ps->pm_time = 2000;

  return qtrue;
}

//============================================================================


/*
===================
PM_WaterJumpMove

Flying out of the water
===================
*/
static void PM_WaterJumpMove( void ) {
  // waterjump has no control, but falls

  PM_StepSlideMove( qtrue, qfalse );

  pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
  if (pm->ps->velocity[2] < 0) {
    // cancel as soon as we are falling down again
    pm->ps->pm_flags &= ~PMF_ALL_TIMES;
    pm->ps->pm_time = 0;
  }
}

/*
===================
PM_WaterMove

===================
*/
static void PM_WaterMove( void ) {
  int   i;
  vec3_t  wishvel;
  float wishspeed;
  vec3_t  wishdir;
  float scale;
  float vel;

  if ( PM_CheckWaterJump() ) {
    PM_WaterJumpMove();
    return;
  }
#if 0
  // jump = head for surface
  if ( pm->cmd.upmove >= 10 ) {
    if (pm->ps->velocity[2] > -300) {
      if ( pm->watertype == CONTENTS_WATER ) {
        pm->ps->velocity[2] = 100;
      } else if (pm->watertype == CONTENTS_SLIME) {
        pm->ps->velocity[2] = 80;
      } else {
        pm->ps->velocity[2] = 50;
      }
    }
  }
#endif
  PM_Friction ();

  scale = PM_CmdScale( &pm->cmd );
  //
  // user intentions
  //
  if ( !scale ) {
    wishvel[0] = 0;
    wishvel[1] = 0;
    wishvel[2] = -60;   // sink towards bottom
  } else {
    for (i=0 ; i<3 ; i++)
      wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove + scale * pml.right[i]*pm->cmd.rightmove;

    wishvel[2] += scale * pm->cmd.upmove;
  }

  VectorCopy (wishvel, wishdir);
  wishspeed = VectorNormalize(wishdir);

  if ( wishspeed > pm->ps->speed * pm_swimScale ) {
    wishspeed = pm->ps->speed * pm_swimScale;
  }

  PM_Accelerate (wishdir, wishspeed, pm_wateraccelerate);

  // make sure we can go up slopes easily under water
  if ( pml.groundPlane && DotProduct( pm->ps->velocity, pml.groundTrace.plane.normal ) < 0 ) {
    vel = VectorLength(pm->ps->velocity);
    // slide along the ground plane
    PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal,
      pm->ps->velocity, OVERCLIP );

    VectorNormalize(pm->ps->velocity);
    VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
  }

  PM_SlideMove( qfalse );
}

/*
===================
PM_JetPackMove

Only with the jetpack
===================
*/
static void PM_JetPackMove( void ) {
  int     i;
  vec3_t  wishvel;
  float   wishspeed;
  vec3_t  wishdir;
  float   scale;

  //normal slowdown
  PM_Friction( );

  scale = PM_CmdScale( &pm->cmd );
  
  // user intentions
  for( i = 0; i < 2; i++ )
    wishvel[ i ] = scale * pml.forward[ i ] * pm->cmd.forwardmove + scale * pml.right[ i ] * pm->cmd.rightmove;

  if( pm->cmd.upmove > 0.0f )
    wishvel[ 2 ] = 48.0f;
  if( pm->cmd.upmove < 0.0f )
    wishvel[ 2 ] = -96.0f;

  VectorCopy( wishvel, wishdir );
  wishspeed = VectorNormalize( wishdir );

  PM_Accelerate( wishdir, wishspeed, pm_flyaccelerate );

  PM_StepSlideMove( qfalse, qfalse );

  PM_ForceLegsAnim( LEGS_LAND );
}




/*
===================
PM_FlyMove

Only with the flight powerup
===================
*/
static void PM_FlyMove( void ) {
  int   i;
  vec3_t  wishvel;
  float wishspeed;
  vec3_t  wishdir;
  float scale;

  // normal slowdown
  PM_Friction ();

  scale = PM_CmdScale( &pm->cmd );
  //
  // user intentions
  //
  if ( !scale ) {
    wishvel[0] = 0;
    wishvel[1] = 0;
    wishvel[2] = 0;
  } else {
    for (i=0 ; i<3 ; i++) {
      wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove + scale * pml.right[i]*pm->cmd.rightmove;
    }

    wishvel[2] += scale * pm->cmd.upmove;
  }

  VectorCopy (wishvel, wishdir);
  wishspeed = VectorNormalize(wishdir);

  PM_Accelerate (wishdir, wishspeed, pm_flyaccelerate);

  PM_StepSlideMove( qfalse, qfalse );
}


/*
===================
PM_AirMove

===================
*/
static void PM_AirMove( void ) {
  int     i;
  vec3_t    wishvel;
  float   fmove, smove;
  vec3_t    wishdir;
  float   wishspeed;
  float   scale;
  usercmd_t cmd;

  PM_Friction();

  fmove = pm->cmd.forwardmove;
  smove = pm->cmd.rightmove;

  cmd = pm->cmd;
  scale = PM_CmdScale( &cmd );

  // set the movementDir so clients can rotate the legs for strafing
  PM_SetMovementDir();

  // project moves down to flat plane
  pml.forward[2] = 0;
  pml.right[2] = 0;
  VectorNormalize (pml.forward);
  VectorNormalize (pml.right);

  for ( i = 0 ; i < 2 ; i++ ) {
    wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
  }
  wishvel[2] = 0;

  VectorCopy (wishvel, wishdir);
  wishspeed = VectorNormalize(wishdir);
  wishspeed *= scale;

  // not on ground, so little effect on velocity
  PM_Accelerate (wishdir, wishspeed, pm_airaccelerate);

  // we may have a ground plane that is very steep, even
  // though we don't have a groundentity
  // slide along the steep plane
  if ( pml.groundPlane ) {
    PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal,
      pm->ps->velocity, OVERCLIP );
  }

#if 0
  //ZOID:  If we are on the grapple, try stair-stepping
  //this allows a player to use the grapple to pull himself
  //over a ledge
  if (pm->ps->pm_flags & PMF_GRAPPLE_PULL)
    PM_StepSlideMove ( qtrue );
  else
    PM_SlideMove ( qtrue );
#endif

  PM_StepSlideMove ( qtrue, qfalse );
}

/*
===================
PM_GrappleMove

===================
*/
static void PM_GrappleMove( void ) {
  vec3_t vel, v;
  float vlen;

  VectorScale(pml.forward, -16, v);
  VectorAdd(pm->ps->grapplePoint, v, v);
  VectorSubtract(v, pm->ps->origin, vel);
  vlen = VectorLength(vel);
  VectorNormalize( vel );

  if (vlen <= 100)
    VectorScale(vel, 10 * vlen, vel);
  else
    VectorScale(vel, 800, vel);

  VectorCopy(vel, pm->ps->velocity);

  pml.groundPlane = qfalse;
}


/*
===================
PM_ClimbMove

===================
*/
static void PM_ClimbMove( void ) {
  int     i;
  vec3_t    wishvel;
  float   fmove, smove;
  vec3_t    wishdir;
  float   wishspeed;
  float   scale;
  usercmd_t cmd;
  float   accelerate;
  float   vel;

  if ( pm->waterlevel > 2 && DotProduct( pml.forward, pml.groundTrace.plane.normal ) > 0 ) {
    // begin swimming
    PM_WaterMove();
    return;
  }


  if ( PM_CheckJump( ) || PM_CheckPounce( ) ) {
    // jumped away
    if ( pm->waterlevel > 1 ) {
      PM_WaterMove();
    } else {
      PM_AirMove();
    }
    return;
  }

  PM_Friction ();

  fmove = pm->cmd.forwardmove;
  smove = pm->cmd.rightmove;

  cmd = pm->cmd;
  scale = PM_CmdScale( &cmd );

  // set the movementDir so clients can rotate the legs for strafing
  PM_SetMovementDir();

  // project the forward and right directions onto the ground plane
  PM_ClipVelocity (pml.forward, pml.groundTrace.plane.normal, pml.forward, OVERCLIP );
  PM_ClipVelocity (pml.right, pml.groundTrace.plane.normal, pml.right, OVERCLIP );
  //
  VectorNormalize (pml.forward);
  VectorNormalize (pml.right);

  for ( i = 0 ; i < 3 ; i++ ) {
    wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
  }
  // when going up or down slopes the wish velocity should Not be zero
//  wishvel[2] = 0;

  VectorCopy (wishvel, wishdir);
  wishspeed = VectorNormalize(wishdir);
  wishspeed *= scale;

  // clamp the speed lower if ducking
  if ( pm->ps->pm_flags & PMF_DUCKED ) {
    if ( wishspeed > pm->ps->speed * pm_duckScale ) {
      wishspeed = pm->ps->speed * pm_duckScale;
    }
  }

  // clamp the speed lower if wading or walking on the bottom
  if ( pm->waterlevel ) {
    float waterScale;

    waterScale = pm->waterlevel / 3.0;
    waterScale = 1.0 - ( 1.0 - pm_swimScale ) * waterScale;
    if ( wishspeed > pm->ps->speed * waterScale ) {
      wishspeed = pm->ps->speed * waterScale;
    }
  }

  // when a player gets hit, they temporarily lose
  // full control, which allows them to be moved a bit
  if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK ) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) {
    accelerate = pm_airaccelerate;
  } else {
    accelerate = pm_accelerate;
  }

  PM_Accelerate (wishdir, wishspeed, accelerate);

  if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK ) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) {
    pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
  }

  vel = VectorLength(pm->ps->velocity);

  // slide along the ground plane
  PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal,
    pm->ps->velocity, OVERCLIP );
  
  // don't decrease velocity when going up or down a slope
  VectorNormalize(pm->ps->velocity);
  VectorScale(pm->ps->velocity, vel, pm->ps->velocity);

  // don't do anything if standing still
  if (!pm->ps->velocity[0] && !pm->ps->velocity[1] && !pm->ps->velocity[2]) {
    return;
  }

  PM_StepSlideMove( qfalse, qfalse );
}


/*
===================
PM_WalkMove

===================
*/
static void PM_WalkMove( void ) {
  int     i;
  vec3_t    wishvel;
  float   fmove, smove;
  vec3_t    wishdir;
  float   wishspeed;
  float   scale;
  usercmd_t cmd;
  float   accelerate;
  float   vel;

  if ( pm->waterlevel > 2 && DotProduct( pml.forward, pml.groundTrace.plane.normal ) > 0 ) {
    // begin swimming
    PM_WaterMove();
    return;
  }


  if( PM_CheckJump( ) || PM_CheckPounce( ) ) {
    // jumped away
    if ( pm->waterlevel > 1 ) {
      PM_WaterMove();
    } else {
      PM_AirMove();
    }
    return;
  }

  PM_Friction ();

  fmove = pm->cmd.forwardmove;
  smove = pm->cmd.rightmove;

  cmd = pm->cmd;
  scale = PM_CmdScale( &cmd );

  // set the movementDir so clients can rotate the legs for strafing
  PM_SetMovementDir();

  // project moves down to flat plane
  pml.forward[2] = 0;
  pml.right[2] = 0;

  // project the forward and right directions onto the ground plane
  PM_ClipVelocity (pml.forward, pml.groundTrace.plane.normal, pml.forward, OVERCLIP );
  PM_ClipVelocity (pml.right, pml.groundTrace.plane.normal, pml.right, OVERCLIP );
  //
  VectorNormalize (pml.forward);
  VectorNormalize (pml.right);

  for ( i = 0 ; i < 3 ; i++ ) {
    wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
  }
  // when going up or down slopes the wish velocity should Not be zero
//  wishvel[2] = 0;

  VectorCopy (wishvel, wishdir);
  wishspeed = VectorNormalize(wishdir);
  wishspeed *= scale;

  // clamp the speed lower if ducking
  if ( pm->ps->pm_flags & PMF_DUCKED ) {
    if ( wishspeed > pm->ps->speed * pm_duckScale ) {
      wishspeed = pm->ps->speed * pm_duckScale;
    }
  }

  // clamp the speed lower if wading or walking on the bottom
  if ( pm->waterlevel ) {
    float waterScale;

    waterScale = pm->waterlevel / 3.0;
    waterScale = 1.0 - ( 1.0 - pm_swimScale ) * waterScale;
    if ( wishspeed > pm->ps->speed * waterScale ) {
      wishspeed = pm->ps->speed * waterScale;
    }
  }

  // when a player gets hit, they temporarily lose
  // full control, which allows them to be moved a bit
  if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK ) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) {
    accelerate = pm_airaccelerate;
  } else {
    accelerate = pm_accelerate;
  }

  PM_Accelerate (wishdir, wishspeed, accelerate);

  //Com_Printf("velocity = %1.1f %1.1f %1.1f\n", pm->ps->velocity[0], pm->ps->velocity[1], pm->ps->velocity[2]);
  //Com_Printf("velocity1 = %1.1f\n", VectorLength(pm->ps->velocity));

  if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK ) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) {
    pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
  } else {
    // don't reset the z velocity for slopes
//    pm->ps->velocity[2] = 0;
  }

  vel = VectorLength(pm->ps->velocity);

  // slide along the ground plane
  PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal,
    pm->ps->velocity, OVERCLIP );

  // don't decrease velocity when going up or down a slope
  VectorNormalize(pm->ps->velocity);
  VectorScale(pm->ps->velocity, vel, pm->ps->velocity);

  // don't do anything if standing still
  if (!pm->ps->velocity[0] && !pm->ps->velocity[1]) {
    return;
  }

  PM_StepSlideMove( qfalse, qfalse );

  //Com_Printf("velocity2 = %1.1f\n", VectorLength(pm->ps->velocity));

}


/*
==============
PM_DeadMove
==============
*/
static void PM_DeadMove( void ) {
  float forward;

  if ( !pml.walking ) {
    return;
  }

  // extra friction

  forward = VectorLength (pm->ps->velocity);
  forward -= 20;
  if ( forward <= 0 ) {
    VectorClear (pm->ps->velocity);
  } else {
    VectorNormalize (pm->ps->velocity);
    VectorScale (pm->ps->velocity, forward, pm->ps->velocity);
  }
}


/*
===============
PM_NoclipMove
===============
*/
static void PM_NoclipMove( void ) {
  float speed, drop, friction, control, newspeed;
  int     i;
  vec3_t    wishvel;
  float   fmove, smove;
  vec3_t    wishdir;
  float   wishspeed;
  float   scale;

  pm->ps->viewheight = DEFAULT_VIEWHEIGHT;

  // friction

  speed = VectorLength (pm->ps->velocity);
  if (speed < 1)
  {
    VectorCopy (vec3_origin, pm->ps->velocity);
  }
  else
  {
    drop = 0;

    friction = pm_friction*1.5; // extra friction
    control = speed < pm_stopspeed ? pm_stopspeed : speed;
    drop += control*friction*pml.frametime;

    // scale the velocity
    newspeed = speed - drop;
    if (newspeed < 0)
      newspeed = 0;
    newspeed /= speed;

    VectorScale (pm->ps->velocity, newspeed, pm->ps->velocity);
  }

  // accelerate
  scale = PM_CmdScale( &pm->cmd );

  fmove = pm->cmd.forwardmove;
  smove = pm->cmd.rightmove;

  for (i=0 ; i<3 ; i++)
    wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
  wishvel[2] += pm->cmd.upmove;

  VectorCopy (wishvel, wishdir);
  wishspeed = VectorNormalize(wishdir);
  wishspeed *= scale;

  PM_Accelerate( wishdir, wishspeed, pm_accelerate );

  // move
  VectorMA (pm->ps->origin, pml.frametime, pm->ps->velocity, pm->ps->origin);
}

//============================================================================

/*
================
PM_FootstepForSurface

Returns an event number apropriate for the groundsurface
================
*/
static int PM_FootstepForSurface( void )
{
  //TA:
  if ( pm->ps->stats[ STAT_STATE ] & SS_CREEPSLOWED )
    return EV_FOOTSTEP_SQUELCH;
    
  if ( pml.groundTrace.surfaceFlags & SURF_NOSTEPS ) 
    return 0;
    
  if ( pml.groundTrace.surfaceFlags & SURF_METALSTEPS )
    return EV_FOOTSTEP_METAL;
    
  return EV_FOOTSTEP;
}


/*
=================
PM_CrashLand

Check for hard landings that generate sound events
=================
*/
static void PM_CrashLand( void ) {
  float   delta;
  float   dist;
  float   vel, acc;
  float   t;
  float   a, b, c, den;

  // decide which landing animation to use
  if ( pm->ps->pm_flags & PMF_BACKWARDS_JUMP ) {
    PM_ForceLegsAnim( LEGS_LANDB );
  } else {
    PM_ForceLegsAnim( LEGS_LAND );
  }

  pm->ps->legsTimer = TIMER_LAND;

  // calculate the exact velocity on landing
  dist = pm->ps->origin[2] - pml.previous_origin[2];
  vel = pml.previous_velocity[2];
  acc = -pm->ps->gravity;

  a = acc / 2;
  b = vel;
  c = -dist;

  den =  b * b - 4 * a * c;
  if ( den < 0 ) {
    return;
  }
  t = (-b - sqrt( den ) ) / ( 2 * a );

  delta = vel + t * acc;
  delta = delta*delta * 0.0001;

  // ducking while falling doubles damage
  if ( pm->ps->pm_flags & PMF_DUCKED ) {
    delta *= 2;
  }

  // never take falling damage if completely underwater
  if ( pm->waterlevel == 3 ) {
    return;
  }

  // reduce falling damage if there is standing water
  if ( pm->waterlevel == 2 ) {
    delta *= 0.25;
  }
  if ( pm->waterlevel == 1 ) {
    delta *= 0.5;
  }

  if ( delta < 1 ) {
    return;
  }

  // create a local entity event to play the sound

  // SURF_NODAMAGE is used for bounce pads where you don't ever
  // want to take damage or play a crunch sound
  if ( !(pml.groundTrace.surfaceFlags & SURF_NODAMAGE) )  {
    if ( delta > 60 ) {
      PM_AddEvent( EV_FALL_FAR );
    } else if ( delta > 40 ) {
      // this is a pain grunt, so don't play it if dead
      if ( pm->ps->stats[STAT_HEALTH] > 0 ) {
        PM_AddEvent( EV_FALL_MEDIUM );
      }
    } else if ( delta > 7 ) {
      PM_AddEvent( EV_FALL_SHORT );
    } else {
      PM_AddEvent( PM_FootstepForSurface() );
    }
  }

  // start footstep cycle over
  pm->ps->bobCycle = 0;
}


/*
=============
PM_CorrectAllSolid
=============
*/
static int PM_CorrectAllSolid( trace_t *trace ) {
  int     i, j, k;
  vec3_t    point;

  if ( pm->debugLevel ) {
    Com_Printf("%i:allsolid\n", c_pmove);
  }

  // jitter around
  for (i = -1; i <= 1; i++) {
    for (j = -1; j <= 1; j++) {
      for (k = -1; k <= 1; k++) {
        VectorCopy(pm->ps->origin, point);
        point[0] += (float) i;
        point[1] += (float) j;
        point[2] += (float) k;
        pm->trace (trace, point, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
        if ( !trace->allsolid ) {
          point[0] = pm->ps->origin[0];
          point[1] = pm->ps->origin[1];
          point[2] = pm->ps->origin[2] - 0.25;

          pm->trace (trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
          pml.groundTrace = *trace;
          return qtrue;
        }
      }
    }
  }

  pm->ps->groundEntityNum = ENTITYNUM_NONE;
  pml.groundPlane = qfalse;
  pml.walking = qfalse;

  return qfalse;
}


/*
=============
PM_GroundTraceMissed

The ground trace didn't hit a surface, so we are in freefall
=============
*/
static void PM_GroundTraceMissed( void ) {
  trace_t   trace;
  vec3_t    point;

  if ( pm->ps->groundEntityNum != ENTITYNUM_NONE ) {
    // we just transitioned into freefall
    if ( pm->debugLevel ) {
      Com_Printf("%i:lift\n", c_pmove);
    }

    // if they aren't in a jumping animation and the ground is a ways away, force into it
    // if we didn't do the trace, the player would be backflipping down staircases
    VectorCopy( pm->ps->origin, point );
    point[2] -= 64;

    pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
    if ( trace.fraction == 1.0 ) {
      if ( pm->cmd.forwardmove >= 0 ) {
        PM_ForceLegsAnim( LEGS_JUMP );
        pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
      } else {
        PM_ForceLegsAnim( LEGS_JUMPB );
        pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
      }
    }
  }

  pm->ps->groundEntityNum = ENTITYNUM_NONE;
  pml.groundPlane = qfalse;
  pml.walking = qfalse;
}


/*
=============
PM_GroundClimbTrace
=============
*/
static void PM_GroundClimbTrace( void )
{
  vec3_t    surfNormal, movedir, lookdir, forward, right, point;
  vec3_t    refNormal = { 0.0f, 0.0f, 1.0f };
  vec3_t    ceilingNormal = { 0.0f, 0.0f, -1.0f };
  float     toAngles[3], surfAngles[3];
  trace_t   trace;
  int       i;

  //used for delta correction
  vec3_t    traceCROSSsurf, traceCROSSref, surfCROSSref;
  float     traceDOTsurf, traceDOTref, surfDOTref, rTtDOTrTsTt;
  float     traceANGsurf, traceANGref, surfANGref;
  vec3_t    horizontal = { 1.0f, 0.0f, 0.0f }; //arbituary vector perpendicular to refNormal
  vec3_t    refTOtrace, refTOsurfTOtrace, tempVec;
  int       rTtANGrTsTt;
  float     ldDOTtCs, d;
  vec3_t    abc;

  //TA: If we're on the ceiling then grapplePoint is a rotation normal.. otherwise its a surface normal.
  //    would have been nice if Carmack had left a few random variables in the ps struct for mod makers
  if( pm->ps->stats[ STAT_STATE ] & SS_WALLCLIMBINGCEILING )
    VectorCopy( ceilingNormal, surfNormal );
  else
    VectorCopy( pm->ps->grapplePoint, surfNormal );

  //construct a vector which reflects the direction the player is looking wrt the surface normal
  AngleVectors( pm->ps->viewangles, forward, NULL, NULL );
  CrossProduct( forward, surfNormal, right );
  VectorNormalize( right );
  CrossProduct( surfNormal, right, movedir );
  VectorNormalize( movedir );

  VectorCopy( movedir, lookdir );

  if( pm->cmd.forwardmove < 0 )
    VectorNegate( movedir, movedir );

  //allow strafe transitions
  if( pm->cmd.rightmove )
  {
    VectorCopy( right, movedir );
    
    if( pm->cmd.rightmove < 0 )
      VectorNegate( movedir, movedir );
  }
  
  for(i = 0; i <= 4; i++)
  {
    switch ( i )
    {
      case 0:
        //we are going to step this frame so skip the transition test
        if( PM_PredictStepMove( ) )
          continue;

        //trace into direction we are moving
        VectorMA( pm->ps->origin, 0.25f, movedir, point );
        pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
        break;

      case 1:
        //trace straight down anto "ground" surface
        VectorMA( pm->ps->origin, -0.25f, surfNormal, point );
        pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
        break;

      case 2:
        if( pml.groundPlane != qfalse && PM_PredictStepMove( ) )
        {
          //step down
          VectorMA( pm->ps->origin, -STEPSIZE, surfNormal, point );
          pm->trace( &trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask );
        }
        else
          continue;
        break;

      case 3:
        //trace "underneath" BBOX so we can traverse angles > 180deg
        if( pml.groundPlane != qfalse )
        {
          VectorMA( pm->ps->origin, -16.0f, surfNormal, point );
          VectorMA( point, -16.0f, movedir, point );
          pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
        }
        else
          continue;
        break;

      case 4:
        //fall back so we don't have to modify PM_GroundTrace too much
        VectorCopy( pm->ps->origin, point );
        point[2] = pm->ps->origin[2] - 0.25f;
        pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
        break;
    }

    //if we hit something
    if( trace.fraction < 1.0 && !( trace.surfaceFlags & ( SURF_SKY | SURF_SLICK ) ) &&
        !( trace.entityNum != ENTITYNUM_WORLD && i != 4 ) )
    {
      if( i == 2 || i == 3 )
      {
        if( i == 2 )
          PM_StepEvent( pm->ps->origin, trace.endpos, surfNormal );
        
        VectorCopy( trace.endpos, pm->ps->origin );
      }

      //calculate a bunch of stuff...
      CrossProduct( trace.plane.normal, surfNormal, traceCROSSsurf );
      VectorNormalize( traceCROSSsurf );

      CrossProduct( trace.plane.normal, refNormal, traceCROSSref );
      VectorNormalize( traceCROSSref );

      CrossProduct( surfNormal, refNormal, surfCROSSref );
      VectorNormalize( surfCROSSref );

      //calculate angle between surf and trace
      traceDOTsurf = DotProduct( trace.plane.normal, surfNormal );
      traceANGsurf = RAD2DEG( acos( traceDOTsurf ) );

      if( traceANGsurf > 180.0f )
        traceANGsurf -= 180.0f;

      //calculate angle between trace and ref
      traceDOTref = DotProduct( trace.plane.normal, refNormal );
      traceANGref = RAD2DEG( acos( traceDOTref ) );

      if( traceANGref > 180.0f )
        traceANGref -= 180.0f;

      //calculate angle between surf and ref
      surfDOTref = DotProduct( surfNormal, refNormal );
      surfANGref = RAD2DEG( acos( surfDOTref ) );

      if( surfANGref > 180.0f )
        surfANGref -= 180.0f;

      //if the trace result and old surface normal are different then we must have transided to a new
      //surface... do some stuff...
      if( !VectorCompare( trace.plane.normal, surfNormal ) )
      {
        //if the trace result or the old vector is not the floor or ceiling correct the YAW angle
        if( !VectorCompare( trace.plane.normal, refNormal ) && !VectorCompare( surfNormal, refNormal ) &&
            !VectorCompare( trace.plane.normal, ceilingNormal ) && !VectorCompare( surfNormal, ceilingNormal ) )
        {
          //behold the evil mindfuck from hell
          //it has fucked mind like nothing has fucked mind before
            
          //calculate reference rotated through to trace plane
          RotatePointAroundVector( refTOtrace, traceCROSSref, horizontal, -traceANGref );
          
          //calculate reference rotated through to surf plane then to trace plane
          RotatePointAroundVector( tempVec, surfCROSSref, horizontal, -surfANGref );
          RotatePointAroundVector( refTOsurfTOtrace, traceCROSSsurf, tempVec, -traceANGsurf );

          //calculate angle between refTOtrace and refTOsurfTOtrace
          rTtDOTrTsTt = DotProduct( refTOtrace, refTOsurfTOtrace );
          rTtANGrTsTt = ANGLE2SHORT( RAD2DEG( acos( rTtDOTrTsTt ) ) );

          if( rTtANGrTsTt > 32768 )
            rTtANGrTsTt -= 32768;

          //set the correction angle
          if( traceCROSSsurf[ 2 ] < 0 )
            rTtANGrTsTt = -rTtANGrTsTt;

          //phew! - correct the angle
          pm->ps->delta_angles[ YAW ] -= rTtANGrTsTt;
        }

        //construct a plane dividing the surf and trace normals
        CrossProduct( traceCROSSsurf, surfNormal, abc );
        VectorNormalize( abc );
        d = DotProduct( abc, pm->ps->origin );

        //construct a point representing where the player is looking
        VectorAdd( pm->ps->origin, lookdir, point );
        
        //check whether point is on one side of the plane, if so invert the correction angle
        if( ( abc[ 0 ] * point[ 0 ] + abc[ 1 ] * point[ 1 ] + abc[ 2 ] * point[ 2 ] - d ) > 0 )
          traceANGsurf = -traceANGsurf;
        
        //find the . product of the lookdir and traceCROSSsurf
        if( ( ldDOTtCs = DotProduct( lookdir, traceCROSSsurf ) ) < 0.0f )
        {
          VectorInverse( traceCROSSsurf );
          ldDOTtCs = DotProduct( lookdir, traceCROSSsurf );
        }
        
        //set the correction angle
        traceANGsurf *= 1.0f - ldDOTtCs;

        if( !( pm->ps->persistant[ PERS_STATE ] & PS_WALLCLIMBINGFOLLOW ) )
        {
          //correct the angle
          pm->ps->delta_angles[ PITCH ] -= ANGLE2SHORT( traceANGsurf );
        }

        //transition from wall to ceiling
        //normal for subsequent viewangle rotations
        if( VectorCompare( trace.plane.normal, ceilingNormal ) )
        {
          CrossProduct( surfNormal, trace.plane.normal, pm->ps->grapplePoint );
          VectorNormalize( pm->ps->grapplePoint );
          pm->ps->stats[ STAT_STATE ] |= SS_WALLCLIMBINGCEILING;
        }

        //transition from ceiling to wall
        //we need to do some different angle correction here cos GPISROTVEC
        if( VectorCompare( surfNormal, ceilingNormal ) )
        {
          vectoangles( trace.plane.normal, toAngles );
          vectoangles( pm->ps->grapplePoint, surfAngles );

          pm->ps->delta_angles[1] -= ANGLE2SHORT( ( ( surfAngles[1] - toAngles[1] ) * 2 ) - 180 );
        }
      }

      pml.groundTrace = trace;

      //so everything knows where we're wallclimbing (ie client side)
      pm->ps->legsAnim |= ANIM_WALLCLIMBING;

      //if we're not stuck to the ceiling then set grapplePoint to be a surface normal
      if( !VectorCompare( trace.plane.normal, ceilingNormal ) )
      {
        //so we know what surface we're stuck to
        VectorCopy( trace.plane.normal, pm->ps->grapplePoint );
        pm->ps->stats[ STAT_STATE ] &= ~SS_WALLCLIMBINGCEILING;
      }

      //IMPORTANT: break out of the for loop if we've hit something
      break;
    }
    else if ( trace.allsolid )
    {
      // do something corrective if the trace starts in a solid...
      //TA: fuck knows what this does with all my new stuff :(
      if ( !PM_CorrectAllSolid(&trace) )
        return;
    }
  }

  if ( trace.fraction >= 1.0 )
  {
    // if the trace didn't hit anything, we are in free fall
    PM_GroundTraceMissed();
    pml.groundPlane = qfalse;
    pml.walking = qfalse;
    pm->ps->legsAnim &= ~ANIM_WALLCLIMBING;

    pm->ps->stats[ STAT_STATE ] &= ~SS_WALLCLIMBINGCEILING;

    //we get very bizarre effects if we don't do this :0
    VectorCopy( refNormal, pm->ps->grapplePoint );
    return;
  }

  pml.groundPlane = qtrue;
  pml.walking = qtrue;

  // hitting solid ground will end a waterjump
  if (pm->ps->pm_flags & PMF_TIME_WATERJUMP)
  {
    pm->ps->pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND);
    pm->ps->pm_time = 0;
  }

  pm->ps->groundEntityNum = trace.entityNum;

  // don't reset the z velocity for slopes
//  pm->ps->velocity[2] = 0;

  PM_AddTouchEnt( trace.entityNum );
}


/*
=============
PM_GroundTrace
=============
*/
static void PM_GroundTrace( void ) {
  vec3_t      point, forward, srotAxis;
  vec3_t      refNormal = { 0.0f, 0.0f, 1.0f };
  vec3_t      ceilingNormal = { 0.0f, 0.0f, -1.0f };
  trace_t     trace;
  float       srotAngle;

  if( BG_ClassHasAbility( pm->ps->stats[ STAT_PCLASS ], SCA_WALLCLIMBER ) )
  {
    //toggle wall climbing if holding crouch
    if( pm->cmd.upmove < 0 && !( pm->ps->pm_flags & PMF_CROUCH_HELD ) )
    {
      if( !( pm->ps->stats[ STAT_STATE ] & SS_WALLCLIMBING ) )
        pm->ps->stats[ STAT_STATE ] |= SS_WALLCLIMBING;
      else if( pm->ps->stats[ STAT_STATE ] & SS_WALLCLIMBING )
        pm->ps->stats[ STAT_STATE ] &= ~SS_WALLCLIMBING;

      pm->ps->pm_flags |= PMF_CROUCH_HELD;
    }
    else if( pm->cmd.upmove >= 0 )
      pm->ps->pm_flags &= ~PMF_CROUCH_HELD;

    if( pm->ps->pm_type == PM_DEAD )
      pm->ps->stats[ STAT_STATE ] &= ~SS_WALLCLIMBING;

    if( pm->ps->stats[ STAT_STATE ] & SS_WALLCLIMBING )
    {
      PM_GroundClimbTrace( );
      return;
    }
  }

  pm->ps->stats[ STAT_STATE ] &= ~SS_WALLCLIMBING;
  pm->ps->stats[ STAT_STATE ] &= ~SS_WALLCLIMBINGCEILING;
  pm->ps->legsAnim &= ~ANIM_WALLCLIMBING;

  //make sure that the surfNormal is reset to the ground
  if( BG_ClassHasAbility( pm->ps->stats[ STAT_PCLASS ], SCA_WALLCLIMBER ) )
    VectorCopy( refNormal, pm->ps->grapplePoint );

  point[0] = pm->ps->origin[0];
  point[1] = pm->ps->origin[1];
  point[2] = pm->ps->origin[2] - 0.25;

  //FIXME: hack until i find out where CONTENTS_BODY is getting unset for uhm.. bodies.
  if( pm->ps->pm_type == PM_DEAD )
    pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, MASK_PLAYERSOLID );
  else
    pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
  pml.groundTrace = trace;

  // do something corrective if the trace starts in a solid...
  if( trace.allsolid )
    if( !PM_CorrectAllSolid(&trace) )
      return;
  
  // if the trace didn't hit anything, we are in free fall
  if( trace.fraction == 1.0 )
  {
    PM_GroundTraceMissed();
    pml.groundPlane = qfalse;
    pml.walking = qfalse;

    return;
  }

  // check if getting thrown off the ground
  if ( pm->ps->velocity[2] > 0 && DotProduct( pm->ps->velocity, trace.plane.normal ) > 10 ) {
    if ( pm->debugLevel ) {
      Com_Printf("%i:kickoff\n", c_pmove);
    }
    // go into jump animation
    if ( pm->cmd.forwardmove >= 0 ) {
      PM_ForceLegsAnim( LEGS_JUMP );
      pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
    } else {
      PM_ForceLegsAnim( LEGS_JUMPB );
      pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
    }

    pm->ps->groundEntityNum = ENTITYNUM_NONE;
    pml.groundPlane = qfalse;
    pml.walking = qfalse;
    return;
  }

  // slopes that are too steep will not be considered onground
  if ( trace.plane.normal[2] < MIN_WALK_NORMAL ) {
    if ( pm->debugLevel ) {
      Com_Printf("%i:steep\n", c_pmove);
    }
    // FIXME: if they can't slide down the slope, let them
    // walk (sharp crevices)
    pm->ps->groundEntityNum = ENTITYNUM_NONE;
    pml.groundPlane = qtrue;
    pml.walking = qfalse;
    return;
  }

  pml.groundPlane = qtrue;
  pml.walking = qtrue;

  // hitting solid ground will end a waterjump
  if (pm->ps->pm_flags & PMF_TIME_WATERJUMP)
  {
    pm->ps->pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND);
    pm->ps->pm_time = 0;
  }

  if ( pm->ps->groundEntityNum == ENTITYNUM_NONE ) {
    // just hit the ground
    if ( pm->debugLevel ) {
      Com_Printf("%i:Land\n", c_pmove);
    }

    if( BG_ClassHasAbility( pm->ps->stats[ STAT_PCLASS ], SCA_TAKESFALLDAMAGE ) )
      PM_CrashLand();

    // don't do landing time if we were just going down a slope
    if ( pml.previous_velocity[2] < -200 ) {
      // don't allow another jump for a little while
      pm->ps->pm_flags |= PMF_TIME_LAND;
      pm->ps->pm_time = 250;
    }
  }

  pm->ps->groundEntityNum = trace.entityNum;

  // don't reset the z velocity for slopes
//  pm->ps->velocity[2] = 0;

  PM_AddTouchEnt( trace.entityNum );
}


/*
=============
PM_SetWaterLevel  FIXME: avoid this twice?  certainly if not moving
=============
*/
static void PM_SetWaterLevel( void ) {
  vec3_t    point;
  int     cont;
  int     sample1;
  int     sample2;

  //
  // get waterlevel, accounting for ducking
  //
  pm->waterlevel = 0;
  pm->watertype = 0;

  point[0] = pm->ps->origin[0];
  point[1] = pm->ps->origin[1];
  point[2] = pm->ps->origin[2] + MINS_Z + 1;
  cont = pm->pointcontents( point, pm->ps->clientNum );

  if ( cont & MASK_WATER ) {
    sample2 = pm->ps->viewheight - MINS_Z;
    sample1 = sample2 / 2;

    pm->watertype = cont;
    pm->waterlevel = 1;
    point[2] = pm->ps->origin[2] + MINS_Z + sample1;
    cont = pm->pointcontents (point, pm->ps->clientNum );
    if ( cont & MASK_WATER ) {
      pm->waterlevel = 2;
      point[2] = pm->ps->origin[2] + MINS_Z + sample2;
      cont = pm->pointcontents (point, pm->ps->clientNum );
      if ( cont & MASK_WATER ){
        pm->waterlevel = 3;
      }
    }
  }

}



/*
==============
PM_CheckDuck

Sets mins, maxs, and pm->ps->viewheight
==============
*/
static void PM_CheckDuck (void)
{
  trace_t trace;
  vec3_t PCmins, PCmaxs, PCcmaxs;
  int PCvh, PCcvh;

  BG_FindBBoxForClass( pm->ps->stats[ STAT_PCLASS ], PCmins, PCmaxs, PCcmaxs, NULL, NULL );
  BG_FindViewheightForClass( pm->ps->stats[ STAT_PCLASS ], &PCvh, &PCcvh );

  //TA: iD bug? you can still crouch when you're a spectator
  if( pm->ps->persistant[PERS_TEAM] == TEAM_SPECTATOR )
    PCcvh = PCvh;

  pm->mins[0] = PCmins[0];
  pm->mins[1] = PCmins[1];

  pm->maxs[0] = PCmaxs[0];
  pm->maxs[1] = PCmaxs[1];

  pm->mins[2] = PCmins[2];

  if (pm->ps->pm_type == PM_DEAD)
  {
    pm->maxs[2] = -8;
    pm->ps->viewheight = DEAD_VIEWHEIGHT;
    return;
  }

  //TA: If the standing and crouching viewheights are the same the class can't crouch
  if( ( pm->cmd.upmove < 0 ) && ( PCvh != PCcvh ) &&
      !( BG_gotItem( UP_JETPACK, pm->ps->stats ) && BG_activated( UP_JETPACK, pm->ps->stats ) ) )
  { // duck
    pm->ps->pm_flags |= PMF_DUCKED;
  }
  else
  { // stand up if possible
    if (pm->ps->pm_flags & PMF_DUCKED)
    {
      // try to stand up
      pm->maxs[2] = PCmaxs[2];
      pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask );
      if (!trace.allsolid)
        pm->ps->pm_flags &= ~PMF_DUCKED;
    }
  }

  if (pm->ps->pm_flags & PMF_DUCKED)
  {
    pm->maxs[2] = PCcmaxs[2];
    pm->ps->viewheight = PCcvh;
  }
  else
  {
    pm->maxs[2] = PCmaxs[2];
    pm->ps->viewheight = PCvh;
  }
}



//===================================================================


/*
===============
PM_Footsteps
===============
*/
static void PM_Footsteps( void ) {
  float   bobmove;
  int     old;
  qboolean  footstep;

  //
  // calculate speed and cycle to be used for
  // all cyclic walking effects
  //
  if( BG_ClassHasAbility( pm->ps->stats[ STAT_PCLASS ], SCA_WALLCLIMBER ) && ( pml.groundPlane ) )
  {
    //TA: FIXME: yes yes i know this is wrong
    pm->xyspeed = sqrt( pm->ps->velocity[0] * pm->ps->velocity[0]
                      + pm->ps->velocity[1] * pm->ps->velocity[1]
                      + pm->ps->velocity[2] * pm->ps->velocity[2] );
  }
  else
    pm->xyspeed = sqrt( pm->ps->velocity[0] * pm->ps->velocity[0]
      + pm->ps->velocity[1] * pm->ps->velocity[1] );

  if ( pm->ps->groundEntityNum == ENTITYNUM_NONE ) {
    // airborne leaves position in cycle intact, but doesn't advance
    if ( pm->waterlevel > 1 ) {
      PM_ContinueLegsAnim( LEGS_SWIM );
    }
    return;
  }

  // if not trying to move
  if ( !pm->cmd.forwardmove && !pm->cmd.rightmove ) {
    if (  pm->xyspeed < 5 ) {
      pm->ps->bobCycle = 0; // start at beginning of cycle again
      if ( pm->ps->pm_flags & PMF_DUCKED ) {
        PM_ContinueLegsAnim( LEGS_IDLECR );
      } else {
        PM_ContinueLegsAnim( LEGS_IDLE );
      }
    }
    return;
  }


  footstep = qfalse;

  if ( pm->ps->pm_flags & PMF_DUCKED ) {
    bobmove = 0.5;  // ducked characters bob much faster
    if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
      PM_ContinueLegsAnim( LEGS_BACKCR );
    }
    else {
      PM_ContinueLegsAnim( LEGS_WALKCR );
    }
    // ducked characters never play footsteps
  /*
  } else  if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
    if ( !( pm->cmd.buttons & BUTTON_WALKING ) ) {
      bobmove = 0.4;  // faster speeds bob faster
      footstep = qtrue;
    } else {
      bobmove = 0.3;
    }
    PM_ContinueLegsAnim( LEGS_BACK );
  */
  } else {
    //TA: switch walking/running anims based on speed
    //if ( !( pm->cmd.buttons & BUTTON_WALKING ) ) {
    if ( pm->xyspeed > 160 ) {
      bobmove = 0.4f; // faster speeds bob faster
      if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
        PM_ContinueLegsAnim( LEGS_BACK );
      }
      else {
        PM_ContinueLegsAnim( LEGS_RUN );
      }
      footstep = qtrue;
    } else {
      bobmove = 0.3f; // walking bobs slow
      if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
        PM_ContinueLegsAnim( LEGS_BACKWALK );
      }
      else {
        PM_ContinueLegsAnim( LEGS_WALK );
      }
    }
  }

  // check for footstep / splash sounds
  old = pm->ps->bobCycle;
  pm->ps->bobCycle = (int)( old + bobmove * pml.msec ) & 255;

  // if we just crossed a cycle boundary, play an apropriate footstep event
  if ( ( ( old + 64 ) ^ ( pm->ps->bobCycle + 64 ) ) & 128 ) {
    if ( pm->waterlevel == 0 ) {
      // on ground will only play sounds if running
      if ( footstep && !pm->noFootsteps ) {
        PM_AddEvent( PM_FootstepForSurface() );
      }
    } else if ( pm->waterlevel == 1 ) {
      // splashing
      PM_AddEvent( EV_FOOTSPLASH );
    } else if ( pm->waterlevel == 2 ) {
      // wading / swimming at surface
      PM_AddEvent( EV_SWIM );
    } else if ( pm->waterlevel == 3 ) {
      // no sound when completely underwater

    }
  }
}

/*
==============
PM_WaterEvents

Generate sound events for entering and leaving water
==============
*/
static void PM_WaterEvents( void ) {    // FIXME?
  //
  // if just entered a water volume, play a sound
  //
  if (!pml.previous_waterlevel && pm->waterlevel) {
    PM_AddEvent( EV_WATER_TOUCH );
  }

  //
  // if just completely exited a water volume, play a sound
  //
  if (pml.previous_waterlevel && !pm->waterlevel) {
    PM_AddEvent( EV_WATER_LEAVE );
  }

  //
  // check for head just going under water
  //
  if (pml.previous_waterlevel != 3 && pm->waterlevel == 3) {
    PM_AddEvent( EV_WATER_UNDER );
  }

  //
  // check for head just coming out of water
  //
  if (pml.previous_waterlevel == 3 && pm->waterlevel != 3) {
    PM_AddEvent( EV_WATER_CLEAR );
  }
}


/*
===============
PM_BeginWeaponChange
===============
*/
static void PM_BeginWeaponChange( int weapon ) {
  if ( weapon < WP_NONE || weapon >= WP_NUM_WEAPONS ) {
    return;
  }

  if ( !BG_gotWeapon( weapon, pm->ps->stats ) && weapon != WP_NONE ) {
    return;
  }

  if ( pm->ps->weaponstate == WEAPON_DROPPING ) {
    return;
  }

  PM_AddEvent( EV_CHANGE_WEAPON );
  pm->ps->weaponstate = WEAPON_DROPPING;
  pm->ps->weaponTime += 200;
  PM_StartTorsoAnim( TORSO_DROP );
}


/*
===============
PM_FinishWeaponChange
===============
*/
static void PM_FinishWeaponChange( void ) {
  int   weapon;

  weapon = pm->cmd.weapon;
  if ( weapon < WP_NONE || weapon >= WP_NUM_WEAPONS ) {
    weapon = WP_NONE;
  }

  if ( !BG_gotWeapon( weapon, pm->ps->stats ) ) {
    weapon = WP_NONE;
  }

  pm->ps->weapon = weapon;
  pm->ps->weaponstate = WEAPON_RAISING;
  pm->ps->weaponTime += 250;
  PM_StartTorsoAnim( TORSO_RAISE );
}


/*
==============
PM_TorsoAnimation

==============
*/
static void PM_TorsoAnimation( void )
{
  if ( pm->ps->weaponstate == WEAPON_READY )
    PM_ContinueTorsoAnim( TORSO_STAND );
}

//synced alt weapons look like the way to go - simpler anyway...
#if 0
static void PM_packWeaponTime( int *psWeaponTime, int weaponTime1, int weaponTime2 )
{
  if( weaponTime1 < 0 && weaponTime2 >= 0 )
    *psWeaponTime = ( *psWeaponTime & 0x00FF ) | ( ( ( weaponTime2 / 10 ) << 8 ) & 0xFF00 );
  if( weaponTime2 < 0 && weaponTime1 >= 0 )
    *psWeaponTime = ( ( weaponTime1 / 10 ) & 0x00FF ) | ( *psWeaponTime & 0xFF00 );
  else if( weaponTime1 >= 0 && weaponTime2 >= 0 )
    *psWeaponTime = ( ( weaponTime1 / 10 ) & 0x00FF ) | ( ( ( weaponTime2 / 10 ) << 8 ) & 0xFF00 );
}

static void PM_unpackWeaponTime( int psWeaponTime, int *weaponTime1, int *weaponTime2 )
{
  if( weaponTime1 != NULL )
    *weaponTime1 = ( psWeaponTime & 0x00FF ) * 10;

  if( weaponTime2 != NULL )
    *weaponTime2 = ( ( psWeaponTime & 0x00FF ) >> 8 ) * 10;
}
#endif

/*
==============
PM_Weapon

Generates weapon events and modifes the weapon counter
==============
*/
static void PM_Weapon( void )
{
  int addTime;
  int ammo, clips, maxclips;
  qboolean attack1 = qfalse;
  qboolean attack2 = qfalse;
  qboolean attack3 = qfalse;
  
  // don't allow attack until all buttons are up
  if ( pm->ps->pm_flags & PMF_RESPAWNED )
    return;

  // ignore if spectator
  if ( pm->ps->persistant[PERS_TEAM] == TEAM_SPECTATOR )
    return;
    
  if( pm->ps->stats[ STAT_STATE ] & SS_INFESTING )
    return;

  if( pm->ps->stats[ STAT_STATE ] & SS_HOVELING )
    return;

  // check for dead player
  if ( pm->ps->stats[STAT_HEALTH] <= 0 )
  {
    pm->ps->weapon = WP_NONE;
    return;
  }

  // make weapon function
  if ( pm->ps->weaponTime > 0 ) {
    pm->ps->weaponTime -= pml.msec;
  }

  //TA: if we haven't got the weapon
  if( !BG_gotWeapon( pm->ps->weapon, pm->ps->stats ) && pm->ps->weapon != WP_NONE )
    PM_BeginWeaponChange( WP_NONE );
      
  // check for weapon change
  // can't change if weapon is firing, but can change
  // again if lowering or raising
  if( pm->ps->weaponTime <= 0 || pm->ps->weaponstate != WEAPON_FIRING )
  {
    //TA: must press use to switch weapons
    if( pm->cmd.buttons & BUTTON_USE_HOLDABLE )
    {
      if( !( pm->ps->pm_flags & PMF_USE_ITEM_HELD ) )
      {
        if( pm->cmd.weapon <= 32 )
        {
          //if trying to select a weapon, select it
          if( pm->ps->weapon != pm->cmd.weapon )
            PM_BeginWeaponChange( pm->cmd.weapon );
        }
        else if( pm->cmd.weapon > 32 )
        {
          //if trying to toggle an upgrade, toggle it
          if( BG_gotItem( pm->cmd.weapon - 32, pm->ps->stats ) ) //sanity check
          {
            if( BG_activated( pm->cmd.weapon - 32, pm->ps->stats ) )
              BG_deactivateItem( pm->cmd.weapon - 32, pm->ps->stats );
            else
              BG_activateItem( pm->cmd.weapon - 32, pm->ps->stats );
          }
        }
        pm->ps->pm_flags |= PMF_USE_ITEM_HELD;                          
      }
    }
    else
      pm->ps->pm_flags &= ~PMF_USE_ITEM_HELD;
  }
  
  if ( pm->ps->weaponTime > 0 )
    return;

  // change weapon if time
  if ( pm->ps->weaponstate == WEAPON_DROPPING )
  {
    PM_FinishWeaponChange();
    return;
  }

  if ( pm->ps->weaponstate == WEAPON_RAISING )
  {
    pm->ps->weaponstate = WEAPON_READY;
    PM_StartTorsoAnim( TORSO_STAND );
    return;
  }

  // start the animation even if out of ammo

  BG_unpackAmmoArray( pm->ps->weapon, pm->ps->ammo, pm->ps->powerups, &ammo, &clips, &maxclips );

  // check for out of ammo
  if ( !ammo && !clips && !BG_FindInfinteAmmoForWeapon( pm->ps->weapon ) )
  {
    PM_AddEvent( EV_NOAMMO );
    pm->ps->weaponTime += 200;
    return;
  }

  //done reloading so give em some ammo
  if( pm->ps->weaponstate == WEAPON_RELOADING )
  {
    if( maxclips > 0 )
    {
      clips--;
      BG_FindAmmoForWeapon( pm->ps->weapon, &ammo, NULL, NULL );
    }

    if( BG_FindUsesEnergyForWeapon( pm->ps->weapon ) &&
        BG_gotItem( UP_BATTPACK, pm->ps->stats ) )
      ammo *= 2;
    
    BG_packAmmoArray( pm->ps->weapon, pm->ps->ammo, pm->ps->powerups, ammo, clips, maxclips );
  }
  
  // check for end of clip 
  if ( !ammo && clips )
  {
    pm->ps->weaponstate = WEAPON_RELOADING;
    
    addTime = BG_FindReloadTimeForWeapon( pm->ps->weapon );

    pm->ps->weaponTime += addTime;
    return;
  }
    
  //check if non-auto primary/secondary attacks are permited
  switch( pm->ps->weapon )
  {
    case WP_VENOM:
      //venom is only autohit
      attack1 = attack2 = attack3 = qfalse;

      if( !pm->autoWeaponHit[ pm->ps->weapon ] )
      {
        pm->ps->weaponTime = 0;
        pm->ps->weaponstate = WEAPON_READY;
        return;
      }
      break;
      
    case WP_POUNCE:
    case WP_POUNCE_UPG:
      //pouncing has primary secondary AND autohit procedures
      attack1 = pm->cmd.buttons & BUTTON_ATTACK;
      attack2 = pm->cmd.buttons & BUTTON_ATTACK2;
      attack3 = pm->cmd.buttons & BUTTON_USE_HOLDABLE;
      
      if( !pm->autoWeaponHit[ pm->ps->weapon ] && !attack1 && !attack2 && !attack3 )
      {
        pm->ps->weaponTime = 0;
        pm->ps->weaponstate = WEAPON_READY;
        return;
      }
      break;

    case WP_LUCIFER_CANON:
      attack1 = pm->cmd.buttons & BUTTON_ATTACK;
      attack2 = pm->cmd.buttons & BUTTON_ATTACK2;
      attack3 = pm->cmd.buttons & BUTTON_USE_HOLDABLE;
      
      if( ( attack1 || pm->ps->stats[ STAT_MISC ] == 0 ) && !attack2 && !attack3 )
      {
        pm->ps->weaponTime = 0;
        pm->ps->weaponstate = WEAPON_READY;
        return;
      }

      //erp this looks confusing
      if( pm->ps->stats[ STAT_MISC ] > 0 )
        attack1 = !attack1;
      break;
      
    default:
      //by default primary and secondary attacks are allowed
      attack1 = pm->cmd.buttons & BUTTON_ATTACK;
      attack2 = pm->cmd.buttons & BUTTON_ATTACK2;
      attack3 = pm->cmd.buttons & BUTTON_USE_HOLDABLE;
      
      if( !attack1 && !attack2 && !attack3 )
      {
        pm->ps->weaponTime = 0;
        pm->ps->weaponstate = WEAPON_READY;
        return;
      }
      break;
  }

  //TA: fire events for non auto weapons
  if( attack3 )
  {
    if( BG_WeaponHasThirdMode( pm->ps->weapon ) )
      PM_AddEvent( EV_FIRE_WEAPON3 );
    else
    {
      pm->ps->weaponTime = 0;
      pm->ps->weaponstate = WEAPON_READY;
      return;
    }
  }
  else if( attack2 )
  {
    if( BG_WeaponHasAltMode( pm->ps->weapon ) )
    {
      if( BG_WeaponModesAreSynced( pm->ps->weapon ) && attack1 )
        PM_AddEvent( EV_FIRE_WEAPONBOTH );
      else
        PM_AddEvent( EV_FIRE_WEAPON2 );
    }
    else
    {
      pm->ps->weaponTime = 0;
      pm->ps->weaponstate = WEAPON_READY;
      return;
    }
  }
  else if( attack1 )
    PM_AddEvent( EV_FIRE_WEAPON );

  //TA: fire events for autohit weapons
  if( pm->autoWeaponHit[ pm->ps->weapon ] )
  {
    switch( pm->ps->weapon )
    {
      case WP_VENOM:
        PM_AddEvent( EV_FIRE_WEAPON );
        break;
    
      case WP_POUNCE:
      case WP_POUNCE_UPG:
        PM_AddEvent( EV_FIRE_WEAPON2 );
        break;

      default:
        break;
    }
  }
    
  PM_StartTorsoAnim( TORSO_ATTACK );

  pm->ps->weaponstate = WEAPON_FIRING;
  
  // take an ammo away if not infinite
  if( !BG_FindInfinteAmmoForWeapon( pm->ps->weapon ) )
  {
    //special case for lCanon
    if( pm->ps->weapon == WP_LUCIFER_CANON && attack1 )
    {
      ammo -= (int)( ceil( ( (float)pm->ps->stats[ STAT_MISC ] / (float)LC_TOTAL_CHARGE ) * 10.0f ) );

      //stay on the safe side
      if( ammo < 0 )
        ammo = 0;
    }
    else
      ammo--;
    
    BG_packAmmoArray( pm->ps->weapon, pm->ps->ammo, pm->ps->powerups, ammo, clips, maxclips );
  }
  
  addTime = BG_FindRepeatRateForWeapon( pm->ps->weapon );

  //FIXME: predicted angles miss a problem??
  if( pm->ps->weapon == WP_CHAINGUN )
  {
    if( pm->ps->pm_flags & PMF_DUCKED )
    {
      pm->ps->delta_angles[ PITCH ] -= ANGLE2SHORT( ( ( random() * 0.5 ) - 0.125 ) * ( 30 / (float)addTime ) );
      pm->ps->delta_angles[ YAW ] -= ANGLE2SHORT( ( ( random() * 0.5 ) - 0.25 ) * ( 30.0 / (float)addTime ) );
    }
    else
    {
      pm->ps->delta_angles[ PITCH ] -= ANGLE2SHORT( ( ( random() * 8 ) - 2 ) * ( 30.0 / (float)addTime ) );
      pm->ps->delta_angles[ YAW ] -= ANGLE2SHORT( ( ( random() * 8 ) - 4 ) * ( 30.0 / (float)addTime ) );
    }
  }
  
  pm->ps->weaponTime += addTime;
}

/*
================
PM_Animate
================
*/
static void PM_Animate( void ) {
  if ( pm->cmd.buttons & BUTTON_GESTURE ) {
    if ( pm->ps->torsoTimer == 0 ) {
      PM_StartTorsoAnim( TORSO_GESTURE );
      pm->ps->torsoTimer = TIMER_GESTURE;
      PM_AddEvent( EV_TAUNT );
    }
  }
}


/*
================
PM_DropTimers
================
*/
static void PM_DropTimers( void ) {
  // drop misc timing counter
  if ( pm->ps->pm_time ) {
    if ( pml.msec >= pm->ps->pm_time ) {
      pm->ps->pm_flags &= ~PMF_ALL_TIMES;
      pm->ps->pm_time = 0;
    } else {
      pm->ps->pm_time -= pml.msec;
    }
  }

  // drop animation counter
  if ( pm->ps->legsTimer > 0 ) {
    pm->ps->legsTimer -= pml.msec;
    if ( pm->ps->legsTimer < 0 ) {
      pm->ps->legsTimer = 0;
    }
  }

  if ( pm->ps->torsoTimer > 0 ) {
    pm->ps->torsoTimer -= pml.msec;
    if ( pm->ps->torsoTimer < 0 ) {
      pm->ps->torsoTimer = 0;
    }
  }
}


/*
================
PM_UpdateViewAngles

This can be used as another entry point when only the viewangles
are being updated instead of a full move
================
*/
void PM_UpdateViewAngles( playerState_t *ps, const usercmd_t *cmd )
{
  short   temp[3];
  int     i;
  vec3_t  surfNormal, xNormal;
  vec3_t  axis[3], rotaxis[3];
  vec3_t  refNormal = { 0.0f, 0.0f, 1.0f };
  vec3_t  ceilingNormal = { 0.0f, 0.0f, -1.0f };
  float   rotAngle; 
  vec3_t  tempang;

  if( ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPINTERMISSION )
    return;   // no view changes at all

  if( ps->pm_type != PM_SPECTATOR && ps->stats[STAT_HEALTH] <= 0 )
    return;   // no view changes at all

  // circularly clamp the angles with deltas
  for( i = 0; i < 3; i++ )
  {
    temp[ i ] = cmd->angles[ i ] + ps->delta_angles[ i ];

    if( i == PITCH )
    {
      // don't let the player look up or down more than 90 degrees
      if( temp[ i ] > 16000 )
      {
        ps->delta_angles[ i ] = 16000 - cmd->angles[ i ];
        temp[ i ] = 16000;
      }
      else if( temp[ i ] < -16000 )
      {
        ps->delta_angles[ i ] = -16000 - cmd->angles[ i ];
        temp[ i ] = -16000;
      }
    }
    tempang[ i ] = SHORT2ANGLE( temp[ i ] );
  }

  //convert viewangles -> axis
  AnglesToAxis( tempang, axis );

  //the grapplePoint being a surfNormal rotation Normal hack... see above :)
  if( ps->stats[ STAT_STATE ] & SS_WALLCLIMBINGCEILING )
  {
    VectorCopy( ceilingNormal, surfNormal );
    VectorCopy( ps->grapplePoint, xNormal );
  }
  else
  {
    //cross the reference normal and the surface normal to get the rotation axis
    VectorCopy( ps->grapplePoint, surfNormal );
    CrossProduct( surfNormal, refNormal, xNormal );
    VectorNormalize( xNormal );
  }

  //if we're a wall climber.. and we're climbing rotate the axis
  if( BG_ClassHasAbility( pm->ps->stats[ STAT_PCLASS ], SCA_WALLCLIMBER ) &&
      ( pm->ps->stats[ STAT_STATE ] & SS_WALLCLIMBING ) &&
      ( VectorLength( xNormal ) != 0 ) )
  {
    //if the normal pointing straight down then the rotAngle will always be 180deg
    if( surfNormal[2] == -1 )
      rotAngle = 180;
    else
      rotAngle = RAD2DEG( acos( DotProduct( surfNormal, refNormal ) ) );

    //hmmm could get away with only one rotation and some clever stuff later... but i'm lazy
    RotatePointAroundVector( rotaxis[0], xNormal, axis[0], -rotAngle );
    RotatePointAroundVector( rotaxis[1], xNormal, axis[1], -rotAngle );
    RotatePointAroundVector( rotaxis[2], xNormal, axis[2], -rotAngle );
  }
  else
  {
    //we can't wall climb/aren't wall climbing
    rotAngle = 0;
    AxisCopy( axis, rotaxis );
  }

  //convert the new axis back to angles
  AxisToAngles( rotaxis, tempang );

  //force angles to -180 <= x <= 180
  //AnglesSubtract( tempang2, 0, tempang2 );
  for( i = 0; i < 3; i++ )
  {
    while( tempang[ i ] > 180 )
      tempang[ i ] -= 360;
      
    while( tempang[ i ] < 180 )
      tempang[ i ] += 360;
  }

  //actually set the viewangles
  for( i = 0; i < 3; i++ )
    ps->viewangles[ i ] = tempang[ i ];

  //pull the view into the lock point
  if( ps->pm_type == PM_GRABBED )
  {
    for( i = 0; i < 3; i++ )
    {
      float diff = AngleSubtract( ps->viewangles[ i ], ps->grapplePoint[ i ] );

      while( diff > 180.0f )
        diff -= 360.0f;
      while( diff < -180.0f )
        diff += 360.0f;

      if( diff < 0 )
        ps->delta_angles[ i ] += ANGLE2SHORT( fabs( diff ) * 0.05f );
      else if( diff > 0 )
        ps->delta_angles[ i ] -= ANGLE2SHORT( fabs( diff ) * 0.05f );
    }
  }
}


/*
================
PmoveSingle

================
*/
void trap_SnapVector( float *v );

void PmoveSingle (pmove_t *pmove)
{
  int ammo, clips, maxclips;
  pm = pmove;

  BG_unpackAmmoArray( pm->ps->weapon, pm->ps->ammo, pm->ps->powerups, &ammo, &clips, &maxclips );

  // this counter lets us debug movement problems with a journal
  // by setting a conditional breakpoint fot the previous frame
  c_pmove++;

  // clear results
  pm->numtouch = 0;
  pm->watertype = 0;
  pm->waterlevel = 0;

  /*if ( pm->ps->stats[STAT_HEALTH] <= 0 ) {
    pm->tracemask &= ~CONTENTS_BODY;  // corpses can fly through bodies
  }*/

  // make sure walking button is clear if they are running, to avoid
  // proxy no-footsteps cheats
  if ( abs( pm->cmd.forwardmove ) > 64 || abs( pm->cmd.rightmove ) > 64 ) {
    pm->cmd.buttons &= ~BUTTON_WALKING;
  }

  // set the talk balloon flag
  if ( pm->cmd.buttons & BUTTON_TALK ) {
    pm->ps->eFlags |= EF_TALK;
  } else {
    pm->ps->eFlags &= ~EF_TALK;
  }

  // set the firing flag for continuous beam weapons
  if ( !(pm->ps->pm_flags & PMF_RESPAWNED) && pm->ps->pm_type != PM_INTERMISSION
    && ( pm->cmd.buttons & BUTTON_ATTACK )
    && ( ( ammo > 0 || clips > 0 ) || BG_FindInfinteAmmoForWeapon( pm->ps->weapon ) ) )
    pm->ps->eFlags |= EF_FIRING;
  else
    pm->ps->eFlags &= ~EF_FIRING;
 
  // set the firing flag for continuous beam weapons
  if ( !(pm->ps->pm_flags & PMF_RESPAWNED) && pm->ps->pm_type != PM_INTERMISSION
    && ( pm->cmd.buttons & BUTTON_ATTACK2 )
    && ( ( ammo > 0 || clips > 0 ) || BG_FindInfinteAmmoForWeapon( pm->ps->weapon ) ) )
    pm->ps->eFlags |= EF_FIRING2;
  else
    pm->ps->eFlags &= ~EF_FIRING2;
 

  // clear the respawned flag if attack and use are cleared
  if ( pm->ps->stats[STAT_HEALTH] > 0 &&
    !( pm->cmd.buttons & (BUTTON_ATTACK | BUTTON_USE_HOLDABLE) ) ) {
    pm->ps->pm_flags &= ~PMF_RESPAWNED;
  }

  // if talk button is down, dissallow all other input
  // this is to prevent any possible intercept proxy from
  // adding fake talk balloons
  if ( pmove->cmd.buttons & BUTTON_TALK ) {
    pmove->cmd.buttons = BUTTON_TALK;
    pmove->cmd.forwardmove = 0;
    pmove->cmd.rightmove = 0;
    pmove->cmd.upmove = 0;
  }

  // clear all pmove local vars
  memset (&pml, 0, sizeof(pml));

  // determine the time
  pml.msec = pmove->cmd.serverTime - pm->ps->commandTime;
  if ( pml.msec < 1 ) {
    pml.msec = 1;
  } else if ( pml.msec > 200 ) {
    pml.msec = 200;
  }
  pm->ps->commandTime = pmove->cmd.serverTime;

  // save old org in case we get stuck
  VectorCopy (pm->ps->origin, pml.previous_origin);

  // save old velocity for crashlanding
  VectorCopy (pm->ps->velocity, pml.previous_velocity);

  pml.frametime = pml.msec * 0.001;

  AngleVectors( pm->ps->viewangles, pml.forward, pml.right, pml.up );

  if ( pm->cmd.upmove < 10 ) {
    // not holding jump
    pm->ps->pm_flags &= ~PMF_JUMP_HELD;
  }

  // decide if backpedaling animations should be used
  if ( pm->cmd.forwardmove < 0 ) {
    pm->ps->pm_flags |= PMF_BACKWARDS_RUN;
  } else if ( pm->cmd.forwardmove > 0 || ( pm->cmd.forwardmove == 0 && pm->cmd.rightmove ) ) {
    pm->ps->pm_flags &= ~PMF_BACKWARDS_RUN;
  }

  if ( pm->ps->pm_type >= PM_DEAD )
  {
    pm->cmd.forwardmove = 0;
    pm->cmd.rightmove = 0;
    pm->cmd.upmove = 0;
  }

  if ( pm->ps->pm_type == PM_SPECTATOR ) {
    // update the viewangles
    PM_UpdateViewAngles( pm->ps, &pm->cmd );
    PM_CheckDuck ();
    PM_FlyMove ();
    PM_DropTimers ();
    return;
  }

  if ( pm->ps->pm_type == PM_NOCLIP ) {
    PM_NoclipMove ();
    PM_DropTimers ();
    return;
  }

  if (pm->ps->pm_type == PM_FREEZE) {
    return;   // no movement at all
  }

  if ( pm->ps->pm_type == PM_INTERMISSION || pm->ps->pm_type == PM_SPINTERMISSION ) {
    return;   // no movement at all
  }

  // set watertype, and waterlevel
  PM_SetWaterLevel();
  pml.previous_waterlevel = pmove->waterlevel;

  // set mins, maxs, and viewheight
  PM_CheckDuck ();

  // set groundentity
  PM_GroundTrace();
  // update the viewangles
  PM_UpdateViewAngles( pm->ps, &pm->cmd );

  if ( pm->ps->pm_type == PM_DEAD || pm->ps->pm_type == PM_GRABBED )
    PM_DeadMove( );

  PM_DropTimers( );

  if( BG_gotItem( UP_JETPACK, pm->ps->stats ) && BG_activated( UP_JETPACK, pm->ps->stats ) )
    PM_JetPackMove( );
  else if( pm->ps->pm_flags & PMF_GRAPPLE_PULL )
  {
    PM_GrappleMove( );
    // We can wiggle a bit
    PM_AirMove( );
  }
  else if( pm->ps->pm_flags & PMF_TIME_WATERJUMP )
    PM_WaterJumpMove( );
  else if ( pm->waterlevel > 1 )
    PM_WaterMove( );
  else if ( pml.walking )
  {
    if( BG_ClassHasAbility( pm->ps->stats[ STAT_PCLASS ], SCA_WALLCLIMBER ) &&
        ( pm->ps->stats[ STAT_STATE ] & SS_WALLCLIMBING ) )
      PM_ClimbMove( ); //TA: walking on any surface
    else
      PM_WalkMove( ); // walking on ground
  }
  else
    PM_AirMove( );

  PM_Animate( );

  // set groundentity, watertype, and waterlevel
  PM_GroundTrace( );
  //TA: must update after every GroundTrace() - yet more clock cycles down the drain :( (14 vec rotations/frame)
  // update the viewangles
  PM_UpdateViewAngles( pm->ps, &pm->cmd );

  PM_SetWaterLevel( );

  // weapons
  PM_Weapon( );

  // torso animation
  PM_TorsoAnimation( );

  // footstep events / legs animations
  PM_Footsteps( );

  // entering / leaving water splashes
  PM_WaterEvents( );

  // snap some parts of playerstate to save network bandwidth
  trap_SnapVector( pm->ps->velocity );
}


/*
================
Pmove

Can be called by either the server or the client
================
*/
void Pmove (pmove_t *pmove) {
  int     finalTime;

  finalTime = pmove->cmd.serverTime;

  if ( finalTime < pmove->ps->commandTime ) {
    return; // should not happen
  }

  if ( finalTime > pmove->ps->commandTime + 1000 ) {
    pmove->ps->commandTime = finalTime - 1000;
  }

  pmove->ps->pmove_framecount = (pmove->ps->pmove_framecount+1) & ((1<<PS_PMOVEFRAMECOUNTBITS)-1);

  // chop the move up if it is too long, to prevent framerate
  // dependent behavior
  while ( pmove->ps->commandTime != finalTime ) {
    int   msec;

    msec = finalTime - pmove->ps->commandTime;

    if ( pmove->pmove_fixed ) {
      if ( msec > pmove->pmove_msec ) {
        msec = pmove->pmove_msec;
      }
    }
    else {
      if ( msec > 66 ) {
        msec = 66;
      }
    }


    pmove->cmd.serverTime = pmove->ps->commandTime + msec;
    PmoveSingle( pmove );

    if ( pmove->ps->pm_flags & PMF_JUMP_HELD ) {
      pmove->cmd.upmove = 20;
    }
  }

}
