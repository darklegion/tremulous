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

#define ALIEN_WDMG_MODIFIER         1.0f
#define ADM(d)                      ((int)((float)d*ALIEN_WDMG_MODIFIER))

#define ABUILDER_BUILD_REPEAT       500
#define ABUILDER_CLAW_DMG           ADM(25)
#define ABUILDER_CLAW_RANGE         48.0f
#define ABUILDER_CLAW_REPEAT        1000
#define ABUILDER_BASE_DELAY         9000
#define ABUILDER_ADV_DELAY          4000

#define SOLDIER_BITE_DMG            ADM(34)
#define SOLDIER_BITE_RANGE          32.0f
#define SOLDIER_BITE_REPEAT         500

#define HYDRA_CLAW_DMG              ADM(50)
#define HYDRA_CLAW_RANGE            96.0f
#define HYDRA_CLAW_REPEAT           500
#define HYDRA_CLAW_U_REPEAT         500
#define HYDRA_GRAB_RANGE            64.0f
#define HYDRA_GRAB_TIME             1000
#define HYDRA_PCLOUD_DMG            ADM(5)
#define HYDRA_PCLOUD_RANGE          200.0f
#define HYDRA_PCLOUD_REPEAT         1000
#define HYDRA_PCLOUD_TIME           10000

#define DRAGOON_CLAW_DMG            ADM(75)
#define DRAGOON_CLAW_RANGE          48.0f
#define DRAGOON_CLAW_REPEAT         750
#define DRAGOON_CLAW_U_REPEAT       750
#define DRAGOON_POUNCE_DMG          ADM(100)
#define DRAGOON_POUNCE_RANGE        48.0f
#define DRAGOON_POUNCE_SPEED        750
#define DRAGOON_POUNCE_SPEED_MOD    0.75f
#define DRAGOON_POUNCE_TIME         1000
#define DRAGOON_SLOWBLOB_DMG        ADM(20)
#define DRAGOON_SLOWBLOB_REPEAT     1000
#define DRAGOON_SLOWBLOB_SPEED      800.0f
#define DRAGOON_SLOWBLOB_SPEED_MOD  0.5f
#define DRAGOON_SLOWBLOB_TIME       5000

#define CHIMERA_CLAW_DMG            ADM(75)
#define CHIMERA_CLAW_RANGE          96.0f
#define CHIMERA_CLAW_REPEAT         400
#define CHIMERA_CLAW_U_REPEAT       300
#define CHIMERA_AREAZAP_DMG         ADM(75)
#define CHIMERA_AREAZAP_RANGE       200.0f
#define CHIMERA_AREAZAP_REPEAT      1500
#define CHIMERA_DIRECTZAP_DMG       ADM(75)
#define CHIMERA_DIRECTZAP_RANGE     200.0f
#define CHIMERA_DIRECTZAP_REPEAT    1500

#define BMOFO_CLAW_DMG              ADM(150)
#define BMOFO_CLAW_RANGE            128.0f
#define BMOFO_CLAW_REPEAT           750
#define BMOFO_KNOCK_RANGE           200.0f
#define BMOFO_KNOCK_REPEAT          15000
#define BMOFO_KOVER_TIME            2000
#define BMOFO_GETUP_TIME            1000
#define BMOFO_REGEN_RANGE           200.0f
#define BMOFO_REGEN_MOD             2.0f



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

#define ALIEN_VALUE_MODIFIER        3.0f
#define AVM(h)                      ((int)((float)h*ALIEN_VALUE_MODIFIER))

#define ABUILDER_SPEED              0.8f
#define ABUILDER_TTE                2000
#define ABUILDER_VALUE              AVM(50)
#define ABUILDER_HEALTH             AHM(50)
#define ABUILDER_REGEN              2

#define ABUILDER_UPG_SPEED          1.0f
#define ABUILDER_UPG_VALUE          AVM(120)
#define ABUILDER_UPG_HEALTH         AHM(75)
#define ABUILDER_UPG_REGEN          3

#define SOLDIER_SPEED               1.3f
#define SOLDIER_TTE                 2000
#define SOLDIER_VALUE               AVM(50)
#define SOLDIER_HEALTH              AHM(25)
#define SOLDIER_REGEN               1

#define HYDRA_SPEED                 1.25f
#define HYDRA_TTE                   3000
#define HYDRA_VALUE                 AVM(100)
#define HYDRA_HEALTH                AHM(50)
#define HYDRA_REGEN                 2

#define HYDRA_UPG_SPEED             1.25f
#define HYDRA_UPG_TTE               3000
#define HYDRA_UPG_VALUE             AVM(120)
#define HYDRA_UPG_HEALTH            AHM(50)
#define HYDRA_UPG_REGEN             3

#define DRAGOON_SPEED               1.2f
#define DRAGOON_TTE                 3000
#define DRAGOON_VALUE               AVM(200)
#define DRAGOON_HEALTH              AHM(100)
#define DRAGOON_REGEN               4

#define DRAGOON_UPG_SPEED           1.2f
#define DRAGOON_UPG_TTE             3000
#define DRAGOON_UPG_VALUE           AVM(240)
#define DRAGOON_UPG_HEALTH          AHM(120)
#define DRAGOON_UPG_REGEN           5

#define CHIMERA_SPEED               1.2f
#define CHIMERA_TTE                 3000
#define CHIMERA_VALUE               AVM(300)
#define CHIMERA_HEALTH              AHM(200)
#define CHIMERA_REGEN               6

#define CHIMERA_UPG_SPEED           1.2f
#define CHIMERA_UPG_TTE             3000
#define CHIMERA_UPG_VALUE           AVM(360)
#define CHIMERA_UPG_HEALTH          AHM(250)
#define CHIMERA_UPG_REGEN           7

#define BMOFO_SPEED                 1.1f
#define BMOFO_VALUE                 AVM(400)
#define BMOFO_HEALTH                AHM(400)
#define BMOFO_REGEN                 9



/*
 * ALIEN buildables
 *
 * _BP            - build points required for this buildable
 * _REGEN         - the amount of health per second regained
 * _SPLASHDAMGE   - the amount of damage caused by this buildable when melting
 * _SPLASHRADIUS  - the radius around which it does this damage
 *
 * CREEP_BASESIZE - the maximum distance a buildable can be from an egg/overmind
 * ALIEN_BHLTH_MODIFIER - overall health modifier for coarse tuning
 * 
 */

#define ALIEN_BHLTH_MODIFIER        1.5f
#define ABHM(h)                     ((int)((float)h*ALIEN_BHLTH_MODIFIER))

#define CREEP_BASESIZE              700 
#define CREEP_TIMEOUT               1000
#define CREEP_MODIFIER              0.5f
#define CREEP_ARMOUR_MODIFIER       0.75f

#define ASPAWN_BP                   100
#define ASPAWN_HEALTH               ABHM(500)
#define ASPAWN_REGEN                10
#define ASPAWN_SPLASHDAMAGE         50
#define ASPAWN_SPLASHRADIUS         50
#define ASPAWN_CREEPSIZE            120
#define ASPAWN_VALUE                150

#define BARRICADE_BP                80
#define BARRICADE_HEALTH            ABHM(250)
#define BARRICADE_REGEN             15
#define BARRICADE_SPLASHDAMAGE      50
#define BARRICADE_SPLASHRADIUS      50
#define BARRICADE_CREEPSIZE         120

#define BOOSTER_BP                  120
#define BOOSTER_HEALTH              ABHM(200)
#define BOOSTER_REGEN               10
#define BOOSTER_SPLASHDAMAGE        50
#define BOOSTER_SPLASHRADIUS        50
#define BOOSTER_CREEPSIZE           120
#define BOOSTER_INTERVAL            30000 //time in msec between uses (per player)

#define ACIDTUBE_BP                 50
#define ACIDTUBE_HEALTH             ABHM(100)
#define ACIDTUBE_REGEN              10
#define ACIDTUBE_SPLASHDAMAGE       30
#define ACIDTUBE_SPLASHRADIUS       200
#define ACIDTUBE_CREEPSIZE          120
#define ACIDTUBE_RANGE              200.0f
#define ACIDTUBE_REPEAT             10000

#define TRAPPER_BP                  150
#define TRAPPER_HEALTH              ABHM(80)
#define TRAPPER_REGEN               8
#define TRAPPER_SPLASHDAMAGE        15
#define TRAPPER_SPLASHRADIUS        100
#define TRAPPER_CREEPSIZE           30
#define TRAPPER_RANGE               400
#define TRAPPER_REPEAT              1000
#define LOCKBLOB_SPEED              500
#define LOCKBLOB_DOT                0.85f // max angle = acos( LOCKBLOB_DOT )

#define OVERMIND_BP                 0
#define OVERMIND_HEALTH             ABHM(1000)
#define OVERMIND_REGEN              15
#define OVERMIND_SPLASHDAMAGE       100
#define OVERMIND_SPLASHRADIUS       300
#define OVERMIND_CREEPSIZE          120
#define OVERMIND_ATTACK_RANGE       150.0f
#define OVERMIND_ATTACK_REPEAT      1000
#define OVERMIND_VALUE              300

#define HOVEL_BP                    80
#define HOVEL_HEALTH                ABHM(750)
#define HOVEL_REGEN                 20
#define HOVEL_SPLASHDAMAGE          20
#define HOVEL_SPLASHRADIUS          200
#define HOVEL_CREEPSIZE             120

#define OBANK_BP                    80
#define OBANK_HEALTH                ABHM(100)
#define OBANK_REGEN                 10
#define OBANK_SPLASHDAMAGE          20
#define OBANK_SPLASHRADIUS          100
#define OBANK_CREEPSIZE             120



/*
 * ALIEN misc
 *
 * ALIENSENSE_RANGE - the distance alien sense is useful for
 * 
 */

#define ALIENSENSE_RANGE            1000.0f
#define ALIENSTAGE2_HLTH_MODIFIER   1.2f
#define ALIENSTAGE3_HLTH_MODIFIER   1.5f



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

#define HUMAN_WDMG_MODIFIER         1.5f
#define HDM(d)                      ((int)((float)d*HUMAN_WDMG_MODIFIER))

#define RIFLE_CLIPSIZE              30
#define RIFLE_SPAWNCLIPS            3
#define RIFLE_MAXCLIPS              3
#define RIFLE_REPEAT                100
#define RIFLE_RELOAD                2000
#define RIFLE_PRICE                 0
#define RIFLE_SPREAD                200
#define RIFLE_DMG                   HDM(9)

#define CHAINGUN_BULLETS            300
#define CHAINGUN_REPEAT             50
#define CHAINGUN_PRICE              200
#define CHAINGUN_SPREAD             1200
#define CHAINGUN_DMG                HDM(14)

#define FLAMER_GAS                  80
#define FLAMER_REPEAT               300
#define FLAMER_PRICE                300
#define FLAMER_DMG                  HDM(30)
#define FLAMER_RADIUS               50
#define FLAMER_LIFETIME             1000.0f
#define FLAMER_SPEED                200.0f
#define FLAMER_LAG                  0.65f  //the amount of player velocity that is added to the fireball

#define MDRIVER_CLIPSIZE            5
#define MDRIVER_SPAWNCLIPS          2
#define MDRIVER_MAXCLIPS            3
#define MDRIVER_PRICE               300
#define MDRIVER_DMG                 HDM(50)
#define MDRIVER_REPEAT              1000
#define MDRIVER_RELOAD              2000

#define PRIFLE_CLIPS                50
#define PRIFLE_SPAWNCLIPS           3
#define PRIFLE_MAXCLIPS             3
#define PRIFLE_PRICE                250
#define PRIFLE_REPEAT               100
#define PRIFLE_RELOAD               2000
#define PRIFLE_DMG                  HDM(15)
#define PRIFLE_SPEED                1500

#define LCANNON_PRICE               400
#define LCANNON_AMMO                30
#define LCANNON_REPEAT              500
#define LCANNON_CHARGEREPEAT        1000
#define LCANNON_RELOAD              2000
#define LCANNON_DAMAGE              HDM(200)
#define LCANNON_SECONDARY_DAMAGE    20
#define LCANNON_SPEED               250
#define LCANNON_CHARGE_TIME         2000

#define LASGUN_PRICE                200
#define LASGUN_AMMO                 300
#define LASGUN_REPEAT               100
#define LASGUN_RELOAD               2000
#define LASGUN_DAMAGE               HDM(10)

#define PAINSAW_PRICE               100
#define PAINSAW_REPEAT              75
#define PAINSAW_DAMAGE              HDM(5)
#define PAINSAW_RANGE               32

#define HBUILD_PRICE                0
#define HBUILD_REPEAT               1000
#define HBUILD_DELAY                9000

#define HBUILD2_PRICE               200
#define HBUILD2_REPEAT              1000
#define HBUILD2_DELAY               4000



/*
 * HUMAN upgrades
 */

#define LIGHTARMOUR_PRICE           100

#define HELMET_PRICE                90
#define HELMET_RANGE                1000.0f

#define ANTITOXIN_PRICE             20

#define BATTPACK_PRICE              100
#define BATTPACK_MODIFIER           2.0f //modifier for extra energy storage available

#define JETPACK_PRICE               120
#define JETPACK_FLOAT_SPEED         128.0f //up movement speed
#define JETPACK_SINK_SPEED          192.0f //down movement speed

#define BSUIT_PRICE                 200

#define MGCLIP_PRICE                0

#define CGAMMO_PRICE                0

#define GAS_PRICE                   0



/*
 * HUMAN buildables
 *
 * _BP            - build points required for this buildable
 * _SPLASHDAMGE   - the amount of damage caused by this buildable when it blows up
 * _SPLASHRADIUS  - the radius around which it does this damage 
 *
 * REACTOR_BASESIZE - the maximum distance a buildable can be from an reactor
 * REPEATER_BASESIZE - the maximum distance a buildable can be from a repeater
 * HUMAN_BHLTH_MODIFIER - overall health modifier for coarse tuning
 * 
 */

#define HUMAN_BHLTH_MODIFIER        1.0f
#define HBHM(h)                     ((int)((float)h*HUMAN_BHLTH_MODIFIER))

#define REACTOR_BASESIZE            1000
#define REPEATER_BASESIZE           500
#define HUMAN_DETONATION_DELAY      5000

#define HSPAWN_BP                   100
#define HSPAWN_HEALTH               HBHM(500)
#define HSPAWN_SPLASHDAMAGE         50
#define HSPAWN_SPLASHRADIUS         100
#define HSPAWN_VALUE                1

#define MEDISTAT_BP                 80
#define MEDISTAT_HEALTH             HBHM(200)
#define MEDISTAT_SPLASHDAMAGE       50
#define MEDISTAT_SPLASHRADIUS       100
#define MAX_MEDISTAT_CLIENTS        1

#define MGTURRET_BP                 80
#define MGTURRET_HEALTH             HBHM(100)
#define MGTURRET_SPLASHDAMAGE       50
#define MGTURRET_SPLASHRADIUS       100
#define MGTURRET_ANGULARSPEED       5  //degrees/think ~= 200deg/sec
#define MGTURRET_ACCURACYTOLERANCE  MGTURRET_ANGULARSPEED / 2 //angular difference for turret to fire
#define MGTURRET_VERTICALCAP        30  // +/- maximum pitch
#define MGTURRET_REPEAT             100
#define MGTURRET_RANGE              250

#define TESLAGEN_BP                 100
#define TESLAGEN_HEALTH             HBHM(200)
#define TESLAGEN_SPLASHDAMAGE       50
#define TESLAGEN_SPLASHRADIUS       100
#define TESLAGEN_REPEAT             500
#define TESLAGEN_RANGE              1500

#define DC_BP                       80
#define DC_HEALTH                   HBHM(150)
#define DC_SPLASHDAMAGE             50
#define DC_SPLASHRADIUS             100

#define BANK_BP                     80
#define BANK_HEALTH                 HBHM(100)
#define BANK_SPLASHDAMAGE           50
#define BANK_SPLASHRADIUS           100

#define ARMOURY_BP                  100
#define ARMOURY_HEALTH              HBHM(175)
#define ARMOURY_SPLASHDAMAGE        50
#define ARMOURY_SPLASHRADIUS        100

#define REACTOR_BP                  0
#define REACTOR_HEALTH              HBHM(1000)
#define REACTOR_SPLASHDAMAGE        200
#define REACTOR_SPLASHRADIUS        300
#define REACTOR_VALUE               2

#define REPEATER_BP                 100
#define REPEATER_HEALTH             HBHM(200)
#define REPEATER_SPLASHDAMAGE       50
#define REPEATER_SPLASHRADIUS       100

#define ENERGY_REFIL_TIME           500 //1/2 second between every clip refil

#define FLOATMINE_BP                50
#define FLOATMINE_HEALTH            HBHM(10)
#define FLOATMINE_SPLASHDAMAGE      250
#define FLOATMINE_SPLASHRADIUS      500

/*
 * HUMAN misc
 */

#define HUMAN_JOG_MODIFIER          0.9f
#define HUMAN_BACK_MODIFIER         0.7f
#define HUMAN_SIDE_MODIFIER         0.8f

#define STAMINA_STOP_RESTORE        10
#define STAMINA_WALK_RESTORE        5
#define STAMINA_SPRINT_TAKE         10
#define STAMINA_LARMOUR_TAKE        5

