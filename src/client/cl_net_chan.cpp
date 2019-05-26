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

#include "client.h"

#include "qcommon/q_shared.h"
#include "qcommon/qcommon.h"

/*
==============
CL_Netchan_Encode

        // first 12 bytes of the data are always:
        long serverId;
        long messageAcknowledge;
        long reliableAcknowledge;

==============
*/
static void CL_Netchan_Encode(msg_t *msg)
{
    int serverId, messageAcknowledge, reliableAcknowledge;
    int i, idx, srdc, sbit;
    bool soob;
    byte key, *string;

    if (msg->cursize <= CL_ENCODE_START)
    {
        return;
    }

    srdc = msg->readcount;
    sbit = msg->bit;
    soob = msg->oob;

    msg->bit = 0;
    msg->readcount = 0;
    msg->oob = false;

    serverId = MSG_ReadLong(msg);
    messageAcknowledge = MSG_ReadLong(msg);
    reliableAcknowledge = MSG_ReadLong(msg);

    msg->oob = soob;
    msg->bit = sbit;
    msg->readcount = srdc;

    string = (byte *)clc.serverCommands[reliableAcknowledge & (MAX_RELIABLE_COMMANDS - 1)];
    idx = 0;
    //
    key = clc.challenge ^ serverId ^ messageAcknowledge;
    for (i = CL_ENCODE_START; i < msg->cursize; i++)
    {
        // modify the key with the last received now acknowledged server command
        if (!string[idx]) idx = 0;
        if (string[idx] > 127 || (string[idx] == '%' && clc.netchan.alternateProtocol == 2))
        {
            key ^= '.' << (i & 1);
        }
        else
        {
            key ^= string[idx] << (i & 1);
        }
        idx++;
        // encode the data with this key
        *(msg->data + i) = (*(msg->data + i)) ^ key;
    }
}

/*
==============
CL_Netchan_Decode

        // first four bytes of the data are always:
        long reliableAcknowledge;

==============
*/
static void CL_Netchan_Decode(msg_t *msg)
{
    long reliableAcknowledge, i, idx;
    byte key, *string;
    int srdc, sbit;
    bool soob;

    srdc = msg->readcount;
    sbit = msg->bit;
    soob = msg->oob;

    msg->oob = false;

    reliableAcknowledge = MSG_ReadLong(msg);

    msg->oob = soob;
    msg->bit = sbit;
    msg->readcount = srdc;

    string = (byte *)clc.reliableCommands[reliableAcknowledge & (MAX_RELIABLE_COMMANDS - 1)];
    idx = 0;
    // xor the client challenge with the netchan sequence number (need something that changes every message)
    key = clc.challenge ^ LittleLong(*(unsigned *)msg->data);
    for (i = msg->readcount + CL_DECODE_START; i < msg->cursize; i++)
    {
        // modify the key with the last sent and with this message acknowledged client command
        if (!string[idx]) idx = 0;
        if (string[idx] > 127 || (string[idx] == '%' && clc.netchan.alternateProtocol == 2))
        {
            key ^= '.' << (i & 1);
        }
        else
        {
            key ^= string[idx] << (i & 1);
        }
        idx++;
        // decode the data with this key
        *(msg->data + i) = *(msg->data + i) ^ key;
    }
}

/*
=================
CL_Netchan_TransmitNextFragment
=================
*/
static bool CL_Netchan_TransmitNextFragment(netchan_t *chan)
{
    if (chan->unsentFragments)
    {
        Netchan_TransmitNextFragment(chan);
        return true;
    }

    return false;
}

/*
===============
CL_Netchan_Transmit
================
*/
void CL_Netchan_Transmit(netchan_t *chan, msg_t *msg)
{
    MSG_WriteByte(msg, clc_EOF);

    if (chan->alternateProtocol != 0) CL_Netchan_Encode(msg);
    Netchan_Transmit(chan, msg->cursize, msg->data);

    // Transmit all fragments without delay
    while (CL_Netchan_TransmitNextFragment(chan))
    {
        Com_DPrintf("WARNING: #462 unsent fragments (not supposed to happen!)\n");
    }
}

/*
=================
CL_Netchan_Process
=================
*/
bool CL_Netchan_Process(netchan_t *chan, msg_t *msg)
{
    int ret;

    ret = Netchan_Process(chan, msg);
    if (!ret) return false;
    if (chan->alternateProtocol != 0) CL_Netchan_Decode(msg);

    return true;
}
