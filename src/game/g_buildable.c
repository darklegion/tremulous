/*
 *  Portions Copyright (C) 2000-2001 Tim Angus
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*  To assertain which portions are licensed under the LGPL and which are
 *  licensed by Id Software, Inc. please run a diff between the equivalent
 *  versions of the "Tremulous" modification and the unmodified "Quake3"
 *  game source code.
 */
                  
#include "g_local.h"

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

  if( self->parentNode && self->parentNode->active )
    return qtrue;
  
  //reset parent
  self->parentNode = NULL;
  
  for ( i = 1, ent = g_entities + i; i < level.num_entities; i++, ent++ )
  {
    if( !ent->classname )
      continue;

    if( !Q_stricmp( ent->classname, "team_human_reactor" ) ||
        !Q_stricmp( ent->classname, "team_human_repeater" ) )
    {
      VectorSubtract( self->s.origin, ent->s.origin, temp_v );
      distance = VectorLength( temp_v );
      if( distance < minDistance )
      {
        closestPower = ent;
        minDistance = distance;
        foundPower = qtrue;
      }
    }
  }

  if( !foundPower )
    return qfalse;
  
  if( (
        !Q_stricmp( closestPower->classname, "team_human_reactor" ) &&
        ( minDistance <= REACTOR_BASESIZE )
      ) || 
      (
        !Q_stricmp( closestPower->classname, "team_human_repeater" ) &&
        !Q_stricmp( self->classname, "team_human_spawn" ) &&
        ( minDistance <= REPEATER_BASESIZE ) &&
        closestPower->powered 
      ) ||
      (
        !Q_stricmp( closestPower->classname, "team_human_repeater" ) &&
        ( minDistance <= REPEATER_BASESIZE ) &&
        closestPower->active &&
        closestPower->powered 
      )
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
nullDieFunction

hack to prevent compilers complaining about function pointer -> NULL conversion
================
*/
void nullDieFunction( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
}

/*
================
D_CreepRecede

Called when an droid spawn dies
================
*/
void D_CreepRecede( gentity_t *self )
{
  if( ( self->timestamp + 100 ) == level.time )
    G_AddEvent( self, EV_ITEM_RECEDE, 0 );
  
  if( ( self->timestamp + 10000 ) > level.time )
  {
    self->nextthink = level.time + 500;
    trap_LinkEntity( self );
  }
  else
    G_FreeEntity( self );
}


/*
================
DSpawn_Melt

Called when an droid spawn dies
================
*/
void DSpawn_Melt( gentity_t *self )
{
  //FIXME: this line crashes the QVM (but not binary when MOD is set to MOD_[H/D]SPAWN
  G_SelectiveRadiusDamage( self->s.pos.trBase, self->parent, self->splashDamage,
    self->splashRadius, self, MOD_SHOTGUN, PTE_DROIDS );

  
  if( ( self->timestamp + 500 ) == level.time )
    G_AddEvent( self, EV_ITEM_RECEDE, 0 );
  
  if( ( self->timestamp + 10000 ) > level.time )
  {
    self->nextthink = level.time + 500;
    trap_LinkEntity( self );
  }
  else
    G_FreeEntity( self );
}

/*
================
DSpawn_Die

Called when an droid spawn dies
================
*/
void DSpawn_Die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
  vec3_t  dir;

  // we don't have a valid direction, so just point straight up
  dir[0] = dir[1] = 0;
  dir[2] = 1;

  G_SelectiveRadiusDamage( self->s.pos.trBase, self->parent, self->splashDamage,
    self->splashRadius, self, self->splashMethodOfDeath, PTE_DROIDS );

  self->s.modelindex = 0; //don't draw the model once its destroyed
  G_AddEvent( self, EV_GIB_DROID, DirToByte( dir ) );
  self->r.contents = CONTENTS_TRIGGER;
  self->timestamp = level.time;
  self->die = nullDieFunction;
  self->think = DSpawn_Melt;
  self->nextthink = level.time + 500; //wait .5 seconds before damaging others
    
  trap_LinkEntity( self );
}


/*
================
DDef1_Die

Called when an droid spawn dies
================
*/
void DDef1_Die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
  vec3_t  dir;
  
  // we don't have a valid direction, so just point straight up
  dir[0] = dir[1] = 0;
  dir[2] = 1;

  G_SelectiveRadiusDamage( self->s.pos.trBase, self->parent, self->splashDamage,
    self->splashRadius, self, self->splashMethodOfDeath, PTE_DROIDS );

  self->s.modelindex = 0; //don't draw the model once its destroyed
  G_AddEvent( self, EV_GIB_DROID, DirToByte( dir ) );
  self->r.contents = CONTENTS_TRIGGER;
  self->timestamp = level.time;
  self->die = nullDieFunction;
  self->think = D_CreepRecede;
  self->nextthink = level.time + 100;

  trap_LinkEntity( self );
}


/*
================
DSpawn_Think

think function for Droid Spawn
================
*/
void DSpawn_Think( gentity_t *self )
{
}


/*
================
DDef1_Think

think function for Droid Spawn
================
*/
void DDef1_Think( gentity_t *self )
{
  int       i;
  gentity_t *ent;
  gentity_t *closestSpawn;
  int       distance = 0;
  int       minDistance = 10000;
  vec3_t    temp_v;
  vec3_t    dir = { 0, 0, 1 }; //up
  
  if( ( self->parentNode == NULL ) || !self->parentNode->inuse )
  {
    for ( i = 1, ent = g_entities + i; i < level.num_entities; i++, ent++ )
    {
      if( !Q_stricmp( ent->classname, "team_droid_spawn" ) ||
          !Q_stricmp( ent->classname, "team_droid_hivemind" ) )
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
    
    if( minDistance <= ( CREEP_BASESIZE * 3 ) )
      self->parentNode = closestSpawn;
    else
    {
      G_Damage( self, NULL, NULL, NULL, NULL, 10000, 0, MOD_SUICIDE );
      return;
    }
  }
  
  G_SelectiveRadiusDamage( self->s.pos.trBase, self->parent, self->splashDamage,
    self->splashRadius, self, self->splashMethodOfDeath, PTE_DROIDS );

  self->nextthink = level.time + 100;
}

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
  
  for ( i = 1, ent = g_entities + i; i < level.num_entities; i++, ent++ )
  {
    if( !Q_stricmp( ent->classname, "team_human_spawn" ) && ent->parentNode == self )
      count++;
      
    if( !Q_stricmp( ent->classname, "team_human_reactor" ) )
      reactor = qtrue;
  }
  
  if( count && reactor )
    self->active = qtrue;
  else
    self->active = qfalse;

  self->powered = reactor;

  self->nextthink = level.time + REFRESH_TIME;
}

/*
================
HMCU_Activate

Called when a human activates an MCU
================
*/
void HMCU_Activate( gentity_t *self, gentity_t *other, gentity_t *activator )
{
  if( activator->client->ps.stats[ STAT_PTEAM ] != PTE_HUMANS ) return;
  
  if( self->powered )
    G_AddPredictableEvent( activator, EV_MENU, MN_H_MCU );
  else
    G_AddPredictableEvent( activator, EV_MENU, MN_H_MCUPOWER );
}

/*
================
HMCU_Think

Think for mcu
================
*/
void HMCU_Think( gentity_t *self )
{
  self->nextthink = level.time + REFRESH_TIME;
  
  self->powered = findPower( self );
}

//TA: the following defense turret code was written by
// "fuzzysteve"           (fuzzysteve@quakefiles.com) and
// Anthony "inolen" Pesch (www.inolen.com)
//with modifications by me of course :)
#define HDEF1_RANGE             500
#define HDEF1_ANGULARSPEED      15
#define HDEF1_FIRINGSPEED       200
#define HDEF1_ACCURACYTOLERANCE HDEF1_ANGULARSPEED - 5
#define HDEF1_VERTICALCAP       20

/*
================
hdef1_trackenemy

Used by HDef1_Think to track enemy location
================
*/
qboolean hdef1_trackenemy( gentity_t *self )
{
  vec3_t  dirToTarget, angleToTarget, angularDiff;
  float   temp;

  VectorSubtract( self->enemy->s.pos.trBase, self->s.pos.trBase, dirToTarget );
  VectorNormalize( dirToTarget );
  vectoangles( dirToTarget, angleToTarget );

  angularDiff[ PITCH ] = AngleSubtract( self->turloc[ PITCH ], angleToTarget[ PITCH ] );
  angularDiff[ YAW ] = AngleSubtract( self->turloc[ YAW ], angleToTarget[ YAW ] );

  if( angularDiff[ PITCH ] < -HDEF1_ACCURACYTOLERANCE )
    self->turloc[ PITCH ] += HDEF1_ANGULARSPEED;
  else if( angularDiff[ PITCH ] > HDEF1_ACCURACYTOLERANCE )
    self->turloc[ PITCH ] -= HDEF1_ANGULARSPEED;
  else
    self->turloc[ PITCH ] = angleToTarget[ PITCH ];

  temp = fabs( self->turloc[ PITCH ] );
  if( temp > 180 )
    temp -= 360;
  
  if( temp < -HDEF1_VERTICALCAP )
    self->turloc[ PITCH ] = (-360)+HDEF1_VERTICALCAP;
  else if( temp > HDEF1_VERTICALCAP )
    self->turloc[ PITCH ] = -HDEF1_VERTICALCAP;
    
  if( angularDiff[ YAW ] < -HDEF1_ACCURACYTOLERANCE )
    self->turloc[ YAW ] += HDEF1_ANGULARSPEED;
  else if( angularDiff[ YAW ] > HDEF1_ACCURACYTOLERANCE )
    self->turloc[ YAW ] -= HDEF1_ANGULARSPEED;
  else
    self->turloc[ YAW ] = angleToTarget[ YAW ];
    
  VectorCopy( self->turloc, self->s.angles2 );
  
  trap_LinkEntity( self );

  if( abs( angleToTarget[ YAW ] - self->turloc[ YAW ] ) <= HDEF1_ACCURACYTOLERANCE &&
      abs( angleToTarget[ PITCH ] - self->turloc[ PITCH ] ) <= HDEF1_ACCURACYTOLERANCE )
    return qtrue;
    
  return qfalse;
}

/*
================
hdef1_fireonemeny

Used by HDef1_Think to fire at enemy
================
*/
void hdef1_fireonenemy( gentity_t *self )
{
  vec3_t  aimVector;
  
  AngleVectors( self->turloc, aimVector, NULL, NULL );
  fire_plasma( self, self->s.pos.trBase, aimVector );
  self->count = level.time + HDEF1_FIRINGSPEED;
}

/*
================
hdef1_checktarget

Used by HDef1_Think to check enemies for validity
================
*/
qboolean hdef1_checktarget(gentity_t *self, gentity_t *target)
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
  if( target->health <= 0 ) // is the target still alive?
    return qfalse;

  VectorSubtract( target->r.currentOrigin, self->r.currentOrigin, distance );
  if( VectorLength( distance ) > HDEF1_RANGE ) // is the target within range?
    return qfalse;

  trap_Trace( &trace, self->s.pos.trBase, NULL, NULL, target->s.pos.trBase, self->s.number, MASK_SHOT );
  if ( trace.contents & CONTENTS_SOLID ) // can we see the target?
    return qfalse;

  return qtrue;
}


/*
================
hdef1_findenemy

Used by HDef1_Think to locate enemy gentities
================
*/
void hdef1_findenemy( gentity_t *ent )
{
  gentity_t *target;

  target = g_entities;

  for (; target < &g_entities[ level.num_entities ]; target++)
  {
    if( !hdef1_checktarget( ent, target ) )
      continue;
      
    ent->enemy = target;
    return;
  }

  ent->enemy = NULL;
}


/*
================
HDef1_Think

think function for Human Defense
================
*/
void HDef1_Think( gentity_t *self )
{
  self->nextthink = level.time + 50;
  
  self->powered = findPower( self );

  if( !self->powered )
  {
    self->nextthink = level.time + REFRESH_TIME;
    return;
  }
  
  if( !hdef1_checktarget( self, self->enemy) )
    hdef1_findenemy( self );
  if( !self->enemy )
    return;

  if( hdef1_trackenemy( self ) && ( self->count < level.time ) )
    hdef1_fireonenemy( self );
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
  G_RadiusDamage( self->s.pos.trBase, self->parent, self->splashDamage,
    self->splashRadius, self, self->splashMethodOfDeath );

  G_FreeEntity( self );
}


/*
================
HSpawn_die

Called when a human spawn dies
================
*/
void HSpawn_Die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
  vec3_t  dir;

  // we don't have a valid direction, so just point straight up
  dir[0] = dir[1] = 0;
  dir[2] = 1;

  self->s.modelindex = 0; //don't draw the model once its destroyed
  G_AddEvent( self, EV_ITEM_EXPLOSION, DirToByte( dir ) );
  self->r.contents = CONTENTS_TRIGGER;
  self->timestamp = level.time;
  self->die = nullDieFunction;
  self->think = HSpawn_Blast;
  self->nextthink = level.time + 1000; //wait 1.5 seconds before damaging others

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
  self->nextthink = level.time + REFRESH_TIME;

  self->powered = findPower( self );
}


/*
================
itemFits

Checks to see if an item fits in a specific area
================
*/
itemBuildError_t itemFits( gentity_t *ent, buildable_t buildable, int distance )
{
  vec3_t            forward;
  vec3_t            angles;
  vec3_t            player_origin, entity_origin;
  vec3_t            mins, maxs;
  vec3_t            temp_v;
  trace_t           tr1, tr2;
  int               i;
  itemBuildError_t  reason = IBE_NONE;
  gentity_t         *tempent, *closestPower;
  int               minDistance, templength;

  VectorCopy( ent->s.apos.trBase, angles );
  angles[PITCH] = 0;  // always forward

  AngleVectors( angles, forward, NULL, NULL );
  VectorCopy( ent->s.pos.trBase, player_origin );
  VectorMA( player_origin, distance, forward, entity_origin );

  BG_FindBBoxForBuildable( buildable, mins, maxs );
  
  trap_Trace( &tr1, entity_origin, mins, maxs, entity_origin, ent->s.number, MASK_PLAYERSOLID );
  trap_Trace( &tr2, player_origin, NULL, NULL, entity_origin, ent->s.number, MASK_PLAYERSOLID );

  if( tr1.fraction < 1.0 || tr2.fraction < 1.0 )
    reason = IBE_NOROOM;
    
  if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_DROIDS )
  {
    //droid criteria
    if( level.droidBuildPoints - BG_FindBuildPointsForBuildable( buildable ) < 0 )
      reason = IBE_NOASSERT;
      
    if( BG_FindCreepTestForBuildable( buildable ) )
    {
      for ( i = 1, tempent = g_entities + i; i < level.num_entities; i++, tempent++ )
      {
        if( !Q_stricmp( tempent->classname, "team_droid_spawn" ) ||
            !Q_stricmp( tempent->classname, "team_droid_hivemind" ) )
        {
          VectorSubtract( entity_origin, tempent->s.origin, temp_v );
          if( VectorLength( temp_v ) <= ( CREEP_BASESIZE * 3 ) )
            break;
        }
      }

      if( i >= level.num_entities )
        reason = IBE_NOCREEP;
    }
    
    for ( i = 1, tempent = g_entities + i; i < level.num_entities; i++, tempent++ )
    {
      if( !Q_stricmp( tempent->classname, "team_droid_hivemind" ) )
        break;
    }

    if( i >= level.num_entities && buildable != BA_D_HIVEMIND )
    {
      if( buildable == BA_D_SPAWN )
        reason = IBE_SPWNWARN;
      else
        reason = IBE_NOHIVEMIND;
    }
      
    if( BG_FindUniqueTestForBuildable( buildable ) )
    {
      for ( i = 1, tempent = g_entities + i; i < level.num_entities; i++, tempent++ )
      {
        if( !Q_stricmp( tempent->classname, "team_droid_hivemind" ) )
        {
          reason = IBE_HIVEMIND;
          break;
        }
      }
    }
  }
  else if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
  {
    //human criteria
    if( level.humanBuildPoints - BG_FindBuildPointsForBuildable( buildable ) < 0 )
      reason = IBE_NOPOWER;

    for ( i = 1, tempent = g_entities + i; i < level.num_entities; i++, tempent++ )
    {
      if( !Q_stricmp( tempent->classname, "team_human_reactor" ) ||
          !Q_stricmp( tempent->classname, "team_human_repeater" ) )
      {
        VectorSubtract( entity_origin, tempent->s.origin, temp_v );
        templength = VectorLength( temp_v );
        if( templength < minDistance )
        {
          closestPower = tempent;
          minDistance = templength;
        }
      }
    }
    
    if( !(( !Q_stricmp( closestPower->classname, "team_human_reactor" ) &&
            minDistance <= REACTOR_BASESIZE ) || 
          ( !Q_stricmp( closestPower->classname, "team_human_repeater" ) &&
            minDistance <= REPEATER_BASESIZE &&
            ( ( buildable == BA_H_SPAWN && closestPower->powered ) ||
              ( closestPower->powered && closestPower->active ) ) ) )
      )
    {
      if( buildable != BA_H_REACTOR && buildable != BA_H_REPEATER )
        reason = IBE_REPEATER;

      if( buildable == BA_H_SPAWN )
        reason = IBE_RPLWARN;
    }

    if( buildable == BA_H_REPEATER )
    {
      for ( i = 1, tempent = g_entities + i; i < level.num_entities; i++, tempent++ )
      {
        if( !Q_stricmp( tempent->classname, "team_human_reactor" ) )
          break;
      }
      
      if( i >= level.num_entities )
        reason = IBE_RPTWARN;
    }
      
    if( BG_FindUniqueTestForBuildable( buildable ) )
    {
      for ( i = 1, tempent = g_entities + i; i < level.num_entities; i++, tempent++ )
      {
        if( !Q_stricmp( tempent->classname, "team_human_reactor" ) )
        {
          reason = IBE_REACTOR;
          break;
        }
      }
    }
  }

  return reason;
}


/*
================
Build_Item

Spawns an item and tosses it forward
================
*/
gentity_t *Build_Item( gentity_t *ent, buildable_t buildable, int distance ) {
  vec3_t  forward;
  vec3_t  angles;
  vec3_t  origin;
  gentity_t *built;

  VectorCopy( ent->s.apos.trBase, angles );
  angles[PITCH] = 0;  // always forward

  AngleVectors( angles, forward, NULL, NULL );
  VectorCopy( ent->s.pos.trBase, origin );
  VectorMA( origin, distance, forward, origin );

  built = G_Spawn();

  built->s.eType = ET_BUILDABLE;

  built->classname = BG_FindEntityNameForBuildable( buildable );
  built->item = BG_FindItemForBuildable( buildable );
  
  built->s.modelindex = built->item - bg_itemlist; // store item number in modelindex

  BG_FindBBoxForBuildable( buildable, built->r.mins, built->r.maxs );
  built->biteam = built->s.modelindex2 = BG_FindTeamForBuildable( buildable );
  built->health = BG_FindHealthForBuildable( buildable );
  
  built->damage = BG_FindDamageForBuildable( buildable );
  built->splashDamage = BG_FindSplashDamageForBuildable( buildable );
  built->splashRadius = BG_FindSplashRadiusForBuildable( buildable );
  built->splashMethodOfDeath = BG_FindMODForBuildable( buildable );
  
  if( BG_FindEventForBuildable( buildable ) != EV_NONE )
    G_AddEvent( built, BG_FindEventForBuildable( buildable ), 0 );
    
  built->nextthink = BG_FindNextThinkForBuildable( buildable );

  if( buildable == BA_D_SPAWN )
  {
    built->die = DSpawn_Die;
    built->think = DSpawn_Think;
  }
  else if( buildable == BA_D_DEF1 )
  {
    built->die = DDef1_Die;
    built->think = DDef1_Think;
  }
  else if( buildable == BA_D_HIVEMIND )
  {
    built->die = DSpawn_Die;
  }
  else if( buildable == BA_H_SPAWN )
  {
    built->die = HSpawn_Die;
    built->think = HSpawn_Think;
  }
  else if( buildable == BA_H_DEF1 )
  {
    built->die = HSpawn_Die;
    built->think = HDef1_Think;
    built->enemy = NULL;
  }
  else if( buildable == BA_H_MCU )
  {
    built->think = HMCU_Think;
    built->die = HSpawn_Die;
    built->use = HMCU_Activate;
  }
  else if( buildable == BA_H_REACTOR )
  {
    built->die = HSpawn_Die;
    built->powered = qtrue;
  }
  else if( buildable == BA_H_REPEATER )
  {
    built->think = HRpt_Think;
    built->die = HSpawn_Die;
  }

  built->takedamage = qtrue;
  built->s.number = built - g_entities;
  built->r.contents = CONTENTS_BODY;
  built->clipmask = MASK_PLAYERSOLID;

  G_SetOrigin( built, origin );
  VectorCopy( angles, built->s.angles );
  built->turloc[ YAW ] = built->s.angles2[ YAW ] = angles[ YAW ];
  VectorCopy( origin, built->s.origin );
  built->s.pos.trType = TR_GRAVITY;
  built->s.pos.trTime = level.time;
  
  trap_LinkEntity (built);

  return built;
}

