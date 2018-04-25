#ifndef CL_CIN_H
#define CL_CIN_H

#include "qcommon/q_platform.h"

// cinematic states
enum e_status {
	FMV_IDLE,
	FMV_PLAY,		// play
	FMV_EOF,		// all other conditions, i.e. stop/EOF/abort
	FMV_ID_BLT,
	FMV_ID_IDLE,
	FMV_LOOPED,
	FMV_ID_WAIT
};

void CL_PlayCinematic_f(void);
void SCR_DrawCinematic(void);
void SCR_RunCinematic(void);
void SCR_StopCinematic(void);
void CIN_UploadCinematic(int handle);
void CIN_CloseAllVideos(void);

SO_PUBLIC int CIN_PlayCinematic(const char *arg0, int xpos, int ypos, int width, int height, int bits);
SO_PUBLIC e_status CIN_StopCinematic(int handle);
SO_PUBLIC e_status CIN_RunCinematic(int handle);
SO_PUBLIC void CIN_DrawCinematic(int handle);
SO_PUBLIC void CIN_SetExtents(int handle, int x, int y, int w, int h);

#endif
