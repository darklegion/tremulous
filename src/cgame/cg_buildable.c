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

//if it ends up this is needed outwith this file i'll make it a bit more tidy
animation_t buildAnimations[ BA_NUM_BUILDABLES ][ MAX_BUILDABLE_ANIMATIONS ];

/*
======================
CG_ParseBuildableAnimationFile

Read a configuration file containing animation coutns and rates
models/buildables/hivemind/animation.cfg, etc
======================
*/
static qboolean CG_ParseBuildableAnimationFile( const char *filename, buildable_t buildable )
{
  char    *text_p, *prev;
  int     len;
  int     i;
  char    *token;
  float   fps;
  int     skip;
  char    text[ 20000 ];
  fileHandle_t  f;
  animation_t *animations;

  animations = buildAnimations[ buildable ];

  // load the file
  len = trap_FS_FOpenFile( filename, &f, FS_READ );
  if ( len <= 0 )
    return qfalse;
    
  if ( len >= sizeof( text ) - 1 )
  {
    CG_Printf( "File %s too long\n", filename );
    return qfalse;
  }
  
  trap_FS_Read( text, len, f );
  text[len] = 0;
  trap_FS_FCloseFile( f );

  // parse the text
  text_p = text;
  skip = 0; // quite the compiler warning

  // read information for each frame
  for ( i = BANIM_NONE + 1; i < MAX_BUILDABLE_ANIMATIONS; i++ )
  {

    token = COM_Parse( &text_p );
    if ( !*token )
      break;
      
    animations[ i ].firstFrame = atoi( token );

    token = COM_Parse( &text_p );
    if ( !*token )
      break;
      
    animations[ i ].numFrames = atoi( token );
    animations[ i ].reversed = qfalse;
    animations[ i ].flipflop = qfalse;
    
    // if numFrames is negative the animation is reversed
    if ( animations[ i ].numFrames < 0 )
    {
      animations[ i ].numFrames = -animations[ i ].numFrames;
      animations[ i ].reversed = qtrue;
    }

    token = COM_Parse( &text_p );
    if ( !*token )
      break;
      
    animations[i].loopFrames = atoi( token );

    token = COM_Parse( &text_p );
    if ( !*token )
      break;
      
    fps = atof( token );
    if ( fps == 0 )
      fps = 1;
    
    animations[ i ].frameLerp = 1000 / fps;
    animations[ i ].initialLerp = 1000 / fps;
  }

  if ( i != MAX_BUILDABLE_ANIMATIONS ) {
    CG_Printf( "Error parsing animation file: %s\n", filename );
    return qfalse;
  }
  
  return qtrue;
}

/*
===============
CG_InitBuildables

Initialises the animation db
===============
*/
void CG_InitBuildables( )
{
  char  filename[MAX_QPATH];
  char  *buildableName;
  char  *modelFile;
  int   i;
  int   j;

  memset( cg_buildables, 0, sizeof( cg_buildables ) );

  for( i = BA_NONE + 1; i < BA_NUM_BUILDABLES; i++ )
  {
    buildableName = BG_FindNameForBuildable( i );
    
    Com_sprintf( filename, sizeof( filename ), "models/buildables/%s/animation.cfg", buildableName );
    if ( !CG_ParseBuildableAnimationFile( filename, i ) )
      Com_Printf( "Failed to load animation file %s\n", filename );

    for( j = 0; j <= 3; j++ )
    {
      if( modelFile = BG_FindModelsForBuildable( i, j ) )
        cg_buildables[ i ].models[ j ] = trap_R_RegisterModel( modelFile );
    }
  }
}

/*
===============
CG_SetBuildableLerpFrameAnimation

may include ANIM_TOGGLEBIT
===============
*/
static void CG_SetBuildableLerpFrameAnimation( buildable_t buildable, lerpFrame_t *lf, int newAnimation )
{
  animation_t *anim;

  lf->animationNumber = newAnimation;
  newAnimation &= ~ANIM_TOGGLEBIT;

  if( newAnimation < 0 || newAnimation >= MAX_BUILDABLE_ANIMATIONS )
    CG_Error( "Bad animation number: %i", newAnimation );

  anim = &buildAnimations[ buildable ][ newAnimation ];

  //this item has just spawned so lf->frameTime will be zero
  if( !lf->animation )
    lf->frameTime = cg.time + 1000; //1 sec delay before starting the spawn anim

  lf->animation = anim;
  lf->animationTime = lf->frameTime + anim->initialLerp;

  if( cg_debugAnim.integer )
    CG_Printf( "Anim: %i\n", newAnimation );
}

/*
===============
CG_RunBuildableLerpFrame

Sets cg.snap, cg.oldFrame, and cg.backlerp
cg.time should be between oldFrameTime and frameTime after exit
===============
*/
static void CG_RunBuildableLerpFrame( centity_t *cent )
{
  int                   f, numFrames;
  animation_t           *anim;
  buildable_t           buildable = cent->currentState.modelindex;
  lerpFrame_t           *lf = &cent->lerpFrame;
  buildableAnimNumber_t newAnimation = cent->buildableAnim;

  // debugging tool to get no animations
  if( cg_animSpeed.integer == 0 )
  {
    lf->oldFrame = lf->frame = lf->backlerp = 0;
    return;
  }

  // see if the animation sequence is switching
  if( newAnimation != lf->animationNumber || !lf->animation )
    CG_SetBuildableLerpFrameAnimation( buildable, lf, newAnimation );

  // if we have passed the current frame, move it to
  // oldFrame and calculate a new frame
  if( cg.time >= lf->frameTime )
  {
    lf->oldFrame = lf->frame;
    lf->oldFrameTime = lf->frameTime;

    // get the next frame based on the animation
    anim = lf->animation;
    if( !anim->frameLerp )
      return;   // shouldn't happen
      
    if ( cg.time < lf->animationTime )
      lf->frameTime = lf->animationTime;    // initial lerp
    else
      lf->frameTime = lf->oldFrameTime + anim->frameLerp;
      
    f = ( lf->frameTime - lf->animationTime ) / anim->frameLerp;
    numFrames = anim->numFrames;
    if(anim->flipflop)
      numFrames *= 2;

    if( f >= numFrames )
    {
      f -= numFrames;
      if( anim->loopFrames )
      {
        f %= anim->loopFrames;
        f += anim->numFrames - anim->loopFrames;
      }
      else
      {
        f = numFrames - 1;
        // the animation is stuck at the end, so it
        // can immediately transition to another sequence
        lf->frameTime = cg.time;
        cent->buildableAnim = cent->currentState.torsoAnim;
      }
    }
    
    if( anim->reversed )
      lf->frame = anim->firstFrame + anim->numFrames - 1 - f;
    else if(anim->flipflop && f>=anim->numFrames)
      lf->frame = anim->firstFrame + anim->numFrames - 1 - (f%anim->numFrames);
    else
      lf->frame = anim->firstFrame + f;
      
    if ( cg.time > lf->frameTime )
    {
      lf->frameTime = cg.time;
      if( cg_debugAnim.integer )
        CG_Printf( "Clamp lf->frameTime\n");
    }
  }

  if( lf->frameTime > cg.time + 200 )
    lf->frameTime = cg.time;

  if( lf->oldFrameTime > cg.time )
    lf->oldFrameTime = cg.time;
    
  // calculate current lerp value
  if ( lf->frameTime == lf->oldFrameTime )
    lf->backlerp = 0;
  else
    lf->backlerp = 1.0 - (float)( cg.time - lf->oldFrameTime ) / ( lf->frameTime - lf->oldFrameTime );
}

/*
===============
CG_BuildableAnimation
===============
*/
static void CG_BuildableAnimation( centity_t *cent, int *old, int *now, float *backLerp )
{
  //if no animation is set default to idle anim
  if( cent->buildableAnim == BANIM_NONE )
    cent->buildableAnim = cent->currentState.torsoAnim;

  CG_RunBuildableLerpFrame( cent );

  *old      = cent->lerpFrame.oldFrame;
  *now      = cent->lerpFrame.frame;
  *backLerp = cent->lerpFrame.backlerp;
}

#define TRACE_DEPTH 128.0f

/*
==================
CG_GhostBuildable
==================
*/
void CG_GhostBuildable( buildable_t buildable )
{
  refEntity_t     ent;
  playerState_t   *ps;
  vec3_t          angles, forward, player_origin, entity_origin, target_origin, normal, cross;
  vec3_t          mins, maxs, start, end;
  float           distance;
  trace_t         tr;
  
  ps = &cg.predictedPlayerState;
  
  memset ( &ent, 0, sizeof( ent ) );

  if( cg.predictedPlayerState.stats[ STAT_STATE ] & SS_WALLCLIMBING )
  {
    if( cg.predictedPlayerState.stats[ STAT_STATE ] & SS_WALLCLIMBINGCEILING )
      VectorSet( normal, 0.0f, 0.0f, -1.0f );
    else
      VectorCopy( cg.predictedPlayerState.grapplePoint, normal );
  }
  else
    VectorSet( normal, 0.0f, 0.0f, 1.0f );
  
  VectorCopy( cg.predictedPlayerState.viewangles, angles );

  AngleVectors( angles, forward, NULL, NULL );
  CrossProduct( forward, normal, cross );
  VectorNormalize( cross );
  CrossProduct( normal, cross, forward );
  VectorNormalize( forward );

  VectorCopy( ps->origin, player_origin );

  distance = BG_FindBuildDistForClass( ps->stats[ STAT_PCLASS ] );
  VectorMA( player_origin, distance, forward, entity_origin );
  
  VectorCopy( entity_origin, target_origin );
  VectorMA( entity_origin, 32, normal, entity_origin );
  VectorMA( target_origin, -128, normal, target_origin );

  BG_FindBBoxForBuildable( buildable, mins, maxs );
  
  CG_Trace( &tr, entity_origin, mins, maxs, target_origin, ps->clientNum, MASK_PLAYERSOLID );
  VectorCopy( tr.endpos, entity_origin );
  VectorMA( entity_origin, 0.1f, normal, entity_origin );

  AngleVectors( angles, forward, NULL, NULL );
  VectorCopy( tr.plane.normal, ent.axis[ 2 ] );
  ProjectPointOnPlane( ent.axis[ 0 ], forward, ent.axis[ 2 ] );
  
  if( !VectorNormalize( ent.axis[ 0 ] ) )
  {
    AngleVectors( angles, NULL, NULL, forward );
    ProjectPointOnPlane( ent.axis[ 0 ], forward, ent.axis[ 2 ] );
    VectorNormalize( ent.axis[ 0 ] );
  }
  
  CrossProduct( ent.axis[ 0 ], ent.axis[ 2 ], ent.axis[ 1 ] );
  ent.axis[ 1 ][ 0 ] = -ent.axis[ 1 ][ 0 ];
  ent.axis[ 1 ][ 1 ] = -ent.axis[ 1 ][ 1 ];
  ent.axis[ 1 ][ 2 ] = -ent.axis[ 1 ][ 2 ];

  VectorMA( entity_origin, -TRACE_DEPTH, tr.plane.normal, end );
  VectorMA( entity_origin, 1.0f, tr.plane.normal, start );
  CG_CapTrace( &tr, start, mins, maxs, end, ps->clientNum, MASK_PLAYERSOLID );
  VectorMA( entity_origin, tr.fraction * -TRACE_DEPTH, tr.plane.normal, ent.origin );

  VectorCopy( ent.origin, ent.lightingOrigin );
  VectorCopy( ent.origin, ent.oldorigin ); // don't positionally lerp at all
    
  ent.hModel = cg_buildables[ buildable ].models[ 0 ];

  if( ps->stats[ STAT_BUILDABLE ] & SB_VALID_TOGGLEBIT )
    ent.customShader = cgs.media.greenBuildShader;
  else
    ent.customShader = cgs.media.redBuildShader;
    
  ent.nonNormalizedAxes = qfalse;

  // add to refresh list
  trap_R_AddRefEntityToScene( &ent );
}

/*
==================
CG_Buildable
==================
*/
void CG_Buildable( centity_t *cent )
{
  refEntity_t     ent;
  entityState_t   *es;
  vec3_t          angles;
  vec3_t          forward, surfNormal, xNormal, end, start, mins, maxs;
  vec3_t          refNormal = { 0.0f, 0.0f, 1.0f };
  float           rotAngle;
  trace_t         tr;

  es = &cent->currentState;
  
  //add creep
  if( es->modelindex2 == BIT_ALIENS )
    CG_Creep( cent );  

  // if set to invisible, skip
  if ( !es->modelindex || ( es->eFlags & EF_NODRAW ) )
    return;

  memset ( &ent, 0, sizeof( ent ) );

  VectorCopy( cent->lerpOrigin, ent.origin );
  VectorCopy( cent->lerpOrigin, ent.oldorigin );
  VectorCopy( cent->lerpOrigin, ent.lightingOrigin );

  VectorCopy( es->origin2, surfNormal );
  CrossProduct( surfNormal, refNormal, xNormal );
  VectorNormalize( xNormal );
  rotAngle = RAD2DEG( acos( DotProduct( surfNormal, refNormal ) ) );
  
  VectorCopy( es->angles, angles );
  BG_FindBBoxForBuildable( es->modelindex, mins, maxs );

  AngleVectors( angles, forward, NULL, NULL );
  VectorCopy( surfNormal, ent.axis[ 2 ] );
  ProjectPointOnPlane( ent.axis[ 0 ], forward, ent.axis[ 2 ] );
  
  if( !VectorNormalize( ent.axis[ 0 ] ) )
  {
    AngleVectors( angles, NULL, NULL, forward );
    ProjectPointOnPlane( ent.axis[ 0 ], forward, ent.axis[ 2 ] );
    VectorNormalize( ent.axis[ 0 ] );
  }
  
  CrossProduct( ent.axis[ 0 ], ent.axis[ 2 ], ent.axis[ 1 ] );
  ent.axis[ 1 ][ 0 ] = -ent.axis[ 1 ][ 0 ];
  ent.axis[ 1 ][ 1 ] = -ent.axis[ 1 ][ 1 ];
  ent.axis[ 1 ][ 2 ] = -ent.axis[ 1 ][ 2 ];

  VectorMA( ent.origin, -TRACE_DEPTH, surfNormal, end );
  VectorMA( ent.origin, 1.0f, surfNormal, start );
  CG_CapTrace( &tr, start, mins, maxs, end, es->number, MASK_PLAYERSOLID );
  VectorMA( ent.origin, tr.fraction * -TRACE_DEPTH, surfNormal, ent.origin );

  VectorCopy( ent.origin, ent.lightingOrigin );
  VectorCopy( ent.origin, ent.oldorigin ); // don't positionally lerp at all

  ent.hModel = cg_buildables[ es->modelindex ].models[ 0 ];

  ent.nonNormalizedAxes = qfalse;

  //run animations
  CG_BuildableAnimation( cent, &ent.oldframe, &ent.frame, &ent.backlerp );
  
  // add to refresh list
  trap_R_AddRefEntityToScene(&ent);

  //turret barrel bit
  if( cg_buildables[ es->modelindex ].models[ 1 ] )
  {
    refEntity_t turretBarrel;
    vec3_t      flatAxis[ 3 ];
    
    memset( &turretBarrel, 0, sizeof( turretBarrel ) );

    turretBarrel.hModel = cg_buildables[ es->modelindex ].models[ 1 ];

    CG_PositionRotatedEntityOnTag( &turretBarrel, &ent, ent.hModel, "tag_turret" );
    VectorCopy( cent->lerpOrigin, turretBarrel.lightingOrigin );
    AnglesToAxis( es->angles2, flatAxis );

    RotatePointAroundVector( turretBarrel.axis[ 0 ], xNormal, flatAxis[ 0 ], -rotAngle );
    RotatePointAroundVector( turretBarrel.axis[ 1 ], xNormal, flatAxis[ 1 ], -rotAngle );
    RotatePointAroundVector( turretBarrel.axis[ 2 ], xNormal, flatAxis[ 2 ], -rotAngle );
    
    turretBarrel.oldframe = ent.oldframe;
    turretBarrel.frame    = ent.frame;
    turretBarrel.backlerp = ent.backlerp;

    trap_R_AddRefEntityToScene( &turretBarrel );
  }

  //turret barrel bit
  if( cg_buildables[ es->modelindex ].models[ 2 ] )
  {
    refEntity_t turretTop;
    vec3_t      flatAxis[ 3 ];
    vec3_t      swivelAngles;
    
    memset( &turretTop, 0, sizeof( turretTop ) );

    VectorCopy( es->angles2, swivelAngles );
    swivelAngles[ PITCH ] = 0.0f;
    
    turretTop.hModel = cg_buildables[ es->modelindex ].models[ 2 ];

    CG_PositionRotatedEntityOnTag( &turretTop, &ent, ent.hModel, "tag_turret" );
    VectorCopy( cent->lerpOrigin, turretTop.lightingOrigin );
    AnglesToAxis( swivelAngles, flatAxis );

    RotatePointAroundVector( turretTop.axis[ 0 ], xNormal, flatAxis[ 0 ], -rotAngle );
    RotatePointAroundVector( turretTop.axis[ 1 ], xNormal, flatAxis[ 1 ], -rotAngle );
    RotatePointAroundVector( turretTop.axis[ 2 ], xNormal, flatAxis[ 2 ], -rotAngle );
    
    turretTop.oldframe = ent.oldframe;
    turretTop.frame    = ent.frame;
    turretTop.backlerp = ent.backlerp;

    trap_R_AddRefEntityToScene( &turretTop );
  }
}
