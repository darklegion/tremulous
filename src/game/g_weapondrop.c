//
// Ported + rewritten ioq3 item-drop.
//
// blowFish
//
#include "g_local.h"

#define DISABLE_TOUCH_TIME 1000
#define MISSILE_PRESTEP_TIME  50

//
// Pickup Weapon
//
// ent - The "weapon" being picked up
// other - The client who picked it up
//
void Pickup_Weapon (gentity_t *ent, gentity_t *other)
{
  int w = ent->s.modelindex;

  if ( w == WP_NONE )
    return;

  other->client->ps.stats[ STAT_WEAPON ] = w;
  other->client->ps.ammo = ent->item.ammo;
  other->client->ps.clips = ent->item.clips;
  G_ForceWeaponChange( other, w );
}

//
// Touch Weapon
//
// ent - The "weapon" being picked up
// other - The client who picked it up
//
void Touch_Weapon (gentity_t *ent, gentity_t *other, trace_t *trace)
{
  if( !other->client
    || other->client->pers.teamSelection == TEAM_NONE
    || other->client->pers.teamSelection == TEAM_ALIENS )
    return;

  if( (other->client->lastDropTime + DISABLE_TOUCH_TIME) > level.time)
    return;

  if ( other->health < 1 )
    return;

  Pickup_Weapon(ent, other);

  // dropped items will not respawn
  if ( ent->flags & FL_DROPPED_ITEM )
  {
    ent->freeAfterEvent = qtrue;
  }

  ent->r.svFlags |= SVF_NOCLIENT;
  ent->s.eFlags |= EF_NODRAW;
  ent->r.contents = 0;

  trap_LinkEntity( ent );
}

#define ITEM_RADIUS 15

//
// Launch Weapon
//
// Spawn a weapon and toss it into the world.
//
gentity_t *LaunchWeapon (gentity_t* client, weapon_t weap, vec3_t origin, vec3_t velocity)
{
    gentity_t   *dropped;

    dropped = G_Spawn();

    dropped->s.eType = ET_WEAPON_DROP;
    dropped->s.modelindex = weap; // store weapon number in modelindex
    dropped->s.modelindex2 = 1; // This is non-zero is it's a dropped item

    dropped->classname = BG_Weapon(weap)->name;
    VectorSet (dropped->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS);
    VectorSet (dropped->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS);
    dropped->r.contents = CONTENTS_TRIGGER;

    dropped->item.ammo = client->client->ps.ammo;
    dropped->item.clips = client->client->ps.clips;

    dropped->touch = Touch_Weapon;

    G_SetOrigin( dropped, origin );
    dropped->s.pos.trType = TR_GRAVITY;
    dropped->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;
    VectorCopy( velocity, dropped->s.pos.trDelta );

    dropped->s.eFlags |= EF_BOUNCE_HALF;
    dropped->think = G_FreeEntity;
    dropped->nextthink = level.time + 30000;

    dropped->flags = FL_DROPPED_ITEM;

    trap_LinkEntity (dropped);

    return dropped;
}

//
// Drop Weapon
//
// Spawns an weapon and tosses it forward
//
gentity_t *G_DropWeapon (gentity_t *ent, weapon_t w, float angle)
{
    vec3_t  velocity;
    vec3_t  angles;

    // set aiming directions
    VectorCopy( ent->s.apos.trBase, angles );
    angles[YAW] += angle;
    angles[PITCH] = 0;  // always forward

    AngleVectors( angles, velocity, NULL, NULL );
    VectorScale( velocity, 150, velocity );
    velocity[2] += 200 + crandom() * 50;

    ent->client->lastDropTime = level.time;
    return LaunchWeapon( ent, w, ent->s.pos.trBase, velocity );
}

//
// Run Weapon Drops
//
void G_RunWeaponDrop (gentity_t *ent)
{
	vec3_t		origin;
	trace_t		tr;
	int			contents;
	int			mask;

	// if its groundentity has been set to none, it may have been pushed off an edge
	if ( ent->s.groundEntityNum == ENTITYNUM_NONE )
    {
		if ( ent->s.pos.trType != TR_GRAVITY )
        {
			ent->s.pos.trType = TR_GRAVITY;
			ent->s.pos.trTime = level.time;
		}
	}

	if ( ent->s.pos.trType == TR_STATIONARY )
    {
		// check think function
		G_RunThink( ent );
		return;
	}

	// get current position
	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );

	// trace a line from the previous position to the current position
	if ( ent->clipmask )
    {
		mask = ent->clipmask;
	}
    else
    {
		mask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
	}

	trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin,
		ent->r.ownerNum, mask );

	VectorCopy( tr.endpos, ent->r.currentOrigin );

	if ( tr.startsolid )
    {
		tr.fraction = 0;
	}

	trap_LinkEntity( ent );	// FIXME: avoid this for stationary?

	// check think function
	G_RunThink( ent );

	if ( tr.fraction == 1 )
    {
		return;
	}

	// if it is in a nodrop volume, remove it
	contents = trap_PointContents( ent->r.currentOrigin, -1 );
	if ( contents & CONTENTS_NODROP )
    {
		G_FreeEntity( ent );
		return;
	}

    G_BounceMissile( ent, &tr );
}
