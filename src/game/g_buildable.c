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
qboolean findPower( gentity_t *self )
{
  int       i;
  gentity_t *ent;
  gentity_t *closestPower;
  int       distance = 0;
  int       minDistance = 10000;
  vec3_t    temp_v;
  qboolean  foundPower = qfalse;

  //if this already has power then stop now
  if( self->parentNode && self->parentNode->active )
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
      if( distance < minDistance && ( ent->active || self->s.modelindex == BA_H_SPAWN ) )
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
      ( closestPower->s.modelindex == BA_H_REPEATER && self->s.modelindex == BA_H_SPAWN &&
        ( minDistance <= REPEATER_BASESIZE ) && closestPower->powered ) ||
      ( closestPower->s.modelindex == BA_H_REPEATER && ( minDistance <= REPEATER_BASESIZE ) &&
        closestPower->active && closestPower->powered )
    )
  {
    self->parentNode = closestPower;

    return qtrue;
  }
  else
    return qfalse;
}

/*
================
findDCC

attempt to find a controlling DCC for self, return qtrue if successful
================
*/
qboolean findDCC( gentity_t *self )
{
  int       i;
  gentity_t *ent;
  gentity_t *closestDCC;
  int       distance = 0;
  int       minDistance = 10000;
  vec3_t    temp_v;
  qboolean  foundDCC = qfalse;

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
findCreep

attempt to find creep for self, return qtrue if successful
================
*/
qboolean findCreep( gentity_t *self )
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
        /*VectorSubtract( self->s.origin, ent->s.origin, temp_v );*/
        VectorSubtract( self->s.origin, ent->s.origin, temp_v );
        distance = VectorLength( temp_v );
        if( distance < minDistance )
        {
          closestSpawn = ent;
          minDistance = distance;
        }
      }
    }
    
    if( minDistance <= ( CREEP_BASESIZE * 3 ) )
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
  dir[0] = dir[1] = 0;
  dir[2] = 1;

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
ABarricade_Pain

pain function for Alien Spawn
================
*/
void ABarricade_Pain( gentity_t *self, gentity_t *attacker, int damage )
{
  if( random() > 0.5f )
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
  dir[0] = dir[1] = 0;
  dir[2] = 1;

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
    
  if( ( self->timestamp + 10000 ) > level.time )
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
  vec3_t    range = { 200, 200, 200 };
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
  dir[0] = dir[1] = 0;
  dir[2] = 1;

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
ABooster_Touch

Called when an alien touches a booster
================
*/
void ABooster_Touch( gentity_t *self, gentity_t *other, trace_t *trace )
{
  if( !( other->client->ps.stats[ STAT_STATE ] & SS_BOOSTED ) )
  {
    other->client->ps.stats[ STAT_STATE ] |= SS_BOOSTED;
    other->client->lastBoostedTime = level.time;
  }
}




//==================================================================================


#define BLOB_PROJSPEED  500
#define MIN_DOT         0.85f // max angle = acos( MIN_DOT )

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
  for( i = 0; ( i * BLOB_PROJSPEED ) / 1000.0f < distanceToTarget; i++ )
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
  if( DotProduct( distance, self->s.origin2 ) < MIN_DOT )
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
  int       count = 0;
  qboolean  reactor = qfalse;
  gentity_t *ent;
  
  //iterate through entities
  for ( i = 1, ent = g_entities + i; i < level.num_entities; i++, ent++ )
  {
    if( !ent->classname || ent->s.eType != ET_BUILDABLE )
      continue;

    if( ent->s.modelindex == BA_H_SPAWN && ent->parentNode == self )
      count++;
      
    if( ent->s.modelindex == BA_H_REACTOR )
      reactor = qtrue;
  }
  
  //if repeater has children and there is a reactor then this is active
  if( count && reactor )
    self->active = qtrue;
  else
    self->active = qfalse;

  self->powered = reactor;

  self->nextthink = level.time + REFRESH_TIME;
}




//==================================================================================



/*
================
HMCU_Activate

Called when a human activates an MCU
================
*/
void HMCU_Activate( gentity_t *self, gentity_t *other, gentity_t *activator )
{
  //only humans can activate this
  if( activator->client->ps.stats[ STAT_PTEAM ] != PTE_HUMANS ) return;
  
  //if this is powered then call the mcu menu
  if( self->powered )
    G_AddPredictableEvent( activator, EV_MENU, MN_H_MCU );
  else
    G_AddPredictableEvent( activator, EV_MENU, MN_H_NOPOWER );
}

/*
================
HMCU_Think

Think for mcu
================
*/
void HMCU_Think( gentity_t *self )
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
  
  //if this is powered then call the mcu menu
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



#define MAX_HEAL_CLIENTS  1

/*
================
HMedistat_Think

think function for Alien Acid Tube
================
*/
void HMedistat_Think( gentity_t *self )
{
  int       entityList[ MAX_GENTITIES ];
  vec3_t    mins, maxs;
  int       i, num;
  gentity_t *player;
  int       healCount = 0;

  VectorAdd( self->s.origin, self->r.maxs, maxs );
  VectorAdd( self->s.origin, self->r.mins, mins );

  mins[ 2 ] += fabs( self->r.mins[ 2 ] ) + self->r.maxs[ 2 ];
  maxs[ 2 ] += 60; //player height
  
  //make sure we have power
  self->powered = findPower( self );

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
      if( player->health < player->client->ps.stats[ STAT_MAX_HEALTH ] && healCount < MAX_HEAL_CLIENTS )
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
  dir[0] = dir[1] = 0;
  dir[2] = -1;

  //do a bit of radius damage
  G_RadiusDamage( self->s.pos.trBase, self->parent, self->splashDamage,
    self->splashRadius, self, self->splashMethodOfDeath );

  //pretty events and item cleanup
  self->s.modelindex = 0; //don't draw the model once its destroyed
  G_AddEvent( self, EV_ITEM_EXPLOSION, DirToByte( dir ) );
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




//TA: the following defense turret code was written by
// "fuzzysteve"           (fuzzysteve@quakefiles.com) and
// Anthony "inolen" Pesch (www.inolen.com)
//with (heavy) modifications by me of course :)
  
#define HDEF1_ANGULARSPEED      10  //degrees/think ~= 200deg/sec
#define HDEF1_ACCURACYTOLERANCE HDEF1_ANGULARSPEED / 2 //angular difference for turret to fire
#define HDEF1_VERTICALCAP       20 //+/- maximum pitch
#define HDEF1_PROJSPEED         2000.0f //speed of projectile (used in prediction)

/*
================
hdef1_trackenemy

Used by HDef1_Think to track enemy location
================
*/
qboolean hdef1_trackenemy( gentity_t *self )
{
  vec3_t  dirToTarget, dttAdjusted, angleToTarget, angularDiff, xNormal;
  vec3_t  refNormal = { 0.0f, 0.0f, 1.0f };
  float   temp, rotAngle;
  float   distanceToTarget = BG_FindRangeForBuildable( self->s.modelindex );
  float   timeTilImpact;
  vec3_t  halfAcceleration;
  vec3_t  thirdJerk;
  int     i;

  VectorSubtract( self->enemy->s.pos.trBase, self->s.pos.trBase, dirToTarget );

//lead targets
#if 0
  distanceToTarget = VectorLength( dirToTarget );
  timeTilImpact = distanceToTarget / 2000.0f;
  VectorMA( self->enemy->s.pos.trBase, timeTilImpact, self->enemy->s.pos.trDelta, dirToTarget );
  VectorSubtract( dirToTarget, self->s.pos.trBase, dirToTarget );
#endif

  //better, but more expensive method
  if( self->dcced )
  {
    VectorScale( self->enemy->acceleration, 1.0f / 2.0f, halfAcceleration );
    VectorScale( self->enemy->jerk, 1.0f / 3.0f, thirdJerk );

    //O( time ) - worst case O( time ) = 250 iterations
    for( i = 0; ( i * HDEF1_PROJSPEED ) / 1000.0f < distanceToTarget; i++ )
    {
      float time = (float)i / 1000.0f;

      VectorMA( self->enemy->s.pos.trBase, time, self->enemy->s.pos.trDelta, dirToTarget );
      VectorMA( dirToTarget, time * time, halfAcceleration, dirToTarget );
      VectorMA( dirToTarget, time * time * time, thirdJerk, dirToTarget );
      VectorSubtract( dirToTarget, self->s.pos.trBase, dirToTarget );
      distanceToTarget = VectorLength( dirToTarget );

      distanceToTarget -= self->enemy->r.maxs[ 0 ];
    }
  }
  
  VectorNormalize( dirToTarget );
  
  CrossProduct( self->s.origin2, refNormal, xNormal );
  VectorNormalize( xNormal );
  rotAngle = RAD2DEG( acos( DotProduct( self->s.origin2, refNormal ) ) );
  RotatePointAroundVector( dttAdjusted, xNormal, dirToTarget, rotAngle );
  
  vectoangles( dttAdjusted, angleToTarget );

  angularDiff[ PITCH ] = AngleSubtract( self->s.angles2[ PITCH ], angleToTarget[ PITCH ] );
  angularDiff[ YAW ] = AngleSubtract( self->s.angles2[ YAW ], angleToTarget[ YAW ] );

  //if not pointing at our target then move accordingly
  if( angularDiff[ PITCH ] < -HDEF1_ACCURACYTOLERANCE )
    self->s.angles2[ PITCH ] += HDEF1_ANGULARSPEED;
  else if( angularDiff[ PITCH ] > HDEF1_ACCURACYTOLERANCE )
    self->s.angles2[ PITCH ] -= HDEF1_ANGULARSPEED;
  else
    self->s.angles2[ PITCH ] = angleToTarget[ PITCH ];

  //disallow vertical movement past a certain limit
  temp = fabs( self->s.angles2[ PITCH ] );
  if( temp > 180 )
    temp -= 360;
  
  if( temp < -HDEF1_VERTICALCAP )
    self->s.angles2[ PITCH ] = (-360)+HDEF1_VERTICALCAP;
  else if( temp > HDEF1_VERTICALCAP )
    self->s.angles2[ PITCH ] = -HDEF1_VERTICALCAP;
    
  //if not pointing at our target then move accordingly
  if( angularDiff[ YAW ] < -HDEF1_ACCURACYTOLERANCE )
    self->s.angles2[ YAW ] += HDEF1_ANGULARSPEED;
  else if( angularDiff[ YAW ] > HDEF1_ACCURACYTOLERANCE )
    self->s.angles2[ YAW ] -= HDEF1_ANGULARSPEED;
  else
    self->s.angles2[ YAW ] = angleToTarget[ YAW ];
    
  AngleVectors( self->s.angles2, dttAdjusted, NULL, NULL );
  RotatePointAroundVector( dirToTarget, xNormal, dttAdjusted, -rotAngle );
  vectoangles( dirToTarget, self->turretAim );

  //if pointing at our target return true
  if( abs( angleToTarget[ YAW ] - self->s.angles2[ YAW ] ) <= HDEF1_ACCURACYTOLERANCE &&
      abs( angleToTarget[ PITCH ] - self->s.angles2[ PITCH ] ) <= HDEF1_ACCURACYTOLERANCE )
    return qtrue;
    
  return qfalse;
}

#define HDEF2_ANGULARSPEED      20  //degrees/think ~= 200deg/sec
#define HDEF2_ACCURACYTOLERANCE HDEF2_ANGULARSPEED / 2 //angular difference for turret to fire
#define HDEF2_VERTICALCAP       30  //- maximum pitch

/*
================
hdef2_trackenemy

Used by HDef1_Think to track enemy location
================
*/
qboolean hdef2_trackenemy( gentity_t *self )
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
  if( angularDiff[ PITCH ] < -HDEF2_ACCURACYTOLERANCE )
    self->s.angles2[ PITCH ] += HDEF2_ANGULARSPEED;
  else if( angularDiff[ PITCH ] > HDEF2_ACCURACYTOLERANCE )
    self->s.angles2[ PITCH ] -= HDEF2_ANGULARSPEED;
  else
    self->s.angles2[ PITCH ] = angleToTarget[ PITCH ];

  //disallow vertical movement past a certain limit
  temp = fabs( self->s.angles2[ PITCH ] );
  if( temp > 180 )
    temp -= 360;
  
  if( temp < -HDEF2_VERTICALCAP )
    self->s.angles2[ PITCH ] = (-360)+HDEF2_VERTICALCAP;
    
  //if not pointing at our target then move accordingly
  if( angularDiff[ YAW ] < -HDEF2_ACCURACYTOLERANCE )
    self->s.angles2[ YAW ] += HDEF2_ANGULARSPEED;
  else if( angularDiff[ YAW ] > HDEF2_ACCURACYTOLERANCE )
    self->s.angles2[ YAW ] -= HDEF2_ANGULARSPEED;
  else
    self->s.angles2[ YAW ] = angleToTarget[ YAW ];
    
  AngleVectors( self->s.angles2, dttAdjusted, NULL, NULL );
  RotatePointAroundVector( dirToTarget, xNormal, dttAdjusted, -rotAngle );
  vectoangles( dirToTarget, self->turretAim );

  //if pointing at our target return true
  if( abs( angleToTarget[ YAW ] - self->s.angles2[ YAW ] ) <= HDEF2_ACCURACYTOLERANCE &&
      abs( angleToTarget[ PITCH ] - self->s.angles2[ PITCH ] ) <= HDEF2_ACCURACYTOLERANCE )
    return qtrue;
    
  return qfalse;
}

/*
================
hdef3_fireonemeny

Used by HDef_Think to fire at enemy
================
*/
void hdef3_fireonenemy( gentity_t *self, int firespeed )
{
  vec3_t  dirToTarget;
 
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
hdef_fireonemeny

Used by HDef_Think to fire at enemy
================
*/
void hdef_fireonenemy( gentity_t *self, int firespeed )
{
  //fire at target
  FireWeapon( self );
  G_setBuildableAnim( self, BANIM_ATTACK1, qfalse );
  self->count = level.time + firespeed;
}

/*
================
hdef_checktarget

Used by HDef_Think to check enemies for validity
================
*/
qboolean hdef_checktarget( gentity_t *self, gentity_t *target, int range )
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
hdef_findenemy

Used by HDef_Think to locate enemy gentities
================
*/
void hdef_findenemy( gentity_t *ent, int range )
{
  gentity_t *target;

  //iterate through entities
  for( target = g_entities; target < &g_entities[ level.num_entities ]; target++ )
  {
    //if target is not valid keep searching
    if( !hdef_checktarget( ent, target, range ) )
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

  //find power for self
  self->powered = findPower( self );

  //if not powered don't do anything and check again for power next think
  if( !self->powered )
  {
    self->nextthink = level.time + REFRESH_TIME;
    return;
  }
  
  //find a dcc for self
  self->dcced = findDCC( self );
  
  //if the current target is not valid find a new one
  if( !hdef_checktarget( self, self->enemy, range ) )
  {
    if( self->enemy )
      self->enemy->targeted = NULL;

    hdef_findenemy( self, range );
  }

  //if a new target cannot be found don't do anything
  if( !self->enemy )
    return;
  
  self->enemy->targeted = self;

  //if we are pointing at our target and we can fire shoot it
  switch( self->s.modelindex )
  {
    case BA_H_DEF1:
      if( hdef1_trackenemy( self ) && ( self->count < level.time ) )
        hdef_fireonenemy( self, firespeed );
      break;
      
    case BA_H_DEF2:
      if( hdef2_trackenemy( self ) && ( self->count < level.time ) )
        hdef_fireonenemy( self, firespeed );
      break;
      
    case BA_H_DEF3:
      if( self->count < level.time )
        hdef3_fireonenemy( self, firespeed );
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
  dir[0] = dir[1] = 0;
  dir[2] = 1;

  self->s.modelindex = 0; //don't draw the model once its destroyed
  G_AddEvent( self, EV_ITEM_EXPLOSION, DirToByte( dir ) );
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
  self->nextthink = level.time + 5000; //wait 1.5 seconds before damaging others
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
  vec3_t    up = { 0, 0, 1 };

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

    closestPower = g_entities + 1; //FIXME
    
    //find the nearest power entity
    for ( i = 1, tempent = g_entities + i; i < level.num_entities; i++, tempent++ )
    {
      if( !tempent->classname || tempent->s.eType != ET_BUILDABLE )
        continue;
        
      if( tempent->s.modelindex == BA_H_REACTOR || tempent->s.modelindex == BA_H_REPEATER )
      {
        VectorSubtract( entity_origin, tempent->s.origin, temp_v );
        templength = VectorLength( temp_v );
        if( templength < minDistance && ( tempent->active || buildable == BA_H_SPAWN ) )
        {
          closestPower = tempent;
          minDistance = templength;
        }
      }
    }
    
    //if this power entity satisfies expression
    if( !(
           ( closestPower->s.modelindex == BA_H_REACTOR && minDistance <= REACTOR_BASESIZE ) || 
           (
             closestPower->s.modelindex == BA_H_REPEATER && minDistance <= REPEATER_BASESIZE &&
             (
               ( buildable == BA_H_SPAWN && closestPower->powered ) ||
               ( closestPower->powered && closestPower->active )
             )
           )
         )
      )
    {
      //tell player to build a repeater to provide power
      if( buildable != BA_H_REACTOR && buildable != BA_H_REPEATER )
        reason = IBE_REPEATER;

      //warn that the current spawn will not be externally powered
      if( buildable == BA_H_SPAWN )
        reason = IBE_RPLWARN;
    }

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
  
  built->damage = BG_FindDamageForBuildable( buildable );
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
      built->pain = ASpawn_Pain;
      break;
      
    case BA_A_HOVEL:
      built->die = AHovel_Die;
      built->use = AHovel_Use;
      built->think = AHovel_Think;
      built->pain = ASpawn_Pain;
      break;
      
    case BA_H_SPAWN:
      built->die = HSpawn_Die;
      built->think = HSpawn_Think;
      break;
      
    case BA_H_DEF1:
    case BA_H_DEF2:
    case BA_H_DEF3:
      built->die = HSpawn_Die;
      built->think = HDef_Think;
      break;
      
    case BA_H_MCU:
      built->think = HMCU_Think;
      built->die = HSpawn_Die;
      built->use = HMCU_Activate;
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
      
    case BA_H_REACTOR:
      built->die = HSpawn_Die;
      built->powered = built->active = qtrue;
      break;
      
    case BA_H_REPEATER:
      built->think = HRpt_Think;
      built->die = HSpawn_Die;
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
  built->physicsBounce = BG_FindBounceForBuildable( buildable );
  built->s.groundEntityNum = -1;
  built->s.pos.trTime = level.time;
  
  if( builder->client->ps.stats[ STAT_STATE ] & SS_WALLCLIMBING )
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
void G_ValidateBuild( gentity_t *ent, buildable_t buildable )
{
  weapon_t      weapon;
  float         dist;
  vec3_t        origin;

  dist = BG_FindBuildDistForClass( ent->client->ps.stats[ STAT_PCLASS ] );
    
  switch( G_itemFits( ent, buildable, dist, origin ) )
  {
    case IBE_NONE:
      G_buildItem( ent, buildable, origin, ent->s.apos.trBase );
      break;

    case IBE_NOASSERT:
      G_AddPredictableEvent( ent, EV_MENU, MN_A_NOASSERT );
      break;

    case IBE_NOHIVEMIND:
      G_AddPredictableEvent( ent, EV_MENU, MN_A_NOHVMND );
      break;

    case IBE_HIVEMIND:
      G_AddPredictableEvent( ent, EV_MENU, MN_A_HIVEMIND );
      break;

    case IBE_NORMAL:
      G_AddPredictableEvent( ent, EV_MENU, MN_A_NORMAL );
      break;

    case IBE_REACTOR:
      G_AddPredictableEvent( ent, EV_MENU, MN_H_REACTOR );
      break;

    case IBE_REPEATER:
      G_AddPredictableEvent( ent, EV_MENU, MN_H_REPEATER );
      break;

    case IBE_NOROOM:
      if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
        G_AddPredictableEvent( ent, EV_MENU, MN_H_NOROOM );
      else
        G_AddPredictableEvent( ent, EV_MENU, MN_A_NOROOM );
      break;

    case IBE_NOPOWER:
      G_AddPredictableEvent( ent, EV_MENU, MN_H_NOPOWER );
      break;
      
    case IBE_SPWNWARN:
      G_AddPredictableEvent( ent, EV_MENU, MN_A_SPWNWARN );
      G_buildItem( ent, buildable, origin, ent->s.apos.trBase );
      break;
      
    case IBE_RPLWARN:
      G_AddPredictableEvent( ent, EV_MENU, MN_H_RPLWARN );
      G_buildItem( ent, buildable, origin, ent->s.apos.trBase );
      break;
      
    case IBE_RPTWARN:
      G_AddPredictableEvent( ent, EV_MENU, MN_H_RPTWARN );
      G_buildItem( ent, buildable, origin, ent->s.apos.trBase );
      break;
  }
}
