#define COMMON_HUD_R 1.0
#define COMMON_HUD_G 0.0
#define COMMON_HUD_B 0.0

//////////////////
//STATIC OBJECTS//
//////////////////

//LEFT RING CIRCLE
itemDef
{
  name "left-ring-circle"
  rect 47.5 410 25 25
  visible MENU_TRUE
  decoration
  forecolor COMMON_HUD_R COMMON_HUD_G COMMON_HUD_B 0.25
  style WINDOW_STYLE_SHADER
  background "ui/assets/neutral/circle.tga"
}

//LEFT ARM
itemDef
{
  name "left-arm"
  rect 77 404.75 104 52.5
  visible MENU_TRUE
  decoration
  forecolor COMMON_HUD_R COMMON_HUD_G COMMON_HUD_B 0.25
  style WINDOW_STYLE_SHADER
  background "ui/assets/alien/left-arm.tga"
}

//LEFT ARM CIRCLE
itemDef
{
  name "left-arm-circle"
  rect 150 417.5 25 25
  visible MENU_TRUE
  decoration
  forecolor COMMON_HUD_R COMMON_HUD_G COMMON_HUD_B 0.25
  style WINDOW_STYLE_SHADER
  background "ui/assets/neutral/circle.tga"
}

//RIGHT RING CIRCLE
itemDef
{
  name "right-ring-circle"
  rect 567 410 25 25
  visible MENU_TRUE
  decoration
  forecolor COMMON_HUD_R COMMON_HUD_G COMMON_HUD_B 0.25
  style WINDOW_STYLE_SHADER
  background "ui/assets/neutral/circle.tga"
}

//RIGHT ARM
itemDef
{
  name "right-arm"
  rect 459 404.75 104 52.5
  visible MENU_TRUE
  decoration
  forecolor COMMON_HUD_R COMMON_HUD_G COMMON_HUD_B 0.25
  style WINDOW_STYLE_SHADER
  background "ui/assets/alien/right-arm.tga"
}

///////////////////
//DYNAMIC OBJECTS//
///////////////////

//BOLT
itemDef
{
  name "bolt"
  rect 52.5 412.5 15 20
  visible MENU_TRUE
  decoration
  forecolor COMMON_HUD_R COMMON_HUD_G COMMON_HUD_B 0.5
  background "ui/assets/alien/bolt.tga"
  ownerdraw CG_PLAYER_BOOST_BOLT
}

//CROSS
itemDef
{
  name "cross"
  rect 155 422.5 15 15
  visible MENU_TRUE
  decoration
  forecolor COMMON_HUD_R COMMON_HUD_G COMMON_HUD_B 0.5
  style WINDOW_STYLE_SHADER
  background "ui/assets/neutral/cross.tga"
}

//LEFT RING
itemDef
{
  name "left-ring"
  rect 7.25 369.5 90.5 106
  visible MENU_TRUE
  decoration
  forecolor COMMON_HUD_R COMMON_HUD_G COMMON_HUD_B 0.5
  background "ui/assets/alien/left-ring.tga"
  ownerdraw CG_PLAYER_BOOSTED
}

//LEFT SPIKES
itemDef
{
  name "left-spikes"
  rect 18.5 381 59 83
  visible MENU_TRUE
  decoration
  forecolor COMMON_HUD_R COMMON_HUD_G COMMON_HUD_B 1.0
  background "ui/assets/alien/left-spikes.tga"
  ownerdraw CG_PLAYER_WALLCLIMBING
}

//RIGHT RING
itemDef
{
  name "right-ring"
  rect 542.25 369.5 90.5 106
  visible MENU_TRUE
  decoration
  forecolor COMMON_HUD_R COMMON_HUD_G COMMON_HUD_B 0.5
  background "ui/assets/alien/right-ring.tga"
  ownerdraw CG_PLAYER_BOOSTED
}

//RIGHT SPIKES
itemDef
{
  name "right-spikes"
  rect 562.5 381 59 83
  visible MENU_TRUE
  decoration
  forecolor COMMON_HUD_R COMMON_HUD_G COMMON_HUD_B 1.0
  background "ui/assets/alien/right-spikes.tga"
  ownerdraw CG_PLAYER_WALLCLIMBING
}

//HEALTH
itemDef
{
  name "health"
  rect 78.5 421.5 60 15
  visible MENU_TRUE
  decoration
  forecolor COMMON_HUD_R COMMON_HUD_G COMMON_HUD_B .5
  ownerdraw CG_PLAYER_HEALTH
}

//ALIEN CLASS ICON
itemDef
{
  name "alien-icon"
  rect 465 417.5 25 25
  visible MENU_TRUE
  decoration
  forecolor COMMON_HUD_R COMMON_HUD_G COMMON_HUD_B 0.6
  ownerdraw CG_PLAYER_WEAPONICON
}

//ORGANS
itemDef
{
  name "organs"
  rect 570.5 415.95 15 15
  visible MENU_TRUE
  decoration
  forecolor COMMON_HUD_R COMMON_HUD_G COMMON_HUD_B .5
  ownerdraw CG_PLAYER_CREDITS_VALUE_NOPAD
}

//ALIENSENSE
itemDef
{
  name "aliensense"
  rect 20 20 600 400
  visible MENU_TRUE
  decoration
  ownerdraw CG_PLAYER_ALIEN_SENSE
}
