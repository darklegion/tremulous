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
 

/*
 * ALIEN weapons
 *
 * _REPEAT  - time in msec until the weapon can be used again
 * _DMG     - amount of damage the weapon does
 *
 * ALIEN_WDMG_MODIFIER - overall damage modifier for coarse tuning
 * 
 */

#define ALIEN_WDMG_MODIFIER         0.80f
#define ADM(d)                      ((int)((float)d*ALIEN_WDMG_MODIFIER))

#define ABUILDER_BUILD_REPEAT       500
#define ABUILDER_CLAW_DMG           ADM(25)
#define ABUILDER_CLAW_RANGE         64.0f
#define ABUILDER_CLAW_WIDTH         4.0f
#define ABUILDER_CLAW_REPEAT        1000
#define ABUILDER_BASE_DELAY         9000
#define ABUILDER_ADV_DELAY          7000
#define ABUILDER_BLOB_DMG           ADM(5)
#define ABUILDER_BLOB_REPEAT        1000
#define ABUILDER_BLOB_SPEED         800.0f
#define ABUILDER_BLOB_SPEED_MOD     0.5f
#define ABUILDER_BLOB_TIME          5000

#define SOLDIER_BITE_DMG            ADM(60)
#define SOLDIER_BITE_RANGE          64.0f
#define SOLDIER_BITE_WIDTH          6.0f
#define SOLDIER_BITE_REPEAT         500

#define HYDRA_CLAW_DMG              ADM(40)
#define HYDRA_CLAW_RANGE            96.0f
#define HYDRA_CLAW_WIDTH            10.0f
#define HYDRA_CLAW_REPEAT           600
#define HYDRA_CLAW_U_REPEAT         500
#define HYDRA_GRAB_RANGE            64.0f
#define HYDRA_GRAB_TIME             1000
#define HYDRA_PCLOUD_DMG            ADM(5)
#define HYDRA_PCLOUD_RANGE          200.0f
#define HYDRA_PCLOUD_REPEAT         2000
#define HYDRA_PCLOUD_TIME           10000

#define CHIMERA_CLAW_DMG            ADM(50)
#define CHIMERA_CLAW_RANGE          96.0f
#define CHIMERA_CLAW_WIDTH          12.0f
#define CHIMERA_CLAW_REPEAT         500
#define CHIMERA_CLAW_U_REPEAT       400
#define CHIMERA_AREAZAP_DMG         ADM(75)
#define CHIMERA_AREAZAP_RANGE       200.0f
#define CHIMERA_AREAZAP_REPEAT      1500
#define CHIMERA_WALLJUMP_MAXSPEED   1000.0f

#define DRAGOON_CLAW_DMG            ADM(100)
#define DRAGOON_CLAW_RANGE          96.0f
#define DRAGOON_CLAW_WIDTH          16.0f
#define DRAGOON_CLAW_REPEAT         700
#define DRAGOON_CLAW_U_REPEAT       600
#define DRAGOON_POUNCE_DMG          ADM(200)
#define DRAGOON_POUNCE_RANGE        96.0f
#define DRAGOON_POUNCE_WIDTH        16.0f
#define DRAGOON_POUNCE_SPEED        700
#define DRAGOON_POUNCE_UPG_SPEED    800
#define DRAGOON_POUNCE_SPEED_MOD    0.75f
#define DRAGOON_POUNCE_TIME         700
#define DRAGOON_BOUNCEBALL_DMG      ADM(100)
#define DRAGOON_BOUNCEBALL_REPEAT   1000
#define DRAGOON_BOUNCEBALL_SPEED    1000.0f

#define BMOFO_CLAW_DMG              ADM(150)
#define BMOFO_CLAW_RANGE            128.0f
#define BMOFO_CLAW_WIDTH            20.0f
#define BMOFO_CLAW_REPEAT           750
#define BMOFO_REGEN_RANGE           200.0f
#define BMOFO_REGEN_MOD             2.0f
#define BMOFO_CHARGE_SPEED          2.0f
#define BMOFO_CHARGE_TIME           3000
#define BMOFO_CHARGE_CHARGE_TIME    1000
#define BMOFO_CHARGE_CHARGE_RATIO   (BMOFO_CHARGE_TIME/BMOFO_CHARGE_CHARGE_TIME)
#define BMOFO_CHARGE_REPEAT         500
#define BMOFO_CHARGE_DMG            ADM(200)



/*
 * ALIEN classes
 *
 * _SPEED   - fraction of Q3A run speed the class can move
 * _TTE     - time in msec for the class to evolve to another
 * _REGEN   - health per second regained
 *
 * ALIEN_HLTH_MODIFIER - overall health modifier for coarse tuning
 * 
 */

#define ALIEN_HLTH_MODIFIER         1.0f
#define AHM(h)                      ((int)((float)h*ALIEN_HLTH_MODIFIER))

#define ALIEN_VALUE_MODIFIER        1.0f
#define AVM(h)                      ((int)((float)h*ALIEN_VALUE_MODIFIER))

#define ABUILDER_SPEED              0.8f
#define ABUILDER_VALUE              AVM(200)
#define ABUILDER_HEALTH             AHM(50)
#define ABUILDER_REGEN              2
#define ABUILDER_COST               0

#define ABUILDER_UPG_SPEED          1.0f
#define ABUILDER_UPG_VALUE          AVM(250)
#define ABUILDER_UPG_HEALTH         AHM(75)
#define ABUILDER_UPG_REGEN          3
#define ABUILDER_UPG_COST           0

#define SOLDIER_SPEED               1.3f
#define SOLDIER_VALUE               AVM(175)
#define SOLDIER_HEALTH              AHM(25)
#define SOLDIER_REGEN               1
#define SOLDIER_COST                0

#define HYDRA_SPEED                 1.25f
#define HYDRA_VALUE                 AVM(225)
#define HYDRA_HEALTH                AHM(75)
#define HYDRA_REGEN                 2
#define HYDRA_COST                  1

#define HYDRA_UPG_SPEED             1.25f
#define HYDRA_UPG_VALUE             AVM(275)
#define HYDRA_UPG_HEALTH            AHM(100)
#define HYDRA_UPG_REGEN             2
#define HYDRA_UPG_COST              1

#define CHIMERA_SPEED               1.2f
#define CHIMERA_VALUE               AVM(350)
#define CHIMERA_HEALTH              AHM(150)
#define CHIMERA_REGEN               3
#define CHIMERA_COST                1

#define CHIMERA_UPG_SPEED           1.2f
#define CHIMERA_UPG_VALUE           AVM(450)
#define CHIMERA_UPG_HEALTH          AHM(175)
#define CHIMERA_UPG_REGEN           4
#define CHIMERA_UPG_COST            1

#define DRAGOON_SPEED               1.2f
#define DRAGOON_VALUE               AVM(500)
#define DRAGOON_HEALTH              AHM(250)
#define DRAGOON_REGEN               5
#define DRAGOON_COST                1

#define DRAGOON_UPG_SPEED           1.2f
#define DRAGOON_UPG_VALUE           AVM(600)
#define DRAGOON_UPG_HEALTH          AHM(300)
#define DRAGOON_UPG_REGEN           6
#define DRAGOON_UPG_COST            1

#define BMOFO_SPEED                 1.15f
#define BMOFO_VALUE                 AVM(800)
#define BMOFO_HEALTH                AHM(400)
#define BMOFO_REGEN                 7
#define BMOFO_COST                  2



/*
 * ALIEN buildables
 *
 * _BP            - build points required for this buildable
 * _BT            - build time required for this buildable
 * _REGEN         - the amount of health per second regained
 * _SPLASHDAMGE   - the amount of damage caused by this buildable when melting
 * _SPLASHRADIUS  - the radius around which it does this damage
 *
 * CREEP_BASESIZE - the maximum distance a buildable can be from an egg/overmind
 * ALIEN_BHLTH_MODIFIER - overall health modifier for coarse tuning
 * 
 */

#define ALIEN_BHLTH_MODIFIER        0.5f
#define ABHM(h)                     ((int)((float)h*ALIEN_BHLTH_MODIFIER))

#define CREEP_BASESIZE              700 
#define CREEP_TIMEOUT               1000
#define CREEP_MODIFIER              0.5f
#define CREEP_ARMOUR_MODIFIER       0.75f
#define CREEP_SCALEDOWN_TIME        3000

#define ASPAWN_BP                   10
#define ASPAWN_BT                   15000
#define ASPAWN_HEALTH               ABHM(500)
#define ASPAWN_REGEN                8
#define ASPAWN_SPLASHDAMAGE         50
#define ASPAWN_SPLASHRADIUS         50
#define ASPAWN_CREEPSIZE            120
#define ASPAWN_VALUE                150

#define BARRICADE_BP                10
#define BARRICADE_BT                20000
#define BARRICADE_HEALTH            ABHM(400)
#define BARRICADE_REGEN             12
#define BARRICADE_SPLASHDAMAGE      50
#define BARRICADE_SPLASHRADIUS      50
#define BARRICADE_CREEPSIZE         120

#define BOOSTER_BP                  12
#define BOOSTER_BT                  15000
#define BOOSTER_HEALTH              ABHM(300)
#define BOOSTER_REGEN               8
#define BOOSTER_SPLASHDAMAGE        50
#define BOOSTER_SPLASHRADIUS        50
#define BOOSTER_CREEPSIZE           120
#define BOOSTER_INTERVAL            30000 //time in msec between uses (per player)

#define ACIDTUBE_BP                 8
#define ACIDTUBE_BT                 15000
#define ACIDTUBE_HEALTH             ABHM(200)
#define ACIDTUBE_REGEN              8
#define ACIDTUBE_SPLASHDAMAGE       30
#define ACIDTUBE_SPLASHRADIUS       300
#define ACIDTUBE_CREEPSIZE          120
#define ACIDTUBE_RANGE              300.0f
#define ACIDTUBE_REPEAT             3000

#define HIVE_BP                     12
#define HIVE_BT                     20000
#define HIVE_HEALTH                 ABHM(200)
#define HIVE_REGEN                  8
#define HIVE_SPLASHDAMAGE           30
#define HIVE_SPLASHRADIUS           200
#define HIVE_CREEPSIZE              120
#define HIVE_RANGE                  400.0f
#define HIVE_REPEAT                 10000
#define HIVE_DMG                    20
#define HIVE_SPEED                  230.0f
#define HIVE_DIR_CHANGE_PERIOD      500

#define TRAPPER_BP                  10
#define TRAPPER_BT                  12000
#define TRAPPER_HEALTH              ABHM(80)
#define TRAPPER_REGEN               6
#define TRAPPER_SPLASHDAMAGE        15
#define TRAPPER_SPLASHRADIUS        100
#define TRAPPER_CREEPSIZE           30
#define TRAPPER_RANGE               400
#define TRAPPER_REPEAT              1000
#define LOCKBLOB_SPEED              500
#define LOCKBLOB_DOT                0.85f // max angle = acos( LOCKBLOB_DOT )

#define OVERMIND_BP                 0
#define OVERMIND_BT                 30000
#define OVERMIND_HEALTH             ABHM(1000)
#define OVERMIND_REGEN              10
#define OVERMIND_SPLASHDAMAGE       15
#define OVERMIND_SPLASHRADIUS       300
#define OVERMIND_CREEPSIZE          120
#define OVERMIND_ATTACK_RANGE       150.0f
#define OVERMIND_ATTACK_REPEAT      1000
#define OVERMIND_VALUE              300

#define HOVEL_BP                    8
#define HOVEL_BT                    15000
#define HOVEL_HEALTH                ABHM(750)
#define HOVEL_REGEN                 20
#define HOVEL_SPLASHDAMAGE          20
#define HOVEL_SPLASHRADIUS          200
#define HOVEL_CREEPSIZE             120



/*
 * ALIEN misc
 *
 * ALIENSENSE_RANGE - the distance alien sense is useful for
 * 
 */

#define ALIENSENSE_RANGE            1000.0f

#define ALIEN_POISON_TIME           10000
#define ALIEN_POISON_DMG            30
#define ALIEN_POISON_DIVIDER        (1.0f/1.32f) //about 1.0/(time`th root of damage)

#define ALIEN_SPAWN_REPEAT_TIME     10000


/*
 * HUMAN weapons
 *
 * _REPEAT  - time between firings
 * _RELOAD  - time needed to reload
 * _PRICE   - amount in credits weapon costs
 *
 * HUMAN_WDMG_MODIFIER - overall damage modifier for coarse tuning
 * 
 */

#define HUMAN_WDMG_MODIFIER         1.1f
#define HDM(d)                      ((int)((float)d*HUMAN_WDMG_MODIFIER))

#define BLASTER_REPEAT              600
#define BLASTER_SPREAD              200
#define BLASTER_SPEED               700
#define BLASTER_DMG                 HDM(10)

#define RIFLE_CLIPSIZE              30
#define RIFLE_SPAWNCLIPS            3
#define RIFLE_MAXCLIPS              3
#define RIFLE_REPEAT                100
#define RIFLE_RELOAD                2000
#define RIFLE_PRICE                 0
#define RIFLE_SPREAD                200
#define RIFLE_DMG                   HDM(5)

#define PAINSAW_PRICE               100
#define PAINSAW_REPEAT              75
#define PAINSAW_DAMAGE              HDM(20)
#define PAINSAW_RANGE               48.0f

#define SHOTGUN_PRICE               150
#define SHOTGUN_SHELLS              8
#define SHOTGUN_PELLETS             8 //used to sync server and client side
#define SHOTGUN_SPAWNCLIPS          2
#define SHOTGUN_MAXCLIPS            2
#define SHOTGUN_REPEAT              1200
#define SHOTGUN_RELOAD              2000
#define SHOTGUN_SPREAD              900
#define SHOTGUN_DMG                 HDM(12)

#define LASGUN_PRICE                250
#define LASGUN_AMMO                 100
#define LASGUN_REPEAT               150
#define LASGUN_RELOAD               2000
#define LASGUN_DAMAGE               HDM(10)

#define MDRIVER_PRICE               350
#define MDRIVER_CLIPSIZE            5
#define MDRIVER_SPAWNCLIPS          2
#define MDRIVER_MAXCLIPS            3
#define MDRIVER_DMG                 HDM(50)
#define MDRIVER_REPEAT              1000
#define MDRIVER_RELOAD              2000

#define CHAINGUN_PRICE              400
#define CHAINGUN_BULLETS            200
#define CHAINGUN_REPEAT             75
#define CHAINGUN_SPREAD             1200
#define CHAINGUN_DMG                HDM(6)

#define PRIFLE_PRICE                400
#define PRIFLE_CLIPS                50
#define PRIFLE_SPAWNCLIPS           2
#define PRIFLE_MAXCLIPS             2
#define PRIFLE_REPEAT               100
#define PRIFLE_RELOAD               2000
#define PRIFLE_DMG                  HDM(10)
#define PRIFLE_SPEED                1000

#define FLAMER_PRICE                450
#define FLAMER_GAS                  80
#define FLAMER_REPEAT               300
#define FLAMER_DMG                  HDM(30)
#define FLAMER_RADIUS               50
#define FLAMER_LIFETIME             1000.0f
#define FLAMER_SPEED                200.0f
#define FLAMER_LAG                  0.65f  //the amount of player velocity that is added to the fireball

#define LCANNON_PRICE               600
#define LCANNON_AMMO                60
#define LCANNON_REPEAT              500
#define LCANNON_CHARGEREPEAT        1000
#define LCANNON_RELOAD              2000
#define LCANNON_DAMAGE              HDM(300)
#define LCANNON_RADIUS              150
#define LCANNON_SECONDARY_DAMAGE    HDM(30)
#define LCANNON_SECONDARY_RADIUS    75
#define LCANNON_SPEED               250
#define LCANNON_CHARGE_TIME         1000
#define LCANNON_TOTAL_CHARGE        255

#define HBUILD_PRICE                0
#define HBUILD_REPEAT               1000
#define HBUILD_DELAY                9000

#define HBUILD2_PRICE               75
#define HBUILD2_REPEAT              1000
#define HBUILD2_DELAY               7000



/*
 * HUMAN upgrades
 */

#define LIGHTARMOUR_PRICE           70

#define HELMET_PRICE                90
#define HELMET_RANGE                1000.0f

#define ANTITOXIN_PRICE             20

#define BATTPACK_PRICE              100
#define BATTPACK_MODIFIER           1.5f //modifier for extra energy storage available

#define JETPACK_PRICE               120
#define JETPACK_FLOAT_SPEED         128.0f //up movement speed
#define JETPACK_SINK_SPEED          192.0f //down movement speed
#define JETPACK_DISABLE_TIME        1000 //time to disable the jetpack when player damaged
#define JETPACK_DISABLE_CHANCE      0.3f

#define BSUIT_PRICE                 200

#define MGCLIP_PRICE                0

#define CGAMMO_PRICE                0

#define GAS_PRICE                   0



/*
 * HUMAN buildables
 *
 * _BP            - build points required for this buildable
 * _BT            - build time required for this buildable
 * _SPLASHDAMGE   - the amount of damage caused by this buildable when it blows up
 * _SPLASHRADIUS  - the radius around which it does this damage 
 *
 * REACTOR_BASESIZE - the maximum distance a buildable can be from an reactor
 * REPEATER_BASESIZE - the maximum distance a buildable can be from a repeater
 * HUMAN_BHLTH_MODIFIER - overall health modifier for coarse tuning
 * 
 */

#define HUMAN_BHLTH_MODIFIER        0.6f
#define HBHM(h)                     ((int)((float)h*HUMAN_BHLTH_MODIFIER))

#define REACTOR_BASESIZE            1000
#define REPEATER_BASESIZE           500
#define HUMAN_DETONATION_DELAY      5000

#define HSPAWN_BP                   10
#define HSPAWN_BT                   10000
#define HSPAWN_HEALTH               HBHM(500)
#define HSPAWN_SPLASHDAMAGE         50
#define HSPAWN_SPLASHRADIUS         100
#define HSPAWN_VALUE                1

#define MEDISTAT_BP                 8
#define MEDISTAT_BT                 10000
#define MEDISTAT_HEALTH             HBHM(300)
#define MEDISTAT_SPLASHDAMAGE       50
#define MEDISTAT_SPLASHRADIUS       100

#define MGTURRET_BP                 8
#define MGTURRET_BT                 10000
#define MGTURRET_HEALTH             HBHM(450)
#define MGTURRET_SPLASHDAMAGE       50
#define MGTURRET_SPLASHRADIUS       100
#define MGTURRET_ANGULARSPEED       6  //degrees/think ~= 200deg/sec
#define MGTURRET_ACCURACYTOLERANCE  MGTURRET_ANGULARSPEED / 1.5f //angular difference for turret to fire
#define MGTURRET_VERTICALCAP        30  // +/- maximum pitch
#define MGTURRET_REPEAT             100
#define MGTURRET_RANGE              250
#define MGTURRET_SPREAD             200
#define MGTURRET_DMG                HDM(5)
#define MGTURRET_DCC_ANGULARSPEED       7
#define MGTURRET_DCC_ACCURACYTOLERANCE  MGTURRET_DCC_ANGULARSPEED / 1.5f

#define TESLAGEN_BP                 10
#define TESLAGEN_BT                 15000
#define TESLAGEN_HEALTH             HBHM(350)
#define TESLAGEN_SPLASHDAMAGE       50
#define TESLAGEN_SPLASHRADIUS       100
#define TESLAGEN_REPEAT             500
#define TESLAGEN_RANGE              300
#define TESLAGEN_DMG                HDM(20)

#define DC_BP                       8
#define DC_BT                       10000
#define DC_HEALTH                   HBHM(150)
#define DC_SPLASHDAMAGE             50
#define DC_SPLASHRADIUS             100

#define ARMOURY_BP                  10
#define ARMOURY_BT                  10000
#define ARMOURY_HEALTH              HBHM(200)
#define ARMOURY_SPLASHDAMAGE        50
#define ARMOURY_SPLASHRADIUS        100

#define REACTOR_BP                  0
#define REACTOR_BT                  20000
#define REACTOR_HEALTH              HBHM(1500)
#define REACTOR_SPLASHDAMAGE        200
#define REACTOR_SPLASHRADIUS        300
#define REACTOR_VALUE               2

#define REPEATER_BP                 10
#define REPEATER_BT                 10000
#define REPEATER_HEALTH             HBHM(200)
#define REPEATER_SPLASHDAMAGE       50
#define REPEATER_SPLASHRADIUS       100

#define ENERGY_REFIL_TIME           1000 //1/2 second between every clip refil

/*
 * HUMAN misc
 */

#define HUMAN_SPRINT_MODIFIER       1.2f
#define HUMAN_JOG_MODIFIER          0.9f
#define HUMAN_BACK_MODIFIER         0.7f
#define HUMAN_SIDE_MODIFIER         0.8f

#define STAMINA_STOP_RESTORE        25
#define STAMINA_WALK_RESTORE        15
#define STAMINA_SPRINT_TAKE         8
#define STAMINA_LARMOUR_TAKE        4

#define HUMAN_SPAWN_REPEAT_TIME     10000

/*
 * Misc
 */

#define MIN_FALL_DISTANCE           30.0f //the fall distance at which fall damage kicks in
#define MAX_FALL_DISTANCE           120.0f //the fall distance at which maximum damage is dealt
#define AVG_FALL_DISTANCE           ((MIN_FALL_DISTANCE+MAX_FALL_DISTANCE)/2.0f)

#define HUMAN_MAX_CREDITS           2000
#define ALIEN_MAX_KILLS             9
