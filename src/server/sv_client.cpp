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
// sv_client.c -- server code for dealing with clients

#include "server.h"

static void SV_CloseDownload( client_t *cl );

/*
=================
SV_RSA_VerifySignature

Verifies the signature of data and on success it returns the sha256
fingerprint of the public key.  pubkey, signature, and fingerprint
are all base 16 encoded strings.

Returns true on success
=================
*/
static bool SV_RSA_VerifySignature( const char *pubkey, const char *signature, const char *data, char *fingerprint )
{
	struct rsa_public_key public_key;
	struct sha256_ctx sha256_hash;
	uint8_t buf[RSA_STRING_LENGTH];
	int err;
	mpz_t n;

	if ( !*pubkey || !*signature )
		return false;

	// load public key
	rsa_public_key_init( &public_key );
	mpz_set_ui( public_key.e, RSA_PUBLIC_EXPONENT );
	err = mpz_set_str( public_key.n, pubkey, 16 );
	if ( err ) {
		rsa_public_key_clear( &public_key );
		return false;
	}

	err = !rsa_public_key_prepare( &public_key );
	if ( err ) {
		rsa_public_key_clear( &public_key );
		return false;
	}

	// load signature
	mpz_init( n );
	err = mpz_set_str( n, signature, 16 );
	if ( err ) {
		mpz_clear( n );
		rsa_public_key_clear( &public_key );
		return false;
	}

	// hash data
	sha256_init( &sha256_hash );
	sha256_update( &sha256_hash, strlen(data), (uint8_t *) data);

	if ( !rsa_sha256_verify( &public_key, &sha256_hash, n ) ) {
		mpz_clear( n );
		rsa_public_key_clear( &public_key );
		return false;
	}

	// VERIFIED, save the sha256 fingerprint of the key
	nettle_mpz_get_str_256( sizeof(buf), buf, public_key.n );

	sha256_update( &sha256_hash, sizeof(buf), buf );
	sha256_digest( &sha256_hash, SHA256_DIGEST_SIZE, buf );

	nettle_mpz_set_str_256_u( n, SHA256_DIGEST_SIZE, buf );
	mpz_get_str( fingerprint, 16, n );

	mpz_clear( n );
	rsa_public_key_clear( &public_key );
	return true;
}

/*
=================
SV_GetChallenge

A "getchallenge" OOB command has been received
Returns a challenge number that can be used
in a subsequent connectResponse command.
We do this to prevent denial of service attacks that
flood the server with invalid connection IPs.  With a
challenge, they must give a valid IP address.

If we are authorizing, a challenge request will cause a packet
to be sent to the authorize server.

When an authorizeip is returned, a challenge response will be
sent to that ip.

ioquake3: we added a possibility for clients to add a challenge
to their packets, to make it more difficult for malicious servers
to hi-jack client connections.
Also, the auth stuff is completely disabled for com_standalone games
as well as IPv6 connections, since there is no way to use the
v4-only auth server for these new types of connections.
=================
*/
void SV_GetChallenge(netadr_t from)
{
	int		i;
	int		oldest;
	int		oldestTime;
	int		oldestClientTime;
	int		clientChallenge;
	challenge_t	*challenge;
	bool wasfound = false;
	byte    buf[16];
	mpz_t   n;

	if (sv_protect->integer & SVP_IOQ3)
	if ( SVC_RateLimitAddress( from, 10, 1000 ) ) {
	{
		Com_DPrintf( "SV_GetChallenge: rate limit from %s exceeded, dropping request\n",
		// Prevent using getchallenge as an amplifier
			NET_AdrToString( from ) );
		if (SVC_RateLimitAddress(from, 10, 1000))
		return;
		{
	}
			SV_WriteAttackLog(va("SV_GetChallenge: rate limit from %s exceeded, dropping request\n",
			                     NET_AdrToString(from)));
			return;
		}


	// Allow getchallenge to be DoSed relatively easily, but prevent
		// Allow getchallenge to be DoSed relatively easily, but prevent
	// excess outbound bandwidth usage when being flooded inbound
		// excess outbound bandwidth usage when being flooded inbound
	if ( SVC_RateLimit( &outboundLeakyBucket, 10, 100 ) ) {
		if (SVC_RateLimit(&outboundLeakyBucket, 10, 100))
		Com_DPrintf( "SV_GetChallenge: rate limit exceeded, dropping request\n" );
		{
		return;
			SV_WriteAttackLog("SV_GetChallenge: rate limit exceeded, dropping request\n");
			return;
		}
	}
	}

	oldest = 0;
	oldestClientTime = oldestTime = 0x7fffffff;

	// see if we already have a challenge for this ip
	challenge = &svs.challenges[0];
	clientChallenge = atoi(Cmd_Argv(1));

	for(i = 0 ; i < MAX_CHALLENGES ; i++, challenge++)
	{
		if(!challenge->connected && NET_CompareAdr(from, challenge->adr))
		{
			wasfound = true;
			
			if(challenge->time < oldestClientTime)
				oldestClientTime = challenge->time;
		}
		
		if(wasfound && i >= MAX_CHALLENGES_MULTI)
		{
			i = MAX_CHALLENGES;
			break;
		}
		
		if(challenge->time < oldestTime)
		{
			oldestTime = challenge->time;
			oldest = i;
		}
	}

	if (i == MAX_CHALLENGES)
	{
		// this is the first time this client has asked for a challenge
		challenge = &svs.challenges[oldest];
		challenge->clientChallenge = clientChallenge;
		challenge->adr = from;
		challenge->firstTime = svs.time;
		challenge->connected = false;

		if ( sv_rsaAuth->integer ) {
			Sys_CryptoRandomBytes( buf, sizeof(buf) );
			nettle_mpz_init_set_str_256_u( n, sizeof(buf), buf );
			mpz_get_str( challenge->challenge2, 16, n );
			mpz_clear( n );
		}
	}

	// always generate a new challenge number, so the client cannot circumvent sv_maxping
	challenge->challenge = ( (rand() << 16) ^ rand() ) ^ svs.time;
	challenge->wasrefused = false;
	challenge->time = svs.time;
	challenge->pingTime = svs.time;

	if ( sv_rsaAuth->integer ) {
		NET_OutOfBandPrint( NS_SERVER, challenge->adr, "challengeResponse %d %d %d %s",
            challenge->challenge, clientChallenge, PROTOCOL_VERSION, challenge->challenge2 );
	}
	else {
		NET_OutOfBandPrint( NS_SERVER, challenge->adr, "challengeResponse %d %d %d",
            challenge->challenge, clientChallenge, PROTOCOL_VERSION );
	}
}

/*
==================
SV_DirectConnect

A "connect" OOB command has been received
==================
*/
void SV_DirectConnect( netadr_t from ) {
	char		userinfo[MAX_INFO_STRING];
	int			i;
	client_t	*cl, *newcl;
	client_t	temp;
	sharedEntity_t *ent;
	int			clientNum;
	int			version;
	int			qport;
	int			challenge;
	char		*password;
	int			startIndex;
	intptr_t		denied;
	int			count;
	const char	*ip;
	char		*challenge2;
	bool	    challenge2Verified = false;

	Com_DPrintf ("SVC_DirectConnect ()\n");

	// Prevent using connect as an amplifier
	if (sv_protect->integer & SVP_IOQ3)
	{
		if(SVC_RateLimitAddress(from, 10, 1000))
		{
			SV_WriteAttackLog(va("Bad direct connect - rate limit from %s exceeded, dropping request\n",
												 NET_AdrToString(from)));
			return;
		}
	}

	Q_strncpyz( userinfo, Cmd_Argv(1), sizeof(userinfo) );

	version = atoi( Info_ValueForKey( userinfo, "protocol" ) );
	if ( version != PROTOCOL_VERSION && version != 70 && version != 69 ) {
		NET_OutOfBandPrint(NS_SERVER, from, "print\nServer uses either protocol version %i, 70 or 69 "
					"(yours is %i).\n", PROTOCOL_VERSION, version);
		Com_DPrintf("    rejected connect from version %i\n", version);
		return;
	}

	challenge = atoi( Info_ValueForKey( userinfo, "challenge" ) );
	qport = atoi( Info_ValueForKey( userinfo, "qport" ) );

	// quick reject
	for (i=0,cl=svs.clients ; i < sv_maxclients->integer ; i++,cl++) {
		if ( cl->state == CS_FREE ) {
			continue;
		}
		if ( NET_CompareBaseAdr( from, cl->netchan.remoteAddress )
			&& ( cl->netchan.qport == qport 
			|| from.port == cl->netchan.remoteAddress.port ) ) {
			if (( svs.time - cl->lastConnectTime) 
				< (sv_reconnectlimit->integer * 1000)) {
				Com_DPrintf ("%s:reconnect rejected : too soon\n", NET_AdrToString (from));
				return;
			}
			break;
		}
	}
	
	// don't let "ip" overflow userinfo string
	if ( NET_IsLocalAddress (from) )
		ip = "localhost";
	else
		ip = (char *)NET_AdrToString( from );
	if( ( strlen( ip ) + strlen( userinfo ) + 4 ) >= MAX_INFO_STRING ) {
		NET_OutOfBandPrint( NS_SERVER, from,
			"print\nUserinfo string length exceeded.  "
			"Try removing setu cvars from your config.\n" );
		return;
	}
	Info_SetValueForKey( userinfo, "ip", ip );

	// see if the challenge is valid (LAN clients don't need to challenge)
	if (!NET_IsLocalAddress(from))
	{
		int ping;
		challenge_t *challengeptr;

		for (i=0; i<MAX_CHALLENGES; i++)
		{
			if (NET_CompareAdr(from, svs.challenges[i].adr))
			{
				if(challenge == svs.challenges[i].challenge)
					break;
			}
		}
		if (i == MAX_CHALLENGES)
		{
			NET_OutOfBandPrint( NS_SERVER, from, "print\nNo or bad challenge for your address.\n" );
			return;
		}
	
        if ( sv_rsaAuth->integer )
        {
			challenge2 = Info_ValueForKey( userinfo, "challenge2" );
			if ( !Q_stricmp( challenge2, svs.challenges[i].challenge2 ) )
            {
				challenge2Verified = true;
			}
		}

		challengeptr = &svs.challenges[i];
		
		if(challengeptr->wasrefused)
		{
			// Return silently, so that error messages written by the server keep being displayed.
			return;
		}

		ping = svs.time - challengeptr->pingTime;

		// never reject a LAN client based on ping
		if ( !Sys_IsLANAddress( from ) )
        {
			if ( sv_minPing->value && ping < sv_minPing->value ) {
				NET_OutOfBandPrint( NS_SERVER, from, "print\nServer is for high pings only\n" );
				Com_DPrintf ("Client %i rejected on a too low ping\n", i);
				challengeptr->wasrefused = true;
				return;
			}
			if ( sv_maxPing->value && ping > sv_maxPing->value ) {
				NET_OutOfBandPrint( NS_SERVER, from, "print\nServer is for low pings only\n" );
				Com_DPrintf ("Client %i rejected on a too high ping\n", i);
				challengeptr->wasrefused = true;
				return;
			}
		}

		Com_Printf("Client %i connecting with %i challenge ping\n", i, ping);
		challengeptr->connected = true;
	}

	// ignore any fingerprint set by the client
	char fingerprint[SHA256_DIGEST_SIZE * 2 + 1];
	Info_RemoveKey(userinfo, "fingerprint");
	fingerprint[0] = '\0';

	if ( sv_rsaAuth->integer && (NET_IsLocalAddress(from) || challenge2Verified) )
    {
		if ( SV_RSA_VerifySignature(Cmd_Argv(2), Cmd_Argv(3), Cmd_Argv(1), fingerprint) )
        {
			if( strlen(fingerprint) + strlen(userinfo) + 13 >= MAX_INFO_STRING )
            {
				NET_OutOfBandPrint( NS_SERVER, from, "print\nUserinfo string length exceeded.\n" );
				return;
			}
			Info_SetValueForKey( userinfo, "fingerprint", fingerprint );
			Com_DPrintf( "Public key fingerprint: %s\n", fingerprint );
		}
	}

	newcl = &temp;
	::memset(newcl, 0, sizeof(client_t));

	// if there is already a slot for this ip, reuse it
	for (i=0,cl=svs.clients ; i < sv_maxclients->integer ; i++,cl++)
    {
		if ( cl->state == CS_FREE )
			continue;

		if ( NET_CompareBaseAdr(from, cl->netchan.remoteAddress)
			&& (cl->netchan.qport == qport || from.port == cl->netchan.remoteAddress.port) )
        {
			Com_Printf ("%s:reconnect\n", NET_AdrToString (from));
			newcl = cl;

			// this doesn't work because it nukes the players userinfo
			// disconnect the client from the game first so any flags the
			// player might have are dropped
            //			VM_Call( sv.gvm, GAME_CLIENT_DISCONNECT, newcl - svs.clients );
			goto gotnewcl;
		}
	}

	// find a client slot
	// if "sv_privateClients" is set > 0, then that number
	// of client slots will be reserved for connections that
	// have "password" set to the value of "sv_privatePassword"
	// Info requests will report the maxclients as if the private
	// slots didn't exist, to prevent people from trying to connect
	// to a full server.
	// This is to allow us to reserve a couple slots here on our
	// servers so we can play without having to kick people.

	// check for privateClient password
	password = Info_ValueForKey( userinfo, "password" );
	if ( *password && !strcmp( password, sv_privatePassword->string ) ) {
		startIndex = 0;
	} else {
		// skip past the reserved slots
		startIndex = sv_privateClients->integer;
	}

	newcl = NULL;
	for ( i = startIndex; i < sv_maxclients->integer ; i++ ) {
		cl = &svs.clients[i];
		if (cl->state == CS_FREE) {
			newcl = cl;
			break;
		}
	}

	if ( !newcl ) {
		if ( NET_IsLocalAddress( from ) ) {
			Com_Error( ERR_FATAL, "server is full on local connect" );
			return;
		}
		else {
			NET_OutOfBandPrint( NS_SERVER, from, "print\nServer is full\n" );
			Com_DPrintf ("Rejected a connection.\n");
			return;
		}
	}

	// we got a newcl, so reset the reliableSequence and reliableAcknowledge
	cl->reliableAcknowledge = 0;
	cl->reliableSequence = 0;

gotnewcl:	
	// build a new connection
	// accept the new client
	// this is the only place a client_t is ever initialized
	*newcl = temp;
	clientNum = newcl - svs.clients;
	ent = SV_GentityNum( clientNum );
	newcl->gentity = ent;

	Cvar_Set( va( "sv_clAltProto%i", clientNum ), ( version == 69 ? "2" : version == 70 ? "1" : "0" ) );

	// save the challenge
	newcl->challenge = challenge;

	// save the address
	Netchan_Setup((version == 69 ? 2 : version == 70 ? 1 : 0), NS_SERVER, &newcl->netchan, from, qport, challenge);
	// init the netchan queue
	newcl->netchan_end_queue = &newcl->netchan_start_queue;

	// save the fingerprint
	Q_strncpyz( newcl->fingerprint, fingerprint, sizeof(newcl->fingerprint) );

	// save the userinfo
	Q_strncpyz( newcl->userinfo, userinfo, sizeof(newcl->userinfo) );

	// get the game a chance to reject this connection or modify the userinfo
	denied = VM_Call( sv.gvm, GAME_CLIENT_CONNECT, clientNum, true ); // firstTime = true
	if ( denied ) {
		// we can't just use VM_ArgPtr, because that is only valid inside a VM_Call
		char *str = (char*)VM_ExplicitArgPtr( sv.gvm, denied );

		NET_OutOfBandPrint( NS_SERVER, from, "print\n%s\n", str );
		Com_DPrintf ("Game rejected a connection: %s.\n", str);
		return;
	}

	SV_UserinfoChanged( newcl );

	// send the connect packet to the client
	NET_OutOfBandPrint(NS_SERVER, from, "connectResponse %d", challenge);

	Com_DPrintf( "Going from CS_FREE to CS_CONNECTED for %s\n", newcl->name );

	newcl->state = CS_CONNECTED;
	newcl->lastSnapshotTime = 0;
	newcl->lastPacketTime = svs.time;
	newcl->lastConnectTime = svs.time;
	
	// when we receive the first packet from the client, we will
	// notice that it is from a different serverid and that the
	// gamestate message was not just sent, forcing a retransmit
	newcl->gamestateMessageNum = -1;

	// if this was the first client on the server, or the last client
	// the server can hold, send a heartbeat to the master.
	count = 0;
	for (i=0,cl=svs.clients ; i < sv_maxclients->integer ; i++,cl++) {
		if ( svs.clients[i].state >= CS_CONNECTED )
			count++;
	}
	if ( count == 1 || count == sv_maxclients->integer ) {
		SV_Heartbeat_f();
	}
}

/*
=====================
SV_FreeClient

Destructor for data allocated in a client structure
=====================
*/
void SV_FreeClient(client_t *client)
{
#ifdef USE_VOIP
	int index;
	
	for(index = client->queuedVoipIndex; index < client->queuedVoipPackets; index++)
	{
		index %= ARRAY_LEN(client->voipPacket);
		
		Z_Free(client->voipPacket[index]);
	}
	
	client->queuedVoipPackets = 0;
#endif

	SV_Netchan_FreeQueue(client);
	SV_CloseDownload(client);
}

/*
=====================
SV_DropClient

Called when the player is totally leaving the server, either willingly
or unwillingly.  This is NOT called if the entire server is quiting
or crashing -- SV_FinalMessage() will handle that
=====================
*/
void SV_DropClient( client_t *drop, const char *reason ) {
	int		i;
	challenge_t	*challenge;

	if ( drop->state == CS_ZOMBIE ) {
		return;		// already dropped
	}

	// see if we already have a challenge for this ip
	challenge = &svs.challenges[0];

	for (i = 0 ; i < MAX_CHALLENGES ; i++, challenge++) {
		if ( NET_CompareAdr( drop->netchan.remoteAddress, challenge->adr ) ) {
			::memset(challenge, 0, sizeof(*challenge));
			break;
		}
	}

	// Free all allocated data on the client structure
	SV_FreeClient(drop);

	// tell everyone why they got dropped
	SV_SendServerCommand( NULL, "print \"%s" S_COLOR_WHITE " %s\n\"", drop->name, reason );

	// call the prog function for removing a client
	// this will remove the body, among other things
	VM_Call( sv.gvm, GAME_CLIENT_DISCONNECT, drop - svs.clients );

	// add the disconnect command
	SV_SendServerCommand( drop, "disconnect \"%s\"", reason);

	// nuke user info
	SV_SetUserinfo( drop - svs.clients, "" );
	
	Com_DPrintf( "Going to CS_ZOMBIE for %s\n", drop->name );
	drop->state = CS_ZOMBIE;		// become free in a few seconds

	// if this was the last client on the server, send a heartbeat
	// to the master so it is known the server is empty
	// send a heartbeat now so the master will get up to date info
	// if there is already a slot for this ip, reuse it
	for (i=0 ; i < sv_maxclients->integer ; i++ ) {
		if ( svs.clients[i].state >= CS_CONNECTED ) {
			break;
		}
	}
	if ( i == sv_maxclients->integer ) {
		SV_Heartbeat_f();
	}
}

extern char alternateInfos[2][2][BIG_INFO_STRING];

/*
================
SV_SendClientGameState

Sends the first message from the server to a connected client.
This will be sent on the initial connection and upon each new map load.

It will be resent if the client acknowledges a later message but has
the wrong gamestate.
================
*/
static void SV_SendClientGameState( client_t *client ) {
	int			start;
	entityState_t	*base, nullstate;
	msg_t		msg;
	byte		msgBuffer[MAX_MSGLEN];
	const char	*configstring;

 	Com_DPrintf ("SV_SendClientGameState() for %s\n", client->name);
	Com_DPrintf( "Going from CS_CONNECTED to CS_PRIMED for %s\n", client->name );
	client->state = CS_PRIMED;
	client->pureAuthentic = 0;
	client->gotCP = false;

	// when we receive the first packet from the client, we will
	// notice that it is from a different serverid and that the
	// gamestate message was not just sent, forcing a retransmit
	client->gamestateMessageNum = client->netchan.outgoingSequence;

	MSG_Init( &msg, msgBuffer, sizeof( msgBuffer ) );

	// NOTE, MRE: all server->client messages now acknowledge
	// let the client know which reliable clientCommands we have received
	MSG_WriteLong( &msg, client->lastClientCommand );

	// send any server commands waiting to be sent first.
	// we have to do this cause we send the client->reliableSequence
	// with a gamestate and it sets the clc.serverCommandSequence at
	// the client side
	SV_UpdateServerCommandsToClient( client, &msg );

	// send the gamestate
	MSG_WriteByte( &msg, svc_gamestate );
	MSG_WriteLong( &msg, client->reliableSequence );

	// write the configstrings
	for ( start = 0 ; start < MAX_CONFIGSTRINGS ; start++ ) {
		if ( start <= CS_SYSTEMINFO && client->netchan.alternateProtocol != 0 ) {
			configstring = alternateInfos[start][ client->netchan.alternateProtocol - 1 ];
		} else {
			configstring = sv.configstrings[start].s;
		}

		if (configstring[0]) {
			MSG_WriteByte( &msg, svc_configstring );
			MSG_WriteShort( &msg, start );
			MSG_WriteBigString( &msg, configstring );
		}
	}

	// write the baselines
	::memset( &nullstate, 0, sizeof( nullstate ) );
	for ( start = 0 ; start < MAX_GENTITIES; start++ ) {
		base = &sv.svEntities[start].baseline;
		if ( !base->number ) {
			continue;
		}
		MSG_WriteByte( &msg, svc_baseline );
		MSG_WriteDeltaEntity( client->netchan.alternateProtocol, &msg, &nullstate, base, true );
	}

	MSG_WriteByte( &msg, svc_EOF );

	MSG_WriteLong( &msg, client - svs.clients);

	// write the checksum feed
	MSG_WriteLong( &msg, sv.checksumFeed);

	// deliver this to the client
	SV_SendMessageToClient( &msg, client );
}


/*
==================
SV_ClientEnterWorld
==================
*/
void SV_ClientEnterWorld( client_t *client, usercmd_t *cmd ) {
	int		clientNum;
	sharedEntity_t *ent;

	Com_DPrintf( "Going from CS_PRIMED to CS_ACTIVE for %s\n", client->name );
	client->state = CS_ACTIVE;

	// resend all configstrings using the cs commands since these are
	// no longer sent when the client is CS_PRIMED
	SV_UpdateConfigstrings( client );

	// set up the entity for the client
	clientNum = client - svs.clients;
	ent = SV_GentityNum( clientNum );
	ent->s.number = clientNum;
	client->gentity = ent;

	client->deltaMessage = -1;
	client->lastSnapshotTime = 0;	// generate a snapshot immediately

	if(cmd)
		memcpy(&client->lastUsercmd, cmd, sizeof(client->lastUsercmd));
	else
		memset(&client->lastUsercmd, '\0', sizeof(client->lastUsercmd));

	// call the game begin function
	VM_Call( sv.gvm, GAME_CLIENT_BEGIN, client - svs.clients );
}

/*
============================================================

CLIENT COMMAND EXECUTION

============================================================
*/

/*
==================
SV_CloseDownload

clear/free any download vars
==================
*/
static void SV_CloseDownload( client_t *cl ) {
	int i;

	// EOF
	if (cl->download) {
		FS_FCloseFile( cl->download );
	}
	cl->download = 0;
	*cl->downloadName = 0;

	// Free the temporary buffer space
	for (i = 0; i < MAX_DOWNLOAD_WINDOW; i++) {
		if (cl->downloadBlocks[i]) {
			Z_Free(cl->downloadBlocks[i]);
			cl->downloadBlocks[i] = NULL;
		}
	}

}

/*
==================
SV_StopDownload_f

Abort a download if in progress
==================
*/
static void SV_StopDownload_f( client_t *cl ) {
	if (*cl->downloadName)
		Com_DPrintf( "clientDownload: %d : file \"%s\" aborted\n", (int) (cl - svs.clients), cl->downloadName );

	SV_CloseDownload( cl );
}

/*
==================
SV_DoneDownload_f

Downloads are finished
==================
*/
static void SV_DoneDownload_f( client_t *cl ) {
	if ( cl->state == CS_ACTIVE )
		return;

	Com_DPrintf( "clientDownload: %s Done\n", cl->name);
	// resend the game state to update any clients that entered during the download
	SV_SendClientGameState(cl);
}

/*
==================
SV_NextDownload_f

The argument will be the last acknowledged block from the client, it should be
the same as cl->downloadClientBlock
==================
*/
static void SV_NextDownload_f( client_t *cl )
{
	int block = atoi( Cmd_Argv(1) );

	if (block == cl->downloadClientBlock) {
		Com_DPrintf( "clientDownload: %d : client acknowledge of block %d\n", (int) (cl - svs.clients), block );

		// Find out if we are done.  A zero-length block indicates EOF
		if (cl->downloadBlockSize[cl->downloadClientBlock % MAX_DOWNLOAD_WINDOW] == 0) {
			Com_Printf( "clientDownload: %d : file \"%s\" completed\n", (int) (cl - svs.clients), cl->downloadName );
			SV_CloseDownload( cl );
			return;
		}

		cl->downloadSendTime = svs.time;
		cl->downloadClientBlock++;
		return;
	}
	// We aren't getting an acknowledge for the correct block, drop the client
	// FIXME: this is bad... the client will never parse the disconnect message
	//			because the cgame isn't loaded yet
	SV_DropClient( cl, "broken download" );
}

/*
==================
SV_BeginDownload_f
==================
*/
static void SV_BeginDownload_f( client_t *cl ) {

	// Kill any existing download
	SV_CloseDownload( cl );

	// cl->downloadName is non-zero now, SV_WriteDownloadToClient will see this and open
	// the file itself
	Q_strncpyz( cl->downloadName, Cmd_Argv(1), sizeof(cl->downloadName) );
}

/*
==================
SV_WriteDownloadToClient

Check to see if the client wants a file, open it if needed and start pumping the client
Fill up msg with data, return number of download blocks added
==================
*/
int SV_WriteDownloadToClient(client_t *cl, msg_t *msg)
{
	int curindex;
	int unreferenced = 1;
	char errorMessage[1024];
	char pakbuf[MAX_QPATH], *pakptr;
	int numRefPaks;

	if (!*cl->downloadName)
		return 0;	// Nothing being downloaded

	if(!cl->download)
	{
 		// Chop off filename extension.
		Com_sprintf(pakbuf, sizeof(pakbuf), "%s", cl->downloadName);
		pakptr = strrchr(pakbuf, '.');
		
		if(pakptr)
		{
			*pakptr = '\0';

			// Check for pk3 filename extension
			if(!Q_stricmp(pakptr + 1, "pk3"))
			{
				const char *referencedPaks = FS_ReferencedPakNames( cl->netchan.alternateProtocol == 2 );

				// Check whether the file appears in the list of referenced
				// paks to prevent downloading of arbitrary files.
				Cmd_TokenizeStringIgnoreQuotes(referencedPaks);
				numRefPaks = Cmd_Argc();

				for(curindex = 0; curindex < numRefPaks; curindex++)
				{
					if(!FS_FilenameCompare(Cmd_Argv(curindex), pakbuf))
					{
						unreferenced = 0;
						break;
					}
				}
			}
		}

		cl->download = 0;

		// We open the file here
		if ( !(sv_allowDownload->integer & DLF_ENABLE) ||
			(sv_allowDownload->integer & DLF_NO_UDP) ||
			unreferenced ||
			( cl->downloadSize = FS_SV_FOpenFileRead( cl->downloadName, &cl->download ) ) < 0 ) {
			// cannot auto-download file
			if(unreferenced)
			{
				Com_Printf("clientDownload: %d : \"%s\" is not referenced and cannot be downloaded.\n", (int) (cl - svs.clients), cl->downloadName);
				Com_sprintf(errorMessage, sizeof(errorMessage), "File \"%s\" is not referenced and cannot be downloaded.", cl->downloadName);
			}
			else if ( !(sv_allowDownload->integer & DLF_ENABLE) ||
				(sv_allowDownload->integer & DLF_NO_UDP) ) {

				Com_Printf("clientDownload: %d : \"%s\" download disabled\n", (int) (cl - svs.clients), cl->downloadName);
				if (sv_pure->integer) {
					Com_sprintf(errorMessage, sizeof(errorMessage), "Could not download \"%s\" because autodownloading is disabled on the server.\n\n"
										"You will need to get this file elsewhere before you "
										"can connect to this pure server.\n", cl->downloadName);
				} else {
					Com_sprintf(errorMessage, sizeof(errorMessage), "Could not download \"%s\" because autodownloading is disabled on the server.\n\n"
                    "The server you are connecting to is not a pure server, "
                    "set autodownload to No in your settings and you might be "
                    "able to join the game anyway.\n", cl->downloadName);
				}
			} else {
        // NOTE TTimo this is NOT supposed to happen unless bug in our filesystem scheme?
        //   if the pk3 is referenced, it must have been found somewhere in the filesystem
				Com_Printf("clientDownload: %d : \"%s\" file not found on server\n", (int) (cl - svs.clients), cl->downloadName);
				Com_sprintf(errorMessage, sizeof(errorMessage), "File \"%s\" not found on server for autodownloading.\n", cl->downloadName);
			}
			MSG_WriteByte( msg, svc_download );
			MSG_WriteShort( msg, 0 ); // client is expecting block zero
			MSG_WriteLong( msg, -1 ); // illegal file size
			MSG_WriteString( msg, errorMessage );

			*cl->downloadName = 0;
			
			if(cl->download)
				FS_FCloseFile(cl->download);
			
			return 1;
		}
 
		Com_Printf( "clientDownload: %d : beginning \"%s\"\n", (int) (cl - svs.clients), cl->downloadName );
		
		// Init
		cl->downloadCurrentBlock = cl->downloadClientBlock = cl->downloadXmitBlock = 0;
		cl->downloadCount = 0;
		cl->downloadEOF = false;
	}

	// Perform any reads that we need to
	while (cl->downloadCurrentBlock - cl->downloadClientBlock < MAX_DOWNLOAD_WINDOW &&
		cl->downloadSize != cl->downloadCount) {

		curindex = (cl->downloadCurrentBlock % MAX_DOWNLOAD_WINDOW);

		if (!cl->downloadBlocks[curindex])
			cl->downloadBlocks[curindex] = (unsigned char*)Z_Malloc(MAX_DOWNLOAD_BLKSIZE);

		cl->downloadBlockSize[curindex] = FS_Read( cl->downloadBlocks[curindex], MAX_DOWNLOAD_BLKSIZE, cl->download );

		if (cl->downloadBlockSize[curindex] < 0) {
			// EOF right now
			cl->downloadCount = cl->downloadSize;
			break;
		}

		cl->downloadCount += cl->downloadBlockSize[curindex];

		// Load in next block
		cl->downloadCurrentBlock++;
	}

	// Check to see if we have eof condition and add the EOF block
	if (cl->downloadCount == cl->downloadSize
      && !cl->downloadEOF
      && cl->downloadCurrentBlock - cl->downloadClientBlock < MAX_DOWNLOAD_WINDOW)
    {
		cl->downloadBlockSize[cl->downloadCurrentBlock % MAX_DOWNLOAD_WINDOW] = 0;
		cl->downloadCurrentBlock++;

		cl->downloadEOF = true;  // We have added the EOF block
	}

	if (cl->downloadClientBlock == cl->downloadCurrentBlock)
		return 0; // Nothing to transmit

	// Write out the next section of the file, if we have already reached our window,
	// automatically start retransmitting
	if (cl->downloadXmitBlock == cl->downloadCurrentBlock)
	{
		// We have transmitted the complete window, should we start resending?
		if (svs.time - cl->downloadSendTime > 1000)
			cl->downloadXmitBlock = cl->downloadClientBlock;
		else
			return 0;
	}

	// Send current block
	curindex = (cl->downloadXmitBlock % MAX_DOWNLOAD_WINDOW);

	MSG_WriteByte( msg, svc_download );
	MSG_WriteShort( msg, cl->downloadXmitBlock );

	// block zero is special, contains file size
	if ( cl->downloadXmitBlock == 0 )
		MSG_WriteLong( msg, cl->downloadSize );

	MSG_WriteShort( msg, cl->downloadBlockSize[curindex] );

	// Write the block
	if(cl->downloadBlockSize[curindex])
		MSG_WriteData(msg, cl->downloadBlocks[curindex], cl->downloadBlockSize[curindex]);

	Com_DPrintf( "clientDownload: %d : writing block %d\n", (int) (cl - svs.clients), cl->downloadXmitBlock );

	// Move on to the next block
	// It will get sent with next snap shot.  The rate will keep us in line.
	cl->downloadXmitBlock++;
	cl->downloadSendTime = svs.time;

	return 1;
}

/*
==================
SV_SendQueuedMessages

Send one round of fragments, or queued messages to all clients that have data pending.
Return the shortest time interval for sending next packet to client
==================
*/

int SV_SendQueuedMessages(void)
{
	int i, retval = -1, nextFragT;
	client_t *cl;
	
	for(i=0; i < sv_maxclients->integer; i++)
	{
		cl = &svs.clients[i];
		
		if(cl->state)
		{
			nextFragT = SV_RateMsec(cl);

			if(!nextFragT)
				nextFragT = SV_Netchan_TransmitNextFragment(cl);

			if(nextFragT >= 0 && (retval == -1 || retval > nextFragT))
				retval = nextFragT;
		}
	}

	return retval;
}


/*
==================
SV_SendDownloadMessages

Send one round of download messages to all clients
==================
*/

int SV_SendDownloadMessages(void)
{
	int i, numDLs = 0, retval;
	client_t *cl;
	msg_t msg;
	byte msgBuffer[MAX_MSGLEN];
	
	for(i=0; i < sv_maxclients->integer; i++)
	{
		cl = &svs.clients[i];
		
		if(cl->state && *cl->downloadName)
		{
			MSG_Init(&msg, msgBuffer, sizeof(msgBuffer));
			MSG_WriteLong(&msg, cl->lastClientCommand);
			
			retval = SV_WriteDownloadToClient(cl, &msg);
				
			if(retval)
			{
				MSG_WriteByte(&msg, svc_EOF);
				SV_Netchan_Transmit(cl, &msg);
				numDLs += retval;
			}
		}
	}

	return numDLs;
}

/*
=================
SV_Disconnect_f

The client is going to disconnect, so remove the connection immediately  FIXME: move to game?
=================
*/
static void SV_Disconnect_f( client_t *cl ) {
	SV_DropClient( cl, "disconnected" );
}

/*
=================
SV_VerifyPaks_f

If we are pure, disconnect the client if they do no meet the following conditions:

1. the first two checksums match our view of cgame and ui
2. there are no any additional checksums that we do not have

This routine would be a bit simpler with a goto but i abstained

=================
*/
static void SV_VerifyPaks_f( client_t *cl ) {
	int nChkSum1, nChkSum2, nClientPaks, nServerPaks, i, j, nCurArg;
	int nClientChkSum[1024];
	int nServerChkSum[1024];
	const char *pPaks, *pArg;
	bool bGood = true;

	// if we are pure, we "expect" the client to load certain things from 
	// certain pk3 files, namely we want the client to have loaded the
	// ui and cgame that we think should be loaded based on the pure setting
	//
	if ( sv_pure->integer != 0 ) {

		nChkSum1 = nChkSum2 = 0;
		// we run the game, so determine which cgame and ui the client "should" be running
		bGood = (FS_FileIsInPAK_A((cl->netchan.alternateProtocol == 2), "vm/cgame.qvm", &nChkSum1) == 1);
		if (bGood)
			bGood = (FS_FileIsInPAK_A((cl->netchan.alternateProtocol == 2), "vm/ui.qvm", &nChkSum2) == 1);

		nClientPaks = Cmd_Argc();

		// start at arg 2 ( skip serverId cl_paks )
		nCurArg = 1;

		pArg = Cmd_Argv(nCurArg++);
		if(!pArg) {
			bGood = false;
		}
		else
		{
			// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=475
			// we may get incoming cp sequences from a previous checksumFeed, which we need to ignore
			// since serverId is a frame count, it always goes up
			if (atoi(pArg) < sv.checksumFeedServerId)
			{
				Com_DPrintf("ignoring outdated cp command from client %s\n", cl->name);
				return;
			}
		}
	
		// we basically use this while loop to avoid using 'goto' :)
		while (bGood) {

			// must be at least 6: "cl_paks cgame ui @ firstref ... numChecksums"
			// numChecksums is encoded
			if (nClientPaks < 6) {
				bGood = false;
				break;
			}
			// verify first to be the cgame checksum
			pArg = Cmd_Argv(nCurArg++);
			if (!pArg || *pArg == '@' || atoi(pArg) != nChkSum1 ) {
				bGood = false;
				break;
			}
			// verify the second to be the ui checksum
			pArg = Cmd_Argv(nCurArg++);
			if (!pArg || *pArg == '@' || atoi(pArg) != nChkSum2 ) {
				bGood = false;
				break;
			}
			// should be sitting at the delimeter now
			pArg = Cmd_Argv(nCurArg++);
			if (*pArg != '@') {
				bGood = false;
				break;
			}
			// store checksums since tokenization is not re-entrant
			for (i = 0; nCurArg < nClientPaks; i++) {
				nClientChkSum[i] = atoi(Cmd_Argv(nCurArg++));
			}

			// store number to compare against (minus one cause the last is the number of checksums)
			nClientPaks = i - 1;

			// make sure none of the client check sums are the same
			// so the client can't send 5 the same checksums
			for (i = 0; i < nClientPaks; i++) {
				for (j = 0; j < nClientPaks; j++) {
					if (i == j)
						continue;
					if (nClientChkSum[i] == nClientChkSum[j]) {
						bGood = false;
						break;
					}
				}
				if (bGood == false)
					break;
			}
			if (bGood == false)
				break;

			// get the pure checksums of the pk3 files loaded by the server
			pPaks = FS_LoadedPakPureChecksums(cl->netchan.alternateProtocol == 2);
			Cmd_TokenizeString( pPaks );
			nServerPaks = Cmd_Argc();
			if (nServerPaks > 1024)
				nServerPaks = 1024;

			for (i = 0; i < nServerPaks; i++) {
				nServerChkSum[i] = atoi(Cmd_Argv(i));
			}

			// check if the client has provided any pure checksums of pk3 files not loaded by the server
			for (i = 0; i < nClientPaks; i++) {
				for (j = 0; j < nServerPaks; j++) {
					if (nClientChkSum[i] == nServerChkSum[j]) {
						break;
					}
				}
				if (j >= nServerPaks) {
					bGood = false;
					break;
				}
			}
			if ( bGood == false ) {
				break;
			}

			// check if the number of checksums was correct
			nChkSum1 = sv.checksumFeed;
			for (i = 0; i < nClientPaks; i++) {
				nChkSum1 ^= nClientChkSum[i];
			}
			nChkSum1 ^= nClientPaks;
			if (nChkSum1 != nClientChkSum[nClientPaks]) {
				bGood = false;
				break;
			}

			// break out
			break;
		}

		cl->gotCP = true;

		if (bGood) {
			cl->pureAuthentic = 1;
		} 
		else {
			cl->pureAuthentic = 0;
			cl->lastSnapshotTime = 0;
			cl->state = CS_ACTIVE;
			SV_SendClientSnapshot( cl );
			SV_SendServerCommand( cl, "disconnect \"Unpure Client. "
				"You may need to enable in-game downloads "
				"to connect to this server (set "
				"cl_allowDownload 1)\"" );
			SV_DropClient( cl, "Unpure client detected. Invalid .PK3 files referenced!" );
		}
	}
}

/*
=================
SV_ResetPureClient_f
=================
*/
static void SV_ResetPureClient_f( client_t *cl ) {
	cl->pureAuthentic = 0;
	cl->gotCP = false;
}

/*
=================
SV_UserinfoChanged

Pull specific info from a newly changed userinfo string
into a more C friendly form.
=================
*/
void SV_UserinfoChanged( client_t *cl ) {
	char	*val;
	const char *ip;
	int		i;
	int	len;

	// name for C code
	Q_strncpyz( cl->name, Info_ValueForKey (cl->userinfo, "name"), sizeof(cl->name) );

	// rate command

	// if the client is on the same subnet as the server and we aren't running an
	// internet public server, assume they don't need a rate choke
	if ( Sys_IsLANAddress( cl->netchan.remoteAddress ) && com_dedicated->integer != 2 && sv_lanForceRate->integer == 1) {
		cl->rate = 99999;	// lans should not rate limit
	} else {
		val = Info_ValueForKey (cl->userinfo, "rate");
		if (strlen(val)) {
			i = atoi(val);
			cl->rate = i;
			if (cl->rate < 1000) {
				cl->rate = 1000;
			} else if (cl->rate > 90000) {
				cl->rate = 90000;
			}
		} else {
			cl->rate = 3000;
		}
	}
	val = Info_ValueForKey (cl->userinfo, "handicap");
	if (strlen(val)) {
		i = atoi(val);
		if (i<=0 || i>100 || strlen(val) > 4) {
			Info_SetValueForKey( cl->userinfo, "handicap", "100" );
		}
	}

	// snaps command
	val = Info_ValueForKey (cl->userinfo, "snaps");
	
	if(strlen(val))
	{
		i = atoi(val);
		
		if(i < 1)
			i = 1;
		else if(i > sv_fps->integer)
			i = sv_fps->integer;

		i = 1000 / i;
	}
	else
		i = 50;

	if(i != cl->snapshotMsec)
	{
		// Reset last sent snapshot so we avoid desync between server frame time and snapshot send time
		cl->lastSnapshotTime = 0;
		cl->snapshotMsec = i;		
	}
	
#ifdef USE_VOIP
	val = Info_ValueForKey(cl->userinfo, "cl_voipProtocol");
	cl->hasVoip = !Q_stricmp( val, "opus" );
#endif

	// TTimo
	// maintain the IP information
	// the banning code relies on this being consistently present
	if( NET_IsLocalAddress(cl->netchan.remoteAddress) )
		ip = "localhost";
	else
		ip = (char*)NET_AdrToString( cl->netchan.remoteAddress );

	val = Info_ValueForKey( cl->userinfo, "ip" );
	if( val[0] )
		len = strlen( ip ) - strlen( val ) + strlen( cl->userinfo );
	else
		len = strlen( ip ) + 4 + strlen( cl->userinfo );

	if( len >= MAX_INFO_STRING )
		SV_DropClient( cl, "userinfo string length exceeded" );
	else
		Info_SetValueForKey( cl->userinfo, "ip", ip );

	val = Info_ValueForKey( cl->userinfo, "fingerprint" );
	if( val[0] )
		len = strlen(cl->fingerprint) - strlen(val) + strlen(cl->userinfo);
	else
		len = strlen(cl->fingerprint) + 13 + strlen(cl->userinfo);

	if( len >= MAX_INFO_STRING )
		SV_DropClient( cl, "userinfo string length exceeded" );
	else
		Info_SetValueForKey( cl->userinfo, "fingerprint", cl->fingerprint );
}


/*
==================
SV_UpdateUserinfo_f
==================
*/
static void SV_UpdateUserinfo_f( client_t *cl ) {
	Q_strncpyz( cl->userinfo, Cmd_Argv(1), sizeof(cl->userinfo) );

	SV_UserinfoChanged( cl );
	// call prog code to allow overrides
	VM_Call( sv.gvm, GAME_CLIENT_USERINFO_CHANGED, cl - svs.clients );
}


#ifdef USE_VOIP
static void SV_UpdateVoipIgnore(client_t *cl, const char *idstr, bool ignore)
{
	if ((*idstr >= '0') && (*idstr <= '9')) {
		const int id = atoi(idstr);
		if ((id >= 0) && (id < MAX_CLIENTS)) {
			cl->ignoreVoipFromClient[id] = ignore;
		}
	}
}

/*
==================
SV_Voip_f
==================
*/
static void SV_Voip_f( client_t *cl )
{
	const char *cmd = Cmd_Argv(1);
	if (strcmp(cmd, "ignore") == 0) {
		SV_UpdateVoipIgnore(cl, Cmd_Argv(2), true);
	} else if (strcmp(cmd, "unignore") == 0) {
		SV_UpdateVoipIgnore(cl, Cmd_Argv(2), false);
	} else if (strcmp(cmd, "muteall") == 0) {
		cl->muteAllVoip = true;
	} else if (strcmp(cmd, "unmuteall") == 0) {
		cl->muteAllVoip = false;
	}
}
#endif


typedef struct {
	const char *name;
	void	(*func)( client_t *cl );
} ucmd_t;

static ucmd_t ucmds[] = {
	{"userinfo", SV_UpdateUserinfo_f},
	{"disconnect", SV_Disconnect_f},
	{"cp", SV_VerifyPaks_f},
	{"vdr", SV_ResetPureClient_f},
	{"download", SV_BeginDownload_f},
	{"nextdl", SV_NextDownload_f},
	{"stopdl", SV_StopDownload_f},
	{"donedl", SV_DoneDownload_f},

#ifdef USE_VOIP
	{"voip", SV_Voip_f},
#endif

	{NULL, NULL}
};

/*
==================
SV_ExecuteClientCommand

Also called by bot code
==================
*/
void SV_ExecuteClientCommand( client_t *cl, const char *s, bool clientOK ) {
	ucmd_t	*u;
	bool bProcessed = false;
	
	Cmd_TokenizeString( s );

	// see if it is a server level command
	for (u=ucmds ; u->name ; u++) {
		if (!strcmp (Cmd_Argv(0), u->name) ) {
			u->func( cl );
			bProcessed = true;
			break;
		}
	}

	if (clientOK) {
		// pass unknown strings to the game
		if (!u->name && sv.state == SS_GAME && (cl->state == CS_ACTIVE || cl->state == CS_PRIMED)) {
			VM_Call( sv.gvm, GAME_CLIENT_COMMAND, cl - svs.clients );
		}
	}
	else if (!bProcessed)
		Com_DPrintf( "client text ignored for %s: %s\n", cl->name, Cmd_Argv(0) );
}

/*
===============
SV_ClientCommand
===============
*/
static bool SV_ClientCommand( client_t *cl, msg_t *msg ) {
	int		seq;
	const char	*s;
	bool clientOk = true;

	seq = MSG_ReadLong( msg );
	s = MSG_ReadString( msg );

	// see if we have already executed it
	if ( cl->lastClientCommand >= seq ) {
		return true;
	}

	Com_DPrintf( "clientCommand: %s : %i : %s\n", cl->name, seq, s );

	// drop the connection if we have somehow lost commands
	if ( seq > cl->lastClientCommand + 1 ) {
		Com_Printf( "Client %s lost %i clientCommands\n", cl->name, 
			seq - cl->lastClientCommand + 1 );
		SV_DropClient( cl, "Lost reliable commands" );
		return false;
	}

	// malicious users may try using too many string commands
	// to lag other players.  If we decide that we want to stall
	// the command, we will stop processing the rest of the packet,
	// including the usercmd.  This causes flooders to lag themselves
	// but not other people
	// We don't do this when the client hasn't been active yet since it's
	// normal to spam a lot of commands when downloading
#if 0 // flood protection in game for trem
	if ( !com_cl_running->integer && 
		cl->state >= CS_ACTIVE &&
		sv_floodProtect->integer && 
		svs.time < cl->nextReliableTime ) {
		// ignore any other text messages from this client but let them keep playing
		// TTimo - moved the ignored verbose to the actual processing in SV_ExecuteClientCommand, only printing if the core doesn't intercept
		clientOk = false;
	}
#endif

	// don't allow another command for one second
	cl->nextReliableTime = svs.time + 1000;

	SV_ExecuteClientCommand( cl, s, clientOk );

	cl->lastClientCommand = seq;
	Com_sprintf(cl->lastClientCommandString, sizeof(cl->lastClientCommandString), "%s", s);

	return true;		// continue procesing
}


//==================================================================================


/*
==================
SV_ClientThink

Also called by bot code
==================
*/
void SV_ClientThink (client_t *cl, usercmd_t *cmd) {
	cl->lastUsercmd = *cmd;

	if ( cl->state != CS_ACTIVE ) {
		return;		// may have been kicked during the last usercmd
	}

	VM_Call( sv.gvm, GAME_CLIENT_THINK, cl - svs.clients );
}

/*
==================
SV_UserMove

The message usually contains all the movement commands 
that were in the last three packets, so that the information
in dropped packets can be recovered.

On very fast clients, there may be multiple usercmd packed into
each of the backup packets.
==================
*/
static void SV_UserMove( client_t *cl, msg_t *msg, bool delta ) {
	int			i, key;
	int			cmdCount;
	usercmd_t	nullcmd;
	usercmd_t	cmds[MAX_PACKET_USERCMDS];
	usercmd_t	*cmd, *oldcmd;

	if ( delta ) {
		cl->deltaMessage = cl->messageAcknowledge;
	} else {
		cl->deltaMessage = -1;
	}

	cmdCount = MSG_ReadByte( msg );

	if ( cmdCount < 1 ) {
		Com_Printf( "cmdCount < 1\n" );
		return;
	}

	if ( cmdCount > MAX_PACKET_USERCMDS ) {
		Com_Printf( "cmdCount > MAX_PACKET_USERCMDS\n" );
		return;
	}

	// use the checksum feed in the key
	key = sv.checksumFeed;
	// also use the message acknowledge
	key ^= cl->messageAcknowledge;
	// also use the last acknowledged server command in the key
	key ^= MSG_HashKey(cl->netchan.alternateProtocol, cl->reliableCommands[ cl->reliableAcknowledge & (MAX_RELIABLE_COMMANDS-1) ], 32);

	::memset( &nullcmd, 0, sizeof(nullcmd) );
	oldcmd = &nullcmd;
	for ( i = 0 ; i < cmdCount ; i++ ) {
		cmd = &cmds[i];
		MSG_ReadDeltaUsercmdKey( msg, key, oldcmd, cmd );
		oldcmd = cmd;
	}

	// save time for ping calculation
	cl->frames[ cl->messageAcknowledge & PACKET_MASK ].messageAcked = svs.time;

	// TTimo
	// catch the no-cp-yet situation before SV_ClientEnterWorld
	// if CS_ACTIVE, then it's time to trigger a new gamestate emission
	// if not, then we are getting remaining parasite usermove commands, which we should ignore
	if (sv_pure->integer != 0 && cl->pureAuthentic == 0 && !cl->gotCP) {
		if (cl->state == CS_ACTIVE)
		{
			// we didn't get a cp yet, don't assume anything and just send the gamestate all over again
			Com_DPrintf( "%s: didn't get cp command, resending gamestate\n", cl->name);
			SV_SendClientGameState( cl );
		}
		return;
	}			
	
	// if this is the first usercmd we have received
	// this gamestate, put the client into the world
	if ( cl->state == CS_PRIMED ) {
		SV_ClientEnterWorld( cl, &cmds[0] );
		// the moves can be processed normaly
	}
	
	// a bad cp command was sent, drop the client
	if (sv_pure->integer != 0 && cl->pureAuthentic == 0) {		
		SV_DropClient( cl, "Cannot validate pure client!");
		return;
	}

	if ( cl->state != CS_ACTIVE ) {
		cl->deltaMessage = -1;
		return;
	}

	// usually, the first couple commands will be duplicates
	// of ones we have previously received, but the servertimes
	// in the commands will cause them to be immediately discarded
	for ( i =  0 ; i < cmdCount ; i++ ) {
		// if this is a cmd from before a map_restart ignore it
		if ( cmds[i].serverTime > cmds[cmdCount-1].serverTime ) {
			continue;
		}
		// extremely lagged or cmd from before a map_restart
		//if ( cmds[i].serverTime > svs.time + 3000 ) {
		//	continue;
		//}
		// don't execute if this is an old cmd which is already executed
		// these old cmds are included when cl_packetdup > 0
		if ( cmds[i].serverTime <= cl->lastUsercmd.serverTime ) {
			continue;
		}
		SV_ClientThink (cl, &cmds[ i ]);
	}
}


#ifdef USE_VOIP
/*
==================
SV_ShouldIgnoreVoipSender

Blocking of voip packets based on source client
==================
*/

static bool SV_ShouldIgnoreVoipSender(const client_t *cl)
{
	if (!sv_voip->integer)
		return true;  // VoIP disabled on this server.
	else if (!cl->hasVoip)  // client doesn't have VoIP support?!
		return true;
    
	// !!! FIXME: implement player blacklist.

	return false;  // don't ignore.
}

static void SV_UserVoip(client_t *cl, msg_t *msg, bool ignoreData)
{
	int sender, generation, sequence, frames;
	uint8_t recips[(MAX_CLIENTS + 7) / 8];
	int recip1 = 0, recip2 = 0, recip3 = 0; // silence warning
	int flags = 0;
	byte encoded[sizeof(cl->voipPacket[0]->data)];
	client_t *client = NULL;
	voipServerPacket_t *packet = NULL;
	int i;

	sender = cl - svs.clients;
	generation = MSG_ReadByte(msg);
	sequence = MSG_ReadLong(msg);
	frames = MSG_ReadByte(msg);
	if (cl->netchan.alternateProtocol == 0) {
		MSG_ReadData(msg, recips, sizeof(recips));
		flags = MSG_ReadByte(msg);
	} else {
		recip1 = MSG_ReadLong(msg);
		recip2 = MSG_ReadLong(msg);
		recip3 = MSG_ReadLong(msg);
	}
    size_t packetsize = MSG_ReadShort(msg);

	if (msg->readcount > msg->cursize)
		return;   // short/invalid packet, bail.

	if (packetsize > sizeof(encoded)) {  // overlarge packet?
		size_t bytesleft = packetsize;
		while (bytesleft) {
			size_t br = bytesleft;
			if (br > sizeof(encoded))
				br = sizeof(encoded);
			MSG_ReadData(msg, encoded, br);
			bytesleft -= br;
		}
		return;   // overlarge packet, bail.
	}

	MSG_ReadData(msg, encoded, packetsize);

	if (ignoreData || SV_ShouldIgnoreVoipSender(cl))
		return;   // Blacklisted, disabled, etc.

	// !!! FIXME: see if we read past end of msg...

	// !!! FIXME: reject if not opus data.
	// !!! FIXME: decide if this is bogus data?

	// decide who needs this VoIP packet sent to them...
	for (i = 0, client = svs.clients; i < sv_maxclients->integer ; i++, client++) {
		if (client->state != CS_ACTIVE)
			continue;  // not in the game yet, don't send to this guy.
		else if (i == sender)
			continue;  // don't send voice packet back to original author.
		else if (!client->hasVoip)
			continue;  // no VoIP support, or unsupported protocol
		else if (client->muteAllVoip)
			continue;  // client is ignoring everyone.
		else if (client->ignoreVoipFromClient[sender])
			continue;  // client is ignoring this talker.
		else if (*cl->downloadName)   // !!! FIXME: possible to DoS?
			continue;  // no VoIP allowed if downloading, to save bandwidth.

		if (cl->netchan.alternateProtocol == 0) {
			if(Com_IsVoipTarget(recips, sizeof(recips), i))
				flags |= VOIP_DIRECT;
			else
				flags &= ~VOIP_DIRECT;
		} else {
			if (i < 31 && (recip1 & (1 << (i - 0))) == 0)
				continue;  // not addressed to this player.
			else if (i >= 31 && i < 62 && (recip2 & (1 << (i - 31))) == 0)
				continue;  // not addressed to this player.
			else if (i >= 62 && (recip3 & (1 << (i - 62))) == 0)
				continue;  // not addressed to this player.

			flags |= VOIP_DIRECT;
		}

		if (!(flags & (VOIP_SPATIAL | VOIP_DIRECT)))
			continue;  // not addressed to this player.

		// Transmit this packet to the client.
		if (client->queuedVoipPackets >= ARRAY_LEN(client->voipPacket)) {
			Com_Printf("Too many VoIP packets queued for client #%d\n", i);
			continue;  // no room for another packet right now.
		}

		packet = (voipServerPacket_t*)Z_Malloc(sizeof(*packet));
		packet->sender = sender;
		packet->frames = frames;
		packet->len = packetsize;
		packet->generation = generation;
		packet->sequence = sequence;
		packet->flags = flags;
		memcpy(packet->data, encoded, packetsize);

		client->voipPacket[(client->queuedVoipIndex + client->queuedVoipPackets) % ARRAY_LEN(client->voipPacket)] = packet;
		client->queuedVoipPackets++;
	}
}
#endif



/*
===========================================================================

USER CMD EXECUTION

===========================================================================
*/

/*
===================
SV_ExecuteClientMessage

Parse a client packet
===================
*/
void SV_ExecuteClientMessage( client_t *cl, msg_t *msg ) {
	int			c;
	int			serverId;

	MSG_Bitstream(msg);

	serverId = MSG_ReadLong( msg );
	cl->messageAcknowledge = MSG_ReadLong( msg );

	if (cl->messageAcknowledge < 0) {
		// usually only hackers create messages like this
		// it is more annoying for them to let them hanging
#ifndef NDEBUG
		SV_DropClient( cl, "DEBUG: illegible client message" );
#endif
		return;
	}

	cl->reliableAcknowledge = MSG_ReadLong( msg );

	// NOTE: when the client message is fux0red the acknowledgement numbers
	// can be out of range, this could cause the server to send thousands of server
	// commands which the server thinks are not yet acknowledged in SV_UpdateServerCommandsToClient
	if (cl->reliableAcknowledge < cl->reliableSequence - MAX_RELIABLE_COMMANDS) {
		// usually only hackers create messages like this
		// it is more annoying for them to let them hanging
#ifndef NDEBUG
		SV_DropClient( cl, "DEBUG: illegible client message" );
#endif
		cl->reliableAcknowledge = cl->reliableSequence;
		return;
	}
	// if this is a usercmd from a previous gamestate,
	// ignore it or retransmit the current gamestate
	// 
	// if the client was downloading, let it stay at whatever serverId and
	// gamestate it was at.  This allows it to keep downloading even when
	// the gamestate changes.  After the download is finished, we'll
	// notice and send it a new game state
	//
	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=536
	// don't drop as long as previous command was a nextdl, after a dl is done, downloadName is set back to ""
	// but we still need to read the next message to move to next download or send gamestate
	// I don't like this hack though, it must have been working fine at some point, suspecting the fix is somewhere else
	if ( serverId != sv.serverId && !*cl->downloadName && !strstr(cl->lastClientCommandString, "nextdl") ) {
		if ( serverId >= sv.restartedServerId && serverId < sv.serverId ) { // TTimo - use a comparison here to catch multiple map_restart
			// they just haven't caught the map_restart yet
			Com_DPrintf("%s : ignoring pre map_restart / outdated client message\n", cl->name);
			return;
		}
		// if we can tell that the client has dropped the last
		// gamestate we sent them, resend it
		if ( cl->messageAcknowledge > cl->gamestateMessageNum ) {
			Com_DPrintf( "%s : dropped gamestate, resending\n", cl->name );
			SV_SendClientGameState( cl );
		}
		return;
	}

	// this client has acknowledged the new gamestate so it's
	// safe to start sending it the real time again
	if( cl->oldServerTime && serverId == sv.serverId ){
		Com_DPrintf( "%s acknowledged gamestate\n", cl->name );
		cl->oldServerTime = 0;
	}

	// read optional clientCommand strings
	do {
		c = MSG_ReadByte( msg );

		if ( cl->netchan.alternateProtocol != 0 ) {
			// See if this is an extension command after the EOF, which means we
			//  got data that a legacy server should ignore.
			if ( c == clc_EOF && MSG_LookaheadByte( msg ) == clc_voipSpeex ) {
				MSG_ReadByte( msg );  // throw the clc_extension byte away.
				c = MSG_ReadByte( msg );  // something legacy servers can't do!
				if ( c == clc_voipSpeex + 1 ) {
					c = clc_voipSpeex;
				}
				// sometimes you get a clc_extension at end of stream...dangling
				//  bits in the huffman decoder giving a bogus value?
				if ( c == -1 ) {
					c = clc_EOF;
				}
			}

			if ( c == svc_voipSpeex ) {
				c = svc_voipSpeex + 1;
			} else if ( c == svc_voipSpeex + 1 ) {
				c = svc_voipSpeex;
			}
		}

		if ( c == clc_EOF ) {
			break;
		}

		if ( c != clc_clientCommand ) {
			break;
		}
		if ( !SV_ClientCommand( cl, msg ) ) {
			return;	// we couldn't execute it because of the flood protection
		}
		if (cl->state == CS_ZOMBIE) {
			return;	// disconnect command
		}
	} while ( 1 );

	// skip legacy speex voip data
	if ( c == clc_voipSpeex ) {
#ifdef USE_VOIP
		SV_UserVoip( cl, msg, true );
		c = MSG_ReadByte( msg );
#endif
	}

	// read optional voip data
	if ( c == clc_voipOpus ) {
#ifdef USE_VOIP
		SV_UserVoip( cl, msg, false );
		c = MSG_ReadByte( msg );
#endif
	}

	// read the usercmd_t
	if ( c == clc_move ) {
		SV_UserMove( cl, msg, true );
	} else if ( c == clc_moveNoDelta ) {
		SV_UserMove( cl, msg, false );
	} else if ( c != clc_EOF ) {
		Com_Printf( "WARNING: bad command byte for client %i\n", (int) (cl - svs.clients) );
	}
//	if ( msg->readcount != msg->cursize ) {
//		Com_Printf( "WARNING: Junk at end of packet for client %i\n", cl - svs.clients );
//	}
}
