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
#ifndef _KEYS_H_
#define _KEYS_H_

#include "qcommon/q_shared.h"
#include "keycodes.h"

struct qkey_t {
    bool down;
    int repeats;  // if > 1, it is autorepeating
    char *binding;
};

struct field_t;

extern bool key_overstrikeMode;
extern qkey_t keys[MAX_KEYS];

// NOTE TTimo the declaration of field_t and Field_Clear is now in qcommon/qcommon.h
void Field_KeyDownEvent(field_t *edit, int key);
void Field_CharEvent(field_t *edit, int ch);
void Field_Draw(field_t *edit, int x, int y, int width, bool showCursor, bool noColorEscape);
void Field_BigDraw(field_t *edit, int x, int y, int width, bool showCursor, bool noColorEscape);

#define COMMAND_HISTORY 32
extern field_t historyEditLines[COMMAND_HISTORY];
extern field_t g_consoleField;
extern field_t chatField;
extern int anykeydown;
extern bool chat_team;
extern int chat_playerNum;

void Key_WriteBindings(fileHandle_t f);
int Key_GetKey(const char *binding);

SO_PUBLIC bool Key_GetOverstrikeMode(void);
SO_PUBLIC bool Key_IsDown(int keynum);
SO_PUBLIC const char *Key_GetBinding(int keynum);
SO_PUBLIC int Key_GetCatcher(void);
SO_PUBLIC void Key_ClearStates(void);
SO_PUBLIC void Key_GetBindingBuf(int keynum, char *buf, int buflen);
SO_PUBLIC void Key_KeynumToStringBuf(int keynum, char *buf, int buflen);
SO_PUBLIC void Key_SetBinding(int keynum, const char *binding);
SO_PUBLIC void Key_SetCatcher(int catcher);
SO_PUBLIC void Key_SetOverstrikeMode(bool state);

#endif
