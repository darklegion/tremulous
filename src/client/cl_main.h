#ifndef CL_MAIN_H
#define CL_MAIN_H

void CL_Init(void);
bool CL_CheckPaused(void);
void CL_Disconnect_f(void);
void CL_InitDownloads(void);
void CL_NextDemo(void);
void CL_NextDownload(void);
void CL_ReadDemoMessage(void);
void CL_StartHunkUsers(bool rendererOnly);
void CL_StopRecord_f(void);

enum demoState_t;
struct refexport_t;

SO_PUBLIC bool CL_GetNews(bool begin);
SO_PUBLIC bool CL_ServerStatus(const char *serverAddress, char *serverStatusString, int maxLen);
SO_PUBLIC bool CL_UpdateVisiblePings_f(int source);
SO_PUBLIC demoState_t CL_DemoState(void);
SO_PUBLIC int CL_DemoPos(void);
/*SO_PUBLIC*/ int CL_GetPingQueueCount(void); // rename LAN_
SO_PUBLIC refexport_t *CL_GetRenderer(void);
SO_PUBLIC void CL_AddReliableCommand(const char *cmd, bool isDisconnectCmd);
SO_PUBLIC void CL_ClearPing(int n); // rename LAN_
SO_PUBLIC void CL_DemoName(char *buffer, int size);
/*SO_PUBLIC*/ void CL_GetPing(int n, char *buf, int buflen, int *pingtime);
/*SO_PUBLIC*/ void CL_GetPingInfo(int n, char *buf, int buflen);

#endif
