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
  G_SelectiveRadiusDamage( self->s.pos.trBase, self->parent, 2,
    self->splashRadius, self, self->splashMethodOfDeath, PTE_DROIDS );

  if( ( self->timestamp + 500 ) == level.time )
    G_AddEvent( self, EV_ITEM_RECEDE, 0 );
  
  if( ( self->timestamp + 10000 ) > level.time )
  {
    self->nextthink = level.time + 500;
    trap_LinkEntity( self );
  }
  else
    G_FreeEntity( self );
  
  //update spawn counts
  CalculateRanks( );
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
  /*if( self->parentNode == NULL )
    self->parentNode = createCreepNode( self );

  VectorCopy( self->s.origin, self->parentNode->s.origin );*/
  
  self->nextthink = level.time + 100;
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
      if( !Q_stricmp( ent->classname, "team_droid_spawn" ) )
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

//TA: the following defense turret code was written by
//         "fuzzysteve"     (fuzzysteve@quakefiles.com) and
//   Anthony "inolen" Pesch (www.inolen.com)
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
  //fire_flamer( self, self->s.pos.trBase, aimVector );
  fire_plasma( self, self->s.pos.trBase, aimVector );
  //G_AddEvent( self, EV_FIRE_WEAPON, 0 );
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
  
  //update spawn counts
  CalculateRanks( );
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
itemFits

Checks to see if an item fits in a specific area
================
*/
qboolean itemFits( gentity_t *ent, gitem_t *item, int distance )
{
  vec3_t    forward;
  vec3_t    angles;
  vec3_t    player_origin, entity_origin;
  vec3_t    mins, maxs;
  vec3_t    temp_v;
  trace_t   tr1, tr2;
  int       i;
  gentity_t *creepent;
  qboolean  creeptest;
  qboolean  nearcreep = qfalse;

  VectorCopy( ent->s.apos.trBase, angles );
  angles[PITCH] = 0;  // always forward

  AngleVectors( angles, forward, NULL, NULL );
  VectorCopy( ent->s.pos.trBase, player_origin );
  VectorMA( player_origin, distance, forward, entity_origin );

  if( !Q_stricmp( item->classname, "team_droid_spawn" ) )
  {
    creeptest = qfalse;
    VectorSet( mins, -15, -15, -15 );
    VectorSet( maxs, 15, 15, 15 );
  }
  else if( !Q_stricmp( item->classname, "team_droid_def1" ) )
  {
    creeptest = qtrue;
    VectorSet( mins, -15, -15, -15 );
    VectorSet( maxs, 15, 15, 15 );
  }
  else if( !Q_stricmp( item->classname, "team_human_spawn" ) )
  {
    creeptest = qfalse;
    VectorSet( mins, -40, -40, -4 );
    VectorSet( maxs, 40, 40, 4 );
  }
  else if( !Q_stricmp( item->classname, "team_human_def1" ) )
  {
    creeptest = qfalse;
    VectorSet( mins, -24, -24, -11 );
    VectorSet( maxs, 24, 24, 11 );
  }
  else if( !Q_stricmp( item->classname, "team_human_mcu" ) )
  {
    creeptest = qfalse;
    VectorSet( mins, -15, -15, -15 );
    VectorSet( maxs, 15, 15, 15 );
  }

  trap_Trace( &tr1, entity_origin, mins, maxs, entity_origin, ent->s.number, MASK_PLAYERSOLID );
  trap_Trace( &tr2, player_origin, NULL, NULL, entity_origin, ent->s.number, MASK_PLAYERSOLID );

  if( creeptest )
  {
    for ( i = 1, creepent = g_entities + i; i < level.num_entities; i++, creepent++ )
    {
      if( !Q_stricmp( creepent->classname, "team_droid_spawn" ) )
      {
        VectorSubtract( entity_origin, creepent->s.origin, temp_v );
        if( VectorLength( temp_v ) <= ( CREEP_BASESIZE * 3 ) )
        {
          nearcreep = qtrue;
          break;
        }
      }
    }
  }
  else
    nearcreep = qtrue;

  if( tr1.fraction >= 1.0 && tr2.fraction >= 1.0 && nearcreep )
    return qtrue;
  else
    return qfalse;
}


/*
================
Build_Item

Spawns an item and tosses it forward
================
*/
gentity_t *Build_Item( gentity_t *ent, gitem_t *item, int distance ) {
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
  built->s.modelindex = item - bg_itemlist; // store item number in modelindex

  built->classname = item->classname;
  built->item = item;

  if( !Q_stricmp( item->classname, "team_droid_spawn" ) )
  {
    VectorSet( built->r.mins, -15, -15, -15 );
    VectorSet( built->r.maxs, 15, 15, 15 );

    built->biteam = BIT_DROIDS;
    built->takedamage = qtrue;
    built->health = 1000;
    built->damage = 50;
    built->splashDamage = 50;
    built->splashRadius = 200;
    built->splashMethodOfDeath = MOD_ASPAWN;
    built->s.modelindex2 = BIT_DROIDS;
    G_AddEvent( built, EV_ITEM_GROW, 0 );
    //built->touch = ASpawn_Touch;
    built->die = DSpawn_Die;
    //built->pain = ASpawn_Pain;
    built->think = DSpawn_Think;
    built->nextthink = level.time + 100;
  }
  else if( !Q_stricmp( item->classname, "team_droid_def1" ) )
  {
    VectorSet( built->r.mins, -15, -15, -15 );
    VectorSet( built->r.maxs, 15, 15, 15 );

    built->biteam = BIT_DROIDS;
    built->takedamage = qtrue;
    built->health = 1000;
    built->damage = 50;
    built->splashDamage = 20;
    built->splashRadius = 50;
    built->splashMethodOfDeath = MOD_ASPAWN;
    built->s.modelindex2 = BIT_DROIDS;
    G_AddEvent( built, EV_ITEM_GROW, 0 );
    //built->touch = ASpawn_Touch;
    built->die = DDef1_Die;
    //built->pain = ASpawn_Pain;
    built->think = DDef1_Think;
    built->nextthink = level.time + 100;
  }
  else if( !Q_stricmp( item->classname, "team_human_spawn" ) )
  {
    VectorSet( built->r.mins, -40, -40, -4 );
    VectorSet( built->r.maxs, 40, 40, 4 );

    built->biteam = BIT_HUMANS;
    built->takedamage = qtrue;
    built->health = 1000;
    built->damage = 50;
    built->splashDamage = 50;
    built->splashRadius = 150;
    built->splashMethodOfDeath = MOD_HSPAWN;
    built->s.modelindex2 = BIT_HUMANS;
    //built->touch = HSpawn_Touch;
    //built->think = HSpawn_Think;
    //built->nextthink = level.time + 1000;
    built->die = HSpawn_Die;
    //built->pain = HSpawn_Pain;
  }
  else if( !Q_stricmp( item->classname, "team_human_def1" ) )
  {
    VectorSet( built->r.mins, -24, -24, -11 );
    VectorSet( built->r.maxs, 24, 24, 11 );

    built->biteam = BIT_HUMANS;
    built->takedamage = qtrue;
    built->health = 1000;
    built->damage = 50;
    built->splashDamage = 20;
    built->splashRadius = 50;
    built->splashMethodOfDeath = MOD_HSPAWN;
    built->s.modelindex2 = BIT_HUMANS;
    //built->touch = ASpawn_Touch;
    built->die = HSpawn_Die;
    //built->pain = ASpawn_Pain;
    built->think = HDef1_Think;
    built->enemy = NULL;
    built->nextthink = level.time + 50;
  }
  else if( !Q_stricmp( item->classname, "team_human_mcu" ) )
  {
    VectorSet( built->r.mins, -15, -15, -15 );
    VectorSet( built->r.maxs, 15, 15, 15 );

    built->biteam = BIT_HUMANS;
    built->takedamage = qtrue;
    built->health = 1000;
    built->damage = 50;
    built->splashDamage = 50;
    built->splashRadius = 150;
    built->splashMethodOfDeath = MOD_HSPAWN;
    built->s.modelindex2 = BIT_HUMANS;
    //built->touch = HSpawn_Touch;
    //built->think = HSpawn_Think;
    //built->nextthink = level.time + 1000;
    built->die = HSpawn_Die;
    //built->pain = HSpawn_Pain;
  }

  built->s.number = built - g_entities;
  built->r.contents = CONTENTS_BODY;
  built->clipmask = MASK_PLAYERSOLID;

  G_SetOrigin( built, origin );
  VectorCopy( angles, built->s.angles );
  built->turloc[ YAW ] = built->s.angles2[ YAW ] = angles[ YAW ];
  VectorCopy( origin, built->s.origin );
  built->s.pos.trType = TR_GRAVITY;
  built->s.pos.trTime = level.time;

  //update spawn counts
  CalculateRanks( );
  
  trap_LinkEntity (built);

  return built;
}

