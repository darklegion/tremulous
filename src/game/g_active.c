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

/*
===============
G_DamageFeedback

Called just before a snapshot is sent to the given player.
Totals up all damage and generates both the player_state_t
damage values to that client for pain blends and kicks, and
global pain sound events for all clients.
===============
*/
void P_DamageFeedback( gentity_t *player )
{
  gclient_t *client;
  float     count;
  vec3_t    angles;

  client = player->client;
  if( client->ps.pm_type == PM_DEAD )
    return;

  // total points of damage shot at the player this frame
  count = client->damage_blood + client->damage_armor;
  if( count == 0 )
    return;   // didn't take any damage

  if( count > 255 )
    count = 255;

  // send the information to the client

  // world damage (falling, slime, etc) uses a special code
  // to make the blend blob centered instead of positional
  if( client->damage_fromWorld )
  {
    client->ps.damagePitch = 255;
    client->ps.damageYaw = 255;

    client->damage_fromWorld = qfalse;
  } 
 else
 {
    vectoangles( client->damage_from, angles );
    client->ps.damagePitch = angles[ PITCH ] / 360.0 * 256;
    client->ps.damageYaw = angles[ YAW ] / 360.0 * 256;
  }

  // play an apropriate pain sound
  if( ( level.time > player->pain_debounce_time ) && !( player->flags & FL_GODMODE ) )
  {
    player->pain_debounce_time = level.time + 700;
    G_AddEvent( player, EV_PAIN, player->health );
    client->ps.damageEvent++;
  }


  client->ps.damageCount = count;

  //
  // clear totals
  //
  client->damage_blood = 0;
  client->damage_armor = 0;
  client->damage_knockback = 0;
}



/*
=============
P_WorldEffects

Check for lava / slime contents and drowning
=============
*/
void P_WorldEffects( gentity_t *ent )
{
  int       waterlevel;

  if( ent->client->noclip )
  {
    ent->client->airOutTime = level.time + 12000; // don't need air
    return;
  }

  waterlevel = ent->waterlevel;

  //
  // check for drowning
  //
  if( waterlevel == 3 )
  {
    // if out of air, start drowning
    if( ent->client->airOutTime < level.time)
    {
      // drown!
      ent->client->airOutTime += 1000;
      if( ent->health > 0 )
      {
        // take more damage the longer underwater
        ent->damage += 2;
        if( ent->damage > 15 )
          ent->damage = 15;

        // play a gurp sound instead of a normal pain sound
        if( ent->health <= ent->damage )
          G_Sound( ent, CHAN_VOICE, G_SoundIndex( "*drown.wav" ) );
        else if( rand( ) & 1 )
          G_Sound( ent, CHAN_VOICE, G_SoundIndex( "sound/player/gurp1.wav" ) );
        else
          G_Sound( ent, CHAN_VOICE, G_SoundIndex( "sound/player/gurp2.wav" ) );

        // don't play a normal pain sound
        ent->pain_debounce_time = level.time + 200;

        G_Damage( ent, NULL, NULL, NULL, NULL,
          ent->damage, DAMAGE_NO_ARMOR, MOD_WATER );
      }
    }
  }
  else
  {
    ent->client->airOutTime = level.time + 12000;
    ent->damage = 2;
  }

  //
  // check for sizzle damage (move to pmove?)
  //
  if( waterlevel &&
      ( ent->watertype & ( CONTENTS_LAVA | CONTENTS_SLIME ) ) )
  {
    if( ent->health > 0 &&
        ent->pain_debounce_time <= level.time  )
    {
      if( ent->watertype & CONTENTS_LAVA )
      {
        G_Damage( ent, NULL, NULL, NULL, NULL,
          30 * waterlevel, 0, MOD_LAVA );
      }

      if( ent->watertype & CONTENTS_SLIME )
      {
        G_Damage( ent, NULL, NULL, NULL, NULL,
          10 * waterlevel, 0, MOD_SLIME );
      }
    }
  }
}



/*
===============
G_SetClientSound
===============
*/
void G_SetClientSound( gentity_t *ent )
{
  if( ent->waterlevel && ( ent->watertype & ( CONTENTS_LAVA | CONTENTS_SLIME ) ) )
    ent->client->ps.loopSound = level.snd_fry;
  else
    ent->client->ps.loopSound = 0;
}



//==============================================================

/*
==============
ClientImpacts
==============
*/
void ClientImpacts( gentity_t *ent, pmove_t *pm )
{
  int       i, j;
  trace_t   trace;
  gentity_t *other;

  memset( &trace, 0, sizeof( trace ) );
  
  for( i = 0; i < pm->numtouch; i++ )
  {
    for( j = 0; j < i; j++ )
    {
      if( pm->touchents[ j ] == pm->touchents[ i ] )
        break;
    }
    
    if( j != i )
      continue; // duplicated
    
    other = &g_entities[ pm->touchents[ i ] ];

    if( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) )
      ent->touch( ent, other, &trace );

    //charge attack
    if( ent->client->ps.weapon == WP_BIGMOFO &&
        ent->client->ps.stats[ STAT_MISC ] > 0 && 
        ent->client->charging )
      ChargeAttack( ent, other );
    
    if( !other->touch )
      continue;

    other->touch( other, ent, &trace );
  }
}

/*
============
G_TouchTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void  G_TouchTriggers( gentity_t *ent )
{
  int       i, num;
  int       touch[MAX_GENTITIES];
  gentity_t *hit;
  trace_t   trace;
  vec3_t    mins, maxs;
  static    vec3_t range = { 40, 40, 52 };

  if( !ent->client )
    return;

  // dead clients don't activate triggers!
  if( ent->client->ps.stats[ STAT_HEALTH ] <= 0 )
    return;

  VectorSubtract( ent->client->ps.origin, range, mins );
  VectorAdd( ent->client->ps.origin, range, maxs );

  num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

  // can't use ent->absmin, because that has a one unit pad
  VectorAdd( ent->client->ps.origin, ent->r.mins, mins );
  VectorAdd( ent->client->ps.origin, ent->r.maxs, maxs );

  for( i = 0; i < num; i++ ) 
  {
    hit = &g_entities[ touch[ i ] ];

    if( !hit->touch && !ent->touch )
      continue;

    if( !( hit->r.contents & CONTENTS_TRIGGER ) )
      continue;

    // ignore most entities if a spectator
    if( ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) ||
        ( ent->client->ps.stats[ STAT_STATE ] & SS_INFESTING ) ||
        ( ent->client->ps.stats[ STAT_STATE ] & SS_HOVELING ) )
    {
      if( hit->s.eType != ET_TELEPORT_TRIGGER &&
          // this is ugly but adding a new ET_? type will
          // most likely cause network incompatibilities
          hit->touch != Touch_DoorTrigger )
      {
        //check for manually triggered doors
        manualTriggerSpectator( hit, ent );
        continue;
      }
    }

    if( !trap_EntityContact( mins, maxs, hit ) )
      continue;

    memset( &trace, 0, sizeof( trace ) );

    if( hit->touch )
      hit->touch( hit, ent, &trace );

    if( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) )
      ent->touch( ent, hit, &trace );
  }

  // if we didn't touch a jump pad this pmove frame
  if( ent->client->ps.jumppad_frame != ent->client->ps.pmove_framecount )
  {
    ent->client->ps.jumppad_frame = 0;
    ent->client->ps.jumppad_ent = 0;
  }
}

/*
=================
SpectatorThink
=================
*/
void SpectatorThink( gentity_t *ent, usercmd_t *ucmd )
{
  pmove_t pm;
  gclient_t *client;

  client = ent->client;

  if( client->sess.spectatorState != SPECTATOR_FOLLOW )
  {
    if( client->sess.spectatorState == SPECTATOR_LOCKED )
      client->ps.pm_type = PM_FREEZE;
    else
      client->ps.pm_type = PM_SPECTATOR;

    client->ps.speed = 400; // faster than normal

    // set up for pmove
    memset( &pm, 0, sizeof( pm ) );
    pm.ps = &client->ps;
    pm.cmd = *ucmd;
    pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY; // spectators can fly through bodies
    pm.trace = trap_Trace;
    pm.pointcontents = trap_PointContents;

    // perform a pmove
    Pmove( &pm );

    // save results of pmove
    VectorCopy( client->ps.origin, ent->s.origin );

    G_TouchTriggers( ent );
    trap_UnlinkEntity( ent );
  }

  client->oldbuttons = client->buttons;
  client->buttons = ucmd->buttons;

  if( ( client->buttons & BUTTON_ATTACK ) && !( client->oldbuttons & BUTTON_ATTACK ) )
  {
    if( client->pers.teamSelection == PTE_NONE )
      G_TriggerMenu( client->ps.clientNum, MN_TEAM );
    else if( client->pers.teamSelection == PTE_ALIENS )
      G_TriggerMenu( client->ps.clientNum, MN_A_CLASS );
    else if( client->pers.teamSelection == PTE_HUMANS )
      G_TriggerMenu( client->ps.clientNum, MN_H_SPAWN );
  }
}



/*
=================
ClientInactivityTimer

Returns qfalse if the client is dropped
=================
*/
qboolean ClientInactivityTimer( gclient_t *client )
{
  if( ! g_inactivity.integer )
  {
    // give everyone some time, so if the operator sets g_inactivity during
    // gameplay, everyone isn't kicked
    client->inactivityTime = level.time + 60 * 1000;
    client->inactivityWarning = qfalse;
  }
  else if( client->pers.cmd.forwardmove ||
           client->pers.cmd.rightmove ||
           client->pers.cmd.upmove ||
           ( client->pers.cmd.buttons & BUTTON_ATTACK ) )
  {
    client->inactivityTime = level.time + g_inactivity.integer * 1000;
    client->inactivityWarning = qfalse;
  }
  else if( !client->pers.localClient )
  {
    if( level.time > client->inactivityTime )
    {
      trap_DropClient( client - level.clients, "Dropped due to inactivity" );
      return qfalse;
    }
    
    if( level.time > client->inactivityTime - 10000 && !client->inactivityWarning )
    {
      client->inactivityWarning = qtrue;
      trap_SendServerCommand( client - level.clients, "cp \"Ten seconds until inactivity drop!\n\"" );
    }
  }
  
  return qtrue;
}

/*
==================
ClientTimerActions

Actions that happen once a second
==================
*/
void ClientTimerActions( gentity_t *ent, int msec )
{
  gclient_t *client;
  usercmd_t *ucmd;
  int       aForward, aRight;

  ucmd = &ent->client->pers.cmd;
  
  aForward  = abs( ucmd->forwardmove );
  aRight    = abs( ucmd->rightmove );
  
  client = ent->client;
  client->time100 += msec;
  client->time1000 += msec;

  while ( client->time100 >= 100 )
  {
    client->time100 -= 100;

    //if not trying to run then not trying to sprint
    if( aForward <= 64 )
      client->ps.stats[ STAT_STATE ] &= ~SS_SPEEDBOOST;

    if( BG_gotItem( UP_JETPACK, client->ps.stats ) && BG_activated( UP_JETPACK, client->ps.stats ) )
      client->ps.stats[ STAT_STATE ] &= ~SS_SPEEDBOOST;
      
    if( ( client->ps.stats[ STAT_STATE ] & SS_SPEEDBOOST ) &&  ucmd->upmove >= 0 )
    {
      //subtract stamina
      if( BG_gotItem( UP_LIGHTARMOUR, client->ps.stats ) )
        client->ps.stats[ STAT_STAMINA ] -= STAMINA_LARMOUR_TAKE;
      else
        client->ps.stats[ STAT_STAMINA ] -= STAMINA_SPRINT_TAKE;
      
      if( client->ps.stats[ STAT_STAMINA ] < -MAX_STAMINA )
        client->ps.stats[ STAT_STAMINA ] = -MAX_STAMINA;
    }
                                                            
    if( ( aForward <= 64 && aForward > 5 ) || ( aRight <= 64 && aRight > 5 ) )
    {
      //restore stamina
      client->ps.stats[ STAT_STAMINA ] += STAMINA_WALK_RESTORE;
      
      if( client->ps.stats[ STAT_STAMINA ] > MAX_STAMINA )
        client->ps.stats[ STAT_STAMINA ] = MAX_STAMINA;
    }
    else if( aForward <= 5 && aRight <= 5 )
    {
      //restore stamina faster
      client->ps.stats[ STAT_STAMINA ] += STAMINA_STOP_RESTORE;
      
      if( client->ps.stats[ STAT_STAMINA ] > MAX_STAMINA )
        client->ps.stats[ STAT_STAMINA ] = MAX_STAMINA;
    }

    //client is charging up for a pounce
    if( client->ps.weapon == WP_DRAGOON || client->ps.weapon == WP_DRAGOON_UPG )
    {
      int pounceSpeed;

      if( client->ps.weapon == WP_DRAGOON )
        pounceSpeed = DRAGOON_POUNCE_SPEED;
      else if( client->ps.weapon == WP_DRAGOON_UPG )
        pounceSpeed = DRAGOON_POUNCE_UPG_SPEED;

      if( client->ps.stats[ STAT_MISC ] < pounceSpeed && ucmd->buttons & BUTTON_ATTACK2 )
        client->ps.stats[ STAT_MISC ] += ( 100.0f / (float)DRAGOON_POUNCE_TIME ) * pounceSpeed;

      if( !( ucmd->buttons & BUTTON_ATTACK2 ) )
      {
        if( client->ps.stats[ STAT_MISC ] > 0 )
        {
          client->allowedToPounce = qtrue;
          client->pouncePayload = client->ps.stats[ STAT_MISC ];
        }

        client->ps.stats[ STAT_MISC ] = 0;
      }

      if( client->ps.stats[ STAT_MISC ] > pounceSpeed )
        client->ps.stats[ STAT_MISC ] = pounceSpeed;
    }

    //client is charging up for a... charge
    if( client->ps.weapon == WP_BIGMOFO )
    {
      if( client->ps.stats[ STAT_MISC ] < BMOFO_CHARGE_TIME && ucmd->buttons & BUTTON_ATTACK2 && 
          ( ucmd->forwardmove > 0 || aRight ) )
      {
        client->charging = qfalse; //should already be off, just making sure

        client->ps.stats[ STAT_MISC ] += (int)( 100 * (float)BMOFO_CHARGE_CHARGE_RATIO );
        
        if( client->ps.stats[ STAT_MISC ] > BMOFO_CHARGE_TIME )
          client->ps.stats[ STAT_MISC ] = BMOFO_CHARGE_TIME;
      }

      if( !( ucmd->buttons & BUTTON_ATTACK2 ) )
      {
        if( client->ps.stats[ STAT_MISC ] > 0 )
        {
          client->ps.stats[ STAT_MISC ] -= 100;
          client->charging = qtrue;

          //if the charger has stopped moving take a chunk of charge away
          if( VectorLength( client->ps.velocity ) < 64.0f )
            client->ps.stats[ STAT_MISC ] = client->ps.stats[ STAT_MISC ] >> 1;

          //can't charge backwards
          if( ucmd->forwardmove < 0 )
            client->ps.stats[ STAT_MISC ] = 0;
        }
        
        if( client->ps.stats[ STAT_MISC ] <= 0 )
        {
          client->ps.stats[ STAT_MISC ] = 0;
          client->charging = qfalse;
        }
      }
    }

    //client is charging up an lcannon
    if( client->ps.weapon == WP_LUCIFER_CANNON )
    {
      int ammo;
      
      BG_unpackAmmoArray( WP_LUCIFER_CANNON, client->ps.ammo, client->ps.powerups, &ammo, NULL, NULL );
      
      if( client->ps.stats[ STAT_MISC ] < LCANNON_TOTAL_CHARGE && ucmd->buttons & BUTTON_ATTACK )
        client->ps.stats[ STAT_MISC ] += ( 100.0f / LCANNON_CHARGE_TIME ) * LCANNON_TOTAL_CHARGE;
      
      if( client->ps.stats[ STAT_MISC ] > LCANNON_TOTAL_CHARGE )
        client->ps.stats[ STAT_MISC ] = LCANNON_TOTAL_CHARGE;

      if( client->ps.stats[ STAT_MISC ] > ( ammo * LCANNON_TOTAL_CHARGE ) / 10 )
        client->ps.stats[ STAT_MISC ] = ammo * LCANNON_TOTAL_CHARGE / 10;
    }
    
    switch( client->ps.weapon )
    {
      case WP_ABUILD:
      case WP_ABUILD2:
      case WP_HBUILD:
      case WP_HBUILD2:
        //set validity bit on buildable
        if( ( client->ps.stats[ STAT_BUILDABLE ] & ~SB_VALID_TOGGLEBIT ) > BA_NONE )
        {
          int     dist = BG_FindBuildDistForClass( ent->client->ps.stats[ STAT_PCLASS ] );
          vec3_t  dummy;
          
          if( G_itemFits( ent, client->ps.stats[ STAT_BUILDABLE ] & ~SB_VALID_TOGGLEBIT,
                          dist, dummy ) == IBE_NONE )
            client->ps.stats[ STAT_BUILDABLE ] |= SB_VALID_TOGGLEBIT;
          else
            client->ps.stats[ STAT_BUILDABLE ] &= ~SB_VALID_TOGGLEBIT;
        }

        //update build timer
        if( client->ps.stats[ STAT_MISC ] > 0 )
          client->ps.stats[ STAT_MISC ] -= 100;
        
        if( client->ps.stats[ STAT_MISC ] < 0 )
          client->ps.stats[ STAT_MISC ] = 0;
        break;
        
      default:
        break;
    }
  }

  while( client->time1000 >= 1000 )
  {
    client->time1000 -= 1000;

    //client is hydra poisoned
    if( client->ps.stats[ STAT_STATE ] & SS_POISONCLOUDED )
      G_Damage( ent, client->lastPoisonCloudedClient, client->lastPoisonCloudedClient, NULL, NULL,
                HYDRA_PCLOUD_DMG, 0, MOD_HYDRA_PCLOUD );
    
    //client is poisoned
    if( client->ps.stats[ STAT_STATE ] & SS_POISONED )
    {
      int i;
      int seconds = ( ( level.time - client->lastPoisonTime ) / 1000 ) + 1;
      int damage = ALIEN_POISON_DMG, damage2;
      
      for( i = 0; i < seconds; i++ )
      {
        if( i == seconds - 1 )
          damage2 = damage;
        
        damage *= ALIEN_POISON_DIVIDER;
      }

      damage = damage2 - damage;

      G_Damage( ent, client->lastPoisonClient, client->lastPoisonClient, NULL, NULL,
                damage, 0, MOD_POISON );
    }

    //replenish alien health
    if( client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS )
    {
      int       entityList[ MAX_GENTITIES ];
      vec3_t    range = { BMOFO_REGEN_RANGE, BMOFO_REGEN_RANGE, BMOFO_REGEN_RANGE };
      vec3_t    mins, maxs, dir;
      int       i, num;
      gentity_t *alienPlayer;
      float     modifier = 1.0f;
      
      VectorAdd( client->ps.origin, range, maxs );
      VectorSubtract( client->ps.origin, range, mins );
      
      num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
      for( i = 0; i < num; i++ )
      {
        alienPlayer = &g_entities[ entityList[ i ] ];
        
        if( alienPlayer->client && alienPlayer->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS &&
            alienPlayer->client->ps.stats[ STAT_PCLASS ] == PCL_A_O_LEV4 )
        {
          modifier = BMOFO_REGEN_MOD;
          break;
        }
      }
      
      if( ent->health < client->ps.stats[ STAT_MAX_HEALTH ] )
        ent->health += BG_FindRegenRateForClass( client->ps.stats[ STAT_PCLASS ] ) * modifier;

      if( ent->health > client->ps.stats[ STAT_MAX_HEALTH ] )
        ent->health = client->ps.stats[ STAT_MAX_HEALTH ];
    }
  }
}

/*
====================
ClientIntermissionThink
====================
*/
void ClientIntermissionThink( gclient_t *client )
{
  client->ps.eFlags &= ~EF_TALK;
  client->ps.eFlags &= ~EF_FIRING;
  client->ps.eFlags &= ~EF_FIRING2;

  // the level will exit when everyone wants to or after timeouts

  // swap and latch button actions
  client->oldbuttons = client->buttons;
  client->buttons = client->pers.cmd.buttons;
  if( client->buttons & ( BUTTON_ATTACK | BUTTON_USE_HOLDABLE ) & ( client->oldbuttons ^ client->buttons ) )
    client->readyToExit = 1;
}


/*
================
ClientEvents

Events will be passed on to the clients for presentation,
but any server game effects are handled here
================
*/
void ClientEvents( gentity_t *ent, int oldEventSequence )
{
  int       i, j;
  int       event;
  gclient_t *client;
  int       damage;
  vec3_t    dir;
  vec3_t    point, mins;
  gentity_t *drop;
  float     fallDistance;
  pClass_t  class;

  client = ent->client;
  class = client->ps.stats[ STAT_PCLASS ];

  if( oldEventSequence < client->ps.eventSequence - MAX_PS_EVENTS )
    oldEventSequence = client->ps.eventSequence - MAX_PS_EVENTS;

  for( i = oldEventSequence; i < client->ps.eventSequence; i++ )
  {
    event = client->ps.events[ i & ( MAX_PS_EVENTS - 1 ) ];

    switch( event )
    {
      case EV_FALL_MEDIUM:
      case EV_FALL_FAR:
        if( ent->s.eType != ET_PLAYER )
          break;    // not in the player model

        fallDistance = ( (float)client->ps.stats[ STAT_FALLDIST ] - MIN_FALL_DISTANCE ) /
                         ( MAX_FALL_DISTANCE - MIN_FALL_DISTANCE );

        if( fallDistance < 0.0f )
          fallDistance = 0.0f;
        else if( fallDistance > 1.0f )
          fallDistance = 1.0f;

        damage = (int)( (float)BG_FindHealthForClass( class ) *
                 BG_FindFallDamageForClass( class ) * fallDistance );
        
        VectorSet( dir, 0, 0, 1 );
        BG_FindBBoxForClass( class, mins, NULL, NULL, NULL, NULL );
        mins[ 0 ] = mins[ 1 ] = 0.0f;
        VectorAdd( client->ps.origin, mins, point );
        
        ent->pain_debounce_time = level.time + 200; // no normal pain sound
        G_Damage( ent, NULL, NULL, dir, point, damage, DAMAGE_NO_LOCDAMAGE, MOD_FALLING );
        break;

      case EV_FIRE_WEAPON:
        FireWeapon( ent );
        break;

      case EV_FIRE_WEAPON2:
        FireWeapon2( ent );
        break;

      case EV_FIRE_WEAPON3:
        FireWeapon3( ent );
        break;

      default:
        break;
    }
  }
}

/*
==============
StuckInOtherClient
==============
*/
static int StuckInOtherClient( gentity_t *ent )
{
  int       i;
  gentity_t *ent2;

  ent2 = &g_entities[ 0 ];
  
  for( i = 0; i < MAX_CLIENTS; i++, ent2++ )
  {
    if( ent2 == ent )
      continue;

    if( !ent2->inuse )
      continue;

    if( !ent2->client )
      continue;

    if( ent2->health <= 0 )
      continue;

    //
    if( ent2->r.absmin[ 0 ] > ent->r.absmax[ 0 ] )
      continue;
    
    if( ent2->r.absmin[ 1 ] > ent->r.absmax[ 1 ] )
      continue;
    
    if( ent2->r.absmin[ 2 ] > ent->r.absmax[ 2 ] )
      continue;
    
    if( ent2->r.absmax[ 0 ] < ent->r.absmin[ 0 ] )
      continue;
    
    if( ent2->r.absmax[ 1 ] < ent->r.absmin[ 1 ] )
      continue;
    
    if( ent2->r.absmax[ 2 ] < ent->r.absmin[ 2 ] )
      continue;
    
    return qtrue;
  }
  
  return qfalse;
}

/*
==============
SendPendingPredictableEvents
==============
*/
void SendPendingPredictableEvents( playerState_t *ps )
{
	gentity_t *t;
	int       event, seq;
	int       extEvent, number;

	// if there are still events pending
	if( ps->entityEventSequence < ps->eventSequence )
  {
		// create a temporary entity for this event which is sent to everyone
		// except the client who generated the event
		seq = ps->entityEventSequence & ( MAX_PS_EVENTS - 1 );
		event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );
		// set external event to zero before calling BG_PlayerStateToEntityState
		extEvent = ps->externalEvent;
		ps->externalEvent = 0;
		// create temporary entity for event
		t = G_TempEntity( ps->origin, event );
		number = t->s.number;
		BG_PlayerStateToEntityState( ps, &t->s, qtrue );
		t->s.number = number;
		t->s.eType = ET_EVENTS + event;
		t->s.eFlags |= EF_PLAYER_EVENT;
		t->s.otherEntityNum = ps->clientNum;
		// send to everyone except the client who generated the event
		t->r.svFlags |= SVF_NOTSINGLECLIENT;
		t->r.singleClient = ps->clientNum;
		// set back external event
		ps->externalEvent = extEvent;
	}
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame on fast clients.

If "g_synchronousClients 1" is set, this will be called exactly
once for each server frame, which makes for smooth demo recording.
==============
*/
void ClientThink_real( gentity_t *ent )
{
  gclient_t *client;
  pmove_t   pm;
  int       oldEventSequence;
  int       msec;
  usercmd_t *ucmd;
  float     speed;

  client = ent->client;

  // don't think if the client is not yet connected (and thus not yet spawned in)
  if( client->pers.connected != CON_CONNECTED )
    return;
  
  // mark the time, so the connection sprite can be removed
  ucmd = &ent->client->pers.cmd;

  // sanity check the command time to prevent speedup cheating
  if( ucmd->serverTime > level.time + 200 )
  {
    ucmd->serverTime = level.time + 200;
//    G_Printf("serverTime <<<<<\n" );
  }
  
  if( ucmd->serverTime < level.time - 1000 )
  {
    ucmd->serverTime = level.time - 1000;
//    G_Printf("serverTime >>>>>\n" );
  }

  msec = ucmd->serverTime - client->ps.commandTime;
  // following others may result in bad times, but we still want
  // to check for follow toggles
  if( msec < 1 && client->sess.spectatorState != SPECTATOR_FOLLOW )
    return;
  
  if( msec > 200 )
    msec = 200;
  
  if( pmove_msec.integer < 8 )
    trap_Cvar_Set( "pmove_msec", "8" );
  else if( pmove_msec.integer > 33 )
    trap_Cvar_Set( "pmove_msec", "33" );

  if( pmove_fixed.integer || client->pers.pmoveFixed )
  {
    ucmd->serverTime = ( ( ucmd->serverTime + pmove_msec.integer - 1 ) / pmove_msec.integer ) * pmove_msec.integer;
    //if (ucmd->serverTime - client->ps.commandTime <= 0)
    //  return;
  }
  
  //
  // check for exiting intermission
  //
  if( level.intermissiontime )
  {
    ClientIntermissionThink( client );
    return;
  }

  // spectators don't do much
  if( client->sess.sessionTeam == TEAM_SPECTATOR )
  {
    if( client->sess.spectatorState == SPECTATOR_SCOREBOARD )
      return;
    
    SpectatorThink( ent, ucmd );
    return;
  }

  // check for inactivity timer, but never drop the local client of a non-dedicated server
  if( !ClientInactivityTimer( client ) )
    return;

  if( client->noclip )
    client->ps.pm_type = PM_NOCLIP;
  else if( client->ps.stats[ STAT_HEALTH ] <= 0 )
    client->ps.pm_type = PM_DEAD;
  else if( client->ps.stats[ STAT_STATE ] & SS_INFESTING ||
           client->ps.stats[ STAT_STATE ] & SS_HOVELING )
    client->ps.pm_type = PM_FREEZE;
  else if( client->ps.stats[ STAT_STATE ] & SS_BLOBLOCKED ||
           client->ps.stats[ STAT_STATE ] & SS_GRABBED )
    client->ps.pm_type = PM_GRABBED;
  else if( BG_gotItem( UP_JETPACK, client->ps.stats ) && BG_activated( UP_JETPACK, client->ps.stats ) )
    client->ps.pm_type = PM_JETPACK;
  else
    client->ps.pm_type = PM_NORMAL;

  if( client->ps.stats[ STAT_STATE ] & SS_GRABBED &&
      client->lastGrabTime + HYDRA_GRAB_TIME < level.time )
    client->ps.stats[ STAT_STATE ] &= ~SS_GRABBED;

  if( client->ps.stats[ STAT_STATE ] & SS_BLOBLOCKED &&
      client->lastLockTime + 5000 < level.time )
    client->ps.stats[ STAT_STATE ] &= ~SS_BLOBLOCKED;

  if( client->ps.stats[ STAT_STATE ] & SS_SLOWLOCKED &&
      client->lastLockTime + ABUILDER_BLOB_TIME < level.time )
    client->ps.stats[ STAT_STATE ] &= ~SS_SLOWLOCKED;

  client->ps.stats[ STAT_BOOSTTIME ] = level.time - client->lastBoostedTime;

  if( client->ps.stats[ STAT_STATE ] & SS_BOOSTED &&
      client->lastBoostedTime + BOOST_TIME < level.time )
    client->ps.stats[ STAT_STATE ] &= ~SS_BOOSTED;

  if( client->ps.stats[ STAT_STATE ] & SS_POISONCLOUDED &&
      client->lastPoisonCloudedTime + HYDRA_PCLOUD_TIME < level.time )
    client->ps.stats[ STAT_STATE ] &= ~SS_POISONCLOUDED;
  
  if( client->ps.stats[ STAT_STATE ] & SS_POISONED &&
      client->lastPoisonTime + ALIEN_POISON_TIME < level.time )
    client->ps.stats[ STAT_STATE ] &= ~SS_POISONED;
  
  client->ps.gravity = g_gravity.value;

  if( BG_gotItem( UP_ANTITOXIN, client->ps.stats ) &&
      BG_activated( UP_ANTITOXIN, client->ps.stats ) )
  {
    if( client->ps.stats[ STAT_STATE ] & SS_POISONED )
    {
      //remove anti toxin
      BG_removeItem( UP_ANTITOXIN, client->ps.stats );
    
      client->ps.stats[ STAT_STATE ] &= ~SS_POISONED;
    }
  }
  
  // set speed
  client->ps.speed = g_speed.value * BG_FindSpeedForClass( client->ps.stats[ STAT_PCLASS ] );

  //TA: slow player if charging up for a pounce
  if( ( client->ps.weapon == WP_DRAGOON || client->ps.weapon == WP_DRAGOON_UPG ) &&
      ucmd->buttons & BUTTON_ATTACK2 )
    client->ps.speed *= DRAGOON_POUNCE_SPEED_MOD;

  //TA: slow the player if slow locked
  if( client->ps.stats[ STAT_STATE ] & SS_SLOWLOCKED )
    client->ps.speed *= ABUILDER_BLOB_SPEED_MOD;
  
  if( client->lastCreepSlowTime + CREEP_TIMEOUT < level.time )
    client->ps.stats[ STAT_STATE ] &= ~SS_CREEPSLOWED;
  
  //randomly disable the jet pack if damaged
  if( BG_gotItem( UP_JETPACK, client->ps.stats ) &&
      BG_activated( UP_JETPACK, client->ps.stats ) &&
      ( client->lastDamageTime + JETPACK_DISABLE_TIME > level.time ) )
  {
    if( random( ) > JETPACK_DISABLE_CHANCE )
      client->ps.pm_type = PM_NORMAL;
  }
  
  // set up for pmove
  oldEventSequence = client->ps.eventSequence;

  memset( &pm, 0, sizeof( pm ) );

  if( !( ucmd->buttons & BUTTON_TALK ) ) //&& client->ps.weaponTime <= 0 ) //TA: erk more server load
  {
    switch( client->ps.weapon )
    {
      case WP_SOLDIER:
        if( client->ps.weaponTime <= 0 )
          pm.autoWeaponHit[ client->ps.weapon ] = CheckVenomAttack( ent );
        break;

      case WP_HYDRA:
      case WP_HYDRA_UPG:
        CheckGrabAttack( ent );
        break;

      case WP_DRAGOON:
      case WP_DRAGOON_UPG:
        if( client->ps.weaponTime <= 0 )
          pm.autoWeaponHit[ client->ps.weapon ] = CheckPounceAttack( ent );
        break;

      default:
        break;
    }
  }

  if( ent->flags & FL_FORCE_GESTURE )
  {
    ent->flags &= ~FL_FORCE_GESTURE;
    ent->client->pers.cmd.buttons |= BUTTON_GESTURE;
  }
  
  pm.ps = &client->ps;
  pm.cmd = *ucmd;
  
  if( pm.ps->pm_type == PM_DEAD )
    pm.tracemask = MASK_PLAYERSOLID; // & ~CONTENTS_BODY;
  
  if( pm.ps->stats[ STAT_STATE ] & SS_INFESTING ||
      pm.ps->stats[ STAT_STATE ] & SS_HOVELING )
    pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
  else
    pm.tracemask = MASK_PLAYERSOLID;
  
  pm.trace = trap_Trace;
  pm.pointcontents = trap_PointContents;
  pm.debugLevel = g_debugMove.integer;
  pm.noFootsteps = 0;

  pm.pmove_fixed = pmove_fixed.integer | client->pers.pmoveFixed;
  pm.pmove_msec = pmove_msec.integer;

  VectorCopy( client->ps.origin, client->oldOrigin );

  Pmove( &pm);

  // save results of pmove
  if( ent->client->ps.eventSequence != oldEventSequence )
    ent->eventTime = level.time;

  if( g_smoothClients.integer )
    BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue );
  else
    BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );

  SendPendingPredictableEvents( &ent->client->ps );

  if( !( ent->client->ps.eFlags & EF_FIRING ) )
    client->fireHeld = qfalse;    // for grapple
  if( !( ent->client->ps.eFlags & EF_FIRING2 ) )
    client->fire2Held = qfalse;

  // use the snapped origin for linking so it matches client predicted versions
  VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );

  VectorCopy( pm.mins, ent->r.mins );
  VectorCopy( pm.maxs, ent->r.maxs );

  ent->waterlevel = pm.waterlevel;
  ent->watertype = pm.watertype;

  // execute client events
  ClientEvents( ent, oldEventSequence );

  // link entity now, after any personal teleporters have been used
  trap_LinkEntity( ent );
  
  if( !ent->client->noclip )
    G_TouchTriggers( ent );

  // NOTE: now copy the exact origin over otherwise clients can be snapped into solid
  VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );

  // touch other objects
  ClientImpacts( ent, &pm );

  // save results of triggers and client events
  if( ent->client->ps.eventSequence != oldEventSequence )
    ent->eventTime = level.time;
              
  // swap and latch button actions
  client->oldbuttons = client->buttons;
  client->buttons = ucmd->buttons;
  client->latched_buttons |= client->buttons & ~client->oldbuttons;

  if( ( client->buttons & BUTTON_GETFLAG ) && !( client->oldbuttons & BUTTON_GETFLAG ) )
  {
    trace_t   trace;
    vec3_t    view, point;
    gentity_t *traceEnt;

    if( client->ps.stats[ STAT_STATE ] & SS_HOVELING )
    {
      gentity_t *hovel = client->hovel;
      
      //only let the player out if there is room
      if( !AHovel_Blocked( hovel, ent, qtrue ) )
      {
        //prevent lerping
        client->ps.eFlags ^= EF_TELEPORT_BIT;
        client->ps.eFlags &= ~EF_NODRAW;

        //client leaves hovel
        client->ps.stats[ STAT_STATE ] &= ~SS_HOVELING;
        
        //hovel is empty
        G_setBuildableAnim( hovel, BANIM_ATTACK1, qfalse );
        hovel->active = qfalse;
      }
      else
      {
        //exit is blocked
        G_TriggerMenu( ent->client->ps.clientNum, MN_A_HOVEL_BLOCKED );
      }
    }
    else
    {
#define USE_OBJECT_RANGE 64
      
      int       entityList[ MAX_GENTITIES ];
      vec3_t    range = { USE_OBJECT_RANGE, USE_OBJECT_RANGE, USE_OBJECT_RANGE };
      vec3_t    mins, maxs, dir;
      int       i, num;
      
      //TA: look for object infront of player
      AngleVectors( client->ps.viewangles, view, NULL, NULL );
      VectorMA( client->ps.origin, USE_OBJECT_RANGE, view, point );
      trap_Trace( &trace, client->ps.origin, NULL, NULL, point, ent->s.number, MASK_SHOT );

      traceEnt = &g_entities[ trace.entityNum ];

      if( traceEnt->use )
        traceEnt->use( traceEnt, ent, ent ); //other and activator are the same in this context
      else
      {
        //no entity in front of player - do a small area search

        VectorAdd( client->ps.origin, range, maxs );
        VectorSubtract( client->ps.origin, range, mins );
        
        num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
        for( i = 0; i < num; i++ )
        {
          traceEnt = &g_entities[ entityList[ i ] ];
          
          if( traceEnt->use )
          {
            traceEnt->use( traceEnt, ent, ent ); //other and activator are the same in this context
            break;
          }
        }
      
        if( i == num && client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS )
        {
          if( client->ps.persistant[ PERS_CREDIT ] > 0 )
          {
            //no nearby objects and alien - show class menu
            G_TriggerMenu( ent->client->ps.clientNum, MN_A_INFEST );
          }
        }
      }
    }
  }
  
  // check for respawning
  if( client->ps.stats[ STAT_HEALTH ] <= 0 )
  {
    // wait for the attack button to be pressed
    if( level.time > client->respawnTime )
    {
      // forcerespawn is to prevent users from waiting out powerups
      if( g_forcerespawn.integer > 0 &&
        ( level.time - client->respawnTime ) > 0 )
      {
        respawn( ent );
        return;
      }

      // pressing attack or use is the normal respawn method
      if( ucmd->buttons & ( BUTTON_ATTACK | BUTTON_USE_HOLDABLE ) )
      {
        respawn( ent );
      }
    }
    return;
  }

  if( level.framenum > client->retriggerArmouryMenu && client->retriggerArmouryMenu )
  {
    G_TriggerMenu( client->ps.clientNum, MN_H_ARMOURY );

    client->retriggerArmouryMenu = 0;
  }

  // perform once-a-second actions
  ClientTimerActions( ent, msec );
}

/*
==================
ClientThink

A new command has arrived from the client
==================
*/
void ClientThink( int clientNum )
{
  gentity_t *ent;

  ent = g_entities + clientNum;
  trap_GetUsercmd( clientNum, &ent->client->pers.cmd );

  // mark the time we got info, so we can display the
  // phone jack if they don't get any for a while
  ent->client->lastCmdTime = level.time;

  if( !(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer )
    ClientThink_real( ent );
}


void G_RunClient( gentity_t *ent )
{
  if( !( ent->r.svFlags & SVF_BOT ) && !g_synchronousClients.integer )
    return;
  
  ent->client->pers.cmd.serverTime = level.time;
  ClientThink_real( ent );
}


/*
==================
SpectatorClientEndFrame

==================
*/
void SpectatorClientEndFrame( gentity_t *ent )
{
  gclient_t *cl;

  // if we are doing a chase cam or a remote view, grab the latest info
  if( ent->client->sess.spectatorState == SPECTATOR_FOLLOW )
  {
    int   clientNum, flags;

    clientNum = ent->client->sess.spectatorClient;

    // team follow1 and team follow2 go to whatever clients are playing
    if( clientNum == -1 )
      clientNum = level.follow1;
    else if( clientNum == -2 )
      clientNum = level.follow2;

    if( clientNum >= 0 )
    {
      cl = &level.clients[ clientNum ];
      
      if( cl->pers.connected == CON_CONNECTED && cl->sess.sessionTeam != TEAM_SPECTATOR )
      {
        flags = ( cl->ps.eFlags & ~( EF_VOTED | EF_TEAMVOTED ) ) | ( ent->client->ps.eFlags & ( EF_VOTED | EF_TEAMVOTED ) );
        ent->client->ps = cl->ps;
        ent->client->ps.pm_flags |= PMF_FOLLOW;
        ent->client->ps.eFlags = flags;
        return;
      }
      else
      {
        // drop them to free spectators unless they are dedicated camera followers
        if( ent->client->sess.spectatorClient >= 0 )
        {
          ent->client->sess.spectatorState = SPECTATOR_FREE;
          ClientBegin( ent->client - level.clients );
        }
      }
    }
  }

  if( ent->client->sess.spectatorState == SPECTATOR_SCOREBOARD )
    ent->client->ps.pm_flags |= PMF_SCOREBOARD;
  else
    ent->client->ps.pm_flags &= ~PMF_SCOREBOARD;
}

/*
==============
ClientEndFrame

Called at the end of each server frame for each connected client
A fast client will have multiple ClientThink for each ClientEdFrame,
while a slow client may have multiple ClientEndFrame between ClientThink.
==============
*/
void ClientEndFrame( gentity_t *ent )
{
  int                 i;
  clientPersistant_t  *pers;

  if( ent->client->sess.sessionTeam == TEAM_SPECTATOR )
  {
    SpectatorClientEndFrame( ent );
    return;
  }

  pers = &ent->client->pers;

  //
  // If the end of unit layout is displayed, don't give
  // the player any normal movement attributes
  //
  if( level.intermissiontime )
    return;

  // burn from lava, etc
  P_WorldEffects( ent );

  // apply all the damage taken this frame
  P_DamageFeedback( ent );

  // add the EF_CONNECTION flag if we haven't gotten commands recently
  if( level.time - ent->client->lastCmdTime > 1000 )
    ent->s.eFlags |= EF_CONNECTION;
  else
    ent->s.eFlags &= ~EF_CONNECTION;

  ent->client->ps.stats[ STAT_HEALTH ] = ent->health; // FIXME: get rid of ent->health...

  G_SetClientSound( ent );

  // set the latest infor
  if( g_smoothClients.integer )
    BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue );
  else
    BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
  
  SendPendingPredictableEvents( &ent->client->ps );
}


