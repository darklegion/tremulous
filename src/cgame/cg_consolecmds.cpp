/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2013 Darklegion Development

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

#include "client/cl_main.h"
#include "qcommon/autocomplete.h"
#include "qcommon/cmd.h"

/*
=================
CG_SizeUp_f

Keybinding command
=================
*/
static void CG_SizeUp_f(void) { Cvar_Set("cg_viewsize", va("%i", MIN(cg_viewsize.integer + 10, 100))); }

/*
=================
CG_SizeDown_f

Keybinding command
=================
*/
static void CG_SizeDown_f(void) { Cvar_Set("cg_viewsize", va("%i", MAX(cg_viewsize.integer - 10, 30))); }

/*
=============
CG_Viewpos_f

Debugging command to print the current position
=============
*/
static void CG_Viewpos_f(void)
{
    CG_Printf("(%i %i %i) : %i\n", (int)cg.refdef.vieworg[0], (int)cg.refdef.vieworg[1], (int)cg.refdef.vieworg[2],
        (int)cg.refdefViewAngles[YAW]);
}

bool CG_RequestScores(void)
{
    if (cg.scoresRequestTime + 2000 < cg.time)
    {
        // the scores are more than two seconds out of data,
        // so request new ones
        cg.scoresRequestTime = cg.time;
        CL_AddReliableCommand("score\n", false);

        return true;
    }
    else
        return false;
}

extern menuDef_t *menuScoreboard;

static void CG_scrollScoresDown_f(void)
{
    if (menuScoreboard && cg.scoreBoardShowing)
    {
        Menu_ScrollFeeder(menuScoreboard, FEEDER_ALIENTEAM_LIST, true);
        Menu_ScrollFeeder(menuScoreboard, FEEDER_HUMANTEAM_LIST, true);
    }
}

static void CG_scrollScoresUp_f(void)
{
    if (menuScoreboard && cg.scoreBoardShowing)
    {
        Menu_ScrollFeeder(menuScoreboard, FEEDER_ALIENTEAM_LIST, false);
        Menu_ScrollFeeder(menuScoreboard, FEEDER_HUMANTEAM_LIST, false);
    }
}

static void CG_ScoresDown_f(void)
{
    if (!cg.showScores)
    {
        Menu_SetFeederSelection(menuScoreboard, FEEDER_ALIENTEAM_LIST, 0, NULL);
        Menu_SetFeederSelection(menuScoreboard, FEEDER_HUMANTEAM_LIST, 0, NULL);
        cg.showScores = true;
    }
    else
    {
        cg.showScores = false;
        cg.numScores = 0;
    }
}

static void CG_ScoresUp_f(void)
{
    if (cg.showScores)
    {
        cg.showScores = false;
        cg.scoreFadeTime = cg.time;
    }
}

void CG_ClientList_f(void)
{
    clientInfo_t *ci;
    int i;
    int count = 0;

    for (i = 0; i < MAX_CLIENTS; i++)
    {
        ci = &cgs.clientinfo[i];
        if (!ci->infoValid)
            continue;

        switch (ci->team)
        {
            case TEAM_ALIENS:
                Com_Printf("%2d " S_COLOR_RED "A   " S_COLOR_WHITE "%s\n", i, ci->name);
                break;

            case TEAM_HUMANS:
                Com_Printf("%2d " S_COLOR_CYAN "H   " S_COLOR_WHITE "%s\n", i, ci->name);
                break;

            default:
            case TEAM_NONE:
            case NUM_TEAMS:
                Com_Printf("%2d S   %s\n", i, ci->name);
                break;
        }

        count++;
    }

    Com_Printf("Listed %2d clients\n", count);
}

static void CG_VoiceMenu_f(void)
{
    char cmd[sizeof("voicemenu3")];

    Cmd_ArgvBuffer(0, cmd, sizeof(cmd));

    switch (cmd[9])
    {
        default:
        case '\0':
            Cvar_Set("ui_voicemenu", "1");
            break;
        case '2':
            Cvar_Set("ui_voicemenu", "2");
            break;
        case '3':
            Cvar_Set("ui_voicemenu", "3");
            break;
    };

    Cbuf_AddText("menu tremulous_voicecmd\n");
}

static void CG_UIMenu_f(void)
{
    Cbuf_AddText(va("menu \"%s\"\n", CG_Argv(1)));
}

static void CG_KillMessage_f(void)
{
    char msg1[33 * 3 + 1];
    char msg2[33 * 3 + 1];
    Cmd_ArgvBuffer(1, msg1, sizeof(msg1));
    Cmd_ArgvBuffer(2, msg2, sizeof(msg2));
    CG_AddToKillMsg(msg1, msg2, WP_GRENADE);
}

static consoleCommand_t commands[] = {{"+scores", CG_ScoresDown_f}, {"-scores", CG_ScoresUp_f},
    {"cgame_memory", BG_MemoryInfo}, {"clientlist", CG_ClientList_f}, {"destroyTestPS", CG_DestroyTestPS_f},
    {"destroyTestTS", CG_DestroyTestTS_f}, {"nextframe", CG_TestModelNextFrame_f}, {"nextskin", CG_TestModelNextSkin_f},
    {"prevframe", CG_TestModelPrevFrame_f}, {"prevskin", CG_TestModelPrevSkin_f}, {"scoresDown", CG_scrollScoresDown_f},
    {"scoresUp", CG_scrollScoresUp_f}, {"sizedown", CG_SizeDown_f}, {"sizeup", CG_SizeUp_f}, {"testgun", CG_TestGun_f},
    {"testmodel", CG_TestModel_f}, {"testPS", CG_TestPS_f}, {"testTS", CG_TestTS_f}, {"ui_menu", CG_UIMenu_f},
    {"viewpos", CG_Viewpos_f}, {"voicemenu", CG_VoiceMenu_f}, {"voicemenu2", CG_VoiceMenu_f},
    {"voicemenu3", CG_VoiceMenu_f}, {"weapnext", CG_NextWeapon_f}, {"weapon", CG_Weapon_f},
    {"weapprev", CG_PrevWeapon_f}, {"zcp", CG_CenterPrint_f}, {"zkill", CG_KillMessage_f}};

/*
=================
CG_ConsoleCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_ArgvBuffer()
=================
*/
bool CG_ConsoleCommand(void)
{
    consoleCommand_t *cmd;

    cmd = static_cast<consoleCommand_t *>(
        bsearch(CG_Argv(0), commands, ARRAY_LEN(commands), sizeof(commands[0]), cmdcmp));

    if (!cmd)
        return false;

    cmd->function();
    return true;
}

/*
==================
CG_CompleteCallVote_f
==================
*/
void CG_CompleteCallVote_f(int argNum)
{
    switch (argNum)
    {
        case 2:
            Field_CompleteList(
                "["
                "\"allowbuild\","
                "\"cancel\","
                "\"denybuild\","
                "\"draw\","
                "\"extend\","
                "\"kick\","
                "\"map\","
                "\"map_restart\","
                "\"mute\","
                "\"nextmap\","
                "\"poll\","
                "\"sudden_death\","
                "\"unmute\" ]");
            break;
    }
}

static consoleCommandCompletions_t commandCompletions[] = {{"callvote", CG_CompleteCallVote_f}};

/*
=================
CG_Console_CompleteArgument

Try to complete the client command line argument given in
argNum. Returns true if a completion function is found in CGAME,
otherwise client tries another completion method.

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_ArgvBuffer()
=================
*/
bool CG_Console_CompleteArgument(int argNum)
{
    consoleCommandCompletions_t *cmd;

    // Skip command prefix character
    cmd = static_cast<consoleCommandCompletions_t *>(bsearch(
        CG_Argv(0) + 1, commandCompletions, ARRAY_LEN(commandCompletions), sizeof(commandCompletions[0]), cmdcmp));

    if (!cmd)
        return false;

    cmd->function(argNum);
    return true;
}

/*
=================
CG_InitConsoleCommands

Let the client system know about all of our commands
so it can perform tab completion
=================
*/
void CG_InitConsoleCommands(void)
{
    int i;

    for (i = 0; i < ARRAY_LEN(commands); i++)
        Cmd_AddCommand(commands[i].cmd, nullptr);

    //
    // the game server will interpret these commands, which will be automatically
    // forwarded to the server after they are not recognized locally
    //
    Cmd_AddCommand("kill", nullptr);
    Cmd_AddCommand("ui_messagemode", nullptr);
    Cmd_AddCommand("ui_messagemode2", nullptr);
    Cmd_AddCommand("ui_messagemode3", nullptr);
    Cmd_AddCommand("ui_messagemode4", nullptr);
    Cmd_AddCommand("say", nullptr);
    Cmd_AddCommand("say_team", nullptr);
    Cmd_AddCommand("vsay", nullptr);
    Cmd_AddCommand("vsay_team", nullptr);
    Cmd_AddCommand("vsay_local", nullptr);
    Cmd_AddCommand("m", nullptr);
    Cmd_AddCommand("mt", nullptr);
    Cmd_AddCommand("give", nullptr);
    Cmd_AddCommand("god", nullptr);
    Cmd_AddCommand("notarget", nullptr);
    Cmd_AddCommand("noclip", nullptr);
    Cmd_AddCommand("team", nullptr);
    Cmd_AddCommand("follow", nullptr);
    Cmd_AddCommand("setviewpos", nullptr);
    Cmd_AddCommand("callvote", nullptr);
    Cmd_AddCommand("vote", nullptr);
    Cmd_AddCommand("callteamvote", nullptr);
    Cmd_AddCommand("teamvote", nullptr);
    Cmd_AddCommand("class", nullptr);
    Cmd_AddCommand("build", nullptr);
    Cmd_AddCommand("buy", nullptr);
    Cmd_AddCommand("sell", nullptr);
    Cmd_AddCommand("reload", nullptr);
    Cmd_AddCommand("itemact", nullptr);
    Cmd_AddCommand("itemdeact", nullptr);
    Cmd_AddCommand("itemtoggle", nullptr);
    Cmd_AddCommand("destroy", nullptr);
    Cmd_AddCommand("deconstruct", nullptr);
    Cmd_AddCommand("ignore", nullptr);
    Cmd_AddCommand("unignore", nullptr);
}
