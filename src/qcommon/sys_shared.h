/*

   This File is part of Tremulous.
   Copyright (C) 2016, wtfbbqhax <victor@badsec.org>.
   Copyright (C) 2016, GrangerHub <grangerhub.com>.

*/

#ifndef SYS_SHARED_H
#define SYS_SHARED_H 1

#include <stdio.h>

#include "qcommon.h"

#ifdef __cplusplus
//extern "C" {
#endif

#define MAX_JOYSTICK_AXIS 16

typedef int cpuFeatures_t;
enum CPU_FEATURES {
  CF_NONE       = 0,
  CF_RDTSC      = 1 << 0,
  CF_MMX        = 1 << 1,
  CF_MMX_EXT    = 1 << 2,
  CF_3DNOW      = 1 << 3,
  CF_3DNOW_EXT  = 1 << 4,
  CF_SSE        = 1 << 5,
  CF_SSE2       = 1 << 6,
  CF_ALTIVEC    = 1 << 7
};

void Sys_Init(void);

// general development dll loading for virtual machine testing
void *QDECL Sys_LoadGameDll(const char *name,
                            intptr_t(QDECL **entryPoint)(int, ...),
                            intptr_t(QDECL *systemcalls)(intptr_t, ...));

void Sys_UnloadDll(void *dllHandle);

void QDECL Sys_Error(const char *error, ...) __attribute__((noreturn, format(printf, 1, 2)));
void Sys_Quit(void) __attribute__((noreturn));

char *Sys_GetClipboardData(void);  // note that this isn't journaled...

void Sys_Print(const char *msg);

// Sys_Milliseconds should only be used for profiling purposes,
// any game related timing information should come from event timestamps
int Sys_Milliseconds(void);

qboolean Sys_RandomBytes(byte *string, int len);
void Sys_CryptoRandomBytes(byte *string, int len);

// the system console is shown when a dedicated server is running
void Sys_DisplaySystemConsole(qboolean show);

cpuFeatures_t Sys_GetProcessorFeatures(void);

void Sys_SetErrorText(const char *text);

void Sys_SendPacket(int length, const void *data, netadr_t to);

qboolean Sys_StringToAdr(const char *s, netadr_t *a, netadrtype_t family);
// Does NOT parse port numbers, only base addresses.

qboolean Sys_IsLANAddress(netadr_t adr);
void Sys_ShowIP(void);

FILE *Sys_FOpen(const char *ospath, const char *mode);
qboolean Sys_Mkdir(const char *path);
FILE *Sys_Mkfifo(const char *ospath);
char *Sys_Cwd(void);
void Sys_SetDefaultInstallPath(const char *path);
char *Sys_DefaultInstallPath(void);

#ifdef __APPLE__
char *Sys_DefaultAppPath(void);
#endif

void Sys_SetDefaultHomePath(const char *path);
char *Sys_DefaultHomePath(void);
const char *Sys_Dirname(char *path);
const char *Sys_Basename(char *path);
char *Sys_ConsoleInput(void);

char **Sys_ListFiles(const char *directory, const char *extension,
                     const char *filter,
                     int *numfiles, qboolean wantsubs);
void Sys_FreeFileList(char **list);
void Sys_Sleep(int msec);

qboolean Sys_LowPhysicalMemory(void);

void Sys_SetEnv(const char *name, const char *value);

qboolean Sys_WritePIDFile(void);

#ifdef __cplusplus
//}
#endif
#endif
