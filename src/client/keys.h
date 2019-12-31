/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2013 Darklegion Development
Copyright (C) 2015-2019 GrangerHub

This file is part of Tremulous.

Tremulous is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 3 of the License,
or (at your option) any later version.

Tremulous is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tremulous; if not, see <https://www.gnu.org/licenses/>

===========================================================================
*/
#ifndef _KEYS_H_
#define _KEYS_H_

#include "keycodes.h"

typedef struct {
    bool down;
    int repeats;  // if > 1, it is autorepeating
    char *binding;
} qkey_t;

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
extern bool chat_admins;
extern bool chat_clans;
extern int chat_playerNum;

void Key_WriteBindings(fileHandle_t f);
void Key_SetBinding(int keynum, const char *binding);
const char *Key_GetBinding(int keynum);
bool Key_IsDown(int keynum);
bool Key_GetOverstrikeMode(void);
void Key_SetOverstrikeMode(bool state);
void Key_ClearStates(void);
int Key_GetKey(const char *binding);
void Key_KeynumToStringBuf(int keynum, char *buf, int buflen);
void Key_GetBindingBuf(int keynum, char *buf, int buflen);

#endif
