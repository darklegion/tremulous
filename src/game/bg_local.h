// Copyright (C) 1999-2000 Id Software, Inc.
//
// bg_local.h -- local definitions for the bg (both games) files

/*
 *  Portions Copyright (C) 2000-2001 Tim Angus
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*  To assertain which portions are licensed under the GPL and which are
 *  licensed by Id Software, Inc. please run a diff between the equivalent
 *  versions of the "Tremulous" modification and the unmodified "Quake3"
 *  game source code.
 */

#define MIN_WALK_NORMAL 0.7f   // can't walk on very steep slopes

#define STEPSIZE    18

#define JUMP_VELOCITY 270

#define TIMER_LAND    130
#define TIMER_GESTURE (34*66+50)

#define OVERCLIP    1.001f

// all of the locals will be zeroed before each
// pmove, just to make damn sure we don't have
// any differences when running on client or server
typedef struct {
  vec3_t    forward, right, up;
  float   frametime;

  int     msec;

  qboolean  walking;
  qboolean  groundPlane;
  trace_t   groundTrace;

  float   impactSpeed;

  vec3_t    previous_origin;
  vec3_t    previous_velocity;
  int     previous_waterlevel;
} pml_t;

//TA: struct that stores smooth ops
//    there is an array of these in bg_pmove.c
typedef struct
{
  float     time;
  
  vec3_t    rotAxis;
  float     rotAngle;
} smooth_t;

//TA: make this into a cvar later....
#define   SMOOTHTIME    300
#define   MAXSMOOTHS    10

//TA: wall climbing local
typedef struct
{
  smooth_t  sList[ MAXSMOOTHS ];

  qboolean  justFallen;
  vec3_t    lastNormal;
  vec3_t    nonSvangles;
} wcl_t;

extern  pmove_t       *pm;
extern  pml_t         pml;

// movement parameters
extern  float pm_stopspeed;
extern  float pm_duckScale;
extern  float pm_swimScale;
extern  float pm_wadeScale;

extern  float pm_accelerate;
extern  float pm_airaccelerate;
extern  float pm_wateraccelerate;
extern  float pm_flyaccelerate;

extern  float pm_friction;
extern  float pm_waterfriction;
extern  float pm_flightfriction;

extern  int   c_pmove;

void PM_ClipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce );
void PM_AddTouchEnt( int entityNum );
void PM_AddEvent( int newEvent );

qboolean  PM_SlideMove( qboolean gravity );
void    PM_StepSlideMove( qboolean gravity );

