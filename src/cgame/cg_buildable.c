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

sfxHandle_t defaultAlienSounds[ MAX_BUILDABLE_ANIMATIONS ];
sfxHandle_t defaultHumanSounds[ MAX_BUILDABLE_ANIMATIONS ];

#define CREEP_SCALEUP_TIME    3000
#define CREEP_SCALEDOWN_TIME  3000
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

  //should the creep be growing or receding?
  if( cent->miscTime >= 0 )
  {
    msec = cg.time - cent->miscTime;
    if( msec >= 0 && msec < scaleUpTime )
      frac = (float)msec / scaleUpTime;
    else
      frac = 1.0f;
  }
  else if( cent->miscTime < 0 )
  {
    msec = cg.time + cent->miscTime;
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
    CG_ImpactMark( cgs.media.greenBloodMarkShader, origin, cent->currentState.origin2,
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
      Com_Printf( "Failed to load animation file %s\n", filename );

    //sound.cfg
    Com_sprintf( filename, sizeof( filename ), "sound/buildables/%s/sound.cfg", buildableName );
    if ( !CG_ParseBuildableSoundFile( filename, i ) )
      Com_Printf( "Failed to load sound file %s\n", filename );

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
  entityState_t *es = &cent->currentState;
  
  //if no animation is set default to idle anim
  if( cent->buildableAnim == BANIM_NONE )
    cent->buildableAnim = es->torsoAnim;

  if( ( cent->oldBuildableAnim ^ es->legsAnim ) & ANIM_TOGGLEBIT )
  {
    if( cent->buildableAnim == es->torsoAnim || es->legsAnim & ANIM_FORCEBIT )
      cent->buildableAnim = cent->oldBuildableAnim = es->legsAnim;
  }
  
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

#define MAX_SMOKE_TIME  500
#define MIN_SMOKE_TIME  100
#define SMOKE_SPREAD    89.0f
#define SMOKE_LIFETIME  1000

#define MAX_SPARK_TIME  5000
#define MIN_SPARK_TIME  800
#define SPARK_SPREAD    80.0f
#define SPARK_LIFETIME  1500

#define BLEED_TIME      1500
#define BLEED_SPREAD    80.0f
#define BLEED_LIFETIME  1000
#define MAX_BLEED_BLOBS 6

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
  int             smokeTime, sparkTime, i, bleedBlobs;
  vec3_t          origin;
  vec3_t          acc = { 0.0f, 0.0f, 50.0f };
  vec3_t          grav = { 0.0f, 0.0f, -DEFAULT_GRAVITY };
  vec3_t          vel = { 0.0f, 0.0f, 0.0f };

  VectorCopy( cent->lerpOrigin, origin );
  
  if( team == BIT_HUMANS )
  {
    //hack to move particle origin away from ground
    origin[ 2 ] += 8.0f;
    
    if( healthFrac < 0.33f && cent->buildableSmokeTime < cg.time )
    {
      //smoke
      smokeTime = healthFrac * 3 * MAX_SMOKE_TIME;
      if( smokeTime < MIN_SMOKE_TIME )
        smokeTime = MIN_SMOKE_TIME;

      VectorSet( vel, 0.0f, 0.0f, 50.0f );

      CG_LaunchSprite( origin, vel, acc, SMOKE_SPREAD,
                       0.5f, 10.0f, 50.0f, 128.0f, 0.0f,
                       rand( ) % 360, cg.time, cg.time,
                       SMOKE_LIFETIME + ( crandom( ) * ( SMOKE_LIFETIME / 2 ) ),
                       cgs.media.smokePuffShader, qfalse, qfalse );

      cent->buildableSmokeTime = cg.time + smokeTime;
    }

    if( healthFrac < 0.2f && cent->buildableSparkTime < cg.time )
    {
      //sparks
      sparkTime = healthFrac * 5 * MAX_SPARK_TIME;
      if( sparkTime < MIN_SPARK_TIME )
        sparkTime = MIN_SPARK_TIME;

      for( i = 0; i < 3; i++ )
      {
        qhandle_t spark;

        if( rand( ) % 1 )
          spark = cgs.media.gibSpark1;
        else
          spark = cgs.media.gibSpark2;

        VectorSet( vel, 0.0f, 0.0f, 200.0f );
        VectorSet( grav, 0.0f, 0.0f, -DEFAULT_GRAVITY );

        CG_LaunchSprite( origin, vel, grav, SPARK_SPREAD,
                         0.6f, 4.0f, 2.0f, 255.0f, 0.0f,
                         rand( ) % 360, cg.time, cg.time,
                         SPARK_LIFETIME + ( crandom( ) * ( SPARK_LIFETIME / 2 ) ),
                         spark, qfalse, qfalse );
      }
      
      cent->buildableSparkTime = cg.time + sparkTime;
    }
  }
  else if( team == BIT_ALIENS )
  {
    //bleed a bit if damaged
    if( healthFrac < 0.33f && cent->buildableBleedTime < cg.time )
    {
      VectorScale( es->origin2, 100.0f, vel );
      VectorSet( grav, 0.0f, 0.0f, -DEFAULT_GRAVITY/4 );

      bleedBlobs = ( 1.0f - ( healthFrac * 3 ) ) * MAX_BLEED_BLOBS + 1;

      for( i = 0; i < bleedBlobs; i++ )
      {
        CG_LaunchSprite( origin, vel, grav, BLEED_SPREAD,
                         0.0f, 4.0f, 20.0f, 255.0f, 0.0f,
                         rand( ) % 360, cg.time, cg.time,
                         BLEED_LIFETIME + ( crandom( ) * ( BLEED_LIFETIME / 2 ) ),
                         cgs.media.greenBloodTrailShader, qfalse, qfalse );
      }
      
      cent->buildableBleedTime = cg.time + BLEED_TIME;
    }
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
  vec3_t          forward, surfNormal, xNormal, end, start, mins, maxs;
  vec3_t          refNormal = { 0.0f, 0.0f, 1.0f };
  float           rotAngle;
  trace_t         tr;
  buildableTeam_t team = BG_FindTeamForBuildable( es->modelindex );
  
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

  if( es->pos.trType == TR_STATIONARY )
  {
    VectorMA( ent.origin, -TRACE_DEPTH, surfNormal, end );
    VectorMA( ent.origin, 1.0f, surfNormal, start );
    CG_CapTrace( &tr, start, mins, maxs, end, es->number, MASK_PLAYERSOLID );
    VectorMA( ent.origin, tr.fraction * -TRACE_DEPTH, surfNormal, ent.origin );
  }
  
  VectorCopy( ent.origin, ent.oldorigin ); // don't positionally lerp at all
  VectorCopy( ent.origin, ent.lightingOrigin );

  ent.hModel = cg_buildables[ es->modelindex ].models[ 0 ];

  ent.nonNormalizedAxes = qfalse;

  //add creep
  if( team == BIT_ALIENS )
  {
    CG_Creep( cent );
    
    if( es->generic1 & B_SPAWNED_TOGGLEBIT )
    {
      //run animations
      CG_BuildableAnimation( cent, &ent.oldframe, &ent.frame, &ent.backlerp );
    }
  }
  else if( team == BIT_HUMANS )
  {
    if( !( es->generic1 & B_SPAWNED_TOGGLEBIT ) )
      ent.customShader = cgs.media.humanSpawningShader;
    else
      CG_BuildableAnimation( cent, &ent.oldframe, &ent.frame, &ent.backlerp );
  }

  // add to refresh list
  trap_R_AddRefEntityToScene( &ent );

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

  if( cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME && es->eFlags & EF_FIRING )
  {
    weaponInfo_t *weapon = &cg_weapons[ es->weapon ];
    
    if( weapon->flashDlightColor[ 0 ] || weapon->flashDlightColor[ 1 ] || weapon->flashDlightColor[ 2 ] )
    {
      trap_R_AddLightToScene( cent->lerpOrigin, 300 + ( rand( ) & 31 ), weapon->flashDlightColor[ 0 ],
        weapon->flashDlightColor[ 1 ], weapon->flashDlightColor[ 2 ] );
    }
  }
    
  //smoke etc for damaged buildables
  CG_BuildableParticleEffects( cent );
}
