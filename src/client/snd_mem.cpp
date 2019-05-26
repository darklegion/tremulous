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

/*****************************************************************************
 * name:		snd_mem.c
 *
 * desc:		sound caching
 *****************************************************************************/

#include "snd_codec.h"
#include "snd_local.h"

#include "qcommon/cvar.h"

#define DEF_COMSOUNDMEGS "8"

/*
===============================================================================

memory management

===============================================================================
*/

static sndBuffer *buffer = NULL;
static sndBuffer *freelist = NULL;
static int inUse = 0;
static int totalInUse = 0;

short *sfxScratchBuffer = NULL;
sfx_t *sfxScratchPointer = NULL;
int sfxScratchIndex = 0;

void SND_free(sndBuffer *v)
{
    *(sndBuffer **)v = freelist;
    freelist = (sndBuffer *)v;
    inUse += sizeof(sndBuffer);
}

sndBuffer *SND_malloc(void)
{
    sndBuffer *v;
redo:
    if (freelist == NULL)
    {
        S_FreeOldestSound();
        goto redo;
    }

    inUse -= sizeof(sndBuffer);
    totalInUse += sizeof(sndBuffer);

    v = freelist;
    freelist = *(sndBuffer **)freelist;
    v->next = NULL;

    return v;
}

void SND_setup(void)
{
    cvar_t* cv = Cvar_Get("com_soundMegs", 
            DEF_COMSOUNDMEGS, CVAR_LATCH | CVAR_ARCHIVE);
    int scs = cv->integer * 1536;

    buffer = (sndBuffer*)malloc(scs * sizeof(sndBuffer));

    // allocate the stack based hunk allocator
    sfxScratchBuffer = (short*)malloc(SND_CHUNK_SIZE * sizeof(short) * 4);
    sfxScratchPointer = NULL;

    inUse = scs * sizeof(sndBuffer);
    sndBuffer* p = buffer;
    sndBuffer* q = p + scs;
    while (--q > p)
    {
        *(sndBuffer **)q = q - 1;
    }

    *(sndBuffer **)q = NULL;
    freelist = p + scs - 1;

    Com_Printf("Sound memory manager started\n");
}

void SND_shutdown(void)
{
    free(sfxScratchBuffer);
    free(buffer);
}

/*
================
ResampleSfx

resample / decimate to the current source rate
================
*/
static int ResampleSfx(sfx_t *sfx, int channels, int inrate, int inwidth, int samples, byte *data, bool compressed)
{
    float stepscale = (float)inrate / dma.speed;  // this is usually 0.5, 1, or 2
    int outcount = samples / stepscale;
    int fracstep = stepscale * 256 * channels;
    sndBuffer* chunk = sfx->soundData;

    int samplefrac = 0;
    int srcsample = 0;

    for (int i = 0; i < outcount; i++)
    {
        srcsample += samplefrac >> 8;
        samplefrac &= 255;
        samplefrac += fracstep;

        for (int j = 0; j < channels; j++)
        {
            int sample;

            if (inwidth == 2)
            {
                sample = ((short *)data)[srcsample + j];
            }
            else
            {
                sample = (int)((unsigned char)(data[srcsample + j]) - 128) << 8;
            }

            int part = (i * channels + j) & (SND_CHUNK_SIZE - 1);
            if (part == 0)
            {
                sndBuffer *newchunk = SND_malloc();

                if (chunk == NULL)
                {
                    sfx->soundData = newchunk;
                }
                else
                {
                    chunk->next = newchunk;
                }
                chunk = newchunk;
            }

            chunk->sndChunk[part] = sample;
        }
    }

    return outcount;
}

/*
================
ResampleSfxRaw

resample / decimate to the current source rate
================
*/
static int ResampleSfxRaw(short *sfx, int channels, int inrate, int inwidth, int samples, byte *data)
{
    float stepscale = (float)inrate / dma.speed;  // this is usually 0.5, 1, or 2
    int outcount = samples / stepscale;
    int fracstep = stepscale * 256 * channels;

    int samplefrac = 0;
    int srcsample = 0;

    for (int i = 0; i < outcount; i++)
    {
        srcsample += samplefrac >> 8;
        srcsample &= 255;
        samplefrac += fracstep;

        for (int j = 0; j < channels; j++)
        {
            int sample;
            if (inwidth == 2)
            {
                sample = LittleShort(((short *)data)[srcsample + j]);
            }
            else
            {
                sample = (int)((unsigned char)(data[srcsample + j]) - 128) << 8;
            }
            sfx[i * channels + j] = sample;
        }
    }
    return outcount;
}

//=============================================================================

/*
==============
S_LoadSound

The filename may be different than sfx->name in the case
of a forced fallback of a player specific sound
==============
*/
bool S_LoadSound(sfx_t *sfx)
{
    snd_info_t info;
    byte *data = (byte*)S_CodecLoad(sfx->soundName, &info);
    if (!data)
    {
        return false;
    }

    int size_per_sec = info.rate * info.channels * info.width;
    if (size_per_sec > 0)
    {
        sfx->duration = (int)(1000.0f * ((double)info.size / size_per_sec));
    }

    if (info.width == 1)
    {
        Com_DPrintf(S_COLOR_YELLOW "WARNING: %s is a 8 bit audio file\n",
                sfx->soundName);
    }

    if (info.rate != 22050)
    {
        Com_DPrintf(S_COLOR_YELLOW "WARNING: %s is not a 22kHz audio file\n",
                sfx->soundName);
    }

    short* samples = (short*)Hunk_AllocateTempMemory(info.channels * info.samples * sizeof(short) * 2);

    sfx->lastTimeUsed = Com_Milliseconds() + 1;

    // each of these compression schemes works just fine
    // but the 16bit quality is much nicer and with a local
    // install assured we can rely upon the sound memory
    // manager to do the right thing for us and page
    // sound in as needed

    if (info.channels == 1 && sfx->soundCompressed == true)
    {
        sfx->soundCompressionMethod = 1;
        sfx->soundData = NULL;
        sfx->soundLength = ResampleSfxRaw(
                    samples,
                    info.channels,
                    info.rate,
                    info.width,
                    info.samples,
                    data + info.dataofs);
        S_AdpcmEncodeSound(sfx, samples);
    }
    else
    {
        sfx->soundCompressionMethod = 0;
        sfx->soundData = NULL;
        sfx->soundLength = ResampleSfx(
                    sfx,
                    info.channels,
                    info.rate,
                    info.width,
                    info.samples,
                    data + info.dataofs,
                    false);
    }

    sfx->soundChannels = info.channels;

    Hunk_FreeTempMemory(samples);
    Hunk_FreeTempMemory(data);

    return true;
}

void S_DisplayFreeMemory(void)
{ 
    Com_Printf("%d bytes free sound buffer memory, %d total used\n", inUse, totalInUse);
}
