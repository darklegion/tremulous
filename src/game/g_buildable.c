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

/*
================
G_setBuildableAnim

Triggers an animation client side
================
*/
void G_setBuildableAnim( gentity_t *ent, buildableAnimNumber_t anim, qboolean force )
{
  int localAnim = anim;

  if( force )
    localAnim |= ANIM_TOGGLEBIT;
    
  G_AddEvent( ent, EV_BUILD_ANIM, localAnim );
}

/*
================
G_setIdleBuildableAnim

Set the animation to use whilst no other animations are running
================
*/
void G_setIdleBuildableAnim( gentity_t *ent, buildableAnimNumber_t anim )
{
  ent->s.torsoAnim = anim;
}

#define REFRESH_TIME  2000

/*
================
findPower

attempt to find power for self, return qtrue if successful
================
*/
static qboolean findPower( gentity_t *self )
{
  int       i;
  gentity_t *ent;
  gentity_t *closestPower;
  int       distance = 0;
  int       minDistance = 10000;
  vec3_t    temp_v;
  qboolean  foundPower = qfalse;

  if( self->biteam != BIT_HUMANS )
    return qfalse;
  
  //reactor is always powered
  if( self->s.modelindex == BA_H_REACTOR )
    return qtrue;
  
  //if this already has power then stop now
  if( self->parentNode && self->parentNode->powered )
    return qtrue;
  
  //reset parent
  self->parentNode = NULL;
  
  //iterate through entities
  for ( i = 1, ent = g_entities + i; i < level.num_entities; i++, ent++ )
  {
    if( !ent->classname || ent->s.eType != ET_BUILDABLE )
      continue;

    //if entity is a power item calculate the distance to it
    if( ent->s.modelindex == BA_H_REACTOR || ent->s.modelindex == BA_H_REPEATER )
    {
      VectorSubtract( self->s.origin, ent->s.origin, temp_v );
      distance = VectorLength( temp_v );
      if( distance < minDistance && ent->powered )
      {
        closestPower = ent;
        minDistance = distance;
        foundPower = qtrue;
      }
    }
  }

  //if there were no power items nearby give up
  if( !foundPower )
    return qfalse;
  
  //bleh
  if( ( closestPower->s.modelindex == BA_H_REACTOR && ( minDistance <= REACTOR_BASESIZE ) ) || 
      ( closestPower->s.modelindex == BA_H_REPEATER && ( minDistance <= REPEATER_BASESIZE ) &&
        closestPower->powered ) )
  {
    self->parentNode = closestPower;

    return qtrue;
  }
  else
    return qfalse;
}

/*
================
isPower

simple wrapper to findPower to check if a location has power
================
*/
static qboolean isPower( vec3_t origin )
{
  gentity_t dummy;

  dummy.parentNode = NULL;
  dummy.biteam = BIT_HUMANS;
  VectorCopy( origin, dummy.s.origin );

  return findPower( &dummy );
}

/*
================
findDCC

attempt to find a controlling DCC for self, return qtrue if successful
================
*/
static qboolean findDCC( gentity_t *self )
{
  int       i;
  gentity_t *ent;
  gentity_t *closestDCC;
  int       distance = 0;
  int       minDistance = 10000;
  vec3_t    temp_v;
  qboolean  foundDCC = qfalse;

  if( self->biteam != BIT_HUMANS )
    return qfalse;
  
  //if this already has dcc then stop now
  if( self->dccNode && self->dccNode->powered )
    return qtrue;
  
  //reset parent
  self->dccNode = NULL;
  
  //iterate through entities
  for( i = 1, ent = g_entities + i; i < level.num_entities; i++, ent++ )
  {
    if( !ent->classname || ent->s.eType != ET_BUILDABLE )
      continue;

    //if entity is a power item calculate the distance to it
    if( ent->s.modelindex == BA_H_DCC )
    {
      VectorSubtract( self->s.origin, ent->s.origin, temp_v );
      distance = VectorLength( temp_v );
      if( distance < minDistance && ent->powered )
      {
        closestDCC = ent;
        minDistance = distance;
        foundDCC = qtrue;
      }
    }
  }

  //if there were no power items nearby give up
  if( !foundDCC )
    return qfalse;
  
  self->dccNode = closestDCC;

  return qtrue;
}

/*
================
isDCC

simple wrapper to findDCC to check for a dcc
================
*/
static qboolean isDCC( )
{
  gentity_t dummy;

  dummy.dccNode = NULL;
  dummy.biteam = BIT_HUMANS;

  return findDCC( &dummy );
}

/*
================
findCreep

attempt to find creep for self, return qtrue if successful
================
*/
static qboolean findCreep( gentity_t *self )
{
  int       i;
  gentity_t *ent;
  gentity_t *closestSpawn;
  int       distance = 0;
  int       minDistance = 10000;
  vec3_t    temp_v;

  //don't check for creep if flying through the air
  if( self->s.groundEntityNum == -1 )
    return qtrue;
  
  //if self does not have a parentNode or it's parentNode is invalid find a new one
  if( ( self->parentNode == NULL ) || !self->parentNode->inuse )
  {
    for ( i = 1, ent = g_entities + i; i < level.num_entities; i++, ent++ )
    {
      if( !ent->classname || ent->s.eType != ET_BUILDABLE )
        continue;

      if( ent->s.modelindex == BA_A_SPAWN || ent->s.modelindex == BA_A_HIVEMIND )
      {
        VectorSubtract( self->s.origin, ent->s.origin, temp_v );
        distance = VectorLength( temp_v );
        if( distance < minDistance )
        {
          closestSpawn = ent;
          minDistance = distance;
        }
      }
    }
    
    if( minDistance <= CREEP_BASESIZE )
    {
      self->parentNode = closestSpawn;
      return qtrue;
    }
    else
      return qfalse;
  }

  //if we haven't returned by now then we must already have a valid parent
  return qtrue;
}

/*
================
isCreep

simple wrapper to findCreep to check if a location has creep
================
*/
static qboolean isCreep( vec3_t origin )
{
  gentity_t dummy;

  dummy.parentNode = NULL;
  VectorCopy( origin, dummy.s.origin );

  return findCreep( &dummy );
}

/*
================
nullDieFunction

hack to prevent compilers complaining about function pointer -> NULL conversion
================
*/
static void nullDieFunction( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
}

/*
================
freeBuildable
================
*/
static void freeBuildable( gentity_t *self )
{
  G_FreeEntity( self );
}


//==================================================================================



/*
================
A_CreepRecede

Called when an alien spawn dies
================
*/
void A_CreepRecede( gentity_t *self )
{
  //if the creep just died begin the recession
  if( !( self->s.eFlags & EF_DEAD ) )
  {
    self->s.eFlags |= EF_DEAD;
    G_AddEvent( self, EV_BUILD_DESTROY, 0 );
  }
  
  //creep is still receeding
  if( ( self->timestamp + 10000 ) > level.time )
  {
    self->nextthink = level.time + 500;
    trap_LinkEntity( self );
  }
  else //creep has died
    G_FreeEntity( self );
}




//==================================================================================




/*
================
ASpawn_Melt

Called when an alien spawn dies
================
*/
void ASpawn_Melt( gentity_t *self )
{
  //FIXME: this line crashes the QVM (but not binary when MOD is set to MOD_[H/D]SPAWN
  G_SelectiveRadiusDamage( self->s.pos.trBase, self->parent, self->splashDamage,
    self->splashRadius, self, MOD_SHOTGUN, PTE_ALIENS );

  //start creep recession
  if( !( self->s.eFlags & EF_DEAD ) )
  {
    self->s.eFlags |= EF_DEAD;
    G_AddEvent( self, EV_BUILD_DESTROY, 0 );
  }
  
  //not dead yet
  if( ( self->timestamp + 10000 ) > level.time )
  {
    self->nextthink = level.time + 500;
    trap_LinkEntity( self );
  }
  else //dead now
    G_FreeEntity( self );
}

/*
================
ASpawn_Blast

Called when an alien spawn dies
================
*/
void ASpawn_Blast( gentity_t *self )
{
  vec3_t  dir;

  // we don't have a valid direction, so just point straight up
  dir[ 0 ] = dir[ 1 ] = 0;
  dir[ 2 ] = 1;

  //do a bit of radius damage
  G_SelectiveRadiusDamage( self->s.pos.trBase, self->parent, self->splashDamage,
    self->splashRadius, self, self->splashMethodOfDeath, PTE_ALIENS );

  //pretty events and item cleanup
  self->s.modelindex = 0; //don't draw the model once its destroyed
  G_AddEvent( self, EV_GIB_ALIEN, DirToByte( dir ) );
  self->r.contents = CONTENTS_TRIGGER;
  self->timestamp = level.time;
  self->think = ASpawn_Melt;
  self->nextthink = level.time + 500; //wait .5 seconds before damaging others
    
  trap_LinkEntity( self );
}

/*
================
ASpawn_Die

Called when an alien spawn dies
================
*/
void ASpawn_Die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
  G_setBuildableAnim( self, BANIM_DESTROY1, qtrue );
  G_setIdleBuildableAnim( self, BANIM_DESTROYED );

  self->die = nullDieFunction;
  self->think = ASpawn_Blast;
  self->nextthink = level.time + 5000; //wait .5 seconds before damaging others
    
  trap_LinkEntity( self );
}

/*
================
ASpawn_Think

think function for Alien Spawn
================
*/
void ASpawn_Think( gentity_t *self )
{
  vec3_t    mins, maxs, origin;
  gentity_t *ent;
  trace_t   tr;
  float     displacement;

  VectorSet( mins, -MAX_ALIEN_BBOX, -MAX_ALIEN_BBOX, -MAX_ALIEN_BBOX );
  VectorSet( maxs,  MAX_ALIEN_BBOX,  MAX_ALIEN_BBOX,  MAX_ALIEN_BBOX );
  
  VectorCopy( self->s.origin, origin );
  displacement = ( self->r.maxs[ 2 ] + MAX_ALIEN_BBOX ) * M_ROOT3 + 1.0f;
  VectorMA( origin, displacement, self->s.origin2, origin );
  
  //only suicide if at rest
  if( self->s.groundEntityNum )
  {
    trap_Trace( &tr, origin, mins, maxs, origin, self->s.number, MASK_SHOT );
    ent = &g_entities[ tr.entityNum ];
    
    if( ent->s.eType == ET_BUILDABLE || ent->s.number == ENTITYNUM_WORLD )
    {
      G_Damage( self, NULL, NULL, NULL, NULL, 10000, 0, MOD_SUICIDE );
      return;
    }

    if( ent->s.eType == ET_CORPSE )
      G_FreeEntity( ent ); //quietly remove
  }

  self->nextthink = level.time + BG_FindNextThinkForBuildable( self->s.modelindex );
}

/*
================
ASpawn_Pain

pain function for Alien Spawn
================
*/
void ASpawn_Pain( gentity_t *self, gentity_t *attacker, int damage )
{
  G_setBuildableAnim( self, BANIM_PAIN1, qfalse );
}





//==================================================================================





/*
================
AOvermind_Think

think function for Alien Acid Tube
================
*/
void AOvermind_Think( gentity_t *self )
{
  int       entityList[ MAX_GENTITIES ];
  vec3_t    range = { OVERMIND_ATTACK_RANGE, OVERMIND_ATTACK_RANGE, OVERMIND_ATTACK_RANGE };
  vec3_t    mins, maxs;
  int       i, num;
  gentity_t *enemy;

  VectorAdd( self->s.origin, range, maxs );
  VectorSubtract( self->s.origin, range, mins );
  
  //do some damage
  num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
  for( i = 0; i < num; i++ )
  {
    enemy = &g_entities[ entityList[ i ] ];
    
    if( enemy->client && enemy->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
    {
      self->timestamp = level.time;
      G_SelectiveRadiusDamage( self->s.pos.trBase, self, self->splashDamage,
        self->splashRadius, self, MOD_SHOTGUN, PTE_ALIENS );
      G_setBuildableAnim( self, BANIM_ATTACK1, qfalse );
    }
  }

  self->nextthink = level.time + BG_FindNextThinkForBuildable( self->s.modelindex );
}






//==================================================================================





/*
================
ABarricade_Pain

pain function for Alien Spawn
================
*/
void ABarricade_Pain( gentity_t *self, gentity_t *attacker, int damage )
{
  if( rand( ) % 1 )
    G_setBuildableAnim( self, BANIM_PAIN1, qfalse );
  else
    G_setBuildableAnim( self, BANIM_PAIN2, qfalse );
}

/*
================
ABarricade_Blast

Called when an alien spawn dies
================
*/
void ABarricade_Blast( gentity_t *self )
{
  vec3_t  dir;

  // we don't have a valid direction, so just point straight up
  dir[ 0 ] = dir[ 1 ] = 0;
  dir[ 2 ] = 1;

  //do a bit of radius damage
  G_SelectiveRadiusDamage( self->s.pos.trBase, self->parent, self->splashDamage,
    self->splashRadius, self, self->splashMethodOfDeath, PTE_ALIENS );

  //pretty events and item cleanup
  self->s.modelindex = 0; //don't draw the model once its destroyed
  G_AddEvent( self, EV_GIB_ALIEN, DirToByte( dir ) );
  self->r.contents = CONTENTS_TRIGGER;
  self->timestamp = level.time;
  self->think = A_CreepRecede;
  self->nextthink = level.time + 500; //wait .5 seconds before damaging others
    
  trap_LinkEntity( self );
}

/*
================
ABarricade_Die

Called when an alien spawn dies
================
*/
void ABarricade_Die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
  G_setBuildableAnim( self, BANIM_DESTROY1, qtrue );
  G_setIdleBuildableAnim( self, BANIM_DESTROYED );
  
  self->die = nullDieFunction;
  self->think = ABarricade_Blast;
  self->nextthink = level.time + 5000;

  trap_LinkEntity( self );
}

/*
================
ABarricade_Think

think function for Alien Barricade
================
*/
void ABarricade_Think( gentity_t *self )
{
  //if there is no creep nearby die
  if( !findCreep( self ) )
  {
    G_Damage( self, NULL, NULL, NULL, NULL, 10000, 0, MOD_SUICIDE );
    return;
  }
  
  self->nextthink = level.time + BG_FindNextThinkForBuildable( self->s.modelindex );
}




//==================================================================================




void AAcidTube_Think( gentity_t *self );

/*
================
AAcidTube_Damage

damage function for Alien Acid Tube
================
*/
void AAcidTube_Damage( gentity_t *self )
{
  if( !( self->s.eFlags & EF_FIRING ) )
  {
    self->s.eFlags |= EF_FIRING;
    G_AddEvent( self, EV_GIB_ALIEN, DirToByte( self->s.origin2 ) );
  }
    
  if( ( self->timestamp + ACIDTUBE_REPEAT ) > level.time )
    self->think = AAcidTube_Damage;
  else
  {
    self->think = AAcidTube_Think;
    self->s.eFlags &= ~EF_FIRING;
  }

  //do some damage
  G_SelectiveRadiusDamage( self->s.pos.trBase, self->parent, self->splashDamage,
    self->splashRadius, self, self->splashMethodOfDeath, PTE_ALIENS );

  self->nextthink = level.time + BG_FindNextThinkForBuildable( self->s.modelindex );
}

/*
================
AAcidTube_Think

think function for Alien Acid Tube
================
*/
void AAcidTube_Think( gentity_t *self )
{
  int       entityList[ MAX_GENTITIES ];
  vec3_t    range = { ACIDTUBE_RANGE, ACIDTUBE_RANGE, ACIDTUBE_RANGE };
  vec3_t    mins, maxs;
  int       i, num;
  gentity_t *enemy;

  VectorAdd( self->s.origin, range, maxs );
  VectorSubtract( self->s.origin, range, mins );
  
  //if there is no creep nearby die
  if( !findCreep( self ) )
  {
    G_Damage( self, NULL, NULL, NULL, NULL, 10000, 0, MOD_SUICIDE );
    return;
  }
  
  //do some damage
  num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
  for( i = 0; i < num; i++ )
  {
    enemy = &g_entities[ entityList[ i ] ];
    
    if( enemy->client && enemy->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
    {
      self->timestamp = level.time;
      self->think = AAcidTube_Damage;
      self->nextthink = level.time + 100;
      G_setBuildableAnim( self, BANIM_ATTACK1, qfalse );
      return;
    }
  }

  self->nextthink = level.time + BG_FindNextThinkForBuildable( self->s.modelindex );
}




//==================================================================================



/*
================
AHovel_Use

Called when an alien uses a hovel
================
*/
void AHovel_Use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
  vec3_t  hovelOrigin, hovelAngles, inverseNormal;
  
  if( self->active )
  {
    //this hovel is in use
    G_AddPredictableEvent( activator, EV_MENU, MN_A_HOVEL_OCCUPIED );
  }
  else if( ( activator->client->ps.stats[ STAT_PCLASS ] == PCL_A_B_BASE ) ||
           ( activator->client->ps.stats[ STAT_PCLASS ] == PCL_A_B_LEV1 ) )
  {
    self->active = qtrue;
    G_setBuildableAnim( self, BANIM_ATTACK1, qfalse );

    //prevent lerping
    activator->client->ps.eFlags ^= EF_TELEPORT_BIT;
    
    activator->client->sess.sessionTeam = TEAM_FREE;
    activator->client->ps.stats[ STAT_STATE ] |= SS_HOVELING;
    activator->client->infestBody = self;
    self->builder = activator;

    VectorCopy( self->s.pos.trBase, hovelOrigin );
    VectorMA( hovelOrigin, 128.0f, self->s.origin2, hovelOrigin );

    VectorCopy( self->s.origin2, inverseNormal );
    VectorInverse( inverseNormal );
    vectoangles( inverseNormal, hovelAngles );

    VectorCopy( activator->s.pos.trBase, activator->client->hovelOrigin );

    G_SetOrigin( activator, hovelOrigin );
    VectorCopy( hovelOrigin, activator->client->ps.origin );
    SetClientViewAngle( activator, hovelAngles );
  }
}


/*
================
AHovel_Think

Think for alien hovel
================
*/
void AHovel_Think( gentity_t *self )
{
  if( self->active )
    G_setIdleBuildableAnim( self, BANIM_IDLE2 );
  else
    G_setIdleBuildableAnim( self, BANIM_IDLE1 );
    
  self->nextthink = level.time + 200;
}

/*
================
AHovel_Die

Die for alien hovel
================
*/
void AHovel_Die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
  vec3_t  dir;

  // we don't have a valid direction, so just point straight up
  dir[ 0 ] = dir[ 1 ] = 0;
  dir[ 2 ] = 1;

  //do a bit of radius damage
  G_SelectiveRadiusDamage( self->s.pos.trBase, self->parent, self->splashDamage,
    self->splashRadius, self, self->splashMethodOfDeath, PTE_ALIENS );

  //pretty events and item cleanup
  self->s.modelindex = 0; //don't draw the model once its destroyed
  G_AddEvent( self, EV_GIB_ALIEN, DirToByte( dir ) );
  self->r.contents = CONTENTS_TRIGGER;
  self->timestamp = level.time;
  self->think = ASpawn_Melt;
  self->nextthink = level.time + 500; //wait .5 seconds before damaging others
  
  //if the hovel is occupied free the occupant
  if( self->active )
  {
    gentity_t *builder = self->builder;
    vec3_t    newOrigin;
    vec3_t    newAngles;
    
    VectorCopy( self->s.angles, newAngles );
    newAngles[ ROLL ] = 0;
    
    VectorCopy( self->s.origin, newOrigin );
    VectorMA( newOrigin, 1.0f, self->s.origin2, newOrigin );
    
    //prevent lerping
    builder->client->ps.eFlags ^= EF_TELEPORT_BIT;
      
    G_SetOrigin( builder, newOrigin );
    VectorCopy( newOrigin, builder->client->ps.origin );
    SetClientViewAngle( builder, newAngles );
    
    //client leaves hovel
    builder->client->ps.stats[ STAT_STATE ] &= ~SS_HOVELING;
  }
  
  trap_LinkEntity( self );
}




//==================================================================================




/*
================
ABank_Activate

Called when an alien activates an organ bank 
================
*/
void ABank_Activate( gentity_t *self, gentity_t *other, gentity_t *activator )
{
  //only aliens can activate this
  if( activator->client->ps.stats[ STAT_PTEAM ] != PTE_ALIENS ) return;
  
  G_AddPredictableEvent( activator, EV_MENU, MN_A_OBANK );
}




//==================================================================================




/*
================
ABooster_Touch

Called when an alien touches a booster
================
*/
void ABooster_Touch( gentity_t *self, gentity_t *other, trace_t *trace )
{
  int       ammo, clips, maxClips;
  gclient_t *client = other->client;
  
  //only allow boostage once every 30 seconds
  if( client->lastBoostedTime + BOOSTER_INTERVAL > level.time )
    return;
  
  //restore ammo, if any
  BG_FindAmmoForWeapon( client->ps.weapon, &ammo, &clips, &maxClips );
  BG_packAmmoArray( client->ps.weapon, client->ps.ammo, client->ps.powerups, ammo, clips, maxClips );
  
  if( !( client->ps.stats[ STAT_STATE ] & SS_BOOSTED ) )
  {
    client->ps.stats[ STAT_STATE ] |= SS_BOOSTED;
    client->lastBoostedTime = level.time;
  }
}




//==================================================================================


/*
================
adef_fireonemeny

Used by ADef2_Think to fire at enemy
================
*/
void adef_fireonenemy( gentity_t *self, int firespeed )
{
  vec3_t  dirToTarget;
  vec3_t  target;
  vec3_t  halfAcceleration, thirdJerk;
  float   distanceToTarget = BG_FindRangeForBuildable( self->s.modelindex );
  int     i;
 
  VectorScale( self->enemy->acceleration, 1.0f / 2.0f, halfAcceleration );
  VectorScale( self->enemy->jerk, 1.0f / 3.0f, thirdJerk );

  //O( time ) - worst case O( time ) = 250 iterations
  for( i = 0; ( i * LOCKBLOB_SPEED ) / 1000.0f < distanceToTarget; i++ )
  {
    float time = (float)i / 1000.0f;

    VectorMA( self->enemy->s.pos.trBase, time, self->enemy->s.pos.trDelta,
              dirToTarget );
    VectorMA( dirToTarget, time * time, halfAcceleration, dirToTarget );
    VectorMA( dirToTarget, time * time * time, thirdJerk, dirToTarget );
    VectorSubtract( dirToTarget, self->s.pos.trBase, dirToTarget );
    distanceToTarget = VectorLength( dirToTarget );

    distanceToTarget -= self->enemy->r.maxs[ 0 ];
  }
  
  VectorNormalize( dirToTarget );
  vectoangles( dirToTarget, self->turretAim );

  //fire at target
  FireWeapon( self );
  G_setBuildableAnim( self, BANIM_ATTACK1, qfalse );
  self->count = level.time + firespeed;
}

/*
================
adef_checktarget

Used by DDef2_Think to check enemies for validity
================
*/
qboolean adef_checktarget( gentity_t *self, gentity_t *target, int range )
{
  vec3_t    distance;
  trace_t   trace;

  if( !target ) // Do we have a target?
    return qfalse;
  if( !target->inuse ) // Does the target still exist?
    return qfalse;
  if( target == self ) // is the target us?
    return qfalse;
  if( !target->client ) // is the target a bot or player?
    return qfalse;
  if( target->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS ) // one of us?
    return qfalse;
  if( target->client->sess.sessionTeam == TEAM_SPECTATOR ) // is the target alive?
    return qfalse;
  if( target->health <= 0 ) // is the target still alive?
    return qfalse;
  if( target->client->ps.stats[ STAT_STATE ] & SS_BLOBLOCKED ) // locked?
    return qfalse;

  VectorSubtract( target->r.currentOrigin, self->r.currentOrigin, distance );
  if( VectorLength( distance ) > range ) // is the target within range?
    return qfalse;

  //only allow a narrow field of "vision"
  VectorNormalize( distance ); //is now direction of target
  if( DotProduct( distance, self->s.origin2 ) < LOCKBLOB_DOT )
    return qfalse;

  trap_Trace( &trace, self->s.pos.trBase, NULL, NULL, target->s.pos.trBase, self->s.number, MASK_SHOT );
  if ( trace.contents & CONTENTS_SOLID ) // can we see the target?
    return qfalse;

  return qtrue;
}

/*
================
adef_findenemy

Used by DDef2_Think to locate enemy gentities
================
*/
void adef_findenemy( gentity_t *ent, int range )
{
  gentity_t *target;

  //iterate through entities
  for( target = g_entities; target < &g_entities[ level.num_entities ]; target++ )
  {
    //if target is not valid keep searching
    if( !adef_checktarget( ent, target, range ) )
      continue;
      
    //we found a target
    ent->enemy = target;
    return;
  }

  //couldn't find a target
  ent->enemy = NULL;
}

/*
================
ATrapper_Think

think function for Alien Defense
================
*/
void ATrapper_Think( gentity_t *self )
{
  int range =     BG_FindRangeForBuildable( self->s.modelindex );
  int firespeed = BG_FindFireSpeedForBuildable( self->s.modelindex );

  self->nextthink = level.time + BG_FindNextThinkForBuildable( self->s.modelindex );

  //if there is no creep nearby die
  if( !findCreep( self ) )
  {
    G_Damage( self, NULL, NULL, NULL, NULL, 10000, 0, MOD_SUICIDE );
    return;
  }

  //if the current target is not valid find a new one
  if( !adef_checktarget( self, self->enemy, range ) )
    adef_findenemy( self, range );

  //if a new target cannot be found don't do anything
  if( !self->enemy )
    return;
  
  //if we are pointing at our target and we can fire shoot it
  if( self->count < level.time )
    adef_fireonenemy( self, firespeed );
}



//==================================================================================



/*
================
HRpt_Think

Think for human power repeater
================
*/
void HRpt_Think( gentity_t *self )
{
  int       i;
  qboolean  reactor = qfalse;
  gentity_t *ent;
  
  //iterate through entities
  for ( i = 1, ent = g_entities + i; i < level.num_entities; i++, ent++ )
  {
    if( !ent->classname || ent->s.eType != ET_BUILDABLE )
      continue;

    if( ent->s.modelindex == BA_H_REACTOR )
      reactor = qtrue;
  }
  
  self->powered = reactor;

  self->nextthink = level.time + REFRESH_TIME;
}

/*
================
HRpt_Use

Use for human power repeater
================
*/
void HRpt_Use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
  int maxAmmo, maxClips;
  int ammo, clips;

  playerState_t *ps = &activator->client->ps;

  if( activator->client->lastRefilTime + ENERGY_REFIL_TIME > level.time )
    return;
  
  if( !BG_FindUsesEnergyForWeapon( ps->weapon ) )
    return;
  
  BG_FindAmmoForWeapon( ps->weapon, &maxAmmo, NULL, &maxClips );
  
  if( BG_gotItem( UP_BATTPACK, ps->stats ) )
    maxAmmo = (int)( (float)maxAmmo * BATTPACK_MODIFIER );
  
  BG_unpackAmmoArray( ps->weapon, ps->ammo, ps->powerups, &ammo, &clips, NULL );

  if( ammo == maxAmmo && clips < maxClips )
  {
    clips++;
    ammo = 0;
  }
  
  //add half max ammo
  ammo += maxAmmo >> 1;
  
  if( ammo > maxAmmo )
    ammo = maxAmmo;

  BG_packAmmoArray( ps->weapon, ps->ammo, ps->powerups, ammo, clips, maxClips );

  activator->client->lastRefilTime = level.time;
}


//==================================================================================



/*
================
HArmoury_Activate

Called when a human activates an Armoury
================
*/
void HArmoury_Activate( gentity_t *self, gentity_t *other, gentity_t *activator )
{
  //only humans can activate this
  if( activator->client->ps.stats[ STAT_PTEAM ] != PTE_HUMANS ) return;
  
  //if this is powered then call the armoury menu
  if( self->powered )
    G_AddPredictableEvent( activator, EV_MENU, MN_H_ARMOURY );
  else
    G_AddPredictableEvent( activator, EV_MENU, MN_H_NOPOWER );
}

/*
================
HArmoury_Think

Think for armoury
================
*/
void HArmoury_Think( gentity_t *self )
{
  //make sure we have power
  self->nextthink = level.time + REFRESH_TIME;
  
  self->powered = findPower( self );
}




//==================================================================================



/*
================
HBank_Activate

Called when a human activates a Bank 
================
*/
void HBank_Activate( gentity_t *self, gentity_t *other, gentity_t *activator )
{
  //only humans can activate this
  if( activator->client->ps.stats[ STAT_PTEAM ] != PTE_HUMANS ) return;
  
  //if this is powered then call the bank menu
  if( self->powered )
    G_AddPredictableEvent( activator, EV_MENU, MN_H_BANK );
  else
    G_AddPredictableEvent( activator, EV_MENU, MN_H_NOPOWER );
}

/*
================
HBank_Think

Think for bank
================
*/
void HBank_Think( gentity_t *self )
{
  //make sure we have power
  self->nextthink = level.time + REFRESH_TIME;
  
  self->powered = findPower( self );
}




//==================================================================================




/*
================
HDCC_Think

Think for dcc
================
*/
void HDCC_Think( gentity_t *self )
{
  //make sure we have power
  self->nextthink = level.time + REFRESH_TIME;
  
  self->powered = findPower( self );
}




//==================================================================================

/*
================
HMedistat_Think

think function for Human Medistation
================
*/
void HMedistat_Think( gentity_t *self )
{
  int       entityList[ MAX_GENTITIES ];
  vec3_t    mins, maxs;
  int       i, num;
  gentity_t *player;
  int       healCount = 0;
  int       maxclients;

  //make sure we have power
  if( !( self->powered = findPower( self ) ) )
  {
    self->nextthink = level.time + REFRESH_TIME;
    return;
  }
  
  if( self->s.modelindex == BA_H_ADVMEDISTAT )
  {
    maxclients = MAX_ADVMEDISTAT_CLIENTS;

    //the advanced medistat requires a DCC
    if( !( self->dcced = findDCC( self ) ) )
    {
      self->nextthink = level.time + REFRESH_TIME;
      return;
    }
  }
  else
    maxclients = MAX_MEDISTAT_CLIENTS;
  
  VectorAdd( self->s.origin, self->r.maxs, maxs );
  VectorAdd( self->s.origin, self->r.mins, mins );

  mins[ 2 ] += fabs( self->r.mins[ 2 ] ) + self->r.maxs[ 2 ];
  maxs[ 2 ] += 60; //player height
  
  //if active use the healing idle
  if( self->active )
    G_setIdleBuildableAnim( self, BANIM_IDLE2 );

  //do some healage
  num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
  for( i = 0; i < num; i++ )
  {
    player = &g_entities[ entityList[ i ] ];
    
    if( player->client && player->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
    {
      if( player->health < player->client->ps.stats[ STAT_MAX_HEALTH ] &&
          player->client->ps.pm_type != PM_DEAD &&
          healCount < maxclients )
      {
        healCount++;
        player->health++;
        
        //start the heal anim
        if( !self->active )
        {
          G_setBuildableAnim( self, BANIM_ATTACK1, qfalse );
          self->active = qtrue;
        }
      }
    }
  }

  //nothing left to heal so go back to idling
  if( healCount == 0 && self->active )
  {
    G_setBuildableAnim( self, BANIM_CONSTRUCT2, qtrue );
    G_setIdleBuildableAnim( self, BANIM_IDLE1 );
    
    self->active = qfalse;
  }
    
  self->nextthink = level.time + BG_FindNextThinkForBuildable( self->s.modelindex );
}




//==================================================================================



/*
================
HFM_Touch

Called when a floatmine is triggered
================
*/
void HFM_Touch( gentity_t *self, gentity_t *other, trace_t *trace )
{
  //can't blow up twice
  if( self->health <= 0 )
    return;
    
  //go boom
  G_Damage( self, NULL, NULL, NULL, NULL, 10000, 0, MOD_SUICIDE );
}

/*
================
HFM_Die

Called when a floatmine dies
================
*/
void HFM_Die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
  vec3_t  dir;
  
  // we don't have a valid direction, so just point straight up
  dir[ 0 ] = dir[ 1 ] = 0;
  dir[ 2 ] = -1;

  //do a bit of radius damage
  G_RadiusDamage( self->s.pos.trBase, self->parent, self->splashDamage,
    self->splashRadius, self, self->splashMethodOfDeath );

  //pretty events and item cleanup
  self->s.modelindex = 0; //don't draw the model once its destroyed
  G_AddEvent( self, EV_BUILDABLE_EXPLOSION, DirToByte( dir ) );
  self->r.contents = CONTENTS_TRIGGER;
  self->timestamp = level.time;

  self->think = freeBuildable;
  self->die = nullDieFunction;
  self->nextthink = level.time + 100;
  
  trap_LinkEntity( self );
}

/*
================
HFM_Think

Think for floatmine
================
*/
void HFM_Think( gentity_t *self )
{
  //make sure we have power
  self->nextthink = level.time + REFRESH_TIME;
  
  if( !( self->powered = findPower( self ) ) )
    G_Damage( self, NULL, NULL, NULL, NULL, 10000, 0, MOD_SUICIDE );
}




//==================================================================================

/*
================
HMGTurret_TrackEnemy

Used by HDef_Think to track enemy location
================
*/
qboolean HMGTurret_TrackEnemy( gentity_t *self )
{
  vec3_t  dirToTarget, dttAdjusted, angleToTarget, angularDiff, xNormal;
  vec3_t  refNormal = { 0.0f, 0.0f, 1.0f };
  float   temp, rotAngle;

  VectorSubtract( self->enemy->s.pos.trBase, self->s.pos.trBase, dirToTarget );

  VectorNormalize( dirToTarget );
  
  CrossProduct( self->s.origin2, refNormal, xNormal );
  VectorNormalize( xNormal );
  rotAngle = RAD2DEG( acos( DotProduct( self->s.origin2, refNormal ) ) );
  RotatePointAroundVector( dttAdjusted, xNormal, dirToTarget, rotAngle );
  
  vectoangles( dttAdjusted, angleToTarget );

  angularDiff[ PITCH ] = AngleSubtract( self->s.angles2[ PITCH ], angleToTarget[ PITCH ] );
  angularDiff[ YAW ] = AngleSubtract( self->s.angles2[ YAW ], angleToTarget[ YAW ] );

  //if not pointing at our target then move accordingly
  if( angularDiff[ PITCH ] < -MGTURRET_ACCURACYTOLERANCE )
    self->s.angles2[ PITCH ] += MGTURRET_ANGULARSPEED;
  else if( angularDiff[ PITCH ] > MGTURRET_ACCURACYTOLERANCE )
    self->s.angles2[ PITCH ] -= MGTURRET_ANGULARSPEED;
  else
    self->s.angles2[ PITCH ] = angleToTarget[ PITCH ];

  //disallow vertical movement past a certain limit
  temp = fabs( self->s.angles2[ PITCH ] );
  if( temp > 180 )
    temp -= 360;
  
  if( temp < -MGTURRET_VERTICALCAP )
    self->s.angles2[ PITCH ] = (-360) + MGTURRET_VERTICALCAP;
    
  //if not pointing at our target then move accordingly
  if( angularDiff[ YAW ] < -MGTURRET_ACCURACYTOLERANCE )
    self->s.angles2[ YAW ] += MGTURRET_ANGULARSPEED;
  else if( angularDiff[ YAW ] > MGTURRET_ACCURACYTOLERANCE )
    self->s.angles2[ YAW ] -= MGTURRET_ANGULARSPEED;
  else
    self->s.angles2[ YAW ] = angleToTarget[ YAW ];
    
  AngleVectors( self->s.angles2, dttAdjusted, NULL, NULL );
  RotatePointAroundVector( dirToTarget, xNormal, dttAdjusted, -rotAngle );
  vectoangles( dirToTarget, self->turretAim );

  //if pointing at our target return true
  if( abs( angleToTarget[ YAW ] - self->s.angles2[ YAW ] ) <= MGTURRET_ACCURACYTOLERANCE &&
      abs( angleToTarget[ PITCH ] - self->s.angles2[ PITCH ] ) <= MGTURRET_ACCURACYTOLERANCE )
    return qtrue;
    
  return qfalse;
}

/*
================
HTeslaGen_FireOnEnemy

Used by HDef_Think to fire at enemy
================
*/
void HTeslaGen_FireOnEnemy( gentity_t *self, int firespeed )
{
  vec3_t  dirToTarget;
 
  //this doesn't operate without a dcc
  if( !self->dcced )
    return;
  
  VectorSubtract( self->enemy->s.pos.trBase, self->s.pos.trBase, dirToTarget );
  VectorNormalize( dirToTarget );
  vectoangles( dirToTarget, self->turretAim );

  //fire at target
  FireWeapon( self );
  G_setBuildableAnim( self, BANIM_ATTACK1, qfalse );
  self->count = level.time + firespeed;
}

/*
================
HMGTurret_FireOnEnemy

Used by HDef_Think to fire at enemy
================
*/
void HMGTurret_FireOnEnemy( gentity_t *self, int firespeed )
{
  //fire at target
  FireWeapon( self );
  G_setBuildableAnim( self, BANIM_ATTACK1, qfalse );
  self->count = level.time + firespeed;
}

/*
================
HDef_CheckTarget

Used by HDef_Think to check enemies for validity
================
*/
qboolean HDef_CheckTarget( gentity_t *self, gentity_t *target, int range )
{
  vec3_t    distance;
  trace_t   trace;

  if( !target ) // Do we have a target?
    return qfalse;
  if( !target->inuse ) // Does the target still exist?
    return qfalse;
  if( target == self ) // is the target us?
    return qfalse;
  if( !target->client ) // is the target a bot or player?
    return qfalse;
  if( target->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS ) // is the target one of us?
    return qfalse;
  if( target->client->sess.sessionTeam == TEAM_SPECTATOR ) // is the target alive?
    return qfalse;
  if( target->client->ps.stats[ STAT_STATE ] & SS_INFESTING ) // is the target alive?
    return qfalse;
  if( target->client->ps.stats[ STAT_STATE ] & SS_HOVELING ) // is the target alive?
    return qfalse;
  if( target->health <= 0 ) // is the target still alive?
    return qfalse;
  if( self->dcced && target->targeted && target->targeted->powered ) //some turret has already selected this target
    return qfalse;

  VectorSubtract( target->r.currentOrigin, self->r.currentOrigin, distance );
  if( VectorLength( distance ) > range ) // is the target within range?
    return qfalse;

  trap_Trace( &trace, self->s.pos.trBase, NULL, NULL, target->s.pos.trBase, self->s.number, MASK_OPAQUE );
  if( trace.fraction < 1.0 ) // can we see the target?
    return qfalse;

  return qtrue;
}


/*
================
HDef_FindEnemy

Used by HDef_Think to locate enemy gentities
================
*/
void HDef_FindEnemy( gentity_t *ent, int range )
{
  gentity_t *target;

  //iterate through entities
  for( target = g_entities; target < &g_entities[ level.num_entities ]; target++ )
  {
    //if target is not valid keep searching
    if( !HDef_CheckTarget( ent, target, range ) )
      continue;
      
    //we found a target
    ent->enemy = target;
    return;
  }

  //couldn't find a target
  ent->enemy = NULL;
}


/*
================
HDef_Think

think function for Human Defense
================
*/
void HDef_Think( gentity_t *self )
{
  int range =     BG_FindRangeForBuildable( self->s.modelindex );
  int firespeed = BG_FindFireSpeedForBuildable( self->s.modelindex );

  self->nextthink = level.time + BG_FindNextThinkForBuildable( self->s.modelindex );

  //if not powered don't do anything and check again for power next think
  if( !( self->powered = findPower( self ) ) )
  {
    self->nextthink = level.time + REFRESH_TIME;
    return;
  }
  
  //find a dcc for self
  self->dcced = findDCC( self );
  
  //if the current target is not valid find a new one
  if( !HDef_CheckTarget( self, self->enemy, range ) )
  {
    if( self->enemy )
      self->enemy->targeted = NULL;

    HDef_FindEnemy( self, range );
  }

  //if a new target cannot be found don't do anything
  if( !self->enemy )
    return;
  
  self->enemy->targeted = self;

  //if we are pointing at our target and we can fire shoot it
  switch( self->s.modelindex )
  {
    case BA_H_MGTURRET:
      if( HMGTurret_TrackEnemy( self ) && ( self->count < level.time ) )
        HMGTurret_FireOnEnemy( self, firespeed );
      break;
      
    case BA_H_TESLAGEN:
      if( self->count < level.time )
        HTeslaGen_FireOnEnemy( self, firespeed );
      break;

    default:
      Com_Printf( S_COLOR_YELLOW "WARNING: Unknown turret type in think\n" );
      break;
  }
}




//==================================================================================




/*
================
HSpawn_blast

Called when a human spawn explodes
think function
================
*/
void HSpawn_Blast( gentity_t *self )
{
  vec3_t  dir;

  // we don't have a valid direction, so just point straight up
  dir[ 0 ] = dir[ 1 ] = 0;
  dir[ 2 ] = 1;

  self->s.modelindex = 0; //don't draw the model once its destroyed
  G_AddEvent( self, EV_BUILDABLE_EXPLOSION, DirToByte( dir ) );
  self->r.contents = CONTENTS_TRIGGER;
  self->timestamp = level.time;

  //do some radius damage
  G_RadiusDamage( self->s.pos.trBase, self->parent, self->splashDamage,
    self->splashRadius, self, self->splashMethodOfDeath );

  self->think = freeBuildable;
  self->nextthink = level.time + 100;
}


/*
================
HSpawn_die

Called when a human spawn dies
================
*/
void HSpawn_Die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
  //pretty events and cleanup
  G_setBuildableAnim( self, BANIM_DESTROY1, qtrue );
  G_setIdleBuildableAnim( self, BANIM_DESTROYED );
  
  self->die = nullDieFunction;
  self->think = HSpawn_Blast;
  self->nextthink = level.time + HUMAN_DETONATION_DELAY;
  self->powered = qfalse; //free up power

  trap_LinkEntity( self );
}

/*
================
HSpawn_Think

Think for human spawn
================
*/
void HSpawn_Think( gentity_t *self )
{
  vec3_t    mins, maxs, origin;
  gentity_t *ent;
  trace_t   tr;
  vec3_t    up = { 0.0f, 0.0f, 1.0f };

  BG_FindBBoxForClass( PCL_H_BASE, mins, maxs, NULL, NULL, NULL );

  VectorCopy( self->s.origin, origin );
  origin[ 2 ] += self->r.maxs[ 2 ] + fabs( mins[ 2 ] ) + 1.0f;
  
  //make sure we have power
  self->powered = findPower( self );

  //only suicide if at rest
  if( self->s.groundEntityNum )
  {
    trap_Trace( &tr, origin, mins, maxs, origin, self->s.number, MASK_SHOT );
    ent = &g_entities[ tr.entityNum ];
    
    if( ent->s.eType == ET_BUILDABLE || ent->s.number == ENTITYNUM_WORLD )
    {
      G_Damage( self, NULL, NULL, NULL, NULL, 10000, 0, MOD_SUICIDE );
      return;
    }

    if( ent->s.eType == ET_CORPSE )
      G_FreeEntity( ent ); //quietly remove
  }

  self->nextthink = level.time + BG_FindNextThinkForBuildable( self->s.modelindex );
}




//==================================================================================




/*
================
G_itemFits

Checks to see if an item fits in a specific area
================
*/
itemBuildError_t G_itemFits( gentity_t *ent, buildable_t buildable, int distance, vec3_t origin )
{
  vec3_t            forward, angles;
  vec3_t            player_origin, entity_origin, target_origin, normal, cross;
  vec3_t            mins, maxs;
  vec3_t            temp_v;
  trace_t           tr1, tr2, tr3;
  int               i;
  itemBuildError_t  reason = IBE_NONE;
  gentity_t         *tempent, *closestPower;
  int               minDistance = 10000;
  int               templength;

  if( ent->client->ps.stats[ STAT_STATE ] & SS_WALLCLIMBING )
  {
    if( ent->client->ps.stats[ STAT_STATE ] & SS_WALLCLIMBINGCEILING )
      VectorSet( normal, 0.0f, 0.0f, -1.0f );
    else
      VectorCopy( ent->client->ps.grapplePoint, normal );
  }
  else
    VectorSet( normal, 0.0f, 0.0f, 1.0f );
  
  //FIXME: must sync with cg_buildable.c/CG_GhostBuildable ick.
  VectorCopy( ent->s.apos.trBase, angles );

  AngleVectors( angles, forward, NULL, NULL );
  CrossProduct( forward, normal, cross );
  VectorNormalize( cross );
  CrossProduct( normal, cross, forward );
  VectorNormalize( forward );
  
  VectorCopy( ent->s.pos.trBase, player_origin );
  VectorMA( player_origin, distance, forward, entity_origin );

  VectorCopy( entity_origin, target_origin );
  VectorMA( entity_origin, 32, normal, entity_origin );
  VectorMA( target_origin, -128, normal, target_origin );

  BG_FindBBoxForBuildable( buildable, mins, maxs );
  
  trap_Trace( &tr1, entity_origin, mins, maxs, target_origin, ent->s.number, MASK_PLAYERSOLID );
  VectorCopy( tr1.endpos, entity_origin );
  VectorMA( entity_origin, 0.1f, normal, entity_origin );

  trap_Trace( &tr2, entity_origin, mins, maxs, entity_origin, ent->s.number, MASK_PLAYERSOLID );
  trap_Trace( &tr3, player_origin, NULL, NULL, entity_origin, ent->s.number, MASK_PLAYERSOLID );

  VectorCopy( entity_origin, origin );

  //this item does not fit here
  if( tr2.fraction < 1.0 || tr3.fraction < 1.0 )
    return IBE_NOROOM; //NO other reason is allowed to override this
    
  if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS )
  {
    //alien criteria

    float     minNormal = BG_FindMinNormalForBuildable( buildable );
    qboolean  invert = BG_FindInvertNormalForBuildable( buildable );

    //can we build at this angle?
    if( !( normal[ 2 ] >= minNormal || ( invert && normal[ 2 ] <= -minNormal ) ) )
      return IBE_NORMAL;
    
    //check there is creep near by for building on
    
    if( BG_FindCreepTestForBuildable( buildable ) )
    {
      if( !isCreep( entity_origin ) )
        reason = IBE_NOCREEP;
    }
    
    //look for a hivemind
    for ( i = 1, tempent = g_entities + i; i < level.num_entities; i++, tempent++ )
    {
      if( !tempent->classname || tempent->s.eType != ET_BUILDABLE )
        continue;
      if( tempent->s.modelindex == BA_A_HIVEMIND )
        break;
    }

    //if none found...
    if( i >= level.num_entities && buildable != BA_A_HIVEMIND )
    {
      if( buildable == BA_A_SPAWN )
        reason = IBE_SPWNWARN;
      else
        reason = IBE_NOHIVEMIND;
    }
    
    //can we only have one of these?
    if( BG_FindUniqueTestForBuildable( buildable ) )
    {
      for ( i = 1, tempent = g_entities + i; i < level.num_entities; i++, tempent++ )
      {
        if( !tempent->classname || tempent->s.eType != ET_BUILDABLE )
          continue;

        if( tempent->s.modelindex == BA_A_HIVEMIND )
        {
          reason = IBE_HIVEMIND;
          break;
        }
      }
    }

    if( level.alienBuildPoints - BG_FindBuildPointsForBuildable( buildable ) < 0 )
      reason = IBE_NOASSERT;
  }
  else if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
  {
    //human criteria
    if( !isPower( entity_origin ) )
    {
      //tell player to build a repeater to provide power
      if( buildable != BA_H_REACTOR && buildable != BA_H_REPEATER )
        reason = IBE_REPEATER;

      //warn that the current spawn will not be externally powered
      if( buildable == BA_H_SPAWN )
        reason = IBE_RPLWARN;
    }

    //this buildable requires a DCC
    if( BG_FindDCCTestForBuildable( buildable ) && !isDCC( ) )
      reason = IBE_NODCC;
    
    //check that there is a parent reactor when building a repeater
    if( buildable == BA_H_REPEATER )
    {
      for ( i = 1, tempent = g_entities + i; i < level.num_entities; i++, tempent++ )
      {
        if( !tempent->classname || tempent->s.eType != ET_BUILDABLE )
          continue;

        if( tempent->s.modelindex == BA_H_REACTOR ) 
          break;
      }
      
      if( i >= level.num_entities )
        reason = IBE_RPTWARN;
    }
      
    //can we only build one of these?
    if( BG_FindUniqueTestForBuildable( buildable ) )
    {
      for ( i = 1, tempent = g_entities + i; i < level.num_entities; i++, tempent++ )
      {
        if( !tempent->classname || tempent->s.eType != ET_BUILDABLE )
          continue;

        if( tempent->s.modelindex == BA_H_REACTOR ) 
        {
          reason = IBE_REACTOR;
          break;
        }
      }
    }

    if( level.humanBuildPoints - BG_FindBuildPointsForBuildable( buildable ) < 0 )
      reason = IBE_NOPOWER;
  }

  return reason;
}


/*
================
G_buildItem

Spawns a buildable
================
*/
gentity_t *G_buildItem( gentity_t *builder, buildable_t buildable, vec3_t origin, vec3_t angles )
{
  gentity_t *built;
  vec3_t    normal;

  //spawn the buildable
  built = G_Spawn();

  built->s.eType = ET_BUILDABLE;

  built->classname = BG_FindEntityNameForBuildable( buildable );
  
  built->s.modelindex = buildable; //so we can tell what this is on the client side
  built->biteam = built->s.modelindex2 = BG_FindTeamForBuildable( buildable );

  BG_FindBBoxForBuildable( buildable, built->r.mins, built->r.maxs );
  built->health = BG_FindHealthForBuildable( buildable );
  
  built->splashDamage = BG_FindSplashDamageForBuildable( buildable );
  built->splashRadius = BG_FindSplashRadiusForBuildable( buildable );
  built->splashMethodOfDeath = BG_FindMODForBuildable( buildable );
  
  G_setIdleBuildableAnim( built, BG_FindAnimForBuildable( buildable ) );
    
  built->nextthink = BG_FindNextThinkForBuildable( buildable );

  //things that vary for each buildable that aren't in the dbase
  switch( buildable )
  {
    case BA_A_SPAWN:
      built->die = ASpawn_Die;
      built->think = ASpawn_Think;
      built->pain = ASpawn_Pain;
      break;
      
    case BA_A_BARRICADE:
      built->die = ABarricade_Die;
      built->think = ABarricade_Think;
      built->pain = ABarricade_Pain;
      break;
      
    case BA_A_BOOSTER:
      built->die = ABarricade_Die;
      built->think = ABarricade_Think;
      built->pain = ABarricade_Pain;
      built->touch = ABooster_Touch;
      break;
      
    case BA_A_ACIDTUBE:
      built->die = ABarricade_Die;
      built->think = AAcidTube_Think;
      built->pain = ASpawn_Pain;
      break;
      
    case BA_A_TRAPPER:
      built->die = ABarricade_Die;
      built->think = ATrapper_Think;
      built->pain = ASpawn_Pain;
      break;
      
    case BA_A_HIVEMIND:
      built->die = ASpawn_Die;
      built->think = AOvermind_Think;
      built->pain = ASpawn_Pain;
      break;
      
    case BA_A_HOVEL:
      built->die = AHovel_Die;
      built->use = AHovel_Use;
      built->think = AHovel_Think;
      built->pain = ASpawn_Pain;
      break;
      
    case BA_A_OBANK:
      built->think = ABarricade_Think;
      built->die = ASpawn_Die;
      built->use = ABank_Activate;
      break;
      
    case BA_H_SPAWN:
      built->die = HSpawn_Die;
      built->think = HSpawn_Think;
      break;
      
    case BA_H_MGTURRET:
    case BA_H_TESLAGEN:
      built->die = HSpawn_Die;
      built->think = HDef_Think;
      break;
      
    case BA_H_ARMOURY:
      built->think = HArmoury_Think;
      built->die = HSpawn_Die;
      built->use = HArmoury_Activate;
      break;
      
    case BA_H_BANK:
      built->think = HBank_Think;
      built->die = HSpawn_Die;
      built->use = HBank_Activate;
      break;
      
    case BA_H_DCC:
      built->think = HDCC_Think;
      built->die = HSpawn_Die;
      break;
      
    case BA_H_MEDISTAT:
      built->think = HMedistat_Think;
      built->die = HSpawn_Die;
      break;
      
    case BA_H_ADVMEDISTAT:
      built->think = HMedistat_Think;
      built->die = HSpawn_Die;
      break;
      
    case BA_H_REACTOR:
      built->die = HSpawn_Die;
      built->use = HRpt_Use;
      built->powered = built->active = qtrue;
      break;
      
    case BA_H_REPEATER:
      built->think = HRpt_Think;
      built->die = HSpawn_Die;
      built->use = HRpt_Use;
      break;
      
    case BA_H_FLOATMINE:
      built->think = HFM_Think;
      built->die = HFM_Die;
      built->touch = HFM_Touch;
      break;
      
    default:
      //erk
      break;
  }

  built->takedamage = qtrue;
  built->s.number = built - g_entities;
  built->r.contents = CONTENTS_BODY;
  built->clipmask = MASK_PLAYERSOLID;
  built->enemy = NULL;
  built->s.weapon = BG_FindProjTypeForBuildable( buildable );

  if( builder->client )
    built->builtBy = builder->client->ps.clientNum;
  else
    built->builtBy = -1;

  G_SetOrigin( built, origin );
  VectorCopy( angles, built->s.angles );
  built->s.angles[ PITCH ] = 0.0f;
  built->s.angles2[ YAW ] = angles[ YAW ];
  built->s.pos.trType = BG_FindTrajectoryForBuildable( buildable );
  built->s.pos.trTime = level.time;
  built->physicsBounce = BG_FindBounceForBuildable( buildable );
  built->s.groundEntityNum = -1;
  
  if( builder->client && builder->client->ps.stats[ STAT_STATE ] & SS_WALLCLIMBING )
  {
    if( builder->client->ps.stats[ STAT_STATE ] & SS_WALLCLIMBINGCEILING )
      VectorSet( normal, 0.0f, 0.0f, -1.0f );
    else
      VectorCopy( builder->client->ps.grapplePoint, normal );
  
    //gently nudge the buildable onto the surface :)
    VectorScale( normal, -50.0f, built->s.pos.trDelta );
  }
  else
    VectorSet( normal, 0.0f, 0.0f, 1.0f );

  built->s.generic1 = (int)( ( (float)built->health /
        (float)BG_FindHealthForBuildable( buildable ) ) * 63.0f );

  if( built->s.generic1 < 0 )
    built->s.generic1 = 0;
    
  if( built->powered = findPower( built ) )
    built->s.generic1 |= B_POWERED_TOGGLEBIT;
  
  if( built->dcced  = findDCC( built ) )
    built->s.generic1 |= B_DCCED_TOGGLEBIT;
    
  VectorCopy( normal, built->s.origin2 );
  
  G_AddEvent( built, EV_BUILD_CONSTRUCT, BANIM_CONSTRUCT1 );

  trap_LinkEntity( built );

  return built;
}

/*
=================
G_ValidateBuild
=================
*/
qboolean G_ValidateBuild( gentity_t *ent, buildable_t buildable )
{
  weapon_t      weapon;
  float         dist;
  vec3_t        origin;

  dist = BG_FindBuildDistForClass( ent->client->ps.stats[ STAT_PCLASS ] );
    
  switch( G_itemFits( ent, buildable, dist, origin ) )
  {
    case IBE_NONE:
      G_buildItem( ent, buildable, origin, ent->s.apos.trBase );
      return qtrue;

    case IBE_NOASSERT:
      G_AddPredictableEvent( ent, EV_MENU, MN_A_NOASSERT );
      return qfalse;

    case IBE_NOHIVEMIND:
      G_AddPredictableEvent( ent, EV_MENU, MN_A_NOHVMND );
      return qfalse;

    case IBE_HIVEMIND:
      G_AddPredictableEvent( ent, EV_MENU, MN_A_HIVEMIND );
      return qfalse;

    case IBE_NORMAL:
      G_AddPredictableEvent( ent, EV_MENU, MN_A_NORMAL );
      return qfalse;

    case IBE_REACTOR:
      G_AddPredictableEvent( ent, EV_MENU, MN_H_REACTOR );
      return qfalse;

    case IBE_REPEATER:
      G_AddPredictableEvent( ent, EV_MENU, MN_H_REPEATER );
      return qfalse;

    case IBE_NOROOM:
      if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
        G_AddPredictableEvent( ent, EV_MENU, MN_H_NOROOM );
      else
        G_AddPredictableEvent( ent, EV_MENU, MN_A_NOROOM );
      return qfalse;

    case IBE_NOPOWER:
      G_AddPredictableEvent( ent, EV_MENU, MN_H_NOPOWER );
      return qfalse;
      
    case IBE_NODCC:
      G_AddPredictableEvent( ent, EV_MENU, MN_H_NODCC );
      return qfalse;
      
    case IBE_SPWNWARN:
      G_AddPredictableEvent( ent, EV_MENU, MN_A_SPWNWARN );
      G_buildItem( ent, buildable, origin, ent->s.apos.trBase );
      return qtrue;
      
    case IBE_RPLWARN:
      G_AddPredictableEvent( ent, EV_MENU, MN_H_RPLWARN );
      G_buildItem( ent, buildable, origin, ent->s.apos.trBase );
      return qtrue;
      
    case IBE_RPTWARN:
      G_AddPredictableEvent( ent, EV_MENU, MN_H_RPTWARN );
      G_buildItem( ent, buildable, origin, ent->s.apos.trBase );
      return qtrue;
  }

  return qfalse;
}

/*
================
FinishSpawningBuildable

Traces down to find where an item should rest, instead of letting them
free fall from their spawn points
================
*/
void FinishSpawningBuildable( gentity_t *ent )
{
  trace_t     tr;
  vec3_t      dest;
  gentity_t   *built;
  buildable_t buildable = ent->s.modelindex;

  built = G_buildItem( ent, buildable, ent->s.pos.trBase, ent->s.angles );
  G_FreeEntity( ent );

  // drop to floor
  if( buildable != BA_NONE && BG_FindTrajectoryForBuildable( buildable ) == TR_BUOYANCY )
    VectorSet( dest, built->s.origin[0], built->s.origin[1], built->s.origin[2] + 4096 );
  else
    VectorSet( dest, built->s.origin[0], built->s.origin[1], built->s.origin[2] - 4096 );

  trap_Trace( &tr, built->s.origin, built->r.mins, built->r.maxs, dest, built->s.number, built->clipmask );
  if( tr.startsolid )
  {
    G_Printf ("FinishSpawningBuildable: %s startsolid at %s\n", built->classname, vtos(built->s.origin));
    G_FreeEntity( built );
    return;
  }

  //point items in the correct direction
  VectorCopy( tr.plane.normal, built->s.origin2 );

  // allow to ride movers
  built->s.groundEntityNum = tr.entityNum;

  G_SetOrigin( built, tr.endpos );

  trap_LinkEntity( built );
}

/*
============
G_SpawnBuildable

Sets the clipping size and plants the object on the floor.

Items can't be immediately dropped to floor, because they might
be on an entity that hasn't spawned yet.
============
*/
void G_SpawnBuildable( gentity_t *ent, buildable_t buildable )
{
  ent->s.modelindex = buildable;

  // some movers spawn on the second frame, so delay item
  // spawns until the third frame so they can ride trains
  ent->nextthink = level.time + FRAMETIME * 2;
  ent->think = FinishSpawningBuildable;
}
