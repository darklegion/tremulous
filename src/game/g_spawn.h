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

#ifndef G_SPAWN_H
#define G_SPAWN_H

#include "qcommon/q_shared.h"

struct gentity_t;

bool G_SpawnString( const char *key, const char *defaultString, char **out );
bool G_SpawnFloat( const char *key, const char *defaultString, float *out );
bool G_SpawnInt( const char *key, const char *defaultString, int *out );
bool G_SpawnVector( const char *key, const char *defaultString, float *out );
bool G_SpawnVector4( const char *key, const char *defaultString, float *out );

bool G_CallSpawn( gentity_t *ent );
char *G_NewString( const char *string );
void G_ParseField( const char *key, const char *value, gentity_t *ent );
void G_SpawnGEntityFromSpawnVars( void );
char *G_AddSpawnVarToken( const char *string );
bool G_ParseSpawnVars( void );
void SP_worldspawn( void );
void G_SpawnEntitiesFromString( void );

// g_client.c
void SP_info_player_start( gentity_t *ent );
void SP_info_player_deathmatch( gentity_t *ent );
void SP_info_player_intermission( gentity_t *ent );

void SP_info_alien_intermission( gentity_t *ent );
void SP_info_human_intermission( gentity_t *ent );

// g_mover.c
void SP_func_plat( gentity_t *ent );
void SP_func_static( gentity_t *ent );
void SP_func_rotating( gentity_t *ent );
void SP_func_bobbing( gentity_t *ent );
void SP_func_pendulum( gentity_t *ent );
void SP_func_button( gentity_t *ent );
void SP_func_door( gentity_t *ent );
void SP_func_door_rotating( gentity_t *ent );
void SP_func_door_model( gentity_t *ent );
void SP_func_train( gentity_t *ent );
void SP_func_timer( gentity_t *self);

// g_trigger.c
void SP_trigger_always( gentity_t *ent );
void SP_trigger_multiple( gentity_t *ent );
void SP_trigger_push( gentity_t *ent );
void SP_trigger_teleport( gentity_t *ent );
void SP_trigger_hurt( gentity_t *ent );
void SP_trigger_stage( gentity_t *ent );
void SP_trigger_win( gentity_t *ent );
void SP_trigger_buildable( gentity_t *ent );
void SP_trigger_class( gentity_t *ent );
void SP_trigger_equipment( gentity_t *ent );
void SP_trigger_gravity( gentity_t *ent );
void SP_trigger_heal( gentity_t *ent );
void SP_trigger_ammo( gentity_t *ent );

// g_target.c
void SP_target_delay( gentity_t *ent );
void SP_target_speaker( gentity_t *ent );
void SP_target_print( gentity_t *ent );
void SP_target_score( gentity_t *ent );
void SP_target_teleporter( gentity_t *ent );
void SP_target_relay( gentity_t *ent );
void SP_target_kill( gentity_t *ent );
void SP_target_position( gentity_t *ent );
void SP_target_location( gentity_t *ent );
void SP_target_push( gentity_t *ent );
void SP_target_rumble( gentity_t *ent );
void SP_target_alien_win( gentity_t *ent );
void SP_target_human_win( gentity_t *ent );
void SP_target_hurt( gentity_t *ent );

// g_misc.c
void SP_light( gentity_t *self );
void SP_info_null( gentity_t *self );
void SP_info_notnull( gentity_t *self );
void SP_path_corner( gentity_t *self );

void SP_misc_teleporter_dest( gentity_t *self );
void SP_misc_model( gentity_t *ent );
void SP_misc_portal_camera( gentity_t *ent );
void SP_misc_portal_surface( gentity_t *ent );

void SP_misc_particle_system( gentity_t *ent );
void SP_misc_anim_model( gentity_t *ent );
void SP_misc_light_flare( gentity_t *ent );

#endif

