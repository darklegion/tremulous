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

/**********************************************************************
  UI_ATOMS.C
 
  User interface building blocks and support functions.
**********************************************************************/
#include "ui_local.h"

qboolean    m_entersound;    // after a frame, so caching won't disrupt the sound

void QDECL Com_Error( int level, const char *error, ... )
{
  va_list    argptr;
  char    text[1024];

  va_start ( argptr, error );
  vsprintf ( text, error, argptr );
  va_end ( argptr );

  trap_Error( va( "%s", text ) );
}

void QDECL Com_Printf( const char *msg, ... )
{
  va_list    argptr;
  char    text[1024];

  va_start ( argptr, msg );
  vsprintf ( text, msg, argptr );
  va_end ( argptr );

  trap_Print( va( "%s", text ) );
}


/*
=================
UI_ClampCvar
=================
*/
float UI_ClampCvar( float min, float max, float value )
{
  if( value < min ) return min;

  if( value > max ) return max;

  return value;
}

/*
=================
UI_StartDemoLoop
=================
*/
void UI_StartDemoLoop( void )
{
  trap_Cmd_ExecuteText( EXEC_APPEND, "d1\n" );
}

char *UI_Argv( int arg )
{
  static char  buffer[MAX_STRING_CHARS];

  trap_Argv( arg, buffer, sizeof( buffer ) );

  return buffer;
}


char *UI_Cvar_VariableString( const char *var_name )
{
  static char  buffer[MAX_STRING_CHARS];

  trap_Cvar_VariableStringBuffer( var_name, buffer, sizeof( buffer ) );

  return buffer;
}

static void  UI_Cache_f( void )
{
  Display_CacheAll();
}

/*
=================
UI_ConsoleCommand
 
FIXME: lookup table
=================
*/
qboolean UI_ConsoleCommand( int realTime )
{
  char  * cmd;
  char  *arg1;

  uiInfo.uiDC.frameTime = realTime - uiInfo.uiDC.realTime;
  uiInfo.uiDC.realTime = realTime;

  cmd = UI_Argv( 0 );

  // ensure minimum menu data is available
  //Menu_Cache();

  if( Q_stricmp ( cmd, "ui_report" ) == 0 )
  {
    UI_Report();
    return qtrue;
  }

  if( Q_stricmp ( cmd, "ui_load" ) == 0 )
  {
    UI_Load();
    return qtrue;
  }

  if( Q_stricmp ( cmd, "remapShader" ) == 0 )
  {
    if( trap_Argc() == 4 )
    {
      char shader1[MAX_QPATH];
      char shader2[MAX_QPATH];
      Q_strncpyz( shader1, UI_Argv( 1 ), sizeof( shader1 ) );
      Q_strncpyz( shader2, UI_Argv( 2 ), sizeof( shader2 ) );
      trap_R_RemapShader( shader1, shader2, UI_Argv( 3 ) );
      return qtrue;
    }
  }

  if( Q_stricmp ( cmd, "ui_cache" ) == 0 )
  {
    UI_Cache_f();
    return qtrue;
  }

  if( Q_stricmp ( cmd, "ui_teamOrders" ) == 0 )
  {
    //UI_TeamOrdersMenu_f();
    return qtrue;
  }

  if( Q_strncmp( cmd, "ui_messagemode", 14 ) == 0 )
  {
    trap_Cvar_Set( "ui_sayBuffer", "" );

    switch( cmd[ 14 ] )
    {
      default:
      case '\0':
        // Global
        uiInfo.chatTeam             = qfalse;
    uiInfo.chatTargetClientNum = -1;
        break;

      case '2':
        // Team
        uiInfo.chatTeam             = qtrue;
        uiInfo.chatTargetClientNum  = -1;
        break;
    }

    trap_Key_SetCatcher( KEYCATCH_UI );
    Menus_ActivateByName( "say" );
    return qtrue;
  }

  if( Q_stricmp ( cmd, "menu" ) == 0 )
  {
    arg1 = UI_Argv( 1 );

    if( Menu_Count( ) > 0 )
    {
      trap_Key_SetCatcher( KEYCATCH_UI );
      Menus_ActivateByName( arg1 );
      return qtrue;
    }
  }

  if( Q_stricmp ( cmd, "closemenus" ) == 0 )
  {
    if( Menu_Count( ) > 0 )
    {
      Menus_CloseAll( qfalse );
      if( Menu_Count( ) == 0 )
      {
        trap_Key_SetCatcher( trap_Key_GetCatcher( ) & ~KEYCATCH_UI );
        trap_Key_ClearStates( );
        trap_Cvar_Set( "cl_paused", "0" );
      }
    }
    return qtrue;    
  }

  return qfalse;
}

void UI_DrawNamedPic( float x, float y, float width, float height, const char *picname )
{
  qhandle_t  hShader;

  hShader = trap_R_RegisterShaderNoMip( picname );
  UI_AdjustFrom640( &x, &y, &width, &height );
  trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, hShader );
}

void UI_DrawHandlePic( float x, float y, float w, float h, qhandle_t hShader )
{
  float  s0;
  float  s1;
  float  t0;
  float  t1;

  if( w < 0 )
  {  // flip about vertical
    w  = -w;
    s0 = 1;
    s1 = 0;
  }
  else
  {
    s0 = 0;
    s1 = 1;
  }

  if( h < 0 )
  {  // flip about horizontal
    h  = -h;
    t0 = 1;
    t1 = 0;
  }
  else
  {
    t0 = 0;
    t1 = 1;
  }

  UI_AdjustFrom640( &x, &y, &w, &h );
  trap_R_DrawStretchPic( x, y, w, h, s0, t0, s1, t1, hShader );
}

/*
================
UI_FillRect
 
Coordinates are 640*480 virtual values
=================
*/
void UI_FillRect( float x, float y, float width, float height, const float *color )
{
  trap_R_SetColor( color );

  UI_AdjustFrom640( &x, &y, &width, &height );
  trap_R_DrawStretchPic( x, y, width, height, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );

  trap_R_SetColor( NULL );
}

void UI_SetColor( const float *rgba )
{
  trap_R_SetColor( rgba );
}
