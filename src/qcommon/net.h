#ifndef QCOMMON_NET_H
#define QCOMMON_NET_H 1

#include <stdint.h>

/*
==============================================================

NET

==============================================================
*/

#define NET_ENABLEV4 0x01
#define NET_ENABLEV6 0x02
// if this flag is set, always attempt ipv6 connections instead of ipv4 if a v6 address is found.
#define NET_PRIOV6 0x04
// disables ipv6 multicast support if set.
#define NET_DISABLEMCAST 0x08

#define NET_ENABLEALT1PROTO 0x01
#define NET_ENABLEALT2PROTO 0x02
#define NET_DISABLEPRIMPROTO 0x04

#define PACKET_BACKUP 32  // number of old messages that must be kept on client and
                          // server for delta compression and ping estimation
#define PACKET_MASK (PACKET_BACKUP - 1)

#define MAX_PACKET_USERCMDS 32  // max number of usercmd_t in a packet

#define MAX_SNAPSHOT_ENTITIES 256

#define PORT_ANY -1

#define MAX_RELIABLE_COMMANDS 128  // max string commands buffered for retransmit

enum netadrtype_t {
    NA_BAD = 0,  // an address lookup failed
    NA_LOOPBACK,
    NA_BROADCAST,
    NA_IP,
    NA_IP6,
    NA_MULTICAST6,
    NA_UNSPEC
};

typedef enum { NS_CLIENT, NS_SERVER } netsrc_t;

#define NET_ADDRSTRMAXLEN 48  // maximum length of an IPv6 address string including trailing '\0'
struct netadr_t {
    enum netadrtype_t type;

    uint8_t ip[4];
    uint8_t ip6[16];

    unsigned short port;
    unsigned long scope_id;  // Needed for IPv6 link-local addresses

    int alternateProtocol;
};

void NET_Init(void);
void NET_Shutdown(void);
void NET_Restart_f(void);
void NET_Config(bool enableNetworking);
void NET_FlushPacketQueue(void);
void NET_SendPacket(netsrc_t sock, int length, const void *data, struct netadr_t to);
void NET_OutOfBandPrint(netsrc_t net_socket, struct netadr_t adr, const char *format, ...)
    __attribute__((format(printf, 3, 4)));
void NET_OutOfBandData(netsrc_t sock, struct netadr_t adr, uint8_t *format, int len);

bool NET_CompareAdr(struct netadr_t a, struct netadr_t b);
bool NET_CompareBaseAdrMask(struct netadr_t a, struct netadr_t b, int netmask);
bool NET_CompareBaseAdr(struct netadr_t a, struct netadr_t b);
bool NET_IsLocalAddress(struct netadr_t adr);
const char *NET_AdrToString(struct netadr_t a);
const char *NET_AdrToStringwPort(struct netadr_t a);
int NET_StringToAdr(const char *s, struct netadr_t *a, enum netadrtype_t family);
bool NET_GetLoopPacket(netsrc_t sock, struct netadr_t *net_from, struct msg_t *net_message);
void NET_JoinMulticast6(void);
void NET_LeaveMulticast6(void);
void NET_Sleep(int msec);

#define MAX_MSGLEN 16384  // max length of a message, which may be fragmented into multiple packets

#define MAX_DOWNLOAD_WINDOW 48  // ACK window of 48 download chunks. Cannot set this higher, or clients
			 	// will overflow the reliable commands buffer
#define MAX_DOWNLOAD_BLKSIZE 1024  // 896 uint8_t block chunks

#define NETCHAN_GENCHECKSUM(challenge, sequence) ((challenge) ^ ((sequence) * (challenge)))

/*
Netchan handles packet fragmentation and out of order / duplicate suppression
*/

typedef struct {
    netsrc_t sock;

    int dropped;  // between last packet and previous

    int alternateProtocol;
    struct netadr_t remoteAddress;
    int qport;  // qport value to write when transmitting

    // sequencing variables
    int incomingSequence;
    int outgoingSequence;

    // incoming fragment assembly buffer
    int fragmentSequence;
    int fragmentLength;
    uint8_t fragmentBuffer[MAX_MSGLEN];

    // outgoing fragment buffer
    // we need to space out the sending of large fragmented messages
    bool unsentFragments;
    int unsentFragmentStart;
    int unsentLength;
    uint8_t unsentBuffer[MAX_MSGLEN];

    int challenge;
    int lastSentTime;
    int lastSentSize;
} netchan_t;

void Netchan_Init(int qport);
void Netchan_Setup(
    int alternateProtocol, netsrc_t sock, netchan_t *chan, struct netadr_t adr, int qport, int challenge);

void Netchan_Transmit(netchan_t *chan, int length, const uint8_t *data);
void Netchan_TransmitNextFragment(netchan_t *chan);

bool Netchan_Process(netchan_t *chan, struct msg_t *msg);

void Sys_SendPacket(int length, const void *data, struct netadr_t to);
bool Sys_StringToAdr(const char *s, struct netadr_t *a, enum netadrtype_t family); // Does NOT parse port numbers, only base addresses.
bool Sys_IsLANAddress(struct netadr_t adr);
void Sys_ShowIP(void); 

#define SV_ENCODE_START 4
#define SV_DECODE_START 12
#define CL_ENCODE_START 12
#define CL_DECODE_START 4

#endif
