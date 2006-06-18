/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2006 Tim Angus

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

// cg_consolecmds.c -- text commands typed in at the local console, or
// executed by a key binding


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

qboolean CG_RequestScores( void )
{
  if( cg.scoresRequestTime + 2000 < cg.time )
  {
    // the scores are more than two seconds out of data,
    // so request new ones
    cg.scoresRequestTime = cg.time;
    //TA: added \n SendClientCommand doesn't call flush( )?
    trap_SendClientCommand( "score\n" );

    return qtrue;
  }
  else
    return qfalse;
}

extern menuDef_t *menuScoreboard;

static void CG_scrollScoresDown_f( void )
{
  if( menuScoreboard && cg.scoreBoardShowing )
  {
    Menu_ScrollFeeder( menuScoreboard, FEEDER_ALIENTEAM_LIST, qtrue );
    Menu_ScrollFeeder( menuScoreboard, FEEDER_HUMANTEAM_LIST, qtrue );
  }
}


static void CG_scrollScoresUp_f( void )
{
  if( menuScoreboard && cg.scoreBoardShowing )
  {
    Menu_ScrollFeeder( menuScoreboard, FEEDER_ALIENTEAM_LIST, qfalse );
    Menu_ScrollFeeder( menuScoreboard, FEEDER_HUMANTEAM_LIST, qfalse );
  }
}

static void CG_ScoresDown_f( void )
{
  if( !cg.showScores )
  {
    Menu_SetFeederSelection( menuScoreboard, FEEDER_ALIENTEAM_LIST, 0, NULL );
    Menu_SetFeederSelection( menuScoreboard, FEEDER_HUMANTEAM_LIST, 0, NULL );
  }

  if( CG_RequestScores( ) )
  {
    // leave the current scores up if they were already
    // displayed, but if this is the first hit, clear them out
    if( !cg.showScores )
    {
      if( cg_debugRandom.integer )
        CG_Printf( "CG_ScoresDown_f: scores out of date\n" );

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
  { "scoresUp", CG_scrollScoresUp_f },
  { "scoresDown", CG_scrollScoresDown_f },
  { "sizeup", CG_SizeUp_f },
  { "sizedown", CG_SizeDown_f },
  { "weapnext", CG_NextWeapon_f },
  { "weapprev", CG_PrevWeapon_f },
  { "weapon", CG_Weapon_f },
  { "tell_target", CG_TellTarget_f },
  { "tell_attacker", CG_TellAttacker_f },
  { "tcmd", CG_TargetCommand_f },
  { "testPS", CG_TestPS_f },
  { "destroyTestPS", CG_DestroyTestPS_f },
  { "testTS", CG_TestTS_f },
  { "destroyTestTS", CG_DestroyTestTS_f },
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
  trap_AddCommand( "reload" );
  trap_AddCommand( "itemact" );
  trap_AddCommand( "itemdeact" );
  trap_AddCommand( "itemtoggle" );
  trap_AddCommand( "destroy" );
  trap_AddCommand( "deconstruct" );
  trap_AddCommand( "menu" );
  trap_AddCommand( "ui_menu" );
  trap_AddCommand( "mapRotation" );
  trap_AddCommand( "stopMapRotation" );
  trap_AddCommand( "advanceMapRotation" );
  trap_AddCommand( "alienWin" );
  trap_AddCommand( "humanWin" );
}
