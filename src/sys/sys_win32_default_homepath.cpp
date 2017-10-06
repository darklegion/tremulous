#include "sys_local.h"

#include "qcommon/cvar.h"
#include "qcommon/q_shared.h"
#include "qcommon/q_platform.h"

#include <windows.h>
#include <lmerr.h>
#include <lmcons.h>
#include <lmwksta.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <conio.h>
#include <wincrypt.h>
#include <shlobj.h>
#include <psapi.h>
#include <float.h>

// Used to determine where to store user-specific files
static char homePath[ MAX_OSPATH ] = { 0 };

/*
================
Sys_DefaultHomePath
================
*/
char *Sys_DefaultHomePath( void )
{
    TCHAR szPath[MAX_PATH];

    if(!*homePath && com_homepath)
    {
        if( !SUCCEEDED( SHGetFolderPath( NULL, CSIDL_APPDATA, NULL, 0, szPath ) ) )
        {
            Com_Printf("Unable to detect CSIDL_APPDATA\n");
            return NULL;
        }
        
        Com_sprintf(homePath, sizeof(homePath), "%s%c", szPath, PATH_SEP);

        if(com_homepath->string[0])
            Q_strcat(homePath, sizeof(homePath), com_homepath->string);
        else
            Q_strcat(homePath, sizeof(homePath), HOMEPATH_NAME_WIN);
    }

    return homePath;
}

