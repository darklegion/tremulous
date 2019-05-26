/*
 * This file is part of Tremulous.
 * Copyright © 2017 Victor Roemer (blowfish) <victor@badsec.org>
 * Copyright (C) 2015-2019 GrangerHub
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License,  or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not,  see <http://www.gnu.org/licenses/>.
 */

#ifndef CVAR_H
#define CVAR_H

#include "q_platform.h"
#include "q_shared.h"

/*
==========================================================

CVARS (console variables)

Many variables can be used for cheating purposes, so when
cheats is zero, force all unspecified variables to their
default values.
==========================================================
*/

#define CVAR_ARCHIVE        0x0001  // set to cause it to be saved to vars.rc
// used for system variables, not for player
// specific configurations
#define CVAR_USERINFO       0x0002  // sent to server on connect or change
#define CVAR_SERVERINFO     0x0004  // sent in response to front end requests
#define CVAR_SYSTEMINFO     0x0008  // these cvars will be duplicated on all clients
#define CVAR_INIT           0x0010  // don't allow change from console at all,
// but can be set from the command line
#define CVAR_LATCH          0x0020  // will only change when C code next does
// a Cvar_Get(), so it can't be changed without proper initialization.
// modified will be set, even though the value hasn't changed yet
#define CVAR_ROM            0x0040  // display only, cannot be set by user at all
#define CVAR_USER_CREATED   0x0080  // created by a set command
#define CVAR_TEMP           0x0100  // can be set even when cheats are disabled, but is not archived
#define CVAR_CHEAT          0x0200  // can not be changed if cheats are disabled
#define CVAR_NORESTART      0x0400  // do not clear when a cvar_restart is issued

#define CVAR_SERVER_CREATED 0x0800  // cvar was created by a server the client connected to.
#define CVAR_VM_CREATED     0x1000  // cvar was created exclusively in one of the VMs.
#define CVAR_PROTECTED      0x2000  // prevent modifying this var from VMs or the server
#define CVAR_ALTERNATE_SYSTEMINFO 0x1000000
// These flags are only returned by the Cvar_Flags() function
#define CVAR_MODIFIED             0x40000000  // Cvar was modified
#define CVAR_NONEXISTENT          0x80000000  // Cvar doesn't exist.

// nothing outside the Cvar_*() functions should modify these fields!
typedef struct cvar_s cvar_t;

struct cvar_s {
    char *name;
    char *string;
    char *resetString;  // cvar_restart will reset to this value
    char *latchedString;  // for CVAR_LATCH vars
    int flags;
    bool modified;  // set each time the cvar is changed
    int modificationCount;  // incremented each time the cvar is changed
    float value;  // atof( string )
    int integer;  // atoi( string )
    bool validate;
    bool integral;
    float min;
    float max;
    char *description;

    cvar_t *next;
    cvar_t *prev;
    cvar_t *hashNext;
    cvar_t *hashPrev;
    int hashIndex;
};

/*
==============================================================

CVAR

==============================================================
*/

/*

cvar_t variables are used to hold scalar or string variables that can be changed
or displayed at the console or prog code as well as accessed directly
in C code.

The user can access cvars from the console in three ways:
r_draworder			prints the current value
r_draworder 0		sets the current value to 0
set r_draworder 0	as above, but creates the cvar if not present

Cvars are restricted from having the same names as commands to keep this
interface from being ambiguous.

The are also occasionally used to communicated information between different
modules of the program.

*/

cvar_t *Cvar_Get(const char *var_name, const char *value, int flags);
// creates the variable if it doesn't exist, or returns the existing one
// if it exists, the value will not be changed, but flags will be ORed in
// that allows variables to be unarchived without needing bitflags
// if value is "", the value will not override a previously set value.

void Cvar_Register(vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags);
// basically a slightly modified Cvar_Get for the interpreted modules

void Cvar_Update(vmCvar_t *vmCvar);
// updates an interpreted modules' version of a cvar

void Cvar_Set(const char *var_name, const char *value);
// will create the variable with no flags if it doesn't exist

cvar_t *Cvar_Set2(const char *var_name, const char *value, bool force);
// same as Cvar_Set, but allows more control over setting of cvar

void Cvar_SetSafe(const char *var_name, const char *value);
// sometimes we set variables from an untrusted source: fail if flags & CVAR_PROTECTED

void Cvar_SetLatched(const char *var_name, const char *value);
// don't set the cvar immediately

void Cvar_SetValue(const char *var_name, float value);
void Cvar_SetValueSafe(const char *var_name, float value);
// expands value to a string and calls Cvar_Set/Cvar_SetSafe

// Validate String used to validate cvar names
bool Cvar_ValidateString(const char *s);
cvar_t *Cvar_FindVar(const char *var_name);
const char *Cvar_Validate(cvar_t *var, const char *value, bool warn);
void Cvar_Print(cvar_t *v);

float Cvar_VariableValue(const char *var_name);
int Cvar_VariableIntegerValue(const char *var_name);
// returns 0 if not defined or non numeric

const char *Cvar_VariableString(const char *var_name);
void Cvar_VariableStringBuffer(const char *var_name, char *buffer, int bufsize);
// returns an empty string if not defined

unsigned int Cvar_Flags(const char *var_name);
// returns CVAR_NONEXISTENT if cvar doesn't exist or the flags of that particular CVAR.

void Cvar_CommandCompletion(void (*callback)(const char *s));
// callback with each valid string

void Cvar_Reset(const char *var_name);
void Cvar_ForceReset(const char *var_name);

void Cvar_SetCheatState(void);
// reset all testing vars to a safe value

bool Cvar_Command(void);
// called by Cmd_ExecuteString when Cmd_Argv(0) doesn't match a known
// command.  Returns true if the command was a variable reference that
// was handled. (print or change)

void Cvar_WriteVariables(fileHandle_t f);
// writes lines containing "set variable value" for all variables
// with the archive flag set to true.

void Cvar_Init(void);

char *Cvar_InfoString(int bit);
char *Cvar_InfoString_Big(int bit);
// returns an info string containing all the cvars that have the given bit set
// in their flags ( CVAR_USERINFO, CVAR_SERVERINFO, CVAR_SYSTEMINFO, etc )
void Cvar_InfoStringBuffer(int bit, char *buff, int buffsize);
void Cvar_CheckRange(cvar_t *cv, float minVal, float maxVal, bool shouldBeIntegral);
void Cvar_SetDescription(cvar_t *var, const char *var_description);

void Cvar_Restart(bool unsetVM);
void Cvar_Restart_f(void);

void Cvar_CompleteCvarName(char *args, int argNum);

extern int cvar_modifiedFlags;
// whenever a cvar is modifed, its flags will be OR'd into this, so
// a single check can determine if any CVAR_USERINFO, CVAR_SERVERINFO,
// etc, variables have been modified since the last check.  The bit
// can then be cleared to allow another change detection.

#endif
