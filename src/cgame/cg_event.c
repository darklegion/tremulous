/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2013 Darklegion Development

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

// cg_event.c -- handle entity events at snapshot or playerstate transitions


#include "cg_local.h"

/*
=======================
CG_AddToKillMsg

=======================
*/
void CG_AddToKillMsg( const char* killername, const char* victimname, int icon )
{
  int   klen, vlen, index;
  char  *kls, *vls;
  char  *k, *v;
  int   lastcolor;
  int   chatHeight;

  if( cg_killMsgHeight.integer < TEAMCHAT_HEIGHT )
    chatHeight = cg_killMsgHeight.integer;
  else
    chatHeight = TEAMCHAT_HEIGHT;

  if( chatHeight <= 0 || cg_killMsgTime.integer <= 0 ) {
    cgs.killMsgPos = cgs.killMsgLastPos = 0;
    return;
  }

  index = cgs.killMsgPos % chatHeight;
  klen = vlen = 0;

  k = cgs.killMsgKillers[ index ]; *k=0;
  v = cgs.killMsgVictims[ index ]; *v=0;
  cgs.killMsgWeapons[ index ] = icon;

  memset( k, '\0', sizeof(cgs.killMsgKillers[index]));
  memset( v, '\0', sizeof(cgs.killMsgVictims[index]));
  kls = vls = NULL;

  lastcolor = '7';

  // Killers name
  while( *killername )
  {
    if( klen > TEAMCHAT_WIDTH-1 ) {
      if( kls ) {
        killername -= ( k - kls );
        killername ++;
        k -= ( k - kls );
      }
      *k = 0;

//      cgs.killMsgMsgTimes[index] = cg.time;
      k = cgs.killMsgKillers[index];
      *k = 0;
      *k++ = Q_COLOR_ESCAPE;
      *k++ = lastcolor;
      klen = 0;
      kls = NULL;
    }

    if( Q_IsColorString( killername ) )
    {
      *k++ = *killername++;
      lastcolor = *killername;
      *k++ = *killername++;
      continue;
    }

    if( *killername == ' ' )
      kls = k;

    *k++ = *killername++;
    klen++;
  }

  // Victims name
  if (victimname)
      while( *victimname )
      {
        if( vlen > TEAMCHAT_WIDTH-1 ) {
          if( vls ) {
            victimname -= ( v - vls );
            victimname ++;
            v -= ( v - vls );
          }
          *v = 0;

          v = cgs.killMsgVictims[index];
          *v = 0;
          *v++ = Q_COLOR_ESCAPE;
          *v++ = lastcolor;
          vlen = 0;
          vls = NULL;
        }

        if( Q_IsColorString( victimname ) )
        {
          *v++ = *victimname++;
          lastcolor = *victimname;
          *v++ = *victimname++;
          continue;
        }

        if( *victimname == ' ' )
          vls = v;

        *v++ = *victimname++;
        vlen++;
      }

  cgs.killMsgMsgTimes[ index ] = cg.time;
  cgs.killMsgPos++;

  if( cgs.killMsgPos - cgs.killMsgLastPos > chatHeight )
    cgs.killMsgLastPos = cgs.killMsgPos - chatHeight;
}

/*
=============
CG_Obituary
=============
*/
static void CG_Obituary( entityState_t *ent )
{
  int           mod;
  int           target, attacker;
  char          *message;
  char          *message2;
  const char    *targetInfo;
  const char    *attackerInfo;
  char          targetName[ MAX_NAME_LENGTH ];
  char          attackerName[ MAX_NAME_LENGTH ];
  char          className[ 64 ];
  gender_t      gender;
  clientInfo_t  *ci;
  qboolean      teamKill = qfalse;
  int           icon = WP_NONE;

  target = ent->otherEntityNum;
  attacker = ent->otherEntityNum2;
  mod = ent->eventParm;

  if( target < 0 || target >= MAX_CLIENTS )
    CG_Error( "CG_Obituary: target out of range" );

  ci = &cgs.clientinfo[ target ];
  gender = ci->gender;

  if( attacker < 0 || attacker >= MAX_CLIENTS )
  {
    attacker = ENTITYNUM_WORLD;
    attackerInfo = NULL;
  }
  else
  {
    attackerInfo = CG_ConfigString( CS_PLAYERS + attacker );
    if( ci && cgs.clientinfo[ attacker ].team == ci->team )
      teamKill = qtrue;
  }

  targetInfo = CG_ConfigString( CS_PLAYERS + target );

  if( !targetInfo )
    return;

  Q_strncpyz( targetName, Info_ValueForKey( targetInfo, "n" ), sizeof( targetName ));

  message2 = "";

  // check for single client messages

  switch( mod )
  {
    case MOD_FALLING:
      message = "fell fowl to gravity";
      break;
    case MOD_CRUSH:
      message = "was squished";
      break;
    case MOD_WATER:
      message = "forgot to pack a snorkel";
      break;
    case MOD_SLIME:
      message = "melted";
      break;
    case MOD_LAVA:
      message = "did a back flip into the lava";
      break;
    case MOD_TARGET_LASER:
      message = "saw the light";
      break;
    case MOD_TRIGGER_HURT:
      message = "was in the wrong place";
      break;
    case MOD_HSPAWN:
      message = "should have run further";
      break;
    case MOD_ASPAWN:
      message = "shouldn't have trod in the acid";
      break;
    case MOD_MGTURRET:
      message = "was gunned down by a turret";
      break;
    case MOD_TESLAGEN:
      message = "was zapped by a tesla generator";
      break;
    case MOD_ATUBE:
      message = "was melted by an acid tube";
      break;
    case MOD_OVERMIND:
      message = "got too close to the overmind";
      break;
    case MOD_REACTOR:
      message = "got too close to the reactor";
      break;
    case MOD_SLOWBLOB:
      message = "should have visited a medical station";
      break;
    case MOD_SWARM:
      message = "was hunted down by the swarm";
      break;
    default:
      message = NULL;
      break;
  }

  if( !message && attacker == target )
  {
    switch( mod )
    {
      case MOD_FLAMER_SPLASH:
        if( gender == GENDER_FEMALE )
          message = "toasted herself";
        else if( gender == GENDER_NEUTER )
          message = "toasted itself";
        else
          message = "toasted himself";
        break;

      case MOD_LCANNON_SPLASH:
        if( gender == GENDER_FEMALE )
          message = "irradiated herself";
        else if( gender == GENDER_NEUTER )
          message = "irradiated itself";
        else
          message = "irradiated himself";
        break;

      case MOD_GRENADE:
        if( gender == GENDER_FEMALE )
          message = "blew herself up";
        else if( gender == GENDER_NEUTER )
          message = "blew itself up";
        else
          message = "blew himself up";
        break;

      case MOD_LEVEL3_BOUNCEBALL:
        if( gender == GENDER_FEMALE )
          message = "sniped herself";
        else if( gender == GENDER_NEUTER )
          message = "sniped itself";
        else
          message = "sniped himself";
        break;

      case MOD_PRIFLE:
        if( gender == GENDER_FEMALE )
          message = "pulse rifled herself";
        else if( gender == GENDER_NEUTER )
          message = "pulse rifled itself";
        else
          message = "pulse rifled himself";
        break;

      default:
        if( gender == GENDER_FEMALE )
          message = "killed herself";
        else if( gender == GENDER_NEUTER )
          message = "killed itself";
        else
          message = "killed himself";
        break;
    }

    if ( cg_killMsg.integer == 2)
    {
      CG_AddToKillMsg(va("%s ^7%s", targetName, message), NULL, WP_NONE);
      return;
    }
  }

  // check for double client messages
  if( !attackerInfo )
  {
    attacker = ENTITYNUM_WORLD;
    strcpy( attackerName, "noname" );
  }
  else
  {
    Q_strncpyz( attackerName, Info_ValueForKey( attackerInfo, "n" ), sizeof( attackerName ));
    // check for kill messages about the current clientNum
    if( target == cg.snap->ps.clientNum )
      Q_strncpyz( cg.killerName, attackerName, sizeof( cg.killerName ) );
  }

  if( attacker != ENTITYNUM_WORLD )
  {
    switch( mod )
    {
      // 
      // HUMANS
      //
      case MOD_PAINSAW:
        icon = WP_PAIN_SAW;
        message = "was sawn by";
        break;
      case MOD_BLASTER:
        icon = WP_BLASTER;
        message = "was blasted by";
        break;
      case MOD_MACHINEGUN:
        icon = WP_MACHINEGUN;
        message = "was machinegunned by";
        break;
      case MOD_CHAINGUN:
        icon = WP_CHAINGUN;
        message = "was chaingunned by";
        break;
      case MOD_SHOTGUN:
        icon = WP_SHOTGUN;
        message = "was gunned down by";
        break;
      case MOD_PRIFLE:
        icon = WP_PULSE_RIFLE;
        message = "was pulse rifled by";
        break;
      case MOD_MDRIVER:
        icon = WP_MASS_DRIVER;
        message = "was mass driven by";
        break;
      case MOD_LASGUN:
        icon = WP_LAS_GUN;
        message = "was lasgunned by";
        break;
      case MOD_FLAMER:
      case MOD_FLAMER_SPLASH:
        icon = WP_FLAMER;
        message = "was grilled by";
        message2 = "'s flamer";
        break;
      case MOD_LCANNON:
        icon = WP_LUCIFER_CANNON;
        message = "felt the full force of";
        message2 = "'s lucifer cannon";
        break;
      case MOD_LCANNON_SPLASH:
        icon = WP_LUCIFER_CANNON;
        message = "was caught in the fallout of";
        message2 = "'s lucifer cannon";
        break;
      case MOD_GRENADE:
        icon = WP_GRENADE;
        message = "couldn't escape";
        message2 = "'s grenade";
        break;

      // 
      // ALIENS
      //
      case MOD_ABUILDER_CLAW:
        icon = WP_ABUILD;
        message = "should leave";
        message2 = "'s buildings alone";
        break;
      case MOD_LEVEL0_BITE:
        icon = WP_ALEVEL0;
        message = "was bitten by";
        break;
      case MOD_LEVEL1_CLAW:
        icon = WP_ALEVEL1;
        message = "was swiped by";
        Com_sprintf( className, 64, "'s %s", BG_ClassConfig( PCL_ALIEN_LEVEL1 )->humanName );
        message2 = className;
        break;
      case MOD_LEVEL1_PCLOUD:
        icon = WP_ALEVEL1;
        message = "was gassed by";
        Com_sprintf( className, 64, "'s %s", BG_ClassConfig( PCL_ALIEN_LEVEL1 )->humanName );
        message2 = className;
        break;
      case MOD_LEVEL2_CLAW:
        icon = WP_ALEVEL2;
        message = "was clawed by";
        Com_sprintf( className, 64, "'s %s", BG_ClassConfig( PCL_ALIEN_LEVEL2 )->humanName );
        message2 = className;
        break;
      case MOD_LEVEL2_ZAP:
        icon = WP_ALEVEL2;
        message = "was zapped by";
        Com_sprintf( className, 64, "'s %s", BG_ClassConfig( PCL_ALIEN_LEVEL2 )->humanName );
        message2 = className;
        break;
      case MOD_LEVEL3_CLAW:
        icon = WP_ALEVEL3;
        message = "was chomped by";
        Com_sprintf( className, 64, "'s %s", BG_ClassConfig( PCL_ALIEN_LEVEL3 )->humanName );
        message2 = className;
        break;
      case MOD_LEVEL3_POUNCE:
        icon = WP_ALEVEL3;
        message = "was pounced upon by";
        Com_sprintf( className, 64, "'s %s", BG_ClassConfig( PCL_ALIEN_LEVEL3 )->humanName );
        message2 = className;
        break;
      case MOD_LEVEL3_BOUNCEBALL:
        icon = WP_ALEVEL3;
        message = "was sniped by";
        Com_sprintf( className, 64, "'s %s", BG_ClassConfig( PCL_ALIEN_LEVEL3 )->humanName );
        message2 = className;
        break;
      case MOD_LEVEL4_CLAW:
        icon = WP_ALEVEL4;
        message = "was mauled by";
        Com_sprintf( className, 64, "'s %s", BG_ClassConfig( PCL_ALIEN_LEVEL4 )->humanName );
        message2 = className;
        break;
      case MOD_LEVEL4_TRAMPLE:
        icon = WP_ALEVEL4;
        message = "should have gotten out of the way of";
        Com_sprintf( className, 64, "'s %s", BG_ClassConfig( PCL_ALIEN_LEVEL4 )->humanName );
        message2 = className;
        break;
      case MOD_LEVEL4_CRUSH:
        message = "was crushed under";
        message2 = "'s weight";
        break;
      case MOD_POISON:
        message = "should have used a medkit against";
        message2 = "'s poison";
        break;

      //
      // MISC..
      //
      case MOD_TELEFRAG:
        message = "tried to invade";
        message2 = "'s personal space";
        break;
      default:
        message = "was killed by";
        break;
    }

    if ( cg_killMsg.integer == 1)
    {
      char killMessage[80];
      if( icon > WP_NONE )
      {
        Com_sprintf(killMessage, sizeof(killMessage), "%s%s",
                    teamKill ? S_COLOR_RED"TEAMMATE "S_COLOR_WHITE:"",
                    targetName);
        CG_AddToKillMsg(attackerName, killMessage, icon);
      }
    }
    else if( message )
    {
      CG_Printf( "%s" S_COLOR_WHITE " %s %s%s" S_COLOR_WHITE "%s\n",
              targetName,
              message,
              teamKill ? S_COLOR_RED "TEAMMATE " S_COLOR_WHITE : "",
              attackerName,
              message2 );
    }

    if( attacker == cg.clientNum )
    {
        CG_CenterPrint(va("You killed %s%s", teamKill ? S_COLOR_RED"TEAMMATE "S_COLOR_WHITE:"",
                            targetName), SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
    }
    if ( cg_killMsg.integer != 1)
        return;
  }

#if 0
  switch (mod) {
      case MOD_SLAP:
          message = "was slapped to death";
          break;
      default:
          break;
  }
#endif

  // we don't know what it was
  if ( cg_killMsg.integer )
  {
    CG_AddToKillMsg(va("%s ^7%s", targetName, message), NULL, WP_NONE);
  }
  else
  {
    CG_Printf( "%s" S_COLOR_WHITE " %s\n", targetName, message );
  }
}

//==========================================================================

/*
================
CG_PainEvent

Also called by playerstate transition
================
*/
void CG_PainEvent( centity_t *cent, int health )
{
  char  *snd;

  // don't do more than two pain sounds a second
  if( cg.time - cent->pe.painTime < 500 )
    return;

  if( health < 25 )
    snd = "*pain25_1.wav";
  else if( health < 50 )
    snd = "*pain50_1.wav";
  else if( health < 75 )
    snd = "*pain75_1.wav";
  else
    snd = "*pain100_1.wav";

  trap_S_StartSound( NULL, cent->currentState.number, CHAN_VOICE,
    CG_CustomSound( cent->currentState.number, snd ) );

  // save pain time for programitic twitch animation
  cent->pe.painTime = cg.time;
  cent->pe.painDirection ^= 1;
}

/*
=========================
CG_Level2Zap
=========================
*/
static void CG_Level2Zap( entityState_t *es )
{
  int           i;
  centity_t     *source = NULL, *target = NULL;

  if( es->misc < 0 || es->misc >= MAX_CLIENTS )
    return;

  source = &cg_entities[ es->misc ];
  for( i = 0; i <= 2; i++ )
  {
    switch( i )
    {
      case 0:
        if( es->time <= 0 )
          continue;

        target = &cg_entities[ es->time ];
        break;

      case 1:
        if( es->time2 <= 0 )
          continue;

        target = &cg_entities[ es->time2 ];
        break;

      case 2:
        if( es->constantLight <= 0 )
          continue;

        target = &cg_entities[ es->constantLight ];
        break;
    }

    if( !CG_IsTrailSystemValid( &source->level2ZapTS[ i ] ) )
      source->level2ZapTS[ i ] = CG_SpawnNewTrailSystem( cgs.media.level2ZapTS );

    if( CG_IsTrailSystemValid( &source->level2ZapTS[ i ] ) )
    {
      CG_SetAttachmentCent( &source->level2ZapTS[ i ]->frontAttachment, source );
      CG_SetAttachmentCent( &source->level2ZapTS[ i ]->backAttachment, target );
      CG_AttachToCent( &source->level2ZapTS[ i ]->frontAttachment );
      CG_AttachToCent( &source->level2ZapTS[ i ]->backAttachment );
    }
  }
  source->level2ZapTime = cg.time;
}

/*
==============
CG_EntityEvent

An entity has an event value
also called by CG_CheckPlayerstateEvents
==============
*/
void CG_EntityEvent( centity_t *cent, vec3_t position )
{
  entityState_t *es;
  int           event;
  vec3_t        dir;
  const char    *s;
  int           clientNum;
  clientInfo_t  *ci;
  int           steptime;

  if( cg.snap->ps.persistant[ PERS_SPECSTATE ] != SPECTATOR_NOT )
    steptime = 200;
  else
    steptime = BG_Class( cg.snap->ps.stats[ STAT_CLASS ] )->steptime;

  es = &cent->currentState;
  event = es->event & ~EV_EVENT_BITS;

  if( cg_debugEvents.integer )
    CG_Printf( "ent:%3i  event:%3i %s\n", es->number, event,
               BG_EventName( event ) );

  if( !event )
    return;

  clientNum = es->clientNum;
  if( clientNum < 0 || clientNum >= MAX_CLIENTS )
    clientNum = 0;

  ci = &cgs.clientinfo[ clientNum ];

  switch( event )
  {
    //
    // movement generated events
    //
    case EV_FOOTSTEP:
      if( cg_footsteps.integer && ci->footsteps != FOOTSTEP_NONE )
      {
        if( ci->footsteps == FOOTSTEP_CUSTOM )
          trap_S_StartSound( NULL, es->number, CHAN_BODY,
            ci->customFootsteps[ rand( ) & 3 ] );
        else
          trap_S_StartSound( NULL, es->number, CHAN_BODY,
            cgs.media.footsteps[ ci->footsteps ][ rand( ) & 3 ] );
      }
      break;

    case EV_FOOTSTEP_METAL:
      if( cg_footsteps.integer && ci->footsteps != FOOTSTEP_NONE )
      {
        if( ci->footsteps == FOOTSTEP_CUSTOM )
          trap_S_StartSound( NULL, es->number, CHAN_BODY,
            ci->customMetalFootsteps[ rand( ) & 3 ] );
        else
          trap_S_StartSound( NULL, es->number, CHAN_BODY,
            cgs.media.footsteps[ FOOTSTEP_METAL ][ rand( ) & 3 ] );
      }
      break;

    case EV_FOOTSTEP_SQUELCH:
      if( cg_footsteps.integer && ci->footsteps != FOOTSTEP_NONE )
      {
        trap_S_StartSound( NULL, es->number, CHAN_BODY,
          cgs.media.footsteps[ FOOTSTEP_FLESH ][ rand( ) & 3 ] );
      }
      break;

    case EV_FOOTSPLASH:
      if( cg_footsteps.integer && ci->footsteps != FOOTSTEP_NONE )
      {
        trap_S_StartSound( NULL, es->number, CHAN_BODY,
          cgs.media.footsteps[ FOOTSTEP_SPLASH ][ rand( ) & 3 ] );
      }
      break;

    case EV_FOOTWADE:
      if( cg_footsteps.integer && ci->footsteps != FOOTSTEP_NONE )
      {
        trap_S_StartSound( NULL, es->number, CHAN_BODY,
          cgs.media.footsteps[ FOOTSTEP_SPLASH ][ rand( ) & 3 ] );
      }
      break;

    case EV_SWIM:
      if( cg_footsteps.integer && ci->footsteps != FOOTSTEP_NONE )
      {
        trap_S_StartSound( NULL, es->number, CHAN_BODY,
          cgs.media.footsteps[ FOOTSTEP_SPLASH ][ rand( ) & 3 ] );
      }
      break;


    case EV_FALL_SHORT:
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.landSound );

      if( clientNum == cg.predictedPlayerState.clientNum )
      {
        // smooth landing z changes
        cg.landChange = -1 * BG_Class( cg.predictedPlayerState.stats[ STAT_CLASS ] )->landBob;
        cg.landTime = cg.time;
      }
      break;

    case EV_FALL_MEDIUM:
      // use normal pain sound
      trap_S_StartSound( NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*pain100_1.wav" ) );

      if( clientNum == cg.predictedPlayerState.clientNum )
      {
        // smooth landing z changes
        cg.landChange = -2 * BG_Class( cg.predictedPlayerState.stats[ STAT_CLASS ] )->landBob;
        cg.landTime = cg.time;
      }
      break;

    case EV_FALL_FAR:
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*fall1.wav" ) );
      cent->pe.painTime = cg.time;  // don't play a pain sound right after this

      if( clientNum == cg.predictedPlayerState.clientNum )
      {
        // smooth landing z changes
        cg.landChange = -3 * BG_Class( cg.predictedPlayerState.stats[ STAT_CLASS ] )->landBob;
        cg.landTime = cg.time;
      }
      break;

    case EV_FALLING:
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*falling1.wav" ) );
      break;

    case EV_STEP_4:
    case EV_STEP_8:
    case EV_STEP_12:
    case EV_STEP_16:    // smooth out step up transitions
    case EV_STEPDN_4:
    case EV_STEPDN_8:
    case EV_STEPDN_12:
    case EV_STEPDN_16:    // smooth out step down transitions
      {
        float  oldStep;
        int    delta;
        int    step;

        if( clientNum != cg.predictedPlayerState.clientNum )
          break;

        // if we are interpolating, we don't need to smooth steps
        if( cg.demoPlayback || ( cg.snap->ps.pm_flags & PMF_FOLLOW ) ||
            cg_nopredict.integer || cg_synchronousClients.integer )
          break;

        // check for stepping up before a previous step is completed
        delta = cg.time - cg.stepTime;

        if( delta < steptime )
          oldStep = cg.stepChange * ( steptime - delta ) / steptime;
        else
          oldStep = 0;

        // add this amount
        if( event >= EV_STEPDN_4 )
        {
          step = 4 * ( event - EV_STEPDN_4 + 1 );
          cg.stepChange = oldStep - step;
        }
        else
        {
          step = 4 * ( event - EV_STEP_4 + 1 );
          cg.stepChange = oldStep + step;
        }

        if( cg.stepChange > MAX_STEP_CHANGE )
          cg.stepChange = MAX_STEP_CHANGE;
        else if( cg.stepChange < -MAX_STEP_CHANGE )
          cg.stepChange = -MAX_STEP_CHANGE;

        cg.stepTime = cg.time;
        break;
      }

    case EV_JUMP:
      trap_S_StartSound( NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*jump1.wav" ) );

      if( BG_ClassHasAbility( cg.predictedPlayerState.stats[ STAT_CLASS ], SCA_WALLJUMPER ) )
      {
        vec3_t  surfNormal, refNormal = { 0.0f, 0.0f, 1.0f };
        vec3_t  rotAxis;

        if( clientNum != cg.predictedPlayerState.clientNum )
          break;

        //set surfNormal
        VectorCopy( cg.predictedPlayerState.grapplePoint, surfNormal );

        //if we are moving from one surface to another smooth the transition
        if( !VectorCompare( surfNormal, cg.lastNormal ) && surfNormal[ 2 ] != 1.0f )
        {
          CrossProduct( refNormal, surfNormal, rotAxis );
          VectorNormalize( rotAxis );

          //add the op
          CG_addSmoothOp( rotAxis, 15.0f, 1.0f );
        }

        //copy the current normal to the lastNormal
        VectorCopy( surfNormal, cg.lastNormal );
      }

      break;

    case EV_LEV1_GRAB:
      trap_S_StartSound( NULL, es->number, CHAN_VOICE, cgs.media.alienL1Grab );
      break;

    case EV_LEV4_TRAMPLE_PREPARE:
      trap_S_StartSound( NULL, es->number, CHAN_VOICE, cgs.media.alienL4ChargePrepare );
      break;

    case EV_LEV4_TRAMPLE_START:
      //FIXME: stop cgs.media.alienL4ChargePrepare playing here
      trap_S_StartSound( NULL, es->number, CHAN_VOICE, cgs.media.alienL4ChargeStart );
      break;

    case EV_TAUNT:
      if( !cg_noTaunt.integer )
        trap_S_StartSound( NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*taunt.wav" ) );
      break;

    case EV_WATER_TOUCH:
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.watrInSound );
      break;

    case EV_WATER_LEAVE:
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.watrOutSound );
      break;

    case EV_WATER_UNDER:
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.watrUnSound );
      break;

    case EV_WATER_CLEAR:
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*gasp.wav" ) );
      break;

    //
    // weapon events
    //
    case EV_NOAMMO:
      trap_S_StartSound( NULL, es->number, CHAN_WEAPON,
                         cgs.media.weaponEmptyClick );
      break;

    case EV_CHANGE_WEAPON:
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.selectSound );
      break;

    case EV_FIRE_WEAPON:
      CG_FireWeapon( cent, WPM_PRIMARY );
      break;

    case EV_FIRE_WEAPON2:
      CG_FireWeapon( cent, WPM_SECONDARY );
      break;

    case EV_FIRE_WEAPON3:
      CG_FireWeapon( cent, WPM_TERTIARY );
      break;

    //=================================================================

    //
    // other events
    //
    case EV_PLAYER_TELEPORT_IN:
      //deprecated
      break;

    case EV_PLAYER_TELEPORT_OUT:
      CG_PlayerDisconnect( position );
      break;

    case EV_BUILD_CONSTRUCT:
      //do something useful here
      break;

    case EV_BUILD_DESTROY:
      //do something useful here
      break;

    case EV_RPTUSE_SOUND:
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.repeaterUseSound );
      break;

    case EV_GRENADE_BOUNCE:
      if( rand( ) & 1 )
        trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.hardBounceSound1 );
      else
        trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.hardBounceSound2 );
      break;

    //
    // missile impacts
    //
    case EV_MISSILE_HIT:
      ByteToDir( es->eventParm, dir );
      CG_MissileHitEntity( es->weapon, es->generic1, position, dir, es->otherEntityNum, es->torsoAnim );
      break;

    case EV_MISSILE_MISS:
      ByteToDir( es->eventParm, dir );
      CG_MissileHitWall( es->weapon, es->generic1, 0, position, dir, IMPACTSOUND_DEFAULT, es->torsoAnim );
      break;

    case EV_MISSILE_MISS_METAL:
      ByteToDir( es->eventParm, dir );
      CG_MissileHitWall( es->weapon, es->generic1, 0, position, dir, IMPACTSOUND_METAL, es->torsoAnim );
      break;

    case EV_HUMAN_BUILDABLE_EXPLOSION:
      ByteToDir( es->eventParm, dir );
      CG_HumanBuildableExplosion( position, dir );
      break;

    case EV_ALIEN_BUILDABLE_EXPLOSION:
      ByteToDir( es->eventParm, dir );
      CG_AlienBuildableExplosion( position, dir );
      break;

    case EV_TESLATRAIL:
      cent->currentState.weapon = WP_TESLAGEN;
      {
        centity_t *source = &cg_entities[ es->misc ];
        centity_t *target = &cg_entities[ es->clientNum ];
        vec3_t    sourceOffset = { 0.0f, 0.0f, 28.0f };

        if( !CG_IsTrailSystemValid( &source->muzzleTS ) )
        {
          source->muzzleTS = CG_SpawnNewTrailSystem( cgs.media.teslaZapTS );

          if( CG_IsTrailSystemValid( &source->muzzleTS ) )
          {
            CG_SetAttachmentCent( &source->muzzleTS->frontAttachment, source );
            CG_SetAttachmentCent( &source->muzzleTS->backAttachment, target );
            CG_AttachToCent( &source->muzzleTS->frontAttachment );
            CG_AttachToCent( &source->muzzleTS->backAttachment );
            CG_SetAttachmentOffset( &source->muzzleTS->frontAttachment, sourceOffset );

            source->muzzleTSDeathTime = cg.time + cg_teslaTrailTime.integer;
          }
        }
      }
      break;

    case EV_BULLET_HIT_WALL:
      ByteToDir( es->eventParm, dir );
      CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qfalse, ENTITYNUM_WORLD );
      break;

    case EV_BULLET_HIT_FLESH:
      CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qtrue, es->eventParm );
      break;

    case EV_SHOTGUN:
      CG_ShotgunFire( es );
      break;

    case EV_GENERAL_SOUND:
      if( cgs.gameSounds[ es->eventParm ] )
        trap_S_StartSound( NULL, es->number, CHAN_VOICE, cgs.gameSounds[ es->eventParm ] );
      else
      {
        s = CG_ConfigString( CS_SOUNDS + es->eventParm );
        trap_S_StartSound( NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, s ) );
      }
      break;

    case EV_GLOBAL_SOUND: // play from the player's head so it never diminishes
      if( cgs.gameSounds[ es->eventParm ] )
        trap_S_StartSound( NULL, cg.snap->ps.clientNum, CHAN_AUTO, cgs.gameSounds[ es->eventParm ] );
      else
      {
        s = CG_ConfigString( CS_SOUNDS + es->eventParm );
        trap_S_StartSound( NULL, cg.snap->ps.clientNum, CHAN_AUTO, CG_CustomSound( es->number, s ) );
      }
      break;

    case EV_PAIN:
      // local player sounds are triggered in CG_CheckLocalSounds,
      // so ignore events on the player
      if( cent->currentState.number != cg.snap->ps.clientNum )
        CG_PainEvent( cent, es->eventParm );
      break;

    case EV_DEATH1:
    case EV_DEATH2:
    case EV_DEATH3:
      trap_S_StartSound( NULL, es->number, CHAN_VOICE,
          CG_CustomSound( es->number, va( "*death%i.wav", event - EV_DEATH1 + 1 ) ) );
      break;

    case EV_OBITUARY:
      CG_Obituary( es );
      break;

    case EV_GIB_PLAYER:
      // no gibbing
      break;

    case EV_STOPLOOPINGSOUND:
      trap_S_StopLoopingSound( es->number );
      es->loopSound = 0;
      break;

    case EV_DEBUG_LINE:
      CG_Beam( cent );
      break;

    case EV_BUILD_DELAY:
      if( clientNum == cg.predictedPlayerState.clientNum )
      {
        trap_S_StartLocalSound( cgs.media.buildableRepairedSound, CHAN_LOCAL_SOUND );
        cg.lastBuildAttempt = cg.time;
      }
      break;

    case EV_BUILD_REPAIR:
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.buildableRepairSound );
      break;

    case EV_BUILD_REPAIRED:
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.buildableRepairedSound );
      break;

    case EV_OVERMIND_ATTACK:
      if( cg.predictedPlayerState.stats[ STAT_TEAM ] == TEAM_ALIENS )
      {
        trap_S_StartLocalSound( cgs.media.alienOvermindAttack, CHAN_ANNOUNCER );
        CG_CenterPrint( "The Overmind is under attack!", 200, GIANTCHAR_WIDTH * 4 );
      }
      break;

    case EV_OVERMIND_DYING:
      if( cg.predictedPlayerState.stats[ STAT_TEAM ] == TEAM_ALIENS )
      {
        trap_S_StartLocalSound( cgs.media.alienOvermindDying, CHAN_ANNOUNCER );
        CG_CenterPrint( "The Overmind is dying!", 200, GIANTCHAR_WIDTH * 4 );
      }
      break;

    case EV_DCC_ATTACK:
      if( cg.predictedPlayerState.stats[ STAT_TEAM ] == TEAM_HUMANS )
      {
        //trap_S_StartLocalSound( cgs.media.humanDCCAttack, CHAN_ANNOUNCER );
        CG_CenterPrint( "Our base is under attack!", 200, GIANTCHAR_WIDTH * 4 );
      }
      break;

    case EV_MGTURRET_SPINUP:
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.turretSpinupSound );
      break;

    case EV_OVERMIND_SPAWNS:
      if( cg.predictedPlayerState.stats[ STAT_TEAM ] == TEAM_ALIENS )
      {
        trap_S_StartLocalSound( cgs.media.alienOvermindSpawns, CHAN_ANNOUNCER );
        CG_CenterPrint( "The Overmind needs spawns!", 200, GIANTCHAR_WIDTH * 4 );
      }
      break;

    case EV_ALIEN_EVOLVE:
      trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.alienEvolveSound );
      {
        particleSystem_t *ps = CG_SpawnNewParticleSystem( cgs.media.alienEvolvePS );

        if( CG_IsParticleSystemValid( &ps ) )
        {
          CG_SetAttachmentCent( &ps->attachment, cent );
          CG_AttachToCent( &ps->attachment );
        }
      }

      if( es->number == cg.clientNum )
      {
        CG_ResetPainBlend( );
        cg.spawnTime = cg.time;
      }
      break;

    case EV_ALIEN_EVOLVE_FAILED:
      if( clientNum == cg.predictedPlayerState.clientNum )
      {
        //FIXME: change to "negative" sound
        trap_S_StartLocalSound( cgs.media.buildableRepairedSound, CHAN_LOCAL_SOUND );
        cg.lastEvolveAttempt = cg.time;
      }
      break;

    case EV_ALIEN_ACIDTUBE:
      {
        particleSystem_t *ps = CG_SpawnNewParticleSystem( cgs.media.alienAcidTubePS );

        if( CG_IsParticleSystemValid( &ps ) )
        {
          CG_SetAttachmentCent( &ps->attachment, cent );
          ByteToDir( es->eventParm, dir );
          CG_SetParticleSystemNormal( ps, dir );
          CG_AttachToCent( &ps->attachment );
        }
      }
      break;

    case EV_MEDKIT_USED:
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.medkitUseSound );
      break;

    case EV_PLAYER_RESPAWN:
      if( es->number == cg.clientNum )
        cg.spawnTime = cg.time;
      break;

    case EV_LEV2_ZAP:
      CG_Level2Zap( es );
      break;

    default:
      CG_Error( "Unknown event: %i", event );
      break;
  }
}


/*
==============
CG_CheckEvents

==============
*/
void CG_CheckEvents( centity_t *cent )
{
  entity_event_t event;
  entity_event_t oldEvent = EV_NONE;

  // check for event-only entities
  if( cent->currentState.eType > ET_EVENTS )
  {
    event = cent->currentState.eType - ET_EVENTS;

    if( cent->previousEvent )
      return; // already fired

    cent->previousEvent = 1;

    cent->currentState.event = cent->currentState.eType - ET_EVENTS;
    
    // Move the pointer to the entity that the
    // event was originally attached to
    if( cent->currentState.eFlags & EF_PLAYER_EVENT )
    {
      cent = &cg_entities[ cent->currentState.otherEntityNum ];
      oldEvent = cent->currentState.event;
      cent->currentState.event = event;
    }
  }
  else
  {
    // check for events riding with another entity
    if( cent->currentState.event == cent->previousEvent )
      return;

    cent->previousEvent = cent->currentState.event;
    if( ( cent->currentState.event & ~EV_EVENT_BITS ) == 0 )
      return;
  }

  // calculate the position at exactly the frame time
  BG_EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, cent->lerpOrigin );
  CG_SetEntitySoundPosition( cent );

  CG_EntityEvent( cent, cent->lerpOrigin );
  
  // If this was a reattached spilled event, restore the original event
  if( oldEvent != EV_NONE )
    cent->currentState.event = oldEvent;
}

