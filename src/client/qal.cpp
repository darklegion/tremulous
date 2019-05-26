/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2013 Darklegion Development
Copyright (C) 2005 Stuart Dalton (badcdev@gmail.com)
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

// Dynamically loads OpenAL

#ifdef USE_OPENAL

#include "qal.h"

#ifdef USE_OPENAL_DLOPEN
#include "sys/sys_loadlib.h"

LPALENABLE qalEnable;
LPALDISABLE qalDisable;
LPALISENABLED qalIsEnabled;
LPALGETSTRING qalGetString;
LPALGETBOOLEANV qalGetBooleanv;
LPALGETINTEGERV qalGetIntegerv;
LPALGETFLOATV qalGetFloatv;
LPALGETDOUBLEV qalGetDoublev;
LPALGETBOOLEAN qalGetBoolean;
LPALGETINTEGER qalGetInteger;
LPALGETFLOAT qalGetFloat;
LPALGETDOUBLE qalGetDouble;
LPALGETERROR qalGetError;
LPALISEXTENSIONPRESENT qalIsExtensionPresent;
LPALGETPROCADDRESS qalGetProcAddress;
LPALGETENUMVALUE qalGetEnumValue;
LPALLISTENERF qalListenerf;
LPALLISTENER3F qalListener3f;
LPALLISTENERFV qalListenerfv;
LPALLISTENERI qalListeneri;
LPALGETLISTENERF qalGetListenerf;
LPALGETLISTENER3F qalGetListener3f;
LPALGETLISTENERFV qalGetListenerfv;
LPALGETLISTENERI qalGetListeneri;
LPALGENSOURCES qalGenSources;
LPALDELETESOURCES qalDeleteSources;
LPALISSOURCE qalIsSource;
LPALSOURCEF qalSourcef;
LPALSOURCE3F qalSource3f;
LPALSOURCEFV qalSourcefv;
LPALSOURCEI qalSourcei;
LPALGETSOURCEF qalGetSourcef;
LPALGETSOURCE3F qalGetSource3f;
LPALGETSOURCEFV qalGetSourcefv;
LPALGETSOURCEI qalGetSourcei;
LPALSOURCEPLAYV qalSourcePlayv;
LPALSOURCESTOPV qalSourceStopv;
LPALSOURCEREWINDV qalSourceRewindv;
LPALSOURCEPAUSEV qalSourcePausev;
LPALSOURCEPLAY qalSourcePlay;
LPALSOURCESTOP qalSourceStop;
LPALSOURCEREWIND qalSourceRewind;
LPALSOURCEPAUSE qalSourcePause;
LPALSOURCEQUEUEBUFFERS qalSourceQueueBuffers;
LPALSOURCEUNQUEUEBUFFERS qalSourceUnqueueBuffers;
LPALGENBUFFERS qalGenBuffers;
LPALDELETEBUFFERS qalDeleteBuffers;
LPALISBUFFER qalIsBuffer;
LPALBUFFERDATA qalBufferData;
LPALGETBUFFERF qalGetBufferf;
LPALGETBUFFERI qalGetBufferi;
LPALDOPPLERFACTOR qalDopplerFactor;
LPALSPEEDOFSOUND qalSpeedOfSound;
LPALDISTANCEMODEL qalDistanceModel;

LPALCCREATECONTEXT qalcCreateContext;
LPALCMAKECONTEXTCURRENT qalcMakeContextCurrent;
LPALCPROCESSCONTEXT qalcProcessContext;
LPALCSUSPENDCONTEXT qalcSuspendContext;
LPALCDESTROYCONTEXT qalcDestroyContext;
LPALCGETCURRENTCONTEXT qalcGetCurrentContext;
LPALCGETCONTEXTSDEVICE qalcGetContextsDevice;
LPALCOPENDEVICE qalcOpenDevice;
LPALCCLOSEDEVICE qalcCloseDevice;
LPALCGETERROR qalcGetError;
LPALCISEXTENSIONPRESENT qalcIsExtensionPresent;
LPALCGETPROCADDRESS qalcGetProcAddress;
LPALCGETENUMVALUE qalcGetEnumValue;
LPALCGETSTRING qalcGetString;
LPALCGETINTEGERV qalcGetIntegerv;
LPALCCAPTUREOPENDEVICE qalcCaptureOpenDevice;
LPALCCAPTURECLOSEDEVICE qalcCaptureCloseDevice;
LPALCCAPTURESTART qalcCaptureStart;
LPALCCAPTURESTOP qalcCaptureStop;
LPALCCAPTURESAMPLES qalcCaptureSamples;

static void *OpenALLib = NULL;

static bool alinit_fail = false;

/*
=================
GPA
=================
*/
static void *GPA(const char *str)
{
	void *rv;

	rv = Sys_LoadFunction(OpenALLib, str);
	if(!rv)
	{
		Com_Printf( " Can't load symbol %s\n", str);
		alinit_fail = true;
		return NULL;
	}
	else
	{
		Com_DPrintf( " Loaded symbol %s (%p)\n", str, rv);
        return rv;
	}
}

/*
=================
QAL_Init
=================
*/
bool QAL_Init(const char *libname)
{
	if(OpenALLib)
		return true;

	if(!(OpenALLib = Sys_LoadDll(libname, true)))
		return false;

	alinit_fail = false;

	qalEnable = (LPALENABLE)GPA("alEnable");
	qalDisable = (LPALDISABLE)GPA("alDisable");
	qalIsEnabled = (LPALISENABLED)GPA("alIsEnabled");
	qalGetString = (LPALGETSTRING)GPA("alGetString");
	qalGetBooleanv = (LPALGETBOOLEANV)GPA("alGetBooleanv");
	qalGetIntegerv = (LPALGETINTEGERV)GPA("alGetIntegerv");
	qalGetFloatv = (LPALGETFLOATV)GPA("alGetFloatv");
	qalGetDoublev = (LPALGETDOUBLEV)GPA("alGetDoublev");
	qalGetBoolean = (LPALGETBOOLEAN)GPA("alGetBoolean");
	qalGetInteger = (LPALGETINTEGER)GPA("alGetInteger");
	qalGetFloat = (LPALGETFLOAT)GPA("alGetFloat");
	qalGetDouble = (LPALGETDOUBLE)GPA("alGetDouble");
	qalGetError = (LPALGETERROR)GPA("alGetError");
	qalIsExtensionPresent = (LPALISEXTENSIONPRESENT)GPA("alIsExtensionPresent");
	qalGetProcAddress = (LPALGETPROCADDRESS)GPA("alGetProcAddress");
	qalGetEnumValue = (LPALGETENUMVALUE)GPA("alGetEnumValue");
	qalListenerf = (LPALLISTENERF)GPA("alListenerf");
	qalListener3f = (LPALLISTENER3F)GPA("alListener3f");
	qalListenerfv = (LPALLISTENERFV)GPA("alListenerfv");
	qalListeneri = (LPALLISTENERI)GPA("alListeneri");
	qalGetListenerf = (LPALGETLISTENERF)GPA("alGetListenerf");
	qalGetListener3f = (LPALGETLISTENER3F)GPA("alGetListener3f");
	qalGetListenerfv = (LPALGETLISTENERFV)GPA("alGetListenerfv");
	qalGetListeneri = (LPALGETLISTENERI)GPA("alGetListeneri");
	qalGenSources = (LPALGENSOURCES)GPA("alGenSources");
	qalDeleteSources = (LPALDELETESOURCES)GPA("alDeleteSources");
	qalIsSource = (LPALISSOURCE)GPA("alIsSource");
	qalSourcef = (LPALSOURCEF)GPA("alSourcef");
	qalSource3f = (LPALSOURCE3F)GPA("alSource3f");
	qalSourcefv = (LPALSOURCEFV)GPA("alSourcefv");
	qalSourcei = (LPALSOURCEI)GPA("alSourcei");
	qalGetSourcef = (LPALGETSOURCEF)GPA("alGetSourcef");
	qalGetSource3f = (LPALGETSOURCE3F)GPA("alGetSource3f");
	qalGetSourcefv = (LPALGETSOURCEFV)GPA("alGetSourcefv");
	qalGetSourcei = (LPALGETSOURCEI)GPA("alGetSourcei");
	qalSourcePlayv = (LPALSOURCEPLAYV)GPA("alSourcePlayv");
	qalSourceStopv = (LPALSOURCESTOPV)GPA("alSourceStopv");
	qalSourceRewindv = (LPALSOURCEREWINDV)GPA("alSourceRewindv");
	qalSourcePausev = (LPALSOURCEPAUSEV)GPA("alSourcePausev");
	qalSourcePlay = (LPALSOURCEPLAY)GPA("alSourcePlay");
	qalSourceStop = (LPALSOURCESTOP)GPA("alSourceStop");
	qalSourceRewind = (LPALSOURCEREWIND)GPA("alSourceRewind");
	qalSourcePause = (LPALSOURCEPAUSE)GPA("alSourcePause");
	qalSourceQueueBuffers = (LPALSOURCEQUEUEBUFFERS)GPA("alSourceQueueBuffers");
	qalSourceUnqueueBuffers = (LPALSOURCEUNQUEUEBUFFERS)GPA("alSourceUnqueueBuffers");
	qalGenBuffers = (LPALGENBUFFERS)GPA("alGenBuffers");
	qalDeleteBuffers = (LPALDELETEBUFFERS)GPA("alDeleteBuffers");
	qalIsBuffer = (LPALISBUFFER)GPA("alIsBuffer");
	qalBufferData = (LPALBUFFERDATA)GPA("alBufferData");
	qalGetBufferf = (LPALGETBUFFERF)GPA("alGetBufferf");
	qalGetBufferi = (LPALGETBUFFERI)GPA("alGetBufferi");
	qalDopplerFactor = (LPALDOPPLERFACTOR)GPA("alDopplerFactor");
	qalSpeedOfSound = (LPALSPEEDOFSOUND)GPA("alSpeedOfSound");
	qalDistanceModel = (LPALDISTANCEMODEL)GPA("alDistanceModel");

	qalcCreateContext = (LPALCCREATECONTEXT)GPA("alcCreateContext");
	qalcMakeContextCurrent = (LPALCMAKECONTEXTCURRENT)GPA("alcMakeContextCurrent");
	qalcProcessContext = (LPALCPROCESSCONTEXT)GPA("alcProcessContext");
	qalcSuspendContext = (LPALCSUSPENDCONTEXT)GPA("alcSuspendContext");
	qalcDestroyContext = (LPALCDESTROYCONTEXT)GPA("alcDestroyContext");
	qalcGetCurrentContext = (LPALCGETCURRENTCONTEXT)GPA("alcGetCurrentContext");
	qalcGetContextsDevice = (LPALCGETCONTEXTSDEVICE)GPA("alcGetContextsDevice");
	qalcOpenDevice = (LPALCOPENDEVICE)GPA("alcOpenDevice");
	qalcCloseDevice = (LPALCCLOSEDEVICE)GPA("alcCloseDevice");
	qalcGetError = (LPALCGETERROR)GPA("alcGetError");
	qalcIsExtensionPresent = (LPALCISEXTENSIONPRESENT)GPA("alcIsExtensionPresent");
	qalcGetProcAddress = (LPALCGETPROCADDRESS)GPA("alcGetProcAddress");
	qalcGetEnumValue = (LPALCGETENUMVALUE)GPA("alcGetEnumValue");
	qalcGetString = (LPALCGETSTRING)GPA("alcGetString");
	qalcGetIntegerv = (LPALCGETINTEGERV)GPA("alcGetIntegerv");
	qalcCaptureOpenDevice = (LPALCCAPTUREOPENDEVICE)GPA("alcCaptureOpenDevice");
	qalcCaptureCloseDevice = (LPALCCAPTURECLOSEDEVICE)GPA("alcCaptureCloseDevice");
	qalcCaptureStart = (LPALCCAPTURESTART)GPA("alcCaptureStart");
	qalcCaptureStop = (LPALCCAPTURESTOP)GPA("alcCaptureStop");
	qalcCaptureSamples = (LPALCCAPTURESAMPLES)GPA("alcCaptureSamples");

	if(alinit_fail)
	{
		QAL_Shutdown();
		Com_Printf( " One or more symbols not found\n");
		return false;
	}

	return true;
}

/*
=================
QAL_Shutdown
=================
*/
void QAL_Shutdown( void )
{
	if(OpenALLib)
	{
		Sys_UnloadLibrary(OpenALLib);
		OpenALLib = NULL;
	}

	qalEnable = NULL;
	qalDisable = NULL;
	qalIsEnabled = NULL;
	qalGetString = NULL;
	qalGetBooleanv = NULL;
	qalGetIntegerv = NULL;
	qalGetFloatv = NULL;
	qalGetDoublev = NULL;
	qalGetBoolean = NULL;
	qalGetInteger = NULL;
	qalGetFloat = NULL;
	qalGetDouble = NULL;
	qalGetError = NULL;
	qalIsExtensionPresent = NULL;
	qalGetProcAddress = NULL;
	qalGetEnumValue = NULL;
	qalListenerf = NULL;
	qalListener3f = NULL;
	qalListenerfv = NULL;
	qalListeneri = NULL;
	qalGetListenerf = NULL;
	qalGetListener3f = NULL;
	qalGetListenerfv = NULL;
	qalGetListeneri = NULL;
	qalGenSources = NULL;
	qalDeleteSources = NULL;
	qalIsSource = NULL;
	qalSourcef = NULL;
	qalSource3f = NULL;
	qalSourcefv = NULL;
	qalSourcei = NULL;
	qalGetSourcef = NULL;
	qalGetSource3f = NULL;
	qalGetSourcefv = NULL;
	qalGetSourcei = NULL;
	qalSourcePlayv = NULL;
	qalSourceStopv = NULL;
	qalSourceRewindv = NULL;
	qalSourcePausev = NULL;
	qalSourcePlay = NULL;
	qalSourceStop = NULL;
	qalSourceRewind = NULL;
	qalSourcePause = NULL;
	qalSourceQueueBuffers = NULL;
	qalSourceUnqueueBuffers = NULL;
	qalGenBuffers = NULL;
	qalDeleteBuffers = NULL;
	qalIsBuffer = NULL;
	qalBufferData = NULL;
	qalGetBufferf = NULL;
	qalGetBufferi = NULL;
	qalDopplerFactor = NULL;
	qalSpeedOfSound = NULL;
	qalDistanceModel = NULL;

	qalcCreateContext = NULL;
	qalcMakeContextCurrent = NULL;
	qalcProcessContext = NULL;
	qalcSuspendContext = NULL;
	qalcDestroyContext = NULL;
	qalcGetCurrentContext = NULL;
	qalcGetContextsDevice = NULL;
	qalcOpenDevice = NULL;
	qalcCloseDevice = NULL;
	qalcGetError = NULL;
	qalcIsExtensionPresent = NULL;
	qalcGetProcAddress = NULL;
	qalcGetEnumValue = NULL;
	qalcGetString = NULL;
	qalcGetIntegerv = NULL;
	qalcCaptureOpenDevice = NULL;
	qalcCaptureCloseDevice = NULL;
	qalcCaptureStart = NULL;
	qalcCaptureStop = NULL;
	qalcCaptureSamples = NULL;
}
#else
bool QAL_Init(const char *libname)
{
	return true;
}
void QAL_Shutdown( void )
{
}
#endif
#endif
