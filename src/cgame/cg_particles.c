// cg_particles.c -- the particle system

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

static baseParticleSystem_t   baseParticleSystems[ MAX_BASEPARTICLE_SYSTEMS ];
static baseParticleEjector_t  baseParticleEjectors[ MAX_BASEPARTICLE_EJECTORS ];
static baseParticle_t         baseParticles[ MAX_BASEPARTICLES ];
static int                    numBaseParticleSystems = 0;
static int                    numBaseParticleEjectors = 0;
static int                    numBaseParticles = 0;

static particleSystem_t     particleSystems[ MAX_PARTICLE_SYSTEMS ];
static particleEjector_t    particleEjectors[ MAX_PARTICLE_EJECTORS ];
static particle_t           particles[ MAX_PARTICLES ];
static particle_t           sortParticles[ MAX_PARTICLES ];

/*
===============
CG_LerpValues

Lerp between two values
===============
*/
static float CG_LerpValues( float a, float b, float f )
{
  if( b == PARTICLES_SAME_AS_INITIAL )
    return a;
  else
    return ( (a) + (f) * ( (b) - (a) ) );
}

/*
===============
CG_RandomiseValue

Randomise some value by some variance
===============
*/
static float CG_RandomiseValue( float value, float variance )
{
  if( value != 0.0f )
    return value * ( 1.0f + ( random( ) * variance ) );
  else
    return random( ) * variance;
}

/*
===============
CG_SpreadVector

Randomly spread a vector by some amount
===============
*/
static void CG_SpreadVector( vec3_t v, float spread )
{
  vec3_t  p, r1, r2;
  float   randomSpread = crandom( ) * spread;
  float   randomRotation = random( ) * 360.0f;

  PerpendicularVector( p, v );

  RotatePointAroundVector( r1, p, v, randomSpread );
  RotatePointAroundVector( r2, v, r1, randomRotation );

  VectorCopy( r2, v );
}

/*
===============
CG_SpawnNewParticle

Introduce a new particle into the world
===============
*/
static particle_t *CG_SpawnNewParticle( baseParticle_t *bp, particleEjector_t *parent )
{
  int                     i, j, start;
  particle_t              *p = NULL;
  particleEjector_t       *pe = parent;
  particleSystem_t        *ps = parent->parent;
  vec3_t                  forward;
  centity_t               *cent = &cg_entities[ ps->attachment.centNum ];

  for( i = 0; i < MAX_PARTICLES; i++ )
  {
    p = &particles[ i ];
    
    if( !p->valid )
    {
      //found a free slot
      p->class = bp;
      p->parent = pe;

      p->birthTime = cg.time;
      p->lifeTime = (int)CG_RandomiseValue( (float)bp->lifeTime, bp->lifeTimeRandFrac );
      
      p->radius.delay = (int)CG_RandomiseValue( (float)bp->radius.delay, bp->radius.delayRandFrac );
      p->radius.initial = CG_RandomiseValue( bp->radius.initial, bp->radius.initialRandFrac );
      p->radius.final = CG_RandomiseValue( bp->radius.final, bp->radius.finalRandFrac );
      
      p->alpha.delay = (int)CG_RandomiseValue( (float)bp->alpha.delay, bp->alpha.delayRandFrac );
      p->alpha.initial = CG_RandomiseValue( bp->alpha.initial, bp->alpha.initialRandFrac );
      p->alpha.final = CG_RandomiseValue( bp->alpha.final, bp->alpha.finalRandFrac );
      
      p->rotation.delay = (int)CG_RandomiseValue( (float)bp->rotation.delay, bp->rotation.delayRandFrac );
      p->rotation.initial = CG_RandomiseValue( bp->rotation.initial, bp->rotation.initialRandFrac );
      p->rotation.final = CG_RandomiseValue( bp->rotation.final, bp->rotation.finalRandFrac );
      
      switch( ps->attachType )
      {
        case PSA_STATIC:
          if( !ps->attachment.staticValid )
            return NULL;
          
          VectorCopy( ps->attachment.origin, p->origin );
          break;

        case PSA_TAG:
          if( !ps->attachment.tagValid )
            return NULL;
          
          AxisCopy( axisDefault, ps->attachment.re.axis );
          CG_PositionRotatedEntityOnTag( &ps->attachment.re, &ps->attachment.parent,
                                         ps->attachment.model, ps->attachment.tagName );
          VectorCopy( ps->attachment.re.origin, p->origin );
          break;
        
        case PSA_CENT_ORIGIN:
          if( !ps->attachment.centValid )
            return NULL;
          
          VectorCopy( cent->lerpOrigin, p->origin );
          break;
      }
        
      VectorAdd( p->origin, bp->displacement, p->origin );
      
      for( j = 0; j <= 2; j++ )
        p->origin[ j ] += ( crandom( ) * bp->randDisplacement );

      switch( bp->velMoveType )
      {
        case PMT_STATIC:
          
          if( bp->velMoveValues.dirType == PMD_POINT )
            VectorSubtract( bp->velMoveValues.point, p->origin, p->velocity );
          else if( bp->velMoveValues.dirType == PMD_LINEAR )
            VectorCopy( bp->velMoveValues.dir, p->velocity );
          
          break;

        case PMT_TAG:
          
          if( !ps->attachment.tagValid )
            return NULL;
          
          if( bp->velMoveValues.dirType == PMD_POINT )
            VectorSubtract( ps->attachment.re.origin, p->origin, p->velocity );
          else if( bp->velMoveValues.dirType == PMD_LINEAR )
            VectorCopy( ps->attachment.re.axis[ 0 ], p->velocity );
          
          break;

        case PMT_CENT_ANGLES:
          
          if( !ps->attachment.centValid )
            return NULL;
          
          if( bp->velMoveValues.dirType == PMD_POINT )
            VectorSubtract( cent->lerpOrigin, p->origin, p->velocity );
          else if( bp->velMoveValues.dirType == PMD_LINEAR )
          {
            AngleVectors( cent->lerpAngles, forward, NULL, NULL );
            VectorCopy( forward, p->velocity );
          }
          
          break;
          
      case PMT_NORMAL:
        
        if( !ps->attachment.normalValid )
          return NULL;
        
        VectorCopy( ps->attachment.normal, p->velocity );

        //normal displacement
        VectorNormalize( p->velocity );
        VectorMA( p->origin, bp->normalDisplacement, p->velocity, p->origin );
        
        break;
      }
            
      VectorNormalize( p->velocity );
      CG_SpreadVector( p->velocity, bp->velMoveValues.dirRandAngle );
      VectorScale( p->velocity,
                   CG_RandomiseValue( bp->velMoveValues.mag, bp->velMoveValues.magRandFrac ),
                   p->velocity );

      if( ps->attachment.centValid )
      {
        VectorMA( p->velocity,
                  CG_RandomiseValue( bp->velMoveValues.parentVelFrac, bp->velMoveValues.parentVelFracRandFrac ),
                  cent->currentState.pos.trDelta, p->velocity );
      }
          
      p->lastEvalTime = cg.time;

      p->valid = qtrue;
      break;
    }
  }

  return p;
}


/*
===============
CG_SpawnNewParticles

Check if there are any ejectors that should be
introducing new particles
===============
*/
static void CG_SpawnNewParticles( void )
{
  int                   i, j;
  particle_t            *p;
  particleSystem_t      *ps;
  particleEjector_t     *pe;
  baseParticleEjector_t *bpe;
  float                 lerpFrac;
  int                   count;

  for( i = 0; i < MAX_PARTICLE_EJECTORS; i++ )
  {
    pe = &particleEjectors[ i ];
    ps = pe->parent;

    if( pe->valid )
    {
      //a non attached particle system can't make particles
      if( !ps->attached )
        continue;

      bpe = particleEjectors[ i ].class;

      while( pe->nextEjectionTime <= cg.time &&
             ( pe->count > 0 || pe->totalParticles == PARTICLES_INFINITE ) )
      {
        for( j = 0; j < bpe->numParticles; j++ )
          CG_SpawnNewParticle( bpe->particles[ j ], pe );
        
        if( pe->count > 0 )
          pe->count--;

        //calculate next ejection time
        lerpFrac = 1.0 - ( (float)pe->count / (float)pe->totalParticles );
        pe->nextEjectionTime = cg.time + CG_RandomiseValue( 
            CG_LerpValues( pe->ejectPeriod.initial,
                           pe->ejectPeriod.final,
                           lerpFrac ),
            pe->ejectPeriod.randFrac );
      }

      if( !pe->parent->valid )
        pe->valid = qfalse;
      
      if( pe->count == 0 )
      {
        count = 0;

        //wait for child particles to die before declaring this pe invalid
        for( j = 0; j < MAX_PARTICLES; j++ )
        {
          p = &particles[ j ];
          
          if( p->valid && p->parent == pe )
            count++;
        }

        if( !count )
          pe->valid = qfalse;
      }
    }
  }
}


/*
===============
CG_SpawnNewParticleEjector

Allocate a new particle ejector
===============
*/
static particleEjector_t *CG_SpawnNewParticleEjector( baseParticleEjector_t *bpe,
                                                      particleSystem_t *parent )
{
  int                     i, start;
  particleEjector_t       *pe = NULL;
  particleSystem_t        *ps = parent;

  for( i = 0; i < MAX_PARTICLE_EJECTORS; i++ )
  {
    pe = &particleEjectors[ i ];
    
    if( !pe->valid )
    {
      //found a free slot
      pe->class = bpe;
      pe->parent = ps;

      pe->ejectPeriod.initial = bpe->eject.initial;
      pe->ejectPeriod.final = bpe->eject.final;
      pe->ejectPeriod.randFrac = bpe->eject.randFrac;

      pe->nextEjectionTime = cg.time +
        (int)CG_RandomiseValue( (float)bpe->eject.delay, bpe->eject.delayRandFrac );
      pe->count = pe->totalParticles =
        (int)CG_RandomiseValue( (float)bpe->totalParticles, bpe->totalParticlesRandFrac );

      pe->valid = qtrue;
      break;
    }
  }

  return pe;
}


/*
===============
CG_SpawnNewParticleSystem

Allocate a new particle system
===============
*/
particleSystem_t *CG_SpawnNewParticleSystem( qhandle_t psHandle )
{
  int                   i, j, start;
  particleSystem_t      *ps = NULL;
  baseParticleSystem_t  *bps = &baseParticleSystems[ psHandle - 1 ];

  if( !bps->registered )
  {
    CG_Printf( S_COLOR_RED "ERROR: particle system %s has not been registered yet\n",
               bps->name );
    return NULL;
  }
    
  for( i = 0; i < MAX_PARTICLE_SYSTEMS; i++ )
  {
    ps = &particleSystems[ i ];
    
    if( !ps->valid )
    {
      //found a free slot
      ps->class = bps;

      for( j = 0; j < bps->numEjectors; j++ )
        CG_SpawnNewParticleEjector( bps->ejectors[ j ], ps );

      ps->valid = qtrue;
      
      if( cg_debugParticles.integer >= 1 )
        CG_Printf( "PS %s created\n", bps->name );

      break;
    }
  }

  return ps;
}

/*
===============
CG_RegisterParticleSystem

Load the shaders required for a particle system
===============
*/
qhandle_t CG_RegisterParticleSystem( char *name )
{
  int                   i, j, k, l;
  baseParticleSystem_t  *bps;
  baseParticleEjector_t *bpe;
  baseParticle_t        *bp;

  for( i = 0; i < MAX_PARTICLE_SYSTEMS; i++ )
  {
    bps = &baseParticleSystems[ i ];
    
    if( !strcmp( bps->name, name ) )
    {
      for( j = 0; j < bps->numEjectors; j++ )
      {
        bpe = bps->ejectors[ j ];
        
        for( l = 0; l < bpe->numParticles; l++ )
        {
          bp = bpe->particles[ l ];

          for( k = 0; k < bp->numFrames; k++ )
            bp->shaders[ k ] = trap_R_RegisterShader( bp->shaderNames[ k ] );
        }
      }

      bps->registered = qtrue;

      //avoid returning 0
      return i + 1;
    }
  }

  CG_Printf( S_COLOR_YELLOW "WARNING: failed to load particle system %s\n", name );
  return 0;
}


/*
===============
atof_neg

atof with an allowance for negative values
===============
*/
static float atof_neg( char *token, qboolean allowNegative )
{
  float value;

  value = atof( token );

  if( !allowNegative && value < 0.0f )
  {
    CG_Printf( S_COLOR_YELLOW "WARNING: negative value %f is now allowed here, "
                              "replaced with 1.0f\n", value );
    value = 1.0f;
  }

  return value;
}

/*
===============
atoi_neg

atoi with an allowance for negative values
===============
*/
static int atoi_neg( char *token, qboolean allowNegative )
{
  int value;

  value = atoi( token );

  if( !allowNegative && value < 0 )
  {
    CG_Printf( S_COLOR_YELLOW "WARNING: negative value %d is now allowed here, "
                              "replaced with 1\n", value );
    value = 1;
  }

  return value;
}


/*
===============
CG_ParseValueAndVariance

Parse a value and its random variance
===============
*/
static void CG_ParseValueAndVariance( char *token, float *value, float *variance )
{
  char  valueBuffer[ 16 ];
  char  varianceBuffer[ 16 ];
  char  *variancePtr = NULL, *varEndPointer = NULL;
  float localValue = 0.0f;
  float localVariance = 0.0f;

  Q_strncpyz( valueBuffer, token, sizeof( valueBuffer ) );
  Q_strncpyz( varianceBuffer, token, sizeof( varianceBuffer ) );

  variancePtr = strchr( valueBuffer, '~' );
  
  //variance included
  if( variancePtr )
  {
    variancePtr[ 0 ] = '\0';
    variancePtr++;

    localValue = atof_neg( valueBuffer, qfalse );
    
    varEndPointer = strchr( variancePtr, '%' );

    if( varEndPointer )
    {
      varEndPointer[ 0 ] = '\0';
      localVariance = atof_neg( variancePtr, qfalse ) / 100.0f;
    }
    else
    {
      if( localValue != 0.0f )
        localVariance = atof_neg( variancePtr, qfalse ) / localValue;
      else
        localVariance = atof_neg( variancePtr, qfalse );
    }
  }
  else
    localValue = atof_neg( valueBuffer, qfalse );

  if( value != NULL )
    *value = localValue;

  if( variance != NULL )
    *variance = localVariance;
}


/*
===============
CG_ParseParticle

Parse a particle section
===============
*/
static qboolean CG_ParseParticle( baseParticle_t *bp, char **text_p )
{
  char  *token;
  float number, randFrac;
  int   i;

  // read optional parameters
  while( 1 )
  {
    token = COM_Parse( text_p );
    
    if( !token )
      break;

    if( !Q_stricmp( token, "" ) )
      return qfalse;

    if( !Q_stricmp( token, "bounce" ) )
    {
      token = COM_Parse( text_p );
      if( !token )
        break;

      CG_ParseValueAndVariance( token, &number, &randFrac );

      bp->bounceFrac = number;
      bp->bounceFracRandFrac = randFrac;

      continue;
    }
    else if( !Q_stricmp( token, "shader" ) )
    {
      token = COM_Parse( text_p );
      if( !token )
        break;
      
      if( !Q_stricmp( token, "sync" ) )
        bp->framerate = 0.0f;
      else
        bp->framerate = atof_neg( token, qfalse );

      token = COM_ParseExt( text_p, qfalse );

      while( token && token[ 0 ] != 0 )
      {
        Q_strncpyz( bp->shaderNames[ bp->numFrames++ ], token, MAX_QPATH );
        token = COM_ParseExt( text_p, qfalse );
      }
      
      if( !token )
        break;
      
      continue;
    }
    ///
    else if( !Q_stricmp( token, "velocityType" ) )
    {
      token = COM_Parse( text_p );
      if( !token )
        break;
      
      if( !Q_stricmp( token, "static" ) )
        bp->velMoveType = PMT_STATIC;
      else if( !Q_stricmp( token, "tag" ) )
        bp->velMoveType = PMT_TAG;
      else if( !Q_stricmp( token, "cent" ) )
        bp->velMoveType = PMT_CENT_ANGLES;
      else if( !Q_stricmp( token, "normal" ) )
        bp->velMoveType = PMT_NORMAL;

      continue;
    }
    else if( !Q_stricmp( token, "velocityDir" ) )
    {
      token = COM_Parse( text_p );
      if( !token )
        break;
      
      if( !Q_stricmp( token, "linear" ) )
        bp->velMoveValues.dirType = PMD_LINEAR;
      else if( !Q_stricmp( token, "point" ) )
        bp->velMoveValues.dirType = PMD_POINT;

      continue;
    }
    else if( !Q_stricmp( token, "velocityMagnitude" ) )
    {
      token = COM_Parse( text_p );
      if( !token )
        break;
      
      CG_ParseValueAndVariance( token, &number, &randFrac );

      bp->velMoveValues.mag = number;
      bp->velMoveValues.magRandFrac = randFrac;

      continue;
    }
    else if( !Q_stricmp( token, "parentVelocityFraction" ) )
    {
      token = COM_Parse( text_p );
      if( !token )
        break;
      
      CG_ParseValueAndVariance( token, &number, &randFrac );

      bp->velMoveValues.parentVelFrac = number;
      bp->velMoveValues.parentVelFracRandFrac = randFrac;

      continue;
    }
    else if( !Q_stricmp( token, "velocity" ) )
    {
      for( i = 0; i <= 2; i++ )
      {
        token = COM_Parse( text_p );
        if( !token )
          break;
        
        bp->velMoveValues.dir[ i ] = atof_neg( token, qtrue );
      }
      
      token = COM_Parse( text_p );
      if( !token )
        break;
      
      CG_ParseValueAndVariance( token, NULL, &randFrac );

      bp->velMoveValues.dirRandAngle = randFrac;
      
      continue;
    }
    else if( !Q_stricmp( token, "velocityPoint" ) )
    {
      for( i = 0; i <= 2; i++ )
      {
        token = COM_Parse( text_p );
        if( !token )
          break;
        
        bp->velMoveValues.point[ i ] = atof_neg( token, qtrue );
      }
      
      token = COM_Parse( text_p );
      if( !token )
        break;
      
      CG_ParseValueAndVariance( token, NULL, &randFrac );

      bp->velMoveValues.pointRandAngle = randFrac;
      
      continue;
    }
    ///
    else if( !Q_stricmp( token, "accelerationType" ) )
    {
      token = COM_Parse( text_p );
      if( !token )
        break;
      
      if( !Q_stricmp( token, "static" ) )
        bp->accMoveType = PMT_STATIC;
      else if( !Q_stricmp( token, "tag" ) )
        bp->accMoveType = PMT_TAG;
      else if( !Q_stricmp( token, "cent" ) )
        bp->accMoveType = PMT_CENT_ANGLES;
      else if( !Q_stricmp( token, "normal" ) )
        bp->accMoveType = PMT_NORMAL;

      continue;
    }
    else if( !Q_stricmp( token, "accelerationDir" ) )
    {
      token = COM_Parse( text_p );
      if( !token )
        break;
      
      if( !Q_stricmp( token, "linear" ) )
        bp->accMoveValues.dirType = PMD_LINEAR;
      else if( !Q_stricmp( token, "point" ) )
        bp->accMoveValues.dirType = PMD_POINT;

      continue;
    }
    else if( !Q_stricmp( token, "accelerationMagnitude" ) )
    {
      token = COM_Parse( text_p );
      if( !token )
        break;
      
      CG_ParseValueAndVariance( token, &number, &randFrac );

      bp->accMoveValues.mag = number;
      bp->accMoveValues.magRandFrac = randFrac;

      continue;
    }
    else if( !Q_stricmp( token, "acceleration" ) )
    {
      for( i = 0; i <= 2; i++ )
      {
        token = COM_Parse( text_p );
        if( !token )
          break;
        
        bp->accMoveValues.dir[ i ] = atof_neg( token, qtrue );
      }
      
      token = COM_Parse( text_p );
      if( !token )
        break;
      
      CG_ParseValueAndVariance( token, NULL, &randFrac );

      bp->accMoveValues.dirRandAngle = randFrac;
      
      continue;
    }
    else if( !Q_stricmp( token, "accelerationPoint" ) )
    {
      for( i = 0; i <= 2; i++ )
      {
        token = COM_Parse( text_p );
        if( !token )
          break;
        
        bp->accMoveValues.point[ i ] = atof_neg( token, qtrue );
      }
      
      token = COM_Parse( text_p );
      if( !token )
        break;
      
      CG_ParseValueAndVariance( token, NULL, &randFrac );

      bp->accMoveValues.pointRandAngle = randFrac;
      
      continue;
    }
    ///
    else if( !Q_stricmp( token, "displacement" ) )
    {
      for( i = 0; i <= 2; i++ )
      {
        token = COM_Parse( text_p );
        if( !token )
          break;
        
        bp->displacement[ i ] = atof_neg( token, qtrue );
      }
      
      token = COM_Parse( text_p );
      if( !token )
        break;

      CG_ParseValueAndVariance( token, NULL, &randFrac );

      bp->randDisplacement = randFrac;

      continue;
    }
    else if( !Q_stricmp( token, "normalDisplacement" ) )
    {
      token = COM_Parse( text_p );
      if( !token )
        break;
      
      bp->normalDisplacement = atof_neg( token, qtrue );

      continue;
    }
    else if( !Q_stricmp( token, "overdrawProtection" ) )
    {
      bp->overdrawProtection = qtrue;

      continue;
    }
    else if( !Q_stricmp( token, "realLight" ) )
    {
      bp->realLight = qtrue;

      continue;
    }
    else if( !Q_stricmp( token, "cullOnStartSolid" ) )
    {
      bp->cullOnStartSolid = qtrue;

      continue;
    }
    else if( !Q_stricmp( token, "radius" ) )
    {
      token = COM_Parse( text_p );
      if( !token )
        break;

      CG_ParseValueAndVariance( token, &number, &randFrac );

      bp->radius.delay = (int)number;
      bp->radius.delayRandFrac = randFrac;

      token = COM_Parse( text_p );
      if( !token )
        break;

      CG_ParseValueAndVariance( token, &number, &randFrac );

      bp->radius.initial = number;
      bp->radius.initialRandFrac = randFrac;

      token = COM_Parse( text_p );
      if( !token )
        break;

      if( !Q_stricmp( token, "-" ) )
      {
        bp->radius.final = PARTICLES_SAME_AS_INITIAL;
        bp->radius.finalRandFrac = 0.0f;
      }
      else
      {
        CG_ParseValueAndVariance( token, &number, &randFrac );

        bp->radius.final = number;
        bp->radius.finalRandFrac = randFrac;
      }

      continue;
    }
    else if( !Q_stricmp( token, "alpha" ) )
    {
      token = COM_Parse( text_p );
      if( !token )
        break;

      CG_ParseValueAndVariance( token, &number, &randFrac );

      bp->alpha.delay = (int)number;
      bp->alpha.delayRandFrac = randFrac;

      token = COM_Parse( text_p );
      if( !token )
        break;

      CG_ParseValueAndVariance( token, &number, &randFrac );

      bp->alpha.initial = number;
      bp->alpha.initialRandFrac = randFrac;

      token = COM_Parse( text_p );
      if( !token )
        break;

      if( !Q_stricmp( token, "-" ) )
      {
        bp->alpha.final = PARTICLES_SAME_AS_INITIAL;
        bp->alpha.finalRandFrac = 0.0f;
      }
      else
      {
        CG_ParseValueAndVariance( token, &number, &randFrac );

        bp->alpha.final = number;
        bp->alpha.finalRandFrac = randFrac;
      }

      continue;
    }
    else if( !Q_stricmp( token, "rotation" ) )
    {
      token = COM_Parse( text_p );
      if( !token )
        break;

      CG_ParseValueAndVariance( token, &number, &randFrac );

      bp->rotation.delay = (int)number;
      bp->rotation.delayRandFrac = randFrac;

      token = COM_Parse( text_p );
      if( !token )
        break;

      CG_ParseValueAndVariance( token, &number, &randFrac );

      bp->rotation.initial = number;
      bp->rotation.initialRandFrac = randFrac;

      token = COM_Parse( text_p );
      if( !token )
        break;

      if( !Q_stricmp( token, "-" ) )
      {
        bp->rotation.final = PARTICLES_SAME_AS_INITIAL;
        bp->rotation.finalRandFrac = 0.0f;
      }
      else
      {
        CG_ParseValueAndVariance( token, &number, &randFrac );

        bp->rotation.final = number;
        bp->rotation.finalRandFrac = randFrac;
      }

      continue;
    }
    else if( !Q_stricmp( token, "lifeTime" ) )
    {
      token = COM_Parse( text_p );
      if( !token )
        break;

      CG_ParseValueAndVariance( token, &number, &randFrac );

      bp->lifeTime = (int)number;
      bp->lifeTimeRandFrac = randFrac;

      continue;
    }
    else if( !Q_stricmp( token, "}" ) )
      return qtrue; //reached the end of this particle
    else
    {
      CG_Printf( S_COLOR_RED "ERROR: unknown token '%s' in particle\n", token );
      return qfalse;
    }
  }

  return qfalse;
}


/*
===============
CG_ParseParticleEjector

Parse a particle ejector section
===============
*/
static qboolean CG_ParseParticleEjector( baseParticleEjector_t *bpe, char **text_p )
{
  char  *token;
  float number, randFrac;

  // read optional parameters
  while( 1 )
  {
    token = COM_Parse( text_p );
    
    if( !token )
      break;

    if( !Q_stricmp( token, "" ) )
      return qfalse;

    if( !Q_stricmp( token, "{" ) )
    {
      if( !CG_ParseParticle( &baseParticles[ numBaseParticles ], text_p ) )
      {
        CG_Printf( S_COLOR_RED "ERROR: failed to parse particle\n" );
        return qfalse;
      }

      if( bpe->numParticles == MAX_PARTICLES_PER_EJECTOR )
      {
        CG_Printf( S_COLOR_RED "ERROR: ejector has > %d particles\n", MAX_PARTICLES_PER_EJECTOR );
        return qfalse;
      }
      else if( numBaseParticles == MAX_BASEPARTICLES )
      {
        CG_Printf( S_COLOR_RED "ERROR: maximum number of particles (%d) reached\n", MAX_BASEPARTICLES );
        return qfalse;
      }
      else
      {
        //start parsing particles again
        bpe->particles[ bpe->numParticles ] = &baseParticles[ numBaseParticles ];
        bpe->numParticles++;
        numBaseParticles++;
      }
      continue;
    }
    else if( !Q_stricmp( token, "delay" ) )
    {
      token = COM_Parse( text_p );
      if( !token )
        break;

      CG_ParseValueAndVariance( token, &number, &randFrac );

      bpe->eject.delay = (int)number;
      bpe->eject.delayRandFrac = randFrac;

      continue;
    }
    else if( !Q_stricmp( token, "period" ) )
    {
      token = COM_Parse( text_p );
      if( !token )
        break;

      bpe->eject.initial = atoi_neg( token, qfalse );
      
      token = COM_Parse( text_p );
      if( !token )
        break;

      if( !Q_stricmp( token, "-" ) )
        bpe->eject.final = PARTICLES_SAME_AS_INITIAL;
      else
        bpe->eject.final = atoi_neg( token, qfalse );
      
      token = COM_Parse( text_p );
      if( !token )
        break;

      CG_ParseValueAndVariance( token, NULL, &bpe->eject.randFrac );

      continue;
    }
    else if( !Q_stricmp( token, "count" ) )
    {
      token = COM_Parse( text_p );
      if( !token )
        break;

      if( !Q_stricmp( token, "infinite" ) )
      {
        bpe->totalParticles = PARTICLES_INFINITE;
        bpe->totalParticlesRandFrac = 0.0f;
      }
      else
      {
        CG_ParseValueAndVariance( token, &number, &randFrac );

        bpe->totalParticles = (int)number;
        bpe->totalParticlesRandFrac = randFrac;
      }

      continue;
    }
    else if( !Q_stricmp( token, "particle" ) ) //acceptable text
      continue;
    else if( !Q_stricmp( token, "}" ) )
      return qtrue; //reached the end of this particle ejector
    else
    {
      CG_Printf( S_COLOR_RED "ERROR: unknown token '%s' in particle ejector\n", token );
      return qfalse;
    }
  }

  return qfalse;
}


/*
===============
CG_ParseParticleSystem

Parse a particle system section
===============
*/
static qboolean CG_ParseParticleSystem( baseParticleSystem_t *bps, char **text_p )
{
  char                  *token;
  int                   i;
  baseParticleEjector_t *bpe;

  // read optional parameters
  while( 1 )
  {
    token = COM_Parse( text_p );
    
    if( !token )
      break;

    if( !Q_stricmp( token, "" ) )
      return qfalse;

    if( !Q_stricmp( token, "{" ) )
    {
      if( !CG_ParseParticleEjector( &baseParticleEjectors[ numBaseParticleEjectors ], text_p ) )
      {
        CG_Printf( S_COLOR_RED "ERROR: failed to parse particle ejector\n" );
        return qfalse;
      }

      bpe = &baseParticleEjectors[ numBaseParticleEjectors ];
      
      //check for infinite count + zero period
      if( bpe->totalParticles == PARTICLES_INFINITE &&
          ( bpe->eject.initial == 0.0f || bpe->eject.final == 0.0f ) )
      {
        CG_Printf( S_COLOR_RED "ERROR: ejector with 'count infinite' potentially has zero period\n" );
        return qfalse;
      }

      if( bps->numEjectors == MAX_EJECTORS_PER_SYSTEM )
      {
        CG_Printf( S_COLOR_RED "ERROR: particle system has > %d ejectors\n", MAX_EJECTORS_PER_SYSTEM );
        return qfalse;
      }
      else if( numBaseParticleEjectors == MAX_BASEPARTICLE_EJECTORS )
      {
        CG_Printf( S_COLOR_RED "ERROR: maximum number of particle ejectors (%d) reached\n", MAX_BASEPARTICLE_EJECTORS );
        return qfalse;
      }
      else
      {
        //start parsing ejectors again
        bps->ejectors[ bps->numEjectors ] = &baseParticleEjectors[ numBaseParticleEjectors ];
        bps->numEjectors++;
        numBaseParticleEjectors++;
      }
      continue;
    }
    else if( !Q_stricmp( token, "ejector" ) ) //acceptable text
      continue;
    else if( !Q_stricmp( token, "}" ) )
      return qtrue; //reached the end of this particle system
    else
    {
      CG_Printf( S_COLOR_RED "ERROR: unknown token '%s' in particle system %s\n", token, bps->name );
      return qfalse;
    }
  }

  return qfalse;
}

/*
===============
CG_ParseParticleFile

Load the particle systems from a particle file
===============
*/
static qboolean CG_ParseParticleFile( const char *fileName )
{
  char          *text_p;
  int           i;
  int           len;
  char          *token;
  char          text[ 20000 ];
  char          psName[ MAX_QPATH ];
  qboolean      psNameSet = qfalse;
  fileHandle_t  f;

  // load the file
  len = trap_FS_FOpenFile( fileName, &f, FS_READ );
  if( len <= 0 )
    return qfalse;

  if( len >= sizeof( text ) - 1 )
  {
    CG_Printf( S_COLOR_RED "ERROR: particle file %s too long\n", fileName );
    return qfalse;
  }
  
  trap_FS_Read( text, len, f );
  text[ len ] = 0;
  trap_FS_FCloseFile( f );

  // parse the text
  text_p = text;

  // read optional parameters
  while( 1 )
  {
    token = COM_Parse( &text_p );
    
    if( !token )
      break;

    if( !Q_stricmp( token, "" ) )
      return qfalse;

    if( !Q_stricmp( token, "{" ) )
    {
      if( psNameSet )
      {
        //check for name space clashes
        for( i = 0; i < numBaseParticleSystems; i++ )
        {
          if( !Q_stricmp( baseParticleSystems[ i ].name, psName ) )
          {
            CG_Printf( S_COLOR_RED "ERROR: a particle system is already named %s\n", psName );
            return qfalse;
          }
        }
        
        Q_strncpyz( baseParticleSystems[ numBaseParticleSystems ].name, psName, MAX_QPATH );
        
        if( !CG_ParseParticleSystem( &baseParticleSystems[ numBaseParticleSystems ], &text_p ) )
        {
          CG_Printf( S_COLOR_RED "ERROR: %s: failed to parse particle system %s\n", fileName, psName );
          return qfalse;
        }

        //start parsing particle systems again
        psNameSet = qfalse;
      
        if( numBaseParticleSystems == MAX_BASEPARTICLE_SYSTEMS )
        {
          CG_Printf( S_COLOR_RED "ERROR: maximum number of particle systems (%d) reached\n", MAX_BASEPARTICLE_EJECTORS );
          return qfalse;
        }
        else
          numBaseParticleSystems++;
        
        continue;
      }
      else
      {
        CG_Printf( S_COLOR_RED "ERROR: unamed particle system\n" );
        return qfalse;
      }
    }

    if( !psNameSet )
    {
      Q_strncpyz( psName, token, sizeof( psName ) );
      psNameSet = qtrue;
    }
    else
    {
      CG_Printf( S_COLOR_RED "ERROR: particle system already named\n" );
      return qfalse;
    }
  }

  return qtrue;
}


/*
===============
CG_LoadParticleSystems

Load particle systems from .particle files
===============
*/
void CG_LoadParticleSystems( void )
{
  int         i;
  const char  *s[ MAX_PARTICLE_FILES ];

  numBaseParticleSystems = 0;
  numBaseParticleEjectors = 0;
  numBaseParticles = 0;

  for( i = 0; i < MAX_PARTICLE_FILES; i++ )
  {
    s[ i ] = CG_ConfigString( CS_PARTICLE_FILES + i );
  
    if( strlen( s[ i ] ) > 0 )
    {
      CG_Printf( "...loading '%s'\n", s[ i ] );
      CG_ParseParticleFile( s[ i ] );
    }
    else
      break;
  }
}


/*
===============
CG_AttachParticleSystemToCent

Attach a particle system to a centity_t
===============
*/
void CG_AttachParticleSystemToCent( particleSystem_t *ps )
{
  if( ps == NULL )
  {
    CG_Printf( S_COLOR_YELLOW "WARNING: tried to modify a NULL particle system\n" );
    return;
  }
  
  ps->attachType = PSA_CENT_ORIGIN;
  ps->attached = qtrue;
}

/*
===============
CG_SetParticleSystemCent

Set a particle system attachment means
===============
*/
void CG_SetParticleSystemCent( particleSystem_t *ps, centity_t *cent )
{
  if( ps == NULL )
  {
    CG_Printf( S_COLOR_YELLOW "WARNING: tried to modify a NULL particle system\n" );
    return;
  }
  
  ps->attachment.centValid = qtrue;
  ps->attachment.centNum = cent->currentState.number;
}

/*
===============
CG_AttachParticleSystemToTag

Attach a particle system to a model tag
===============
*/
void CG_AttachParticleSystemToTag( particleSystem_t *ps )
{
  if( ps == NULL )
  {
    CG_Printf( S_COLOR_YELLOW "WARNING: tried to modify a NULL particle system\n" );
    return;
  }
  
  ps->attachType = PSA_TAG;
  ps->attached = qtrue;
}

/*
===============
CG_SetParticleSystemToTag

Set a particle system attachment means
===============
*/
void CG_SetParticleSystemTag( particleSystem_t *ps, refEntity_t parent,
                              qhandle_t model, char *tagName )
{
  if( ps == NULL )
  {
    CG_Printf( S_COLOR_YELLOW "WARNING: tried to modify a NULL particle system\n" );
    return;
  }
  
  ps->attachment.tagValid = qtrue;
  ps->attachment.parent = parent;
  ps->attachment.model = model;
  strncpy( ps->attachment.tagName, tagName, MAX_STRING_CHARS );
}

/*
===============
CG_AttachParticleSystemToOrigin

Attach a particle system to a point in space
===============
*/
void CG_AttachParticleSystemToOrigin( particleSystem_t *ps )
{
  if( ps == NULL )
  {
    CG_Printf( S_COLOR_YELLOW "WARNING: tried to modify a NULL particle system\n" );
    return;
  }
  
  ps->attachType = PSA_STATIC;
  ps->attached = qtrue;
}

/*
===============
CG_SetParticleSystemOrigin

Set a particle system attachment means
===============
*/
void CG_SetParticleSystemOrigin( particleSystem_t *ps, vec3_t origin )
{
  if( ps == NULL )
  {
    CG_Printf( S_COLOR_YELLOW "WARNING: tried to modify a NULL particle system\n" );
    return;
  }
  
  ps->attachment.staticValid = qtrue;
  VectorCopy( origin, ps->attachment.origin );
}

/*
===============
CG_SetParticleSystemNormal

Set a particle system attachment means
===============
*/
void CG_SetParticleSystemNormal( particleSystem_t *ps, vec3_t normal )
{
  if( ps == NULL )
  {
    CG_Printf( S_COLOR_YELLOW "WARNING: tried to modify a NULL particle system\n" );
    return;
  }
  
  ps->attachment.normalValid = qtrue;
  VectorCopy( normal, ps->attachment.normal );
}


/*
===============
CG_DestroyParticleSystem

Destroy a particle system

This doesn't actually invalidate anything, it just stops
particle ejectors from producing new particles so the 
garbage collector will eventually remove this system
===============
*/
void CG_DestroyParticleSystem( particleSystem_t *ps )
{
  int               i;
  particleEjector_t *pe;
  
  if( ps == NULL )
  {
    CG_Printf( S_COLOR_YELLOW "WARNING: tried to destroy a NULL particle system\n" );
    return;
  }
  
  if( cg_debugParticles.integer >= 1 )
    CG_Printf( "PS destroyed\n" );

  for( i = 0; i < MAX_PARTICLE_EJECTORS; i++ )
  {
    pe = &particleEjectors[ i ];
    
    if( pe->valid && pe->parent == ps )
      pe->totalParticles = pe->count = 0;
  }
}

/*
===============
CG_IsParticleSystemInfinite

Test a particle system for 'count infinite' ejectors
===============
*/
qboolean CG_IsParticleSystemInfinite( particleSystem_t *ps )
{
  int               i;
  particleEjector_t *pe;

  if( ps == NULL )
  {
    CG_Printf( S_COLOR_YELLOW "WARNING: tried to test a NULL particle system\n" );
    return qfalse;
  }
  
  //don't bother checking already invalid systems
  if( !ps->valid )
    return qfalse;
  
  for( i = 0; i < MAX_PARTICLE_EJECTORS; i++ )
  {
    pe = &particleEjectors[ i ];
    
    if( pe->valid && pe->parent == ps )
    {
      if( pe->totalParticles == PARTICLES_INFINITE )
        return qtrue;
    }
  }

  return qfalse;
}


/*
===============
CG_GarbageCollectParticleSystems

Destroy inactive particle systems
===============
*/
static void CG_GarbageCollectParticleSystems( void )
{
  int               i, j, count;
  particleSystem_t  *ps;
  particleEjector_t *pe;

  for( i = 0; i < MAX_PARTICLE_SYSTEMS; i++ )
  {
    ps = &particleSystems[ i ];
    count = 0;
    
    //don't bother checking already invalid systems
    if( !ps->valid )
      continue;
    
    for( j = 0; j < MAX_PARTICLE_EJECTORS; j++ )
    {
      pe = &particleEjectors[ j ];
      
      if( pe->valid && pe->parent == ps )
        count++;
    }

    if( !count )
      ps->valid = qfalse;

    //check systems where the parent cent has left the PVS
    //( centNum 0 - player entity, is always valid )
    if( ps->attachType == PSA_CENT_ORIGIN && ps->attachment.centNum != 0 )
    {
      if( !cg_entities[ ps->attachment.centNum ].valid )
        ps->valid = qfalse;
    }

    if( cg_debugParticles.integer >= 1 && !ps->valid )
      CG_Printf( "PS garbage collected\n" );
  }
}


/*
===============
CG_CalculateTimeFrac

Calculate the fraction of time passed
===============
*/
static float CG_CalculateTimeFrac( int birth, int life, int delay )
{
  float frac;

  frac = ( (float)cg.time - (float)( birth + delay ) ) / (float)( life - delay );

  if( frac < 0.0f )
    frac = 0.0f;
  else if( frac > 1.0f )
    frac = 1.0f;

  return frac;
}

/*
===============
CG_EvaluateParticlePhysics

Compute the physics on a specific particle
===============
*/
static void CG_EvaluateParticlePhysics( particle_t *p )
{
  particleSystem_t  *ps = p->parent->parent;
  baseParticle_t    *bp = p->class;
  vec3_t            acceleration, forward, newOrigin;
  vec3_t            mins, maxs;
  float             deltaTime, bounce, radius, dot;
  trace_t           trace;
  centity_t         *cent;
  
  switch( bp->accMoveType )
  {
    case PMT_STATIC:
      
      if( bp->accMoveValues.dirType == PMD_POINT )
        VectorSubtract( bp->accMoveValues.point, p->origin, acceleration );
      else if( bp->accMoveValues.dirType == PMD_LINEAR )
        VectorCopy( bp->accMoveValues.dir, acceleration );
      
      break;

    case PMT_TAG:
      
      if( !ps->attachment.tagValid )
        return;
      
      if( bp->accMoveValues.dirType == PMD_POINT )
        VectorSubtract( ps->attachment.re.origin, p->origin, acceleration );
      else if( bp->accMoveValues.dirType == PMD_LINEAR )
        VectorCopy( ps->attachment.re.axis[ 0 ], acceleration );
      
      break;

    case PMT_CENT_ANGLES:
      
      if( !ps->attachment.centValid )
        return;
      
      cent = &cg_entities[ ps->attachment.centNum ];
      
      if( bp->accMoveValues.dirType == PMD_POINT )
        VectorSubtract( cent->lerpOrigin, p->origin, acceleration );
      else if( bp->accMoveValues.dirType == PMD_LINEAR )
      {
        AngleVectors( cent->lerpAngles, forward, NULL, NULL );
        VectorCopy( forward, acceleration );
      }
      
      break;
    
    case PMT_NORMAL:
      
      if( !ps->attachment.normalValid )
        return;
      
      VectorCopy( ps->attachment.normal, acceleration );
      
      break;
  }

#define MAX_ACC_RADIUS 1000.0f
  
  if( bp->accMoveValues.dirType == PMD_POINT )
  {
    //FIXME: so this fall off is a bit... odd -- it works..
    float r2 = DotProduct( acceleration, acceleration ); // = radius^2
    float scale = ( MAX_ACC_RADIUS - r2 ) / MAX_ACC_RADIUS;

    if( scale > 1.0f )
      scale = 1.0f;
    else if( scale < 0.1f )
      scale = 0.1f;
    
    scale *= CG_RandomiseValue( bp->accMoveValues.mag, bp->accMoveValues.magRandFrac );
    
    VectorNormalize( acceleration );
    CG_SpreadVector( acceleration, bp->accMoveValues.dirRandAngle );
    VectorScale( acceleration, scale, acceleration );
  }
  else if( bp->accMoveValues.dirType == PMD_LINEAR )
  {
    VectorNormalize( acceleration );
    CG_SpreadVector( acceleration, bp->accMoveValues.dirRandAngle );
    VectorScale( acceleration,
                 CG_RandomiseValue( bp->accMoveValues.mag, bp->accMoveValues.magRandFrac ),
                 acceleration );
  }
  
  radius = CG_LerpValues( p->radius.initial,
                 p->radius.final,
                 CG_CalculateTimeFrac( p->birthTime,
                                       p->lifeTime,
                                       p->radius.delay ) );
  
  VectorSet( mins, -radius, -radius, -radius );
  VectorSet( maxs, radius, radius, radius );

  bounce = CG_RandomiseValue( bp->bounceFrac, bp->bounceFracRandFrac );

  deltaTime = (float)( cg.time - p->lastEvalTime ) * 0.001;
  VectorMA( p->velocity, deltaTime, acceleration, p->velocity );
  VectorMA( p->origin, deltaTime, p->velocity, newOrigin );
  p->lastEvalTime = cg.time;
  
  CG_Trace( &trace, p->origin, mins, maxs, newOrigin, -1, CONTENTS_SOLID );
  
  //not hit anything or not a collider
  if( trace.fraction == 1.0f || bounce == 0.0f )
  {
    VectorCopy( newOrigin, p->origin );
    return;
  }
  
  //remove particles that get into a CONTENTS_NODROP brush
  if( ( trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_NODROP ) ||
      ( bp->cullOnStartSolid && trace.startsolid ) )
  {
    if( cg_debugParticles.integer >= 1 )
      CG_Printf( "Particle in CONTENTS_NODROP or trace.startsolid\n" );

    p->valid = qfalse;
    return;
  }

  //reflect the velocity on the trace plane
  dot = DotProduct( p->velocity, trace.plane.normal );
  VectorMA( p->velocity, -2.0f * dot, trace.plane.normal, p->velocity );

  VectorScale( p->velocity, bounce, p->velocity );

  VectorCopy( trace.endpos, p->origin );
}


#define GETKEY(x,y) (((x)>>y)&0xFF)

/*
===============
CG_Radix
===============
*/
static void CG_Radix( int bits, int size, particle_t *source, particle_t *dest )
{
  int count[ 256 ];
  int index[ 256 ];
  int i;
  
  memset( count, 0, sizeof( count ) );
  
  for( i = 0; i < size; i++ )
    count[ GETKEY( source[ i ].sortKey, bits ) ]++;

  index[ 0 ] = 0;
  
  for( i = 1; i < 256; i++ )
    index[ i ] = index[ i - 1 ] + count[ i - 1 ];
  
  for( i = 0; i < size; i++ )
    dest[ index[ GETKEY( source[ i ].sortKey, bits ) ]++ ] = source[ i ];
}

/*
===============
CG_RadixSort

Radix sort with 4 byte size buckets
===============
*/
static void CG_RadixSort( particle_t *source, particle_t *temp, int size )
{
  CG_Radix( 0,   size, source, temp );
  CG_Radix( 8,   size, temp, source );
  CG_Radix( 16,  size, source, temp );
  CG_Radix( 24,  size, temp, source );
}

/*
===============
CG_CompactAndSortParticles

Depth sort the particles
===============
*/
static void CG_CompactAndSortParticles( void )
{
  int     i, j = 0;
  int     numParticles;
  vec3_t  delta;

  for( i = MAX_PARTICLES - 1; i >= 0; i-- )
  {
    if( particles[ i ].valid )
    {
      while( particles[ j ].valid )
        j++;

      //no more holes
      if( j >= i )
        break;

      particles[ j ] = particles[ i ];
      memset( &particles[ i ], 0, sizeof( particles[ 0 ] ) );
    }
  }

  numParticles = i;
  
  //set sort keys
  for( i = 0; i < numParticles; i++ )
  {
    VectorSubtract( particles[ i ].origin, cg.refdef.vieworg, delta );
    particles[ i ].sortKey = DotProduct( delta, delta );
  }
  
  CG_RadixSort( particles, sortParticles, numParticles );
}

/*
===============
CG_RenderParticle

Actually render a particle
===============
*/
static void CG_RenderParticle( particle_t *p )
{
  refEntity_t     re;
  float           timeFrac, frac;
  int             index;
  baseParticle_t  *bp = p->class;
  vec3_t          alight, dlight, lightdir;
  int             i;
    
  memset( &re, 0, sizeof( refEntity_t ) );

  for( i = 0; i <= 3; re.shaderRGBA[ i++ ] = 0xFF );

  timeFrac = CG_CalculateTimeFrac( p->birthTime, p->lifeTime, 0 );

  re.reType = RT_SPRITE;
  re.shaderTime = p->birthTime / 1000.0f; //FIXME: allow user to change?

  re.shaderRGBA[ 3 ] = (byte)( (float)0xFF * 
                       CG_LerpValues( p->alpha.initial,
                             p->alpha.final,
                             CG_CalculateTimeFrac( p->birthTime,
                                                   p->lifeTime,
                                                   p->alpha.delay ) ) );
  
  re.radius = CG_LerpValues( p->radius.initial,
                    p->radius.final,
                    CG_CalculateTimeFrac( p->birthTime,
                                          p->lifeTime,
                                          p->radius.delay ) );
  
  re.rotation = CG_LerpValues( p->rotation.initial,
                      p->rotation.final,
                      CG_CalculateTimeFrac( p->birthTime,
                                            p->lifeTime,
                                            p->rotation.delay ) );

  // if the view would be "inside" the sprite, kill the sprite
  // so it doesn't add too much overdraw
  if( Distance( p->origin, cg.refdef.vieworg ) < re.radius && bp->overdrawProtection )
    return;

  //apply environmental lighting to the particle
  if( bp->realLight )
  {
    trap_R_LightForPoint( p->origin, alight, dlight, lightdir );
    for( i = 0; i <= 2; i++ )
      re.shaderRGBA[ i ] = (int)alight[ i ];
  }

  if( bp->framerate == 0.0f )
  {
    //sync animation time to lifeTime of particle
    index = (int)( timeFrac * ( bp->numFrames + 1 ) );

    if( index >= bp->numFrames )
      index = bp->numFrames - 1;

    re.customShader = bp->shaders[ index ];
  }
  else
  {
    //looping animation
    index = (int)( bp->framerate * timeFrac * p->lifeTime * 0.001 ) % bp->numFrames;
    re.customShader = bp->shaders[ index ];
  }

  VectorCopy( p->origin, re.origin );

  trap_R_AddRefEntityToScene( &re );
}

/*
===============
CG_AddParticles

Add particles to the scene
===============
*/
void CG_AddParticles( void )
{
  int           i;
  particle_t    *p;
  int           numPS = 0, numPE = 0, numP = 0;
  
  //remove expired particle systems
  CG_GarbageCollectParticleSystems( );
  
  //check each ejector and introduce any new particles
  CG_SpawnNewParticles( );

  //sorting
  if( cg_depthSortParticles.integer )
    CG_CompactAndSortParticles( );

  for( i = 0; i < MAX_PARTICLES; i++ )
  {
    p = &particles[ i ];
    
    if( p->valid )
    {
      if( p->birthTime + p->lifeTime > cg.time )
      {
        //particle is active
        CG_EvaluateParticlePhysics( p );
        CG_RenderParticle( p );
      }
      else
        p->valid = qfalse;
    }
  }

  if( cg_debugParticles.integer >= 2 )
  {
    for( i = 0; i < MAX_PARTICLE_SYSTEMS; i++ )
      if( particleSystems[ i ].valid )
        numPS++;
    
    for( i = 0; i < MAX_PARTICLE_EJECTORS; i++ )
      if( particleEjectors[ i ].valid )
        numPE++;
    
    for( i = 0; i < MAX_PARTICLES; i++ )
      if( particles[ i ].valid )
        numP++;

    CG_Printf( "PS: %d  PE: %d  P: %d\n", numPS, numPE, numP );
  }
}
