/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2013 Darklegion Development
Copyright (C) 2015-2018 GrangerHub

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

#ifndef _SND_PUBLIC_H_
#define _SND_PUBLIC_H_

#include "qcommon/q_shared.h"

void S_Init( void );
void S_Shutdown( void );

// cinematics and voice-over-network will send raw samples
// 1.0 volume will be direct output of source samples
void S_RawSamples(int stream, int samples, int rate, int width, int channels,
				   const byte *data, float volume, int entityNum);

// stop all sounds and the background track
void S_StopAllSounds( void );
void S_Update( void );
void S_DisableSounds( void );
void S_BeginRegistration( void );
void S_DisplayFreeMemory(void);
void S_ClearSoundBuffer( void );
void SNDDMA_Activate( void );
void S_UpdateBackgroundTrack( void );

#ifdef USE_VOIP
void S_StartCapture( void );
int S_AvailableCaptureSamples( void );
void S_Capture( int samples, byte *data );
void S_StopCapture( void );
void S_MasterGain( float gain );
#endif

// if origin is NULL, the sound will be dynamically sourced from the entity
SO_PUBLIC void S_StartSound( vec3_t origin, int entnum, int entchannel, sfxHandle_t sfx );
SO_PUBLIC void S_StartLocalSound( sfxHandle_t sfx, int channelNum );
SO_PUBLIC void S_StartBackgroundTrack( const char *intro, const char *loop );
SO_PUBLIC void S_StopBackgroundTrack( void );

// all continuous looping sounds must be added before calling S_Update
SO_PUBLIC void S_ClearLoopingSounds( bool killall );
SO_PUBLIC void S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
SO_PUBLIC void S_AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
SO_PUBLIC void S_StopLoopingSound(int entityNum );

// recompute the relative volumes for all running sounds relative to the given
// entityNum / orientation
SO_PUBLIC void S_Respatialize( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater );

// let the sound system know where an entity currently is
SO_PUBLIC void S_UpdateEntityPosition( int entityNum, const vec3_t origin );

// RegisterSound will allways return a valid sample, even if it has to create a
// placeholder.  This prevents continuous filesystem checks for missing files
SO_PUBLIC sfxHandle_t S_RegisterSound( const char *sample, bool compressed );

SO_PUBLIC int S_SoundDuration( sfxHandle_t handle );

#endif
