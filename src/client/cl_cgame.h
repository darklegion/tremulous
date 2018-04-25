#ifndef CL_CGAME_H
#define CL_CGAME_H

void CL_InitCGame(void);
void CL_ShutdownCGame(void);
bool CL_GameCommand(void);
void CL_GameConsoleText(void);
void CL_CGameRendering(stereoFrame_t stereo);
void CL_SetCGameTime(void);
void CL_FirstSnapshot(void);
void CL_ShaderStateChanged(void);

struct snapshot_t;
struct gameState_t;
struct glconfig_t;

// retrieve a text command from the server stream.
// the current snapshot will hold the number of the most recent command
// false can be returned if the client system handled the command
// argc() / argv() can be used to examine the parameters of the command
SO_PUBLIC bool CL_GetServerCommand(int serverCommandNumber);

// a snapshot get can fail if the snapshot (or the entties it holds) is so
// old that it has fallen out of the client system queue
SO_PUBLIC bool CL_GetSnapshot(int snapshotNumber, snapshot_t*);
SO_PUBLIC bool CL_GetUserCmd(int cmdNumber, usercmd_t *ucmd);

// cgame will poll each frame to see if a newer snapshot has arrived
// that it is interested in.  The time is returned seperately so that
// snapshot latency can be calculated.
SO_PUBLIC void CL_GetCurrentSnapshotNumber(int *snapshotNumber, int *serverTime);

// the gamestate should be grabbed at startup, and whenever a
// configstring changes
SO_PUBLIC void CL_GetGameState(gameState_t*);

// The glconfig_t will not change during the life of a cgame.
// If it needs to change, the entire cgame will be restarted, because
// all the qhandle_t are then invalid.
SO_PUBLIC void CL_GetGlconfig(glconfig_t*);

SO_PUBLIC void CL_SetUserCmdValue(int userCmdValue, float sensitivityScale);

// returns the most recent command number that can be passed to GetUserCmd
// this will always be at least one higher than the number in the current
// snapshot, and it may be quite a few higher if it is a fast computer on
// a lagged connection
SO_PUBLIC int CL_GetCurrentCmdNumber( void );

#endif
