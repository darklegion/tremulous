#ifndef MENUDEF_H
#define MENUDEF_H

#define ITEM_TYPE_ANY          -1 // invalid type
#define ITEM_TYPE_NONE          0 // no specified type
#define ITEM_TYPE_TEXT          1 // simple text
#define ITEM_TYPE_BUTTON        2 // button, basically text with a border
#define ITEM_TYPE_RADIOBUTTON   3 // toggle button, may be grouped
#define ITEM_TYPE_CHECKBOX      4 // check box
#define ITEM_TYPE_EDITFIELD     5 // editable text, associated with a cvar
#define ITEM_TYPE_SAYFIELD      6 // the chat field
#define ITEM_TYPE_CYCLE         7 // cycling list
#define ITEM_TYPE_LISTBOX       8 // scrollable list
#define ITEM_TYPE_COMBOBOX      9 // drop down scrollable list
#define ITEM_TYPE_MODEL        10 // model
#define ITEM_TYPE_OWNERDRAW    11 // owner draw, has an associated ownerdraw number
#define ITEM_TYPE_NUMERICFIELD 12 // editable text, associated with a cvar
#define ITEM_TYPE_SLIDER       13 // mouse speed, volume, etc.
#define ITEM_TYPE_YESNO        14 // boolean cvar setting
#define ITEM_TYPE_MULTI        15 // multiple list setting, enumerated
#define ITEM_TYPE_BIND         16 // keyboard control configuration

#define ALIGN_LEFT   0 // left alignment
#define ALIGN_CENTER 1 // center alignment
#define ALIGN_RIGHT  2 // right alignment
#define ASPECT_NONE  3 // no aspect compensation
#define ALIGN_NONE   4

#define VALIGN_BOTTOM 0 // bottom alignment
#define VALIGN_CENTER 1 // center alignment
#define VALIGN_TOP    2 // top alignment
#define VALIGN_NONE   3

#define ITEM_TEXTSTYLE_NORMAL          0 // normal text
#define ITEM_TEXTSTYLE_BLINK           1 // fast blinking
#define ITEM_TEXTSTYLE_PULSE           2 // slow pulsing
#define ITEM_TEXTSTYLE_SHADOWED        3 // drop shadow (need a color for this)
#define ITEM_TEXTSTYLE_OUTLINED        4 // apparently unimplemented
#define ITEM_TEXTSTYLE_OUTLINESHADOWED 5 // apparently unimplemented
#define ITEM_TEXTSTYLE_SHADOWEDMORE    6 // drop shadow (need a color for this)
#define ITEM_TEXTSTYLE_NEON            7 // glow (need a color for this)

#define WINDOW_BORDER_NONE       0 // no border
#define WINDOW_BORDER_FULL       1 // full border based on border color (single pixel)
#define WINDOW_BORDER_HORZ       2 // horizontal borders only
#define WINDOW_BORDER_VERT       3 // vertical borders only
#define WINDOW_BORDER_KCGRADIENT 4 // horizontal border using the gradient bars

#define WINDOW_STYLE_EMPTY     0 // no background
#define WINDOW_STYLE_FILLED    1 // filled with background color
#define WINDOW_STYLE_GRADIENT  2 // gradient bar based on background color
#define WINDOW_STYLE_SHADER    3 // use background shader
#define WINDOW_STYLE_TEAMCOLOR 4 // team color
#define WINDOW_STYLE_CINEMATIC 5 // cinematic

#define MENU_TRUE                         1     // uh.. true
#define MENU_FALSE                        0     // and false

#define HUD_VERTICAL   0
#define HUD_HORIZONTAL 1

// list box element types
#define LISTBOX_TEXT  0
#define LISTBOX_IMAGE 1

// list feeders
#define FEEDER_SERVERS               0 // servers
#define FEEDER_MAPS                  1 // all maps available, in graphic format
#define FEEDER_ALIENTEAM_LIST        2 // alien team members
#define FEEDER_HUMANTEAM_LIST        3 // human team members
#define FEEDER_TEAM_LIST             4 // team members for team voting
#define FEEDER_PLAYER_LIST           5 // players
#define FEEDER_NEWS                  6 // news
#define FEEDER_MODS                  7 // list of available mods
#define FEEDER_DEMOS                 8 // list of available demo files
#define FEEDER_SERVERSTATUS          9 // server status
#define FEEDER_FINDPLAYER           10 // find player

#define FEEDER_CINEMATICS           11 // cinematics
#define FEEDER_TREMTEAMS            12 // teams
#define FEEDER_TREMALIENCLASSES     13 // alien classes
#define FEEDER_TREMHUMANITEMS       14 // human items
#define FEEDER_TREMHUMANARMOURYBUY  15 // human buy
#define FEEDER_TREMHUMANARMOURYSELL 16 // human sell
#define FEEDER_TREMALIENUPGRADE     17 // alien upgrade
#define FEEDER_TREMALIENBUILD       18 // alien buildables
#define FEEDER_TREMHUMANBUILD       19 // human buildables
#define FEEDER_IGNORE_LIST          20 // ignored players

#define FEEDER_HELP_LIST            21 // help topics
#define FEEDER_RESOLUTIONS          22 // display resolutions

// display flags
#define UI_SHOW_FAVORITESERVERS           0x00000001
#define UI_SHOW_NOTFAVORITESERVERS        0x00000002

#define UI_SHOW_VOTEACTIVE                0x00000004
#define UI_SHOW_CANVOTE                   0x00000008
#define UI_SHOW_TEAMVOTEACTIVE            0x00000010
#define UI_SHOW_CANTEAMVOTE               0x00000020

#define UI_SHOW_NOTSPECTATING             0x00000040

// owner draw types
#define CG_PLAYER_HEALTH              0
#define CG_PLAYER_HEALTH_CROSS        1
#define CG_PLAYER_AMMO_VALUE          2
#define CG_PLAYER_CLIPS_VALUE         3
#define CG_PLAYER_BUILD_TIMER         4
#define CG_PLAYER_CREDITS_VALUE       5
#define CG_PLAYER_CREDITS_VALUE_NOPAD 6
#define CG_PLAYER_STAMINA             7
#define CG_PLAYER_STAMINA_1           8
#define CG_PLAYER_STAMINA_2           9
#define CG_PLAYER_STAMINA_3          10
#define CG_PLAYER_STAMINA_4          11
#define CG_PLAYER_STAMINA_BOLT       12
#define CG_PLAYER_BOOST_BOLT         13
#define CG_PLAYER_CLIPS_RING         14
#define CG_PLAYER_BUILD_TIMER_RING   15
#define CG_PLAYER_SELECT             16
#define CG_PLAYER_SELECTTEXT         17
#define CG_PLAYER_WEAPONICON         18
#define CG_PLAYER_WALLCLIMBING       19
#define CG_PLAYER_BOOSTED            20
#define CG_PLAYER_POISON_BARBS       21
#define CG_PLAYER_ALIEN_SENSE        22
#define CG_PLAYER_HUMAN_SCANNER      23
#define CG_PLAYER_USABLE_BUILDABLE   24
#define CG_PLAYER_CHARGE_BAR_BG      25
#define CG_PLAYER_CHARGE_BAR         26
#define CG_PLAYER_CROSSHAIR          27
#define CG_PLAYER_LOCATION           28
#define CG_TEAMOVERLAY               29
#define CG_PLAYER_CREDITS_FRACTION   30

#define CG_KILLER                    31
#define CG_SPECTATORS                32
#define CG_FOLLOW                    33
// loading screen
#define CG_LOAD_LEVELSHOT            34
#define CG_LOAD_MEDIA                35
#define CG_LOAD_MEDIA_LABEL          36
#define CG_LOAD_BUILDABLES           37
#define CG_LOAD_BUILDABLES_LABEL     38
#define CG_LOAD_CHARMODEL            39
#define CG_LOAD_CHARMODEL_LABEL      40
#define CG_LOAD_OVERALL              41
#define CG_LOAD_LEVELNAME            42
#define CG_LOAD_MOTD                 43
#define CG_LOAD_HOSTNAME             44

#define CG_FPS                       45
#define CG_FPS_FIXED                 46
#define CG_TIMER                     47
#define CG_TIMER_MINS                48
#define CG_TIMER_SECS                49
#define CG_SNAPSHOT                  50
#define CG_LAGOMETER                 51
#define CG_SPEEDOMETER               52
#define CG_PLAYER_CROSSHAIRNAMES     53
#define CG_STAGE_REPORT_TEXT         54
#define CG_ALIENS_SCORE_LABEL        55
#define CG_HUMANS_SCORE_LABEL        56
#define CG_DEMO_PLAYBACK             57
#define CG_DEMO_RECORDING            58

#define CG_CONSOLE                   59
#define CG_TUTORIAL                  60
#define CG_CLOCK                     61

#define UI_NETSOURCE                 62
#define UI_NETMAPPREVIEW             63
#define UI_NETMAPCINEMATIC           64
#define UI_SERVERREFRESHDATE         65
#define UI_SERVERMOTD                66
#define UI_GLINFO                    67
#define UI_KEYBINDSTATUS             68
#define UI_SELECTEDMAPPREVIEW        69
#define UI_SELECTEDMAPNAME           70

#define UI_TEAMINFOPANE              71
#define UI_ACLASSINFOPANE            72
#define UI_AUPGRADEINFOPANE          73
#define UI_HITEMINFOPANE             74
#define UI_HBUYINFOPANE              75
#define UI_HSELLINFOPANE             76
#define UI_ABUILDINFOPANE            77
#define UI_HBUILDINFOPANE            78
#define UI_HELPINFOPANE              79

#endif
