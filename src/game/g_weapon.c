// Copyright (C) 1999-2000 Id Software, Inc.
//
// g_weapon.c
// perform the server side effects of a weapon firing

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

static  vec3_t  forward, right, up;
static  vec3_t  muzzle;

#define NUM_NAILSHOTS 15

/*
================
G_BounceProjectile
================
*/
void G_BounceProjectile( vec3_t start, vec3_t impact, vec3_t dir, vec3_t endout ) {
  vec3_t v, newv;
  float dot;

  VectorSubtract( impact, start, v );
  dot = DotProduct( v, dir );
  VectorMA( v, -2*dot, dir, newv );

  VectorNormalize(newv);
  VectorMA(impact, 8192, newv, endout);
}

/*
======================================================================

MACHINEGUN

======================================================================
*/

/*
======================
SnapVectorTowards

Round a vector to integers for more efficient network
transmission, but make sure that it rounds towards a given point
rather than blindly truncating.  This prevents it from truncating
into a wall.
======================
*/
void SnapVectorTowards( vec3_t v, vec3_t to ) {
  int   i;

  for ( i = 0 ; i < 3 ; i++ ) {
    if ( to[i] <= v[i] ) {
      v[i] = (int)v[i];
    } else {
      v[i] = (int)v[i] + 1;
    }
  }
}

#define MACHINEGUN_SPREAD 200
#define MACHINEGUN_DAMAGE 7
#define MACHINEGUN_TEAM_DAMAGE  5   // wimpier MG in teamplay

#define CHAINGUN_SPREAD 1200
#define CHAINGUN_DAMAGE 14

void Bullet_Fire( gentity_t *ent, float spread, int damage, int mod )
{
  trace_t   tr;
  vec3_t    end;
  float   r;
  float   u;
  gentity_t *tent;
  gentity_t *traceEnt;

  r = random() * M_PI * 2.0f;
  u = sin(r) * crandom() * spread * 16;
  r = cos(r) * crandom() * spread * 16;
  VectorMA (muzzle, 8192*16, forward, end);
  VectorMA (end, r, right, end);
  VectorMA (end, u, up, end);

  trap_Trace (&tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT);
  if( tr.surfaceFlags & SURF_NOIMPACT )
    return;

  traceEnt = &g_entities[ tr.entityNum ];

  // snap the endpos to integers, but nudged towards the line
  SnapVectorTowards( tr.endpos, muzzle );

  // send bullet impact
  if( traceEnt->takedamage && traceEnt->client )
  {
    tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_FLESH );
    tent->s.eventParm = traceEnt->s.number;
    if( LogAccuracyHit( traceEnt, ent ) )
      ent->client->accuracy_hits++;
  }
  else
  {
    tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_WALL );
    tent->s.eventParm = DirToByte( tr.plane.normal );
  }
  tent->s.otherEntityNum = ent->s.number;

  if( traceEnt->takedamage )
  {
    G_Damage( traceEnt, ent, ent, forward, tr.endpos,
      damage, 0, MOD_MACHINEGUN);
  }
}

/*
======================================================================

MASS DRIVER

======================================================================
*/

void massDriverFire( gentity_t *ent )
{
  trace_t   tr;
  vec3_t    end;
  gentity_t *tent;
  gentity_t *traceEnt;

  VectorMA( muzzle, 8192*16, forward, end );

  trap_Trace( &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );
  if( tr.surfaceFlags & SURF_NOIMPACT )
    return;

  traceEnt = &g_entities[ tr.entityNum ];

  // snap the endpos to integers, but nudged towards the line
  SnapVectorTowards( tr.endpos, muzzle );

  // send impact
  tent = G_TempEntity( tr.endpos, EV_MASS_DRIVER_HIT );
  tent->s.eventParm = DirToByte( tr.plane.normal );

  if( traceEnt->takedamage )
  {
    G_Damage( traceEnt, ent, ent, forward, tr.endpos,
      50, 0, MOD_MACHINEGUN);
  }
}

/*
======================================================================

LOCKBLOB

======================================================================
*/

void Weapon_LockBlobLauncher_Fire( gentity_t *ent )
{
  gentity_t *m;

  m = fire_lockblob( ent, muzzle, forward );

//  VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );  // "real" physics
}

/*
======================================================================

PLASMAGUN

======================================================================
*/

void Weapon_Plasma_Fire (gentity_t *ent) {
  gentity_t *m;

  m = fire_plasma (ent, muzzle, forward);

//  VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );  // "real" physics
}

/*
======================================================================

PULSE RIFLE

======================================================================
*/

void Weapon_PulseRifle_Fire (gentity_t *ent)
{
  gentity_t *m;

  m = fire_pulseRifle( ent, muzzle, forward );

//  VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );  // "real" physics
}

/*
======================================================================

FLAME THROWER

======================================================================
*/

void Weapon_Flamer_Fire (gentity_t *ent) {
  gentity_t *m;

  m = fire_flamer (ent, muzzle, forward);

  VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );  // "real" physics
}

/*
======================================================================

TESLA GENERATOR

======================================================================
*/


void Weapon_TeslaFire( gentity_t *ent )
{
  trace_t   tr;
  vec3_t    end;
  gentity_t *traceEnt, *tent;
  int     damage, i, passent;

  damage = 8;

  VectorMA( muzzle, LIGHTNING_RANGE, forward, end );

  trap_Trace( &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );

  if( tr.entityNum == ENTITYNUM_NONE )
    return;

  traceEnt = &g_entities[ tr.entityNum ];

  if( traceEnt->takedamage)
  {
    G_Damage( traceEnt, ent, ent, forward, tr.endpos,
      damage, 0, MOD_LIGHTNING);
  }

  // snap the endpos to integers to save net bandwidth, but nudged towards the line
  SnapVectorTowards( tr.endpos, muzzle );

  // send railgun beam effect
  tent = G_TempEntity( tr.endpos, EV_TESLATRAIL );

  // set player number for custom colors on the railtrail
  tent->s.clientNum = ent->s.clientNum;

  VectorCopy( muzzle, tent->s.origin2 );
  // move origin a bit to come closer to the drawn gun muzzle
  VectorMA( tent->s.origin2, 16, up, tent->s.origin2 );

  // no explosion at end if SURF_NOIMPACT, but still make the trail
  if( tr.surfaceFlags & SURF_NOIMPACT )
    tent->s.eventParm = 255;  // don't make the explosion at the end
  else
    tent->s.eventParm = DirToByte( tr.plane.normal );
}


//======================================================================

/*
======================================================================

BUILD GUN

======================================================================
*/


/*
===============
Weapon_Cancel_Build
===============
*/
void Weapon_Cancel_Build( gentity_t *ent )
{
  ent->client->ps.stats[ STAT_BUILDABLE ] = BA_NONE;
}

/*
===============
Weapon_Build_Fire
===============
*/
void Weapon_Build_Fire( gentity_t *ent, dynMenu_t menu )
{
  if( ( ent->client->ps.stats[ STAT_BUILDABLE ] & ~SB_VALID_TOGGLEBIT ) > BA_NONE )
  {
    G_ValidateBuild( ent, ent->client->ps.stats[ STAT_BUILDABLE ] & ~SB_VALID_TOGGLEBIT );
    ent->client->ps.stats[ STAT_BUILDABLE ] = BA_NONE;
    return;
  }

  G_AddPredictableEvent( ent, EV_MENU, menu );
}


/*
======================================================================

VENOM

======================================================================
*/

/*
===============
CheckVenomAttack
===============
*/
qboolean CheckVenomAttack( gentity_t *ent )
{
  trace_t   tr;
  vec3_t    end;
  gentity_t *tent;
  gentity_t *traceEnt;
  int     damage;

  // set aiming directions
  AngleVectors (ent->client->ps.viewangles, forward, right, up);

  CalcMuzzlePoint( ent, forward, right, up, muzzle );

  VectorMA (muzzle, 32, forward, end);

  trap_Trace (&tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT);
  if ( tr.surfaceFlags & SURF_NOIMPACT )
    return qfalse;

  traceEnt = &g_entities[ tr.entityNum ];

  if( !traceEnt->takedamage)
    return qfalse;
  if( !traceEnt->client )
    return qfalse;
  if( traceEnt->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS )
    return qfalse;

  // send blood impact
  if ( traceEnt->takedamage && traceEnt->client )
  {
    tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
    tent->s.otherEntityNum = traceEnt->s.number;
    tent->s.eventParm = DirToByte( tr.plane.normal );
    tent->s.weapon = ent->s.weapon;
  }

  G_Damage( traceEnt, ent, ent, forward, tr.endpos, 5, DAMAGE_NO_KNOCKBACK, MOD_VENOM );
  if( traceEnt->client )
  {
    if( !( traceEnt->client->ps.stats[ STAT_STATE ] & SS_POISONED ) )
    {
      traceEnt->client->ps.stats[ STAT_STATE ] |= SS_POISONED;
      traceEnt->client->lastPoisonTime = level.time;
    }
  }

  return qtrue;
}

/*
===============
Weapon_Venom_Fire
===============
*/
void Weapon_Venom_Fire( gentity_t *ent )
{
}

/*
======================================================================

GRAB AND CLAW

======================================================================
*/

/*
===============
Weapon_GClaw_Fire
===============
*/
void Weapon_GClaw_Fire( gentity_t *ent )
{
  trace_t   tr;
  vec3_t    end;
  gentity_t *tent;
  gentity_t *traceEnt;

  // set aiming directions
  AngleVectors (ent->client->ps.viewangles, forward, right, up);

  CalcMuzzlePoint( ent, forward, right, up, muzzle );

  VectorMA( muzzle, 32, forward, end );

  trap_Trace( &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );
  if ( tr.surfaceFlags & SURF_NOIMPACT )
    return;

  traceEnt = &g_entities[ tr.entityNum ];

  // send blood impact
  if ( traceEnt->takedamage && traceEnt->client )
  {
    tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
    tent->s.otherEntityNum = traceEnt->s.number;
    tent->s.eventParm = DirToByte( tr.plane.normal );
    tent->s.weapon = ent->s.weapon;
  }

  if ( traceEnt->takedamage )
    G_Damage( traceEnt, ent, ent, forward, tr.endpos, 5, DAMAGE_NO_KNOCKBACK, MOD_VENOM );
}

/*
===============
Weapon_Grab_Fire
===============
*/
void Weapon_Grab_Fire( gentity_t *ent )
{
  trace_t   tr;
  vec3_t    end;
  gentity_t *tent;
  gentity_t *traceEnt;
  int     damage;

  // set aiming directions
  AngleVectors (ent->client->ps.viewangles, forward, right, up);

  CalcMuzzlePoint( ent, forward, right, up, muzzle );

  VectorMA( muzzle, 32, forward, end );

  trap_Trace( &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );
  if ( tr.surfaceFlags & SURF_NOIMPACT )
    return;

  traceEnt = &g_entities[ tr.entityNum ];

  if( !traceEnt->takedamage )
    return;
  if( !traceEnt->client )
    return;
  if( traceEnt->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS )
    return;
    
  if( traceEnt->client )
  {
    //lock client
    traceEnt->client->ps.stats[ STAT_STATE ] |= SS_GRABBED;
    traceEnt->client->lastGrabTime = level.time;
    VectorCopy( traceEnt->client->ps.viewangles, traceEnt->client->ps.grapplePoint );
  }
}

/*
===============
CheckGrabAttack
===============
*/
qboolean CheckGrabAttack( gentity_t *ent )
{
  trace_t   tr;
  vec3_t    end;
  gentity_t *tent;
  gentity_t *traceEnt;
  int     damage;

  // set aiming directions
  AngleVectors (ent->client->ps.viewangles, forward, right, up);

  CalcMuzzlePoint( ent, forward, right, up, muzzle );

  VectorMA (muzzle, 32, forward, end);

  trap_Trace (&tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT);
  if ( tr.surfaceFlags & SURF_NOIMPACT )
    return qfalse;

  traceEnt = &g_entities[ tr.entityNum ];

  if( !traceEnt->takedamage)
    return qfalse;
  if( !traceEnt->client )
    return qfalse;
  if( traceEnt->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS )
    return qfalse;
    
  return qtrue;
}

/*
======================================================================

CLAW AND POUNCE

======================================================================
*/

/*
===============
Weapon_Claw_Fire
===============
*/
void Weapon_Claw_Fire( gentity_t *ent )
{
  trace_t   tr;
  vec3_t    end;
  gentity_t *tent;
  gentity_t *traceEnt;

  // set aiming directions
  AngleVectors (ent->client->ps.viewangles, forward, right, up);

  CalcMuzzlePoint( ent, forward, right, up, muzzle );

  VectorMA( muzzle, 32, forward, end );

  trap_Trace( &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );
  if ( tr.surfaceFlags & SURF_NOIMPACT )
    return;

  traceEnt = &g_entities[ tr.entityNum ];

  // send blood impact
  if ( traceEnt->takedamage && traceEnt->client )
  {
    tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
    tent->s.otherEntityNum = traceEnt->s.number;
    tent->s.eventParm = DirToByte( tr.plane.normal );
    tent->s.weapon = ent->s.weapon;
  }

  if ( traceEnt->takedamage )
    G_Damage( traceEnt, ent, ent, forward, tr.endpos, 50, DAMAGE_NO_KNOCKBACK, MOD_VENOM );
}

/*
===============
CheckPounceAttack
===============
*/
qboolean CheckPounceAttack( gentity_t *ent )
{
  trace_t   tr;
  vec3_t    end;
  gentity_t *tent;
  gentity_t *traceEnt;
  int     damage;

  if( !ent->client->allowedToPounce )
    return qfalse;

  if( ent->client->ps.groundEntityNum != ENTITYNUM_NONE )
    return qfalse;

  // set aiming directions
  AngleVectors (ent->client->ps.viewangles, forward, right, up);

  CalcMuzzlePoint( ent, forward, right, up, muzzle );

  VectorMA (muzzle, 48, forward, end);

  trap_Trace (&tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT);

  //miss
  if( tr.fraction >= 1.0 )
    return qfalse;
    
  if ( tr.surfaceFlags & SURF_NOIMPACT )
    return qfalse;

  traceEnt = &g_entities[ tr.entityNum ];

  // send blood impact
  if ( traceEnt->takedamage && traceEnt->client )
  {
    tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
    tent->s.otherEntityNum = traceEnt->s.number;
    tent->s.eventParm = DirToByte( tr.plane.normal );
    tent->s.weapon = ent->s.weapon;
  }

  if( !traceEnt->takedamage)
    return qfalse;

  damage = (int)( (float)ent->client->pouncePayload / ( MAX_POUNCE_SPEED / 100.0f ) );

  G_Damage( traceEnt, ent, ent, forward, tr.endpos, damage, DAMAGE_NO_KNOCKBACK, MOD_VENOM );

  ent->client->allowedToPounce = qfalse;
  
  return qtrue;
}

//======================================================================

/*
===============
LogAccuracyHit
===============
*/
qboolean LogAccuracyHit( gentity_t *target, gentity_t *attacker ) 
{ 
  //TA: theres a crash bug in here somewhere, but i'm too lazy to find it hence,
  return qfalse;
  
  /*if( !target->takedamage ) {
    return qfalse;
  }

  if ( target == attacker ) {
    return qfalse;
  }

  if( !target->client ) {
    return qfalse;
  }

  if( !attacker->client ) {
    return qfalse;
  }

  if( target->client->ps.stats[STAT_HEALTH] <= 0 ) {
    return qfalse;
  }

  if ( OnSameTeam( target, attacker ) ) {
    return qfalse;
  }

  return qtrue;*/
}


/*
===============
CalcMuzzlePoint

set muzzle location relative to pivoting eye
===============
*/
void CalcMuzzlePoint( gentity_t *ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint ) {
  VectorCopy( ent->s.pos.trBase, muzzlePoint );
  muzzlePoint[2] += ent->client->ps.viewheight;
  VectorMA( muzzlePoint, 1, forward, muzzlePoint );
  // snap to integer coordinates for more efficient network bandwidth usage
  SnapVector( muzzlePoint );
}

/*
===============
FireWeapon2
===============
*/
void FireWeapon2( gentity_t *ent )
{
  if( ent->client )
  {
    // set aiming directions
    AngleVectors (ent->client->ps.viewangles, forward, right, up);
    CalcMuzzlePoint( ent, forward, right, up, muzzle );
  }
  else
  {
    AngleVectors( ent->s.angles2, forward, right, up );
    VectorCopy( ent->s.pos.trBase, muzzle );
  }

  // fire the specific weapon
  switch( ent->s.weapon )
  {
    case WP_TESLAGEN:
      Weapon_TeslaFire( ent );
      break;
    case WP_MACHINEGUN:
      Bullet_Fire( ent, MACHINEGUN_SPREAD, MACHINEGUN_DAMAGE, MOD_MACHINEGUN );
      break;
    case WP_CHAINGUN:
      Bullet_Fire( ent, CHAINGUN_SPREAD, CHAINGUN_DAMAGE, MOD_CHAINGUN );
      break;
    case WP_FLAMER:
      Weapon_Flamer_Fire( ent );
      break;
    case WP_PLASMAGUN:
      Weapon_Plasma_Fire( ent );
      break;
    case WP_PULSE_RIFLE:
      Weapon_PulseRifle_Fire( ent );
      break;
    case WP_MASS_DRIVER:
      massDriverFire( ent );
      break;
    case WP_LOCKBLOB_LAUNCHER:
      break;
    case WP_VENOM:
      Weapon_Venom_Fire( ent );
      break;
    case WP_GRAB_CLAW:
      Weapon_Grab_Fire( ent );
      break;
    case WP_POUNCE:
      break;
    case WP_ABUILD:
      Weapon_Cancel_Build( ent );
      break;
    case WP_ABUILD2:
      Weapon_Cancel_Build( ent );
      break;
    case WP_HBUILD:
      Weapon_Cancel_Build( ent );
      break;
    case WP_HBUILD2:
      Weapon_Cancel_Build( ent );
      break;
    default:
  // FIXME    G_Error( "Bad ent->s.weapon" );
      break;
  }
}

/*
===============
FireWeapon
===============
*/
void FireWeapon( gentity_t *ent )
{
  if( ent->client )
  {
    // set aiming directions
    AngleVectors (ent->client->ps.viewangles, forward, right, up);
    CalcMuzzlePoint( ent, forward, right, up, muzzle );
  }
  else
  {
    AngleVectors( ent->turretAim, forward, right, up );
    /*AngleVectors( ent->s.angles2, forward, right, up );*/
    VectorCopy( ent->s.pos.trBase, muzzle );
  }

  // fire the specific weapon
  switch( ent->s.weapon )
  {
    case WP_TESLAGEN:
      Weapon_TeslaFire( ent );
      break;
    case WP_MACHINEGUN:
      Bullet_Fire( ent, MACHINEGUN_SPREAD, MACHINEGUN_DAMAGE, MOD_MACHINEGUN );
      break;
    case WP_CHAINGUN:
      Bullet_Fire( ent, CHAINGUN_SPREAD, CHAINGUN_DAMAGE, MOD_CHAINGUN );
      break;
    case WP_FLAMER:
      Weapon_Flamer_Fire( ent );
      break;
    case WP_PLASMAGUN:
      Weapon_Plasma_Fire( ent );
      break;
    case WP_PULSE_RIFLE:
      Weapon_PulseRifle_Fire( ent );
      break;
    case WP_MASS_DRIVER:
      massDriverFire( ent );
      break;
    case WP_LOCKBLOB_LAUNCHER:
      Weapon_LockBlobLauncher_Fire( ent );
      break;
    case WP_VENOM:
      Weapon_Venom_Fire( ent );
      break;
    case WP_GRAB_CLAW:
      Weapon_GClaw_Fire( ent );
      break;
    case WP_POUNCE:
      Weapon_Claw_Fire( ent );
      break;
    case WP_ABUILD:
      Weapon_Build_Fire( ent, MN_A_BUILD );
      break;
    case WP_ABUILD2:
      Weapon_Build_Fire( ent, MN_A_BUILD );
      break;
    case WP_HBUILD:
      Weapon_Build_Fire( ent, MN_H_BUILD );
      break;
    case WP_HBUILD2:
      Weapon_Build_Fire( ent, MN_H_BUILD );
      break;
    default:
  // FIXME    G_Error( "Bad ent->s.weapon" );
      break;
  }
}

