/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2013 Darklegion Development

This file is part of Tremulous.

Tremulous is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Tremulous is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tremulous; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "msg.h"
#include "net.h"
#include "cmd.h"
#include "cvar.h"
#include "q_shared.h"
#include "qcommon.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#if WINVER < 0x501
#ifdef __MINGW32__
// wspiapi.h isn't available on MinGW, so if it's
// present it's because the end user has added it
// and we should look for it in our tree
#include "wspiapi.h"
#else
#include <wspiapi.h>
#endif
#else
#include <ws2spi.h>
#endif

typedef int socklen_t;
#ifdef ADDRESS_FAMILY
#define sa_family_t ADDRESS_FAMILY
#else
typedef unsigned short sa_family_t;
#endif

#define EAGAIN WSAEWOULDBLOCK
#define EADDRNOTAVAIL WSAEADDRNOTAVAIL
#define EAFNOSUPPORT WSAEAFNOSUPPORT
#define ECONNRESET WSAECONNRESET
typedef u_long ioctlarg_t;
#define socketError WSAGetLastError()

static WSADATA winsockdata;
static bool winsockInitialized = false;

#else

#if MAC_OS_X_VERSION_MIN_REQUIRED == 1020
// needed for socklen_t on OSX 10.2
#define _BSD_SOCKLEN_T_
#endif

#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#if !defined(__sun) && !defined(__sgi)
#include <ifaddrs.h>
#endif

#ifdef __sun
#include <sys/filio.h>
#endif

typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#define ioctlsocket ioctl
typedef int ioctlarg_t;
#define socketError errno

#endif

static bool usingSocks = false;
static int networkingEnabled = 0;

static cvar_t *net_enabled;
static cvar_t *net_alternateProtocols;

static cvar_t *net_socksEnabled;
static cvar_t *net_socksServer;
static cvar_t *net_socksPort;
static cvar_t *net_socksUsername;
static cvar_t *net_socksPassword;

static cvar_t *net_ip;
static cvar_t *net_ip6;
static cvar_t *net_ports[3];
static cvar_t *net_port6s[3];
static cvar_t *net_mcast6addr;
static cvar_t *net_mcast6iface;

static cvar_t *net_dropsim;

static struct sockaddr socksRelayAddr;

static SOCKET ip_sockets[3] = {INVALID_SOCKET, INVALID_SOCKET, INVALID_SOCKET};
static SOCKET ip6_sockets[3] = {INVALID_SOCKET, INVALID_SOCKET, INVALID_SOCKET};
/*
TODO: accommodate
static SOCKET	socks_socket = INVALID_SOCKET;
static SOCKET	multicast6_socket = INVALID_SOCKET;
*/

// Keep track of currently joined multicast group.
static struct ipv6_mreq curgroup;
// And the currently bound address.
static struct sockaddr_in6 boundto;

#ifndef IF_NAMESIZE
#define IF_NAMESIZE 16
#endif

// use an admin local address per default so that network admins can decide on how to handle quake3 traffic.
#define NET_MULTICAST_IP6 "ff04::696f:7175:616b:6533"

#define MAX_IPS 32

typedef struct {
    char ifname[IF_NAMESIZE];

    netadrtype_t type;
    sa_family_t family;
    struct sockaddr_storage addr;
    struct sockaddr_storage netmask;
} nip_localaddr_t;

static nip_localaddr_t localIP[MAX_IPS];
static int numIP;

//=============================================================================

/*
====================
NET_ErrorString
====================
*/
const char *NET_ErrorString(void)
{
#ifdef _WIN32
    // FIXME: replace with FormatMessage?
    switch (socketError)
    {
        case WSAEINTR:
            return "WSAEINTR";
        case WSAEBADF:
            return "WSAEBADF";
        case WSAEACCES:
            return "WSAEACCES";
        case WSAEDISCON:
            return "WSAEDISCON";
        case WSAEFAULT:
            return "WSAEFAULT";
        case WSAEINVAL:
            return "WSAEINVAL";
        case WSAEMFILE:
            return "WSAEMFILE";
        case WSAEWOULDBLOCK:
            return "WSAEWOULDBLOCK";
        case WSAEINPROGRESS:
            return "WSAEINPROGRESS";
        case WSAEALREADY:
            return "WSAEALREADY";
        case WSAENOTSOCK:
            return "WSAENOTSOCK";
        case WSAEDESTADDRREQ:
            return "WSAEDESTADDRREQ";
        case WSAEMSGSIZE:
            return "WSAEMSGSIZE";
        case WSAEPROTOTYPE:
            return "WSAEPROTOTYPE";
        case WSAENOPROTOOPT:
            return "WSAENOPROTOOPT";
        case WSAEPROTONOSUPPORT:
            return "WSAEPROTONOSUPPORT";
        case WSAESOCKTNOSUPPORT:
            return "WSAESOCKTNOSUPPORT";
        case WSAEOPNOTSUPP:
            return "WSAEOPNOTSUPP";
        case WSAEPFNOSUPPORT:
            return "WSAEPFNOSUPPORT";
        case WSAEAFNOSUPPORT:
            return "WSAEAFNOSUPPORT";
        case WSAEADDRINUSE:
            return "WSAEADDRINUSE";
        case WSAEADDRNOTAVAIL:
            return "WSAEADDRNOTAVAIL";
        case WSAENETDOWN:
            return "WSAENETDOWN";
        case WSAENETUNREACH:
            return "WSAENETUNREACH";
        case WSAENETRESET:
            return "WSAENETRESET";
        case WSAECONNABORTED:
            return "WSWSAECONNABORTEDAEINTR";
        case WSAECONNRESET:
            return "WSAECONNRESET";
        case WSAENOBUFS:
            return "WSAENOBUFS";
        case WSAEISCONN:
            return "WSAEISCONN";
        case WSAENOTCONN:
            return "WSAENOTCONN";
        case WSAESHUTDOWN:
            return "WSAESHUTDOWN";
        case WSAETOOMANYREFS:
            return "WSAETOOMANYREFS";
        case WSAETIMEDOUT:
            return "WSAETIMEDOUT";
        case WSAECONNREFUSED:
            return "WSAECONNREFUSED";
        case WSAELOOP:
            return "WSAELOOP";
        case WSAENAMETOOLONG:
            return "WSAENAMETOOLONG";
        case WSAEHOSTDOWN:
            return "WSAEHOSTDOWN";
        case WSASYSNOTREADY:
            return "WSASYSNOTREADY";
        case WSAVERNOTSUPPORTED:
            return "WSAVERNOTSUPPORTED";
        case WSANOTINITIALISED:
            return "WSANOTINITIALISED";
        case WSAHOST_NOT_FOUND:
            return "WSAHOST_NOT_FOUND";
        case WSATRY_AGAIN:
            return "WSATRY_AGAIN";
        case WSANO_RECOVERY:
            return "WSANO_RECOVERY";
        case WSANO_DATA:
            return "WSANO_DATA";
        default:
            return "NO ERROR";
    }
#else
    return strerror(socketError);
#endif
}

static void NetadrToSockadr(netadr_t *a, struct sockaddr *s)
{
    if (a->type == NA_BROADCAST)
    {
        ((struct sockaddr_in *)s)->sin_family = AF_INET;
        ((struct sockaddr_in *)s)->sin_port = a->port;
#ifdef __FreeBSD__
        ((struct sockaddr_in *)s)->sin_addr.s_addr = INADDR_ANY;
#else
        ((struct sockaddr_in *)s)->sin_addr.s_addr = INADDR_BROADCAST;
#endif
    }
    else if (a->type == NA_IP)
    {
        ((struct sockaddr_in *)s)->sin_family = AF_INET;
        ((struct sockaddr_in *)s)->sin_addr.s_addr = *(int *)&a->ip;
        ((struct sockaddr_in *)s)->sin_port = a->port;
    }
    else if (a->type == NA_IP6)
    {
        ((struct sockaddr_in6 *)s)->sin6_family = AF_INET6;
        ((struct sockaddr_in6 *)s)->sin6_addr = *((struct in6_addr *)&a->ip6);
        ((struct sockaddr_in6 *)s)->sin6_port = a->port;
        ((struct sockaddr_in6 *)s)->sin6_scope_id = a->scope_id;
    }
    else if (a->type == NA_MULTICAST6)
    {
        ((struct sockaddr_in6 *)s)->sin6_family = AF_INET6;
        ((struct sockaddr_in6 *)s)->sin6_addr = curgroup.ipv6mr_multiaddr;
        ((struct sockaddr_in6 *)s)->sin6_port = a->port;
    }
}

static void SockadrToNetadr(struct sockaddr *s, netadr_t *a)
{
    if (s->sa_family == AF_INET)
    {
        a->type = NA_IP;
        *(int *)&a->ip = ((struct sockaddr_in *)s)->sin_addr.s_addr;
        a->port = ((struct sockaddr_in *)s)->sin_port;
    }
    else if (s->sa_family == AF_INET6)
    {
        a->type = NA_IP6;
        memcpy(a->ip6, &((struct sockaddr_in6 *)s)->sin6_addr, sizeof(a->ip6));
        a->port = ((struct sockaddr_in6 *)s)->sin6_port;
        a->scope_id = ((struct sockaddr_in6 *)s)->sin6_scope_id;
    }
    a->alternateProtocol = 0;
}

static struct addrinfo *SearchAddrInfo(struct addrinfo *hints, sa_family_t family)
{
    while (hints)
    {
        if (hints->ai_family == family) return hints;

        hints = hints->ai_next;
    }

    return NULL;
}

/*
=============
Sys_StringToSockaddr
=============
*/
static bool Sys_StringToSockaddr(const char *s, struct sockaddr *sadr, int sadr_len, sa_family_t family)
{
    struct addrinfo hints;
    struct addrinfo *res = NULL;
    struct addrinfo *search = NULL;
    struct addrinfo *hintsp;
    int retval;

    memset(sadr, '\0', sizeof(*sadr));
    memset(&hints, '\0', sizeof(hints));

    hintsp = &hints;
    hintsp->ai_family = family;
    hintsp->ai_socktype = SOCK_DGRAM;

    retval = getaddrinfo(s, NULL, hintsp, &res);

    if (!retval)
    {
        if (family == AF_UNSPEC)
        {
            // Decide here and now which protocol family to use
            if (net_enabled->integer & NET_PRIOV6)
            {
                if (net_enabled->integer & NET_ENABLEV6) search = SearchAddrInfo(res, AF_INET6);

                if (!search && (net_enabled->integer & NET_ENABLEV4)) search = SearchAddrInfo(res, AF_INET);
            }
            else
            {
                if (net_enabled->integer & NET_ENABLEV4) search = SearchAddrInfo(res, AF_INET);

                if (!search && (net_enabled->integer & NET_ENABLEV6)) search = SearchAddrInfo(res, AF_INET6);
            }
        }
        else
            search = SearchAddrInfo(res, family);

        if (search)
        {
            if (search->ai_addrlen > sadr_len) search->ai_addrlen = sadr_len;

            memcpy(sadr, search->ai_addr, search->ai_addrlen);
            freeaddrinfo(res);

            return true;
        }
        else
            Com_Printf("Sys_StringToSockaddr: Error resolving %s: No address of required type found.\n", s);
    }
    else
        Com_Printf("Sys_StringToSockaddr: Error resolving %s: %s\n", s, gai_strerror(retval));

    if (res) freeaddrinfo(res);

    return false;
}

/*
=============
Sys_SockaddrToString
=============
*/
static void Sys_SockaddrToString(char *dest, int destlen, struct sockaddr *input)
{
    socklen_t inputlen;

    if (input->sa_family == AF_INET6)
        inputlen = sizeof(struct sockaddr_in6);
    else
        inputlen = sizeof(struct sockaddr_in);

    if (getnameinfo(input, inputlen, dest, destlen, NULL, 0, NI_NUMERICHOST) && destlen > 0) *dest = '\0';
}

/*
=============
Sys_StringToAdr
=============
*/
bool Sys_StringToAdr(const char *s, netadr_t *a, netadrtype_t family)
{
    struct sockaddr_storage sadr;
    sa_family_t fam;

    switch (family)
    {
        case NA_IP:
            fam = AF_INET;
            break;
        case NA_IP6:
            fam = AF_INET6;
            break;
        default:
            fam = AF_UNSPEC;
            break;
    }
    if (!Sys_StringToSockaddr(s, (struct sockaddr *)&sadr, sizeof(sadr), fam))
    {
        return false;
    }

    SockadrToNetadr((struct sockaddr *)&sadr, a);
    return true;
}

/*
===================
NET_CompareBaseAdrMask

Compare without port, and up to the bit number given in netmask.
===================
*/
bool NET_CompareBaseAdrMask(netadr_t a, netadr_t b, int netmask)
{
    uint8_t cmpmask, *addra, *addrb;
    int curbyte;

    if (a.alternateProtocol != b.alternateProtocol) return false;

    if (a.type != b.type) return false;

    if (a.type == NA_LOOPBACK) return true;

    if (a.type == NA_IP)
    {
        addra = (uint8_t *)&a.ip;
        addrb = (uint8_t *)&b.ip;

        if (netmask < 0 || netmask > 32) netmask = 32;
    }
    else if (a.type == NA_IP6)
    {
        addra = (uint8_t *)&a.ip6;
        addrb = (uint8_t *)&b.ip6;

        if (netmask < 0 || netmask > 128) netmask = 128;
    }
    else
    {
        Com_Printf("NET_CompareBaseAdr: bad address type\n");
        return false;
    }

    curbyte = netmask >> 3;

    if (curbyte && memcmp(addra, addrb, curbyte)) return false;

    netmask &= 0x07;
    if (netmask)
    {
        cmpmask = (1 << netmask) - 1;
        cmpmask <<= 8 - netmask;

        if ((addra[curbyte] & cmpmask) == (addrb[curbyte] & cmpmask)) return true;
    }
    else
        return true;

    return false;
}

/*
===================
NET_CompareBaseAdr

Compares without the port
===================
*/
bool NET_CompareBaseAdr(netadr_t a, netadr_t b) { return NET_CompareBaseAdrMask(a, b, -1); }
const char *NET_AdrToString(netadr_t a)
{
    static char s[NET_ADDRSTRMAXLEN];

    if (a.type == NA_LOOPBACK)
        Com_sprintf(s, sizeof(s), "loopback");
    else if (a.type == NA_IP || a.type == NA_IP6)
    {
        struct sockaddr_storage sadr;

        memset(&sadr, 0, sizeof(sadr));
        NetadrToSockadr(&a, (struct sockaddr *)&sadr);
        Sys_SockaddrToString(s, sizeof(s), (struct sockaddr *)&sadr);
    }

    return s;
}

const char *NET_AdrToStringwPort(netadr_t a)
{
    static char s[NET_ADDRSTRMAXLEN];

    if (a.type == NA_LOOPBACK)
        Com_sprintf(s, sizeof(s), "loopback");
    else if (a.type == NA_IP)
        Com_sprintf(s, sizeof(s), "%s:%hu", NET_AdrToString(a), ntohs(a.port));
    else if (a.type == NA_IP6)
        Com_sprintf(s, sizeof(s), "[%s]:%hu", NET_AdrToString(a), ntohs(a.port));

    return s;
}

bool NET_CompareAdr(netadr_t a, netadr_t b)
{
    if (!NET_CompareBaseAdr(a, b)) return false;

    if (a.type == NA_IP || a.type == NA_IP6)
    {
        if (a.port == b.port) return true;
    }
    else
        return true;

    return false;
}

bool NET_IsLocalAddress(netadr_t adr) { return (bool)(adr.type == NA_LOOPBACK); }
//=============================================================================

/*
==================
NET_GetPacket

Receive one packet
==================
*/
bool NET_GetPacket(netadr_t *net_from, msg_t *net_message, fd_set *fdr)
{
    int a;
    int ret;
    struct sockaddr_storage from;
    socklen_t fromlen;
    int err;

    for (a = 0; a < 3; ++a)
    {
        // indent
        if (ip_sockets[a] != INVALID_SOCKET && FD_ISSET(ip_sockets[a], fdr))
        {
            fromlen = sizeof(from);
            ret = recvfrom(
                ip_sockets[a], (char *)net_message->data, net_message->maxsize, 0, (struct sockaddr *)&from, &fromlen);

            if (ret == SOCKET_ERROR)
            {
                err = socketError;

                if (err != EAGAIN && err != ECONNRESET) Com_Printf("NET_GetPacket: %s\n", NET_ErrorString());
            }
            else
            {
                memset(((struct sockaddr_in *)&from)->sin_zero, 0, 8);

                if (usingSocks && memcmp(&from, &socksRelayAddr, fromlen) == 0)
                {
                    if (ret < 10 || net_message->data[0] != 0 || net_message->data[1] != 0 ||
                        net_message->data[2] != 0 || net_message->data[3] != 1)
                    {
                        return false;
                    }
                    net_from->type = NA_IP;
                    net_from->ip[0] = net_message->data[4];
                    net_from->ip[1] = net_message->data[5];
                    net_from->ip[2] = net_message->data[6];
                    net_from->ip[3] = net_message->data[7];
                    net_from->port = *(short *)&net_message->data[8];
                    net_message->readcount = 10;
                }
                else
                {
                    SockadrToNetadr((struct sockaddr *)&from, net_from);
                    net_message->readcount = 0;
                }

                net_from->alternateProtocol = a;

                if (ret >= net_message->maxsize)
                {
                    Com_Printf("Oversize packet from %s\n", NET_AdrToString(*net_from));
                    return false;
                }

                net_message->cursize = ret;
                return true;
            }
        }

        if (ip6_sockets[a] != INVALID_SOCKET && FD_ISSET(ip6_sockets[a], fdr))
        {
            fromlen = sizeof(from);
            ret = recvfrom(
                ip6_sockets[a], (char *)net_message->data, net_message->maxsize, 0, (struct sockaddr *)&from, &fromlen);

            if (ret == SOCKET_ERROR)
            {
                err = socketError;

                if (err != EAGAIN && err != ECONNRESET) Com_Printf("NET_GetPacket: %s\n", NET_ErrorString());
            }
            else
            {
                SockadrToNetadr((struct sockaddr *)&from, net_from);
                net_message->readcount = 0;

                net_from->alternateProtocol = a;

                if (ret >= net_message->maxsize)
                {
                    Com_Printf("Oversize packet from %s\n", NET_AdrToString(*net_from));
                    return false;
                }

                net_message->cursize = ret;
                return true;
            }
        }

        /*
        TODO: accommodate
        if(multicast6_socket != INVALID_SOCKET && multicast6_socket != ip6_socket && FD_ISSET(multicast6_socket, fdr))
        {
                fromlen = sizeof(from);
                ret = recvfrom(multicast6_socket, (char*)net_message->data, net_message->maxsize, 0, (struct sockaddr *)
        &from, &fromlen);

                if (ret == SOCKET_ERROR)
                {
                        err = socketError;

                        if( err != EAGAIN && err != ECONNRESET )
                                Com_Printf( "NET_GetPacket: %s\n", NET_ErrorString() );
                }
                else
                {
                        SockadrToNetadr((struct sockaddr *) &from, net_from);
                        net_message->readcount = 0;

                        if(ret >= net_message->maxsize)
                        {
                                Com_Printf( "Oversize packet from %s\n", NET_AdrToString (*net_from) );
                                return false;
                        }

                        net_message->cursize = ret;
                        return true;
                }
        }
        */
        // outdent
    }

    return false;
}

//=============================================================================

static char socksBuf[4096];

/*
==================
Sys_SendPacket
==================
*/
void Sys_SendPacket(int length, const void *data, netadr_t to)
{
    int ret = SOCKET_ERROR;
    struct sockaddr_storage addr;

    if (to.type != NA_BROADCAST && to.type != NA_IP && to.type != NA_IP6 && to.type != NA_MULTICAST6)
    {
        Com_Error(ERR_FATAL, "Sys_SendPacket: bad address type");
        return;
    }

    if ((ip_sockets[to.alternateProtocol] == INVALID_SOCKET && to.type == NA_IP) ||
        (ip_sockets[to.alternateProtocol] == INVALID_SOCKET && to.type == NA_BROADCAST) ||
        (ip6_sockets[to.alternateProtocol] == INVALID_SOCKET && to.type == NA_IP6) ||
        (/* TODO: accommodate ip6_socket == INVALID_SOCKET && */ to.type == NA_MULTICAST6))
        return;

    if (to.type == NA_MULTICAST6 && (net_enabled->integer & NET_DISABLEMCAST)) return;

    memset(&addr, 0, sizeof(addr));
    NetadrToSockadr(&to, (struct sockaddr *)&addr);

    if (usingSocks && to.type == NA_IP)
    {
        socksBuf[0] = 0;  // reserved
        socksBuf[1] = 0;
        socksBuf[2] = 0;  // fragment (not fragmented)
        socksBuf[3] = 1;  // address type: IPV4
        *(int *)&socksBuf[4] = ((struct sockaddr_in *)&addr)->sin_addr.s_addr;
        *(short *)&socksBuf[8] = ((struct sockaddr_in *)&addr)->sin_port;
        memcpy(&socksBuf[10], data, length);
        ret = sendto(ip_sockets[to.alternateProtocol], (const char *)socksBuf, length + 10, 0, &socksRelayAddr,
            sizeof(socksRelayAddr));
    }
    else
    {
        if (addr.ss_family == AF_INET)
            ret = sendto(ip_sockets[to.alternateProtocol], (const char *)data, length, 0, (struct sockaddr *)&addr,
                sizeof(struct sockaddr_in));
        else if (addr.ss_family == AF_INET6)
            ret = sendto(ip6_sockets[to.alternateProtocol], (const char *)data, length, 0, (struct sockaddr *)&addr,
                sizeof(struct sockaddr_in6));
    }
    if (ret == SOCKET_ERROR)
    {
        int err = socketError;

        // wouldblock is silent
        if (err == EAGAIN)
        {
            return;
        }

        // some PPP links do not allow broadcasts and return an error
        if ((err == EADDRNOTAVAIL) && ((to.type == NA_BROADCAST)))
        {
            return;
        }

        Com_Printf("Sys_SendPacket: %s\n", NET_ErrorString());
    }
}

//=============================================================================

/*
==================
Sys_IsLANAddress

LAN clients will have their rate var ignored
==================
*/
bool Sys_IsLANAddress(netadr_t adr)
{
    int index, run, addrsize;
    bool differed;
    uint8_t *compareadr, *comparemask, *compareip;

    if (adr.type == NA_LOOPBACK)
    {
        return true;
    }

    if (adr.type == NA_IP)
    {
        // RFC1918:
        // 10.0.0.0        -   10.255.255.255  (10/8 prefix)
        // 172.16.0.0      -   172.31.255.255  (172.16/12 prefix)
        // 192.168.0.0     -   192.168.255.255 (192.168/16 prefix)
        if (adr.ip[0] == 10) return true;
        if (adr.ip[0] == 172 && (adr.ip[1] & 0xf0) == 16) return true;
        if (adr.ip[0] == 192 && adr.ip[1] == 168) return true;

        if (adr.ip[0] == 127) return true;
    }
    else if (adr.type == NA_IP6)
    {
        if (adr.ip6[0] == 0xfe && (adr.ip6[1] & 0xc0) == 0x80) return true;
        if ((adr.ip6[0] & 0xfe) == 0xfc) return true;
    }

    // Now compare against the networks this computer is member of.
    for (index = 0; index < numIP; index++)
    {
        if (localIP[index].type == adr.type)
        {
            if (adr.type == NA_IP)
            {
                compareip = (uint8_t *)&((struct sockaddr_in *)&localIP[index].addr)->sin_addr.s_addr;
                comparemask = (uint8_t *)&((struct sockaddr_in *)&localIP[index].netmask)->sin_addr.s_addr;
                compareadr = adr.ip;

                addrsize = sizeof(adr.ip);
            }
            else
            {
                // TODO? should we check the scope_id here?

                compareip = (uint8_t *)&((struct sockaddr_in6 *)&localIP[index].addr)->sin6_addr;
                comparemask = (uint8_t *)&((struct sockaddr_in6 *)&localIP[index].netmask)->sin6_addr;
                compareadr = adr.ip6;

                addrsize = sizeof(adr.ip6);
            }

            differed = false;
            for (run = 0; run < addrsize; run++)
            {
                if ((compareip[run] & comparemask[run]) != (compareadr[run] & comparemask[run]))
                {
                    differed = true;
                    break;
                }
            }

            if (!differed) return true;
        }
    }

    return false;
}

/*
==================
Sys_ShowIP
==================
*/
void Sys_ShowIP(void)
{
    int i;
    char addrbuf[NET_ADDRSTRMAXLEN];

    for (i = 0; i < numIP; i++)
    {
        Sys_SockaddrToString(addrbuf, sizeof(addrbuf), (struct sockaddr *)&localIP[i].addr);

        if (localIP[i].type == NA_IP)
            Com_Printf("IP: %s\n", addrbuf);
        else if (localIP[i].type == NA_IP6)
            Com_Printf("IP6: %s\n", addrbuf);
    }
}

//=============================================================================

/*
====================
NET_IPSocket
====================
*/
SOCKET NET_IPSocket(int alternateProtocol, char *net_interface, int port, int *err)
{
    SOCKET newsocket;
    struct sockaddr_in address;
    ioctlarg_t _true = 1;
    int i = 1;

    *err = 0;

    if (net_interface)
    {
        Com_Printf("Opening%s IP socket: %s:%i\n",
            (alternateProtocol == 2 ? " alternate-2" : alternateProtocol == 1 ? " alternate-1" : ""), net_interface,
            port);
    }
    else
    {
        Com_Printf("Opening%s IP socket: 0.0.0.0:%i\n",
            (alternateProtocol == 2 ? " alternate-2" : alternateProtocol == 1 ? " alternate-1" : ""), port);
    }

    if ((newsocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
    {
        *err = socketError;
        Com_Printf("WARNING: NET_IPSocket: socket: %s\n", NET_ErrorString());
        return newsocket;
    }
    // make it non-blocking
    if (ioctlsocket(newsocket, FIONBIO, &_true) == SOCKET_ERROR)
    {
        Com_Printf("WARNING: NET_IPSocket: ioctl FIONBIO: %s\n", NET_ErrorString());
        *err = socketError;
        closesocket(newsocket);
        return INVALID_SOCKET;
    }

    // make it broadcast capable
    if (setsockopt(newsocket, SOL_SOCKET, SO_BROADCAST, (char *)&i, sizeof(i)) == SOCKET_ERROR)
    {
        Com_Printf("WARNING: NET_IPSocket: setsockopt SO_BROADCAST: %s\n", NET_ErrorString());
    }

    if (!net_interface || !net_interface[0])
    {
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        if (!Sys_StringToSockaddr(net_interface, (struct sockaddr *)&address, sizeof(address), AF_INET))
        {
            closesocket(newsocket);
            return INVALID_SOCKET;
        }
    }

    if (port == PORT_ANY)
    {
        address.sin_port = 0;
    }
    else
    {
        address.sin_port = htons((short)port);
    }

    if (bind(newsocket, (const sockaddr *)&address, sizeof(address)) == SOCKET_ERROR)
    {
        Com_Printf("WARNING: NET_IPSocket: bind: %s\n", NET_ErrorString());
        *err = socketError;
        closesocket(newsocket);
        return INVALID_SOCKET;
    }

    return newsocket;
}

/*
====================
NET_IP6Socket
====================
*/
SOCKET NET_IP6Socket(int alternateProtocol, char *net_interface, int port, struct sockaddr_in6 *bindto, int *err)
{
    SOCKET newsocket;
    struct sockaddr_in6 address;
    ioctlarg_t _true = 1;

    *err = 0;

    if (net_interface)
    {
        // Print the name in brackets if there is a colon:
        if (Q_CountChar(net_interface, ':'))
            Com_Printf("Opening%s IP6 socket: [%s]:%i\n",
                (alternateProtocol == 2 ? " alternate-2" : alternateProtocol == 1 ? " alternate-1" : ""), net_interface,
                port);
        else
            Com_Printf("Opening%s IP6 socket: %s:%i\n",
                (alternateProtocol == 2 ? " alternate-2" : alternateProtocol == 1 ? " alternate-1" : ""), net_interface,
                port);
    }
    else
        Com_Printf("Opening%s IP6 socket: [::]:%i\n",
            (alternateProtocol == 2 ? " alternate-2" : alternateProtocol == 1 ? " alternate-1" : ""), port);

    if ((newsocket = socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
    {
        *err = socketError;
        Com_Printf("WARNING: NET_IP6Socket: socket: %s\n", NET_ErrorString());
        return newsocket;
    }

    // make it non-blocking
    if (ioctlsocket(newsocket, FIONBIO, &_true) == SOCKET_ERROR)
    {
        Com_Printf("WARNING: NET_IP6Socket: ioctl FIONBIO: %s\n", NET_ErrorString());
        *err = socketError;
        closesocket(newsocket);
        return INVALID_SOCKET;
    }

#ifdef IPV6_V6ONLY
    {
        int i = 1;

        // ipv4 addresses should not be allowed to connect via this socket.
        if (setsockopt(newsocket, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&i, sizeof(i)) == SOCKET_ERROR)
        {
            // win32 systems don't seem to support this anyways.
            Com_DPrintf("WARNING: NET_IP6Socket: setsockopt IPV6_V6ONLY: %s\n", NET_ErrorString());
        }
    }
#endif

    if (!net_interface || !net_interface[0])
    {
        address.sin6_family = AF_INET6;
        address.sin6_addr = in6addr_any;
    }
    else
    {
        if (!Sys_StringToSockaddr(net_interface, (struct sockaddr *)&address, sizeof(address), AF_INET6))
        {
            closesocket(newsocket);
            return INVALID_SOCKET;
        }
    }

    if (port == PORT_ANY)
    {
        address.sin6_port = 0;
    }
    else
    {
        address.sin6_port = htons((short)port);
    }

    if (bind(newsocket, (const sockaddr *)&address, sizeof(address)) == SOCKET_ERROR)
    {
        Com_Printf("WARNING: NET_IP6Socket: bind: %s\n", NET_ErrorString());
        *err = socketError;
        closesocket(newsocket);
        return INVALID_SOCKET;
    }

    if (bindto) *bindto = address;

    return newsocket;
}

/*
====================
NET_SetMulticast
Set the current multicast group
====================
*/
void NET_SetMulticast6(void)
{
    struct sockaddr_in6 addr;

    if (!*net_mcast6addr->string ||
        !Sys_StringToSockaddr(net_mcast6addr->string, (struct sockaddr *)&addr, sizeof(addr), AF_INET6))
    {
        Com_Printf(
            "WARNING: NET_JoinMulticast6: Incorrect multicast address given, "
            "please set cvar %s to a sane value.\n",
            net_mcast6addr->name);

        Cvar_SetValue(net_enabled->name, net_enabled->integer | NET_DISABLEMCAST);

        return;
    }

    memcpy(&curgroup.ipv6mr_multiaddr, &addr.sin6_addr, sizeof(curgroup.ipv6mr_multiaddr));

    if (*net_mcast6iface->string)
    {
#ifdef _WIN32
        curgroup.ipv6mr_interface = net_mcast6iface->integer;
#else
        curgroup.ipv6mr_interface = if_nametoindex(net_mcast6iface->string);
#endif
    }
    else
        curgroup.ipv6mr_interface = 0;
}

/*
====================
NET_JoinMulticast
Join an ipv6 multicast group
====================
*/
void NET_JoinMulticast6(void)
{
    /*
    TODO: accommodate
    int err;

    if(ip6_socket == INVALID_SOCKET || multicast6_socket != INVALID_SOCKET || (net_enabled->integer & NET_DISABLEMCAST))
            return;

    if(IN6_IS_ADDR_MULTICAST(&boundto.sin6_addr) || IN6_IS_ADDR_UNSPECIFIED(&boundto.sin6_addr))
    {
            // The way the socket was bound does not prohibit receiving multi-cast packets. So we don't need to open a
    new one.
            multicast6_socket = ip6_socket;
    }
    else
    {
            if((multicast6_socket = NET_IP6Socket(net_mcast6addr->string, ntohs(boundto.sin6_port), NULL, &err)) ==
    INVALID_SOCKET)
            {
                    // If the OS does not support binding to multicast addresses, like WinXP, at least try with the
    normal file descriptor.
                    multicast6_socket = ip6_socket;
            }
    }

    if(curgroup.ipv6mr_interface)
    {
            if (setsockopt(multicast6_socket, IPPROTO_IPV6, IPV6_MULTICAST_IF,
                                    (char *) &curgroup.ipv6mr_interface, sizeof(curgroup.ipv6mr_interface)) < 0)
            {
                    Com_Printf("NET_JoinMulticast6: Couldn't set scope on multicast socket: %s\n", NET_ErrorString());

                    if(multicast6_socket != ip6_socket)
                    {
                            closesocket(multicast6_socket);
                            multicast6_socket = INVALID_SOCKET;
                            return;
                    }
            }
    }

    if (setsockopt(multicast6_socket, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char *) &curgroup, sizeof(curgroup)))
    {
            Com_Printf("NET_JoinMulticast6: Couldn't join multicast group: %s\n", NET_ErrorString());

            if(multicast6_socket != ip6_socket)
            {
                    closesocket(multicast6_socket);
                    multicast6_socket = INVALID_SOCKET;
                    return;
            }
    }
    */
}

void NET_LeaveMulticast6()
{
    /*
    TODO: accommodate
    if(multicast6_socket != INVALID_SOCKET)
    {
            if(multicast6_socket != ip6_socket)
                    closesocket(multicast6_socket);
            else
                    setsockopt(multicast6_socket, IPPROTO_IPV6, IPV6_LEAVE_GROUP, (char *) &curgroup, sizeof(curgroup));

            multicast6_socket = INVALID_SOCKET;
    }
    */
}

/*
====================
NET_OpenSocks
====================
*/
void NET_OpenSocks(int port)
{
    /*
    TODO: accommodate
    struct sockaddr_in	address;
    struct hostent		*h;
    int					len;
    bool			rfc1929;
    unsigned char		buf[64];

    usingSocks = false;

    Com_Printf( "Opening connection to SOCKS server.\n" );

    if ( ( socks_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ) == INVALID_SOCKET ) {
            Com_Printf( "WARNING: NET_OpenSocks: socket: %s\n", NET_ErrorString() );
            return;
    }

    h = gethostbyname( net_socksServer->string );
    if ( h == NULL ) {
            Com_Printf( "WARNING: NET_OpenSocks: gethostbyname: %s\n", NET_ErrorString() );
            return;
    }
    if ( h->h_addrtype != AF_INET ) {
            Com_Printf( "WARNING: NET_OpenSocks: gethostbyname: address type was not AF_INET\n" );
            return;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = *(int *)h->h_addr_list[0];
    address.sin_port = htons( (short)net_socksPort->integer );

    if ( connect( socks_socket, (struct sockaddr *)&address, sizeof( address ) ) == SOCKET_ERROR ) {
            Com_Printf( "NET_OpenSocks: connect: %s\n", NET_ErrorString() );
            return;
    }

    // send socks authentication handshake
    if ( *net_socksUsername->string || *net_socksPassword->string ) {
            rfc1929 = true;
    }
    else {
            rfc1929 = false;
    }

    buf[0] = 5;		// SOCKS version
    // method count
    if ( rfc1929 ) {
            buf[1] = 2;
            len = 4;
    }
    else {
            buf[1] = 1;
            len = 3;
    }
    buf[2] = 0;		// method #1 - method id #00: no authentication
    if ( rfc1929 ) {
            buf[2] = 2;		// method #2 - method id #02: username/password
    }
    if ( send( socks_socket, (void *)buf, len, 0 ) == SOCKET_ERROR ) {
            Com_Printf( "NET_OpenSocks: send: %s\n", NET_ErrorString() );
            return;
    }

    // get the response
    len = recv( socks_socket, (void *)buf, 64, 0 );
    if ( len == SOCKET_ERROR ) {
            Com_Printf( "NET_OpenSocks: recv: %s\n", NET_ErrorString() );
            return;
    }
    if ( len != 2 || buf[0] != 5 ) {
            Com_Printf( "NET_OpenSocks: bad response\n" );
            return;
    }
    switch( buf[1] ) {
    case 0:	// no authentication
            break;
    case 2: // username/password authentication
            break;
    default:
            Com_Printf( "NET_OpenSocks: request denied\n" );
            return;
    }

    // do username/password authentication if needed
    if ( buf[1] == 2 ) {
            int		ulen;
            int		plen;

            // build the request
            ulen = strlen( net_socksUsername->string );
            plen = strlen( net_socksPassword->string );

            buf[0] = 1;		// username/password authentication version
            buf[1] = ulen;
            if ( ulen ) {
                    memcpy( &buf[2], net_socksUsername->string, ulen );
            }
            buf[2 + ulen] = plen;
            if ( plen ) {
                    memcpy( &buf[3 + ulen], net_socksPassword->string, plen );
            }

            // send it
            if ( send( socks_socket, (void *)buf, 3 + ulen + plen, 0 ) == SOCKET_ERROR ) {
                    Com_Printf( "NET_OpenSocks: send: %s\n", NET_ErrorString() );
                    return;
            }

            // get the response
            len = recv( socks_socket, (void *)buf, 64, 0 );
            if ( len == SOCKET_ERROR ) {
                    Com_Printf( "NET_OpenSocks: recv: %s\n", NET_ErrorString() );
                    return;
            }
            if ( len != 2 || buf[0] != 1 ) {
                    Com_Printf( "NET_OpenSocks: bad response\n" );
                    return;
            }
            if ( buf[1] != 0 ) {
                    Com_Printf( "NET_OpenSocks: authentication failed\n" );
                    return;
            }
    }

    // send the UDP associate request
    buf[0] = 5;		// SOCKS version
    buf[1] = 3;		// command: UDP associate
    buf[2] = 0;		// reserved
    buf[3] = 1;		// address type: IPV4
    *(int *)&buf[4] = INADDR_ANY;
    *(short *)&buf[8] = htons( (short)port );		// port
    if ( send( socks_socket, (void *)buf, 10, 0 ) == SOCKET_ERROR ) {
            Com_Printf( "NET_OpenSocks: send: %s\n", NET_ErrorString() );
            return;
    }

    // get the response
    len = recv( socks_socket, (void *)buf, 64, 0 );
    if( len == SOCKET_ERROR ) {
            Com_Printf( "NET_OpenSocks: recv: %s\n", NET_ErrorString() );
            return;
    }
    if( len < 2 || buf[0] != 5 ) {
            Com_Printf( "NET_OpenSocks: bad response\n" );
            return;
    }
    // check completion code
    if( buf[1] != 0 ) {
            Com_Printf( "NET_OpenSocks: request denied: %i\n", buf[1] );
            return;
    }
    if( buf[3] != 1 ) {
            Com_Printf( "NET_OpenSocks: relay address is not IPV4: %i\n", buf[3] );
            return;
    }
    ((struct sockaddr_in *)&socksRelayAddr)->sin_family = AF_INET;
    ((struct sockaddr_in *)&socksRelayAddr)->sin_addr.s_addr = *(int *)&buf[4];
    ((struct sockaddr_in *)&socksRelayAddr)->sin_port = *(short *)&buf[8];
    memset( ((struct sockaddr_in *)&socksRelayAddr)->sin_zero, 0, 8 );

    usingSocks = true;
    */
}

/*
=====================
NET_AddLocalAddress
=====================
*/
static void NET_AddLocalAddress(char *ifname, struct sockaddr *addr, struct sockaddr *netmask)
{
    int addrlen;
    sa_family_t family;

    // only add addresses that have all required info.
    if (!addr || !netmask || !ifname) return;

    family = addr->sa_family;

    if (numIP < MAX_IPS)
    {
        if (family == AF_INET)
        {
            addrlen = sizeof(struct sockaddr_in);
            localIP[numIP].type = NA_IP;
        }
        else if (family == AF_INET6)
        {
            addrlen = sizeof(struct sockaddr_in6);
            localIP[numIP].type = NA_IP6;
        }
        else
            return;

        Q_strncpyz(localIP[numIP].ifname, ifname, sizeof(localIP[numIP].ifname));

        localIP[numIP].family = family;

        memcpy(&localIP[numIP].addr, addr, addrlen);
        memcpy(&localIP[numIP].netmask, netmask, addrlen);

        numIP++;
    }
}

#if defined(__linux__) || defined(__APPLE__) || defined(__BSD__)
static void NET_GetLocalAddress(void)
{
    struct ifaddrs *ifap, *search;

    numIP = 0;

    if (getifaddrs(&ifap))
        Com_Printf("NET_GetLocalAddress: Unable to get list of network interfaces: %s\n", NET_ErrorString());
    else
    {
        for (search = ifap; search; search = search->ifa_next)
        {
            // Only add interfaces that are up.
            if (ifap->ifa_flags & IFF_UP) NET_AddLocalAddress(search->ifa_name, search->ifa_addr, search->ifa_netmask);
        }

        freeifaddrs(ifap);

        Sys_ShowIP();
    }
}
#else
static void NET_GetLocalAddress(void)
{
    char hostname[256];
    struct addrinfo hint;
    struct addrinfo *res = NULL;

    numIP = 0;

    if (gethostname(hostname, 256) == SOCKET_ERROR) return;

    memset(&hint, 0, sizeof(hint));

    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_DGRAM;

    if (!getaddrinfo(hostname, NULL, &hint, &res))
    {
        struct sockaddr_in mask4;
        struct sockaddr_in6 mask6;
        struct addrinfo *search;

        /* On operating systems where it's more difficult to find out the configured interfaces, we'll just assume a
         * netmask with all bits set. */

        memset(&mask4, 0, sizeof(mask4));
        memset(&mask6, 0, sizeof(mask6));
        mask4.sin_family = AF_INET;
        memset(&mask4.sin_addr.s_addr, 0xFF, sizeof(mask4.sin_addr.s_addr));
        mask6.sin6_family = AF_INET6;
        memset(&mask6.sin6_addr, 0xFF, sizeof(mask6.sin6_addr));

        // add all IPs from returned list.
        for (search = res; search; search = search->ai_next)
        {
            if (search->ai_family == AF_INET)
                NET_AddLocalAddress("", search->ai_addr, (struct sockaddr *)&mask4);
            else if (search->ai_family == AF_INET6)
                NET_AddLocalAddress("", search->ai_addr, (struct sockaddr *)&mask6);
        }

        Sys_ShowIP();
    }

    if (res) freeaddrinfo(res);
}
#endif

/*
====================
NET_OpenIP
====================
*/
void NET_OpenIP(void)
{
    int a;
    int i;
    int err;
    int ports[3];
    int port6s[3];

    for (a = 0; a < 3; ++a)
    {
        ports[a] = net_ports[a]->integer;
        port6s[a] = net_port6s[a]->integer;
    }

    NET_GetLocalAddress();

    for (a = 0; a < 3; ++a)
    {
        // indent
        if (a == 0 && (net_alternateProtocols->integer & NET_DISABLEPRIMPROTO)) continue;
        if (a == 1 && !(net_alternateProtocols->integer & NET_ENABLEALT1PROTO)) continue;
        if (a == 2 && !(net_alternateProtocols->integer & NET_ENABLEALT2PROTO)) continue;

        // automatically scan for a valid port, so multiple
        // dedicated servers can be started without requiring
        // a different net_port for each one

        if (net_enabled->integer & NET_ENABLEV6)
        {
            for (i = 0; i < 10; i++)
            {
                ip6_sockets[a] = NET_IP6Socket(a, net_ip6->string, port6s[a] + i, &boundto, &err);
                if (ip6_sockets[a] != INVALID_SOCKET)
                {
                    Cvar_SetValue((a == 2 ? "net_alt2port6" : a == 1 ? "net_alt1port6" : "net_port6"), port6s[a] + i);
                    break;
                }
                else
                {
                    if (err == EAFNOSUPPORT) break;
                }
            }
            if (ip6_sockets[a] == INVALID_SOCKET)
                Com_Printf("WARNING: Couldn't bind to a%s v6 ip address.\n",
                    (a == 2 ? "n alternate-2" : a == 1 ? "n alternate-1" : ""));
        }

        if (net_enabled->integer & NET_ENABLEV4)
        {
            for (i = 0; i < 10; i++)
            {
                ip_sockets[a] = NET_IPSocket(a, net_ip->string, ports[a] + i, &err);
                if (ip_sockets[a] != INVALID_SOCKET)
                {
                    Cvar_SetValue((a == 2 ? "net_alt2port" : a == 1 ? "net_alt1port" : "net_port"), ports[a] + i);

                    if (net_socksEnabled->integer) NET_OpenSocks(ports[a] + i);

                    break;
                }
                else
                {
                    if (err == EAFNOSUPPORT) break;
                }
            }

            if (ip_sockets[a] == INVALID_SOCKET)
                Com_Printf("WARNING: Couldn't bind to a%s v4 ip address.\n",
                    (a == 2 ? "n alternate-2" : a == 1 ? "n alternate-1" : ""));
        }
        // outdent
    }
}

//===================================================================

/*
====================
NET_GetCvars
====================
*/
static bool NET_GetCvars(void)
{
    int modified;
    int a;

#ifdef DEDICATED
    // I want server owners to explicitly turn on ipv6 support.
    net_enabled = Cvar_Get("net_enabled", "1", CVAR_LATCH | CVAR_ARCHIVE);
#else
    /* End users have it enabled so they can connect to ipv6-only hosts, but ipv4 will be
     * used if available due to ping */
    net_enabled = Cvar_Get("net_enabled", "3", CVAR_LATCH | CVAR_ARCHIVE);
#endif
    modified = net_enabled->modified;
    net_enabled->modified = false;

    net_alternateProtocols = Cvar_Get("net_alternateProtocols", "3", CVAR_LATCH | CVAR_ARCHIVE);
    modified += net_alternateProtocols->modified;
    net_alternateProtocols->modified = false;

    net_ip = Cvar_Get("net_ip", "0.0.0.0", CVAR_LATCH);
    modified += net_ip->modified;
    net_ip->modified = false;

    net_ip6 = Cvar_Get("net_ip6", "::", CVAR_LATCH);
    modified += net_ip6->modified;
    net_ip6->modified = false;

    for (a = 0; a < 3; ++a)
    {
        net_ports[a] = Cvar_Get((a == 2 ? "net_alt2port" : a == 1 ? "net_alt1port" : "net_port"),
            (a == 2 ? XSTRING(ALT2PORT_SERVER) : a == 1 ? XSTRING(ALT1PORT_SERVER) : XSTRING(PORT_SERVER)), CVAR_LATCH);
        modified += net_ports[a]->modified;
        net_ports[a]->modified = false;

        net_port6s[a] = Cvar_Get((a == 2 ? "net_alt2port6" : a == 1 ? "net_alt1port6" : "net_port6"),
            (a == 2 ? XSTRING(ALT2PORT_SERVER) : a == 1 ? XSTRING(ALT1PORT_SERVER) : XSTRING(PORT_SERVER)), CVAR_LATCH);
        modified += net_port6s[a]->modified;
        net_port6s[a]->modified = false;
    }

    // Some cvars for configuring multicast options which facilitates scanning for servers on local subnets.
    net_mcast6addr = Cvar_Get("net_mcast6addr", NET_MULTICAST_IP6, CVAR_LATCH | CVAR_ARCHIVE);
    modified += net_mcast6addr->modified;
    net_mcast6addr->modified = false;

#ifdef _WIN32
    net_mcast6iface = Cvar_Get("net_mcast6iface", "0", CVAR_LATCH | CVAR_ARCHIVE);
#else
    net_mcast6iface = Cvar_Get("net_mcast6iface", "", CVAR_LATCH | CVAR_ARCHIVE);
#endif
    modified += net_mcast6iface->modified;
    net_mcast6iface->modified = false;

    net_socksEnabled = Cvar_Get("net_socksEnabled", "0", CVAR_LATCH | CVAR_ARCHIVE);
    modified += net_socksEnabled->modified;
    net_socksEnabled->modified = false;

    net_socksServer = Cvar_Get("net_socksServer", "", CVAR_LATCH | CVAR_ARCHIVE);
    modified += net_socksServer->modified;
    net_socksServer->modified = false;

    net_socksPort = Cvar_Get("net_socksPort", "1080", CVAR_LATCH | CVAR_ARCHIVE);
    modified += net_socksPort->modified;
    net_socksPort->modified = false;

    net_socksUsername = Cvar_Get("net_socksUsername", "", CVAR_LATCH | CVAR_ARCHIVE);
    modified += net_socksUsername->modified;
    net_socksUsername->modified = false;

    net_socksPassword = Cvar_Get("net_socksPassword", "", CVAR_LATCH | CVAR_ARCHIVE);
    modified += net_socksPassword->modified;
    net_socksPassword->modified = false;

    net_dropsim = Cvar_Get("net_dropsim", "", CVAR_TEMP);

    return modified ? true : false;
}

/*
====================
NET_Config
====================
*/
void NET_Config(bool enableNetworking)
{
    bool modified;
    bool stop;
    bool start;
    int a;

    // get any latched changes to cvars
    modified = NET_GetCvars();

    if (!net_enabled->integer)
    {
        enableNetworking = false;
    }

    // if enable state is the same and no cvars were modified, we have nothing to do
    if (enableNetworking == networkingEnabled && !modified)
    {
        return;
    }

    if (enableNetworking == networkingEnabled)
    {
        if (enableNetworking)
        {
            stop = true;
            start = true;
        }
        else
        {
            stop = false;
            start = false;
        }
    }
    else
    {
        if (enableNetworking)
        {
            stop = false;
            start = true;
        }
        else
        {
            stop = true;
            start = false;
        }
        networkingEnabled = enableNetworking;
    }

    if (stop)
    {
        for (a = 0; a < 3; ++a)
        {
            if (ip_sockets[a] != INVALID_SOCKET)
            {
                closesocket(ip_sockets[a]);
                ip_sockets[a] = INVALID_SOCKET;
            }

            if (ip6_sockets[a] != INVALID_SOCKET)
            {
                closesocket(ip6_sockets[a]);
                ip6_sockets[a] = INVALID_SOCKET;
            }
        }

        /*
        TODO: accommodate
        if(multicast6_socket != INVALID_SOCKET)
        {
                if(multicast6_socket != ip6_socket)
                        closesocket(multicast6_socket);

                multicast6_socket = INVALID_SOCKET;
        }

        if ( socks_socket != INVALID_SOCKET ) {
                closesocket( socks_socket );
                socks_socket = INVALID_SOCKET;
        }
        */
    }

    if (start)
    {
        if (net_enabled->integer)
        {
            NET_OpenIP();
            NET_SetMulticast6();
        }
    }
}

/*
====================
NET_Init
====================
*/
void NET_Init(void)
{
#ifdef _WIN32
    int r;

    r = WSAStartup(MAKEWORD(1, 1), &winsockdata);
    if (r)
    {
        Com_Printf("WARNING: Winsock initialization failed, returned %d\n", r);
        return;
    }

    winsockInitialized = true;
    Com_Printf("Winsock Initialized\n");
#endif

    NET_Config(true);

    Cmd_AddCommand("net_restart", NET_Restart_f);
}

/*
====================
NET_Shutdown
====================
*/
void NET_Shutdown(void)
{
    if (!networkingEnabled)
    {
        return;
    }

    NET_Config(false);

#ifdef _WIN32
    WSACleanup();
    winsockInitialized = false;
#endif
}

/*
====================
NET_Event

Called from NET_Sleep which uses select() to determine which sockets have seen action.
====================
*/

void NET_Event(fd_set *fdr)
{
    uint8_t bufData[MAX_MSGLEN + 1];
    netadr_t from;
    msg_t netmsg;

    memset(&from, 0, sizeof(from));

    while (1)
    {
        MSG_Init(&netmsg, bufData, sizeof(bufData));

        if (NET_GetPacket(&from, &netmsg, fdr))
        {
            if (net_dropsim->value > 0.0f && net_dropsim->value <= 100.0f)
            {
                // com_dropsim->value percent of incoming packets get dropped.
                if (rand() < (int)(((double)RAND_MAX) / 100.0 * (double)net_dropsim->value))
                    continue;  // drop this packet
            }

            if (com_sv_running->integer)
                Com_RunAndTimeServerPacket(&from, &netmsg);
            else
                CL_PacketEvent(from, &netmsg);
        }
        else
            break;
    }
}

/*
====================
NET_Sleep

Sleeps msec or until something happens on the network
====================
*/
void NET_Sleep(int msec)
{
    struct timeval timeout;
    fd_set fdr;
    int retval;
    int a;
    SOCKET highestfd = INVALID_SOCKET;

    if (msec < 0) msec = 0;

    FD_ZERO(&fdr);

    for (a = 0; a < 3; ++a)
    {
        if (ip_sockets[a] != INVALID_SOCKET)
        {
            FD_SET(ip_sockets[a], &fdr);

            if (highestfd == INVALID_SOCKET || ip_sockets[a] > highestfd) highestfd = ip_sockets[a];
        }
        if (ip6_sockets[a] != INVALID_SOCKET)
        {
            FD_SET(ip6_sockets[a], &fdr);

            if (highestfd == INVALID_SOCKET || ip6_sockets[a] > highestfd) highestfd = ip6_sockets[a];
        }
    }

#ifdef _WIN32
    if (highestfd == INVALID_SOCKET)
    {
        // windows ain't happy when select is called without valid FDs
        SleepEx(msec, 0);
        return;
    }
#endif

    timeout.tv_sec = msec / 1000;
    timeout.tv_usec = (msec % 1000) * 1000;

    retval = select(highestfd + 1, &fdr, NULL, NULL, &timeout);

    if (retval == SOCKET_ERROR)
        Com_Printf("Warning: select() syscall failed: %s\n", NET_ErrorString());
    else if (retval > 0)
        NET_Event(&fdr);
}

/*
====================
NET_Restart_f
====================
*/
void NET_Restart_f(void) { NET_Config(true); }
