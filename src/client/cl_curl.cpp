/*
===========================================================================
Copyright (C) 2006 Tony J. White (tjw@tjw.org)
Copyright (C) 2000-2013 Darklegion Development
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

#include "client.h"

#ifdef USE_CURL_DLOPEN
#include "sys/sys_loadlib.h"

cvar_t *cl_cURLLib;

char* (*qcurl_version)(void);

CURL* (*qcurl_easy_init)(void);
CURLcode (*qcurl_easy_setopt)(CURL *curl, CURLoption option, ...);
CURLcode (*qcurl_easy_perform)(CURL *curl);
void (*qcurl_easy_cleanup)(CURL *curl);
CURLcode (*qcurl_easy_getinfo)(CURL *curl, CURLINFO info, ...);
CURL* (*qcurl_easy_duphandle)(CURL *curl);
void (*qcurl_easy_reset)(CURL *curl);
const char *(*qcurl_easy_strerror)(CURLcode);

CURLM* (*qcurl_multi_init)(void);
CURLMcode (*qcurl_multi_add_handle)(CURLM*, CURL*curl_handle);
CURLMcode (*qcurl_multi_remove_handle)(CURLM*, CURL*);
CURLMcode (*qcurl_multi_fdset)(CURLM*, fd_set* read_set, fd_set* write_set, fd_set* exc_set, int*);
CURLMcode (*qcurl_multi_perform)(CURLM*, int*);
CURLMcode (*qcurl_multi_cleanup)(CURLM*);
CURLMsg *(*qcurl_multi_info_read)(CURLM*, int*);
const char *(*qcurl_multi_strerror)(CURLMcode);

struct curl_slist* (*qcurl_slist_append)(struct curl_slist*, const char*);
void               (*qcurl_slist_free_all)(struct curl_slist*);

CURLcode (*qcurl_global_init)(long);
void     (*qcurl_global_cleanup)(void);


static void *cURLLib = NULL;

/*
=================
GPA
=================
*/
static void *GPA(const char *str)
{
	void* rv = Sys_LoadFunction(cURLLib, str);
	if(!rv)
	{
		Com_Printf("Can't load symbol %s\n", str);
		clc.cURLEnabled = false;
		return NULL;
	}
	else
	{
		Com_DPrintf("Loaded symbol %s (0x%p)\n", str, rv);
        return rv;
	}
}
#endif /* USE_CURL_DLOPEN */

/*
=================
CL_cURL_Init
=================
*/
bool CL_cURL_Init()
{
#ifdef USE_CURL_DLOPEN
	cl_cURLLib = Cvar_Get("cl_cURLLib", DEFAULT_CURL_LIB, CVAR_ARCHIVE | CVAR_PROTECTED);

	if(cURLLib)
		return true;

	Com_Printf("Loading \"%s\"...", cl_cURLLib->string);
	if( !(cURLLib = Sys_LoadDll(cl_cURLLib->string, true)) )
	{
#ifdef ALTERNATE_CURL_LIB
		// On some linux distributions there is no libcurl.so.3, but only libcurl.so.4. That one works too.
		if( !(cURLLib = Sys_LoadDll(ALTERNATE_CURL_LIB, true)) )
#endif
			return false;
	}

	clc.cURLEnabled = true;

	qcurl_version = (decltype(qcurl_version)) GPA("curl_version");

	qcurl_easy_init = (decltype(qcurl_easy_init)) GPA("curl_easy_init");
	qcurl_easy_setopt = (decltype(qcurl_easy_setopt)) GPA("curl_easy_setopt");
	qcurl_easy_perform = (decltype(qcurl_easy_perform)) GPA("curl_easy_perform");
	qcurl_easy_cleanup = (decltype(qcurl_easy_cleanup)) GPA("curl_easy_cleanup");
	qcurl_easy_getinfo = (decltype(qcurl_easy_getinfo)) GPA("curl_easy_getinfo");
	qcurl_easy_duphandle = (decltype(qcurl_easy_duphandle)) GPA("curl_easy_duphandle");
	qcurl_easy_reset = (decltype(qcurl_easy_reset)) GPA("curl_easy_reset");
	qcurl_easy_strerror = (decltype(qcurl_easy_strerror)) GPA("curl_easy_strerror");
	
	qcurl_multi_init = (decltype(qcurl_multi_init)) GPA("curl_multi_init");
	qcurl_multi_add_handle = (decltype(qcurl_multi_add_handle)) GPA("curl_multi_add_handle");
	qcurl_multi_remove_handle = (decltype(qcurl_multi_remove_handle)) GPA("curl_multi_remove_handle");
	qcurl_multi_fdset = (decltype(qcurl_multi_fdset)) GPA("curl_multi_fdset");
	qcurl_multi_perform = (decltype(qcurl_multi_perform)) GPA("curl_multi_perform");
	qcurl_multi_cleanup = (decltype(qcurl_multi_cleanup)) GPA("curl_multi_cleanup");
	qcurl_multi_info_read = (decltype(qcurl_multi_info_read)) GPA("curl_multi_info_read");
	qcurl_multi_strerror = (decltype(qcurl_multi_strerror)) GPA("curl_multi_strerror");
    qcurl_slist_append = (decltype(qcurl_slist_append)) GPA("curl_slist_append");
    qcurl_slist_free_all = (decltype(qcurl_slist_free_all)) GPA("curl_slist_free_all");
    qcurl_global_init = (decltype(qcurl_global_init)) GPA("curl_global_init");
    qcurl_global_cleanup = (decltype(qcurl_global_cleanup)) GPA("curl_global_cleanup");

	if(!clc.cURLEnabled)
	{
		CL_cURL_Shutdown();
		Com_Printf("FAIL One or more symbols not found\n");
		return false;
	}
	Com_Printf("OK\n");

	return true;
#else
	clc.cURLEnabled = true;
	return true;
#endif /* USE_CURL_DLOPEN */
}

/*
=================
CL_cURL_Shutdown
=================
*/
void CL_cURL_Shutdown( void )
{
	CL_cURL_Cleanup();
#ifdef USE_CURL_DLOPEN
	if(cURLLib)
	{
		Sys_UnloadLibrary(cURLLib);
		cURLLib = NULL;
	}
	qcurl_easy_init = NULL;
	qcurl_easy_setopt = NULL;
	qcurl_easy_perform = NULL;
	qcurl_easy_cleanup = NULL;
	qcurl_easy_getinfo = NULL;
	qcurl_easy_duphandle = NULL;
	qcurl_easy_reset = NULL;

	qcurl_multi_init = NULL;
	qcurl_multi_add_handle = NULL;
	qcurl_multi_remove_handle = NULL;
	qcurl_multi_fdset = NULL;
	qcurl_multi_perform = NULL;
	qcurl_multi_cleanup = NULL;
	qcurl_multi_info_read = NULL;
	qcurl_multi_strerror = NULL;
#endif /* USE_CURL_DLOPEN */
}

void CL_cURL_Cleanup(void)
{
	if(clc.downloadCURLM) {
		CURLMcode result;

		if(clc.downloadCURL) {
			result = qcurl_multi_remove_handle(clc.downloadCURLM,
				clc.downloadCURL);
			if(result != CURLM_OK) {
				Com_DPrintf("qcurl_multi_remove_handle failed: %s\n", qcurl_multi_strerror(result));
			}
			qcurl_easy_cleanup(clc.downloadCURL);
		}
		result = qcurl_multi_cleanup(clc.downloadCURLM);
		if(result != CURLM_OK) {
			Com_DPrintf("CL_cURL_Cleanup: qcurl_multi_cleanup failed: %s\n", qcurl_multi_strerror(result));
		}
		clc.downloadCURLM = NULL;
		clc.downloadCURL = NULL;
	}
	else if(clc.downloadCURL) {
		qcurl_easy_cleanup(clc.downloadCURL);
		clc.downloadCURL = NULL;
	}
}

static int CL_cURL_CallbackProgress( void *dummy, double dltotal, double dlnow,
	double ultotal, double ulnow )
{
	clc.downloadSize = (int)dltotal;
	Cvar_SetValue( "cl_downloadSize", clc.downloadSize );
	clc.downloadCount = (int)dlnow;
	Cvar_SetValue( "cl_downloadCount", clc.downloadCount );
	return 0;
}

static size_t CL_cURL_CallbackWrite(void *buffer, size_t size, size_t nmemb,
	void *stream)
{
	FS_Write( buffer, size*nmemb, ((fileHandle_t*)stream)[0] );
	return size*nmemb;
}

CURLcode qcurl_easy_setopt_warn(CURL *curl, CURLoption option, ...)
{
	CURLcode result;

	va_list argp;
	va_start(argp, option);

	if(option < CURLOPTTYPE_OBJECTPOINT) {
		long longValue = va_arg(argp, long);
		result = qcurl_easy_setopt(curl, option, longValue);
	} else if(option < CURLOPTTYPE_OFF_T) {
		void *pointerValue = va_arg(argp, void *);
		result = qcurl_easy_setopt(curl, option, pointerValue);
	} else {
		curl_off_t offsetValue = va_arg(argp, curl_off_t);
		result = qcurl_easy_setopt(curl, option, offsetValue);
	}

	if(result != CURLE_OK) {
		Com_DPrintf("qcurl_easy_setopt failed: %s\n", qcurl_easy_strerror(result));
	}
	va_end(argp);

	return result;
}

void CL_cURL_BeginDownload( const char *localName, const char *remoteURL )
{
	CURLMcode result;

	clc.cURLUsed = true;
	Com_Printf("URL: %s\n", remoteURL);
	Com_DPrintf("***** CL_cURL_BeginDownload *****\n"
		"Localname: %s\n"
		"RemoteURL: %s\n"
		"****************************\n", localName, remoteURL);
	CL_cURL_Cleanup();
	Q_strncpyz(clc.downloadURL, remoteURL, sizeof(clc.downloadURL));
	Q_strncpyz(clc.downloadName, localName, sizeof(clc.downloadName));
	Com_sprintf(clc.downloadTempName, sizeof(clc.downloadTempName),
		"%s.tmp", localName);

	// Set so UI gets access to it
	Cvar_Set("cl_downloadName", localName);
	Cvar_Set("cl_downloadSize", "0");
	Cvar_Set("cl_downloadCount", "0");
	Cvar_SetValue("cl_downloadTime", cls.realtime);

	clc.downloadBlock = 0; // Starting new file
	clc.downloadCount = 0;

	clc.downloadCURL = qcurl_easy_init();
	if(!clc.downloadCURL) {
		Com_Error(ERR_DROP, "CL_cURL_BeginDownload: qcurl_easy_init() "
			"failed");
		return;
	}
	clc.download = FS_SV_FOpenFileWrite(clc.downloadTempName);
	if(!clc.download) {
		Com_Error(ERR_DROP, "CL_cURL_BeginDownload: failed to open "
			"%s for writing", clc.downloadTempName);
		return;
	}

	if(com_developer->integer)
		qcurl_easy_setopt_warn(clc.downloadCURL, CURLOPT_VERBOSE, 1);
	qcurl_easy_setopt_warn(clc.downloadCURL, CURLOPT_URL, clc.downloadURL);
	qcurl_easy_setopt_warn(clc.downloadCURL, CURLOPT_TRANSFERTEXT, 0);
	qcurl_easy_setopt_warn(clc.downloadCURL, CURLOPT_REFERER, va("Tremulous://%s", NET_AdrToString(clc.serverAddress)));
	qcurl_easy_setopt_warn(clc.downloadCURL, CURLOPT_USERAGENT, va("%s %s", Q3_VERSION, qcurl_version()));
	qcurl_easy_setopt_warn(clc.downloadCURL, CURLOPT_WRITEFUNCTION, CL_cURL_CallbackWrite);
	qcurl_easy_setopt_warn(clc.downloadCURL, CURLOPT_WRITEDATA, &clc.download);
	qcurl_easy_setopt_warn(clc.downloadCURL, CURLOPT_NOPROGRESS, 0);
	qcurl_easy_setopt_warn(clc.downloadCURL, CURLOPT_PROGRESSFUNCTION, CL_cURL_CallbackProgress);
	qcurl_easy_setopt_warn(clc.downloadCURL, CURLOPT_PROGRESSDATA, NULL);
	qcurl_easy_setopt_warn(clc.downloadCURL, CURLOPT_FAILONERROR, 1);
	qcurl_easy_setopt_warn(clc.downloadCURL, CURLOPT_FOLLOWLOCATION, 1);
	qcurl_easy_setopt_warn(clc.downloadCURL, CURLOPT_MAXREDIRS, 5);
	qcurl_easy_setopt_warn(clc.downloadCURL, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS | CURLPROTO_FTP | CURLPROTO_FTPS );
	clc.downloadCURLM = qcurl_multi_init();	
	if(!clc.downloadCURLM)
    {
		qcurl_easy_cleanup(clc.downloadCURL);
		clc.downloadCURL = NULL;
		Com_Error(ERR_DROP, "CL_cURL_BeginDownload: qcurl_multi_init() "
			"failed");
		return;
	}

	result = qcurl_multi_add_handle(clc.downloadCURLM, clc.downloadCURL);
	if(result != CURLM_OK)
    {
		qcurl_easy_cleanup(clc.downloadCURL);
		clc.downloadCURL = NULL;
		Com_Error(ERR_DROP,"CL_cURL_BeginDownload: qcurl_multi_add_handle() failed: %s", qcurl_multi_strerror(result));
		return;
	}

	if(!(clc.sv_allowDownload & DLF_NO_DISCONNECT) && !clc.cURLDisconnected)
    {
		CL_AddReliableCommand("disconnect", true);
		CL_WritePacket();
		CL_WritePacket();
		CL_WritePacket();
		clc.cURLDisconnected = true;
	}
}

void CL_cURL_PerformDownload(void)
{
	CURLMcode res;
	CURLMsg *msg;
	int c;
	int i = 0;

	res = qcurl_multi_perform(clc.downloadCURLM, &c);
	while(res == CURLM_CALL_MULTI_PERFORM && i < 100) {
		res = qcurl_multi_perform(clc.downloadCURLM, &c);
		i++;
	}
	if(res == CURLM_CALL_MULTI_PERFORM)
		return;
	msg = qcurl_multi_info_read(clc.downloadCURLM, &c);
	if(msg == NULL) {
		return;
	}
	FS_FCloseFile(clc.download);
	if(msg->msg == CURLMSG_DONE && msg->data.result == CURLE_OK) {
		FS_SV_Rename(clc.downloadTempName, clc.downloadName, false);
		clc.downloadRestart = true;
	}
	else {
		long code;

		qcurl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &code);	
		Com_Error(ERR_DROP, "Download Error: %s Code: %ld URL: %s",
			qcurl_easy_strerror(msg->data.result),
			code, clc.downloadURL);
	}

	CL_NextDownload();
}
