// Copyright (C) 1999-2000 Id Software, Inc.
//
// bg_misc.c -- both games misc functions, all completely stateless

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
                  
#include "q_shared.h"
#include "bg_public.h"

buildableAttributes_t bg_buildableList[ ] =
{
  {
    BA_A_SPAWN,            //int       buildNum;     
    "bioegg",              //char      *buildName;
    "Egg",                 //char      *humanName;
    "team_alien_spawn",    //char      *entityName;
    { "models/buildables/bioegg/bioegg.md3", 0, 0, 0 },
    { -15, -15, -15 },     //vec3_t    mins;
    { 15, 15, 15 },        //vec3_t    maxs;
    TR_GRAVITY,            //trType_t  traj;
    0.0,                   //float     bounce;
    ASPAWN_BP,             //int       buildPoints;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    ASPAWN_HEALTH,         //int       health;
    ASPAWN_REGEN,          //int       regenRate;
    ASPAWN_SPLASHDAMAGE,   //int       splashDamage;
    ASPAWN_SPLASHRADIUS,   //int       splashRadius;
    MOD_ASPAWN,            //int       meansOfDeath;
    BIT_ALIENS,            //int       team;
    ( 1 << WP_ABUILD )|( 1 << WP_ABUILD2 ),    //weapon_t  buildWeapon;
    BANIM_IDLE1,           //int       idleAnim;
    100,                   //int       nextthink;
    qfalse,                //qboolean  usable;
    0,                     //int       turretRange;
    0,                     //int       turretFireSpeed;
    WP_NONE,               //weapon_t  turretProjType;
    0.5f,                  //float     minNormal;
    qtrue,                 //qboolean  invertNormal;
    qfalse,                //qboolean  creepTest;
    ASPAWN_CREEPSIZE,      //int       creepSize;
    qfalse,                //qboolean  dccTest;
    qfalse                 //qboolean  reactorTest;
  },
  {
    BA_A_BARRICADE,        //int       buildNum;
    "barricade",           //char      *buildName;
    "Barricade",           //char      *humanName;
    "team_alien_barricade",//char      *entityName;
    { "models/buildables/barricade/barricade.md3", 0, 0, 0 },
    { -35, -35, -15 },     //vec3_t    mins;
    { 35, 35, 60 },        //vec3_t    maxs;
    TR_GRAVITY,            //trType_t  traj;
    0.0,                   //float     bounce;
    BARRICADE_BP,          //int       buildPoints;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    BARRICADE_HEALTH,      //int       health;
    BARRICADE_REGEN,       //int       regenRate;
    BARRICADE_SPLASHDAMAGE,//int       splashDamage;
    BARRICADE_SPLASHRADIUS,//int       splashRadius;
    MOD_ASPAWN,            //int       meansOfDeath;
    BIT_ALIENS,            //int       team;
    ( 1 << WP_ABUILD )|( 1 << WP_ABUILD2 ),    //weapon_t  buildWeapon;
    BANIM_IDLE1,           //int       idleAnim;
    100,                   //int       nextthink;
    qfalse,                //qboolean  usable;
    0,                     //int       turretRange;
    0,                     //int       turretFireSpeed;
    WP_NONE,               //weapon_t  turretProjType;
    0.707f,                //float     minNormal;
    qfalse,                //qboolean  invertNormal;
    qtrue,                 //qboolean  creepTest;
    BARRICADE_CREEPSIZE,   //int       creepSize;
    qfalse,                //qboolean  dccTest;
    qfalse                 //qboolean  reactorTest;
  },
  {
    BA_A_BOOSTER,          //int       buildNum;
    "booster",             //char      *buildName;
    "Booster",             //char      *humanName;
    "team_alien_booster",  //char      *entityName;
    { "models/buildables/booster/booster.md3", 0, 0, 0 },
    { -26, -26, -9 },     //vec3_t     mins;
    { 26, 26, 9 },        //vec3_t     maxs;
    TR_GRAVITY,            //trType_t  traj;
    0.0,                   //float     bounce;
    BOOSTER_BP,            //int       buildPoints;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    BOOSTER_HEALTH,        //int       health;
    BOOSTER_REGEN,         //int       regenRate;
    BOOSTER_SPLASHDAMAGE,  //int       splashDamage;
    BOOSTER_SPLASHRADIUS,  //int       splashRadius;
    MOD_ASPAWN,            //int       meansOfDeath;
    BIT_ALIENS,            //int       team;
    ( 1 << WP_ABUILD )|( 1 << WP_ABUILD2 ),    //weapon_t  buildWeapon;
    BANIM_IDLE1,           //int       idleAnim;
    100,                   //int       nextthink;
    qfalse,                //qboolean  usable;
    0,                     //int       turretRange;
    0,                     //int       turretFireSpeed;
    WP_NONE,               //weapon_t  turretProjType;
    0.707f,                //float     minNormal;
    qfalse,                //qboolean  invertNormal;
    qtrue,                 //qboolean  creepTest;
    BOOSTER_CREEPSIZE,     //int       creepSize;
    qfalse,                //qboolean  dccTest;
    qfalse                 //qboolean  reactorTest;
  },
  {
    BA_A_ACIDTUBE,         //int       buildNum;
    "acid_tube",           //char      *buildName;
    "Acid Tube",           //char      *humanName;
    "team_alien_acid_tube",//char      *entityName;
    { "models/buildables/acid_tube/acid_tube.md3", 0, 0, 0 },
    { -35, -35, -11 },     //vec3_t    mins;
    { 35, 35, 40 },        //vec3_t    maxs;
    TR_GRAVITY,            //trType_t  traj;
    0.0,                   //float     bounce;
    ACIDTUBE_BP,           //int       buildPoints;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    ACIDTUBE_HEALTH,       //int       health;
    ACIDTUBE_REGEN,        //int       regenRate;
    ACIDTUBE_SPLASHDAMAGE, //int       splashDamage;
    ACIDTUBE_SPLASHRADIUS, //int       splashRadius;
    MOD_ASPAWN,            //int       meansOfDeath;
    BIT_ALIENS,            //int       team;
    ( 1 << WP_ABUILD )|( 1 << WP_ABUILD2 ),    //weapon_t  buildWeapon;
    BANIM_IDLE1,           //int       idleAnim;
    500,                   //int       nextthink;
    qfalse,                //qboolean  usable;
    0,                     //int       turretRange;
    0,                     //int       turretFireSpeed;
    WP_NONE,               //weapon_t  turretProjType;
    0.707f,                //float     minNormal;
    qtrue,                 //qboolean  invertNormal;
    qtrue,                 //qboolean  creepTest;
    ACIDTUBE_CREEPSIZE,    //int       creepSize;
    qfalse,                //qboolean  dccTest;
    qfalse                 //qboolean  reactorTest;
  },
  {
    BA_A_TRAPPER,          //int       buildNum;
    "trapper",             //char      *buildName;
    "Trapper",             //char      *humanName;
    "team_alien_trapper",  //char      *entityName;
    { "models/buildables/trapper/trapper.md3", 0, 0, 0 },
    { -15, -15, -15 },     //vec3_t    mins;
    { 15, 15, 15 },        //vec3_t    maxs;
    TR_GRAVITY,            //trType_t  traj;
    0.0,                   //float     bounce;
    TRAPPER_BP,            //int       buildPoints;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    TRAPPER_HEALTH,        //int       health;
    TRAPPER_REGEN,         //int       regenRate;
    TRAPPER_SPLASHDAMAGE,  //int       splashDamage;
    TRAPPER_SPLASHRADIUS,  //int       splashRadius;
    MOD_ASPAWN,            //int       meansOfDeath;
    BIT_ALIENS,            //int       team;
    ( 1 << WP_ABUILD )|( 1 << WP_ABUILD2 ),    //weapon_t  buildWeapon;
    BANIM_IDLE1,           //int       idleAnim;
    100,                   //int       nextthink;
    qfalse,                //qboolean  usable;
    TRAPPER_RANGE,         //int       turretRange;
    TRAPPER_REPEAT,        //int       turretFireSpeed;
    WP_LOCKBLOB_LAUNCHER,  //weapon_t  turretProjType;
    0.0f,                  //float     minNormal;
    qtrue,                 //qboolean  invertNormal;
    qtrue,                 //qboolean  creepTest;
    TRAPPER_CREEPSIZE,     //int       creepSize;
    qfalse,                //qboolean  dccTest;
    qfalse                 //qboolean  reactorTest;
  },
  {
    BA_A_HIVEMIND,         //int       buildNum;
    "hivemind",            //char      *buildName;
    "Overmind",            //char      *humanName;
    "team_alien_hivemind", //char      *entityName;
    { "models/buildables/hivemind/hivemind.md3", 0, 0, 0 },
    { -45, -45, -15 },     //vec3_t    mins;
    { 45, 45, 95 },        //vec3_t    maxs;
    TR_GRAVITY,            //trType_t  traj;
    0.0,                   //float     bounce;
    OVERMIND_BP,           //int       buildPoints;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    OVERMIND_HEALTH,       //int       health;
    OVERMIND_REGEN,        //int       regenRate;
    OVERMIND_SPLASHDAMAGE, //int       splashDamage;
    OVERMIND_SPLASHRADIUS, //int       splashRadius;
    MOD_ASPAWN,            //int       meansOfDeath;
    BIT_ALIENS,            //int       team;
    ( 1 << WP_ABUILD )|( 1 << WP_ABUILD2 ),    //weapon_t  buildWeapon;
    BANIM_IDLE1,           //int       idleAnim;
    OVERMIND_ATTACK_REPEAT,//int       nextthink;
    qfalse,                //qboolean  usable;
    0,                     //int       turretRange;
    0,                     //int       turretFireSpeed;
    WP_NONE,               //weapon_t  turretProjType;
    0.707f,                //float     minNormal;
    qfalse,                //qboolean  invertNormal;
    qfalse,                //qboolean  creepTest;
    OVERMIND_CREEPSIZE,    //int       creepSize;
    qfalse,                //qboolean  dccTest;
    qtrue                  //qboolean  reactorTest;
  },
  {
    BA_A_HOVEL,            //int       buildNum;
    "hovel",               //char      *buildName;
    "Hovel",               //char      *humanName;
    "team_alien_hovel",    //char      *entityName;
    { "models/buildables/hovel/hovel.md3", 0, 0, 0 },
    { -50, -50, -20 },     //vec3_t    mins;
    { 50, 50, 20 },        //vec3_t    maxs;
    TR_GRAVITY,            //trType_t  traj;
    0.0,                   //float     bounce;
    HOVEL_BP,              //int       buildPoints;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    HOVEL_HEALTH,          //int       health;
    HOVEL_REGEN,           //int       regenRate;
    HOVEL_SPLASHDAMAGE,    //int       splashDamage;
    HOVEL_SPLASHRADIUS,    //int       splashRadius;
    MOD_ASPAWN,            //int       meansOfDeath;
    BIT_ALIENS,            //int       team;
    ( 1 << WP_ABUILD )|( 1 << WP_ABUILD2 ),    //weapon_t  buildWeapon;
    BANIM_IDLE1,           //int       idleAnim;
    150,                   //int       nextthink;
    qtrue,                 //qboolean  usable;
    0,                     //int       turretRange;
    0,                     //int       turretFireSpeed;
    WP_NONE,               //weapon_t  turretProjType;
    0.707f,                //float     minNormal;
    qtrue,                 //qboolean  invertNormal;
    qtrue,                 //qboolean  creepTest;
    HOVEL_CREEPSIZE,       //int       creepSize;
    qfalse,                //qboolean  dccTest;
    qfalse                 //qboolean  reactorTest;
  },
  {
    BA_H_SPAWN,            //int       buildNum;
    "replicator",          //char      *buildName;
    "Replicator",          //char      *humanName;
    "team_human_spawn",    //char      *entityName;
    { "models/buildables/replicator/replicator.md3", 0, 0, 0 },
    { -40, -40, -4 },      //vec3_t    mins;
    { 40, 40, 4 },         //vec3_t    maxs;
    TR_GRAVITY,            //trType_t  traj;
    0.0,                   //float     bounce;
    HSPAWN_BP,             //int       buildPoints;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    HSPAWN_HEALTH,         //int       health;
    0,                     //int       regenRate;
    HSPAWN_SPLASHDAMAGE,   //int       splashDamage;
    HSPAWN_SPLASHRADIUS,   //int       splashRadius;
    MOD_HSPAWN,            //int       meansOfDeath;
    BIT_HUMANS,            //int       team;
    ( 1 << WP_HBUILD )|( 1 << WP_HBUILD2 ),    //weapon_t  buildWeapon;
    BANIM_IDLE1,           //int       idleAnim;
    100,                   //int       nextthink;
    qfalse,                //qboolean  usable;
    0,                     //int       turretRange;
    0,                     //int       turretFireSpeed;
    WP_NONE,               //weapon_t  turretProjType;
    0.907f,                //float     minNormal;
    qfalse,                //qboolean  invertNormal;
    qfalse,                //qboolean  creepTest;
    0,                     //int       creepSize;
    qfalse,                //qboolean  dccTest;
    qfalse                 //qboolean  reactorTest;
  },
  {
    BA_H_MEDISTAT,         //int       buildNum;
    "medistat",            //char      *buildName;
    "Medistation",         //char      *humanName;
    "team_human_medistat", //char      *entityName;
    { "models/buildables/medistat/medistat.md3", 0, 0, 0 },
    { -35, -35, -7 },      //vec3_t    mins;
    { 35, 35, 7 },         //vec3_t    maxs;
    TR_GRAVITY,            //trType_t  traj;
    0.0,                   //float     bounce;
    MEDISTAT_BP,           //int       buildPoints;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    MEDISTAT_HEALTH,       //int       health;
    0,                     //int       regenRate;
    MEDISTAT_SPLASHDAMAGE, //int       splashDamage;
    MEDISTAT_SPLASHRADIUS, //int       splashRadius;
    MOD_HSPAWN,            //int       meansOfDeath;
    BIT_HUMANS,            //int       team;
    ( 1 << WP_HBUILD )|( 1 << WP_HBUILD2 ),    //weapon_t  buildWeapon;
    BANIM_IDLE1,           //int       idleAnim;
    100,                   //int       nextthink;
    qfalse,                //qboolean  usable;
    0,                     //int       turretRange;
    0,                     //int       turretFireSpeed;
    WP_NONE,               //weapon_t  turretProjType;
    0.907f,                //float     minNormal;
    qfalse,                //qboolean  invertNormal;
    qfalse,                //qboolean  creepTest;
    0,                     //int       creepSize;
    qfalse,                //qboolean  dccTest;
    qfalse                 //qboolean  reactorTest;
  },
  {
    BA_H_MGTURRET,         //int       buildNum;
    "mgturret",            //char      *buildName;
    "Machinegun Turret",   //char      *humanName;
    "team_human_def2",     //char      *entityName;
    { "models/buildables/mgturret/turret_base.md3",
      "models/buildables/mgturret/turret_barrel.md3",
      "models/buildables/mgturret/turret_top.md3", 0 },
    { -25, -25, -20 },     //vec3_t    mins;
    { 25, 25, 20 },        //vec3_t    maxs;
    TR_GRAVITY,            //trType_t  traj;
    0.0,                   //float     bounce;
    MGTURRET_BP,           //int       buildPoints;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    MGTURRET_HEALTH,       //int       health;
    0,                     //int       regenRate;
    MGTURRET_SPLASHDAMAGE, //int       splashDamage;
    MGTURRET_SPLASHRADIUS, //int       splashRadius;
    MOD_HSPAWN,            //int       meansOfDeath;
    BIT_HUMANS,            //int       team;
    ( 1 << WP_HBUILD )|( 1 << WP_HBUILD2 ),   //weapon_t  buildWeapon;
    BANIM_IDLE1,           //int       idleAnim;
    50,                    //int       nextthink;
    qfalse,                //qboolean  usable;
    MGTURRET_RANGE,        //int       turretRange;
    MGTURRET_REPEAT,       //int       turretFireSpeed;
    WP_MACHINEGUN,         //weapon_t  turretProjType;
    0.907f,                //float     minNormal;
    qfalse,                //qboolean  invertNormal;
    qfalse,                //qboolean  creepTest;
    0,                     //int       creepSize;
    qfalse,                //qboolean  dccTest;
    qfalse                 //qboolean  reactorTest;
  },
  {
    BA_H_TESLAGEN,         //int       buildNum;
    "tesla",               //char      *buildName;
    "Tesla Generator",     //char      *humanName;
    "team_human_tesla",    //char      *entityName;
    { "models/buildables/tesla/tesla.md3", 0, 0, 0 },
    { -22, -22, -40 },     //vec3_t    mins;
    { 22, 22, 40 },        //vec3_t    maxs;
    TR_GRAVITY,            //trType_t  traj;
    0.0,                   //float     bounce;
    TESLAGEN_BP,           //int       buildPoints;
    ( 1 << S3 ),           //int       stages
    TESLAGEN_HEALTH,       //int       health;
    0,                     //int       regenRate;
    TESLAGEN_SPLASHDAMAGE, //int       splashDamage;
    TESLAGEN_SPLASHRADIUS, //int       splashRadius;
    MOD_HSPAWN,            //int       meansOfDeath;
    BIT_HUMANS,            //int       team;
    ( 1 << WP_HBUILD2 ),   //weapon_t  buildWeapon;
    BANIM_IDLE1,           //int       idleAnim;
    150,                   //int       nextthink;
    qfalse,                //qboolean  usable;
    TESLAGEN_RANGE,        //int       turretRange;
    TESLAGEN_REPEAT,       //int       turretFireSpeed;
    WP_TESLAGEN,           //weapon_t  turretProjType;
    0.907f,                //float     minNormal;
    qfalse,                //qboolean  invertNormal;
    qfalse,                //qboolean  creepTest;
    0,                     //int       creepSize;
    qtrue,                 //qboolean  dccTest;
    qfalse                 //qboolean  reactorTest;
  },
  {
    BA_H_DCC,              //int       buildNum;
    "dcc",                 //char      *buildName;
    "Defence Computer",    //char      *humanName;
    "team_human_dcc",      //char      *entityName;
    { "models/buildables/dcc/dcc.md3", 0, 0, 0 },
    { -35, -35, -13 },     //vec3_t    mins;
    { 35, 35, 47 },        //vec3_t    maxs;
    TR_GRAVITY,            //trType_t  traj;
    0.0,                   //float     bounce;
    DC_BP,                 //int       buildPoints;
    ( 1 << S3 ),           //int       stages
    DC_HEALTH,             //int       health;
    0,                     //int       regenRate;
    DC_SPLASHDAMAGE,       //int       splashDamage;
    DC_SPLASHRADIUS,       //int       splashRadius;
    MOD_HSPAWN,            //int       meansOfDeath;
    BIT_HUMANS,            //int       team;
    ( 1 << WP_HBUILD2 ),   //weapon_t  buildWeapon;
    BANIM_IDLE1,           //int       idleAnim;
    100,                   //int       nextthink;
    qfalse,                //qboolean  usable;
    0,                     //int       turretRange;
    0,                     //int       turretFireSpeed;
    WP_NONE,               //weapon_t  turretProjType;
    0.907f,                //float     minNormal;
    qfalse,                //qboolean  invertNormal;
    qfalse,                //qboolean  creepTest;
    0,                     //int       creepSize;
    qfalse,                //qboolean  dccTest;
    qfalse                 //qboolean  reactorTest;
  },
  {
    BA_H_BANK,             //int       buildNum;
    "bank",                //char      *buildName;
    "Bank",                //char      *humanName;
    "team_human_bank",     //char      *entityName;
    { "models/buildables/bank/bank.md3", 0, 0, 0 },
    { -25, -25, 0 },       //vec3_t    mins;
    { 25, 25, 45 },        //vec3_t    maxs;
    TR_GRAVITY,            //trType_t  traj;
    0.0,                   //float     bounce;
    BANK_BP,               //int       buildPoints;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    BANK_HEALTH,           //int       health;
    0,                     //int       regenRate;
    BANK_SPLASHDAMAGE,     //int       splashDamage;
    BANK_SPLASHRADIUS,     //int       splashRadius;
    MOD_HSPAWN,            //int       meansOfDeath;
    BIT_HUMANS,            //int       team;
    ( 1 << WP_HBUILD )|( 1 << WP_HBUILD2 ),   //weapon_t  buildWeapon;
    BANIM_IDLE1,           //int       idleAnim;
    100,                   //int       nextthink;
    qtrue,                 //qboolean  usable;
    0,                     //int       turretRange;
    0,                     //int       turretFireSpeed;
    WP_NONE,               //weapon_t  turretProjType;
    0.907f,                //float     minNormal;
    qfalse,                //qboolean  invertNormal;
    qfalse,                //qboolean  creepTest;
    0,                     //int       creepSize;
    qfalse,                //qboolean  dccTest;
    qfalse                 //qboolean  reactorTest;
  },
  {
    BA_H_ARMOURY,          //int       buildNum;
    "mcu",                 //char      *buildName;
    "Armoury",             //char      *humanName;
    "team_human_armoury",  //char      *entityName;
    { "models/buildables/mcu/mcu.md3", 0, 0, 0 },
    { -40, -40, -13 },     //vec3_t    mins;
    { 40, 40, 50 },        //vec3_t    maxs;
    TR_GRAVITY,            //trType_t  traj;
    0.0,                   //float     bounce;
    ARMOURY_BP,            //int       buildPoints;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    ARMOURY_HEALTH,        //int       health;
    0,                     //int       regenRate;
    ARMOURY_SPLASHDAMAGE,  //int       splashDamage;
    ARMOURY_SPLASHRADIUS,  //int       splashRadius;
    MOD_HSPAWN,            //int       meansOfDeath;
    BIT_HUMANS,            //int       team;
    ( 1 << WP_HBUILD )|( 1 << WP_HBUILD2 ),    //weapon_t  buildWeapon;
    BANIM_IDLE1,           //int       idleAnim;
    100,                   //int       nextthink;
    qtrue,                 //qboolean  usable;
    0,                     //int       turretRange;
    0,                     //int       turretFireSpeed;
    WP_NONE,               //weapon_t  turretProjType;
    0.907f,                //float     minNormal;
    qfalse,                //qboolean  invertNormal;
    qfalse,                //qboolean  creepTest;
    0,                     //int       creepSize;
    qfalse,                //qboolean  dccTest;
    qfalse                 //qboolean  reactorTest;
  },
  {
    BA_H_REACTOR,          //int       buildNum;
    "reactor",             //char      *buildName;
    "Reactor",             //char      *humanName;
    "team_human_reactor",  //char      *entityName;
    { "models/buildables/reactor/reactor.md3", 0, 0, 0 },
    { -50, -50, -15 },     //vec3_t    mins;
    { 50, 50, 95 },        //vec3_t    maxs;
    TR_GRAVITY,            //trType_t  traj;
    0.0,                   //float     bounce;
    REACTOR_BP,            //int       buildPoints;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    REACTOR_HEALTH,        //int       health;
    0,                     //int       regenRate;
    REACTOR_SPLASHDAMAGE,  //int       splashDamage;
    REACTOR_SPLASHRADIUS,  //int       splashRadius;
    MOD_HSPAWN,            //int       meansOfDeath;
    BIT_HUMANS,            //int       team;
    ( 1 << WP_HBUILD )|( 1 << WP_HBUILD2 ),    //weapon_t  buildWeapon;
    BANIM_IDLE1,           //int       idleAnim;
    -1,                    //int       nextthink;
    qtrue,                 //qboolean  usable;
    0,                     //int       turretRange;
    0,                     //int       turretFireSpeed;
    WP_NONE,               //weapon_t  turretProjType;
    0.907f,                //float     minNormal;
    qfalse,                //qboolean  invertNormal;
    qfalse,                //qboolean  creepTest;
    0,                     //int       creepSize;
    qfalse,                //qboolean  dccTest;
    qtrue                  //qboolean  reactorTest;
  },
  {
    BA_H_REPEATER,         //int       buildNum;
    "repeater",            //char      *buildName;
    "Repeater",            //char      *humanName;
    "team_human_repeater", //char      *entityName;
    { "models/buildables/repeater/repeater.md3", 0, 0, 0 },
    { -15, -15, -15 },     //vec3_t    mins;
    { 15, 15, 25 },        //vec3_t    maxs;
    TR_GRAVITY,            //trType_t  traj;
    0.0,                   //float     bounce;
    REPEATER_BP,           //int       buildPoints;
    ( 1 << S2 )|( 1 << S3 ), //int  stages
    REPEATER_HEALTH,       //int       health;
    0,                     //int       regenRate;
    REPEATER_SPLASHDAMAGE, //int       splashDamage;
    REPEATER_SPLASHRADIUS, //int       splashRadius;
    MOD_HSPAWN,            //int       meansOfDeath;
    BIT_HUMANS,            //int       team;
    ( 1 << WP_HBUILD )|( 1 << WP_HBUILD2 ),    //weapon_t  buildWeapon;
    BANIM_IDLE1,           //int       idleAnim;
    100,                   //int       nextthink;
    qtrue,                 //qboolean  usable;
    0,                     //int       turretRange;
    0,                     //int       turretFireSpeed;
    WP_NONE,               //weapon_t  turretProjType;
    0.907f,                //float     minNormal;
    qfalse,                //qboolean  invertNormal;
    qfalse,                //qboolean  creepTest;
    0,                     //int       creepSize;
    qfalse,                //qboolean  dccTest;
    qfalse                 //qboolean  reactorTest;
  },
  {
    BA_H_FLOATMINE,        //int       buildNum;
    "floatmine",           //char      *buildName;
    "Floatmine",           //char      *humanName;
    "team_human_floatmine",//char      *entityName;
    { "models/buildables/floatmine/floatmine.md3", 0, 0, 0 },
    { -25, -25, -25 },     //vec3_t    mins;
    { 25, 25, 25 },        //vec3_t    maxs;
    TR_BUOYANCY,           //trType_t  traj;
    0.2,                   //float     bounce;
    FLOATMINE_BP,          //int       buildPoints;
    ( 1 << S2 )|( 1 << S3 ), //int  stages
    FLOATMINE_HEALTH,      //int       health;
    0,                     //int       regenRate;
    FLOATMINE_SPLASHDAMAGE,//int       splashDamage;
    FLOATMINE_SPLASHRADIUS,//int       splashRadius;
    MOD_HSPAWN,            //int       meansOfDeath;
    BIT_HUMANS,            //int       team;
    ( 1 << WP_HBUILD2 ),   //weapon_t  buildWeapon;
    BANIM_IDLE1,           //int       idleAnim;
    100,                   //int       nextthink;
    qfalse,                //qboolean  usable;
    0,                     //int       turretRange;
    0,                     //int       turretFireSpeed;
    WP_NONE,               //weapon_t  turretProjType;
    0.707f,                //float     minNormal;
    qfalse,                //qboolean  invertNormal;
    qfalse,                //qboolean  creepTest;
    0,                     //int       creepSize;
    qfalse,                //qboolean  dccTest;
    qfalse                 //qboolean  reactorTest;
  }
};

int   bg_numBuildables = sizeof( bg_buildableList ) / sizeof( bg_buildableList[ 0 ] );

/*
==============
BG_FindBuildNumForName
==============
*/
int BG_FindBuildNumForName( char *name )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( !Q_stricmp( bg_buildableList[ i ].buildName, name ) )
      return bg_buildableList[ i ].buildNum;
  }

  //wimp out
  return BA_NONE;
}

/*
==============
BG_FindBuildNumForEntityName
==============
*/
int BG_FindBuildNumForEntityName( char *name )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( !Q_stricmp( bg_buildableList[ i ].entityName, name ) )
      return bg_buildableList[ i ].buildNum;
  }

  //wimp out
  return BA_NONE;
}

/*
==============
BG_FindNameForBuildNum
==============
*/
char *BG_FindNameForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
      return bg_buildableList[ i ].buildName;
  }

  //wimp out
  return 0;
}

/*
==============
BG_FindHumanNameForBuildNum
==============
*/
char *BG_FindHumanNameForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
      return bg_buildableList[ i ].humanName;
  }

  //wimp out
  return 0;
}

/*
==============
BG_FindEntityNameForBuildNum
==============
*/
char *BG_FindEntityNameForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
      return bg_buildableList[ i ].entityName;
  }

  //wimp out
  return 0;
}

/*
==============
BG_FindModelsForBuildNum
==============
*/
char *BG_FindModelsForBuildable( int bclass, int modelNum )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
      return bg_buildableList[ i ].models[ modelNum ];
  }

  //wimp out
  return 0;
}

/*
==============
BG_FindBBoxForBuildable
==============
*/
void BG_FindBBoxForBuildable( int bclass, vec3_t mins, vec3_t maxs )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      if( mins != NULL )
        VectorCopy( bg_buildableList[ i ].mins, mins );
        
      if( maxs != NULL )
        VectorCopy( bg_buildableList[ i ].maxs, maxs );
        
      return;
    }
  }
  
  if( mins != NULL )
    VectorCopy( bg_buildableList[ 0 ].mins, mins );
    
  if( maxs != NULL )
    VectorCopy( bg_buildableList[ 0 ].maxs, maxs );
}

/*
==============
BG_FindTrajectoryForBuildable
==============
*/
trType_t BG_FindTrajectoryForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      return bg_buildableList[ i ].traj;
    }
  }
  
  return TR_GRAVITY;
}

/*
==============
BG_FindBounceForBuildable
==============
*/
float BG_FindBounceForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      return bg_buildableList[ i ].bounce;
    }
  }
  
  return 0.0;
}

/*
==============
BG_FindBuildPointsForBuildable
==============
*/
int BG_FindBuildPointsForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      return bg_buildableList[ i ].buildPoints;
    }
  }
  
  return 1000;
}

/*
==============
BG_FindStagesForBuildable
==============
*/
qboolean BG_FindStagesForBuildable( int bclass, stage_t stage )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      if( bg_buildableList[ i ].stages & ( 1 << stage ) )
        return qtrue;
      else
        return qfalse;
    }
  }
  
  return qfalse;
}

/*
==============
BG_FindHealthForBuildable
==============
*/
int BG_FindHealthForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      return bg_buildableList[ i ].health;
    }
  }
  
  return 1000;
}

/*
==============
BG_FindRegenRateForBuildable
==============
*/
int BG_FindRegenRateForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      return bg_buildableList[ i ].regenRate;
    }
  }
  
  return 0;
}

/*
==============
BG_FindSplashDamageForBuildable
==============
*/
int BG_FindSplashDamageForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      return bg_buildableList[ i ].splashDamage;
    }
  }
  
  return 50;
}

/*
==============
BG_FindSplashRadiusForBuildable
==============
*/
int BG_FindSplashRadiusForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      return bg_buildableList[ i ].splashRadius;
    }
  }
  
  return 200;
}

/*
==============
BG_FindMODForBuildable
==============
*/
int BG_FindMODForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      return bg_buildableList[ i ].meansOfDeath;
    }
  }
  
  return MOD_UNKNOWN;
}

/*
==============
BG_FindTeamForBuildable
==============
*/
int BG_FindTeamForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      return bg_buildableList[ i ].team;
    }
  }
  
  return BIT_NONE;
}

/*
==============
BG_FindBuildWeaponForBuildable
==============
*/
weapon_t BG_FindBuildWeaponForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      return bg_buildableList[ i ].buildWeapon;
    }
  }
  
  return BA_NONE;
}

/*
==============
BG_FindAnimForBuildable
==============
*/
int BG_FindAnimForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      return bg_buildableList[ i ].idleAnim;
    }
  }
  
  return BANIM_IDLE1;
}

/*
==============
BG_FindNextThinkForBuildable
==============
*/
int BG_FindNextThinkForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      return bg_buildableList[ i ].nextthink;
    }
  }
  
  return 100;
}

/*
==============
BG_FindUsableForBuildable
==============
*/
qboolean BG_FindUsableForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      return bg_buildableList[ i ].usable;
    }
  }
  
  return qfalse;
}

/*
==============
BG_FindFireSpeedForBuildable
==============
*/
int BG_FindFireSpeedForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      return bg_buildableList[ i ].turretFireSpeed;
    }
  }
  
  return 1000;
}

/*
==============
BG_FindRangeForBuildable
==============
*/
int BG_FindRangeForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      return bg_buildableList[ i ].turretRange;
    }
  }
  
  return 1000;
}

/*
==============
BG_FindProjTypeForBuildable
==============
*/
weapon_t BG_FindProjTypeForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      return bg_buildableList[ i ].turretProjType;
    }
  }
  
  return WP_NONE;
}

/*
==============
BG_FindMinNormalForBuildable
==============
*/
float BG_FindMinNormalForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      return bg_buildableList[ i ].minNormal;
    }
  }
  
  return 0.707f;
}

/*
==============
BG_FindInvertNormalForBuildable
==============
*/
qboolean BG_FindInvertNormalForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      return bg_buildableList[ i ].invertNormal;
    }
  }
  
  return qfalse;
}

/*
==============
BG_FindCreepTestForBuildable
==============
*/
int BG_FindCreepTestForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      return bg_buildableList[ i ].creepTest;
    }
  }
  
  return qfalse;
}

/*
==============
BG_FindCreepSizeForBuildable
==============
*/
int BG_FindCreepSizeForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      return bg_buildableList[ i ].creepSize;
    }
  }
  
  return CREEP_BASESIZE;
}

/*
==============
BG_FindDCCTestForBuildable
==============
*/
int BG_FindDCCTestForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      return bg_buildableList[ i ].dccTest;
    }
  }
  
  return qfalse;
}

/*
==============
BG_FindUniqueTestForBuildable
==============
*/
int BG_FindUniqueTestForBuildable( int bclass )
{
  int i;

  for( i = 0; i < bg_numBuildables; i++ )
  {
    if( bg_buildableList[ i ].buildNum == bclass )
    {
      return bg_buildableList[ i ].reactorTest;
    }
  }
  
  return qfalse;
}

////////////////////////////////////////////////////////////////////////////////

classAttributes_t bg_classList[ ] =
{
  { 
    PCL_A_B_BASE,                                   //int     classnum;
    "builder",                                      //char    *className;
    "Builder",                                      //char    *humanName;
    "lucy",                                         //char    *modelname;
    1.0f,                                           //float   modelScale;
    "default",                                      //char    *skinname;
    "alien_hud",                                    //char    *hudname;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ),            //int  stages
    { -15, -15, -20 },                              //vec3_t  mins;
    { 15, 15, 20 },                                 //vec3_t  maxs;
    { 15, 15, 20 },                                 //vec3_t  crouchmaxs;
    { -15, -15, -4 },                               //vec3_t  deadmins;
    { 15, 15, 4 },                                  //vec3_t  deadmaxs;
    20, 20,                                         //int     viewheight, crouchviewheight;
    ABUILDER_HEALTH,                                //int     health;
    ABUILDER_REGEN,                                 //int     regenRate;
    SCA_TAKESFALLDAMAGE|SCA_FOVWARPS|
      SCA_NOFOOTSTEPS,                              //int     abilities;
    WP_ABUILD,                                      //weapon_t  startWeapon
    95.0f,                                          //float   buildDist;
    80,                                             //int     fov;
    0.001f,                                         //float   bob;
    350,                                            //int     steptime;
    ABUILDER_SPEED,                                 //float   speed;
    1.0f,                                           //float   sticky;
    { PCL_A_B_LEV1, PCL_NONE, PCL_NONE },           //int     children[ 3 ];
    ABUILDER_TTE,                                   //int     timetoevolve;
    ABUILDER_VALUE                                  //int     value;
  },
  { 
    PCL_A_B_LEV1,                                   //int     classnum;
    "builderupg",                                   //char    *classname;
    "Advanced Builder",                             //char    *humanname;
    "lucy",                                         //char    *modelname;
    1.0f,                                           //float   modelScale;
    "angel",                                        //char    *skinname;
    "alien_hud",                                    //char    *hudname;
    ( 1 << S2 )|( 1 << S3 ),                        //int  stages
    { -20, -20, -20 },                              //vec3_t  mins;
    { 20, 20, 20 },                                 //vec3_t  maxs;
    { 20, 20, 20 },                                 //vec3_t  crouchmaxs;
    { -20, -20, -4 },                               //vec3_t  deadmins;
    { 20, 20, 4 },                                  //vec3_t  deadmaxs;
    20, 20,                                         //int     viewheight, crouchviewheight;
    ABUILDER_UPG_HEALTH,                            //int     health;
    ABUILDER_UPG_REGEN,                             //int     regenRate;
    SCA_CANJUMP|SCA_FOVWARPS|SCA_WALLCLIMBER|
      SCA_NOFOOTSTEPS,                              //int     abilities;
    WP_ABUILD2,                                     //weapon_t  startWeapon
    95.0f,                                          //float   buildDist;
    110,                                            //int     fov;
    0.001f,                                         //float   bob;
    200,                                            //int     steptime;
    ABUILDER_UPG_SPEED,                             //float   speed;
    1.0f,                                           //float   sticky;
    { PCL_NONE, PCL_NONE, PCL_NONE },               //int     children[ 3 ];
    0,                                              //int     timetoevolve;
    ABUILDER_UPG_VALUE                              //int     value;
  },
  {
    PCL_A_O_BASE,                                   //int     classnum;
    "soldier",                                      //char    *classname;
    "Soldier",                                      //char    *humanname;
    "jumper",                                       //char    *modelname;
    0.2f,                                           //float   modelScale;
    "default",                                      //char    *skinname;
    "alien_hud",                                    //char    *hudname;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ),            //int  stages
    { -15, -15, -6 },                               //vec3_t  mins;
    { 15, 15, 6 },                                  //vec3_t  maxs;
    { 15, 15, 6 },                                  //vec3_t  crouchmaxs;
    { -15, -15, -4 },                               //vec3_t  deadmins;
    { 15, 15, 4 },                                  //vec3_t  deadmaxs;
    6, 6,                                           //int     viewheight, crouchviewheight;
    SOLDIER_HEALTH,                                 //int     health;
    SOLDIER_REGEN,                                  //int     regenRate;
    SCA_WALLCLIMBER|SCA_CANJUMP|SCA_NOWEAPONDRIFT|
      SCA_FOVWARPS|SCA_ALIENSENSE|SCA_NOFOOTSTEPS,  //int     abilities;
    WP_VENOM,                                       //weapon_t  startWeapon
    0.0f,                                           //float   buildDist;
    140,                                            //int     fov;
    0.0f,                                           //float   bob;
    25,                                             //int     steptime;
    SOLDIER_SPEED,                                  //float   speed;
    5.0f,                                           //float   sticky;
    { PCL_A_O_LEV1, PCL_NONE, PCL_NONE },           //int     children[ 3 ];
    SOLDIER_TTE,                                    //int     timetoevolve;
    SOLDIER_VALUE                                   //int     value;
  },
  {
    PCL_A_O_LEV1,                                   //int     classnum;
    "dragoon",                                      //char    *classname;
    "Dragoon",                                      //char    *humanname;
    "prowl",                                        //char    *modelname;
    0.75f,                                          //float   modelScale;
    "default",                                      //char    *skinname;
    "alien_hud",                                    //char    *hudname;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ),            //int  stages
    { -24, -24, -16 },                              //vec3_t  mins;
    { 24, 24, 16 },                                 //vec3_t  maxs;
    { 24, 24, 16 },                                 //vec3_t  crouchmaxs;
    { -24, -24, -4 },                               //vec3_t  deadmins;
    { 24, 24, 4 },                                  //vec3_t  deadmaxs;
    18, 18,                                         //int     viewheight, crouchviewheight;
    DRAGOON_HEALTH,                                 //int     health;
    DRAGOON_REGEN,                                  //int     regenRate;
    SCA_CANJUMP|SCA_NOWEAPONDRIFT|
      SCA_FOVWARPS|SCA_ALIENSENSE|SCA_NOFOOTSTEPS,  //int     abilities;
    WP_POUNCE,                                      //weapon_t  startWeapon
    0.0f,                                           //float   buildDist;
    110,                                            //int     fov;
    0.0005f,                                        //float   bob;
    25,                                             //int     steptime;
    DRAGOON_SPEED,                                  //float   speed;
    3.0f,                                           //float   sticky;
    { PCL_A_O_LEV2, PCL_A_O_LEV1_UPG, PCL_NONE },   //int     children[ 3 ];
    DRAGOON_TTE,                                    //int     timetoevolve;
    DRAGOON_VALUE                                   //int     value;
  },
  {
    PCL_A_O_LEV1_UPG,                               //int     classnum;
    "dragoonupg",                                   //char    *classname;
    "Dragoon Upgrade",                              //char    *humanname;
    "prowl",                                        //char    *modelname;
    0.75f,                                          //float   modelScale;
    "default",                                      //char    *skinname;
    "alien_hud",                                    //char    *hudname;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ),            //int  stages
    { -24, -24, -16 },                              //vec3_t  mins;
    { 24, 24, 16 },                                 //vec3_t  maxs;
    { 24, 24, 16 },                                 //vec3_t  crouchmaxs;
    { -24, -24, -4 },                               //vec3_t  deadmins;
    { 24, 24, 4 },                                  //vec3_t  deadmaxs;
    20, 20,                                         //int     viewheight, crouchviewheight;
    DRAGOON_UPG_HEALTH,                             //int     health;
    DRAGOON_UPG_REGEN,                              //int     regenRate;
    SCA_CANJUMP|SCA_NOWEAPONDRIFT|
      SCA_FOVWARPS|SCA_ALIENSENSE|SCA_NOFOOTSTEPS,  //int     abilities;
    WP_POUNCE_UPG,                                  //weapon_t  startWeapon
    0.0f,                                           //float   buildDist;
    110,                                            //int     fov;
    0.0005f,                                        //float   bob;
    25,                                             //int     steptime;
    DRAGOON_UPG_SPEED,                              //float   speed;
    3.0f,                                           //float   sticky;
    { PCL_A_O_LEV2, PCL_NONE, PCL_NONE },           //int     children[ 3 ];
    DRAGOON_UPG_TTE,                                //int     timetoevolve;
    DRAGOON_UPG_VALUE                               //int     value;
  },
  {
    PCL_A_O_LEV2,                                   //int     classnum;
    "hydra",                                        //char    *classname;
    "Hydra",                                        //char    *humanname;
    "spitter",                                      //char    *modelname;
    0.6f,                                           //float   modelScale;
    "default",                                      //char    *skinname;
    "alien_hud",                                    //char    *hudname;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ),            //int  stages
    { -18, -18, -18 },                              //vec3_t  mins;
    { 18, 18, 18 },                                 //vec3_t  maxs;
    { 18, 18, 18 },                                 //vec3_t  crouchmaxs;
    { -18, -18, -4 },                               //vec3_t  deadmins;
    { 18, 18, 4 },                                  //vec3_t  deadmaxs;
    18, 18,                                         //int     viewheight, crouchviewheight;
    HYDRA_HEALTH,                                   //int     health;
    HYDRA_REGEN,                                    //int     regenRate;
    SCA_CANJUMP|SCA_NOWEAPONDRIFT|
      SCA_FOVWARPS|SCA_WALLCLIMBER|SCA_ALIENSENSE|
      SCA_NOFOOTSTEPS,                              //int     abilities;
    WP_GRAB_CLAW,                                   //weapon_t  startWeapon
    0.0f,                                           //float   buildDist;
    100,                                            //int     fov;
    0.001f,                                         //float   bob;
    25,                                             //int     steptime;
    HYDRA_SPEED,                                    //float   speed;
    4.0f,                                           //float   sticky;
    { PCL_A_O_LEV3, PCL_A_O_LEV2_UPG, PCL_NONE },   //int     children[ 3 ];
    HYDRA_TTE,                                      //int     timetoevolve;
    HYDRA_VALUE                                     //int     value;
  },
  {
    PCL_A_O_LEV2_UPG,                               //int     classnum;
    "hydraupg",                                     //char    *classname;
    "Hydra Upgrade",                                //char    *humanname;
    "spitter",                                      //char    *modelname;
    0.7f,                                           //float   modelScale;
    "blue",                                         //char    *skinname;
    "alien_hud",                                    //char    *hudname;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ),            //int  stages
    { -20, -20, -20 },                              //vec3_t  mins;
    { 20, 20, 20 },                                 //vec3_t  maxs;
    { 20, 20, 20 },                                 //vec3_t  crouchmaxs;
    { -20, -20, -4 },                               //vec3_t  deadmins;
    { 20, 20, 4 },                                  //vec3_t  deadmaxs;
    20, 20,                                         //int     viewheight, crouchviewheight;
    HYDRA_UPG_HEALTH,                               //int     health;
    HYDRA_UPG_REGEN,                                //int     regenRate;
    SCA_CANJUMP|SCA_NOWEAPONDRIFT|SCA_FOVWARPS|
      SCA_WALLCLIMBER|SCA_ALIENSENSE|
      SCA_NOFOOTSTEPS,                              //int     abilities;
    WP_GRAB_CLAW_UPG,                               //weapon_t  startWeapon
    0.0f,                                           //float   buildDist;
    100,                                            //int     fov;
    0.001f,                                         //float   bob;
    25,                                             //int     steptime;
    HYDRA_UPG_SPEED,                                //float   speed;
    4.0f,                                           //float   sticky;
    { PCL_A_O_LEV3, PCL_NONE, PCL_NONE },           //int     children[ 3 ];
    HYDRA_UPG_TTE,                                  //int     timetoevolve;
    HYDRA_UPG_VALUE                                 //int     value;
  },
  {
    PCL_A_O_LEV3,                                   //int     classnum;
    "chimera",                                      //char    *classname;
    "Chimera",                                      //char    *humanname;
    "tarantula",                                    //char    *modelname;
    0.75f,                                          //float   modelScale;
    "default",                                      //char    *skinname;
    "alien_hud",                                    //char    *hudname;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ),            //int  stages
    { -22, -22, -22 },                              //vec3_t  mins;
    { 22, 22, 22 },                                 //vec3_t  maxs;
    { 22, 22, 22 },                                 //vec3_t  crouchmaxs;
    { -22, -22, -4 },                               //vec3_t  deadmins;
    { 22, 22, 4 },                                  //vec3_t  deadmaxs;
    30, 30,                                         //int     viewheight, crouchviewheight;
    CHIMERA_HEALTH,                                 //int     health;
    CHIMERA_REGEN,                                  //int     regenRate;
    SCA_CANJUMP|SCA_NOWEAPONDRIFT|
      SCA_FOVWARPS|SCA_ALIENSENSE|SCA_NOFOOTSTEPS,  //int     abilities;
    WP_AREA_ZAP,                                    //weapon_t  startWeapon
    0.0f,                                           //float   buildDist;
    90,                                             //int     fov;
    0.0f,                                           //float   bob;
    60,                                             //int     steptime;
    CHIMERA_SPEED,                                  //float   speed;
    5.0f,                                           //float   sticky;
    { PCL_A_O_LEV4, PCL_A_O_LEV3_UPG, PCL_NONE },   //int     children[ 3 ];
    CHIMERA_TTE,                                    //int     timetoevolve;
    CHIMERA_VALUE                                   //int     value;
  },
  {
    PCL_A_O_LEV3_UPG,                               //int     classnum;
    "chimeraupg",                                   //char    *classname;
    "Chimera Upgrade",                              //char    *humanname;
    "tarantula",                                    //char    *modelname;
    0.9f,                                           //float   modelScale;
    "red",                                          //char    *skinname;
    "alien_hud",                                    //char    *hudname;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ),            //int  stages
    { -24, -24, -24 },                              //vec3_t  mins;
    { 24, 24, 24 },                                 //vec3_t  maxs;
    { 24, 24, 24 },                                 //vec3_t  crouchmaxs;
    { -24, -24, -4 },                               //vec3_t  deadmins;
    { 24, 24, 4 },                                  //vec3_t  deadmaxs;
    32, 32,                                         //int     viewheight, crouchviewheight;
    CHIMERA_UPG_HEALTH,                             //int     health;
    CHIMERA_UPG_REGEN,                              //int     regenRate;
    SCA_CANJUMP|SCA_NOWEAPONDRIFT|
      SCA_FOVWARPS|SCA_ALIENSENSE|SCA_NOFOOTSTEPS,  //int     abilities;
    WP_DIRECT_ZAP,                                  //weapon_t  startWeapon
    0.0f,                                           //float   buildDist;
    90,                                             //int     fov;
    0.0f,                                           //float   bob;
    60,                                             //int     steptime;
    CHIMERA_UPG_SPEED,                              //float   speed;
    5.0f,                                           //float   sticky;
    { PCL_A_O_LEV4, PCL_NONE, PCL_NONE },           //int     children[ 3 ];
    CHIMERA_UPG_TTE,                                //int     timetoevolve;
    CHIMERA_UPG_VALUE                               //int     value;
  },
  {
    PCL_A_O_LEV4,                                   //int     classnum;
    "bigmofo",                                      //char    *classname;
    "Big Mofo",                                     //char    *humanname;
    "bug",                                          //char    *modelname;
    1.2f,                                           //float   modelScale;
    "default",                                      //char    *skinname;
    "alien_hud",                                    //char    *hudname;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ),            //int  stages
    { -30, -30, -29 },                              //vec3_t  mins;
    { 30, 30, 29 },                                 //vec3_t  maxs;
    { 30, 30, 29 },                                 //vec3_t  crouchmaxs;
    { -15, -15, -4 },                               //vec3_t  deadmins;
    { 15, 15, 4 },                                  //vec3_t  deadmaxs;
    50, 50,                                         //int     viewheight, crouchviewheight;
    BMOFO_HEALTH,                                   //int     health;
    BMOFO_REGEN,                                    //int     regenRate;
    SCA_CANJUMP|SCA_NOWEAPONDRIFT|
      SCA_FOVWARPS|SCA_ALIENSENSE|SCA_NOFOOTSTEPS,  //int     abilities;
    WP_GROUND_POUND,                                //weapon_t  startWeapon
    0.0f,                                           //float   buildDist;
    90,                                             //int     fov;
    0.0f,                                           //float   bob;
    60,                                             //int     steptime;
    BMOFO_SPEED,                                    //float   speed;
    5.0f,                                           //float   sticky;
    { PCL_NONE, PCL_NONE, PCL_NONE },               //int     children[ 3 ];
    0,                                              //int     timetoevolve;
    BMOFO_VALUE                                     //int     value;
  },
  {
    PCL_H_BASE,                                     //int     classnum;
    "human",                                        //char    *classname;
    "Human",                                        //char    *humanname;
    "sarge",                                        //char    *modelname;
    1.0f,                                           //float   modelScale;
    "default",                                      //char    *skinname;
    "human_hud",                                    //char    *hudname;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ),            //int  stages
    { -15, -15, -24 },                              //vec3_t  mins;
    { 15, 15, 32 },                                 //vec3_t  maxs;
    { 15, 15, 16 },                                 //vec3_t  crouchmaxs;
    { -15, -15, -4 },                               //vec3_t  deadmins;
    { 15, 15, 4 },                                  //vec3_t  deadmaxs;
    26, 12,                                         //int     viewheight, crouchviewheight;
    100,                                            //int     health;
    0,                                              //int     regenRate;
    SCA_TAKESFALLDAMAGE|SCA_CANJUMP,                //int     abilities;
    WP_NONE, //special-cased in g_client.c          //weapon_t  startWeapon
    110.0f,                                         //float   buildDist;
    90,                                             //int     fov;
    0.002f,                                         //float   bob;
    200,                                            //int     steptime;
    1.0f,                                           //float   speed;
    1.0f,                                           //float   sticky;
    { PCL_NONE, PCL_NONE, PCL_NONE },               //int     children[ 3 ];
    0,                                              //int     timetoevolve;
    0                                               //int     value;
  },
  {
    //this isn't a real class, but a dummy to force the client to precache the model
    //FIXME: one day do this in a less hacky fashion
    PCL_H_BSUIT, "bsuit", "bsuit",
    
    "keel",
    1.0f,                                    
    "default",
    
    "bsuit", ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), { 0, 0, 0 }, { 0, 0, 0, },                             
    { 0, 0, 0, }, { 0, 0, 0, }, { 0, 0, 0, }, 0, 0, 0, 0, 0, WP_NONE, 0.0f, 0,   
    0.0f, 0, 1.0f, 1.0f, { PCL_NONE, PCL_NONE, PCL_NONE }, 0, 0
  }
};

int   bg_numPclasses = sizeof( bg_classList ) / sizeof( bg_classList[ 0 ] );

/*
==============
BG_FindClassNumForName
==============
*/
int BG_FindClassNumForName( char *name )
{
  int i;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( !Q_stricmp( bg_classList[ i ].className, name ) )
      return bg_classList[ i ].classNum;
  }

  //wimp out
  return PCL_NONE;
}

/*
==============
BG_FindNameForClassNum
==============
*/
char *BG_FindNameForClassNum( int pclass )
{
  int i;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( bg_classList[ i ].classNum == pclass )
      return bg_classList[ i ].className;
  }

  //wimp out
  return 0;
}

/*
==============
BG_FindHumanNameForClassNum
==============
*/
char *BG_FindHumanNameForClassNum( int pclass )
{
  int i;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( bg_classList[ i ].classNum == pclass )
      return bg_classList[ i ].humanName;
  }

  //wimp out
  return 0;
}

/*
==============
BG_FindModelNameForClass
==============
*/
char *BG_FindModelNameForClass( int pclass )
{
  int i;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( bg_classList[ i ].classNum == pclass )
      return bg_classList[ i ].modelName;
  }

  //note: must return a valid modelName!
  return bg_classList[ 0 ].modelName;
}

/*
==============
BG_FindModelScaleForClass
==============
*/
float BG_FindModelScaleForClass( int pclass )
{
  int i;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( bg_classList[ i ].classNum == pclass )
    {
      return bg_classList[ i ].modelScale;
    }
  }
  
  return 1.0f;
}

/*
==============
BG_FindSkinNameForClass
==============
*/
char *BG_FindSkinNameForClass( int pclass )
{
  int i;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( bg_classList[ i ].classNum == pclass )
      return bg_classList[ i ].skinName;
  }

  //note: must return a valid modelName!
  return bg_classList[ 0 ].skinName;
}

/*
==============
BG_FindHudNameForClass
==============
*/
char *BG_FindHudNameForClass( int pclass )
{
  int i;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( bg_classList[ i ].classNum == pclass )
      return bg_classList[ i ].hudName;
  }

  //note: must return a valid hudName!
  return bg_classList[ 0 ].hudName;
}

/*
==============
BG_FindStagesForClass
==============
*/
qboolean BG_FindStagesForClass( int pclass, stage_t stage )
{
  int i;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( bg_classList[ i ].classNum == pclass )
    {
      if( bg_classList[ i ].stages & ( 1 << stage ) )
        return qtrue;
      else
        return qfalse;
    }
  }
  
  return qfalse;
}

/*
==============
BG_FindBBoxForClass
==============
*/
void BG_FindBBoxForClass( int pclass, vec3_t mins, vec3_t maxs, vec3_t cmaxs, vec3_t dmins, vec3_t dmaxs )
{
  int i;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( bg_classList[ i ].classNum == pclass )
    {
      if( mins != NULL )
        VectorCopy( bg_classList[ i ].mins,        mins );
        
      if( maxs != NULL )
        VectorCopy( bg_classList[ i ].maxs,        maxs );
        
      if( cmaxs != NULL )
        VectorCopy( bg_classList[ i ].crouchMaxs,  cmaxs );
        
      if( dmins != NULL )
        VectorCopy( bg_classList[ i ].deadMins,    dmins );
        
      if( dmaxs != NULL )
        VectorCopy( bg_classList[ i ].deadMaxs,    dmaxs );

      return;
    }
  }
  
  if( mins != NULL )
    VectorCopy( bg_classList[ 0 ].mins,        mins );
    
  if( maxs != NULL )
    VectorCopy( bg_classList[ 0 ].maxs,        maxs );
    
  if( cmaxs != NULL )
    VectorCopy( bg_classList[ 0 ].crouchMaxs,  cmaxs );
    
  if( dmins != NULL )
    VectorCopy( bg_classList[ 0 ].deadMins,    dmins );
    
  if( dmaxs != NULL )
    VectorCopy( bg_classList[ 0 ].deadMaxs,    dmaxs );
}

/*
==============
BG_FindViewheightForClass
==============
*/
void BG_FindViewheightForClass( int pclass, int *viewheight, int *cViewheight )
{
  int i;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( bg_classList[ i ].classNum == pclass )
    {
      if( viewheight != NULL )
        *viewheight = bg_classList[ i ].viewheight;
        
      if( cViewheight != NULL )
        *cViewheight = bg_classList[ i ].crouchViewheight;

      return;
    }
  }
  
  if( viewheight != NULL )
    *viewheight = bg_classList[ 0 ].viewheight;
    
  if( cViewheight != NULL )
    *cViewheight = bg_classList[ 0 ].crouchViewheight;
}

/*
==============
BG_FindHealthForClass
==============
*/
int BG_FindHealthForClass( int pclass )
{
  int i;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( bg_classList[ i ].classNum == pclass )
    {
      return bg_classList[ i ].health;
    }
  }
  
  return 100;
}

/*
==============
BG_FindRegenRateForClass
==============
*/
int BG_FindRegenRateForClass( int pclass )
{
  int i;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( bg_classList[ i ].classNum == pclass )
    {
      return bg_classList[ i ].regenRate;
    }
  }
  
  return 0;
}

/*
==============
BG_FindFovForClass
==============
*/
int BG_FindFovForClass( int pclass )
{
  int i;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( bg_classList[ i ].classNum == pclass )
    {
      return bg_classList[ i ].fov;
    }
  }
  
  return 90;
}

/*
==============
BG_FindBobForClass
==============
*/
float BG_FindBobForClass( int pclass )
{
  int i;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( bg_classList[ i ].classNum == pclass )
    {
      return bg_classList[ i ].bob;
    }
  }
  
  return 0.002;
}

/*
==============
BG_FindSpeedForClass
==============
*/
float BG_FindSpeedForClass( int pclass )
{
  int i;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( bg_classList[ i ].classNum == pclass )
    {
      return bg_classList[ i ].speed;
    }
  }
  
  return 1.0;
}

/*
==============
BG_FindStickyForClass
==============
*/
float BG_FindStickyForClass( int pclass )
{
  int i;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( bg_classList[ i ].classNum == pclass )
    {
      return bg_classList[ i ].sticky;
    }
  }
  
  return 1.0;
}

/*
==============
BG_FindSteptimeForClass
==============
*/
int BG_FindSteptimeForClass( int pclass )
{
  int i;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( bg_classList[ i ].classNum == pclass )
    {
      return bg_classList[ i ].steptime;
    }
  }
  
  return 200;
}

/*
==============
BG_ClassHasAbility
==============
*/
qboolean BG_ClassHasAbility( int pclass, int ability )
{
  int i;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( bg_classList[ i ].classNum == pclass )
    {
      return ( bg_classList[ i ].abilities & ability );
    }
  }

  //hack to get CANJUMP when a spectator
  if( ability == SCA_CANJUMP )
    return qtrue;
  else
    return qfalse;
}

/*
==============
BG_FindStartWeaponForClass
==============
*/
weapon_t BG_FindStartWeaponForClass( int pclass )
{
  int i;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( bg_classList[ i ].classNum == pclass )
    {
      return bg_classList[ i ].startWeapon;
    }
  }
  
  return WP_NONE;
}

/*
==============
BG_FindBuildDistForClass
==============
*/
float BG_FindBuildDistForClass( int pclass )
{
  int i;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( bg_classList[ i ].classNum == pclass )
    {
      return bg_classList[ i ].buildDist;
    }
  }
  
  return 0.0f;
}

/*
==============
BG_ClassCanEvolveFromTo
==============
*/
int BG_ClassCanEvolveFromTo( int fclass, int tclass, int credits, int num )
{
  int i, j;

  //base case
  if( credits + 1 == 0 )
    return 0;

  if( tclass == PCL_NONE )
    return 0;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( bg_classList[ i ].classNum == fclass )
    {
      for( j = 0; j <= 3; j++ )
        if( bg_classList[ i ].children[ j ] == tclass )
          return num + 1;

      for( j = 0; j <= 3; j++ )
      {
        int sub = BG_ClassCanEvolveFromTo(  bg_classList[ i ].children[ j ],
                                            tclass, credits - 1, num + 1 );
        if( sub )
          return sub;
      }
      
      return 0; //may as well return by this point
    }
  }

  return 0;
}

/*
==============
BG_FindEvolveTimeForClass
==============
*/
int BG_FindEvolveTimeForClass( int pclass )
{
  int i;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( bg_classList[ i ].classNum == pclass )
    {
      return bg_classList[ i ].timeToEvolve;
    }
  }
  
  return 5000;
}

/*
==============
BG_FindValueOfClass
==============
*/
int BG_FindValueOfClass( int pclass )
{
  int i;

  for( i = 0; i < bg_numPclasses; i++ )
  {
    if( bg_classList[ i ].classNum == pclass )
    {
      return bg_classList[ i ].value;
    }
  }
  
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

weaponAttributes_t bg_weapons[ ] =
{
  {
    WP_MACHINEGUN,        //int       weaponNum;
    RIFLE_PRICE,          //int       price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_WEAPON,          //int       slots;
    "rifle",              //char      *weaponName;
    "Rifle",              //char      *weaponHumanName;
    { "models/weapons2/machinegun/machinegun.md3", 0, 0, 0 },
    "icons/iconw_rifle",
    "gfx/2d/crosshaira", 24,
    RIFLE_CLIPSIZE,       //int       quan;
    RIFLE_SPAWNCLIPS,     //int       clips;
    RIFLE_MAXCLIPS,       //int       maxClips;
    qfalse,               //int       infiniteAmmo;
    qfalse,               //int       usesEnergy;
    RIFLE_REPEAT,         //int       repeatRate1;
    0,                    //int       repeatRate2;
    0,                    //int       repeatRate3;
    RIFLE_RELOAD,         //int       reloadTime;
    qfalse,               //qboolean  hasAltMode;
    qfalse,               //qboolean  hasThirdMode;
    qtrue,                //qboolean  purchasable;
    0,                    //int       buildDelay;
    WUT_HUMANS            //WUTeam_t  team;
  },
  {
    WP_FLAMER,            //int       weaponNum;
    FLAMER_PRICE,         //int       price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_WEAPON,          //int       slots;
    "flamer",             //char      *weaponName;
    "Flame Thrower",      //char      *weaponHumanName;
    { "models/weapons2/plasma/plasma.md3", 0, 0, 0 },
    "icons/iconw_flamer",
    "gfx/2d/crosshaira", 24,
    FLAMER_GAS,           //int       quan;
    0,                    //int       clips;
    0,                    //int       maxClips;
    qfalse,               //int       infiniteAmmo;
    qfalse,               //int       usesEnergy;
    FLAMER_REPEAT,        //int       repeatRate1;
    0,                    //int       repeatRate2;
    0,                    //int       repeatRate3;
    0,                    //int       reloadTime;
    qfalse,               //qboolean  hasAltMode;
    qfalse,               //qboolean  hasThirdMode;
    qtrue,                //qboolean  purchasable;
    0,                    //int       buildDelay;
    WUT_HUMANS            //WUTeam_t  team;
  },
  {
    WP_CHAINGUN,          //int       weaponNum;
    CHAINGUN_PRICE,       //int       price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_WEAPON,          //int       slots;
    "chaingun",           //char      *weaponName;
    "Chaingun",           //char      *weaponHumanName;
    { "models/weapons2/machinegun/machinegun.md3", 0, 0, 0 },
    "icons/iconw_chaingun",
    "gfx/2d/crosshairb", 48,
    CHAINGUN_BULLETS,     //int       quan;
    0,                    //int       clips;
    0,                    //int       maxClips;
    qfalse,               //int       infiniteAmmo;
    qfalse,               //int       usesEnergy;
    CHAINGUN_REPEAT,      //int       repeatRate1;
    0,                    //int       repeatRate2;
    0,                    //int       repeatRate3;
    0,                    //int       reloadTime;
    qfalse,               //qboolean  hasAltMode;
    qfalse,               //qboolean  hasThirdMode;
    qtrue,                //qboolean  purchasable;
    0,                    //int       buildDelay;
    WUT_HUMANS            //WUTeam_t  team;
  },
  {
    WP_MASS_DRIVER,       //int       weaponNum;
    MDRIVER_PRICE,        //int       price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_WEAPON,          //int       slots;
    "mdriver",            //char      *weaponName;
    "Mass Driver",        //char      *weaponHumanName;
    { "models/weapons2/bfg/bfg.md3", 0, 0, 0 },
    "icons/iconw_driver",
    "gfx/2d/crosshaira", 24,
    MDRIVER_CLIPSIZE,     //int       quan;
    MDRIVER_SPAWNCLIPS,   //int       clips;
    MDRIVER_MAXCLIPS,     //int       maxClips;
    qfalse,               //int       infiniteAmmo;
    qtrue,                //int       usesEnergy;
    MDRIVER_REPEAT,       //int       repeatRate1;
    0,                    //int       repeatRate2;
    0,                    //int       repeatRate3;
    MDRIVER_RELOAD,       //int       reloadTime;
    qfalse,               //qboolean  hasAltMode;
    qfalse,               //qboolean  hasThirdMode;
    qtrue,                //qboolean  purchasable;
    0,                    //int       buildDelay;
    WUT_HUMANS            //WUTeam_t  team;
  },
  {
    WP_PULSE_RIFLE,       //int       weaponNum;
    PRIFLE_PRICE,         //int       price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_WEAPON,          //int       slots;
    "prifle",             //char      *weaponName;
    "Pulse Rifle",        //char      *weaponHumanName;
    { "models/weapons2/prifle/prifle.md3", 0, 0, 0 },
    "icons/iconw_pulse",
    "gfx/2d/crosshaira", 24,
    PRIFLE_CLIPS,         //int       quan;
    PRIFLE_SPAWNCLIPS,    //int       clips;
    PRIFLE_MAXCLIPS,      //int       maxClips;
    qfalse,               //int       infiniteAmmo;
    qtrue,                //int       usesEnergy;
    PRIFLE_REPEAT,        //int       repeatRate1;
    0,                    //int       repeatRate2;
    0,                    //int       repeatRate3;
    PRIFLE_RELOAD,        //int       reloadTime;
    qfalse,               //qboolean  hasAltMode;
    qfalse,               //qboolean  hasThirdMode;
    qtrue,                //qboolean  purchasable;
    0,                    //int       buildDelay;
    WUT_HUMANS            //WUTeam_t  team;
  },
  {
    WP_LUCIFER_CANON,     //int       weaponNum;
    LCANON_PRICE,         //int       price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_WEAPON,          //int       slots;
    "lcanon",             //char      *weaponName;
    "Lucifer Canon",      //char      *weaponHumanName;
    { "models/weapons2/bfg/bfg.md3", 0, 0, 0 },
    "icons/iconw_lucifer",
    "gfx/2d/crosshaira", 24,
    LCANON_AMMO,          //int       quan;
    0,                    //int       clips;
    0,                    //int       maxClips;
    qfalse,               //int       infiniteAmmo;
    qtrue,                //int       usesEnergy;
    LCANON_REPEAT,        //int       repeatRate1;
    LCANON_CHARGEREPEAT,  //int       repeatRate2;
    0,                    //int       repeatRate3;
    LCANON_RELOAD,        //int       reloadTime;
    qtrue,                //qboolean  hasAltMode;
    qfalse,               //qboolean  hasThirdMode;
    qtrue,                //qboolean  purchasable;
    0,                    //int       buildDelay;
    WUT_HUMANS            //WUTeam_t  team;
  },
  {
    WP_LAS_GUN,           //int       weaponNum;
    LASGUN_PRICE,         //int       price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_WEAPON,          //int       slots;
    "lgun",               //char      *weaponName;
    "Las Gun",            //char      *weaponHumanName;
    { "models/weapons2/grenadel/grenadel.md3", 0, 0, 0 },
    "icons/iconw_lasgun",
    "gfx/2d/crosshaira", 24,
    LASGUN_AMMO,          //int       quan;
    0,                    //int       clips;
    0,                    //int       maxClips;
    qfalse,               //int       infiniteAmmo;
    qtrue,                //int       usesEnergy;
    LASGUN_REPEAT,        //int       repeatRate1;
    0,                    //int       repeatRate2;
    0,                    //int       repeatRate3;
    LASGUN_RELOAD,        //int       reloadTime;
    qfalse,               //qboolean  hasAltMode;
    qfalse,               //qboolean  hasThirdMode;
    qtrue,                //qboolean  purchasable;
    0,                    //int       buildDelay;
    WUT_HUMANS            //WUTeam_t  team;
  },
  {
    WP_PAIN_SAW,          //int       weaponNum;
    PAINSAW_PRICE,        //int       price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_WEAPON,          //int       slots;
    "psaw",               //char      *weaponName;
    "Pain Saw",           //char      *weaponHumanName;
    { "models/weapons2/gauntlet/gauntlet.md3", 0, 0, 0 },
    "icons/iconw_saw",
    NULL, 0,
    0,                    //int       quan;
    0,                    //int       clips;
    0,                    //int       maxClips;
    qtrue,                //int       infiniteAmmo;
    qfalse,               //int       usesEnergy;
    PAINSAW_REPEAT,       //int       repeatRate1;
    0,                    //int       repeatRate2;
    0,                    //int       repeatRate3;
    0,                    //int       reloadTime;
    qfalse,               //qboolean  hasAltMode;
    qfalse,               //qboolean  hasThirdMode;
    qtrue,                //qboolean  purchasable;
    0,                    //int       buildDelay;
    WUT_HUMANS            //WUTeam_t  team;
  },
  {
    WP_HBUILD,            //int       weaponNum;
    HBUILD_PRICE,         //int       price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_WEAPON,          //int       slots;
    "ckit",               //char      *weaponName;
    "Construction Kit",   //char      *weaponHumanName;
    { "models/weapons2/gauntlet/gauntlet.md3", 0, 0, 0 },
    "icons/iconw_construct",
    NULL, 0,
    0,                    //int       quan;
    0,                    //int       clips;
    0,                    //int       maxClips;
    qtrue,                //int       infiniteAmmo;
    qfalse,               //int       usesEnergy;
    HBUILD_REPEAT,        //int       repeatRate1;
    HBUILD_REPEAT,        //int       repeatRate2;
    0,                    //int       repeatRate3;
    0,                    //int       reloadTime;
    qtrue,                //qboolean  hasAltMode;
    qfalse,               //qboolean  hasThirdMode;
    qtrue,                //qboolean  purchasable;
    HBUILD_DELAY,         //int       buildDelay;
    WUT_HUMANS            //WUTeam_t  team;
  },
  {
    WP_HBUILD2,           //int       weaponNum;
    HBUILD2_PRICE,        //int       price;
    ( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_WEAPON,          //int       slots;
    "ackit",              //char      *weaponName;
    "Adv Construction Kit",//char      *weaponHumanName;
    { "models/weapons2/gauntlet/gauntlet.md3", 0, 0, 0 },
    "icons/iconw_construct",
    NULL, 0,
    0,                    //int       quan;
    0,                    //int       clips;
    0,                    //int       maxClips;
    qtrue,                //int       infiniteAmmo;
    qfalse,               //int       usesEnergy;
    HBUILD2_REPEAT,       //int       repeatRate1;
    HBUILD2_REPEAT,       //int       repeatRate2;
    0,                    //int       repeatRate3;
    0,                    //int       reloadTime;
    qtrue,                //qboolean  hasAltMode;
    qfalse,               //qboolean  hasThirdMode;
    qtrue,                //qboolean  purchasable;
    HBUILD2_DELAY,        //int       buildDelay;
    WUT_HUMANS            //WUTeam_t  team;
  },
  {
    WP_ABUILD,            //int       weaponNum;
    0,                    //int       price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_WEAPON,          //int       slots;
    "abuild",             //char      *weaponName;
    "Alien build weapon", //char      *weaponHumanName;
    { "models/weapons2/gauntlet/gauntlet.md3", 0, 0, 0 },
    "icons/iconw_gauntlet",
    NULL, 0,
    0,                    //int       quan;
    0,                    //int       clips;
    0,                    //int       maxClips;
    qtrue,                //int       infiniteAmmo;
    qfalse,               //int       usesEnergy;
    ABUILDER_BUILD_REPEAT,//int       repeatRate1;
    0,                    //int       repeatRate2;
    0,                    //int       repeatRate3;
    0,                    //int       reloadTime;
    qtrue,                //qboolean  hasAltMode;
    qfalse,               //qboolean  hasThirdMode;
    qfalse,               //qboolean  purchasable;
    ABUILDER_BASE_DELAY,  //int       buildDelay;
    WUT_ALIENS            //WUTeam_t  team;
  },
  {
    WP_ABUILD2,           //int       weaponNum;
    0,                    //int       price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_WEAPON,          //int       slots;
    "abuild2",            //char      *weaponName;
    "Alien build weapon2",//char      *weaponHumanName;
    { "models/weapons2/gauntlet/gauntlet.md3", 0, 0, 0 },
    "icons/iconw_gauntlet",
    NULL, 0,
    0,                    //int       quan;
    0,                    //int       clips;
    0,                    //int       maxClips;
    qtrue,                //int       infiniteAmmo;
    qfalse,               //int       usesEnergy;
    ABUILDER_BUILD_REPEAT,//int       repeatRate1;
    ABUILDER_CLAW_REPEAT, //int       repeatRate2;
    0,                    //int       repeatRate3;
    0,                    //int       reloadTime;
    qtrue,                //qboolean  hasAltMode;
    qfalse,               //qboolean  hasThirdMode;
    qfalse,               //qboolean  purchasable;
    ABUILDER_ADV_DELAY,   //int       buildDelay;
    WUT_ALIENS            //WUTeam_t  team;
  },
  {
    WP_VENOM,             //int       weaponNum;
    0,                    //int       price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_WEAPON,          //int       slots;
    "venom",              //char      *weaponName;
    "Venom",              //char      *weaponHumanName;
    { "models/weapons2/gauntlet/gauntlet.md3", 0, 0, 0 },
    "icons/iconw_gauntlet",
    NULL, 0,
    0,                    //int       quan;
    0,                    //int       clips;
    0,                    //int       maxClips;
    qtrue,                //int       infiniteAmmo;
    qfalse,               //int       usesEnergy;
    SOLDIER_BITE_REPEAT,  //int       repeatRate1;
    0,                    //int       repeatRate2;
    0,                    //int       repeatRate3;
    0,                    //int       reloadTime;
    qfalse,               //qboolean  hasAltMode;
    qfalse,               //qboolean  hasThirdMode;
    qfalse,               //qboolean  purchasable;
    0,                    //int       buildDelay;
    WUT_ALIENS            //WUTeam_t  team;
  },
  {
    WP_POUNCE,            //int       weaponNum;
    0,                    //int       price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_WEAPON,          //int       slots;
    "pounce",             //char      *weaponName;
    "Claw and pounce",    //char      *weaponHumanName;
    { "models/weapons2/gauntlet/gauntlet.md3", 0, 0, 0 },
    "icons/iconw_gauntlet",
    NULL, 0,
    0,                    //int       quan;
    0,                    //int       clips;
    0,                    //int       maxClips;
    qtrue,                //int       infiniteAmmo;
    qfalse,               //int       usesEnergy;
    DRAGOON_CLAW_REPEAT,  //int       repeatRate1;
    0,                    //int       repeatRate2;
    0,                    //int       repeatRate3;
    0,                    //int       reloadTime;
    qfalse,               //qboolean  hasAltMode;
    qfalse,               //qboolean  hasThirdMode;
    qfalse,               //qboolean  purchasable;
    0,                    //int       buildDelay;
    WUT_ALIENS            //WUTeam_t  team;
  },
  {
    WP_POUNCE_UPG,        //int       weaponNum;
    0,                    //int       price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_WEAPON,          //int       slots;
    "pounce_upgrade",     //char      *weaponName;
    "Claw and pounce (upgrade)", //char      *weaponHumanName;
    { "models/weapons2/gauntlet/gauntlet.md3", 0, 0, 0 },
    "icons/iconw_gauntlet",
    NULL, 0,
    3,                    //int       quan;
    0,                    //int       clips;
    0,                    //int       maxClips;
    qtrue,                //int       infiniteAmmo;
    qfalse,               //int       usesEnergy;
    DRAGOON_CLAW_U_REPEAT,//int       repeatRate1;
    0,                    //int       repeatRate2;
    DRAGOON_SLOWBLOB_REPEAT,//int       repeatRate3;
    0,                    //int       reloadTime;
    qfalse,               //qboolean  hasAltMode;
    qtrue,                //qboolean  hasThirdMode;
    qfalse,               //qboolean  purchasable;
    0,                    //int       buildDelay;
    WUT_ALIENS            //WUTeam_t  team;
  },
  {
    WP_GRAB_CLAW,         //int       weaponNum;
    0,                    //int       price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_WEAPON,          //int       slots;
    "grabandclaw",        //char      *weaponName;
    "Claws",              //char      *weaponHumanName;
    { "models/weapons2/gauntlet/gauntlet.md3", 0, 0, 0 },
    "icons/iconw_gauntlet",
    NULL, 0,
    0,                    //int       quan;
    0,                    //int       clips;
    0,                    //int       maxClips;
    qtrue,                //int       infiniteAmmo;
    qfalse,               //int       usesEnergy;
    HYDRA_CLAW_REPEAT,    //int       repeatRate1;
    0,                    //int       repeatRate2;
    0,                    //int       repeatRate3;
    0,                    //int       reloadTime;
    qfalse,               //qboolean  hasAltMode;
    qfalse,               //qboolean  hasThirdMode;
    qfalse,               //qboolean  purchasable;
    0,                    //int       buildDelay;
    WUT_ALIENS            //WUTeam_t  team;
  },
  {
    WP_GRAB_CLAW_UPG,     //int       weaponNum;
    0,                    //int       price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_WEAPON,          //int       slots;
    "grabandclaw_upgrade",//char      *weaponName;
    "Claws Upgrade",      //char      *weaponHumanName;
    { "models/weapons2/gauntlet/gauntlet.md3", 0, 0, 0 },
    "icons/iconw_gauntlet",
    NULL, 0,
    0,                    //int       quan;
    0,                    //int       clips;
    0,                    //int       maxClips;
    qtrue,                //int       infiniteAmmo;
    qfalse,               //int       usesEnergy;
    HYDRA_CLAW_U_REPEAT,  //int       repeatRate1;
    HYDRA_PCLOUD_REPEAT,  //int       repeatRate2;
    0,                    //int       repeatRate3;
    0,                    //int       reloadTime;
    qtrue,                //qboolean  hasAltMode;
    qfalse,               //qboolean  hasThirdMode;
    qfalse,               //qboolean  purchasable;
    0,                    //int       buildDelay;
    WUT_ALIENS            //WUTeam_t  team;
  },
  {
    WP_AREA_ZAP,          //int       weaponNum;
    0,                    //int       price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_WEAPON,          //int       slots;
    "areazap",            //char      *weaponName;
    "Area Zap",           //char      *weaponHumanName;
    { "models/weapons2/gauntlet/gauntlet.md3", 0, 0, 0 },
    "icons/iconw_gauntlet",
    NULL, 0,
    0,                    //int       quan;
    0,                    //int       clips;
    0,                    //int       maxClips;
    qtrue,                //int       infiniteAmmo;
    qfalse,               //int       usesEnergy;
    CHIMERA_CLAW_REPEAT,  //int       repeatRate1;
    CHIMERA_AREAZAP_REPEAT,//int       repeatRate2;
    0,                    //int       repeatRate3;
    0,                    //int       reloadTime;
    qtrue,                //qboolean  hasAltMode;
    qfalse,               //qboolean  hasThirdMode;
    qfalse,               //qboolean  purchasable;
    0,                    //int       buildDelay;
    WUT_ALIENS            //WUTeam_t  team;
  },
  {
    WP_DIRECT_ZAP,        //int       weaponNum;
    0,                    //int       price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_WEAPON,          //int       slots;
    "directzap",          //char      *weaponName;
    "Directed Zap",       //char      *weaponHumanName;
    { "models/weapons2/gauntlet/gauntlet.md3", 0, 0, 0 },
    "icons/iconw_gauntlet",
    NULL, 0,
    0,                    //int       quan;
    0,                    //int       clips;
    0,                    //int       maxClips;
    qtrue,                //int       infiniteAmmo;
    qfalse,               //int       usesEnergy;
    CHIMERA_CLAW_U_REPEAT,//int       repeatRate1;
    CHIMERA_DIRECTZAP_REPEAT,//int       repeatRate2;
    CHIMERA_AREAZAP_REPEAT,//int       repeatRate3;
    0,                    //int       reloadTime;
    qtrue,                //qboolean  hasAltMode;
    qtrue,                //qboolean  hasThirdMode;
    qfalse,               //qboolean  purchasable;
    0,                    //int       buildDelay;
    WUT_ALIENS            //WUTeam_t  team;
  },
  {
    WP_GROUND_POUND,      //int       weaponNum;
    0,                    //int       price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_WEAPON,          //int       slots;
    "groundpound",        //char      *weaponName;
    "Ground Pound",       //char      *weaponHumanName;
    { "models/weapons2/gauntlet/gauntlet.md3", 0, 0, 0 },
    "icons/iconw_gauntlet",
    NULL, 0,
    0,                    //int       quan;
    0,                    //int       clips;
    0,                    //int       maxClips;
    qtrue,                //int       infiniteAmmo;
    qfalse,               //int       usesEnergy;
    BMOFO_CLAW_REPEAT,    //int       repeatRate1;
    BMOFO_KNOCK_REPEAT,   //int       repeatRate2;
    BMOFO_CLAW_REPEAT,    //int       repeatRate3;
    0,                    //int       reloadTime;
    qtrue,                //qboolean  hasAltMode;
    qtrue,                //qboolean  hasThirdMode;
    qfalse,               //qboolean  purchasable;
    0,                    //int       buildDelay;
    WUT_ALIENS            //WUTeam_t  team;
  },
  {
    WP_LOCKBLOB_LAUNCHER, //int       weaponNum;
    0,                    //int       price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_WEAPON,          //int       slots;
    "lockblob",           //char      *weaponName;
    "Lock Blob",          //char      *weaponHumanName;
    { "models/weapons2/gauntlet/gauntlet.md3", 0, 0, 0 },
    "icons/iconw_gauntlet",
    NULL, 0,
    0,                    //int       quan;
    0,                    //int       clips;
    0,                    //int       maxClips;
    qtrue,                //int       infiniteAmmo;
    qfalse,               //int       usesEnergy;
    500,                  //int       repeatRate1;
    500,                  //int       repeatRate2;
    500,                  //int       repeatRate3;
    0,                    //int       reloadTime;
    qfalse,               //qboolean  hasAltMode;
    qfalse,               //qboolean  hasThirdMode;
    qfalse,               //qboolean  purchasable;
    0,                    //int       buildDelay;
    WUT_ALIENS            //WUTeam_t  team;
  },
  {
    WP_TESLAGEN,          //int       weaponNum;
    0,                    //int       price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_WEAPON,          //int       slots;
    "teslagen",           //char      *weaponName;
    "Tesla Generator",    //char      *weaponHumanName;
    { "models/weapons2/gauntlet/gauntlet.md3", 0, 0, 0 },
    "icons/iconw_gauntlet",
    NULL, 0,
    0,                    //int       quan;
    0,                    //int       clips;
    0,                    //int       maxClips;
    qtrue,                //int       infiniteAmmo;
    qtrue,                //int       usesEnergy;
    500,                  //int       repeatRate1;
    500,                  //int       repeatRate2;
    500,                  //int       repeatRate3;
    0,                    //int       reloadTime;
    qfalse,               //qboolean  hasAltMode;
    qfalse,               //qboolean  hasThirdMode;
    qfalse,               //qboolean  purchasable;
    0,                    //int       buildDelay;
    WUT_HUMANS            //WUTeam_t  team;
  }
};

int   bg_numWeapons = sizeof( bg_weapons ) / sizeof( bg_weapons[ 0 ] );

/*
==============
BG_FindPriceForWeapon
==============
*/
int BG_FindPriceForWeapon( int weapon )
{
  int i;

  for( i = 0; i < bg_numWeapons; i++ )
  {
    if( bg_weapons[ i ].weaponNum == weapon )
    {
      return bg_weapons[ i ].price;
    }
  }
  
  return 100;
}

/*
==============
BG_FindStagesForWeapon
==============
*/
qboolean BG_FindStagesForWeapon( int weapon, stage_t stage )
{
  int i;

  for( i = 0; i < bg_numWeapons; i++ )
  {
    if( bg_weapons[ i ].weaponNum == weapon )
    {
      if( bg_weapons[ i ].stages & ( 1 << stage ) )
        return qtrue;
      else
        return qfalse;
    }
  }
  
  return qfalse;
}

/*
==============
BG_FindSlotsForWeapon
==============
*/
int BG_FindSlotsForWeapon( int weapon )
{
  int i;

  for( i = 0; i < bg_numWeapons; i++ )
  {
    if( bg_weapons[ i ].weaponNum == weapon )
    {
      return bg_weapons[ i ].slots;
    }
  }
  
  return SLOT_WEAPON;
}

/*
==============
BG_FindNameForWeapon
==============
*/
char *BG_FindNameForWeapon( int weapon )
{
  int i;

  for( i = 0; i < bg_numWeapons; i++ )
  {
    if( bg_weapons[ i ].weaponNum == weapon )
      return bg_weapons[ i ].weaponName;
  }

  //wimp out
  return 0;
}

/*
==============
BG_FindWeaponNumForName
==============
*/
int BG_FindWeaponNumForName( char *name )
{
  int i;

  for( i = 0; i < bg_numWeapons; i++ )
  {
    if( !Q_stricmp( bg_weapons[ i ].weaponName, name ) )
      return bg_weapons[ i ].weaponNum;
  }

  //wimp out
  return WP_NONE;
}

/*
==============
BG_FindHumanNameForWeapon
==============
*/
char *BG_FindHumanNameForWeapon( int weapon )
{
  int i;

  for( i = 0; i < bg_numWeapons; i++ )
  {
    if( bg_weapons[ i ].weaponNum == weapon )
      return bg_weapons[ i ].weaponHumanName;
  }

  //wimp out
  return 0;
}

/*
==============
BG_FindModelsForWeapon
==============
*/
char *BG_FindModelsForWeapon( int weapon, int modelNum )
{
  int i;

  for( i = 0; i < bg_numWeapons; i++ )
  {
    if( bg_weapons[ i ].weaponNum == weapon )
      return bg_weapons[ i ].models[ modelNum ];
  }

  //wimp out
  return 0;
}

/*
==============
BG_FindIconForWeapon
==============
*/
char *BG_FindIconForWeapon( int weapon )
{
  int i;

  for( i = 0; i < bg_numWeapons; i++ )
  {
    if( bg_weapons[ i ].weaponNum == weapon )
      return bg_weapons[ i ].icon;
  }

  //wimp out
  return 0;
}

/*
==============
BG_FindCrosshairForWeapon
==============
*/
char *BG_FindCrosshairForWeapon( int weapon )
{
  int i;

  for( i = 0; i < bg_numWeapons; i++ )
  {
    if( bg_weapons[ i ].weaponNum == weapon )
      return bg_weapons[ i ].crosshair;
  }

  //wimp out
  return 0;
}

/*
==============
BG_FindCrosshairSizeForWeapon
==============
*/
int BG_FindCrosshairSizeForWeapon( int weapon )
{
  int i;

  for( i = 0; i < bg_numWeapons; i++ )
  {
    if( bg_weapons[ i ].weaponNum == weapon )
      return bg_weapons[ i ].crosshairSize;
  }
  
  return 24;
}

/*
==============
BG_FindAmmoForWeapon
==============
*/
void BG_FindAmmoForWeapon( int weapon, int *quan, int *clips, int *maxClips )
{
  int i;

  for( i = 0; i < bg_numWeapons; i++ )
  {
    if( bg_weapons[ i ].weaponNum == weapon )
    {
      if( quan != NULL )
        *quan = bg_weapons[ i ].quan;
      if( clips != NULL )
        *clips = bg_weapons[ i ].clips;
      if( maxClips != NULL )
        *maxClips = bg_weapons[ i ].maxClips;

      //no need to keep going
      break;
    }
  }
}

/*
==============
BG_FindInfinteAmmoForWeapon
==============
*/
qboolean BG_FindInfinteAmmoForWeapon( int weapon )
{
  int i;

  for( i = 0; i < bg_numWeapons; i++ )
  {
    if( bg_weapons[ i ].weaponNum == weapon )
    {
      return bg_weapons[ i ].infiniteAmmo;
    }
  }

  return qfalse;
}

/*
==============
BG_FindUsesEnergyForWeapon
==============
*/
qboolean BG_FindUsesEnergyForWeapon( int weapon )
{
  int i;

  for( i = 0; i < bg_numWeapons; i++ )
  {
    if( bg_weapons[ i ].weaponNum == weapon )
    {
      return bg_weapons[ i ].usesEnergy;
    }
  }

  return qfalse;
}

/*
==============
BG_FindRepeatRate1ForWeapon
==============
*/
int BG_FindRepeatRate1ForWeapon( int weapon )
{
  int i;

  for( i = 0; i < bg_numWeapons; i++ )
  {
    if( bg_weapons[ i ].weaponNum == weapon )
      return bg_weapons[ i ].repeatRate1;
  }
  
  return 1000;
}

/*
==============
BG_FindRepeatRate2ForWeapon
==============
*/
int BG_FindRepeatRate2ForWeapon( int weapon )
{
  int i;

  for( i = 0; i < bg_numWeapons; i++ )
  {
    if( bg_weapons[ i ].weaponNum == weapon )
      return bg_weapons[ i ].repeatRate2;
  }
  
  return 1000;
}

/*
==============
BG_FindRepeatRate3ForWeapon
==============
*/
int BG_FindRepeatRate3ForWeapon( int weapon )
{
  int i;

  for( i = 0; i < bg_numWeapons; i++ )
  {
    if( bg_weapons[ i ].weaponNum == weapon )
      return bg_weapons[ i ].repeatRate3;
  }
  
  return 1000;
}

/*
==============
BG_FindReloadTimeForWeapon
==============
*/
int BG_FindReloadTimeForWeapon( int weapon )
{
  int i;

  for( i = 0; i < bg_numWeapons; i++ )
  {
    if( bg_weapons[ i ].weaponNum == weapon )
    {
      return bg_weapons[ i ].reloadTime;
    }
  }
  
  return 1000;
}

/*
==============
BG_WeaponHasAltMode
==============
*/
qboolean BG_WeaponHasAltMode( int weapon )
{
  int i;

  for( i = 0; i < bg_numWeapons; i++ )
  {
    if( bg_weapons[ i ].weaponNum == weapon )
    {
      return bg_weapons[ i ].hasAltMode;
    }
  }

  return qfalse;
}

/*
==============
BG_WeaponHasThirdMode
==============
*/
qboolean BG_WeaponHasThirdMode( int weapon )
{
  int i;

  for( i = 0; i < bg_numWeapons; i++ )
  {
    if( bg_weapons[ i ].weaponNum == weapon )
    {
      return bg_weapons[ i ].hasThirdMode;
    }
  }

  return qfalse;
}

/*
==============
BG_FindPurchasableForWeapon
==============
*/
qboolean BG_FindPurchasableForWeapon( int weapon )
{
  int i;

  for( i = 0; i < bg_numWeapons; i++ )
  {
    if( bg_weapons[ i ].weaponNum == weapon )
    {
      return bg_weapons[ i ].purchasable;
    }
  }

  return qfalse;
}

/*
==============
BG_FindBuildDelayForWeapon
==============
*/
int BG_FindBuildDelayForWeapon( int weapon )
{
  int i;

  for( i = 0; i < bg_numWeapons; i++ )
  {
    if( bg_weapons[ i ].weaponNum == weapon )
    {
      return bg_weapons[ i ].buildDelay;
    }
  }
  
  return 0;
}

/*
==============
BG_FindTeamForWeapon
==============
*/
WUTeam_t BG_FindTeamForWeapon( int weapon )
{
  int i;

  for( i = 0; i < bg_numWeapons; i++ )
  {
    if( bg_weapons[ i ].weaponNum == weapon )
    {
      return bg_weapons[ i ].team;
    }
  }

  return WUT_NONE;
}

////////////////////////////////////////////////////////////////////////////////

upgradeAttributes_t bg_upgrades[ ] =
{
  {
    UP_CHESTARMOUR,         //int   upgradeNum;
    CHESTARMOUR_PRICE,      //int   price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_TORSO,             //int   slots;
    "carmour",              //char  *upgradeName;
    "Chest Armour",         //char  *upgradeHumanName;
    "icons/iconw_bfg",
    WP_NONE,                //weapon_t weaponAmmo;
    0,                      //int   ammo;
    0,                      //int   clips;
    WUT_HUMANS              //WUTeam_t  team;
  },
  {
    UP_LIMBARMOUR,          //int   upgradeNum;        
    LIMBARMOUR_PRICE,       //int   price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_ARMS|SLOT_LEGS,    //int   slots;
    "larmour",              //char  *upgradeName;
    "Limb Armour",          //char  *upgradeHumanName;
    "icons/iconw_plasma",
    WP_NONE,                //weapon_t weaponAmmo;
    0,                      //int   ammo;
    0,                      //int   clips;
    WUT_HUMANS              //WUTeam_t  team;
  },
  {
    UP_HELMET,              //int   upgradeNum;
    HELMET_PRICE,           //int   price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_HEAD,              //int   slots;
    "helmet",               //char  *upgradeName;
    "Helmet",               //char  *upgradeHumanName;
    "icons/iconw_gauntlet",
    WP_NONE,                //weapon_t weaponAmmo;
    0,                      //int   ammo;
    0,                      //int   clips;
    WUT_HUMANS              //WUTeam_t  team;
  },
  {
    UP_ANTITOXIN,           //int   upgradeNum;
    ANTITOXIN_PRICE,        //int   price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_NONE,              //int   slots;
    "atoxin",               //char  *upgradeName;
    "Anti-toxin",           //char  *upgradeHumanName;
    "icons/iconw_machinegun",
    WP_NONE,                //weapon_t weaponAmmo;
    0,                      //int   ammo;
    0,                      //int   clips;
    WUT_HUMANS              //WUTeam_t  team;
  },
  {
    UP_BATTPACK,            //int   upgradeNum;
    BATTPACK_PRICE,         //int   price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_BACKPACK,          //int   slots;
    "battpack",             //char  *upgradeName;
    "Battery Pack",         //char  *upgradeHumanName;
    "icons/iconw_gauntlet",
    WP_NONE,                //weapon_t weaponAmmo;
    0,                      //int   ammo;
    0,                      //int   clips;
    WUT_HUMANS              //WUTeam_t  team;
  },
  {
    UP_JETPACK,             //int   upgradeNum;
    JETPACK_PRICE,          //int   price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_BACKPACK,          //int   slots;
    "jetpack",              //char  *upgradeName;
    "Jet Pack",             //char  *upgradeHumanName;
    "icons/iconw_gauntlet",
    WP_NONE,                //weapon_t weaponAmmo;
    0,                      //int   ammo;
    0,                      //int   clips;
    WUT_HUMANS              //WUTeam_t  team;
  },
  {
    UP_BATTLESUIT,                            //int   upgradeNum;
    BSUIT_PRICE,                              //int   price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ),      //int  stages
    SLOT_HEAD|SLOT_TORSO|SLOT_ARMS|SLOT_LEGS, //int   slots;
    "bsuit",                                  //char  *upgradeName;
    "Battlesuit",                             //char  *upgradeHumanName;
    "icons/iconw_gauntlet",
    WP_NONE,                //weapon_t weaponAmmo;
    0,                      //int   ammo;
    0,                      //int   clips;
    WUT_HUMANS              //WUTeam_t  team;
  },
  {
    UP_MGCLIP,              //int   upgradeNum;
    MGCLIP_PRICE,           //int   price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_NONE,              //int   slots;
    "mgclip",               //char  *upgradeName;
    "1 Rifle Clip",         //char  *upgradeHumanName;
    "icons/iconw_gauntlet",
    WP_MACHINEGUN,          //weapon_t weaponAmmo;
    0,                      //int   ammo;
    1,                      //int   clips;
    WUT_HUMANS              //WUTeam_t  team;
  },
  {
    UP_CGAMMO,              //int   upgradeNum;
    CGAMMO_PRICE,           //int   price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_NONE,              //int   slots;
    "cgammo",               //char  *upgradeName;
    "Chaingun bullets",     //char  *upgradeHumanName;
    "icons/iconw_gauntlet",
    WP_CHAINGUN,            //weapon_t weaponAmmo;
    100,                    //int   ammo;
    0,                      //int   clips;
    WUT_HUMANS              //WUTeam_t  team;
  },
  {
    UP_GAS,                 //int   upgradeNum;
    GAS_PRICE,              //int   price;
    ( 1 << S1 )|( 1 << S2 )|( 1 << S3 ), //int  stages
    SLOT_NONE,              //int   slots;
    "gas",                  //char  *upgradeName;
    "Flamer gas",           //char  *upgradeHumanName;
    "icons/iconw_gauntlet",
    WP_FLAMER,              //weapon_t weaponAmmo;
    200,                    //int   ammo;
    0,                      //int   clips;
    WUT_HUMANS              //WUTeam_t  team;
  }
};

int   bg_numUpgrades = sizeof( bg_upgrades ) / sizeof( bg_upgrades[ 0 ] );

/*
==============
BG_FindPriceForUpgrade
==============
*/
int BG_FindPriceForUpgrade( int upgrade )
{
  int i;

  for( i = 0; i < bg_numUpgrades; i++ )
  {
    if( bg_upgrades[ i ].upgradeNum == upgrade )
    {
      return bg_upgrades[ i ].price;
    }
  }
  
  return 100;
}

/*
==============
BG_FindStagesForUpgrade
==============
*/
qboolean BG_FindStagesForUpgrade( int upgrade, stage_t stage )
{
  int i;

  for( i = 0; i < bg_numUpgrades; i++ )
  {
    if( bg_upgrades[ i ].upgradeNum == upgrade )
    {
      if( bg_upgrades[ i ].stages & ( 1 << stage ) )
        return qtrue;
      else
        return qfalse;
    }
  }
  
  return qfalse;
}

/*
==============
BG_FindSlotsForUpgrade
==============
*/
int BG_FindSlotsForUpgrade( int upgrade )
{
  int i;

  for( i = 0; i < bg_numUpgrades; i++ )
  {
    if( bg_upgrades[ i ].upgradeNum == upgrade )
    {
      return bg_upgrades[ i ].slots;
    }
  }
  
  return SLOT_NONE;
}

/*
==============
BG_FindNameForUpgrade
==============
*/
char *BG_FindNameForUpgrade( int upgrade )
{
  int i;

  for( i = 0; i < bg_numUpgrades; i++ )
  {
    if( bg_upgrades[ i ].upgradeNum == upgrade )
      return bg_upgrades[ i ].upgradeName;
  }

  //wimp out
  return 0;
}

/*
==============
BG_FindUpgradeNumForName
==============
*/
int BG_FindUpgradeNumForName( char *name )
{
  int i;

  for( i = 0; i < bg_numUpgrades; i++ )
  {
    if( !Q_stricmp( bg_upgrades[ i ].upgradeName, name ) )
      return bg_upgrades[ i ].upgradeNum;
  }

  //wimp out
  return UP_NONE;
}

/*
==============
BG_FindHumanNameForUpgrade
==============
*/
char *BG_FindHumanNameForUpgrade( int upgrade )
{
  int i;

  for( i = 0; i < bg_numUpgrades; i++ )
  {
    if( bg_upgrades[ i ].upgradeNum == upgrade )
      return bg_upgrades[ i ].upgradeHumanName;
  }

  //wimp out
  return 0;
}

/*
==============
BG_FindIconForUpgrade
==============
*/
char *BG_FindIconForUpgrade( int upgrade )
{
  int i;

  for( i = 0; i < bg_numUpgrades; i++ )
  {
    if( bg_upgrades[ i ].upgradeNum == upgrade )
      return bg_upgrades[ i ].icon;
  }

  //wimp out
  return 0;
}

/*
==============
BG_FindWeaponAmmoForUpgrade
==============
*/
weapon_t BG_FindWeaponAmmoForUpgrade( int upgrade )
{
  int i;

  for( i = 0; i < bg_numUpgrades; i++ )
  {
    if( bg_upgrades[ i ].upgradeNum == upgrade )
      return bg_upgrades[ i ].weaponAmmo;
  }

  return WP_NONE;
}

/*
==============
BG_FindAmmoForUpgrade
==============
*/
void BG_FindAmmoForUpgrade( int upgrade, int *ammo, int *clips )
{
  int i;

  for( i = 0; i < bg_numUpgrades; i++ )
  {
    if( bg_upgrades[ i ].upgradeNum == upgrade )
    {
      if( ammo != NULL )
        *ammo = bg_upgrades[ i ].ammo;

      if( clips != NULL )
        *clips = bg_upgrades[ i ].clips;
    }
  }
}

/*
==============
BG_FindTeamForUpgrade
==============
*/
WUTeam_t BG_FindTeamForUpgrade( int upgrade )
{
  int i;

  for( i = 0; i < bg_numUpgrades; i++ )
  {
    if( bg_upgrades[ i ].upgradeNum == upgrade )
    {
      return bg_upgrades[ i ].team;
    }
  }

  return WUT_NONE;
}

////////////////////////////////////////////////////////////////////////////////

/*
================
BG_EvaluateTrajectory

================
*/
void BG_EvaluateTrajectory( const trajectory_t *tr, int atTime, vec3_t result )
{
  float   deltaTime;
  float   phase;

  switch( tr->trType )
  {
    case TR_STATIONARY:
    case TR_INTERPOLATE:
      VectorCopy( tr->trBase, result );
      break;
      
    case TR_LINEAR:
      deltaTime = ( atTime - tr->trTime ) * 0.001;  // milliseconds to seconds
      VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
      break;
      
    case TR_SINE:
      deltaTime = ( atTime - tr->trTime ) / (float)tr->trDuration;
      phase = sin( deltaTime * M_PI * 2 );
      VectorMA( tr->trBase, phase, tr->trDelta, result );
      break;
      
    case TR_LINEAR_STOP:
      if( atTime > tr->trTime + tr->trDuration )
        atTime = tr->trTime + tr->trDuration;

      deltaTime = ( atTime - tr->trTime ) * 0.001;  // milliseconds to seconds
      if( deltaTime < 0 )
        deltaTime = 0;
      
      VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
      break;
      
    case TR_GRAVITY:
      deltaTime = ( atTime - tr->trTime ) * 0.001;  // milliseconds to seconds
      VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
      result[ 2 ] -= 0.5 * DEFAULT_GRAVITY * deltaTime * deltaTime;   // FIXME: local gravity...
      break;
      
    case TR_BUOYANCY:
      deltaTime = ( atTime - tr->trTime ) * 0.001;  // milliseconds to seconds
      VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
      result[ 2 ] += 0.5 * DEFAULT_GRAVITY * deltaTime * deltaTime;   // FIXME: local gravity...
      break;
      
    default:
      Com_Error( ERR_DROP, "BG_EvaluateTrajectory: unknown trType: %i", tr->trTime );
      break;
  }
}

/*
================
BG_EvaluateTrajectoryDelta

For determining velocity at a given time
================
*/
void BG_EvaluateTrajectoryDelta( const trajectory_t *tr, int atTime, vec3_t result )
{
  float deltaTime;
  float phase;

  switch( tr->trType )
  {
    case TR_STATIONARY:
    case TR_INTERPOLATE:
      VectorClear( result );
      break;
      
    case TR_LINEAR:
      VectorCopy( tr->trDelta, result );
      break;
      
    case TR_SINE:
      deltaTime = ( atTime - tr->trTime ) / (float)tr->trDuration;
      phase = cos( deltaTime * M_PI * 2 );  // derivative of sin = cos
      phase *= 0.5;
      VectorScale( tr->trDelta, phase, result );
      break;
      
    case TR_LINEAR_STOP:
      if( atTime > tr->trTime + tr->trDuration )
      {
        VectorClear( result );
        return;
      }
      VectorCopy( tr->trDelta, result );
      break;
      
    case TR_GRAVITY:
      deltaTime = ( atTime - tr->trTime ) * 0.001;  // milliseconds to seconds
      VectorCopy( tr->trDelta, result );
      result[ 2 ] -= DEFAULT_GRAVITY * deltaTime;   // FIXME: local gravity...
      break;
      
    case TR_BUOYANCY:
      deltaTime = ( atTime - tr->trTime ) * 0.001;  // milliseconds to seconds
      VectorCopy( tr->trDelta, result );
      result[ 2 ] += DEFAULT_GRAVITY * deltaTime;   // FIXME: local gravity...
      break;
      
    default:
      Com_Error( ERR_DROP, "BG_EvaluateTrajectoryDelta: unknown trType: %i", tr->trTime );
      break;
  }
}

char *eventnames[ ] =
{
  "EV_NONE",

  "EV_FOOTSTEP",
  "EV_FOOTSTEP_METAL",
  "EV_FOOTSTEP_SQUELCH",
  "EV_FOOTSPLASH",
  "EV_FOOTWADE",
  "EV_SWIM",

  "EV_STEP_4",
  "EV_STEP_8",
  "EV_STEP_12",
  "EV_STEP_16",

  "EV_STEPDN_4",
  "EV_STEPDN_8",
  "EV_STEPDN_12",
  "EV_STEPDN_16",

  "EV_FALL_SHORT",
  "EV_FALL_MEDIUM",
  "EV_FALL_FAR",

  "EV_JUMP_PAD",      // boing sound at origin", jump sound on player

  "EV_JUMP",
  "EV_WATER_TOUCH", // foot touches
  "EV_WATER_LEAVE", // foot leaves
  "EV_WATER_UNDER", // head touches
  "EV_WATER_CLEAR", // head leaves

  "EV_NOAMMO",
  "EV_CHANGE_WEAPON",
  "EV_NEXT_WEAPON",
  "EV_FIRE_WEAPON",
  "EV_FIRE_WEAPON2",
  "EV_FIRE_WEAPON3",

  "EV_PLAYER_RESPAWN", //TA: for fovwarp effects
  "EV_PLAYER_TELEPORT_IN",
  "EV_PLAYER_TELEPORT_OUT",

  "EV_GRENADE_BOUNCE",    // eventParm will be the soundindex

  "EV_GENERAL_SOUND",
  "EV_GLOBAL_SOUND",    // no attenuation

  "EV_BULLET_HIT_FLESH",
  "EV_BULLET_HIT_WALL",
  "EV_LAS_HIT_FLESH",
  "EV_LAS_HIT_WALL",
  "EV_MASS_DRIVER_HIT",

  "EV_MISSILE_HIT",
  "EV_MISSILE_MISS",
  "EV_MISSILE_MISS_METAL",
  "EV_BUILDABLE_EXPLOSION", //TA: human item explosions
  "EV_TESLATRAIL",
  "EV_ALIENZAP",
  "EV_BULLET",        // otherEntity is the shooter

  "EV_PAIN",
  "EV_DEATH1",
  "EV_DEATH2",
  "EV_DEATH3",
  "EV_OBITUARY",

  "EV_GIB_PLAYER",      // gib a previously living player
  "EV_GIB_ALIEN",       //TA: generic green gib for aliens

  "EV_BUILD_CONSTRUCT", //TA
  "EV_BUILD_DESTROY",   //TA
  "EV_BUILD_ANIM",      //TA

  "EV_DEBUG_LINE",
  "EV_STOPLOOPINGSOUND",
  "EV_TAUNT",

  "EV_MENU",            //TA: menu event
  "EV_BUILD_DELAY",     //TA: can't build yet
  "EV_POISONCLOUD",     //TA: client poisoned
  "EV_KNOCKOVER",       //TA: client knocked over
  "EV_GETUP"            //TA: client getting up
};

/*
===============
BG_AddPredictableEventToPlayerstate

Handles the sequence numbers
===============
*/

void  trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

void BG_AddPredictableEventToPlayerstate( int newEvent, int eventParm, playerState_t *ps )
{
#ifdef _DEBUG
  {
    char buf[ 256 ];
    trap_Cvar_VariableStringBuffer( "showevents", buf, sizeof( buf ) );
    
    if( atof( buf ) != 0 )
    {
#ifdef QAGAME
      Com_Printf( " game event svt %5d -> %5d: num = %20s parm %d\n",
                  ps->pmove_framecount/*ps->commandTime*/, ps->eventSequence, eventnames[ newEvent ], eventParm);
#else
      Com_Printf( "Cgame event svt %5d -> %5d: num = %20s parm %d\n",
                  ps->pmove_framecount/*ps->commandTime*/, ps->eventSequence, eventnames[ newEvent ], eventParm);
#endif
    }
  }
#endif
  ps->events[ ps->eventSequence & ( MAX_PS_EVENTS - 1 ) ] = newEvent;
  ps->eventParms[ ps->eventSequence & ( MAX_PS_EVENTS - 1 ) ] = eventParm;
  ps->eventSequence++;
}


/*
========================
BG_TouchJumpPad
========================
*/
void BG_TouchJumpPad( playerState_t *ps, entityState_t *jumppad )
{
  vec3_t  angles;
  float   p;
  int     effectNum;

  // spectators don't use jump pads
  if( ps->pm_type != PM_NORMAL )
    return;

  // if we didn't hit this same jumppad the previous frame
  // then don't play the event sound again if we are in a fat trigger
  if( ps->jumppad_ent != jumppad->number )
  {
    vectoangles( jumppad->origin2, angles);
    
    p = fabs( AngleNormalize180( angles[ PITCH ] ) );
    
    if( p < 45 )
      effectNum = 0;
    else
      effectNum = 1;
    
    BG_AddPredictableEventToPlayerstate( EV_JUMP_PAD, effectNum, ps );
  }
  
  // remember hitting this jumppad this frame
  ps->jumppad_ent = jumppad->number;
  ps->jumppad_frame = ps->pmove_framecount;
  // give the player the velocity from the jumppad
  VectorCopy( jumppad->origin2, ps->velocity );
}


/*
========================
BG_PlayerStateToEntityState

This is done after each set of usercmd_t on the server,
and after local prediction on the client
========================
*/
void BG_PlayerStateToEntityState( playerState_t *ps, entityState_t *s, qboolean snap )
{
  int     i;
  vec3_t  ceilingNormal = { 0, 0, -1 };

  if( ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPECTATOR || ps->pm_type == PM_FREEZE )
    s->eType = ET_INVISIBLE;
  else if ( ps->stats[STAT_HEALTH] <= GIB_HEALTH )
    s->eType = ET_INVISIBLE;
  else
    s->eType = ET_PLAYER;

  s->number = ps->clientNum;

  s->pos.trType = TR_INTERPOLATE;
  VectorCopy( ps->origin, s->pos.trBase );
  
  if( snap )
    SnapVector( s->pos.trBase );
  
  //set the trDelta for flag direction
  VectorCopy( ps->velocity, s->pos.trDelta );

  s->apos.trType = TR_INTERPOLATE;
  VectorCopy( ps->viewangles, s->apos.trBase );
  
  if( snap )
    SnapVector( s->apos.trBase );

  //TA: i need for other things :)
  //s->angles2[YAW] = ps->movementDir;
  s->time2 = ps->movementDir;
  s->legsAnim = ps->legsAnim;
  s->torsoAnim = ps->torsoAnim;
  s->clientNum = ps->clientNum;   // ET_PLAYER looks here instead of at number
                    // so corpses can also reference the proper config
  s->eFlags = ps->eFlags;
  if( ps->stats[STAT_HEALTH] <= 0 )
    s->eFlags |= EF_DEAD;
  else
    s->eFlags &= ~EF_DEAD;

  if( ps->externalEvent )
  {
    s->event = ps->externalEvent;
    s->eventParm = ps->externalEventParm;
  }
  else if( ps->entityEventSequence < ps->eventSequence )
  {
    int   seq;

    if( ps->entityEventSequence < ps->eventSequence - MAX_PS_EVENTS )
      ps->entityEventSequence = ps->eventSequence - MAX_PS_EVENTS;

    seq = ps->entityEventSequence & ( MAX_PS_EVENTS - 1 );
    s->event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );
    s->eventParm = ps->eventParms[ seq ];
    ps->entityEventSequence++;
  }

  s->weapon = ps->weapon;
  s->groundEntityNum = ps->groundEntityNum;

  //store items held and active items in otherEntityNum
  s->modelindex = 0;
  s->modelindex2 = 0;
  for( i = UP_NONE + 1; i < UP_NUM_UPGRADES; i++ )
  {
    if( BG_gotItem( i, ps->stats ) )
    {
      s->modelindex |= 1 << i;

      if( BG_activated( i, ps->stats ) )
        s->modelindex2 |= 1 << i;
    }
  }

  //TA: use powerups field to store team/class info:
  s->powerups = ps->stats[ STAT_PTEAM ] | ( ps->stats[ STAT_PCLASS ] << 8 );

  //TA: have to get the surfNormal thru somehow...
  VectorCopy( ps->grapplePoint, s->angles2 );
  if( ps->stats[ STAT_STATE ] & SS_WALLCLIMBINGCEILING )
    s->eFlags |= EF_WALLCLIMBCEILING;

  s->loopSound = ps->loopSound;
  s->generic1 = ps->generic1;
}


/*
========================
BG_PlayerStateToEntityStateExtraPolate

This is done after each set of usercmd_t on the server,
and after local prediction on the client
========================
*/
void BG_PlayerStateToEntityStateExtraPolate( playerState_t *ps, entityState_t *s, int time, qboolean snap )
{
  int     i;
  vec3_t  ceilingNormal = { 0, 0, -1 };

  if( ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPECTATOR || ps->pm_type == PM_FREEZE )
    s->eType = ET_INVISIBLE;
  else if( ps->stats[STAT_HEALTH] <= GIB_HEALTH )
    s->eType = ET_INVISIBLE;
  else
    s->eType = ET_PLAYER;

  s->number = ps->clientNum;

  s->pos.trType = TR_LINEAR_STOP;
  VectorCopy( ps->origin, s->pos.trBase );

  if( snap )
    SnapVector( s->pos.trBase );

  // set the trDelta for flag direction and linear prediction
  VectorCopy( ps->velocity, s->pos.trDelta );
  // set the time for linear prediction
  s->pos.trTime = time;
  // set maximum extra polation time
  s->pos.trDuration = 50; // 1000 / sv_fps (default = 20)

  s->apos.trType = TR_INTERPOLATE;
  VectorCopy( ps->viewangles, s->apos.trBase );
  if( snap )
    SnapVector( s->apos.trBase );

  //TA: i need for other things :)
  //s->angles2[YAW] = ps->movementDir;
  s->time2 = ps->movementDir;
  s->legsAnim = ps->legsAnim;
  s->torsoAnim = ps->torsoAnim;
  s->clientNum = ps->clientNum;   // ET_PLAYER looks here instead of at number
                    // so corpses can also reference the proper config
  s->eFlags = ps->eFlags;
  
  if( ps->stats[STAT_HEALTH] <= 0 )
    s->eFlags |= EF_DEAD;
  else
    s->eFlags &= ~EF_DEAD;

  if( ps->externalEvent )
  {
    s->event = ps->externalEvent;
    s->eventParm = ps->externalEventParm;
  }
  else if( ps->entityEventSequence < ps->eventSequence )
  {
    int   seq;

    if( ps->entityEventSequence < ps->eventSequence - MAX_PS_EVENTS )
      ps->entityEventSequence = ps->eventSequence - MAX_PS_EVENTS;

    seq = ps->entityEventSequence & ( MAX_PS_EVENTS - 1 );
    s->event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );
    s->eventParm = ps->eventParms[ seq ];
    ps->entityEventSequence++;
  }

  s->weapon = ps->weapon;
  s->groundEntityNum = ps->groundEntityNum;

  //store items held and active items in otherEntityNum
  s->modelindex = 0;
  s->modelindex2 = 0;
  
  for( i = UP_NONE + 1; i < UP_NUM_UPGRADES; i++ )
  {
    if( BG_gotItem( i, ps->stats ) )
    {
      s->modelindex |= 1 << i;

      if( BG_activated( i, ps->stats ) )
        s->modelindex2 |= 1 << i;
    }
  }

  //TA: use powerups field to store team/class info:
  s->powerups = ps->stats[ STAT_PTEAM ] | ( ps->stats[ STAT_PCLASS ] << 8 );

  //TA: have to get the surfNormal thru somehow...
  VectorCopy( ps->grapplePoint, s->angles2 );
  if( ps->stats[ STAT_STATE ] & SS_WALLCLIMBINGCEILING )
    s->eFlags |= EF_WALLCLIMBCEILING;

  s->loopSound = ps->loopSound;
  s->generic1 = ps->generic1;
}

//TA: extract the ammo quantity from the array
void BG_unpackAmmoArray( int weapon, int ammo[ ], int ammo2[ ], int *quan, int *clips, int *maxclips )
{
  int   ammoarray[32];
  int   i;
  
  for( i = 0; i <= 15; i++ )
    ammoarray[ i ] = ammo[ i ];

  for( i = 16; i <= 31; i++ )
    ammoarray[ i ] = ammo2[ i - 16 ];

  if( quan != NULL )
    *quan = ammoarray[ weapon ] & 0x03FF;

  if( clips != NULL )
    *clips = ( ammoarray[ weapon ] >> 10 ) & 0x07;

  if( maxclips != NULL )
    *maxclips = ( ammoarray[ weapon ] >> 13 ) & 0x07;
}

//TA: pack the ammo quantity into the array
void BG_packAmmoArray( int weapon, int ammo[ ], int ammo2[ ], int quan, int clips, int maxclips )
{
  int   weaponvalue;

  weaponvalue = quan | ( clips << 10 ) | ( maxclips << 13 );

  if( weapon <= 15 )
    ammo[ weapon ] = weaponvalue;
  else if( weapon >= 16 )
    ammo2[ weapon - 16 ] = weaponvalue;
}

//TA: pack weapons into the array
void BG_packWeapon( int weapon, int stats[ ] )
{
  int  weaponList;

  weaponList = ( stats[ STAT_WEAPONS ] & 0x0000FFFF ) | ( ( stats[ STAT_WEAPONS2 ] << 16 ) & 0xFFFF0000 );

  weaponList |= ( 1 << weapon );
  
  stats[ STAT_WEAPONS ] = weaponList & 0x0000FFFF;
  stats[ STAT_WEAPONS2 ] = ( weaponList & 0xFFFF0000 ) >> 16;

  if( stats[ STAT_SLOTS ] & BG_FindSlotsForWeapon( weapon ) )
    Com_Printf( S_COLOR_YELLOW "WARNING: Held items conflict with weapon %d\n", weapon );

  stats[ STAT_SLOTS ] |= BG_FindSlotsForWeapon( weapon );
}

//TA: remove weapons from the array
void BG_removeWeapon( int weapon, int stats[ ] )
{
  int  weaponList;

  weaponList = ( stats[ STAT_WEAPONS ] & 0x0000FFFF ) | ( ( stats[ STAT_WEAPONS2 ] << 16 ) & 0xFFFF0000 );

  weaponList &= ~( 1 << weapon );
  
  stats[ STAT_WEAPONS ] = weaponList & 0x0000FFFF;
  stats[ STAT_WEAPONS2 ] = ( weaponList & 0xFFFF0000 ) >> 16;
  
  stats[ STAT_SLOTS ] &= ~BG_FindSlotsForWeapon( weapon );
}

//TA: check whether array contains weapon
qboolean BG_gotWeapon( int weapon, int stats[ ] )
{
  int  weaponList;

  weaponList = ( stats[ STAT_WEAPONS ] & 0x0000FFFF ) | ( ( stats[ STAT_WEAPONS2 ] << 16 ) & 0xFFFF0000 );

  return( weaponList & ( 1 << weapon ) );
}

//TA: pack items into array
void BG_packItem( int item, int stats[ ] )
{
  stats[ STAT_ITEMS ] |= ( 1 << item );

  if( stats[ STAT_SLOTS ] & BG_FindSlotsForUpgrade( item ) )
    Com_Printf( S_COLOR_YELLOW "WARNING: Held items conflict with upgrade %d\n", item );
    
  stats[ STAT_SLOTS ] |= BG_FindSlotsForUpgrade( item );
}

//TA: remove items from array
void BG_removeItem( int item, int stats[ ] )
{
  stats[ STAT_ITEMS ] &= ~( 1 << item );

  stats[ STAT_SLOTS ] &= ~BG_FindSlotsForUpgrade( item );
}

//TA: check if item is in array
qboolean BG_gotItem( int item, int stats[ ] )
{
  return( stats[ STAT_ITEMS ] & ( 1 << item ) );
}

//TA: set item active in array
void BG_activateItem( int item, int stats[ ] )
{
  stats[ STAT_ACTIVEITEMS ] |= ( 1 << item );
}

//TA: set item deactive in array
void BG_deactivateItem( int item, int stats[ ] )
{
  stats[ STAT_ACTIVEITEMS ] &= ~( 1 << item );
}

//TA: check if item active in array
qboolean BG_activated( int item, int stats[ ] )
{
  return( stats[ STAT_ACTIVEITEMS ] & ( 1 << item ) );
}

qboolean BG_rotateAxis( vec3_t surfNormal, vec3_t inAxis[ 3 ],
                        vec3_t outAxis[ 3 ], qboolean inverse, qboolean ceiling )
{
  vec3_t  refNormal = { 0.0f, 0.0f, 1.0f };
  vec3_t  ceilingNormal = { 0.0f, 0.0f, -1.0f };
  vec3_t  localNormal, xNormal;
  float   rotAngle;
  
  //the grapplePoint being a surfNormal rotation Normal hack... see above :)
  if( ceiling )
  {
    VectorCopy( ceilingNormal, localNormal );
    VectorCopy( surfNormal, xNormal );
  }
  else
  {
    //cross the reference normal and the surface normal to get the rotation axis
    VectorCopy( surfNormal, localNormal );
    CrossProduct( localNormal, refNormal, xNormal );
    VectorNormalize( xNormal );
  }

  //if we're a wall climber.. and we're climbing rotate the axis
  if( VectorLength( xNormal ) != 0.0f )
  {
    //if the normal pointing straight down then the rotAngle will always be 180deg
    if( surfNormal[ 2 ] == -1.0f )
      rotAngle = 180.0f;
    else
      rotAngle = RAD2DEG( acos( DotProduct( localNormal, refNormal ) ) );

    if( inverse )
      rotAngle = -rotAngle;

    AngleNormalize180( rotAngle );

    //hmmm could get away with only one rotation and some clever stuff later... but i'm lazy
    RotatePointAroundVector( outAxis[ 0 ], xNormal, inAxis[ 0 ], -rotAngle );
    RotatePointAroundVector( outAxis[ 1 ], xNormal, inAxis[ 1 ], -rotAngle );
    RotatePointAroundVector( outAxis[ 2 ], xNormal, inAxis[ 2 ], -rotAngle );
  }
  else
    return qfalse;

  return qtrue;
}
