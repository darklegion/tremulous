// Copyright (C) 1999-2000 Id Software, Inc.
//
// g_misc.c

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


/*QUAKED func_group (0 0 0) ?
Used to group brushes together just for editor convenience.  They are turned into normal brushes by the utilities.
*/


/*QUAKED info_camp (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for calculations in the utilities (spotlights, etc), but removed during gameplay.
*/
void SP_info_camp( gentity_t *self ) {
  G_SetOrigin( self, self->s.origin );
}


/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for calculations in the utilities (spotlights, etc), but removed during gameplay.
*/
void SP_info_null( gentity_t *self ) {
  G_FreeEntity( self );
}


/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for in-game calculation, like jumppad targets.
target_position does the same thing
*/
void SP_info_notnull( gentity_t *self ){
  G_SetOrigin( self, self->s.origin );
}


/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) linear
Non-displayed light.
"light" overrides the default 300 intensity.
Linear checbox gives linear falloff instead of inverse square
Lights pointed at a target will be spotlights.
"radius" overrides the default 64 unit radius of a spotlight at the target point.
*/
void SP_light( gentity_t *self ) {
  G_FreeEntity( self );
}



/*
=================================================================================

TELEPORTERS

=================================================================================
*/

void TeleportPlayer( gentity_t *player, vec3_t origin, vec3_t angles ) {
  gentity_t *tent;

  // use temp events at source and destination to prevent the effect
  // from getting dropped by a second player event
  if ( player->client->sess.sessionTeam != TEAM_SPECTATOR ) {
    tent = G_TempEntity( player->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
    tent->s.clientNum = player->s.clientNum;

    tent = G_TempEntity( origin, EV_PLAYER_TELEPORT_IN );
    tent->s.clientNum = player->s.clientNum;
  }

  // unlink to make sure it can't possibly interfere with G_KillBox
  trap_UnlinkEntity (player);

  VectorCopy ( origin, player->client->ps.origin );
  player->client->ps.origin[2] += 1;

  // spit the player out
  AngleVectors( angles, player->client->ps.velocity, NULL, NULL );
  VectorScale( player->client->ps.velocity, 400, player->client->ps.velocity );
  player->client->ps.pm_time = 160;   // hold time
  player->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;

  // toggle the teleport bit so the client knows to not lerp
  player->client->ps.eFlags ^= EF_TELEPORT_BIT;

  // set angles
  SetClientViewAngle( player, angles );

  // kill anything at the destination
  if ( player->client->sess.sessionTeam != TEAM_SPECTATOR ) {
    G_KillBox (player);
  }

  // save results of pmove
  BG_PlayerStateToEntityState( &player->client->ps, &player->s, qtrue );

  // use the precise origin for linking
  VectorCopy( player->client->ps.origin, player->r.currentOrigin );

  if ( player->client->sess.sessionTeam != TEAM_SPECTATOR ) {
    trap_LinkEntity (player);
  }
}


/*QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
Point teleporters at these.
Now that we don't have teleport destination pads, this is just
an info_notnull
*/
void SP_misc_teleporter_dest( gentity_t *ent ) {
}


//===========================================================

/*QUAKED misc_model (1 0 0) (-16 -16 -16) (16 16 16)
"model"   arbitrary .md3 file to display
*/
void SP_misc_model( gentity_t *ent ) {

#if 0
  ent->s.modelindex = G_ModelIndex( ent->model );
  VectorSet (ent->mins, -16, -16, -16);
  VectorSet (ent->maxs, 16, 16, 16);
  trap_LinkEntity (ent);

  G_SetOrigin( ent, ent->s.origin );
  VectorCopy( ent->s.angles, ent->s.apos.trBase );
#else
  G_FreeEntity( ent );
#endif
}

//===========================================================

void locateCamera( gentity_t *ent ) {
  vec3_t    dir;
  gentity_t *target;
  gentity_t *owner;

  owner = G_PickTarget( ent->target );
  if ( !owner ) {
    G_Printf( "Couldn't find target for misc_partal_surface\n" );
    G_FreeEntity( ent );
    return;
  }
  ent->r.ownerNum = owner->s.number;

  // frame holds the rotate speed
  if ( owner->spawnflags & 1 ) {
    ent->s.frame = 25;
  } else if ( owner->spawnflags & 2 ) {
    ent->s.frame = 75;
  }

  // swing camera ?
  if ( owner->spawnflags & 4 ) {
    // set to 0 for no rotation at all
    ent->s.powerups = 0;
  }
  else {
    ent->s.powerups = 1;
  }

  // clientNum holds the rotate offset
  ent->s.clientNum = owner->s.clientNum;

  VectorCopy( owner->s.origin, ent->s.origin2 );

  // see if the portal_camera has a target
  target = G_PickTarget( owner->target );
  if ( target ) {
    VectorSubtract( target->s.origin, owner->s.origin, dir );
    VectorNormalize( dir );
  } else {
    G_SetMovedir( owner->s.angles, dir );
  }

  ent->s.eventParm = DirToByte( dir );
}

/*QUAKED misc_portal_surface (0 0 1) (-8 -8 -8) (8 8 8)
The portal surface nearest this entity will show a view from the targeted misc_portal_camera, or a mirror view if untargeted.
This must be within 64 world units of the surface!
*/
void SP_misc_portal_surface(gentity_t *ent) {
  VectorClear( ent->r.mins );
  VectorClear( ent->r.maxs );
  trap_LinkEntity (ent);

  ent->r.svFlags = SVF_PORTAL;
  ent->s.eType = ET_PORTAL;

  if ( !ent->target ) {
    VectorCopy( ent->s.origin, ent->s.origin2 );
  } else {
    ent->think = locateCamera;
    ent->nextthink = level.time + 100;
  }
}

/*QUAKED misc_portal_camera (0 0 1) (-8 -8 -8) (8 8 8) slowrotate fastrotate noswing

The target for a misc_portal_director.  You can set either angles or target another entity to determine the direction of view.
"roll" an angle modifier to orient the camera around the target vector;
*/
void SP_misc_portal_camera(gentity_t *ent) {
  float roll;

  VectorClear( ent->r.mins );
  VectorClear( ent->r.maxs );
  trap_LinkEntity (ent);

  G_SpawnFloat( "roll", "0", &roll );

  ent->s.clientNum = roll/360.0 * 256;
}

/*
======================================================================

  SHOOTERS

======================================================================
*/

void Use_Shooter( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
  vec3_t    dir;
  float   deg;
  vec3_t    up, right;

  // see if we have a target
  if ( ent->enemy ) {
    VectorSubtract( ent->enemy->r.currentOrigin, ent->s.origin, dir );
    VectorNormalize( dir );
  } else {
    VectorCopy( ent->movedir, dir );
  }

  // randomize a bit
  PerpendicularVector( up, dir );
  CrossProduct( up, dir, right );

  deg = crandom() * ent->random;
  VectorMA( dir, deg, up, dir );

  deg = crandom() * ent->random;
  VectorMA( dir, deg, right, dir );

  VectorNormalize( dir );

/*  switch ( ent->s.weapon ) {
  case WP_GRENADE_LAUNCHER:
    fire_grenade( ent, ent->s.origin, dir );
    break;
  case WP_ROCKET_LAUNCHER:
    fire_rocket( ent, ent->s.origin, dir );
    break;
  }*/

  G_AddEvent( ent, EV_FIRE_WEAPON, 0 );
}


static void InitShooter_Finish( gentity_t *ent ) {
  ent->enemy = G_PickTarget( ent->target );
  ent->think = 0;
  ent->nextthink = 0;
}

void InitShooter( gentity_t *ent, int weapon ) {
  ent->use = Use_Shooter;
  ent->s.weapon = weapon;

  /*RegisterItem( BG_FindItemForWeapon( weapon ) );*/

  G_SetMovedir( ent->s.angles, ent->movedir );

  if ( !ent->random ) {
    ent->random = 1.0;
  }
  ent->random = sin( M_PI * ent->random / 180 );
  // target might be a moving object, so we can't set movedir for it
  if ( ent->target ) {
    ent->think = InitShooter_Finish;
    ent->nextthink = level.time + 500;
  }
  trap_LinkEntity( ent );
}

/*QUAKED shooter_rocket (1 0 0) (-16 -16 -16) (16 16 16)
Fires at either the target or the current direction.
"random" the number of degrees of deviance from the taget. (1.0 default)
*/
void SP_shooter_rocket( gentity_t *ent ) {
  //InitShooter( ent, WP_ROCKET_LAUNCHER );
}

/*QUAKED shooter_plasma (1 0 0) (-16 -16 -16) (16 16 16)
Fires at either the target or the current direction.
"random" is the number of degrees of deviance from the taget. (1.0 default)
*/
void SP_shooter_plasma( gentity_t *ent ) {
  //InitShooter( ent, WP_PLASMAGUN);
}

/*QUAKED shooter_grenade (1 0 0) (-16 -16 -16) (16 16 16)
Fires at either the target or the current direction.
"random" is the number of degrees of deviance from the taget. (1.0 default)
*/
void SP_shooter_grenade( gentity_t *ent ) {
  //InitShooter( ent, WP_GRENADE_LAUNCHER);
}

/*
======================================================================

  NEAT EFFECTS AND STUFF FOR TREMULOUS

======================================================================
*/

//TA: position/colour/intensity calculating function
void ShineTorch( gentity_t *self )
{
  vec3_t  origin, angles;

  VectorCopy( self->parent->s.pos.trBase, origin );
  VectorCopy( self->parent->s.apos.trBase, angles );

  G_SetOrigin( self, origin );

  VectorCopy( angles, self->s.apos.trBase );

  //so we can use the predicted values client side if available
  self->s.clientNum = self->parent->s.number;

  trap_LinkEntity( self );
}

//TA: use function for spriter
void SP_use_spriter( gentity_t *self, gentity_t *other, gentity_t *activator )
{
  //toggle EF_NODRAW
  if( self->s.eFlags & EF_NODRAW )
    self->s.eFlags &= ~EF_NODRAW;
  else
    self->s.eFlags |= EF_NODRAW;
}

//TA: spawn function for spriter
void SP_misc_spriter( gentity_t *self )
{
  vec3_t  accel;
  
  G_SetOrigin( self, self->s.origin );

  //set a bunch of stuff to be visible client side
  VectorCopy( self->acceleration, self->s.origin2 );
  
  self->s.time = (int)self->speed;
  self->s.time2 = (int)self->wait;
  self->s.powerups = (int)self->random;
  self->s.angles2[ 0 ] = self->physicsBounce;
  
  self->s.modelindex = self->pos1[ 0 ];
  self->s.modelindex2 = self->pos1[ 1 ];
  
  self->s.legsAnim = self->pos2[ 0 ];
  self->s.torsoAnim = self->pos2[ 1 ];

  if( self->count > 0 )
    self->s.angles2[ 1 ] = ( 1000 / self->count );
  else
    self->s.angles2[ 1 ] = 1000;

  //add the shader to the client precache list
  self->s.weapon = G_ShaderIndex( self->targetShaderName );

  if( self->spawnflags & 1 )
    self->s.eFlags |= EF_OVERDRAW_OFF;
  if( self->spawnflags & ( 1 << 1 ) )
    self->s.eFlags |= EF_REAL_LIGHT;
  if( self->spawnflags & ( 1 << 2 ) )
    self->s.eFlags |= EF_NODRAW;

  self->use = SP_use_spriter;

  self->s.eType = ET_SPRITER;
  
  trap_LinkEntity( self );
}

//TA: use function for anim model
void SP_use_anim_model( gentity_t *self, gentity_t *other, gentity_t *activator )
{
  if( self->spawnflags & 1 )
  {
    //if spawnflag 1 is set
    //toggle EF_NODRAW
    if( self->s.eFlags & EF_NODRAW )
      self->s.eFlags &= ~EF_NODRAW;
    else
      self->s.eFlags |= EF_NODRAW;
  }
  else
  {
    //if the animation loops then toggle the animation
    //toggle EF_MOVER_STOP
    if( self->s.eFlags & EF_MOVER_STOP )
      self->s.eFlags &= ~EF_MOVER_STOP;
    else
      self->s.eFlags |= EF_MOVER_STOP;
  }
}

//TA: spawn function for anim model
void SP_misc_anim_model( gentity_t *self )
{
  self->s.powerups  = (int)self->animation[ 0 ];
  self->s.weapon    = (int)self->animation[ 1 ];
  self->s.torsoAnim = (int)self->animation[ 2 ];
  self->s.legsAnim  = (int)self->animation[ 3 ];
  
  self->s.angles2[ 0 ] = self->pos2[ 0 ];
  
  //add the model to the client precache list
  self->s.modelindex = G_ModelIndex( self->model );

  self->use = SP_use_anim_model;

  self->s.eType = ET_ANIMMAPOBJ;
  
  trap_LinkEntity( self );
}

//TA: spawn function for lens flares
void SP_misc_light_flare( gentity_t *self )
{
  self->s.eType = ET_LIGHTFLARE;
  self->s.modelindex = G_ShaderIndex( self->targetShaderName );
  VectorCopy( self->pos2, self->s.origin2 );

  trap_LinkEntity( self );
}
