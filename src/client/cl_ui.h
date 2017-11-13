#ifndef CL_UI_H
#define CL_UI_H

#include "qcommon/q_platform.h"

//
// cl_ui.c
//
void CL_InitUI(void);
void CL_ShutdownUI(void);

SO_PUBLIC bool CL_GetNews(bool begin);
SO_PUBLIC bool CL_GetConfigString(int i, char *buf, int size);
SO_PUBLIC bool LAN_ServerIsVisible(int source, int n);
SO_PUBLIC int LAN_AddServer(int source, const char *name, const char *address);
SO_PUBLIC int LAN_CompareServers(int source, int sortKey, int sortDir, int s1, int s2);
SO_PUBLIC int LAN_GetServerCount(int source);
SO_PUBLIC int LAN_GetServerPing(int source, int n);
SO_PUBLIC void CL_GetClientState(uiClientState_t *state);
SO_PUBLIC void LAN_GetServerAddressString(int source, int n, char *buf, int buflen);
SO_PUBLIC void LAN_GetServerInfo(int source, int n, char *buf, int buflen);
SO_PUBLIC void LAN_LoadCachedServers(void);
SO_PUBLIC void LAN_MarkServerVisible(int source, int n, bool visible);
SO_PUBLIC void LAN_RemoveServer(int source, const char *addr);
SO_PUBLIC void LAN_ResetPings(int source);
SO_PUBLIC void LAN_SaveServersToCache(void);

#endif
