// Copyright (C) 1999-2000 Id Software, Inc.
//
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

// g_client.c -- client functions that don't happen every frame

static vec3_t playerMins = {-15, -15, -24};
static vec3_t playerMaxs = {15, 15, 32};

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for deathmatch games.
The first time a player enters the game, they will be at an 'initial' spot.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_deathmatch( gentity_t *ent ) {
  int   i;

  G_SpawnInt( "nobots", "0", &i);
  if ( i ) {
    ent->flags |= FL_NO_BOTS;
  }
  G_SpawnInt( "nohumans", "0", &i );
  if ( i ) {
    ent->flags |= FL_NO_HUMANS;
  }
}

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
equivelant to info_player_deathmatch
*/
void SP_info_player_start(gentity_t *ent) {
  ent->classname = "info_player_deathmatch";
  SP_info_player_deathmatch( ent );
}

/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
*/
void SP_info_player_intermission( gentity_t *ent ) {

}

/*QUAKED info_droid_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
*/
void SP_info_droid_intermission( gentity_t *ent ) {

}

/*QUAKED info_human_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
*/
void SP_info_human_intermission( gentity_t *ent ) {

}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
SpotWouldTelefrag

================
*/
qboolean SpotWouldTelefrag( gentity_t *spot ) {
  int     i, num;
  int     touch[MAX_GENTITIES];
  gentity_t *hit;
  vec3_t    mins, maxs;

  VectorAdd( spot->s.origin, playerMins, mins );
  VectorAdd( spot->s.origin, playerMaxs, maxs );
  num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

  for (i=0 ; i<num ; i++) {
    hit = &g_entities[touch[i]];
    //if ( hit->client && hit->client->ps.stats[STAT_HEALTH] > 0 ) {
    if( hit->client ) {
      return qtrue;
    }

  }

  return qfalse;
}

/*
================
SelectNearestDeathmatchSpawnPoint

Find the spot that we DON'T want to use
================
*/
#define MAX_SPAWN_POINTS  128
gentity_t *SelectNearestDeathmatchSpawnPoint( vec3_t from ) {
  gentity_t *spot;
  vec3_t    delta;
  float   dist, nearestDist;
  gentity_t *nearestSpot;

  nearestDist = 999999;
  nearestSpot = NULL;
  spot = NULL;

  while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {

    VectorSubtract( spot->s.origin, from, delta );
    dist = VectorLength( delta );
    if ( dist < nearestDist ) {
      nearestDist = dist;
      nearestSpot = spot;
    }
  }

  return nearestSpot;
}


/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point that doesn't telefrag
================
*/
#define MAX_SPAWN_POINTS  128
gentity_t *SelectRandomDeathmatchSpawnPoint( void ) {
  gentity_t *spot;
  int     count;
  int     selection;
  gentity_t *spots[MAX_SPAWN_POINTS];

  count = 0;
  spot = NULL;

  while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
    if ( SpotWouldTelefrag( spot ) ) {
      continue;
    }
    spots[ count ] = spot;
    count++;
  }

  if ( !count ) { // no spots that won't telefrag
    return G_Find( NULL, FOFS(classname), "info_player_deathmatch");
  }

  selection = rand() % count;
  return spots[ selection ];
}


/*
===========
SelectRandomFurthestSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t *SelectRandomFurthestSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles ) {
  gentity_t *spot;
  vec3_t    delta;
  float   dist;
  float   list_dist[64];
  gentity_t *list_spot[64];
  int     numSpots, rnd, i, j;

  numSpots = 0;
  spot = NULL;

  while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
    if ( SpotWouldTelefrag( spot ) ) {
      continue;
    }
    VectorSubtract( spot->s.origin, avoidPoint, delta );
    dist = VectorLength( delta );
    for (i = 0; i < numSpots; i++) {
      if ( dist > list_dist[i] ) {
        if ( numSpots >= 64 )
          numSpots = 64-1;
        for (j = numSpots; j > i; j--) {
          list_dist[j] = list_dist[j-1];
          list_spot[j] = list_spot[j-1];
        }
        list_dist[i] = dist;
        list_spot[i] = spot;
        numSpots++;
        if (numSpots > 64)
          numSpots = 64;
        break;
      }
    }
    if (i >= numSpots && numSpots < 64) {
      list_dist[numSpots] = dist;
      list_spot[numSpots] = spot;
      numSpots++;
    }
  }
  if (!numSpots) {
    spot = G_Find( NULL, FOFS(classname), "info_player_deathmatch");
    if (!spot)
      G_Error( "Couldn't find a spawn point" );
    VectorCopy (spot->s.origin, origin);
    origin[2] += 9;
    VectorCopy (spot->s.angles, angles);
    return spot;
  }

  // select a random spot from the spawn points furthest away
  rnd = random() * (numSpots / 2);

  VectorCopy (list_spot[rnd]->s.origin, origin);
  origin[2] += 9;
  VectorCopy (list_spot[rnd]->s.angles, angles);

  return list_spot[rnd];
}


/*
================
SelectDroidSpawnPoint

go to a random point that doesn't telefrag
================
*/
gentity_t *SelectDroidSpawnPoint( void ) {
  gentity_t *spot;
  int     count;
  int     selection;
  gentity_t *spots[MAX_SPAWN_POINTS];

  count = 0;
  spot = NULL;

  while ((spot = G_Find (spot, FOFS(classname), "team_droid_spawn")) != NULL) {
    if ( SpotWouldTelefrag( spot ) || ( spot->health <= 0 ) ) {
      continue;
    }
    spots[ count ] = spot;
    count++;
  }

  if ( !count ) { // no spots that won't telefrag
    spot = G_Find( NULL, FOFS(classname), "team_droid_spawn");
    if( spot->health > 0 )
      return spot;
    else
      return NULL;

  }

  selection = rand() % count;
  return spots[ selection ];
}


/*
================
SelectHumanSpawnPoint

go to a random point that doesn't telefrag
================
*/
gentity_t *SelectHumanSpawnPoint( void ) {
  gentity_t *spot;
  int     count;
  int     selection;
  gentity_t *spots[MAX_SPAWN_POINTS];

  count = 0;
  spot = NULL;

  while ((spot = G_Find (spot, FOFS(classname), "team_human_spawn")) != NULL) {
    if ( SpotWouldTelefrag( spot ) || ( spot->health <= 0 ) ) {
      continue;
    }
    spots[ count ] = spot;
    count++;
  }

  if ( !count ) { // no spots that won't telefrag
    spot = G_Find( NULL, FOFS(classname), "team_human_spawn");
    if( spot->health > 0 )
      return spot;
    else
      return NULL;
  }

  selection = rand() % count;
  return spots[ selection ];
}


/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t *SelectSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles ) {
  return SelectRandomFurthestSpawnPoint( avoidPoint, origin, angles );

  /*
  gentity_t *spot;
  gentity_t *nearestSpot;

  nearestSpot = SelectNearestDeathmatchSpawnPoint( avoidPoint );

  spot = SelectRandomDeathmatchSpawnPoint ( );
  if ( spot == nearestSpot ) {
    // roll again if it would be real close to point of death
    spot = SelectRandomDeathmatchSpawnPoint ( );
    if ( spot == nearestSpot ) {
      // last try
      spot = SelectRandomDeathmatchSpawnPoint ( );
    }
  }

  // find a single player start spot
  if (!spot) {
    G_Error( "Couldn't find a spawn point" );
  }

  VectorCopy (spot->s.origin, origin);
  origin[2] += 9;
  VectorCopy (spot->s.angles, angles);

  return spot;
  */
}


/*
===========
SelectTremulousSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t *SelectTremulousSpawnPoint( int team, vec3_t origin, vec3_t angles )
{
  gentity_t *spot;

  if( team == PTE_DROIDS )
    spot = SelectDroidSpawnPoint( );
  else if( team == PTE_HUMANS )
    spot = SelectHumanSpawnPoint( );

  //no available spots
  if( !spot )
  {
    return NULL;
  }

  // find a single player start spot
  if (!spot) {
    //G_Error( "Couldn't find a spawn point" );
  }

  //TA: why isn't spot->s.origin being set?
  VectorCopy (spot->s.pos.trBase, origin);
  VectorCopy (spot->s.angles, angles);

  if( team == PTE_DROIDS )
    origin[2] += 40;
  else if( team == PTE_HUMANS )
    origin[2] += 29;

  return spot;
  
}


/*
===========
SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
gentity_t *SelectInitialSpawnPoint( vec3_t origin, vec3_t angles ) {
  gentity_t *spot;

  spot = NULL;
  while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
    if ( spot->spawnflags & 1 ) {
      break;
    }
  }

  if ( !spot || SpotWouldTelefrag( spot ) ) {
    return SelectSpawnPoint( vec3_origin, origin, angles );
  }

  VectorCopy (spot->s.origin, origin);
  origin[2] += 9;
  VectorCopy (spot->s.angles, angles);

  return spot;
}

/*
===========
SelectSpectatorSpawnPoint

============
*/
gentity_t *SelectSpectatorSpawnPoint( vec3_t origin, vec3_t angles ) {
  FindIntermissionPoint();

  VectorCopy( level.intermission_origin, origin );
  VectorCopy( level.intermission_angle, angles );

  return NULL;
}


/*
===========
SelectDroidLockSpawnPoint

Try to find a spawn point for droid intermission otherwise
use normal intermission spawn.
============
*/
gentity_t *SelectDroidLockSpawnPoint( vec3_t origin, vec3_t angles ) {
  gentity_t *spot;

  spot = NULL;
  spot = G_Find (spot, FOFS(classname), "info_droid_intermission");

  if ( !spot ) {
    return SelectSpectatorSpawnPoint( origin, angles );
  }

  VectorCopy (spot->s.origin, origin);
  VectorCopy (spot->s.angles, angles);

  return spot;
}


/*
===========
SelectHumanLockSpawnPoint

Try to find a spawn point for human intermission otherwise
use normal intermission spawn.
============
*/
gentity_t *SelectHumanLockSpawnPoint( vec3_t origin, vec3_t angles ) {
  gentity_t *spot;

  spot = NULL;
  spot = G_Find (spot, FOFS(classname), "info_human_intermission");

  if ( !spot ) {
    return SelectSpectatorSpawnPoint( origin, angles );
  }

  VectorCopy (spot->s.origin, origin);
  VectorCopy (spot->s.angles, angles);

  return spot;
}


/*
=======================================================================

BODYQUE

=======================================================================
*/


/*
=============
BodySink

After sitting around for five seconds, fall into the ground and dissapear
=============
*/
void BodySink( gentity_t *ent ) {
  if ( level.time - ent->timestamp > 6500 ) {
    // the body ques are never actually freed, they are just unlinked
    trap_UnlinkEntity( ent );
    ent->physicsObject = qfalse;
    return;
  }
  ent->nextthink = level.time + 100;
  ent->s.pos.trBase[2] -= 1;
}



/*
================
InfestBody

Called when a droid infests a body
================
*/
void InfestBody( gentity_t *self, gentity_t *other, gentity_t *activator )
{
  if( activator->client->ps.stats[ STAT_PTEAM ] != PTE_DROIDS ) return;

  G_AddPredictableEvent( activator, EV_MENU, MN_D_INFEST );
}

/*
=============
SpawnCorpse

A player is respawning, so make an entity that looks
just like the existing corpse to leave behind.
=============
*/
void SpawnCorpse( gentity_t *ent ) {
  gentity_t   *body;
  int         contents;
  vec3_t      origin, dest;
  trace_t     tr;

  VectorCopy( ent->r.currentOrigin, origin );

  trap_UnlinkEntity (ent);

  // if client is in a nodrop area, don't leave the body
  contents = trap_PointContents( ent->s.origin, -1 );
  if ( contents & CONTENTS_NODROP ) {
    return;
  }
  
  body = G_Spawn( );

  if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
    body->classname = "humanCorpse";
  else
    body->classname = "droidCorpse";
    
  body->s = ent->s;
  body->r.s = body->s;
  body->s.eFlags = EF_DEAD;
  body->s.eType = ET_CORPSE;
  body->s.number = body - g_entities;
  body->timestamp = level.time;
  body->s.event = 0;
  body->r.contents = CONTENTS_BODY;
  body->clipmask = MASK_PLAYERSOLID;
  body->s.clientNum = ent->client->ps.stats[ STAT_PCLASS ];
  
  if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
    body->use = InfestBody;

  switch ( body->s.legsAnim & ~ANIM_TOGGLEBIT ) {
  case BOTH_DEATH1:
  case BOTH_DEAD1:
    body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD1;
    break;
  case BOTH_DEATH2:
  case BOTH_DEAD2:
    body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD2;
    break;
  case BOTH_DEATH3:
  case BOTH_DEAD3:
  default:
    body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD3;
    break;
  }

  //body->die = body_die;

  // don't take more damage if already gibbed
  if ( ent->health <= GIB_HEALTH ) {
    body->takedamage = qfalse;
  } else {
    body->takedamage = qtrue;
  }

  //make the make player entity disappear
  ent->takedamage = qfalse;
  ent->s.eType = ET_INVISIBLE;
  ent->r.contents = 0;
  ent->s.solid = 0;
  ent->r.s.solid = 0;
  body->health = ent->health = ent->client->ps.stats[STAT_HEALTH];
  ent->health = ent->client->ps.stats[STAT_HEALTH] = GIB_HEALTH - 1;
  
  //change body dimensions
  BG_FindBBoxForClass( ent->client->ps.stats[ STAT_PCLASS ], NULL, NULL, NULL, body->r.mins, body->r.maxs );

  //drop to floor
  VectorSet( dest, origin[0], origin[1], origin[2] - 4096 );
  trap_Trace( &tr, origin, body->r.mins, body->r.maxs, dest, body->s.number, body->clipmask );
  VectorCopy( tr.endpos, origin );

  G_SetOrigin( body, origin );
  VectorCopy( origin, body->s.origin );
  body->s.pos.trType = TR_GRAVITY;
  body->s.pos.trTime = level.time;
  VectorCopy( ent->client->ps.velocity, body->s.pos.trDelta );
            
  VectorCopy ( body->s.pos.trBase, body->r.currentOrigin );
  trap_LinkEntity( body );

}

//======================================================================


/*
==================
SetClientViewAngle

==================
*/
void SetClientViewAngle( gentity_t *ent, vec3_t angle ) {
  int     i;

  // set the delta angle
  for (i=0 ; i<3 ; i++) {
    int   cmdAngle;

    cmdAngle = ANGLE2SHORT(angle[i]);
    ent->client->ps.delta_angles[i] = cmdAngle - ent->client->pers.cmd.angles[i];
  }
  VectorCopy( angle, ent->s.angles );
  VectorCopy (ent->s.angles, ent->client->ps.viewangles);
}

/*
================
respawn
================
*/
void respawn( gentity_t *ent ) {
  gentity_t *tent;

  SpawnCorpse( ent );

  //TA: Clients can't respawn - they must go thru the class cmd
  ClientSpawn( ent, NULL );

  //FIXME: need different spawn effects for different teams

  // add a teleportation effect
  //tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
  //tent->s.clientNum = ent->s.clientNum;
}

/*
================
TeamCount

Returns number of players on a team
================
*/
team_t TeamCount( int ignoreClientNum, int team ) {
  int   i;
  int   count = 0;

  for ( i = 0 ; i < level.maxclients ; i++ ) {
    if ( i == ignoreClientNum ) {
      continue;
    }
    if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
      continue;
    }
    if ( level.clients[i].sess.sessionTeam == team ) {
      count++;
    }
  }

  return count;
}


/*
================
TeamLeader

Returns the client number of the team leader
================
*/
int TeamLeader( int team ) {
  int   i;

  for ( i = 0 ; i < level.maxclients ; i++ ) {
    if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
      continue;
    }
    if ( level.clients[i].sess.sessionTeam == team ) {
      if ( level.clients[i].sess.teamLeader )
        return i;
    }
  }

  return -1;
}


/*
================
PickTeam

================
*/
team_t PickTeam( int ignoreClientNum ) {
  int   counts[TEAM_NUM_TEAMS];

  counts[TEAM_DROIDS] = TeamCount( ignoreClientNum, TEAM_DROIDS );
  counts[TEAM_HUMANS] = TeamCount( ignoreClientNum, TEAM_HUMANS );

  if ( counts[TEAM_DROIDS] > counts[TEAM_HUMANS] ) {
    return TEAM_HUMANS;
  }
  if ( counts[TEAM_HUMANS] > counts[TEAM_DROIDS] ) {
    return TEAM_DROIDS;
  }
  // equal team count, so join the team with the lowest score
  if ( level.teamScores[TEAM_DROIDS] > level.teamScores[TEAM_HUMANS] ) {
    return TEAM_HUMANS;
  }
  return TEAM_DROIDS;
}

/*
===========
ForceClientSkin

Forces a client's skin (for teamplay)
===========
*/
static void ForceClientSkin( gclient_t *client, char *model, const char *skin ) {
  char *p;

  if ((p = Q_strrchr(model, '/')) != 0) {
    *p = 0;
  }

  Q_strcat(model, MAX_QPATH, "/");
  Q_strcat(model, MAX_QPATH, skin);
}


/*
===========
ClientCheckName
============
*/
static void ClientCleanName( const char *in, char *out, int outSize ) {
  int   len, colorlessLen;
  char  ch;
  char  *p;
  int   spaces;

  //save room for trailing null byte
  outSize--;

  len = 0;
  colorlessLen = 0;
  p = out;
  *p = 0;
  spaces = 0;

  while( 1 ) {
    ch = *in++;
    if( !ch ) {
      break;
    }

    // don't allow leading spaces
    if( !*p && ch == ' ' ) {
      continue;
    }

    // check colors
    if( ch == Q_COLOR_ESCAPE ) {
      // solo trailing carat is not a color prefix
      if( !*in ) {
        break;
      }

      // don't allow black in a name, period
      if( ColorIndex(*in) == 0 ) {
        in++;
        continue;
      }

      // make sure room in dest for both chars
      if( len > outSize - 2 ) {
        break;
      }

      *out++ = ch;
      *out++ = *in++;
      len += 2;
      continue;
    }

    // don't allow too many consecutive spaces
    if( ch == ' ' ) {
      spaces++;
      if( spaces > 3 ) {
        continue;
      }
    }
    else {
      spaces = 0;
    }

    if( len > outSize - 1 ) {
      break;
    }

    *out++ = ch;
    colorlessLen++;
    len++;
  }
  *out = 0;

  // don't allow empty names
  if( *p == 0 || colorlessLen == 0 ) {
    Q_strncpyz( p, "UnnamedPlayer", outSize );
  }
}


/*
===========
ClientUserInfoChanged

Called from ClientConnect when the player first connects and
directly by the server system when the player updates a userinfo variable.

The game can override any of the settings and call trap_SetUserinfo
if desired.
============
*/
void ClientUserinfoChanged( int clientNum ) {
  gentity_t *ent;
  int   teamTask, teamLeader, team, health;
  char  *s;
  char  model[MAX_QPATH];
  char  oldname[MAX_STRING_CHARS];
  gclient_t *client;
  char  c1[MAX_INFO_STRING];
  char  redTeam[MAX_INFO_STRING];
  char  blueTeam[MAX_INFO_STRING];
  char  userinfo[MAX_INFO_STRING];

  ent = g_entities + clientNum;
  client = ent->client;

  trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

  // check for malformed or illegal info strings
  if ( !Info_Validate(userinfo) ) {
    strcpy (userinfo, "\\name\\badinfo");
  }

  // check for local client
  s = Info_ValueForKey( userinfo, "ip" );
  if ( !strcmp( s, "localhost" ) ) {
    client->pers.localClient = qtrue;
  }

  // check the item prediction
  s = Info_ValueForKey( userinfo, "cg_predictItems" );
  if ( !atoi( s ) ) {
    client->pers.predictItemPickup = qfalse;
  } else {
    client->pers.predictItemPickup = qtrue;
  }

  // set name
  Q_strncpyz ( oldname, client->pers.netname, sizeof( oldname ) );
  s = Info_ValueForKey (userinfo, "name");
  ClientCleanName( s, client->pers.netname, sizeof(client->pers.netname) );

  if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
    if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
      Q_strncpyz( client->pers.netname, "scoreboard", sizeof(client->pers.netname) );
    }
  }

  if ( client->pers.connected == CON_CONNECTED ) {
    if ( strcmp( oldname, client->pers.netname ) ) {
      trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " renamed to %s\n\"", oldname,
        client->pers.netname) );
    }
  }

  // set max health
  health = atoi( Info_ValueForKey( userinfo, "handicap" ) );
  client->pers.maxHealth = health;
  if ( client->pers.maxHealth < 1 || client->pers.maxHealth > 100 ) {
    client->pers.maxHealth = 100;
  }

  //hack to force a client update if the config string does not change between spawning
  if( client->pers.pclass == PCL_NONE )
    client->pers.maxHealth = 0;

  // set model
  s = BG_FindModelNameForClass( client->pers.pclass );
  Q_strncpyz( model, s, sizeof( model ) );

  // team
  switch( client->sess.sessionTeam ) {
  case TEAM_HUMANS:
    ForceClientSkin(client, model, "red");
    break;
  case TEAM_DROIDS:
    ForceClientSkin(client, model, "blue");
    break;
  }
  if ( g_gametype.integer >= GT_TEAM && client->sess.sessionTeam == TEAM_SPECTATOR ) {
    // don't ever use a default skin in teamplay, it would just waste memory
    ForceClientSkin(client, model, "red");
  }



  // teamInfo
  s = Info_ValueForKey( userinfo, "teamoverlay" );
  if ( ! *s || atoi( s ) != 0 ) {
    client->pers.teamInfo = qtrue;
  } else {
    client->pers.teamInfo = qfalse;
  }

  // team task (0 = none, 1 = offence, 2 = defence)
  teamTask = atoi(Info_ValueForKey(userinfo, "teamtask"));
  // team Leader (1 = leader, 0 is normal player)
  teamLeader = client->sess.teamLeader;

  // colors
  strcpy(c1, Info_ValueForKey( userinfo, "color" ));
  strcpy(redTeam, "humans");
  strcpy(blueTeam, "droids");
                
  // send over a subset of the userinfo keys so other clients can
  // print scoreboards, display models, and play custom sounds
  if ( ent->r.svFlags & SVF_BOT ) {
    s = va("n\\%s\\t\\%i\\model\\%s\\hmodel\\%s\\c1\\%s\\hc\\%i\\w\\%i\\l\\%i\\skill\\%s\\tt\\%d\\tl\\%d",
      client->pers.netname, client->sess.sessionTeam, model, model, c1,
      client->pers.maxHealth, client->sess.wins, client->sess.losses,
      Info_ValueForKey( userinfo, "skill" ), teamTask, teamLeader );
  } else {
    s = va("n\\%s\\t\\%i\\model\\%s\\hmodel\\%s\\g_redteam\\%s\\g_blueteam\\%s\\c1\\%s\\hc\\%i\\w\\%i\\l\\%i\\tt\\%d\\tl\\%d",
      client->pers.netname, client->sess.sessionTeam, model, model, redTeam, blueTeam, c1,
      client->pers.maxHealth, client->sess.wins, client->sess.losses, teamTask, teamLeader);
  }

  trap_SetConfigstring( CS_PLAYERS+clientNum, s );

  G_LogPrintf( "ClientUserinfoChanged: %i %s\n", clientNum, s );
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
Called again for every map change or tournement restart.

The session information will be valid after exit.

Return NULL if the client should be allowed, otherwise return
a string with the reason for denial.

Otherwise, the client will be sent the current gamestate
and will eventually get to ClientBegin.

firstTime will be qtrue the very first time a client connects
to the server machine, but qfalse on map changes and tournement
restarts.
============
*/
char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot ) {
  char    *value;
  gclient_t *client;
  char    userinfo[MAX_INFO_STRING];
  gentity_t *ent;

  ent = &g_entities[ clientNum ];

  trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

  // check to see if they are on the banned IP list
  value = Info_ValueForKey (userinfo, "ip");
  if ( G_FilterPacket( value ) ) {
    return "Banned.";
  }

  // check for a password
  value = Info_ValueForKey (userinfo, "password");
  if ( g_password.string[0] && Q_stricmp( g_password.string, "none" ) &&
    strcmp( g_password.string, value) != 0) {
    return "Invalid password";
  }

  // they can connect
  ent->client = level.clients + clientNum;
  client = ent->client;

  memset( client, 0, sizeof(*client) );

  client->pers.connected = CON_CONNECTING;

  // read or initialize the session data
  if ( firstTime || level.newSession ) {
    G_InitSessionData( client, userinfo );
  }
  G_ReadSessionData( client );

  //TA: rip bots
  /*if( isBot ) {
    ent->r.svFlags |= SVF_BOT;
    ent->inuse = qtrue;
    if( !G_BotConnect( clientNum, !firstTime ) ) {
      return "BotConnectfailed";
    }
  }*/

  // get and distribute relevent paramters
  G_LogPrintf( "ClientConnect: %i\n", clientNum );
  ClientUserinfoChanged( clientNum );

  // don't do the "xxx connected" messages if they were caried over from previous level
  if ( firstTime ) {
    trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " connected\n\"", client->pers.netname) );
  }

  if ( g_gametype.integer >= GT_TEAM &&
    client->sess.sessionTeam != TEAM_SPECTATOR ) {
    BroadcastTeamChange( client, -1 );
  }

  // count current clients and rank for scoreboard
  CalculateRanks();

  return NULL;
}

/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns
============
*/
void ClientBegin( int clientNum ) {
  gentity_t *ent;
  gclient_t *client;
  gentity_t *tent;
  int     flags;

  ent = g_entities + clientNum;

  //TA: rip bots
  /*if( ent->botDelayBegin ) {
    G_QueueBotBegin( clientNum );
    ent->botDelayBegin = qfalse;
    return;
  }*/

  client = level.clients + clientNum;

  if ( ent->r.linked ) {
    trap_UnlinkEntity( ent );
  }
  G_InitGentity( ent );
  ent->touch = 0;
  ent->pain = 0;
  ent->client = client;

  client->pers.connected = CON_CONNECTED;
  client->pers.enterTime = level.time;
  client->pers.teamState.state = TEAM_BEGIN;

  // save eflags around this, because changing teams will
  // cause this to happen with a valid entity, and we
  // want to make sure the teleport bit is set right
  // so the viewpoint doesn't interpolate through the
  // world to the new position
  flags = client->ps.eFlags;
  memset( &client->ps, 0, sizeof( client->ps ) );
  client->ps.eFlags = flags;

  // locate ent at a spawn point

  ClientSpawn( ent, NULL );

  if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
    // send event
    tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
    tent->s.clientNum = ent->s.clientNum;

    if ( g_gametype.integer != GT_TOURNAMENT ) {
      trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " entered the game\n\"", client->pers.netname) );
    }
  }
  G_LogPrintf( "ClientBegin: %i\n", clientNum );

  // count current clients and rank for scoreboard
  CalculateRanks();
}

/*
===========
ClientSpawn

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState
============
*/
void ClientSpawn( gentity_t *ent, gentity_t *spawn ) {
  int                 index;
  vec3_t              spawn_origin, spawn_angles;
  gclient_t           *client;
  int                 i;
  clientPersistant_t  saved;
  clientSession_t     savedSess;
  int                 persistant[MAX_PERSISTANT];
  gentity_t           *spawnPoint;
  int                 flags;
  int                 savedPing;
  int                 ammoIndex, ammoSubIndex;
  int                 teamLocal;
  int                 accuracy_hits, accuracy_shots;
  int                 savedEvents[MAX_PS_EVENTS];
  int                 eventSequence;
  char                userinfo[MAX_INFO_STRING];
  vec3_t              bodyMaxs, classMins, up = { 0, 0, 1 };
      

  index = ent - g_entities;
  client = ent->client;

  teamLocal = client->pers.pteam;

  //TA: only start client if chosen a class and joined a team
  if( client->pers.pclass == 0 && teamLocal == 0 )
  {
    client->sess.sessionTeam = TEAM_SPECTATOR;
    client->sess.spectatorState = SPECTATOR_FREE;
  }
  else if( client->pers.pclass == 0 )
  {
    client->sess.sessionTeam = TEAM_SPECTATOR;
    client->sess.spectatorState = SPECTATOR_LOCKED;
  }

  // find a spawn point
  // do it before setting health back up, so farthest
  // ranging doesn't count this client
  if ( client->sess.sessionTeam == TEAM_SPECTATOR )
  {
    if( teamLocal == PTE_NONE )
      spawnPoint = SelectSpectatorSpawnPoint ( spawn_origin, spawn_angles);
    else if( teamLocal == PTE_DROIDS )
      spawnPoint = SelectDroidLockSpawnPoint ( spawn_origin, spawn_angles);
    else if( teamLocal == PTE_HUMANS )
      spawnPoint = SelectHumanLockSpawnPoint ( spawn_origin, spawn_angles);
  }
  else
  {
    //this is an infest spawn
    if( spawn != NULL )
    {
      //spawn as new droid
      VectorCopy( spawn->s.pos.trBase, spawn_origin );
      VectorCopy( spawn->s.angles, spawn_angles );
      
      BG_FindBBoxForClass( spawn->s.clientNum, NULL, NULL, NULL, NULL, bodyMaxs );
      BG_FindBBoxForClass( ent->client->pers.pclass, classMins, NULL, NULL, NULL, NULL );

      spawn_origin[ 2 ] += bodyMaxs[ 2 ] + abs( classMins[ 2 ] ) + 1;
      G_AddEvent( spawn, EV_GIB_DROID, DirToByte( up ) );
      spawn->freeAfterEvent = qtrue;
    }
    else
    {
      // don't spawn near existing origin if possible
      spawnPoint = SelectTremulousSpawnPoint( teamLocal, spawn_origin, spawn_angles );

      if( spawnPoint == NULL )
      {
        trap_SendServerCommand( ent-g_entities, va("print \"No suitable spawns available\n\"" ) );
        return;
      }
    }
  }
  client->pers.teamState.state = TEAM_ACTIVE;

  // toggle the teleport bit so the client knows to not lerp
  flags = ent->client->ps.eFlags & ( EF_TELEPORT_BIT | EF_VOTED | EF_TEAMVOTED );
  flags ^= EF_TELEPORT_BIT;

  // clear everything but the persistant data

  saved = client->pers;
  savedSess = client->sess;
  savedPing = client->ps.ping;
  accuracy_hits = client->accuracy_hits;
  accuracy_shots = client->accuracy_shots;
  for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
    persistant[i] = client->ps.persistant[i];
  }
  // also save the predictable events otherwise we might get double or dropped events
  for (i = 0; i < MAX_PS_EVENTS; i++) {
    savedEvents[i] = client->ps.events[i];
  }
  eventSequence = client->ps.eventSequence;
  memset (client, 0, sizeof(*client));
  
  client->pers = saved;
  client->sess = savedSess;
  client->ps.ping = savedPing;
  client->accuracy_hits = accuracy_hits;
  client->accuracy_shots = accuracy_shots;
  client->lastkilled_client = -1;
  for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
    client->ps.persistant[i] = persistant[i];
  }
  for (i = 0; i < MAX_PS_EVENTS; i++) {
    client->ps.events[i] = savedEvents[i];
  }
  client->ps.eventSequence = eventSequence;

  if( client->sess.sessionTeam == TEAM_SPECTATOR )
  {
    if( teamLocal == PTE_DROIDS )
      G_AddPredictableEvent( ent, EV_MENU, MN_D_CLASS );
    else if( teamLocal == PTE_HUMANS )
      G_AddPredictableEvent( ent, EV_MENU, MN_H_SPAWN );
  }

  // increment the spawncount so the client will detect the respawn
  client->ps.persistant[PERS_SPAWN_COUNT]++;
  client->ps.persistant[PERS_TEAM] = client->sess.sessionTeam;

  client->airOutTime = level.time + 12000;

  trap_GetUserinfo( index, userinfo, sizeof(userinfo) );
  client->ps.eFlags = flags;

  //Com_Printf( "ent->client->pers->pclass = %i\n", ent->client->pers.pclass );

  ent->s.groundEntityNum = ENTITYNUM_NONE;
  ent->client = &level.clients[index];
  ent->takedamage = qtrue;
  ent->inuse = qtrue;
  ent->classname = "player";
  ent->r.contents = CONTENTS_BODY;
  ent->clipmask = MASK_PLAYERSOLID;
  ent->die = player_die;
  ent->waterlevel = 0;
  ent->watertype = 0;
  ent->flags = 0;

  client->ps.stats[ STAT_WEAPONS ] = 0;
  client->ps.stats[ STAT_WEAPONS2 ] = 0;
  client->ps.stats[ STAT_SLOTS ] = 0;

  client->ps.eFlags = flags;
  client->ps.clientNum = index;
  
  BG_FindBBoxForClass( ent->client->pers.pclass, ent->r.mins, ent->r.maxs, NULL, NULL, NULL );

  client->pers.maxHealth = client->ps.stats[ STAT_MAX_HEALTH ] = BG_FindHealthForClass( ent->client->pers.pclass );
  client->ps.stats[ STAT_ARMOR ] = BG_FindArmorForClass( ent->client->pers.pclass );
  client->classSpeed = BG_FindSpeedForClass( ent->client->pers.pclass );

  // clear entity values
  switch( ent->client->pers.pclass )
  {
    case PCL_D_B_BASE:
      BG_packWeapon( WP_ABUILD, client->ps.stats );
      BG_packAmmoArray( WP_ABUILD, client->ps.ammo, client->ps.powerups, 0, 0, 0 );
      break;

    case PCL_D_O_BASE:
      BG_packWeapon( WP_VENOM, client->ps.stats );
      BG_packAmmoArray( WP_VENOM, client->ps.ammo, client->ps.powerups, 0, 0, 0 );
      break;

    case PCL_D_D_BASE:
      BG_packWeapon( WP_VENOM, client->ps.stats );
      BG_packAmmoArray( WP_VENOM, client->ps.ammo, client->ps.powerups, 0, 0, 0 );
      break;
      
    case PCL_H_BASE:
      if( client->pers.pitem == WP_MACHINEGUN )
      {
        BG_packWeapon( WP_MACHINEGUN, client->ps.stats );
        BG_packAmmoArray( WP_MACHINEGUN, client->ps.ammo, client->ps.powerups, CS_MG, 4, 4 );
      }
      else if( client->pers.pitem == WP_HBUILD )
      {
        BG_packWeapon( WP_HBUILD, client->ps.stats );
        BG_packAmmoArray( WP_HBUILD, client->ps.ammo, client->ps.powerups, 0, 0, 0 );
      }
      break;
      
    //eventually remove this case (or report an error) when all classes implemented
    default:
      BG_packWeapon( WP_MACHINEGUN, client->ps.stats );
      BG_packAmmoArray( WP_MACHINEGUN, client->ps.ammo, client->ps.powerups, 100, 0, 0 );
  }

  ent->client->ps.stats[ STAT_PCLASS ] = ent->client->pers.pclass;
  ent->client->ps.stats[ STAT_PTEAM ] = ent->client->pers.pteam;

  // health will count down towards max_health
  ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH]; //* 1.25;

  G_SetOrigin( ent, spawn_origin );
  VectorCopy( spawn_origin, client->ps.origin );

  // the respawned flag will be cleared after the attack and jump keys come up
  client->ps.pm_flags |= PMF_RESPAWNED;

  trap_GetUsercmd( client - level.clients, &ent->client->pers.cmd );
  SetClientViewAngle( ent, spawn_angles );

  if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {

  } else {
    G_KillBox( ent );
    trap_LinkEntity (ent);

    // force the base weapon up
    client->ps.weapon = WP_NONE;
    client->ps.weaponstate = WEAPON_READY;

  }

  // don't allow full run speed for a bit
  client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
  client->ps.pm_time = 100;

  //TA: STAT_SPAWNTIME for droid fov effects
  client->respawnTime = level.time;
  G_AddPredictableEvent( ent, EV_PLAYER_RESPAWN, 0 );

  client->inactivityTime = level.time + g_inactivity.integer * 1000;
  client->latched_buttons = 0;

  // set default animations
  client->ps.torsoAnim = TORSO_STAND;
  client->ps.legsAnim = LEGS_IDLE;

  if ( level.intermissiontime ) {
    MoveClientToIntermission( ent );
  } else {
    // fire the targets of the spawn point
    G_UseTargets( spawnPoint, ent );

    // select the highest weapon number available, after any
    // spawn given items have fired
    client->ps.weapon = 1;
    for ( i = WP_NUM_WEAPONS - 1 ; i > 0 ; i-- ) {
      if ( BG_gotWeapon( i, client->ps.stats ) ) {
        client->ps.weapon = i;
        break;
      }
    }
  }

  // run a client frame to drop exactly to the floor,
  // initialize animations and other things
  client->ps.commandTime = level.time - 100;
  ent->client->pers.cmd.serverTime = level.time;
  ClientThink( ent-g_entities );

  // positively link the client, even if the command times are weird
  if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
    BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
    VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );
    trap_LinkEntity( ent );
  }

  //TA: must do this here so the number of active clients is calculated
  CalculateRanks();

  // run the presend to set anything else
  ClientEndFrame( ent );

  // clear entity state values
  BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
}


/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.

This should NOT be called directly by any game logic,
call trap_DropClient(), which will call this and do
server system housekeeping.
============
*/
void ClientDisconnect( int clientNum ) {
  gentity_t *ent;
  gentity_t *tent;
  int     i;

  ent = g_entities + clientNum;
  if ( !ent->client ) {
    return;
  }

  // stop any following clients
  for ( i = 0 ; i < level.maxclients ; i++ ) {
    if ( level.clients[i].sess.sessionTeam == TEAM_SPECTATOR
      && level.clients[i].sess.spectatorState == SPECTATOR_FOLLOW
      && level.clients[i].sess.spectatorClient == clientNum ) {
      StopFollowing( &g_entities[i] );
    }
  }

  // send effect if they were completely connected
  if ( ent->client->pers.connected == CON_CONNECTED
    && ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
    tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
    tent->s.clientNum = ent->s.clientNum;

    // They don't get to take powerups with them!
    // Especially important for stuff like CTF flags
    TossClientItems ( ent );
  }

  G_LogPrintf( "ClientDisconnect: %i\n", clientNum );

  // if we are playing in tourney mode and losing, give a win to the other player
  if ( ( g_gametype.integer == GT_TOURNAMENT )
    && !level.intermissiontime
    && !level.warmupTime && level.sortedClients[1] == clientNum ) {
    level.clients[ level.sortedClients[0] ].sess.wins++;
    ClientUserinfoChanged( level.sortedClients[0] );
  }

  trap_UnlinkEntity (ent);
  ent->s.modelindex = 0;
  ent->inuse = qfalse;
  ent->classname = "disconnected";
  ent->client->pers.connected = CON_DISCONNECTED;
  ent->client->ps.persistant[PERS_TEAM] = TEAM_FREE;
  ent->client->sess.sessionTeam = TEAM_FREE;

  trap_SetConfigstring( CS_PLAYERS + clientNum, "");

  CalculateRanks();

  //TA: rip bots
  /*if ( ent->r.svFlags & SVF_BOT ) {
    BotAIShutdownClient( clientNum );
  }*/
}
