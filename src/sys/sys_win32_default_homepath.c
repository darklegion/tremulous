
/*
================
Sys_DefaultHomePath
================
*/
char *Sys_DefaultHomePath( void )
{
	TCHAR szPath[MAX_PATH];
	FARPROC qSHGetFolderPath;
	HMODULE shfolder = LoadLibrary("shfolder.dll");

	if(shfolder == NULL)
	{
		Com_Printf("Unable to load SHFolder.dll\n");
		return NULL;
	}

	if(!*homePath && com_homepath)
	{
		qSHGetFolderPath = GetProcAddress(shfolder, "SHGetFolderPathA");
		if(qSHGetFolderPath == NULL)
		{
			Com_Printf("Unable to find SHGetFolderPath in SHFolder.dll\n");
			FreeLibrary(shfolder);
			return NULL;
		}

		if( !SUCCEEDED( qSHGetFolderPath( NULL, CSIDL_APPDATA, NULL, 0, szPath ) ) )
		{
			Com_Printf("Unable to detect CSIDL_APPDATA\n");
			FreeLibrary(shfolder);
			return NULL;
		}
		
		Com_sprintf(homePath, sizeof(homePath), "%s%c", szPath, PATH_SEP);

		if(com_homepath->string[0])
			Q_strcat(homePath, sizeof(homePath), com_homepath->string);
		else
			Q_strcat(homePath, sizeof(homePath), HOMEPATH_NAME_WIN);
	}

	FreeLibrary(shfolder);
	return homePath;
}


