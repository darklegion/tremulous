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

char *cg_buildableSoundNames[ MAX_BUILDABLE_ANIMATIONS ] =
{
  "construct1.wav",
  "construct2.wav",
  "idle1.wav",
  "idle2.wav",
  "idle3.wav",
  "attack1.wav",
  "attack2.wav",
  "spawn1.wav",
  "spawn2.wav",
  "pain1.wav",
  "pain2.wav",
  "destroy1.wav",
  "destroy2.wav",
  "destroyed.wav"
};

static sfxHandle_t defaultAlienSounds[ MAX_BUILDABLE_ANIMATIONS ];
static sfxHandle_t defaultHumanSounds[ MAX_BUILDABLE_ANIMATIONS ];


/*
==================
CG_AlienBuildableExplosionFragment
==================
*/
void CG_AlienBuildableExplosionFragment( vec3_t origin, vec3_t velocity, qhandle_t hModel )
{
  localEntity_t *le;
  refEntity_t   *re;

  le = CG_AllocLocalEntity( );
  re = &le->refEntity;

  le->leType = LE_FRAGMENT;
  le->startTime = cg.time;
  le->endTime = le->startTime + 5000 + random( ) * 3000;

  VectorCopy( origin, re->origin );
  AxisCopy( axisDefault, re->axis );
  re->hModel = hModel;

  le->pos.trType = TR_GRAVITY;
  VectorCopy( origin, le->pos.trBase );
  VectorCopy( velocity, le->pos.trDelta );
  le->pos.trTime = cg.time;

  le->bounceFactor = 0.3;

  le->leBounceSoundType = LEBS_BLOOD;
  le->leMarkType = LEMT_GREENBLOOD;
}

/*
===================
CG_AlienBuildableExplosion

Generated a bunch of gibs launching out from a location
===================
*/
#define GGIB_VELOCITY   250
#define GGIB_JUMP       250
void CG_AlienBuildableExplosion( vec3_t origin, vec3_t dir )
{
  vec3_t            velocity;
  int               count;
  particleSystem_t  *ps;

  trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.alienBuildableExplosion );
  
  // allow gibs to be turned off for speed
  if( cg_gibs.integer )
  {
    for( count = 0; count <= 9; count++ )
    {
      velocity[ 0 ] = crandom( ) * GGIB_VELOCITY;
      velocity[ 1 ] = crandom( ) * GGIB_VELOCITY;
      velocity[ 2 ] = GGIB_JUMP + crandom( ) * GGIB_VELOCITY;
      CG_AlienBuildableExplosionFragment( origin, velocity, cgs.media.gibAbdomen );
    }
  }
  
  //particle system
  ps = CG_SpawnNewParticleSystem( cgs.media.alienBuildableDestroyedPS );
  CG_SetParticleSystemOrigin( ps, origin );
  CG_SetParticleSystemNormal( ps, dir );
  CG_AttachParticleSystemToOrigin( ps );
}


/*
==================
CG_HumanBuildableExplosionFragment
==================
*/
static void CG_HumanBuildableExplosionFragment( vec3_t origin, vec3_t velocity, qhandle_t hModel )
{
  localEntity_t *le;
  refEntity_t   *re;

  le = CG_AllocLocalEntity( );
  re = &le->refEntity;

  le->leType = LE_FRAGMENT;
  le->startTime = cg.time;
  le->endTime = le->startTime + 10000 + ( crandom( ) * 3000 );

  VectorCopy( origin, re->origin );
  AxisCopy( axisDefault, re->axis );
  re->hModel = hModel;
  re->shaderTime = cg.time / 1000.0f;

  le->pos.trType = TR_GRAVITY;
  VectorCopy( origin, le->pos.trBase );
  VectorCopy( velocity, le->pos.trDelta );
  le->pos.trTime = cg.time;

  le->bounceFactor = 0.3;

  le->leBounceSoundType = LEBS_BANG;
  le->leMarkType = LEMT_BANG;
}


/*
=================
CG_HumanBuildableExplosion

Called for human buildables as they are destroyed
=================
*/
#define EXF_VELOCITY  250
#define EXF_JUMP      250
void CG_HumanBuildableExplosion( vec3_t origin, vec3_t dir )
{
  vec3_t            lightColor, fragOrigin, velocity;
  localEntity_t     *le;
  int               i;
  qhandle_t         gibModel;
  particleSystem_t  *ps;

  lightColor[ 0 ] = 1;
  lightColor[ 1 ] = 0.75;
  lightColor[ 2 ] = 0.0;

  trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.humanBuildableExplosion );

  //
  // create the explosion -- only the light is important really
  //
  le = CG_MakeExplosion( origin, dir, cgs.media.dishFlashModel,
                                      cgs.media.explosionShader,
                                      1000, qtrue );
  le->light = 300;
  VectorCopy( lightColor, le->lightColor );

  //fragments
  for( i = 1; i <= 8; i++ )
  {
    VectorCopy( origin, fragOrigin );
    velocity[ 0 ] = crandom( ) * EXF_VELOCITY;
    velocity[ 1 ] = crandom( ) * EXF_VELOCITY;
    velocity[ 2 ] = EXF_JUMP + crandom( ) * EXF_VELOCITY;
    
    switch( i )
    {
      case 1: gibModel = cgs.media.metalGib1; break;
      case 2: gibModel = cgs.media.metalGib2; break;
      case 3: gibModel = cgs.media.metalGib3; break;
      case 4: gibModel = cgs.media.metalGib4; break;
      case 5: gibModel = cgs.media.metalGib5; break;
      case 6: gibModel = cgs.media.metalGib6; break;
      case 7: gibModel = cgs.media.metalGib7; break;
      case 8: gibModel = cgs.media.metalGib8; break;
    }
    
    CG_HumanBuildableExplosionFragment( fragOrigin, velocity, gibModel );
  }
  
  //particle system
  ps = CG_SpawnNewParticleSystem( cgs.media.humanBuildableDestroyedPS );
  CG_SetParticleSystemOrigin( ps, origin );
  CG_SetParticleSystemNormal( ps, dir );
  CG_AttachParticleSystemToOrigin( ps );
}


#define CREEP_SIZE            64.0f

/*
==================
CG_Creep
==================
*/
static void CG_Creep( centity_t *cent )
{
  polyVert_t    verts[ 4 ];
  vec3_t        square[ 4 ];
  vec2_t        tex[ 4 ];
  int           i, msec, seed;
  float         size, newsize, frac;
  float         length;
  trace_t       tr, tr2;
  vec3_t        temp, origin, p1, p2;
  int           scaleUpTime = BG_FindBuildTimeForBuildable( cent->currentState.modelindex );
  int           time;

  time = cent->currentState.time;

  //should the creep be growing or receding?
  if( time >= 0 )
  {
    msec = cg.time - time;
    if( msec >= 0 && msec < scaleUpTime )
      frac = (float)msec / scaleUpTime;
    else
      frac = 1.0f;
  }
  else if( time < 0 )
  {
    msec = cg.time + time;
    if( msec >= 0 && msec < CREEP_SCALEDOWN_TIME )
      frac = 1.0f - ( (float)msec / CREEP_SCALEDOWN_TIME );
    else
      frac = 0.0f;
  }

  VectorCopy( cent->currentState.origin2, temp );
  VectorScale( temp, -4096, temp );
  VectorAdd( temp, cent->lerpOrigin, temp );

  CG_Trace( &tr, cent->lerpOrigin, NULL, NULL, temp, cent->currentState.number, MASK_SOLID );

  VectorCopy( tr.endpos, origin );

  size = CREEP_SIZE * frac;

  if( size > 0.0f )
    CG_ImpactMark( cgs.media.creepShader, origin, cent->currentState.origin2,
                   0.0f, 1.0f, 1.0f, 1.0f, 1.0f, qfalse, size, qtrue );
}

/*
======================
CG_ParseBuildableAnimationFile

Read a configuration file containing animation counts and rates
models/buildables/hivemind/animation.cfg, etc
======================
*/
static qboolean CG_ParseBuildableAnimationFile( const char *filename, buildable_t buildable )
{
  char          *text_p, *prev;
  int           len;
  int           i;
  char          *token;
  float         fps;
  char          text[ 20000 ];
  fileHandle_t  f;
  animation_t   *animations;

  animations = cg_buildables[ buildable ].animations;

  // load the file
  len = trap_FS_FOpenFile( filename, &f, FS_READ );
  if( len <= 0 )
    return qfalse;
    
  if( len >= sizeof( text ) - 1 )
  {
    CG_Printf( "File %s too long\n", filename );
    return qfalse;
  }
  
  trap_FS_Read( text, len, f );
  text[ len ] = 0;
  trap_FS_FCloseFile( f );

  // parse the text
  text_p = text;

  // read information for each frame
  for( i = BANIM_NONE + 1; i < MAX_BUILDABLE_ANIMATIONS; i++ )
  {

    token = COM_Parse( &text_p );
    if( !*token )
      break;
      
    animations[ i ].firstFrame = atoi( token );

    token = COM_Parse( &text_p );
    if( !*token )
      break;
      
    animations[ i ].numFrames = atoi( token );
    animations[ i ].reversed = qfalse;
    animations[ i ].flipflop = qfalse;
    
    // if numFrames is negative the animation is reversed
    if( animations[ i ].numFrames < 0 )
    {
      animations[ i ].numFrames = -animations[ i ].numFrames;
      animations[ i ].reversed = qtrue;
    }

    token = COM_Parse( &text_p );
    if ( !*token )
      break;
      
    animations[i].loopFrames = atoi( token );

    token = COM_Parse( &text_p );
    if( !*token )
      break;
      
    fps = atof( token );
    if( fps == 0 )
      fps = 1;
    
    animations[ i ].frameLerp = 1000 / fps;
    animations[ i ].initialLerp = 1000 / fps;
  }

  if( i != MAX_BUILDABLE_ANIMATIONS )
  {
    CG_Printf( "Error parsing animation file: %s\n", filename );
    return qfalse;
  }
  
  return qtrue;
}

/*
======================
CG_ParseBuildableSoundFile

Read a configuration file containing sound properties
sound/buildables/hivemind/sound.cfg, etc
======================
*/
static qboolean CG_ParseBuildableSoundFile( const char *filename, buildable_t buildable )
{
  char          *text_p, *prev;
  int           len;
  int           i;
  char          *token;
  char          text[ 20000 ];
  fileHandle_t  f;
  sound_t       *sounds;

  sounds = cg_buildables[ buildable ].sounds;

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

  // read information for each frame
  for( i = BANIM_NONE + 1; i < MAX_BUILDABLE_ANIMATIONS; i++ )
  {

    token = COM_Parse( &text_p );
    if ( !*token )
      break;
      
    sounds[ i ].enabled = atoi( token );

    token = COM_Parse( &text_p );
    if ( !*token )
      break;
      
    sounds[ i ].looped = atoi( token );
    
  }

  if( i != MAX_BUILDABLE_ANIMATIONS )
  {
    CG_Printf( "Error parsing sound file: %s\n", filename );
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
  char          filename[ MAX_QPATH ];
  char          soundfile[ MAX_QPATH ];
  char          *buildableName;
  char          *modelFile;
  int           i;
  int           j;
  fileHandle_t  f;

  memset( cg_buildables, 0, sizeof( cg_buildables ) );

  //default sounds
  for( j = BANIM_NONE + 1; j < MAX_BUILDABLE_ANIMATIONS; j++ )
  {
    strcpy( soundfile, cg_buildableSoundNames[ j - 1 ] );
    
    Com_sprintf( filename, sizeof( filename ), "sound/buildables/alien/%s", soundfile );
    defaultAlienSounds[ j ] = trap_S_RegisterSound( filename, qfalse );
    
    Com_sprintf( filename, sizeof( filename ), "sound/buildables/human/%s", soundfile );
    defaultHumanSounds[ j ] = trap_S_RegisterSound( filename, qfalse );
  }
  
  cg.buildablesFraction = 0.0f;
  
  for( i = BA_NONE + 1; i < BA_NUM_BUILDABLES; i++ )
  {
    buildableName = BG_FindNameForBuildable( i );
    
    //animation.cfg
    Com_sprintf( filename, sizeof( filename ), "models/buildables/%s/animation.cfg", buildableName );
    if ( !CG_ParseBuildableAnimationFile( filename, i ) )
      Com_Printf( S_COLOR_YELLOW "WARNING: failed to load animation file %s\n", filename );

    //sound.cfg
    Com_sprintf( filename, sizeof( filename ), "sound/buildables/%s/sound.cfg", buildableName );
    if ( !CG_ParseBuildableSoundFile( filename, i ) )
      Com_Printf( S_COLOR_YELLOW "WARNING: failed to load sound file %s\n", filename );

    //models
    for( j = 0; j <= 3; j++ )
    {
      if( modelFile = BG_FindModelsForBuildable( i, j ) )
        cg_buildables[ i ].models[ j ] = trap_R_RegisterModel( modelFile );
    }

    //sounds
    for( j = BANIM_NONE + 1; j < MAX_BUILDABLE_ANIMATIONS; j++ )
    {
      strcpy( soundfile, cg_buildableSoundNames[ j - 1 ] );
      Com_sprintf( filename, sizeof( filename ), "sound/buildables/%s/%s", buildableName, soundfile );

      if( cg_buildables[ i ].sounds[ j ].enabled )
      {
        if( trap_FS_FOpenFile( filename, &f, FS_READ ) > 0 )
        {
          //file exists so close it
          trap_FS_FCloseFile( f );

          cg_buildables[ i ].sounds[ j ].sound = trap_S_RegisterSound( filename, qfalse );
        }
        else
        {
          //file doesn't exist - use default
          if( BG_FindTeamForBuildable( i ) == BIT_ALIENS )
            cg_buildables[ i ].sounds[ j ].sound = defaultAlienSounds[ j ];
          else
            cg_buildables[ i ].sounds[ j ].sound = defaultHumanSounds[ j ];
        }
      }
    }

    cg.buildablesFraction = (float)i / (float)( BA_NUM_BUILDABLES - 1 );
    trap_UpdateScreen( );
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

  if( newAnimation < 0 || newAnimation >= MAX_BUILDABLE_ANIMATIONS )
    CG_Error( "Bad animation number: %i", newAnimation );

  anim = &cg_buildables[ buildable ].animations[ newAnimation ];

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
  buildable_t           buildable = cent->currentState.modelindex;
  lerpFrame_t           *lf = &cent->lerpFrame;
  animation_t           *anim;
  buildableAnimNumber_t newAnimation = cent->buildableAnim & ~( ANIM_TOGGLEBIT|ANIM_FORCEBIT );

  // debugging tool to get no animations
  if( cg_animSpeed.integer == 0 )
  {
    lf->oldFrame = lf->frame = lf->backlerp = 0;
    return;
  }

  // see if the animation sequence is switching
  if( newAnimation != lf->animationNumber || !lf->animation )
  {
    CG_SetBuildableLerpFrameAnimation( buildable, lf, newAnimation );

    if( !cg_buildables[ buildable ].sounds[ newAnimation ].looped &&
        cg_buildables[ buildable ].sounds[ newAnimation ].enabled )
      trap_S_StartSound( cent->lerpOrigin, cent->currentState.number, CHAN_AUTO,
        cg_buildables[ buildable ].sounds[ newAnimation ].sound );
  }

  if( cg_buildables[ buildable ].sounds[ lf->animationNumber ].looped &&
      cg_buildables[ buildable ].sounds[ lf->animationNumber ].enabled )
    trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin,
      cg_buildables[ buildable ].sounds[ lf->animationNumber ].sound );

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
    else if( anim->flipflop && f >= anim->numFrames )
      lf->frame = anim->firstFrame + anim->numFrames - 1 - ( f % anim->numFrames );
    else
      lf->frame = anim->firstFrame + f;
      
    if( cg.time > lf->frameTime )
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
  if( lf->frameTime == lf->oldFrameTime )
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
  entityState_t *es = &cent->currentState;
  
  //if no animation is set default to idle anim
  if( cent->buildableAnim == BANIM_NONE )
    cent->buildableAnim = es->torsoAnim;
  
  //display the first frame of the construction anim if not yet spawned
  if( !( es->generic1 & B_SPAWNED_TOGGLEBIT ) )
  {
    animation_t *anim = &cg_buildables[ es->modelindex ].animations[ BANIM_CONSTRUCT1 ];
    
    //so that when animation starts for real it has sensible numbers
    cent->lerpFrame.oldFrameTime =
      cent->lerpFrame.frameTime =
      cent->lerpFrame.animationTime =
      cg.time;

    *old      = cent->lerpFrame.oldFrame = anim->firstFrame;
    *now      = cent->lerpFrame.frame    = anim->firstFrame;
    *backLerp = cent->lerpFrame.backlerp = 0.0f;

    //ensure that an animation is triggered once the buildable has spawned
    cent->oldBuildableAnim = BANIM_NONE;
  }
  else
  {
    if( ( cent->oldBuildableAnim ^ es->legsAnim ) & ANIM_TOGGLEBIT )
    {
      if( cg_debugAnim.integer )
        CG_Printf( "%d->%d l:%d t:%d %s(%d)\n",
                   cent->oldBuildableAnim, cent->buildableAnim,
                   es->legsAnim, es->torsoAnim,
                   BG_FindHumanNameForBuildable( es->modelindex ), es->number );

      if( cent->buildableAnim == es->torsoAnim || es->legsAnim & ANIM_FORCEBIT )
        cent->buildableAnim = cent->oldBuildableAnim = es->legsAnim;
      else
        cent->buildableAnim = cent->oldBuildableAnim = es->torsoAnim;
    }
    
    CG_RunBuildableLerpFrame( cent );

    *old      = cent->lerpFrame.oldFrame;
    *now      = cent->lerpFrame.frame;
    *backLerp = cent->lerpFrame.backlerp;
  }
}

#define TRACE_DEPTH 64.0f

/*
===============
CG_PositionAndOrientateBuildable
===============
*/
static void CG_PositionAndOrientateBuildable( const vec3_t angles, const vec3_t inOrigin,
                                              const vec3_t normal, const int skipNumber,
                                              const vec3_t mins, const vec3_t maxs,
                                              vec3_t outAxis[ 3 ], vec3_t outOrigin )
{
  vec3_t  forward, start, end;
  trace_t tr;
  
  AngleVectors( angles, forward, NULL, NULL );
  VectorCopy( normal, outAxis[ 2 ] );
  ProjectPointOnPlane( outAxis[ 0 ], forward, outAxis[ 2 ] );
  
  if( !VectorNormalize( outAxis[ 0 ] ) )
  {
    AngleVectors( angles, NULL, NULL, forward );
    ProjectPointOnPlane( outAxis[ 0 ], forward, outAxis[ 2 ] );
    VectorNormalize( outAxis[ 0 ] );
  }
  
  CrossProduct( outAxis[ 0 ], outAxis[ 2 ], outAxis[ 1 ] );
  outAxis[ 1 ][ 0 ] = -outAxis[ 1 ][ 0 ];
  outAxis[ 1 ][ 1 ] = -outAxis[ 1 ][ 1 ];
  outAxis[ 1 ][ 2 ] = -outAxis[ 1 ][ 2 ];

  VectorMA( inOrigin, -TRACE_DEPTH, normal, end );
  VectorMA( inOrigin, 1.0f, normal, start );
  CG_CapTrace( &tr, start, mins, maxs, end, skipNumber, MASK_PLAYERSOLID );
  
  if( tr.fraction == 1.0f )
  {
    //erm we missed completely - try again with a box trace
    CG_Trace( &tr, start, mins, maxs, end, skipNumber, MASK_PLAYERSOLID );
  }
  
  VectorMA( inOrigin, tr.fraction * -TRACE_DEPTH, normal, outOrigin );
}

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
  
  memset( &ent, 0, sizeof( ent ) );

  BG_FindBBoxForBuildable( buildable, mins, maxs );

  BG_PositionBuildableRelativeToPlayer( ps, mins, maxs, CG_Trace, entity_origin, angles, &tr );

  CG_PositionAndOrientateBuildable( ps->viewangles, entity_origin, tr.plane.normal, ps->clientNum,
                                    mins, maxs, ent.axis, ent.origin );

  //offset on the Z axis if required
  VectorMA( ent.origin, BG_FindZOffsetForBuildable( buildable ), tr.plane.normal, ent.origin );
  
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
CG_BuildableParticleEffects
==================
*/
static void CG_BuildableParticleEffects( centity_t *cent )
{
  entityState_t   *es = &cent->currentState;
  buildableTeam_t team = BG_FindTeamForBuildable( es->modelindex );
  int             health = es->generic1 & ~( B_POWERED_TOGGLEBIT | B_DCCED_TOGGLEBIT | B_SPAWNED_TOGGLEBIT );
  float           healthFrac = (float)health / B_HEALTH_SCALE;
  
  if( !( es->generic1 & B_SPAWNED_TOGGLEBIT ) )
    return;
  
  if( team == BIT_HUMANS )
  {
    if( healthFrac < 0.33f && !CG_IsParticleSystemValid( &cent->buildablePS ) )
    {
      cent->buildablePS = CG_SpawnNewParticleSystem( cgs.media.humanBuildableDamagedPS );
      CG_SetParticleSystemCent( cent->buildablePS, cent );
      CG_AttachParticleSystemToCent( cent->buildablePS );
    }
    else if( healthFrac >= 0.33f && CG_IsParticleSystemValid( &cent->buildablePS ) )
      CG_DestroyParticleSystem( &cent->buildablePS );
  }
  else if( team == BIT_ALIENS )
  {
    if( healthFrac < 0.33f && !CG_IsParticleSystemValid( &cent->buildablePS ) )
    {
      cent->buildablePS = CG_SpawnNewParticleSystem( cgs.media.alienBuildableDamagedPS );
      CG_SetParticleSystemCent( cent->buildablePS, cent );
      CG_SetParticleSystemNormal( cent->buildablePS, es->origin2 );
      CG_AttachParticleSystemToCent( cent->buildablePS );
    }
    else if( healthFrac >= 0.33f && CG_IsParticleSystemValid( &cent->buildablePS ) )
      CG_DestroyParticleSystem( &cent->buildablePS );
  }
}


#define HEALTH_BAR_WIDTH  50.0f
#define HEALTH_BAR_HEIGHT 5.0f

/*
==================
CG_BuildableHealthBar
==================
*/
static void CG_BuildableHealthBar( centity_t *cent )
{
  vec3_t          origin, origin2, down, right, back, downLength, rightLength;
  float           rimWidth = HEALTH_BAR_HEIGHT / 15.0f;
  float           doneWidth, leftWidth, progress;
  int             health;
  qhandle_t       shader;
  entityState_t   *es;
  vec3_t          mins, maxs;

  es = &cent->currentState;

  health = es->generic1 & ~( B_POWERED_TOGGLEBIT | B_DCCED_TOGGLEBIT | B_SPAWNED_TOGGLEBIT );
  progress = (float)health / B_HEALTH_SCALE;
  
  if( progress < 0.0f )
    progress = 0.0f;
  else if( progress > 1.0f )
    progress = 1.0f;
  
  if( progress < 0.33f )
    shader = cgs.media.redBuildShader;
  else
    shader = cgs.media.greenBuildShader;
  
  doneWidth = ( HEALTH_BAR_WIDTH - 2 * rimWidth ) * progress;
  leftWidth = ( HEALTH_BAR_WIDTH - 2 * rimWidth ) - doneWidth;
  
  VectorCopy( cg.refdef.viewaxis[ 2 ], down );
  VectorInverse( down );
  VectorCopy( cg.refdef.viewaxis[ 1 ], right );
  VectorInverse( right );
  VectorSubtract( cg.refdef.vieworg, cent->lerpOrigin, back );
  VectorNormalize( back );
  VectorCopy( cent->lerpOrigin, origin );

  BG_FindBBoxForBuildable( es->modelindex, mins, maxs );
  VectorMA( origin, 48.0f, es->origin2, origin );
  VectorMA( origin, -HEALTH_BAR_WIDTH / 2.0f, right, origin );
  VectorMA( origin, maxs[ 0 ] + 8.0f, back, origin );
  
  VectorCopy( origin, origin2 );
  VectorScale( right, rimWidth + doneWidth, rightLength );
  VectorScale( down, HEALTH_BAR_HEIGHT, downLength );
  CG_DrawPlane( origin2, downLength, rightLength, shader );
  
  VectorMA( origin, rimWidth + doneWidth, right, origin2 );
  VectorScale( right, leftWidth, rightLength );
  VectorScale( down, rimWidth, downLength );
  CG_DrawPlane( origin2, downLength, rightLength, shader );
  
  VectorMA( origin, rimWidth + doneWidth, right, origin2 );
  VectorMA( origin2, HEALTH_BAR_HEIGHT - rimWidth, down, origin2 );
  VectorScale( right, leftWidth, rightLength );
  VectorScale( down, rimWidth, downLength );
  CG_DrawPlane( origin2, downLength, rightLength, shader );
  
  VectorMA( origin, HEALTH_BAR_WIDTH - rimWidth, right, origin2 );
  VectorScale( right, rimWidth, rightLength );
  VectorScale( down, HEALTH_BAR_HEIGHT, downLength );
  CG_DrawPlane( origin2, downLength, rightLength, shader );
  
  if( !( es->generic1 & B_POWERED_TOGGLEBIT ) &&
      BG_FindTeamForBuildable( es->modelindex ) == BIT_HUMANS )
  {
    VectorMA( origin, 15.0f, right, origin2 );
    VectorMA( origin2, HEALTH_BAR_HEIGHT + 5.0f, down, origin2 );
    VectorScale( right, HEALTH_BAR_WIDTH / 2.0f - 5.0f, rightLength );
    VectorScale( down,  HEALTH_BAR_WIDTH / 2.0f - 5.0f, downLength );
    CG_DrawPlane( origin2, downLength, rightLength, cgs.media.noPowerShader );
  }
}

/*
==================
CG_Buildable
==================
*/
void CG_Buildable( centity_t *cent )
{
  refEntity_t     ent;
  entityState_t   *es = &cent->currentState;
  vec3_t          angles;
  vec3_t          forward, surfNormal, xNormal, mins, maxs;
  vec3_t          refNormal = { 0.0f, 0.0f, 1.0f };
  float           rotAngle;
  buildableTeam_t team = BG_FindTeamForBuildable( es->modelindex );
  
  //must be before EF_NODRAW check
  if( team == BIT_ALIENS )
    CG_Creep( cent );
  
  // if set to invisible, skip
  if( es->eFlags & EF_NODRAW )
  {
    if( CG_IsParticleSystemValid( &cent->buildablePS ) )
      CG_DestroyParticleSystem( &cent->buildablePS );

    return;
  }

  memset ( &ent, 0, sizeof( ent ) );

  VectorCopy( cent->lerpOrigin, ent.origin );
  VectorCopy( cent->lerpOrigin, ent.oldorigin );
  VectorCopy( cent->lerpOrigin, ent.lightingOrigin );

  VectorCopy( es->origin2, surfNormal );
  
  VectorCopy( es->angles, angles );
  BG_FindBBoxForBuildable( es->modelindex, mins, maxs );
  
  if( es->pos.trType == TR_STATIONARY )
    CG_PositionAndOrientateBuildable( angles, ent.origin, surfNormal, es->number,
                                      mins, maxs, ent.axis, ent.origin );
  
  //offset on the Z axis if required
  VectorMA( ent.origin, BG_FindZOffsetForBuildable( es->modelindex ), surfNormal, ent.origin );
  
  VectorCopy( ent.origin, ent.oldorigin ); // don't positionally lerp at all
  VectorCopy( ent.origin, ent.lightingOrigin );

  ent.hModel = cg_buildables[ es->modelindex ].models[ 0 ];

  ent.nonNormalizedAxes = qfalse;

  if( !( es->generic1 & B_SPAWNED_TOGGLEBIT ) )
  {
    sfxHandle_t prebuildSound;
    
    if( team == BIT_HUMANS )
    {
      ent.customShader = cgs.media.humanSpawningShader;
      prebuildSound = cgs.media.humanBuildablePrebuild;
    }
    else if( team == BIT_ALIENS )
      prebuildSound = cgs.media.alienBuildablePrebuild;

    trap_S_AddLoopingSound( es->number, cent->lerpOrigin, vec3_origin, prebuildSound );
  }
      
  CG_BuildableAnimation( cent, &ent.oldframe, &ent.frame, &ent.backlerp );

  //add to refresh list
  trap_R_AddRefEntityToScene( &ent );

  CrossProduct( surfNormal, refNormal, xNormal );
  VectorNormalize( xNormal );
  rotAngle = RAD2DEG( acos( DotProduct( surfNormal, refNormal ) ) );
  
  //turret barrel bit
  if( cg_buildables[ es->modelindex ].models[ 1 ] )
  {
    refEntity_t turretBarrel;
    vec3_t      flatAxis[ 3 ];
    
    memset( &turretBarrel, 0, sizeof( turretBarrel ) );

    turretBarrel.hModel = cg_buildables[ es->modelindex ].models[ 1 ];

    CG_PositionEntityOnTag( &turretBarrel, &ent, ent.hModel, "tag_turret" );
    VectorCopy( cent->lerpOrigin, turretBarrel.lightingOrigin );
    AnglesToAxis( es->angles2, flatAxis );

    RotatePointAroundVector( turretBarrel.axis[ 0 ], xNormal, flatAxis[ 0 ], -rotAngle );
    RotatePointAroundVector( turretBarrel.axis[ 1 ], xNormal, flatAxis[ 1 ], -rotAngle );
    RotatePointAroundVector( turretBarrel.axis[ 2 ], xNormal, flatAxis[ 2 ], -rotAngle );
    
    turretBarrel.oldframe = ent.oldframe;
    turretBarrel.frame    = ent.frame;
    turretBarrel.backlerp = ent.backlerp;

    turretBarrel.customShader = ent.customShader;
    
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

    turretTop.customShader = ent.customShader;
    
    trap_R_AddRefEntityToScene( &turretTop );
  }

  switch( cg.predictedPlayerState.weapon )
  {
    case WP_ABUILD:
    case WP_ABUILD2:
    case WP_HBUILD:
    case WP_HBUILD2:
      if( BG_FindTeamForBuildable( es->modelindex ) ==
          BG_FindTeamForWeapon( cg.predictedPlayerState.weapon ) )
        CG_BuildableHealthBar( cent );
      break;

    default:
      break;
  }

  //weapon effects for turrets
  if( es->eFlags & EF_FIRING )
  {
    weaponInfo_t  *weapon = &cg_weapons[ es->weapon ];
      
    if( cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME || 
        BG_FindProjTypeForBuildable( es->modelindex ) == WP_TESLAGEN )
    {
      if( weapon->wim[ WPM_PRIMARY ].flashDlightColor[ 0 ] ||
          weapon->wim[ WPM_PRIMARY ].flashDlightColor[ 1 ] ||
          weapon->wim[ WPM_PRIMARY ].flashDlightColor[ 2 ] )
      {
        trap_R_AddLightToScene( cent->lerpOrigin, 300 + ( rand( ) & 31 ),
            weapon->wim[ WPM_PRIMARY ].flashDlightColor[ 0 ],
            weapon->wim[ WPM_PRIMARY ].flashDlightColor[ 1 ],
            weapon->wim[ WPM_PRIMARY ].flashDlightColor[ 2 ] );
      }
    }
      
    if( weapon->wim[ WPM_PRIMARY ].firingSound )
    {
      trap_S_AddLoopingSound( es->number, cent->lerpOrigin, vec3_origin, weapon->wim[ WPM_PRIMARY ].firingSound );
    }
    else if( weapon->readySound )
      trap_S_AddLoopingSound( es->number, cent->lerpOrigin, vec3_origin, weapon->readySound );
  }
    
  //smoke etc for damaged buildables
  CG_BuildableParticleEffects( cent );
}
