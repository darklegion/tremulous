// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_consolecmds.c -- text commands typed in at the local console, or
// executed by a key binding

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

#include "cg_local.h"



void CG_TargetCommand_f( void )
{
  int   targetNum;
  char  test[ 4 ];

  targetNum = CG_CrosshairPlayer( );
  if( !targetNum )
    return;

  trap_Argv( 1, test, 4 );
  trap_SendConsoleCommand( va( "gc %i %i", targetNum, atoi( test ) ) );
}



/*
=================
CG_SizeUp_f

Keybinding command
=================
*/
static void CG_SizeUp_f( void )
{
  trap_Cvar_Set( "cg_viewsize", va( "%i", (int)( cg_viewsize.integer + 10 ) ) );
}


/*
=================
CG_SizeDown_f

Keybinding command
=================
*/
static void CG_SizeDown_f( void )
{
  trap_Cvar_Set( "cg_viewsize", va( "%i", (int)( cg_viewsize.integer - 10 ) ) );
}


/*
=============
CG_Viewpos_f

Debugging command to print the current position
=============
*/
static void CG_Viewpos_f( void )
{
  CG_Printf( "(%i %i %i) : %i\n", (int)cg.refdef.vieworg[ 0 ],
    (int)cg.refdef.vieworg[ 1 ], (int)cg.refdef.vieworg[ 2 ],
    (int)cg.refdefViewAngles[ YAW ] );
}


static void CG_ScoresDown_f( void )
{
  if( cg.scoresRequestTime + 2000 < cg.time )
  {
    // the scores are more than two seconds out of data,
    // so request new ones
    cg.scoresRequestTime = cg.time;
    trap_SendClientCommand( "score" );

    // leave the current scores up if they were already
    // displayed, but if this is the first hit, clear them out
    if( !cg.showScores )
    {
      cg.showScores = qtrue;
      cg.numScores = 0;
    }
  }
  else
  {
    // show the cached contents even if they just pressed if it
    // is within two seconds
    cg.showScores = qtrue;
  }
}

static void CG_ScoresUp_f( void )
{
  if( cg.showScores )
  {
    cg.showScores = qfalse;
    cg.scoreFadeTime = cg.time;
  }
}

static void CG_TellTarget_f( void )
{
  int   clientNum;
  char  command[ 128 ];
  char  message[ 128 ];

  clientNum = CG_CrosshairPlayer( );
  if( clientNum == -1 )
    return;

  trap_Args( message, 128 );
  Com_sprintf( command, 128, "tell %i %s", clientNum, message );
  trap_SendClientCommand( command );
}

static void CG_TellAttacker_f( void )
{
  int   clientNum;
  char  command[ 128 ];
  char  message[ 128 ];

  clientNum = CG_LastAttacker( );
  if( clientNum == -1 )
    return;

  trap_Args( message, 128 );
  Com_sprintf( command, 128, "tell %i %s", clientNum, message );
  trap_SendClientCommand( command );
}

/*
==================
CG_DecodeMP3_f
==================
*/
void CG_DecodeMP3_f( void )
{
  char  mp3file[ MAX_QPATH ];
  char  wavfile[ MAX_QPATH ];

  if( trap_Argc( ) < 2 )
  {
    CG_Printf( "usage: decodeMP3 <mp3file> <wavfile>\n" );
    return;
  }
  
  Q_strncpyz( mp3file, CG_Argv( 1 ), MAX_QPATH );
  Q_strncpyz( wavfile, CG_Argv( 2 ), MAX_QPATH );

  S_decodeMP3( mp3file, wavfile );
}

typedef struct
{
  char  *cmd;
  void  (*function)( void );
} consoleCommand_t;

static consoleCommand_t commands[ ] =
{
  { "testgun", CG_TestGun_f },
  { "testmodel", CG_TestModel_f },
  { "nextframe", CG_TestModelNextFrame_f },
  { "prevframe", CG_TestModelPrevFrame_f },
  { "nextskin", CG_TestModelNextSkin_f },
  { "prevskin", CG_TestModelPrevSkin_f },
  { "viewpos", CG_Viewpos_f },
  { "+scores", CG_ScoresDown_f },
  { "-scores", CG_ScoresUp_f },
  { "+zoom", CG_ZoomDown_f },
  { "-zoom", CG_ZoomUp_f },
  { "sizeup", CG_SizeUp_f },
  { "sizedown", CG_SizeDown_f },
  { "weapnext", CG_NextWeapon_f },
  { "weapprev", CG_PrevWeapon_f },
  { "weapon", CG_Weapon_f },
  { "tell_target", CG_TellTarget_f },
  { "tell_attacker", CG_TellAttacker_f },
  { "tcmd", CG_TargetCommand_f },
  { "decodeMP3", CG_DecodeMP3_f },
};


/*
=================
CG_ConsoleCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
qboolean CG_ConsoleCommand( void )
{
  const char  *cmd;
  const char  *arg1;
  int         i;

  cmd = CG_Argv( 0 );

  //TA: ugly hacky special case
  if( !Q_stricmp( cmd, "ui_menu" ) )
  {
    arg1 = CG_Argv( 1 );
    trap_SendConsoleCommand( va( "menu %s\n", arg1 ) );
    return qtrue;
  }

  for( i = 0; i < sizeof( commands ) / sizeof( commands[ 0 ] ); i++ )
  {
    if( !Q_stricmp( cmd, commands[ i ].cmd ) )
    {
      commands[ i ].function( );
      return qtrue;
    }
  }

  return qfalse;
}


/*
=================
CG_InitConsoleCommands

Let the client system know about all of our commands
so it can perform tab completion
=================
*/
void CG_InitConsoleCommands( void )
{
  int   i;

  for( i = 0 ; i < sizeof( commands ) / sizeof( commands[ 0 ] ) ; i++ )
    trap_AddCommand( commands[ i ].cmd );

  //
  // the game server will interpret these commands, which will be automatically
  // forwarded to the server after they are not recognized locally
  //
  trap_AddCommand( "kill" );
  trap_AddCommand( "say" );
  trap_AddCommand( "say_team" );
  trap_AddCommand( "tell" );
  trap_AddCommand( "vsay" );
  trap_AddCommand( "vsay_team" );
  trap_AddCommand( "vtell" );
  trap_AddCommand( "vtaunt" );
  trap_AddCommand( "vosay" );
  trap_AddCommand( "vosay_team" );
  trap_AddCommand( "votell" );
  trap_AddCommand( "give" );
  trap_AddCommand( "god" );
  trap_AddCommand( "notarget" );
  trap_AddCommand( "noclip" );
  trap_AddCommand( "team" );
  trap_AddCommand( "follow" );
  trap_AddCommand( "levelshot" );
  trap_AddCommand( "addbot" );
  trap_AddCommand( "setviewpos" );
  trap_AddCommand( "callvote" );
  trap_AddCommand( "vote" );
  trap_AddCommand( "callteamvote" );
  trap_AddCommand( "teamvote" );
  trap_AddCommand( "stats" );
  trap_AddCommand( "teamtask" );
  trap_AddCommand( "class" );
  trap_AddCommand( "build" );
  trap_AddCommand( "buy" );
  trap_AddCommand( "sell" );
  trap_AddCommand( "deposit" );
  trap_AddCommand( "withdraw" );
  trap_AddCommand( "spawnbody" );
  trap_AddCommand( "itemact" );
  trap_AddCommand( "itemdeact" );
  trap_AddCommand( "itemtoggle" );
  trap_AddCommand( "destroy" );
  trap_AddCommand( "deconstruct" );
  trap_AddCommand( "menu" );
  trap_AddCommand( "ui_menu" );
  trap_AddCommand( "mapRotation" );
  trap_AddCommand( "stopMapRotation" );
  trap_AddCommand( "alienWin" );
  trap_AddCommand( "humanWin" );
}
