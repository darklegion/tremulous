// Copyright (C) 1999-2000 Id Software, Inc.
//

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

#include "g_local.h"

#define	MISSILE_PRESTEP_TIME	50

/*
================
G_BounceMissile

================
*/
void G_BounceMissile( gentity_t *ent, trace_t *trace ) {
	vec3_t	velocity;
	float	dot;
	int		hitTime;

	// reflect the velocity on the trace plane
	hitTime = level.previousTime + ( level.time - level.previousTime ) * trace->fraction;
	BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->plane.normal );
	VectorMA( velocity, -2*dot, trace->plane.normal, ent->s.pos.trDelta );

	if ( ent->s.eFlags & EF_BOUNCE_HALF ) {
		VectorScale( ent->s.pos.trDelta, 0.65, ent->s.pos.trDelta );
		// check for stop
		if ( trace->plane.normal[2] > 0.2 && VectorLength( ent->s.pos.trDelta ) < 40 ) {
			G_SetOrigin( ent, trace->endpos );
			return;
		}
	}

	VectorAdd( ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trTime = level.time;
}


/*
================
G_ExplodeMissile

Explode a missile without an impact
================
*/
void G_ExplodeMissile( gentity_t *ent ) {
  vec3_t    dir;
  vec3_t    origin;

  BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
  SnapVector( origin );
  G_SetOrigin( ent, origin );

  // we don't have a valid direction, so just point straight up
  dir[0] = dir[1] = 0;
  dir[2] = 1;

  ent->s.eType = ET_GENERAL;

  //TA: tired... can't be fucked... hack
  if( ent->s.weapon != WP_LOCKBLOB_LAUNCHER && ent->s.weapon != WP_FLAMER )
    G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( dir ) );

  ent->freeAfterEvent = qtrue;

/*  // splash damage
  if ( ent->splashDamage ) {
    if( G_RadiusDamage( ent->r.currentOrigin, ent->parent, ent->splashDamage, ent->splashRadius, ent
      , ent->splashMethodOfDeath ) ) {
      g_entities[ent->r.ownerNum].client->accuracy_hits++;
    }
  }*/

  trap_LinkEntity( ent );
}


/*
================
G_MissileImpact

================
*/
void G_MissileImpact( gentity_t *ent, trace_t *trace ) {
	gentity_t		*other;
	qboolean		hitClient = qfalse;

	other = &g_entities[trace->entityNum];

	// check for bounce
	if ( !other->takedamage &&
		( ent->s.eFlags & ( EF_BOUNCE | EF_BOUNCE_HALF ) ) ) {
		G_BounceMissile( ent, trace );
		G_AddEvent( ent, EV_GRENADE_BOUNCE, 0 );
		return;
	}

	// impact damage
	if (other->takedamage) {
		// FIXME: wrong damage direction?
		if ( ent->damage ) {
			vec3_t	velocity;

			if( LogAccuracyHit( other, &g_entities[ent->r.ownerNum] ) ) {
        g_entities[ent->r.ownerNum].client->accuracy_hits++;
				hitClient = qtrue;
			}
			BG_EvaluateTrajectoryDelta( &ent->s.pos, level.time, velocity );
			if ( VectorLength( velocity ) == 0 ) {
				velocity[2] = 1;	// stepped on a grenade
			}
			G_Damage (other, ent, &g_entities[ent->r.ownerNum], velocity,
				ent->s.origin, ent->damage, 
				0, ent->methodOfDeath);
		}
	}

	if( !strcmp( ent->classname, "lockblob" ) )
  {
    if( other->client && other->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
    {
      other->client->ps.stats[ STAT_STATE ] |= SS_BLOBLOCKED;
      other->client->lastLockTime = level.time;
      VectorCopy( other->client->ps.viewangles, other->client->ps.grapplePoint );
    }
  }
  
	// is it cheaper in bandwidth to just remove this ent and create a new
	// one, rather than changing the missile into the explosion?

	if ( other->takedamage && other->client ) {
		G_AddEvent( ent, EV_MISSILE_HIT, DirToByte( trace->plane.normal ) );
		ent->s.otherEntityNum = other->s.number;
  } else if( trace->surfaceFlags & SURF_METALSTEPS ) {
    G_AddEvent( ent, EV_MISSILE_MISS_METAL, DirToByte( trace->plane.normal ) );
	} else {
		G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( trace->plane.normal ) );
	}

	ent->freeAfterEvent = qtrue;

	// change over to a normal entity right at the point of impact
	ent->s.eType = ET_GENERAL;

	SnapVectorTowards( trace->endpos, ent->s.pos.trBase );	// save net bandwidth

	G_SetOrigin( ent, trace->endpos );

	// splash damage (doesn't apply to person directly hit)
	if ( ent->splashDamage ) {
		if( G_RadiusDamage( trace->endpos, ent->parent, ent->splashDamage, ent->splashRadius, 
			other, ent->splashMethodOfDeath ) ) {
			if( !hitClient ) {
        g_entities[ent->r.ownerNum].client->accuracy_hits++;
			}
		}
	}

	trap_LinkEntity( ent );
}


/*
================
G_RunMissile

================
*/
void G_RunMissile( gentity_t *ent )
{
	vec3_t		origin;
	trace_t		tr;
  int     passent;

	// get current position
	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );

  // if this missile bounced off an invulnerability sphere
  if ( ent->target_ent )
  {
    passent = ent->target_ent->s.number;
  }
  else
  {
    // ignore interactions with the missile owner
    passent = ent->r.ownerNum;
  }
  // trace a line from the previous position to the current position
  trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, ent->clipmask );

  if( tr.startsolid || tr.allsolid )
  {
    // make sure the tr.entityNum is set to the entity we're stuck in
    trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, passent, ent->clipmask );
		tr.fraction = 0;
	}
  else
    VectorCopy( tr.endpos, ent->r.currentOrigin );

	trap_LinkEntity( ent );

	if( tr.fraction != 1 )
  {
		// never explode or bounce on sky
		if ( tr.surfaceFlags & SURF_NOIMPACT )
    {
			// If grapple, reset owner
      if (ent->parent && ent->parent->client && ent->parent->client->hook == ent)
				ent->parent->client->hook = NULL;
        
			G_FreeEntity( ent );
			return;
		}

		G_MissileImpact( ent, &tr );
		if ( ent->s.eType != ET_MISSILE )
			return;		// exploded
	}

	// check think function after bouncing
	G_RunThink( ent );
}


//=============================================================================

/*
=================
fire_flamer

=================
*/
gentity_t *fire_flamer( gentity_t *self, vec3_t start, vec3_t dir )
{
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "flame";
	bolt->nextthink = level.time + FIREBALL_LIFETIME;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_FLAMER;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 60;
	bolt->splashDamage = 65;
	bolt->splashRadius = 45;
	bolt->methodOfDeath = MOD_FLAMER;
	bolt->splashMethodOfDeath = MOD_FLAMER_SPLASH;
	bolt->clipmask = MASK_SHOT;
  bolt->target_ent = NULL;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	//VectorMA( self->client->ps.velocity, 300, dir, bolt->s.pos.trDelta );
	VectorScale( dir, 200, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}	

//=============================================================================

/*
=================
fire_plasma

=================
*/
gentity_t *fire_plasma( gentity_t *self, vec3_t start, vec3_t dir )
{
  gentity_t *bolt;

  VectorNormalize (dir);

  bolt = G_Spawn();
  bolt->classname = "plasma";
  bolt->nextthink = level.time + 10000;
  bolt->think = G_ExplodeMissile;
  bolt->s.eType = ET_MISSILE;
  bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
  bolt->s.weapon = WP_PLASMAGUN;
  bolt->r.ownerNum = self->s.number;
  bolt->parent = self;
  bolt->damage = 20;
  bolt->splashDamage = 15;
  bolt->splashRadius = 20;
	//bolt->methodOfDeath = MOD_FLAMER;
	//bolt->splashMethodOfDeath = MOD_FLAMER_SPLASH;
  bolt->clipmask = MASK_SHOT;
  bolt->target_ent = NULL;

  bolt->s.pos.trType = TR_LINEAR;
  bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;   // move a bit on the very first frame
  VectorCopy( start, bolt->s.pos.trBase );
  VectorScale( dir, 2000, bolt->s.pos.trDelta );
  SnapVector( bolt->s.pos.trDelta );      // save net bandwidth

  VectorCopy (start, bolt->r.currentOrigin);

  return bolt;
}

/*
=================
fire_pulseRifle

=================
*/
gentity_t *fire_pulseRifle( gentity_t *self, vec3_t start, vec3_t dir )
{
  gentity_t *bolt;

  VectorNormalize (dir);

  bolt = G_Spawn();
  bolt->classname = "pulse";
  bolt->nextthink = level.time + 10000;
  bolt->think = G_ExplodeMissile;
  bolt->s.eType = ET_MISSILE;
  bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
  bolt->s.weapon = WP_PULSE_RIFLE;
  bolt->r.ownerNum = self->s.number;
  bolt->parent = self;
  bolt->damage = 20;
  bolt->splashDamage = 0;
  bolt->splashRadius = 0;
	//bolt->methodOfDeath = MOD_FLAMER;
	//bolt->splashMethodOfDeath = MOD_FLAMER_SPLASH;
  bolt->clipmask = MASK_SHOT;
  bolt->target_ent = NULL;

  bolt->s.pos.trType = TR_LINEAR;
  bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;   // move a bit on the very first frame
  VectorCopy( start, bolt->s.pos.trBase );
  VectorScale( dir, 1500, bolt->s.pos.trDelta );
  SnapVector( bolt->s.pos.trDelta );      // save net bandwidth

  VectorCopy (start, bolt->r.currentOrigin);

  return bolt;
}

//=============================================================================

/*
=================
fire_lockblob
=================
*/
gentity_t *fire_lockblob( gentity_t *self, vec3_t start, vec3_t dir )
{
	gentity_t	*bolt;

	VectorNormalize ( dir );

	bolt = G_Spawn( );
	bolt->classname = "lockblob";
	bolt->nextthink = level.time + 15000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_LOCKBLOB_LAUNCHER;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 0;
	bolt->splashDamage = 0;
	bolt->splashRadius = 0;
	bolt->methodOfDeath = MOD_ROCKET;
	bolt->splashMethodOfDeath = MOD_ROCKET_SPLASH;
	bolt->clipmask = MASK_SHOT;
  bolt->target_ent = NULL;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 500, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}

