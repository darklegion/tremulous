/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2013 Darklegion Development
Copyright (C) 2012-2018 ET:Legacy team <mail@etlegacy.com>
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
// server.h

#ifndef SERVER_H
#define SERVER_H 1

#include "game/g_public.h"
#include "qcommon/cmd.h"
#include "qcommon/crypto.h"
#include "qcommon/cvar.h"
#include "qcommon/files.h"
#include "qcommon/huffman.h"
#include "qcommon/msg.h"
#include "qcommon/net.h"
#include "qcommon/q_shared.h"
#include "qcommon/qcommon.h"
#include "qcommon/vm.h"
#include "sys/sys_shared.h"

//=============================================================================

#define PERS_SCORE 0  // !!! MUST NOT CHANGE, SERVER AND GAME BOTH REFERENCE !!!
#define CS_WARMUP 5 // !!! MUST NOT CHANGE, SERVER AND GAME BOTH REFERENCE !!!

// server attack protection
#define SVP_IOQ3        0x0001      ///< 1  - ioQuake3 way
#define SVP_OWOLF       0x0002      ///< 2  - OpenWolf way
#define SVP_CONSOLE     0x0004      ///< 4  - console print

#define MAX_ENT_CLUSTERS 16

#ifdef USE_VOIP
#define VOIP_QUEUE_LENGTH 64
struct voipServerPacket_t {
    int generation;
    int sequence;
    int frames;
    int len;
    int sender;
    int flags;
    byte data[4000];
};
#endif // USE_VOIP

struct svEntity_t {
    struct worldSector_t *worldSector;
    svEntity_t *nextEntityInWorldSector;

    entityState_t baseline;  // for delta compression of initial sighting
    int numClusters;  // if -1, use headnode instead
    int clusternums[MAX_ENT_CLUSTERS];
    int lastCluster;  // if all the clusters don't fit in clusternums
    int areanum, areanum2;
    int snapshotCounter;  // used to prevent double adding from portal views
};

enum serverState_t {
    SS_DEAD,  // no map loaded
    SS_LOADING,  // spawning level entities
    SS_GAME  // actively running
};

struct configString_t {
    char *s;
    bool restricted;  // if true, don't send to clientList
    clientList_t clientList;
};

struct server_t {
    serverState_t state;
    bool restarting;  // if true, send configstring changes during SS_LOADING
    int serverId;  // changes each server start
    int restartedServerId;  // serverId before a map_restart
    int checksumFeed;  // the feed key that we use to compute the pure checksum strings
    // https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=475
    // the serverId associated with the current checksumFeed (always <= serverId)
    int checksumFeedServerId;
    int snapshotCounter;  // incremented for each snapshot built
    int timeResidual;  // <= 1000 / sv_frame->value
    int nextFrameTime;  // when time > nextFrameTime, process world
    configString_t configstrings[MAX_CONFIGSTRINGS];
    svEntity_t svEntities[MAX_GENTITIES];

    char *entityParsePoint;  // used during game VM init

    // the game virtual machine will update these on init and changes
    sharedEntity_t *gentities;
    int gentitySize;
    int num_entities;  // current number, <= MAX_GENTITIES

    playerState_t *gameClients;
    int gameClientSize;  // will be > sizeof(playerState_t) due to game private data

    int restartTime;
    int time;

    vm_t *gvm;  // game virtual machine
};

struct clientSnapshot_t {
    int areabytes;
    byte areabits[MAX_MAP_AREA_BYTES];  // portalarea visibility bits
    playerState_t ps;
    int num_entities;
    int first_entity;  // into the circular sv_packet_entities[]
    // the entities MUST be in increasing state number
    // order, otherwise the delta compression will fail
    int messageSent;  // time the message was transmitted
    int messageAcked;  // time the message was acked
    int messageSize;  // used to rate drop packets
};

enum clientState_t {
    CS_FREE,  // can be reused for a new connection
    CS_ZOMBIE,  // client has been disconnected, but don't reuse
    // connection for a couple seconds
    CS_CONNECTED,  // has been assigned to a client_t, but no gamestate yet
    CS_PRIMED,  // gamestate has been sent, but client hasn't sent a usercmd
    CS_ACTIVE  // client is fully in game
};

struct netchan_buffer_t {
    msg_t msg;
    byte msgBuffer[MAX_MSGLEN];
    netchan_buffer_t *next;
};

struct client_t {
    clientState_t state;
    char userinfo[MAX_INFO_STRING];  // name, etc
    char userinfobuffer[MAX_INFO_STRING];  ///< used for buffering of user info

    char reliableCommands[MAX_RELIABLE_COMMANDS][MAX_STRING_CHARS];
    int reliableSequence;  // last added reliable message, not necesarily sent or acknowledged yet
    int reliableAcknowledge;  // last acknowledged reliable message
    int reliableSent;  // last sent reliable message, not necesarily acknowledged yet
    int messageAcknowledge;

    int gamestateMessageNum;  // netchan->outgoingSequence of gamestate
    int challenge;

    usercmd_t lastUsercmd;
    int lastMessageNum;  // for delta compression
    int lastClientCommand;  // reliable client message sequence
    char lastClientCommandString[MAX_STRING_CHARS];
    sharedEntity_t *gentity;  // SV_GentityNum(clientnum)
    char name[MAX_NAME_LENGTH];  // extracted from userinfo, high bits masked

    // downloading
    char downloadName[MAX_QPATH];  // if not empty string, we are downloading
    fileHandle_t download;  // file being downloaded
    int downloadSize;  // total bytes (can't use EOF because of paks)
    int downloadCount;  // bytes sent
    int downloadClientBlock;  // last block we sent to the client, awaiting ack
    int downloadCurrentBlock;  // current block number
    int downloadXmitBlock;  // last block we xmited
    unsigned char *downloadBlocks[MAX_DOWNLOAD_WINDOW];  // the buffers for the download blocks
    int downloadBlockSize[MAX_DOWNLOAD_WINDOW];
    bool downloadEOF;  // We have sent the EOF block
    int downloadSendTime;  // time we last got an ack from the client

    int deltaMessage;  // frame last client usercmd message
    int nextReliableTime;  // svs.time when another reliable command will be allowed
    int nextReliableUserTime;  // svs.time when another userinfo change will be allowed
    int lastPacketTime;  // svs.time when packet was last received
    int lastConnectTime;  // svs.time when connection started
    int lastSnapshotTime;  // svs.time of last sent snapshot
    bool rateDelayed;  // true if nextSnapshotTime was set based on rate instead of snapshotMsec
    int timeoutCount;  // must timeout a few frames in a row so debugging doesn't break
    clientSnapshot_t frames[PACKET_BACKUP];  // updates can be delta'd from here
    int ping;
    int rate;  // bytes / second
    int snapshotMsec;  // requests a snapshot every snapshotMsec unless rate choked
    int pureAuthentic;
    bool gotCP;  // TTimo - additional flag to distinguish between a bad pure checksum, and no cp command at all
    netchan_t netchan;
    // TTimo
    // queuing outgoing fragmented messages to send them properly, without udp packet bursts
    // in case large fragmented messages are stacking up
    // buffer them into this queue, and hand them out to netchan as needed
    netchan_buffer_t *netchan_start_queue;
    netchan_buffer_t **netchan_end_queue;

    char fingerprint[SHA256_DIGEST_SIZE * 2 + 1];

#ifdef USE_VOIP
    bool hasVoip;
    bool muteAllVoip;
    bool ignoreVoipFromClient[MAX_CLIENTS];
    voipServerPacket_t *voipPacket[VOIP_QUEUE_LENGTH];
    size_t queuedVoipPackets;
    int queuedVoipIndex;
#endif

    int oldServerTime;
    bool csUpdated[MAX_CONFIGSTRINGS];
};

//=============================================================================
#define STATFRAMES 200 ///< 5 seconds - assumed we run 40 fps

/**
 * @struct svstats_t
 * @brief
 */
struct svstats_t {
    double active;
    double idle;
    int count;

    double latched_active;
    double latched_idle;

    float cpu;
    float avg;
};

// MAX_CHALLENGES is made large to prevent a denial
// of service attack that could cycle all of them
// out before legitimate users connected
#define MAX_CHALLENGES 2048
// Allow a certain amount of challenges to have the same IP address
// to make it a bit harder to DOS one single IP address from connecting
// while not allowing a single ip to grab all challenge resources
#define MAX_CHALLENGES_MULTI (MAX_CHALLENGES / 2)

#define AUTHORIZE_TIMEOUT 5000

struct challenge_t {
    netadr_t adr;
    int challenge;
    char challenge2[33];
    int clientChallenge;  // challenge number coming from the client
    int time;  // time the last packet was sent to the autherize server
    int pingTime;  // time the challenge response was sent to client
    int firstTime;  // time the adr was first used, for authorize timeout checks
    bool wasrefused;
    bool connected;
};

/**
 * @struct receipt_t
 * @brief
 */
struct receipt_t {
    netadr_t adr;
    int time;
};

/**
 * @def MAX_INFO_RECEIPTS
 * @brief the maximum number of getstatus+getinfo responses that we send in
 * a two second time period.
 */
#define MAX_INFO_RECEIPTS  48

/**
 * @struct tempBan_s
 * @typedef tempBan_t
 * @brief
 */
struct tempBan_t {
    netadr_t adr;
    int endtime;
};

#define MAX_TEMPBAN_ADDRESSES               MAX_CLIENTS

#define SERVER_PERFORMANCECOUNTER_FRAMES    600
#define SERVER_PERFORMANCECOUNTER_SAMPLES   6

// this structure will be cleared only when the game dll changes
struct serverStatic_t {
    bool initialized;  // sv_init has completed

    int time;  // will be strictly increasing across level changes

    int snapFlagServerBit;  // ^= SNAPFLAG_SERVERCOUNT every SV_SpawnServer()

    client_t *clients;  // [sv_maxclients->integer];
    int numSnapshotEntities;  // sv_maxclients->integer*PACKET_BACKUP*MAX_SNAPSHOT_ENTITIES
    int nextSnapshotEntities;  // next snapshotEntities to use
    entityState_t *snapshotEntities;  // [numSnapshotEntities]
    int nextHeartbeatTime;
    challenge_t challenges[MAX_CHALLENGES];  // to prevent invalid IPs from connecting
    receipt_t   infoReceipts[MAX_INFO_RECEIPTS];
    netadr_t redirectAddress;  // for rcon return messages

    netadr_t authorizeAddress;  // for rcon return messages

    int sampleTimes[SERVER_PERFORMANCECOUNTER_SAMPLES];
    int currentSampleIndex;
    int totalFrameTime;
    int currentFrameIndex;
    int serverLoad;
    svstats_t stats;
};

//=============================================================================

extern serverStatic_t svs;  // persistant server info across maps
extern server_t sv;  // cleared each map

extern cvar_t *sv_fps;
extern cvar_t *sv_timeout;
extern cvar_t *sv_zombietime;
extern cvar_t *sv_rconPassword;
extern cvar_t *sv_privatePassword;
extern cvar_t *sv_allowDownload;
extern cvar_t *sv_maxclients;

extern cvar_t *sv_privateClients;
extern cvar_t *sv_hostname;
extern cvar_t *sv_masters[3][MAX_MASTER_SERVERS];
extern cvar_t *sv_reconnectlimit;
extern cvar_t *sv_showloss;
extern cvar_t *sv_padPackets;
extern cvar_t *sv_killserver;
extern cvar_t *sv_mapname;
extern cvar_t *sv_mapChecksum;
extern cvar_t *sv_serverid;
extern cvar_t *sv_minRate;
extern cvar_t *sv_maxRate;
extern cvar_t *sv_dlRate;
extern cvar_t *sv_minPing;
extern cvar_t *sv_maxPing;
extern cvar_t *sv_pure;
extern cvar_t *sv_lanForceRate;
extern cvar_t *sv_banFile;

extern	cvar_t *sv_protect;
extern	cvar_t *sv_protectLog;

#ifdef USE_VOIP
extern cvar_t *sv_voip;
extern cvar_t *sv_voipProtocol;
#endif

extern cvar_t *sv_rsaAuth;

//===========================================================

//
// sv_main.c
//
struct leakyBucket_t {
    netadrtype_t type;

    union {
        byte _4[4];
        byte _6[16];
    } ipv;

    int lastTime;
    signed char burst;

    long hash;

    leakyBucket_t *prev, *next;
};

extern leakyBucket_t outboundLeakyBucket;

bool SVC_RateLimit(leakyBucket_t *bucket, int burst, int period);
bool SVC_RateLimitAddress(netadr_t from, int burst, int period);

void SV_FinalMessage(const char *message);
void QDECL SV_SendServerCommand(client_t *cl, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

void SV_AddOperatorCommands(void);
void SV_RemoveOperatorCommands(void);

void SV_MasterShutdown(void);
int SV_RateMsec(client_t *client);

//
// sv_init.c
//
void SV_SetConfigstring(int index, const char *val);
void SV_GetConfigstring(int index, char *buffer, int bufferSize);
void SV_SetConfigstringRestrictions(int index, const clientList_t *clientList);
void SV_UpdateConfigstrings(client_t *client);

void SV_SetUserinfo(int index, const char *val);
void SV_GetUserinfo(int index, char *buffer, int bufferSize);

void SV_ChangeMaxClients(void);
void SV_SpawnServer(char *server);
void SV_WriteAttackLog(const char *log);

#ifdef NDEBUG
#define SV_WriteAttackLogD(x)
#else
#define SV_WriteAttackLogD(x) SV_WriteAttackLog(x)
#endif

//
// sv_client.c
//
void SV_GetChallenge(netadr_t from);

void SV_DirectConnect(netadr_t from);

void SV_ExecuteClientMessage(client_t *cl, msg_t *msg);
void SV_UserinfoChanged(client_t *cl);

void SV_ClientEnterWorld(client_t *client, usercmd_t *cmd);
void SV_FreeClient(client_t *client);
void SV_DropClient(client_t *drop, const char *reason);

void SV_ExecuteClientCommand(client_t *cl, const char *s, bool clientOK);
void SV_ClientThink(client_t *cl, usercmd_t *cmd);

int SV_WriteDownloadToClient(client_t *cl, msg_t *msg);
int SV_SendDownloadMessages(void);
int SV_SendQueuedMessages(void);

//
// sv_ccmds.c
//
void SV_Heartbeat_f(void);

//
// sv_snapshot.c
//
void SV_AddServerCommand(client_t *client, const char *cmd);
void SV_UpdateServerCommandsToClient(client_t *client, msg_t *msg);
void SV_WriteFrameToClient(client_t *client, msg_t *msg);
void SV_SendMessageToClient(msg_t *msg, client_t *client);
void SV_SendClientMessages(void);
void SV_SendClientSnapshot(client_t *client);

//
// sv_game.c
//
int SV_NumForGentity(sharedEntity_t *ent);
sharedEntity_t *SV_GentityNum(int num);
playerState_t *SV_GameClientNum(int num);
svEntity_t *SV_SvEntityForGentity(sharedEntity_t *gEnt);
sharedEntity_t *SV_GEntityForSvEntity(svEntity_t *svEnt);
void SV_InitGameProgs(void);
void SV_ShutdownGameProgs(void);
void SV_RestartGameProgs(void);
bool SV_inPVS(const vec3_t p1, const vec3_t p2);

//============================================================
//
// high level object sorting to reduce interaction tests
//

void SV_ClearWorld(void);
// called after the world model has been loaded, before linking any entities

void SV_UnlinkEntity(sharedEntity_t *ent);
// call before removing an entity, and before trying to move one,
// so it doesn't clip against itself

void SV_LinkEntity(sharedEntity_t *ent);
// Needs to be called any time an entity changes origin, mins, maxs,
// or solid.  Automatically unlinks if needed.
// sets ent->r.absmin and ent->r.absmax
// sets ent->leafnums[] for pvs determination even if the entity
// is not solid

clipHandle_t SV_ClipHandleForEntity(const sharedEntity_t *ent);

void SV_SectorList_f(void);

int SV_AreaEntities(const vec3_t mins, const vec3_t maxs, int *entityList, int maxcount);
// fills in a table of entity numbers with entities that have bounding boxes
// that intersect the given area.  It is possible for a non-axial bmodel
// to be returned that doesn't actually intersect the area on an exact
// test.
// returns the number of pointers filled in
// The world entity is never returned in this list.

int SV_PointContents(const vec3_t p, int passEntityNum);
// returns the CONTENTS_* value from the world and all entities at the given point.

void SV_Trace(trace_t *results, const vec3_t start, vec3_t mins, vec3_t maxs, const vec3_t end, int passEntityNum,
    int contentmask, traceType_t type);
// mins and maxs are relative

// if the entire move stays in a solid volume, trace.allsolid will be set,
// trace.startsolid will be set, and trace.fraction will be 0

// if the starting point is in a solid, it will be allowed to move out
// to an open area

// passEntityNum is explicitly excluded from clipping checks (normally ENTITYNUM_NONE)

void SV_ClipToEntity(trace_t *trace, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
    int entityNum, int contentmask, traceType_t type);
// clip to a specific entity

//
// sv_net_chan.c
//
void SV_Netchan_Transmit(client_t *client, msg_t *msg);
int SV_Netchan_TransmitNextFragment(client_t *client);
bool SV_Netchan_Process(client_t *client, msg_t *msg);
void SV_Netchan_FreeQueue(client_t *client);

#endif
