/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2006 Tim Angus

This file is part of Tremulous.

Tremulous is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Tremulous is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tremulous; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "g_local.h"

// from g_combat.c
extern char *modNames[ ];

/*
================
G_SetBuildableAnim

Triggers an animation client side
================
*/
void G_SetBuildableAnim( gentity_t *ent, buildableAnimNumber_t anim, qboolean force )
{
  int localAnim = anim;

  if( force )
    localAnim |= ANIM_FORCEBIT;

  localAnim |= ( ( ent->s.legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT );

  ent->s.legsAnim = localAnim;
}

/*
================
G_SetIdleBuildableAnim

Set the animation to use whilst no other animations are running
================
*/
void G_SetIdleBuildableAnim( gentity_t *ent, buildableAnimNumber_t anim )
{
  ent->s.torsoAnim = anim;
}

/*
===============
G_CheckSpawnPoint

Check if a spawn at a specified point is valid
===============
*/
gentity_t *G_CheckSpawnPoint( int spawnNum, vec3_t origin, vec3_t normal,
    buildable_t spawn, vec3_t spawnOrigin )
{
  float   displacement;
  vec3_t  mins, maxs;
  vec3_t  cmins, cmaxs;
  vec3_t  localOrigin;
  trace_t tr;

  BG_FindBBoxForBuildable( spawn, mins, maxs );

  if( spawn == BA_A_SPAWN )
  {
    VectorSet( cmins, -MAX_ALIEN_BBOX, -MAX_ALIEN_BBOX, -MAX_ALIEN_BBOX );
    VectorSet( cmaxs,  MAX_ALIEN_BBOX,  MAX_ALIEN_BBOX,  MAX_ALIEN_BBOX );

    displacement = ( maxs[ 2 ] + MAX_ALIEN_BBOX ) * M_ROOT3;
    VectorMA( origin, displacement, normal, localOrigin );

    trap_Trace( &tr, origin, NULL, NULL, localOrigin, spawnNum, MASK_SHOT );

    if( tr.entityNum != ENTITYNUM_NONE )
      return &g_entities[ tr.entityNum ];

    trap_Trace( &tr, localOrigin, cmins, cmaxs, localOrigin, -1, MASK_PLAYERSOLID );

    if( tr.entityNum == ENTITYNUM_NONE )
    {
      if( spawnOrigin != NULL )
        VectorCopy( localOrigin, spawnOrigin );

      return NULL;
    }
    else
      return &g_entities[ tr.entityNum ];
  }
  else if( spawn == BA_H_SPAWN )
  {
    BG_FindBBoxForClass( PCL_HUMAN, cmins, cmaxs, NULL, NULL, NULL );

    VectorCopy( origin, localOrigin );
    localOrigin[ 2 ] += maxs[ 2 ] + fabs( cmins[ 2 ] ) + 1.0f;

    trap_Trace( &tr, origin, NULL, NULL, localOrigin, spawnNum, MASK_SHOT );

    if( tr.entityNum != ENTITYNUM_NONE )
      return &g_entities[ tr.entityNum ];

    trap_Trace( &tr, localOrigin, cmins, cmaxs, localOrigin, -1, MASK_PLAYERSOLID );

    if( tr.entityNum == ENTITYNUM_NONE )
    {
      if( spawnOrigin != NULL )
        VectorCopy( localOrigin, spawnOrigin );

      return NULL;
    }
    else
      return &g_entities[ tr.entityNum ];
  }

  return NULL;
}

/*
================
G_NumberOfDependants

Return number of entities that depend on this one
================
*/
static int G_NumberOfDependants( gentity_t *self )
{
  int       i, n = 0;
  gentity_t *ent;

  for ( i = 1, ent = g_entities + i; i < level.num_entities; i++, ent++ )
  {
    if( ent->s.eType != ET_BUILDABLE )
      continue;

    if( ent->parentNode == self )
      n++;
  }

  return n;
}

#define POWER_REFRESH_TIME  2000

/*
================
G_FindPower

attempt to find power for self, return qtrue if successful
================
*/
static qboolean G_FindPower( gentity_t *self )
{
  int       i;
  gentity_t *ent;
  gentity_t *closestPower = NULL;
  int       distance = 0;
  int       minDistance = 10000;
  vec3_t    temp_v;

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
    if( ent->s.eType != ET_BUILDABLE )
      continue;

    //if entity is a power item calculate the distance to it
    if( ( ent->s.modelindex == BA_H_REACTOR || ent->s.modelindex == BA_H_REPEATER ) &&
        ent->spawned )
    {
      VectorSubtract( self->s.origin, ent->s.origin, temp_v );
      distance = VectorLength( temp_v );

      if( distance < minDistance && ent->powered &&
          ( ( ent->s.modelindex == BA_H_REACTOR &&
            distance <= REACTOR_BASESIZE ) ||
          ( ent->s.modelindex == BA_H_REPEATER &&
            distance <= REPEATER_BASESIZE ) ) ) {

          closestPower = ent;
          minDistance = distance;
      }
    }
  }

  //if there were no power items nearby give up
  if( closestPower ) {
    self->parentNode = closestPower;
    return qtrue;
  }
  else
    return qfalse;
}

/*
================
G_IsPowered

Simple wrapper to G_FindPower to check if a location has power
================
*/
qboolean G_IsPowered( vec3_t origin )
{
  gentity_t dummy;

  dummy.parentNode = NULL;
  dummy.biteam = BIT_HUMANS;
  dummy.s.modelindex = BA_NONE;
  VectorCopy( origin, dummy.s.origin );

  return G_FindPower( &dummy );
}

/*
================
G_FindDCC

attempt to find a controlling DCC for self, return qtrue if successful
================
*/
static qboolean G_FindDCC( gentity_t *self )
{
  int       i;
  gentity_t *ent;
  gentity_t *closestDCC = NULL;
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
    if( ent->s.eType != ET_BUILDABLE )
      continue;

    //if entity is a dcc calculate the distance to it
    if( ent->s.modelindex == BA_H_DCC && ent->spawned )
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

  //if there was no nearby DCC give up
  if( !foundDCC )
    return qfalse;

  self->dccNode = closestDCC;

  return qtrue;
}

/*
================
G_IsDCCBuilt

simple wrapper to G_FindDCC to check for a dcc
================
*/
qboolean G_IsDCCBuilt( void )
{
  gentity_t dummy;

  memset( &dummy, 0, sizeof( gentity_t ) );

  dummy.dccNode = NULL;
  dummy.biteam = BIT_HUMANS;

  return G_FindDCC( &dummy );
}

/*
================
G_FindOvermind

Attempt to find an overmind for self
================
*/
static qboolean G_FindOvermind( gentity_t *self )
{
  int       i;
  gentity_t *ent;

  if( self->biteam != BIT_ALIENS )
    return qfalse;

  //if this already has overmind then stop now
  if( self->overmindNode && self->overmindNode->health > 0 )
    return qtrue;

  //reset parent
  self->overmindNode = NULL;

  //iterate through entities
  for( i = 1, ent = g_entities + i; i < level.num_entities; i++, ent++ )
  {
    if( ent->s.eType != ET_BUILDABLE )
      continue;

    //if entity is an overmind calculate the distance to it
    if( ent->s.modelindex == BA_A_OVERMIND && ent->spawned && ent->health > 0 )
    {
      self->overmindNode = ent;
      return qtrue;
    }
  }

  return qfalse;
}

/*
================
G_IsOvermindBuilt

Simple wrapper to G_FindOvermind to check if a location has an overmind
================
*/
qboolean G_IsOvermindBuilt( void )
{
  gentity_t dummy;

  memset( &dummy, 0, sizeof( gentity_t ) );

  dummy.overmindNode = NULL;
  dummy.biteam = BIT_ALIENS;

  return G_FindOvermind( &dummy );
}

/*
================
G_FindCreep

attempt to find creep for self, return qtrue if successful
================
*/
static qboolean G_FindCreep( gentity_t *self )
{
  int       i;
  gentity_t *ent;
  gentity_t *closestSpawn = NULL;
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
      if( ent->s.eType != ET_BUILDABLE )
        continue;

      if( ( ent->s.modelindex == BA_A_SPAWN || ent->s.modelindex == BA_A_OVERMIND ) &&
          ent->spawned )
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
G_IsCreepHere

simple wrapper to G_FindCreep to check if a location has creep
================
*/
static qboolean G_IsCreepHere( vec3_t origin )
{
  gentity_t dummy;

  memset( &dummy, 0, sizeof( gentity_t ) );

  dummy.parentNode = NULL;
  dummy.s.modelindex = BA_NONE;
  VectorCopy( origin, dummy.s.origin );

  return G_FindCreep( &dummy );
}

/*
================
G_CreepSlow

Set any nearby humans' SS_CREEPSLOWED flag
================
*/
static void G_CreepSlow( gentity_t *self )
{
  int         entityList[ MAX_GENTITIES ];
  vec3_t      range;
  vec3_t      mins, maxs;
  int         i, num;
  gentity_t   *enemy;
  buildable_t buildable = self->s.modelindex;
  float       creepSize = (float)BG_FindCreepSizeForBuildable( buildable );

  VectorSet( range, creepSize, creepSize, creepSize );

  VectorAdd( self->s.origin, range, maxs );
  VectorSubtract( self->s.origin, range, mins );

  //find humans
  num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
  for( i = 0; i < num; i++ )
  {
    enemy = &g_entities[ entityList[ i ] ];

    if( enemy->client && enemy->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS &&
        enemy->client->ps.groundEntityNum != ENTITYNUM_NONE &&
        G_Visible( self, enemy ) )
    {
      enemy->client->ps.stats[ STAT_STATE ] |= SS_CREEPSLOWED;
      enemy->client->lastCreepSlowTime = level.time;
    }
  }
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

    if( self->spawned )
      self->s.time = -level.time;
    else
      self->s.time = -( level.time -
          (int)( (float)CREEP_SCALEDOWN_TIME *
                 ( 1.0f - ( (float)( level.time - self->buildTime ) /
                            (float)BG_FindBuildTimeForBuildable( self->s.modelindex ) ) ) ) );
  }

  //creep is still receeding
  if( ( self->timestamp + 10000 ) > level.time )
    self->nextthink = level.time + 500;
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
  G_SelectiveRadiusDamage( self->s.pos.trBase, self, self->splashDamage,
    self->splashRadius, self, self->splashMethodOfDeath, PTE_ALIENS );

  //start creep recession
  if( !( self->s.eFlags & EF_DEAD ) )
  {
    self->s.eFlags |= EF_DEAD;
    G_AddEvent( self, EV_BUILD_DESTROY, 0 );

    if( self->spawned )
      self->s.time = -level.time;
    else
      self->s.time = -( level.time -
          (int)( (float)CREEP_SCALEDOWN_TIME *
                 ( 1.0f - ( (float)( level.time - self->buildTime ) /
                            (float)BG_FindBuildTimeForBuildable( self->s.modelindex ) ) ) ) );
  }

  //not dead yet
  if( ( self->timestamp + 10000 ) > level.time )
    self->nextthink = level.time + 500;
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

  VectorCopy( self->s.origin2, dir );

  //do a bit of radius damage
  G_SelectiveRadiusDamage( self->s.pos.trBase, self, self->splashDamage,
    self->splashRadius, self, self->splashMethodOfDeath, PTE_ALIENS );

  //pretty events and item cleanup
  self->s.eFlags |= EF_NODRAW; //don't draw the model once it's destroyed
  G_AddEvent( self, EV_ALIEN_BUILDABLE_EXPLOSION, DirToByte( dir ) );
  self->timestamp = level.time;
  self->think = ASpawn_Melt;
  self->nextthink = level.time + 500; //wait .5 seconds before damaging others

  self->r.contents = 0;    //stop collisions...
  trap_LinkEntity( self ); //...requires a relink
}

/*
================
ASpawn_Die

Called when an alien spawn dies
================
*/
void ASpawn_Die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
  G_SetBuildableAnim( self, BANIM_DESTROY1, qtrue );
  G_SetIdleBuildableAnim( self, BANIM_DESTROYED );

  self->die = nullDieFunction;
  self->think = ASpawn_Blast;

  if( self->spawned )
    self->nextthink = level.time + 5000;
  else
    self->nextthink = level.time; //blast immediately

  self->s.eFlags &= ~EF_FIRING; //prevent any firing effects

  if( attacker && attacker->client )
  {
    if( attacker->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
    {
      if( self->s.modelindex == BA_A_OVERMIND )
        G_AddCreditToClient( attacker->client, OVERMIND_VALUE, qtrue );
      else if( self->s.modelindex == BA_A_SPAWN )
        G_AddCreditToClient( attacker->client, ASPAWN_VALUE, qtrue );
    }
    else
    {
      G_TeamCommand( PTE_ALIENS,
        va( "print \"%s ^3DESTROYED^7 by teammate %s^7\n\"",
          BG_FindHumanNameForBuildable( self->s.modelindex ), 
          attacker->client->pers.netname ) );
    }
    G_LogPrintf( "Decon: %i %i %i: %s destroyed %s by %s\n",
      attacker->client->ps.clientNum, self->s.modelindex, mod,
      attacker->client->pers.netname, 
      BG_FindNameForBuildable( self->s.modelindex ),
      modNames[ mod ] );
  }
}

/*
================
ASpawn_Think

think function for Alien Spawn
================
*/
void ASpawn_Think( gentity_t *self )
{
  gentity_t *ent;

  if( self->spawned )
  {
    //only suicide if at rest
    if( self->s.groundEntityNum )
    {
      if( ( ent = G_CheckSpawnPoint( self->s.number, self->s.origin,
              self->s.origin2, BA_A_SPAWN, NULL ) ) != NULL )
      {
        if( ent->s.eType == ET_BUILDABLE || ent->s.number == ENTITYNUM_WORLD ||
            ent->s.eType == ET_MOVER )
        {
          G_Damage( self, NULL, NULL, NULL, NULL, 10000, 0, MOD_SUICIDE );
          return;
        }

        if( ent->s.eType == ET_CORPSE )
          G_FreeEntity( ent ); //quietly remove
      }
    }
  }

  G_CreepSlow( self );

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
  G_SetBuildableAnim( self, BANIM_PAIN1, qfalse );
}





//==================================================================================





#define OVERMIND_ATTACK_PERIOD 10000
#define OVERMIND_DYING_PERIOD  5000
#define OVERMIND_SPAWNS_PERIOD 30000

/*
================
AOvermind_Think

Think function for Alien Overmind
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

  if( self->spawned && ( self->health > 0 ) )
  {
    //do some damage
    num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
    for( i = 0; i < num; i++ )
    {
      enemy = &g_entities[ entityList[ i ] ];

      if( enemy->client && enemy->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
      {
        self->timestamp = level.time;
        G_SelectiveRadiusDamage( self->s.pos.trBase, self, self->splashDamage,
          self->splashRadius, self, MOD_OVERMIND, PTE_ALIENS );
        G_SetBuildableAnim( self, BANIM_ATTACK1, qfalse );
      }
    }

    // just in case an egg finishes building after we tell overmind to stfu
    if( level.numAlienSpawns > 0 )
      level.overmindMuted = qfalse;

    //low on spawns
    if( !level.overmindMuted && level.numAlienSpawns <= 0 &&
        level.time > self->overmindSpawnsTimer )
    {
      qboolean haveBuilder = qfalse;
      gentity_t *builder;

      self->overmindSpawnsTimer = level.time + OVERMIND_SPAWNS_PERIOD;
      G_BroadcastEvent( EV_OVERMIND_SPAWNS, 0 );

      for( i = 0; i < level.numConnectedClients; i++ )
      {
        builder = &g_entities[ level.sortedClients[ i ] ];
        if( builder->spawned &&
          ( builder->client->pers.classSelection == PCL_ALIEN_BUILDER0 ||
            builder->client->pers.classSelection == PCL_ALIEN_BUILDER0_UPG ) )
        {
          haveBuilder = qtrue;
          break;
        }
      }
      // aliens now know they have no eggs, but they're screwed, so stfu
      if( !haveBuilder || G_TimeTilSuddenDeath( ) <= 0 )
        level.overmindMuted = qtrue;
    }

    //overmind dying
    if( self->health < ( OVERMIND_HEALTH / 10.0f ) && level.time > self->overmindDyingTimer )
    {
      self->overmindDyingTimer = level.time + OVERMIND_DYING_PERIOD;
      G_BroadcastEvent( EV_OVERMIND_DYING, 0 );
    }

    //overmind under attack
    if( self->health < self->lastHealth && level.time > self->overmindAttackTimer )
    {
      self->overmindAttackTimer = level.time + OVERMIND_ATTACK_PERIOD;
      G_BroadcastEvent( EV_OVERMIND_ATTACK, 0 );
    }

    self->lastHealth = self->health;
  }
  else
    self->overmindSpawnsTimer = level.time + OVERMIND_SPAWNS_PERIOD;

  G_CreepSlow( self );

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
    G_SetBuildableAnim( self, BANIM_PAIN1, qfalse );
  else
    G_SetBuildableAnim( self, BANIM_PAIN2, qfalse );
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

  VectorCopy( self->s.origin2, dir );

  //do a bit of radius damage
  G_SelectiveRadiusDamage( self->s.pos.trBase, self, self->splashDamage,
    self->splashRadius, self, self->splashMethodOfDeath, PTE_ALIENS );

  //pretty events and item cleanup
  self->s.eFlags |= EF_NODRAW; //don't draw the model once its destroyed
  G_AddEvent( self, EV_ALIEN_BUILDABLE_EXPLOSION, DirToByte( dir ) );
  self->timestamp = level.time;
  self->think = A_CreepRecede;
  self->nextthink = level.time + 500; //wait .5 seconds before damaging others

  self->r.contents = 0;    //stop collisions...
  trap_LinkEntity( self ); //...requires a relink
}

/*
================
ABarricade_Die

Called when an alien spawn dies
================
*/
void ABarricade_Die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
  G_SetBuildableAnim( self, BANIM_DESTROY1, qtrue );
  G_SetIdleBuildableAnim( self, BANIM_DESTROYED );

  self->die = nullDieFunction;
  self->think = ABarricade_Blast;
  self->s.eFlags &= ~EF_FIRING; //prevent any firing effects

  if( self->spawned )
    self->nextthink = level.time + 5000;
  else
    self->nextthink = level.time; //blast immediately

  if( attacker && attacker->client )
  {
    if( attacker->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS )
    {
      G_TeamCommand( PTE_ALIENS,
        va( "print \"%s ^3DESTROYED^7 by teammate %s^7\n\"",
          BG_FindHumanNameForBuildable( self->s.modelindex ), 
          attacker->client->pers.netname ) );
    }
    G_LogPrintf( "Decon: %i %i %i: %s destroyed %s by %s\n",
      attacker->client->ps.clientNum, self->s.modelindex, mod,
      attacker->client->pers.netname, 
      BG_FindNameForBuildable( self->s.modelindex ),
      modNames[ mod ] );
  }
}

/*
================
ABarricade_Think

Think function for Alien Barricade
================
*/
void ABarricade_Think( gentity_t *self )
{
  //if there is no creep nearby die
  if( !G_FindCreep( self ) )
  {
    G_Damage( self, NULL, NULL, NULL, NULL, 10000, 0, MOD_SUICIDE );
    return;
  }

  G_CreepSlow( self );

  self->nextthink = level.time + BG_FindNextThinkForBuildable( self->s.modelindex );
}




//==================================================================================




void AAcidTube_Think( gentity_t *self );

/*
================
AAcidTube_Damage

Damage function for Alien Acid Tube
================
*/
void AAcidTube_Damage( gentity_t *self )
{
  if( self->spawned )
  {
    if( !( self->s.eFlags & EF_FIRING ) )
    {
      self->s.eFlags |= EF_FIRING;
      G_AddEvent( self, EV_ALIEN_ACIDTUBE, DirToByte( self->s.origin2 ) );
    }

    if( ( self->timestamp + ACIDTUBE_REPEAT ) > level.time )
      self->think = AAcidTube_Damage;
    else
    {
      self->think = AAcidTube_Think;
      self->s.eFlags &= ~EF_FIRING;
    }

    //do some damage
    G_SelectiveRadiusDamage( self->s.pos.trBase, self, self->splashDamage,
      self->splashRadius, self, self->splashMethodOfDeath, PTE_ALIENS );
  }

  G_CreepSlow( self );

  self->nextthink = level.time + BG_FindNextThinkForBuildable( self->s.modelindex );
}

/*
================
AAcidTube_Think

Think function for Alien Acid Tube
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
  if( !G_FindCreep( self ) )
  {
    G_Damage( self, NULL, NULL, NULL, NULL, 10000, 0, MOD_SUICIDE );
    return;
  }

  if( self->spawned && G_FindOvermind( self ) )
  {
    //do some damage
    num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
    for( i = 0; i < num; i++ )
    {
      enemy = &g_entities[ entityList[ i ] ];

      if( !G_Visible( self, enemy ) )
        continue;

      if( enemy->client && enemy->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
      {
        self->timestamp = level.time;
        self->think = AAcidTube_Damage;
        self->nextthink = level.time + 100;
        G_SetBuildableAnim( self, BANIM_ATTACK1, qfalse );
        return;
      }
    }
  }

  G_CreepSlow( self );

  self->nextthink = level.time + BG_FindNextThinkForBuildable( self->s.modelindex );
}




//==================================================================================




/*
================
AHive_Think

Think function for Alien Hive
================
*/
void AHive_Think( gentity_t *self )
{
  int       entityList[ MAX_GENTITIES ];
  vec3_t    range = { ACIDTUBE_RANGE, ACIDTUBE_RANGE, ACIDTUBE_RANGE };
  vec3_t    mins, maxs;
  int       i, num;
  gentity_t *enemy;
  vec3_t    dirToTarget;

  self->nextthink = level.time + BG_FindNextThinkForBuildable( self->s.modelindex );

  VectorAdd( self->s.origin, range, maxs );
  VectorSubtract( self->s.origin, range, mins );

  //if there is no creep nearby die
  if( !G_FindCreep( self ) )
  {
    G_Damage( self, NULL, NULL, NULL, NULL, 10000, 0, MOD_SUICIDE );
    return;
  }

  if( self->timestamp < level.time )
    self->active = qfalse; //nothing has returned in HIVE_REPEAT seconds, forget about it

  if( self->spawned && !self->active && G_FindOvermind( self ) )
  {
    //do some damage
    num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
    for( i = 0; i < num; i++ )
    {
      enemy = &g_entities[ entityList[ i ] ];

      if( enemy->health <= 0 )
        continue;

      if( !G_Visible( self, enemy ) )
        continue;

      if( enemy->client && enemy->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
      {
        self->active = qtrue;
        self->target_ent = enemy;
        self->timestamp = level.time + HIVE_REPEAT;

        VectorSubtract( enemy->s.pos.trBase, self->s.pos.trBase, dirToTarget );
        VectorNormalize( dirToTarget );
        vectoangles( dirToTarget, self->turretAim );

        //fire at target
        FireWeapon( self );
        G_SetBuildableAnim( self, BANIM_ATTACK1, qfalse );
        return;
      }
    }
  }

  G_CreepSlow( self );
}




//==================================================================================




#define HOVEL_TRACE_DEPTH 128.0f

/*
================
AHovel_Blocked

Is this hovel entrance blocked?
================
*/
qboolean AHovel_Blocked( gentity_t *hovel, gentity_t *player, qboolean provideExit )
{
  vec3_t    forward, normal, origin, start, end, angles, hovelMaxs;
  vec3_t    mins, maxs;
  float     displacement;
  trace_t   tr;

  BG_FindBBoxForBuildable( BA_A_HOVEL, NULL, hovelMaxs );
  BG_FindBBoxForClass( player->client->ps.stats[ STAT_PCLASS ],
                       mins, maxs, NULL, NULL, NULL );

  VectorCopy( hovel->s.origin2, normal );
  AngleVectors( hovel->s.angles, forward, NULL, NULL );
  VectorInverse( forward );

  displacement = VectorMaxComponent( maxs ) * M_ROOT3 +
                 VectorMaxComponent( hovelMaxs ) * M_ROOT3 + 1.0f;

  VectorMA( hovel->s.origin, displacement, forward, origin );
  vectoangles( forward, angles );

  VectorMA( origin, HOVEL_TRACE_DEPTH, normal, start );

  //compute a place up in the air to start the real trace
  trap_Trace( &tr, origin, mins, maxs, start, player->s.number, MASK_PLAYERSOLID );
  VectorMA( origin, ( HOVEL_TRACE_DEPTH * tr.fraction ) - 1.0f, normal, start );
  VectorMA( origin, -HOVEL_TRACE_DEPTH, normal, end );

  trap_Trace( &tr, start, mins, maxs, end, player->s.number, MASK_PLAYERSOLID );

  if( tr.startsolid )
    return qtrue;

  VectorCopy( tr.endpos, origin );

  trap_Trace( &tr, origin, mins, maxs, origin, player->s.number, MASK_PLAYERSOLID );

  if( provideExit )
  {
    G_SetOrigin( player, origin );
    VectorCopy( origin, player->client->ps.origin );
    VectorCopy( vec3_origin, player->client->ps.velocity );
    SetClientViewAngle( player, angles );
  }

  if( tr.fraction < 1.0f )
    return qtrue;
  else
    return qfalse;
}

/*
================
APropHovel_Blocked

Wrapper to test a hovel placement for validity
================
*/
static qboolean APropHovel_Blocked( vec3_t origin, vec3_t angles, vec3_t normal,
                                    gentity_t *player )
{
  gentity_t hovel;

  VectorCopy( origin, hovel.s.origin );
  VectorCopy( angles, hovel.s.angles );
  VectorCopy( normal, hovel.s.origin2 );

  return AHovel_Blocked( &hovel, player, qfalse );
}

/*
================
AHovel_Use

Called when an alien uses a hovel
================
*/
void AHovel_Use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
  vec3_t  hovelOrigin, hovelAngles, inverseNormal;

  if( self->spawned && G_FindOvermind( self ) )
  {
    if( self->active )
    {
      //this hovel is in use
      G_TriggerMenu( activator->client->ps.clientNum, MN_A_HOVEL_OCCUPIED );
    }
    else if( ( ( activator->client->ps.stats[ STAT_PCLASS ] == PCL_ALIEN_BUILDER0 ) ||
               ( activator->client->ps.stats[ STAT_PCLASS ] == PCL_ALIEN_BUILDER0_UPG ) ) &&
             activator->health > 0 && self->health > 0 )
    {
      if( AHovel_Blocked( self, activator, qfalse ) )
      {
        //you can get in, but you can't get out
        G_TriggerMenu( activator->client->ps.clientNum, MN_A_HOVEL_BLOCKED );
        return;
      }

      self->active = qtrue;
      G_SetBuildableAnim( self, BANIM_ATTACK1, qfalse );

      //prevent lerping
      activator->client->ps.eFlags ^= EF_TELEPORT_BIT;
      activator->client->ps.eFlags |= EF_NODRAW;
      G_UnlaggedClear( activator );

      activator->client->ps.stats[ STAT_STATE ] |= SS_HOVELING;
      activator->client->hovel = self;
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
}


/*
================
AHovel_Think

Think for alien hovel
================
*/
void AHovel_Think( gentity_t *self )
{
  if( self->spawned )
  {
    if( self->active )
      G_SetIdleBuildableAnim( self, BANIM_IDLE2 );
    else
      G_SetIdleBuildableAnim( self, BANIM_IDLE1 );
  }

  G_CreepSlow( self );

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

  VectorCopy( self->s.origin2, dir );

  //do a bit of radius damage
  G_SelectiveRadiusDamage( self->s.pos.trBase, self, self->splashDamage,
    self->splashRadius, self, self->splashMethodOfDeath, PTE_ALIENS );

  //pretty events and item cleanup
  self->s.eFlags |= EF_NODRAW; //don't draw the model once its destroyed
  G_AddEvent( self, EV_ALIEN_BUILDABLE_EXPLOSION, DirToByte( dir ) );
  self->s.eFlags &= ~EF_FIRING; //prevent any firing effects
  self->timestamp = level.time;
  self->think = ASpawn_Melt;
  self->nextthink = level.time + 500; //wait .5 seconds before damaging others
  self->die = nullDieFunction;

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
    builder->client->ps.eFlags &= ~EF_NODRAW;
    G_UnlaggedClear( builder );

    G_SetOrigin( builder, newOrigin );
    VectorCopy( newOrigin, builder->client->ps.origin );
    SetClientViewAngle( builder, newAngles );

    //client leaves hovel
    builder->client->ps.stats[ STAT_STATE ] &= ~SS_HOVELING;
  }

  self->r.contents = 0;    //stop collisions...
  trap_LinkEntity( self ); //...requires a relink

  if( attacker && attacker->client )
  {
    if( attacker->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS )
    {
      G_TeamCommand( PTE_ALIENS,
        va( "print \"%s ^3DESTROYED^7 by teammate %s^7\n\"",
          BG_FindHumanNameForBuildable( self->s.modelindex ), 
          attacker->client->pers.netname ) );
    }
    G_LogPrintf( "Decon: %i %i %i: %s destroyed %s by %s\n",
      attacker->client->ps.clientNum, self->s.modelindex, mod,
      attacker->client->pers.netname, 
      BG_FindNameForBuildable( self->s.modelindex ),
      modNames[ mod ] );
  }
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
  gclient_t *client = other->client;

  if( !self->spawned )
    return;

  if( !G_FindOvermind( self ) )
    return;

  if( !client )
    return;

  if( client && client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
    return;

  //only allow boostage once every 30 seconds
  if( client->lastBoostedTime + BOOSTER_INTERVAL > level.time )
    return;

  if( !( client->ps.stats[ STAT_STATE ] & SS_BOOSTED ) )
  {
    client->ps.stats[ STAT_STATE ] |= SS_BOOSTED;
    client->lastBoostedTime = level.time;
  }
}




//==================================================================================

#define TRAPPER_ACCURACY 10 // lower is better

/*
================
ATrapper_FireOnEnemy

Used by ATrapper_Think to fire at enemy
================
*/
void ATrapper_FireOnEnemy( gentity_t *self, int firespeed, float range )
{
  gentity_t *enemy = self->enemy;
  vec3_t    dirToTarget;
  vec3_t    halfAcceleration, thirdJerk;
  float     distanceToTarget = BG_FindRangeForBuildable( self->s.modelindex );
  int       lowMsec = 0;
  int       highMsec = (int)( (
    ( ( distanceToTarget * LOCKBLOB_SPEED ) +
      ( distanceToTarget * BG_FindSpeedForClass( enemy->client->ps.stats[ STAT_PCLASS ] ) ) ) /
    ( LOCKBLOB_SPEED * LOCKBLOB_SPEED ) ) * 1000.0f );

  VectorScale( enemy->acceleration, 1.0f / 2.0f, halfAcceleration );
  VectorScale( enemy->jerk, 1.0f / 3.0f, thirdJerk );

  // highMsec and lowMsec can only move toward
  // one another, so the loop must terminate
  while( highMsec - lowMsec > TRAPPER_ACCURACY )
  {
    int   partitionMsec = ( highMsec + lowMsec ) / 2;
    float time = (float)partitionMsec / 1000.0f;
    float projectileDistance = LOCKBLOB_SPEED * time;

    VectorMA( enemy->s.pos.trBase, time, enemy->s.pos.trDelta, dirToTarget );
    VectorMA( dirToTarget, time * time, halfAcceleration, dirToTarget );
    VectorMA( dirToTarget, time * time * time, thirdJerk, dirToTarget );
    VectorSubtract( dirToTarget, self->s.pos.trBase, dirToTarget );
    distanceToTarget = VectorLength( dirToTarget );

    if( projectileDistance < distanceToTarget )
      lowMsec = partitionMsec;
    else if( projectileDistance > distanceToTarget )
      highMsec = partitionMsec;
    else if( projectileDistance == distanceToTarget )
      break; // unlikely to happen
  }

  VectorNormalize( dirToTarget );
  vectoangles( dirToTarget, self->turretAim );

  //fire at target
  FireWeapon( self );
  G_SetBuildableAnim( self, BANIM_ATTACK1, qfalse );
  self->count = level.time + firespeed;
}

/*
================
ATrapper_CheckTarget

Used by ATrapper_Think to check enemies for validity
================
*/
qboolean ATrapper_CheckTarget( gentity_t *self, gentity_t *target, int range )
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
ATrapper_FindEnemy

Used by ATrapper_Think to locate enemy gentities
================
*/
void ATrapper_FindEnemy( gentity_t *ent, int range )
{
  gentity_t *target;

  //iterate through entities
  for( target = g_entities; target < &g_entities[ level.num_entities ]; target++ )
  {
    //if target is not valid keep searching
    if( !ATrapper_CheckTarget( ent, target, range ) )
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

  G_CreepSlow( self );

  self->nextthink = level.time + BG_FindNextThinkForBuildable( self->s.modelindex );

  //if there is no creep nearby die
  if( !G_FindCreep( self ) )
  {
    G_Damage( self, NULL, NULL, NULL, NULL, 10000, 0, MOD_SUICIDE );
    return;
  }

  if( self->spawned && G_FindOvermind( self ) )
  {
    //if the current target is not valid find a new one
    if( !ATrapper_CheckTarget( self, self->enemy, range ) )
      ATrapper_FindEnemy( self, range );

    //if a new target cannot be found don't do anything
    if( !self->enemy )
      return;

    //if we are pointing at our target and we can fire shoot it
    if( self->count < level.time )
      ATrapper_FireOnEnemy( self, firespeed, range );
  }
}



//==================================================================================



/*
================
HRepeater_Think

Think for human power repeater
================
*/
void HRepeater_Think( gentity_t *self )
{
  int       i;
  qboolean  reactor = qfalse;
  gentity_t *ent;

  if( self->spawned )
  {
    //iterate through entities
    for ( i = 1, ent = g_entities + i; i < level.num_entities; i++, ent++ )
    {
      if( ent->s.eType != ET_BUILDABLE )
        continue;

      if( ent->s.modelindex == BA_H_REACTOR && ent->spawned )
        reactor = qtrue;
    }
  }

  if( G_NumberOfDependants( self ) == 0 )
  {
    //if no dependants for x seconds then disappear
    if( self->count < 0 )
      self->count = level.time;
    else if( self->count > 0 && ( ( level.time - self->count ) > REPEATER_INACTIVE_TIME ) )
      G_Damage( self, NULL, NULL, NULL, NULL, 10000, 0, MOD_SUICIDE );
  }
  else
    self->count = -1;

  self->powered = reactor;

  self->nextthink = level.time + POWER_REFRESH_TIME;
}

/*
================
HRepeater_Use

Use for human power repeater
================
*/
void HRepeater_Use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
  if( self->health <= 0 )
    return;

  if( !self->spawned )
    return;

  if( other )
    G_GiveClientMaxAmmo( other, qtrue );
}


#define DCC_ATTACK_PERIOD 10000

/*
================
HReactor_Think

Think function for Human Reactor
================
*/
void HReactor_Think( gentity_t *self )
{
  int       entityList[ MAX_GENTITIES ];
  vec3_t    range = { REACTOR_ATTACK_RANGE, REACTOR_ATTACK_RANGE, REACTOR_ATTACK_RANGE };
  vec3_t    mins, maxs;
  int       i, num;
  gentity_t *enemy, *tent;

  VectorAdd( self->s.origin, range, maxs );
  VectorSubtract( self->s.origin, range, mins );

  if( self->spawned && ( self->health > 0 ) )
  {
    //do some damage
    num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
    for( i = 0; i < num; i++ )
    {
      enemy = &g_entities[ entityList[ i ] ];

      if( enemy->client && enemy->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS )
      {
        self->timestamp = level.time;
        G_SelectiveRadiusDamage( self->s.pos.trBase, self, REACTOR_ATTACK_DAMAGE,
          REACTOR_ATTACK_RANGE, self, MOD_REACTOR, PTE_HUMANS );

        tent = G_TempEntity( enemy->s.pos.trBase, EV_TESLATRAIL );

        VectorCopy( self->s.pos.trBase, tent->s.origin2 );

        tent->s.generic1 = self->s.number; //src
        tent->s.clientNum = enemy->s.number; //dest
      }
    }

    //reactor under attack
    if( self->health < self->lastHealth &&
        level.time > level.humanBaseAttackTimer && G_IsDCCBuilt( ) )
    {
      level.humanBaseAttackTimer = level.time + DCC_ATTACK_PERIOD;
      G_BroadcastEvent( EV_DCC_ATTACK, 0 );
    }

    self->lastHealth = self->health;
  }

  self->nextthink = level.time + BG_FindNextThinkForBuildable( self->s.modelindex );
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
  if( self->spawned )
  {
    //only humans can activate this
    if( activator->client->ps.stats[ STAT_PTEAM ] != PTE_HUMANS )
      return;

    //if this is powered then call the armoury menu
    if( self->powered )
      G_TriggerMenu( activator->client->ps.clientNum, MN_H_ARMOURY );
    else
      G_TriggerMenu( activator->client->ps.clientNum, MN_H_NOTPOWERED );
  }
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
  self->nextthink = level.time + POWER_REFRESH_TIME;

  self->powered = G_FindPower( self );
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
  self->nextthink = level.time + POWER_REFRESH_TIME;

  self->powered = G_FindPower( self );
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
  qboolean  occupied = qfalse;

  self->nextthink = level.time + BG_FindNextThinkForBuildable( self->s.modelindex );

  //make sure we have power
  if( !( self->powered = G_FindPower( self ) ) )
  {
    self->nextthink = level.time + POWER_REFRESH_TIME;
    return;
  }

  if( self->spawned )
  {
    VectorAdd( self->s.origin, self->r.maxs, maxs );
    VectorAdd( self->s.origin, self->r.mins, mins );

    mins[ 2 ] += fabs( self->r.mins[ 2 ] ) + self->r.maxs[ 2 ];
    maxs[ 2 ] += 60; //player height

    //if active use the healing idle
    if( self->active )
      G_SetIdleBuildableAnim( self, BANIM_IDLE2 );

    //check if a previous occupier is still here
    num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
    for( i = 0; i < num; i++ )
    {
      player = &g_entities[ entityList[ i ] ];

      if( player->client && player->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
      {
        if( player->health < player->client->ps.stats[ STAT_MAX_HEALTH ] &&
            player->client->ps.pm_type != PM_DEAD &&
            self->enemy == player )
          occupied = qtrue;
      }
    }

    if( !occupied )
    {
      self->enemy = NULL;

      //look for something to heal
      for( i = 0; i < num; i++ )
      {
        player = &g_entities[ entityList[ i ] ];

        if( player->client && player->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
        {
          if( player->health < player->client->ps.stats[ STAT_MAX_HEALTH ] &&
              player->client->ps.pm_type != PM_DEAD )
          {
            self->enemy = player;

            //start the heal anim
            if( !self->active )
            {
              G_SetBuildableAnim( self, BANIM_ATTACK1, qfalse );
              self->active = qtrue;
            }
          }
          else if( !BG_InventoryContainsUpgrade( UP_MEDKIT, player->client->ps.stats ) )
            BG_AddUpgradeToInventory( UP_MEDKIT, player->client->ps.stats );
        }
      }
    }

    //nothing left to heal so go back to idling
    if( !self->enemy && self->active )
    {
      G_SetBuildableAnim( self, BANIM_CONSTRUCT2, qtrue );
      G_SetIdleBuildableAnim( self, BANIM_IDLE1 );

      self->active = qfalse;
    }
    else if( self->enemy ) //heal!
    {
      if( self->enemy->client && self->enemy->client->ps.stats[ STAT_STATE ] & SS_POISONED )
        self->enemy->client->ps.stats[ STAT_STATE ] &= ~SS_POISONED;

      if( self->enemy->client && self->enemy->client->ps.stats[ STAT_STATE ] & SS_MEDKIT_ACTIVE )
        self->enemy->client->ps.stats[ STAT_STATE ] &= ~SS_MEDKIT_ACTIVE;

      self->enemy->health++;

      //if they're completely healed, give them a medkit
      if( self->enemy->health >= self->enemy->client->ps.stats[ STAT_MAX_HEALTH ] &&
          !BG_InventoryContainsUpgrade( UP_MEDKIT, self->enemy->client->ps.stats ) )
        BG_AddUpgradeToInventory( UP_MEDKIT, self->enemy->client->ps.stats );
    }
  }
}




//==================================================================================




/*
================
HMGTurret_TrackEnemy

Used by HMGTurret_Think to track enemy location
================
*/
qboolean HMGTurret_TrackEnemy( gentity_t *self )
{
  vec3_t  dirToTarget, dttAdjusted, angleToTarget, angularDiff, xNormal;
  vec3_t  refNormal = { 0.0f, 0.0f, 1.0f };
  float   temp, rotAngle;
  float   accuracyTolerance, angularSpeed;

  if( self->lev1Grabbed )
  {
    //can't turn fast if grabbed
    accuracyTolerance = MGTURRET_GRAB_ACCURACYTOLERANCE;
    angularSpeed = MGTURRET_GRAB_ANGULARSPEED;
  }
  else if( self->dcced )
  {
    accuracyTolerance = MGTURRET_DCC_ACCURACYTOLERANCE;
    angularSpeed = MGTURRET_DCC_ANGULARSPEED;
  }
  else
  {
    accuracyTolerance = MGTURRET_ACCURACYTOLERANCE;
    angularSpeed = MGTURRET_ANGULARSPEED;
  }

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
  if( angularDiff[ PITCH ] < (-accuracyTolerance) )
    self->s.angles2[ PITCH ] += angularSpeed;
  else if( angularDiff[ PITCH ] > accuracyTolerance )
    self->s.angles2[ PITCH ] -= angularSpeed;
  else
    self->s.angles2[ PITCH ] = angleToTarget[ PITCH ];

  //disallow vertical movement past a certain limit
  temp = fabs( self->s.angles2[ PITCH ] );
  if( temp > 180 )
    temp -= 360;

  if( temp < -MGTURRET_VERTICALCAP )
    self->s.angles2[ PITCH ] = (-360) + MGTURRET_VERTICALCAP;

  //if not pointing at our target then move accordingly
  if( angularDiff[ YAW ] < (-accuracyTolerance) )
    self->s.angles2[ YAW ] += angularSpeed;
  else if( angularDiff[ YAW ] > accuracyTolerance )
    self->s.angles2[ YAW ] -= angularSpeed;
  else
    self->s.angles2[ YAW ] = angleToTarget[ YAW ];

  AngleVectors( self->s.angles2, dttAdjusted, NULL, NULL );
  RotatePointAroundVector( dirToTarget, xNormal, dttAdjusted, -rotAngle );
  vectoangles( dirToTarget, self->turretAim );

  //if pointing at our target return true
  if( abs( angleToTarget[ YAW ] - self->s.angles2[ YAW ] ) <= accuracyTolerance &&
      abs( angleToTarget[ PITCH ] - self->s.angles2[ PITCH ] ) <= accuracyTolerance )
    return qtrue;

  return qfalse;
}


/*
================
HMGTurret_CheckTarget

Used by HMGTurret_Think to check enemies for validity
================
*/
qboolean HMGTurret_CheckTarget( gentity_t *self, gentity_t *target, qboolean ignorePainted )
{
  trace_t   trace;
  gentity_t *traceEnt;

  if( !target )
    return qfalse;

  if( !target->client )
    return qfalse;

  if( target->client->ps.stats[ STAT_STATE ] & SS_HOVELING )
    return qfalse;

  if( target->health <= 0 )
    return qfalse;

  if( Distance( self->s.origin, target->s.pos.trBase ) > MGTURRET_RANGE )
    return qfalse;

  //some turret has already selected this target
  if( self->dcced && target->targeted && target->targeted->powered && !ignorePainted )
    return qfalse;

  trap_Trace( &trace, self->s.pos.trBase, NULL, NULL, target->s.pos.trBase, self->s.number, MASK_SHOT );

  traceEnt = &g_entities[ trace.entityNum ];

  if( !traceEnt->client )
    return qfalse;

  if( traceEnt->client && traceEnt->client->ps.stats[ STAT_PTEAM ] != PTE_ALIENS )
    return qfalse;

  return qtrue;
}


/*
================
HMGTurret_FindEnemy

Used by HMGTurret_Think to locate enemy gentities
================
*/
void HMGTurret_FindEnemy( gentity_t *self )
{
  int       entityList[ MAX_GENTITIES ];
  vec3_t    range;
  vec3_t    mins, maxs;
  int       i, num;
  gentity_t *target;

  VectorSet( range, MGTURRET_RANGE, MGTURRET_RANGE, MGTURRET_RANGE );
  VectorAdd( self->s.origin, range, maxs );
  VectorSubtract( self->s.origin, range, mins );

  //find aliens
  num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
  for( i = 0; i < num; i++ )
  {
    target = &g_entities[ entityList[ i ] ];

    if( target->client && target->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS )
    {
      //if target is not valid keep searching
      if( !HMGTurret_CheckTarget( self, target, qfalse ) )
        continue;

      //we found a target
      self->enemy = target;
      return;
    }
  }

  if( self->dcced )
  {
    //check again, this time ignoring painted targets
    for( i = 0; i < num; i++ )
    {
      target = &g_entities[ entityList[ i ] ];

      if( target->client && target->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS )
      {
        //if target is not valid keep searching
        if( !HMGTurret_CheckTarget( self, target, qtrue ) )
          continue;

        //we found a target
        self->enemy = target;
        return;
      }
    }
  }

  //couldn't find a target
  self->enemy = NULL;
}


/*
================
HMGTurret_Think

Think function for MG turret
================
*/
void HMGTurret_Think( gentity_t *self )
{
  int firespeed = BG_FindFireSpeedForBuildable( self->s.modelindex );

  self->nextthink = level.time + BG_FindNextThinkForBuildable( self->s.modelindex );

  //used for client side muzzle flashes
  self->s.eFlags &= ~EF_FIRING;

  //if not powered don't do anything and check again for power next think
  if( !( self->powered = G_FindPower( self ) ) )
  {
    self->nextthink = level.time + POWER_REFRESH_TIME;
    return;
  }

  if( self->spawned )
  {
    //find a dcc for self
    self->dcced = G_FindDCC( self );

    //if the current target is not valid find a new one
    if( !HMGTurret_CheckTarget( self, self->enemy, qfalse ) )
    {
      if( self->enemy )
        self->enemy->targeted = NULL;

      HMGTurret_FindEnemy( self );
    }

    //if a new target cannot be found don't do anything
    if( !self->enemy )
      return;

    self->enemy->targeted = self;

    //if we are pointing at our target and we can fire shoot it
    if( HMGTurret_TrackEnemy( self ) && ( self->count < level.time ) )
    {
      //fire at target
      FireWeapon( self );

      self->s.eFlags |= EF_FIRING;
      G_AddEvent( self, EV_FIRE_WEAPON, 0 );
      G_SetBuildableAnim( self, BANIM_ATTACK1, qfalse );

      self->count = level.time + firespeed;
    }
  }
}




//==================================================================================




/*
================
HTeslaGen_Think

Think function for Tesla Generator
================
*/
void HTeslaGen_Think( gentity_t *self )
{
  int       entityList[ MAX_GENTITIES ];
  vec3_t    range;
  vec3_t    mins, maxs;
  vec3_t    dir;
  int       i, num;
  gentity_t *enemy;

  self->nextthink = level.time + BG_FindNextThinkForBuildable( self->s.modelindex );

  //if not powered don't do anything and check again for power next think
  if( !( self->powered = G_FindPower( self ) ) || !( self->dcced = G_FindDCC( self ) ) )
  {
    self->s.eFlags &= ~EF_FIRING;
    self->nextthink = level.time + POWER_REFRESH_TIME;
    return;
  }

  if( self->spawned && self->count < level.time )
  {
    //used to mark client side effects
    self->s.eFlags &= ~EF_FIRING;

    VectorSet( range, TESLAGEN_RANGE, TESLAGEN_RANGE, TESLAGEN_RANGE );
    VectorAdd( self->s.origin, range, maxs );
    VectorSubtract( self->s.origin, range, mins );

    //find aliens
    num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
    for( i = 0; i < num; i++ )
    {
      enemy = &g_entities[ entityList[ i ] ];

      if( enemy->client && enemy->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS &&
          enemy->health > 0 &&
          Distance( enemy->s.pos.trBase, self->s.pos.trBase ) <= TESLAGEN_RANGE )
      {
        VectorSubtract( enemy->s.pos.trBase, self->s.pos.trBase, dir );
        VectorNormalize( dir );
        vectoangles( dir, self->turretAim );

        //fire at target
        FireWeapon( self );
      }
    }

    if( self->s.eFlags & EF_FIRING )
    {
      G_AddEvent( self, EV_FIRE_WEAPON, 0 );

      //doesn't really need an anim
      //G_SetBuildableAnim( self, BANIM_ATTACK1, qfalse );

      self->count = level.time + TESLAGEN_REPEAT;
    }
  }
}




//==================================================================================




/*
================
HSpawn_Disappear

Called when a human spawn is destroyed before it is spawned
think function
================
*/
void HSpawn_Disappear( gentity_t *self )
{
  vec3_t  dir;

  // we don't have a valid direction, so just point straight up
  dir[ 0 ] = dir[ 1 ] = 0;
  dir[ 2 ] = 1;

  self->s.eFlags |= EF_NODRAW; //don't draw the model once its destroyed
  self->timestamp = level.time;

  self->think = freeBuildable;
  self->nextthink = level.time + 100;

  self->r.contents = 0;    //stop collisions...
  trap_LinkEntity( self ); //...requires a relink
}


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

  self->s.eFlags |= EF_NODRAW; //don't draw the model once its destroyed
  G_AddEvent( self, EV_HUMAN_BUILDABLE_EXPLOSION, DirToByte( dir ) );
  self->timestamp = level.time;

  //do some radius damage
  G_RadiusDamage( self->s.pos.trBase, self, self->splashDamage,
    self->splashRadius, self, self->splashMethodOfDeath );

  self->think = freeBuildable;
  self->nextthink = level.time + 100;

  self->r.contents = 0;    //stop collisions...
  trap_LinkEntity( self ); //...requires a relink
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
  G_SetBuildableAnim( self, BANIM_DESTROY1, qtrue );
  G_SetIdleBuildableAnim( self, BANIM_DESTROYED );

  self->die = nullDieFunction;
  self->powered = qfalse; //free up power
  self->s.eFlags &= ~EF_FIRING; //prevent any firing effects

  if( self->spawned )
  {
    self->think = HSpawn_Blast;
    self->nextthink = level.time + HUMAN_DETONATION_DELAY;
  }
  else
  {
    self->think = HSpawn_Disappear;
    self->nextthink = level.time; //blast immediately
  }

  if( attacker && attacker->client )
  {
    if( attacker->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS )
    {
      if( self->s.modelindex == BA_H_REACTOR )
        G_AddCreditToClient( attacker->client, REACTOR_VALUE, qtrue );
      else if( self->s.modelindex == BA_H_SPAWN )
        G_AddCreditToClient( attacker->client, HSPAWN_VALUE, qtrue );
    }
    else
    {
      G_TeamCommand( PTE_HUMANS,
        va( "print \"%s ^3DESTROYED^7 by teammate %s^7\n\"",
          BG_FindHumanNameForBuildable( self->s.modelindex ), 
          attacker->client->pers.netname ) );
    }
    G_LogPrintf( "Decon: %i %i %i: %s destroyed %s by %s\n",
      attacker->client->ps.clientNum, self->s.modelindex, mod,
      attacker->client->pers.netname, 
      BG_FindNameForBuildable( self->s.modelindex ),
      modNames[ mod ] );
  }
}

/*
================
HSpawn_Think

Think for human spawn
================
*/
void HSpawn_Think( gentity_t *self )
{
  gentity_t *ent;

  //make sure we have power
  self->powered = G_FindPower( self );

  if( self->spawned )
  {
    //only suicide if at rest
    if( self->s.groundEntityNum )
    {
      if( ( ent = G_CheckSpawnPoint( self->s.number, self->s.origin,
              self->s.origin2, BA_H_SPAWN, NULL ) ) != NULL )
      {
        if( ent->s.eType == ET_BUILDABLE || ent->s.number == ENTITYNUM_WORLD ||
            ent->s.eType == ET_MOVER )
        {
          G_Damage( self, NULL, NULL, NULL, NULL, 10000, 0, MOD_SUICIDE );
          return;
        }

        if( ent->s.eType == ET_CORPSE )
          G_FreeEntity( ent ); //quietly remove
      }
    }

    //spawn under attack
    if( self->health < self->lastHealth &&
        level.time > level.humanBaseAttackTimer && G_IsDCCBuilt( ) )
    {
      level.humanBaseAttackTimer = level.time + DCC_ATTACK_PERIOD;
      G_BroadcastEvent( EV_DCC_ATTACK, 0 );
    }

    self->lastHealth = self->health;
  }

  self->nextthink = level.time + BG_FindNextThinkForBuildable( self->s.modelindex );
}




//==================================================================================


/*
============
G_BuildableTouchTriggers

Find all trigger entities that a buildable touches.
============
*/
void G_BuildableTouchTriggers( gentity_t *ent )
{
  int       i, num;
  int       touch[ MAX_GENTITIES ];
  gentity_t *hit;
  trace_t   trace;
  vec3_t    mins, maxs;
  vec3_t    bmins, bmaxs;
  static    vec3_t range = { 10, 10, 10 };

  // dead buildables don't activate triggers!
  if( ent->health <= 0 )
    return;

  BG_FindBBoxForBuildable( ent->s.modelindex, bmins, bmaxs );

  VectorAdd( ent->s.origin, bmins, mins );
  VectorAdd( ent->s.origin, bmaxs, maxs );

  VectorSubtract( mins, range, mins );
  VectorAdd( maxs, range, maxs );

  num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

  VectorAdd( ent->s.origin, bmins, mins );
  VectorAdd( ent->s.origin, bmaxs, maxs );

  for( i = 0; i < num; i++ )
  {
    hit = &g_entities[ touch[ i ] ];

    if( !hit->touch )
      continue;

    if( !( hit->r.contents & CONTENTS_TRIGGER ) )
      continue;

    //ignore buildables not yet spawned
    if( !ent->spawned )
      continue;

    if( !trap_EntityContact( mins, maxs, hit ) )
      continue;

    memset( &trace, 0, sizeof( trace ) );

    if( hit->touch )
      hit->touch( hit, ent, &trace );
  }
}


/*
===============
G_BuildableThink

General think function for buildables
===============
*/
void G_BuildableThink( gentity_t *ent, int msec )
{
  int bHealth = BG_FindHealthForBuildable( ent->s.modelindex );
  int bRegen = BG_FindRegenRateForBuildable( ent->s.modelindex );
  int bTime = BG_FindBuildTimeForBuildable( ent->s.modelindex );

  //pack health, power and dcc

  //toggle spawned flag for buildables
  if( !ent->spawned && ent->health > 0 )
  {
    if( ent->buildTime + bTime < level.time )
      ent->spawned = qtrue;
  }

  ent->s.generic1 = (int)( ( (float)ent->health / (float)bHealth ) * B_HEALTH_MASK );

  if( ent->s.generic1 < 0 )
    ent->s.generic1 = 0;

  if( ent->powered )
    ent->s.generic1 |= B_POWERED_TOGGLEBIT;

  if( ent->dcced )
    ent->s.generic1 |= B_DCCED_TOGGLEBIT;

  if( ent->spawned )
    ent->s.generic1 |= B_SPAWNED_TOGGLEBIT;

  if( ent->deconstruct )
    ent->s.generic1 |= B_MARKED_TOGGLEBIT;

  ent->time1000 += msec;

  if( ent->time1000 >= 1000 )
  {
    ent->time1000 -= 1000;

    if( !ent->spawned && ent->health > 0 )
      ent->health += (int)( ceil( (float)bHealth / (float)( bTime * 0.001 ) ) );
    else if( ent->biteam == BIT_ALIENS && ent->health > 0 && ent->health < bHealth &&
        bRegen && ( ent->lastDamageTime + ALIEN_REGEN_DAMAGE_TIME ) < level.time )
      ent->health += bRegen;

    if( ent->health > bHealth )
      ent->health = bHealth;
  }

  if( ent->lev1Grabbed && ent->lev1GrabTime + LEVEL1_GRAB_TIME < level.time )
    ent->lev1Grabbed = qfalse;

  if( ent->clientSpawnTime > 0 )
    ent->clientSpawnTime -= msec;

  if( ent->clientSpawnTime < 0 )
    ent->clientSpawnTime = 0;

  //check if this buildable is touching any triggers
  G_BuildableTouchTriggers( ent );

  //fall back on normal physics routines
  G_Physics( ent, msec );
}


/*
===============
G_BuildableRange

Check whether a point is within some range of a type of buildable
===============
*/
qboolean G_BuildableRange( vec3_t origin, float r, buildable_t buildable )
{
  int       entityList[ MAX_GENTITIES ];
  vec3_t    range;
  vec3_t    mins, maxs;
  int       i, num;
  gentity_t *ent;

  VectorSet( range, r, r, r );
  VectorAdd( origin, range, maxs );
  VectorSubtract( origin, range, mins );

  num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
  for( i = 0; i < num; i++ )
  {
    ent = &g_entities[ entityList[ i ] ];

    if( ent->s.eType != ET_BUILDABLE )
      continue;

    if( ent->biteam == BIT_HUMANS && !ent->powered )
      continue;

    if( ent->s.modelindex == buildable && ent->spawned )
      return qtrue;
  }

  return qfalse;
}

/*
===============
G_CompareBuildablesForRemoval

qsort comparison function for a buildable removal list
===============
*/
static int G_CompareBuildablesForRemoval( const void *a, const void *b )
{
  int       precedence[ ] =
  {
    BA_NONE,

    BA_A_BARRICADE,
    BA_A_ACIDTUBE,
    BA_A_TRAPPER,
    BA_A_HIVE,
    BA_A_BOOSTER,
    BA_A_HOVEL,
    BA_A_SPAWN,
    BA_A_OVERMIND,

    BA_H_MGTURRET,
    BA_H_TESLAGEN,
    BA_H_DCC,
    BA_H_MEDISTAT,
    BA_H_ARMOURY,
    BA_H_SPAWN,
    BA_H_REPEATER,
    BA_H_REACTOR
  };

  gentity_t *buildableA, *buildableB;
  int       i;
  int       aPrecedence = 0, bPrecedence = 0;

  buildableA = *(gentity_t **)a;
  buildableB = *(gentity_t **)b;

  // If they're the same type then pick the one marked earliest
  if( buildableA->s.modelindex == buildableB->s.modelindex )
    return buildableA->deconstructTime - buildableB->deconstructTime;

  for( i = 0; i < sizeof( precedence ) / sizeof( precedence[ 0 ] ); i++ )
  {
    if( buildableA->s.modelindex == precedence[ i ] )
      aPrecedence = i;

    if( buildableB->s.modelindex == precedence[ i ] )
      bPrecedence = i;
  }

  return aPrecedence - bPrecedence;
}

/*
===============
G_FreeMarkedBuildables

Free up build points for a team by deconstructing marked buildables
===============
*/
void G_FreeMarkedBuildables( void )
{
  int       i;
  gentity_t *ent;

  if( !g_markDeconstruct.integer )
    return; // Not enabled, can't deconstruct anything

  for( i = 0; i < level.numBuildablesForRemoval; i++ )
  {
    ent = level.markedBuildables[ i ];

    G_FreeEntity( ent );
  }
}

/*
===============
G_SufficientBPAvailable

Determine if enough build points can be released for the buildable
and list the buildables that much be destroyed if this is the case
===============
*/
static qboolean G_SufficientBPAvailable( buildableTeam_t team,
                                         int             buildPoints,
                                         buildable_t     buildable )
{
  int       i;
  int       numBuildables = level.numBuildablesForRemoval;
  int       pointsYielded = 0;
  gentity_t *ent;
  qboolean  unique = BG_FindUniqueTestForBuildable( buildable );
  int       remainingBP, remainingSpawns;
  int       numBuildablesInTheWay = level.numBuildablesForRemoval;

  if( team == BIT_ALIENS )
  {
    remainingBP     = level.alienBuildPoints;
    remainingSpawns = level.numAlienSpawns;
  }
  else if( team == BIT_HUMANS )
  {
    remainingBP     = level.humanBuildPoints;
    remainingSpawns = level.numHumanSpawns;
  }
  else
    return qfalse;

  // Simple non-marking case
  if( !g_markDeconstruct.integer )
  {
    if( remainingBP - buildPoints < 0 )
      return qfalse;
    else
      return qtrue;
  }

  // Set buildPoints to the number extra that are required
  buildPoints -= remainingBP;

  level.numBuildablesForRemoval = 0;

  // Build a list of buildable entities
  for( i = 1, ent = g_entities + i; i < level.num_entities; i++, ent++ )
  {
    if( !ent->inuse )
      continue;

    if( ent->health <= 0 )
      continue;

    // Don't allow destruction of hovel with granger inside
    if( ent->s.modelindex == BA_A_HOVEL && ent->active )
      continue;

    if( ent->biteam != team )
      continue;

    // Prevent destruction of the last spawn
    if( remainingSpawns <= 1 )
    {
      if( ent->s.modelindex == BA_A_SPAWN || ent->s.modelindex == BA_H_SPAWN )
        continue;
    }

    // If it's a unique buildable, it can only be replaced by the same type
    if( unique && ent->s.modelindex != buildable )
      continue;

    if( ent->deconstruct )
      level.markedBuildables[ numBuildables++ ] = ent;
  }

  // We still need build points, but have no candidates for removal
  if( buildPoints > 0 && numBuildables == 0 )
    return qfalse;

  // Sort everything that was added to the list, but leave what was already 
  // there in the front (those buildings are blocking the new buildable)
  qsort( level.markedBuildables + numBuildablesInTheWay,
    numBuildables - numBuildablesInTheWay,
    sizeof( level.markedBuildables[ 0 ] ), G_CompareBuildablesForRemoval );

  // if any buildings are in the way of what we're building
  // we must force them to be deconned regardless of bp, so this won't work
  if( numBuildablesInTheWay == 0 )
  {
    // Do a pass looking for a buildable of the same type that we're
    // building and mark it (and only it) for destruction if found
    for( i = 0; i < numBuildables; i++ )
    {
      ent = level.markedBuildables[ i ];

      if( ent->s.modelindex == buildable )
      {
        // If we're removing what we're building this will always work
        level.markedBuildables[ 0 ]   = ent;
        level.numBuildablesForRemoval = 1;
        return qtrue;
      }
    }
  }

  // Determine if there are enough markees to yield the required BP
  for( ; pointsYielded < buildPoints && level.numBuildablesForRemoval < numBuildables;
       level.numBuildablesForRemoval++ )
  {
    ent = level.markedBuildables[ level.numBuildablesForRemoval ];
    pointsYielded += BG_FindBuildPointsForBuildable( ent->s.modelindex );
  }

  if( level.numBuildablesForRemoval < numBuildablesInTheWay )
  {
    level.numBuildablesForRemoval = numBuildablesInTheWay;
  }

  // Not enough points yielded
  if( pointsYielded < buildPoints )
  {
    level.numBuildablesForRemoval = 0;
    return qfalse;
  }
  else
  {
    return qtrue;
  }
}

/*
================
G_CanBuild

Checks to see if a buildable can be built
================
*/
itemBuildError_t G_CanBuild( gentity_t *ent, buildable_t buildable, int distance, vec3_t origin )
{
  vec3_t            angles;
  vec3_t            entity_origin, normal;
  vec3_t            mins, maxs, mins1, maxs1;
  int               num;
  int               entitylist[ MAX_GENTITIES ];
  trace_t           tr1, tr2, tr3;
  int               i;
  itemBuildError_t  reason = IBE_NONE;
  gentity_t         *tempent;
  float             minNormal;
  qboolean          invert;
  int               contents;
  playerState_t     *ps = &ent->client->ps;
  int               buildPoints;

  BG_FindBBoxForBuildable( buildable, mins, maxs );

  BG_PositionBuildableRelativeToPlayer( ps, mins, maxs, trap_Trace, entity_origin, angles, &tr1 );
  VectorAdd( entity_origin, mins, mins1 );
  VectorAdd( entity_origin, maxs, maxs1 );
  num = trap_EntitiesInBox( mins1, maxs1, entitylist, MAX_GENTITIES );
  trap_Trace( &tr2, entity_origin, mins, maxs, entity_origin, ent->s.number, MASK_DEADSOLID );
  trap_Trace( &tr3, ps->origin, NULL, NULL, entity_origin, ent->s.number, MASK_DEADSOLID );

  VectorCopy( entity_origin, origin );

  VectorCopy( tr1.plane.normal, normal );
  minNormal = BG_FindMinNormalForBuildable( buildable );
  invert = BG_FindInvertNormalForBuildable( buildable );

  //can we build at this angle?
  if( !( normal[ 2 ] >= minNormal || ( invert && normal[ 2 ] <= -minNormal ) ) )
    reason = IBE_NORMAL;

  if( tr1.entityNum != ENTITYNUM_WORLD )
    reason = IBE_NORMAL;

  //check there is enough room to spawn from (presuming this is a spawn)
  if( G_CheckSpawnPoint( -1, origin, normal, buildable, NULL ) != NULL )
    reason = IBE_NORMAL;

  contents = trap_PointContents( entity_origin, -1 );
  buildPoints = BG_FindBuildPointsForBuildable( buildable );

  //force buildings that are blocking the current building to be
  //deconstructed before other marked buildings
  level.numBuildablesForRemoval = 0;
  for(i = 0; i < num; i++)
  {
    gentity_t *tent = &g_entities[ entitylist[ i ] ];
    if( tent->s.eType == ET_PLAYER )
    {
      reason = IBE_NOROOM;
      break;
    }
    else if( tent->biteam != ent->client->ps.stats[ STAT_PTEAM ] )
    {
      reason = IBE_NOROOM;
    }
    else if( tent->s.eType == ET_BUILDABLE && !tent->deconstruct )
    {
      reason = IBE_NOROOM;
      break;
    }
    else
    {
      if( tent->s.modelindex == BA_H_SPAWN && level.numHumanSpawns <= 1 )
      {
        reason = IBE_NOROOM;
        break;
      }
      else if( tent->s.modelindex == BA_A_SPAWN && level.numHumanSpawns <= 1 )
      {
        reason = IBE_NOROOM;
        break;
      }
      else if( tent->s.modelindex == BA_H_REACTOR && buildable != BA_H_REACTOR )
      {
        reason = IBE_NOROOM;
        break;
      }
      else if( tent->s.modelindex == BA_A_OVERMIND && buildable != BA_A_OVERMIND )
      {
        reason = IBE_NOROOM;
        break;
      }
      level.markedBuildables[ level.numBuildablesForRemoval++ ] = tent;
    }
  }
  if( reason != IBE_NONE )
  {
    level.numBuildablesForRemoval = 0;
  }
  if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS )
  {
    //alien criteria

    if( buildable == BA_A_HOVEL )
    {
      vec3_t    builderMins, builderMaxs;

      //this assumes the adv builder is the biggest thing that'll use the hovel
      BG_FindBBoxForClass( PCL_ALIEN_BUILDER0_UPG, builderMins, builderMaxs, NULL, NULL, NULL );

      if( APropHovel_Blocked( angles, origin, normal, ent ) )
        reason = IBE_HOVELEXIT;
    }

    //check there is creep near by for building on
    if( BG_FindCreepTestForBuildable( buildable ) )
    {
      if( !G_IsCreepHere( entity_origin ) )
        reason = IBE_NOCREEP;
    }

    //check permission to build here
    if( tr1.surfaceFlags & SURF_NOALIENBUILD || tr1.surfaceFlags & SURF_NOBUILD ||
        contents & CONTENTS_NOALIENBUILD || contents & CONTENTS_NOBUILD )
      reason = IBE_PERMISSION;

    //look for an Overmind
    for ( i = 1, tempent = g_entities + i; i < level.num_entities; i++, tempent++ )
    {
      if( tempent->s.eType != ET_BUILDABLE )
        continue;
      if( tempent->s.modelindex == BA_A_OVERMIND && tempent->spawned &&
        tempent->health > 0 )
        break;
    }

    //if none found...
    if( i >= level.num_entities && buildable != BA_A_OVERMIND )
      reason = IBE_NOOVERMIND;

    //can we only have one of these?
    if( BG_FindUniqueTestForBuildable( buildable ) )
    {
      for ( i = 1, tempent = g_entities + i; i < level.num_entities; i++, tempent++ )
      {
        if( tempent->s.eType != ET_BUILDABLE )
          continue;

        if( tempent->s.modelindex == buildable && !tempent->deconstruct )
        {
          switch( buildable )
          {
            case BA_A_OVERMIND:
              reason = IBE_OVERMIND;
              break;

            case BA_A_HOVEL:
              reason = IBE_HOVEL;
              break;

            default:
              Com_Error( ERR_FATAL, "No reason for denying build of %d\n", buildable );
              break;
          }

          break;
        }
        if( tempent->s.modelindex == BA_A_HOVEL &&
            buildable == BA_A_HOVEL &&
            tempent->active )
        {
          reason = IBE_HOVEL;
        }
        else if( tempent->s.modelindex == buildable )
        {
          level.markedBuildables[ level.numBuildablesForRemoval++ ] = tempent;
        }
      }
    }

    if( !G_SufficientBPAvailable( BIT_ALIENS, buildPoints, buildable ) )
      reason = IBE_NOASSERT;
  }
  else if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
  {
    //human criteria
    if( !G_IsPowered( entity_origin ) )
    {
      //tell player to build a repeater to provide power
      if( buildable != BA_H_REACTOR && buildable != BA_H_REPEATER )
        reason = IBE_REPEATER;
    }

    //this buildable requires a DCC
    if( BG_FindDCCTestForBuildable( buildable ) && !G_IsDCCBuilt( ) )
      reason = IBE_NODCC;

    //check that there is a parent reactor when building a repeater
    if( buildable == BA_H_REPEATER )
    {
      for ( i = 1, tempent = g_entities + i; i < level.num_entities; i++, tempent++ )
      {
        if( tempent->s.eType != ET_BUILDABLE )
          continue;

        if( tempent->s.modelindex == BA_H_REACTOR )
          break;
      }

      if( i >= level.num_entities )
      {
        //no reactor present

        //check for other nearby repeaters
        for ( i = 1, tempent = g_entities + i; i < level.num_entities; i++, tempent++ )
        {
          if( tempent->s.eType != ET_BUILDABLE )
            continue;

          if( tempent->s.modelindex == BA_H_REPEATER &&
              Distance( tempent->s.origin, entity_origin ) < REPEATER_BASESIZE )
          {
            reason = IBE_RPTWARN2;
            break;
          }
        }

        if( reason == IBE_NONE )
          reason = IBE_RPTWARN;
      }
      else if( G_IsPowered( entity_origin ) )
        reason = IBE_RPTWARN2;
    }

    //check permission to build here
    if( tr1.surfaceFlags & SURF_NOHUMANBUILD || tr1.surfaceFlags & SURF_NOBUILD ||
        contents & CONTENTS_NOHUMANBUILD || contents & CONTENTS_NOBUILD )
      reason = IBE_PERMISSION;

    //can we only build one of these?
    if( BG_FindUniqueTestForBuildable( buildable ) )
    {
      for ( i = 1, tempent = g_entities + i; i < level.num_entities; i++, tempent++ )
      {
        if( tempent->s.eType != ET_BUILDABLE )
          continue;

        if( tempent->s.modelindex == BA_H_REACTOR && !tempent->deconstruct )
        {
          reason = IBE_REACTOR;
          break;
        }
        if( tempent->s.modelindex == buildable )
          level.markedBuildables[ level.numBuildablesForRemoval++ ] = tempent;	
      }
    }

    if( !G_SufficientBPAvailable( BIT_HUMANS, buildPoints, buildable ) )
      reason = IBE_NOPOWER;
  }

  //this item does not fit here
  if( reason == IBE_NONE && ( tr2.fraction < 1.0 || tr3.fraction < 1.0 ) )
    return IBE_NOROOM;

  return reason;
}


/*
================
G_Build

Spawns a buildable
================
*/
static gentity_t *G_Build( gentity_t *builder, buildable_t buildable, vec3_t origin, vec3_t angles )
{
  gentity_t *built;
  vec3_t    normal;

  // Free existing buildables
  G_FreeMarkedBuildables( );

  //spawn the buildable
  built = G_Spawn();

  built->s.eType = ET_BUILDABLE;

  built->classname = BG_FindEntityNameForBuildable( buildable );

  built->s.modelindex = buildable; //so we can tell what this is on the client side
  built->biteam = built->s.modelindex2 = BG_FindTeamForBuildable( buildable );

  BG_FindBBoxForBuildable( buildable, built->r.mins, built->r.maxs );

  // detect the buildable's normal vector
  if( !builder->client )
  {
    // initial base layout created by server

    if( builder->s.origin2[ 0 ]
      || builder->s.origin2[ 1 ]
      || builder->s.origin2[ 2 ] )
    {
      VectorCopy( builder->s.origin2, normal );
    }
    else if( BG_FindTrajectoryForBuildable( buildable ) == TR_BUOYANCY )
      VectorSet( normal, 0.0f, 0.0f, -1.0f );
    else 
      VectorSet( normal, 0.0f, 0.0f, 1.0f );
  } 
  else
  {
    // in-game building by a player

    if( builder->client->ps.stats[ STAT_STATE ] & SS_WALLCLIMBING )
    {
      if( builder->client->ps.stats[ STAT_STATE ] & SS_WALLCLIMBINGCEILING )
        VectorSet( normal, 0.0f, 0.0f, -1.0f );
      else
        VectorCopy( builder->client->ps.grapplePoint, normal );
    }
    else
      VectorSet( normal, 0.0f, 0.0f, 1.0f );
  }

  // when building the initial layout, spawn the entity slightly off its
  // target surface so that it can be "dropped" onto it 
  if( !builder->client )
    VectorMA( origin, 1.0f, normal, origin );

  built->health = 1;

  built->splashDamage = BG_FindSplashDamageForBuildable( buildable );
  built->splashRadius = BG_FindSplashRadiusForBuildable( buildable );
  built->splashMethodOfDeath = BG_FindMODForBuildable( buildable );

  built->nextthink = BG_FindNextThinkForBuildable( buildable );

  built->takedamage = qtrue;
  built->spawned = qfalse;
  built->buildTime = built->s.time = level.time;

  // build instantly in cheat mode
  if( builder->client && g_cheats.integer )
  {
    built->health = BG_FindHealthForBuildable( buildable );
    built->buildTime = built->s.time =
      level.time - BG_FindBuildTimeForBuildable( buildable );
  }

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

    case BA_A_HIVE:
      built->die = ABarricade_Die;
      built->think = AHive_Think;
      built->pain = ASpawn_Pain;
      break;

    case BA_A_TRAPPER:
      built->die = ABarricade_Die;
      built->think = ATrapper_Think;
      built->pain = ASpawn_Pain;
      break;

    case BA_A_OVERMIND:
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

    case BA_H_SPAWN:
      built->die = HSpawn_Die;
      built->think = HSpawn_Think;
      break;

    case BA_H_MGTURRET:
      built->die = HSpawn_Die;
      built->think = HMGTurret_Think;
      break;

    case BA_H_TESLAGEN:
      built->die = HSpawn_Die;
      built->think = HTeslaGen_Think;
      break;

    case BA_H_ARMOURY:
      built->think = HArmoury_Think;
      built->die = HSpawn_Die;
      built->use = HArmoury_Activate;
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
      built->think = HReactor_Think;
      built->die = HSpawn_Die;
      built->use = HRepeater_Use;
      built->powered = built->active = qtrue;
      break;

    case BA_H_REPEATER:
      built->think = HRepeater_Think;
      built->die = HSpawn_Die;
      built->use = HRepeater_Use;
      built->count = -1;
      break;

    default:
      //erk
      break;
  }

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
  
  // gently nudge the buildable onto the surface :)
  VectorScale( normal, -50.0f, built->s.pos.trDelta );

  // set turret angles
  VectorCopy( builder->s.angles2, built->s.angles2 );

  VectorCopy( angles, built->s.angles );
  built->s.angles[ PITCH ] = 0.0f;
  built->s.angles2[ YAW ] = angles[ YAW ];
  built->s.pos.trType = BG_FindTrajectoryForBuildable( buildable );
  built->s.pos.trTime = level.time;
  built->physicsBounce = BG_FindBounceForBuildable( buildable );
  built->s.groundEntityNum = -1;

  built->s.generic1 = (int)( ( (float)built->health /
        (float)BG_FindHealthForBuildable( buildable ) ) * B_HEALTH_MASK );

  if( built->s.generic1 < 0 )
    built->s.generic1 = 0;

  if( ( built->powered = G_FindPower( built ) ) )
    built->s.generic1 |= B_POWERED_TOGGLEBIT;

  if( ( built->dcced = G_FindDCC( built ) ) )
    built->s.generic1 |= B_DCCED_TOGGLEBIT;

  built->s.generic1 &= ~B_SPAWNED_TOGGLEBIT;

  VectorCopy( normal, built->s.origin2 );

  G_AddEvent( built, EV_BUILD_CONSTRUCT, 0 );

  G_SetIdleBuildableAnim( built, BG_FindAnimForBuildable( buildable ) );

  if( built->builtBy >= 0 )
    G_SetBuildableAnim( built, BANIM_CONSTRUCT1, qtrue );

  trap_LinkEntity( built );

  return built;
}

/*
=================
G_BuildIfValid
=================
*/
qboolean G_BuildIfValid( gentity_t *ent, buildable_t buildable )
{
  float         dist;
  vec3_t        origin;

  dist = BG_FindBuildDistForClass( ent->client->ps.stats[ STAT_PCLASS ] );

  switch( G_CanBuild( ent, buildable, dist, origin ) )
  {
    case IBE_NONE:
      G_Build( ent, buildable, origin, ent->s.apos.trBase );
      return qtrue;

    case IBE_NOASSERT:
      G_TriggerMenu( ent->client->ps.clientNum, MN_A_NOASSERT );
      return qfalse;

    case IBE_NOOVERMIND:
      G_TriggerMenu( ent->client->ps.clientNum, MN_A_NOOVMND );
      return qfalse;

    case IBE_NOCREEP:
      G_TriggerMenu( ent->client->ps.clientNum, MN_A_NOCREEP );
      return qfalse;

    case IBE_OVERMIND:
      G_TriggerMenu( ent->client->ps.clientNum, MN_A_OVERMIND );
      return qfalse;

    case IBE_HOVEL:
      G_TriggerMenu( ent->client->ps.clientNum, MN_A_HOVEL );
      return qfalse;

    case IBE_HOVELEXIT:
      G_TriggerMenu( ent->client->ps.clientNum, MN_A_HOVEL_EXIT );
      return qfalse;

    case IBE_NORMAL:
      if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
        G_TriggerMenu( ent->client->ps.clientNum, MN_H_NORMAL );
      else
        G_TriggerMenu( ent->client->ps.clientNum, MN_A_NORMAL );
      return qfalse;

    case IBE_PERMISSION:
      if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
        G_TriggerMenu( ent->client->ps.clientNum, MN_H_NORMAL );
      else
        G_TriggerMenu( ent->client->ps.clientNum, MN_A_NORMAL );
      return qfalse;

    case IBE_REACTOR:
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_REACTOR );
      return qfalse;

    case IBE_REPEATER:
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_REPEATER );
      return qfalse;

    case IBE_NOROOM:
      if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
        G_TriggerMenu( ent->client->ps.clientNum, MN_H_NOROOM );
      else
        G_TriggerMenu( ent->client->ps.clientNum, MN_A_NOROOM );
      return qfalse;

    case IBE_NOPOWER:
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_NOPOWER );
      return qfalse;

    case IBE_NODCC:
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_NODCC );
      return qfalse;

    case IBE_SPWNWARN:
      G_TriggerMenu( ent->client->ps.clientNum, MN_A_SPWNWARN );
      G_Build( ent, buildable, origin, ent->s.apos.trBase );
      return qtrue;

    case IBE_TNODEWARN:
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_TNODEWARN );
      G_Build( ent, buildable, origin, ent->s.apos.trBase );
      return qtrue;

    case IBE_RPTWARN:
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_RPTWARN );
      G_Build( ent, buildable, origin, ent->s.apos.trBase );
      return qtrue;

    case IBE_RPTWARN2:
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_RPTWARN2 );
      return qfalse;

    default:
      break;
  }

  return qfalse;
}

/*
================
G_FinishSpawningBuildable

Traces down to find where an item should rest, instead of letting them
free fall from their spawn points
================
*/
static void G_FinishSpawningBuildable( gentity_t *ent )
{
  trace_t     tr;
  vec3_t      dest;
  gentity_t   *built;
  buildable_t buildable = ent->s.modelindex;

  built = G_Build( ent, buildable, ent->s.pos.trBase, ent->s.angles );
  G_FreeEntity( ent );

  built->takedamage = qtrue;
  built->spawned = qtrue; //map entities are already spawned
  built->health = BG_FindHealthForBuildable( buildable );
  built->s.generic1 |= B_SPAWNED_TOGGLEBIT;

  // drop towards normal surface
  VectorScale( built->s.origin2, -4096.0f, dest );
  VectorAdd( dest, built->s.origin, dest );

  trap_Trace( &tr, built->s.origin, built->r.mins, built->r.maxs, dest, built->s.number, built->clipmask );

  if( tr.startsolid )
  {
    G_Printf( S_COLOR_YELLOW "G_FinishSpawningBuildable: %s startsolid at %s\n",
              built->classname, vtos( built->s.origin ) );
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
  ent->think = G_FinishSpawningBuildable;
}

/*
============
G_LayoutSave

============
*/
void G_LayoutSave( char *name )
{
  char map[ MAX_QPATH ];
  char fileName[ MAX_OSPATH ];
  fileHandle_t f;
  int len;
  int i;
  gentity_t *ent;
  char *s;

  trap_Cvar_VariableStringBuffer( "mapname", map, sizeof( map ) );
  if( !map[ 0 ] )
  {
    G_Printf( "LayoutSave( ): no map is loaded\n" );
    return;
  }
  Com_sprintf( fileName, sizeof( fileName ), "layouts/%s/%s.dat", map, name );

  len = trap_FS_FOpenFile( fileName, &f, FS_WRITE );
  if( len < 0 )
  {
    G_Printf( "layoutsave: could not open %s\n", fileName );
    return;
  }

  G_Printf("layoutsave: saving layout to %s\n", fileName );

  for( i = MAX_CLIENTS; i < level.num_entities; i++ )
  {
    ent = &level.gentities[ i ];
    if( ent->s.eType != ET_BUILDABLE )
      continue;

    s = va( "%i %f %f %f %f %f %f %f %f %f %f %f %f\n",
      ent->s.modelindex,
      ent->s.pos.trBase[ 0 ],
      ent->s.pos.trBase[ 1 ],
      ent->s.pos.trBase[ 2 ],
      ent->s.angles[ 0 ],
      ent->s.angles[ 1 ],
      ent->s.angles[ 2 ],
      ent->s.origin2[ 0 ],
      ent->s.origin2[ 1 ],
      ent->s.origin2[ 2 ],
      ent->s.angles2[ 0 ],
      ent->s.angles2[ 1 ],
      ent->s.angles2[ 2 ] );
    trap_FS_Write( s, strlen( s ), f );
  }
  trap_FS_FCloseFile( f );
}

int G_LayoutList( const char *map, char *list, int len )
{
  // up to 128 single character layout names could fit in layouts
  char fileList[ ( MAX_CVAR_VALUE_STRING / 2 ) * 5 ] = {""};
  char layouts[ MAX_CVAR_VALUE_STRING ] = {""};
  int numFiles, i, fileLen = 0, listLen;
  int  count = 0;
  char *filePtr;

  Q_strcat( layouts, sizeof( layouts ), "*BUILTIN* " );  
  numFiles = trap_FS_GetFileList( va( "layouts/%s", map ), ".dat",
    fileList, sizeof( fileList ) );
  filePtr = fileList;
  for( i = 0; i < numFiles; i++, filePtr += fileLen + 1 )
  {
    fileLen = strlen( filePtr );
    listLen = strlen( layouts );
    if( fileLen < 5 )
      continue; 

    // list is full, stop trying to add to it
    if( ( listLen + fileLen ) >= sizeof( layouts ) )
      break;

    Q_strcat( layouts,  sizeof( layouts ), filePtr );
    listLen = strlen( layouts );

    // strip extension and add space delimiter
    layouts[ listLen - 4 ] = ' ';
    layouts[ listLen - 3 ] = '\0';
    count++;
  }
  if( count != numFiles )
  {
    G_Printf( S_COLOR_YELLOW "WARNING: layout list was truncated to %d "
      "layouts, but %d layout files exist in layouts/%s/.\n",
      count, numFiles, map );
  }
  Q_strncpyz( list, layouts, len );
  return count + 1;
}

/*
============
G_LayoutSelect

set level.layout based on g_layouts or g_layoutAuto
============
*/
void G_LayoutSelect( void )
{
  char fileName[ MAX_OSPATH ];
  char layouts[ MAX_CVAR_VALUE_STRING ];
  char layouts2[ MAX_CVAR_VALUE_STRING ];
  char *l; 
  char map[ MAX_QPATH ];
  char *s;
  int cnt = 0;
  int layoutNum;

  Q_strncpyz( layouts, g_layouts.string, sizeof( layouts ) );
  trap_Cvar_VariableStringBuffer( "mapname", map, sizeof( map ) );

  // one time use cvar 
  trap_Cvar_Set( "g_layouts", "" );
 
  // pick an included layout at random if no list has been provided 
  if( !layouts[ 0 ] && g_layoutAuto.integer )
  {
    G_LayoutList( map, layouts, sizeof( layouts ) );
  }

  if( !layouts[ 0 ] )
    return;

  Q_strncpyz( layouts2, layouts, sizeof( layouts2 ) );
  l = &layouts2[ 0 ];
  layouts[ 0 ] = '\0';
  s = COM_ParseExt( &l, qfalse );
  while( *s )
  {
    if( !Q_stricmp( s, "*BUILTIN*" ) )
    {
      Q_strcat( layouts, sizeof( layouts ), s );
      Q_strcat( layouts, sizeof( layouts ), " " );
      cnt++;
      s = COM_ParseExt( &l, qfalse );
      continue;
    }
    
    Com_sprintf( fileName, sizeof( fileName ), "layouts/%s/%s.dat", map, s );
    if( trap_FS_FOpenFile( fileName, NULL, FS_READ ) > 0 )
    {
      Q_strcat( layouts, sizeof( layouts ), s );
      Q_strcat( layouts, sizeof( layouts ), " " );
      cnt++;
    }
    else
      G_Printf( S_COLOR_YELLOW "WARNING: layout \"%s\" does not exist\n", s );
    s = COM_ParseExt( &l, qfalse );
  }
  if( !cnt )
  {
      G_Printf( S_COLOR_RED "ERROR: none of the specified layouts could be "
        "found, using map default\n" );
      return;
  }
  layoutNum = ( rand( ) & ( cnt - 1 ) ) + 1;
  cnt = 0;

  Q_strncpyz( layouts2, layouts, sizeof( layouts2 ) );
  l = &layouts2[ 0 ];
  s = COM_ParseExt( &l, qfalse );
  while( *s )
  {
    Q_strncpyz( level.layout, s, sizeof( level.layout ) );
    cnt++;
    if( cnt >= layoutNum )
      break;
    s = COM_ParseExt( &l, qfalse );
  }
  G_Printf("using layout \"%s\" from list ( %s)\n", level.layout, layouts ); 
}

static void G_LayoutBuildItem( buildable_t buildable, vec3_t origin,
  vec3_t angles, vec3_t origin2, vec3_t angles2 )
{
  gentity_t *builder;
  
  builder = G_Spawn( );
  builder->client = 0;
  VectorCopy( origin, builder->s.pos.trBase );
  VectorCopy( angles, builder->s.angles );
  VectorCopy( origin2, builder->s.origin2 );
  VectorCopy( angles2, builder->s.angles2 );
  G_SpawnBuildable( builder, buildable );
}

/*
============
G_LayoutLoad

load the layout .dat file indicated by level.layout and spawn buildables
as if a builder was creating them
============
*/
void G_LayoutLoad( void )
{
  fileHandle_t f;
  int len;
  char *layout;
  char map[ MAX_QPATH ];
  int buildable = BA_NONE;
  vec3_t origin = { 0.0f, 0.0f, 0.0f };
  vec3_t angles = { 0.0f, 0.0f, 0.0f };
  vec3_t origin2 = { 0.0f, 0.0f, 0.0f };
  vec3_t angles2 = { 0.0f, 0.0f, 0.0f };
  char line[ MAX_STRING_CHARS ];
  int i = 0;

  if( !level.layout[ 0 ] || !Q_stricmp( level.layout, "*BUILTIN*" ) )
    return;
 
  trap_Cvar_VariableStringBuffer( "mapname", map, sizeof( map ) );
  len = trap_FS_FOpenFile( va( "layouts/%s/%s.dat", map, level.layout ),
    &f, FS_READ );
  if( len < 0 )
  {
    G_Printf( "ERROR: layout %s could not be opened\n", level.layout );
    return;
  }
  layout = G_Alloc( len + 1 );
  trap_FS_Read( layout, len, f );
  *( layout + len ) = '\0';
  trap_FS_FCloseFile( f );
  while( *layout )
  {
    if( i >= sizeof( line ) - 1 )
    {
      G_Printf( S_COLOR_RED "ERROR: line overflow in %s before \"%s\"\n",
       va( "layouts/%s/%s.dat", map, level.layout ), line );
      return; 
    }
    line[ i++ ] = *layout;
    line[ i ] = '\0';
    if( *layout == '\n' )
    {
      i = 0; 
      sscanf( line, "%d %f %f %f %f %f %f %f %f %f %f %f %f\n",
        &buildable,
        &origin[ 0 ], &origin[ 1 ], &origin[ 2 ],
        &angles[ 0 ], &angles[ 1 ], &angles[ 2 ],
        &origin2[ 0 ], &origin2[ 1 ], &origin2[ 2 ],
        &angles2[ 0 ], &angles2[ 1 ], &angles2[ 2 ] );

      if( buildable > BA_NONE && buildable < BA_NUM_BUILDABLES )
        G_LayoutBuildItem( buildable, origin, angles, origin2, angles2 );
      else
        G_Printf( S_COLOR_YELLOW "WARNING: bad buildable number (%d) in "
          " layout.  skipping\n", buildable );
    }
    layout++;
  }
}

