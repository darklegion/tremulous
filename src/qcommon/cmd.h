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

#ifndef CMD_H
#define CMD_H

/*
==============================================================

CMD

Command text buffering and command execution

==============================================================
*/

/*

Any number of commands can be added in a frame, from several different sources.
Most commands come from either keybindings or console line input, but entire text
files can be execed.

*/

void Cbuf_Init(void);
// allocates an initial text buffer that will grow as needed

void Cbuf_AddText(const char *text);
// Adds command text at the end of the buffer, does NOT add a final \n

void Cbuf_ExecuteText(int exec_when, const char *text);
// this can be used in place of either Cbuf_AddText or Cbuf_InsertText

void Cbuf_Execute(void);
// Pulls off \n terminated lines of text from the command buffer and sends
// them through Cmd_ExecuteString.  Stops when the buffer is empty.
// Normally called once per frame, but may be explicitly invoked.
// Do not call inside a command function, or current args will be destroyed.

//===========================================================================

/*

Command execution takes a null terminated string, breaks it into tokens,
then searches for a command or variable that matches the first token.

*/

using xcommand_t = void(*)();

void Cmd_Init(void);

bool Cmd_CommadExists( const char *cmd_name );

void Cmd_AddCommand(const char *cmd_name, xcommand_t function);
// called by the init functions of other parts of the program to
// register commands and functions to call for them.
// The cmd_name is referenced later, so it should not be in temp memory
// if function is NULL, the command will be forwarded to the server
// as a clc_clientCommand instead of executed locally

void Cmd_RemoveCommand(const char *cmd_name);

typedef void (*completionFunc_t)(char *args, int argNum);

// don't allow VMs to remove system commands
void Cmd_RemoveCommandSafe(const char *cmd_name);

void Cmd_CommandCompletion(void (*callback)(const char *s));
// callback with each valid string
void Cmd_SetCommandCompletionFunc(const char *command, completionFunc_t complete);
void Cmd_CompleteArgument(const char *command, char *args, int argNum);
void Cmd_CompleteCfgName(char *args, int argNum);

int Cmd_Argc(void);
const char *Cmd_Argv(int arg);
void Cmd_ArgvBuffer(int arg, char *buffer, int bufferLength);
char *Cmd_Args(void);
char *Cmd_ArgsFrom(int arg);
void Cmd_ArgsBuffer(char *buffer, int bufferLength);
void Cmd_LiteralArgsBuffer(char *buffer, int bufferLength);
char *Cmd_Cmd(void);
// The functions that execute commands get their parameters with these
// functions. Cmd_Argv () will return an empty string, not a NULL
// if arg > argc, so string operations are allways safe.

void Cmd_TokenizeString(const char *text);
void Cmd_TokenizeStringIgnoreQuotes(const char *text_in);
// Takes a null terminated string.  Does not need to be /n terminated.
// breaks the string up into arg tokens.

void Cmd_ExecuteString(const char *text);
// Parses a single line of text into arguments and tries to execute it
// as if it was typed at the console

void Cmd_SaveCmdContext(void);
void Cmd_RestoreCmdContext(void);

#endif
