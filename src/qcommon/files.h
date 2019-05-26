/*
 * This file is part of Tremulous.
 * Copyright © 2016 Victor Roemer (blowfish) <victor@badsec.org>
 * Copyright (C) 2015-2019 GrangerHub
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License,  or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not,  see <http://www.gnu.org/licenses/>.
 */

#ifndef QC_FILES_H
#define QC_FILES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "q_platform.h"
#include "q_shared.h"

// referenced flags
// these are in loop specific order so don't change the order
#define FS_GENERAL_REF  0x01
#define FS_UI_REF       0x02
#define FS_CGAME_REF    0x04

#define MAX_FILE_HANDLES 64

#define BASEGAME        "gpp"

#ifdef DEDICATED
#define Q3CONFIG_CFG "autogen_server.cfg"
#else
#define Q3CONFIG_CFG "autogen.cfg"
#endif

/*
 =============================================================

 QUAKE3 FILESYSTEM

 All of Quake's data access is through a hierarchical file system,  but the contents of
 the file system can be transparently merged from several sources.

 A "qpath" is a reference to game file data.  MAX_ZPATH is 256 characters,  which must include
 a terminating zero. "..",  "\\",  and ":" are explicitly illegal in qpaths to prevent any
 references outside the quake directory system.

 The "base path" is the path to the directory holding all the game directories and usually
 the executable.  It defaults to ".",  but can be overridden with a "+set fs_basepath c:\quake3"
 command line to allow code debugging in a different directory.  Basepath cannot
 be modified at all after startup.  Any files that are created (demos,  screenshots, 
 etc) will be created relative to the base path,  so base path should usually be writable.

 The "home path" is the path used for all write access. On win32 systems we have "base path"
 == "home path",  but on *nix systems the base installation is usually readonly,  and
 "home path" points to ~/.q3a or similar

 The user can also install custom mods and content in "home path",  so it should be searched
 along with "home path" and "cd path" for game content.


 The "base game" is the directory under the paths where data comes from by default,  and
 can be "base".

 The "current game" may be the same as the base game,  or it may be the name of another
 directory under the paths that should be searched for files before looking in the base game.
 This is the basis for addons.

 Clients automatically set the game directory after receiving a gamestate from a server, 
 so only servers need to worry about +set fs_game.

 No other directories outside of the base game and current game will ever be referenced by
 filesystem functions.

 To save disk space and speed loading,  directory trees can be collapsed into zip files.
 The files use a ".pk3" extension to prevent users from unzipping them accidentally,  but
 otherwise the are simply normal uncompressed zip files.  A game directory can have multiple
 zip files of the form "pak0.pk3",  "pak1.pk3",  etc.  Zip files are searched in decending order
 from the highest number to the lowest,  and will always take precedence over the filesystem.
 This allows a pk3 distributed as a patch to override all existing data.

 Because we will have updated executables freely available online,  there is no point to
 trying to restrict demo / oem versions of the game with code changes.  Demo / oem versions
 should be exactly the same executables as release versions,  but with different data that
 automatically restricts where game media can come from to prevent add-ons from working.

 File search order: when FS_FOpenFileRead gets called it will go through the fs_searchpaths
 structure and stop on the first successful hit. fs_searchpaths is built with successive
 calls to FS_AddGameDirectory

 Additionaly,  we search in several subdirectories:
 current game is the current mode
 base game is a variable to allow mods based on other mods
 (such as base + missionpack content combination in a mod for instance)
 BASEGAME is the hardcoded base game ("base")

 e.g. the qpath "sound/newstuff/test.wav" would be searched for in the following places:

 home path + current game's zip files
 home path + current game's directory
 base path + current game's zip files
 base path + current game's directory
 cd path + current game's zip files
 cd path + current game's directory

 home path + base game's zip file
 home path + base game's directory
 base path + base game's zip file
 base path + base game's directory
 cd path + base game's zip file
 cd path + base game's directory

 home path + BASEGAME's zip file
 home path + BASEGAME's directory
 base path + BASEGAME's zip file
 base path + BASEGAME's directory
 cd path + BASEGAME's zip file
 cd path + BASEGAME's directory

 server download,  to be written to home path + current game's directory


 The filesystem can be safely shutdown and reinitialized with different
 basedir / cddir / game combinations,  but all other subsystems that rely on it
 (sound,  video) must also be forced to restart.

 Because the same files are loaded by both the clip model (CM_) and renderer (TR_)
 subsystems,  a simple single-file caching scheme is used.  The CM_ subsystems will
 load the file with a request to cache.  Only one file will be kept cached at a time, 
 so any models that are going to be referenced by both subsystems should alternate
 between the CM_ load function and the ref load function.

 TODO: A qpath that starts with a leading slash will always refer to the base game,  even if another
 game is currently active.  This allows character models,  skins,  and sounds to be downloaded
 to a common directory no matter which game is active.

 How to prevent downloading zip files?
 Pass pk3 file names in systeminfo,  and download before FS_Restart (void)?

 Aborting a download disconnects the client from the server.

 How to mark files as downloadable?  Commercial add-ons won't be downloadable.

 Non-commercial downloads will want to download the entire zip file.
 the game would have to be reset to actually read the zip in

 Auto-update information

 Path separators

 Casing

   separate server gamedir and client gamedir,  so if the user starts
   a local game after having connected to a network game,  it won't stick
   with the network game.

   allow menu options for game selection?

 Read / write config to floppy option.

 Different version coexistance?

 When building a pak file,  make sure a autogen.cfg isn't present in it, 
 or configs will never get loaded from disk!

   todo:

   downloading (outside fs?)
   game directory passing and restarting

 =============================================================================
*/

//enum FS_Mode {
//	FS_READ,
//	FS_WRITE,
//	FS_APPEND,
//	FS_APPEND_SYNC
//};
//
//enum FS_Origin {
//	FS_SEEK_CUR,
//	FS_SEEK_END,
//	FS_SEEK_SET
//};

const char*  FS_GetCurrentGameDir (void);
void         FS_FilenameCompletion (const char* dir, const char* ext, bool stripExt, void (* callback)(const char* s), bool allowNonPureFilesOnDisk);
int          FS_FOpenFileByMode (const char* qpath, fileHandle_t* f, enum FS_Mode mode);
bool     FS_ConditionalRestart (int checksumFeed, bool disconnect);
void         FS_InitFilesystem (void);
void         FS_PureServerSetReferencedPaks (const char* pakSums, const char* pakNames);
void         FS_Restart (int checksumFeed);
void         FS_PureServerSetLoadedPaks (const char* pakSums, const char* pakNames);
void         FS_ClearPakReferences (int flags);
const char*  FS_ReferencedPakNames (bool alternate);
const char*  FS_ReferencedPakPureChecksums (void);
const char*  FS_ReferencedPakChecksums (bool alternate);
const char*  FS_LoadedPakPureChecksums (bool alternate);
const char*  FS_LoadedPakNames (bool alternate);
const char*  FS_LoadedPakChecksums (bool alternate);
void         FS_Shutdown (bool closemfp);
bool     FS_ComparePaks (char* neededpaks, int len, bool dlstring);
bool     FS_CheckDirTraversal (const char* checkdir);
void         FS_AddGameDirectory (const char* path, const char* dir);
bool     FS_Which (const char* filename, void* searchPath);
void         FS_SortFileList (char** filelist, int numfiles);
int          FS_PathCmp (const char* s1, const char* s2);
void         FS_ConvertPath (char* s);
int          FS_GetModList (char* listbuf, int bufsize);
int          FS_GetFileList (const char* path, const char* extension, char* listbuf, int bufsize);
void         FS_FreeFileList (char** list);
int          FS_GetFilteredFiles (const char *path, const char *extension, const char *filter, char *listbuf, int bufsize);
char**       FS_ListFiles (const char* path, const char* extension, int* numfiles);
char**       FS_ListFilteredFiles (const char* path, const char* extension, const char* filter, int* numfiles, bool allowNonPureFilesOnDisk);
bool     FS_CompareZipChecksum (const char* zipfile);
void         FS_WriteFile (const char* qpath, const void* buffer, int size);
void         FS_FreeFile (void* buffer);
long         FS_ReadFile (const char* qpath, void** buffer);
void         FS_Flush (fileHandle_t f);
long         FS_ReadFileDir (const char* qpath, void* searchPath, bool unpure, void** buffer);
int          FS_FileIsInPAK_A(bool alternate, const char *filename, int *pChecksum);
int          FS_FileIsInPAK (const char* filename, int* pChecksum);
int          FS_FTell (fileHandle_t f);
int          FS_Seek (fileHandle_t f, long offset, enum FS_Origin origin);
void QDECL   FS_Printf (fileHandle_t h, const char* fmt, ...);
int          FS_Write (const void* buffer, int len, fileHandle_t h);
int          FS_Read (void* buffer, int len, fileHandle_t f);
int          FS_Read (void* buffer, int len, fileHandle_t f);
int          FS_FindVM (void** startSearch, char* found, int foundlen, const char* name, int enableDll);
long         FS_FOpenFileRead (const char* filename, fileHandle_t* file, bool uniqueFILE);
long         FS_FOpenFileReadDir (const char* filename, void* search, fileHandle_t* file, bool uniqueFILE, bool unpure);
bool     FS_FilenameCompare (const char* s1, const char* s2);
fileHandle_t FS_FCreateOpenPipeFile (const char* filename);
fileHandle_t FS_FOpenFileAppend (const char* filename);
fileHandle_t FS_FOpenFileWrite (const char* filename);
void         FS_FCloseFile (fileHandle_t f);
void         FS_Rename (const char* from, const char* to);
void         FS_SV_Rename (const char* from, const char* to, bool safe);
long         FS_SV_FOpenFileRead (const char* filename, fileHandle_t* fp);
fileHandle_t FS_SV_FOpenFileWrite (const char* filename);
bool     FS_SV_FileExists (const char* file);
bool     FS_FileExists (const char* file);
bool     FS_FileInPathExists (const char* testpath);
void         FS_HomeRemove (const char* homePath);
void         FS_Remove (const char* osPath);
bool         FS_BrowseHomepath ( void );
bool         FS_OpenBaseGamePath( const char *baseGamePath );
bool     FS_CreatePath (const char* OSPath);
char*        FS_BuildOSPath (const char* base, const char* game, const char* qpath);
long         FS_filelength (fileHandle_t f);
void         FS_ReplaceSeparators (char *path);
void         FS_ForceFlush (fileHandle_t f);
int          FS_LoadStack (void);
bool         FS_Initialized (void);

void         FS_Which_f (void);
void         FS_TouchFile_f (void);
void         FS_Path_f (void);
void         FS_NewDir_f (void);
void         FS_Dir_f (void);


// XXX Delete me.
#if defined (FS_MISSING)
extern FILE *missingFiles;
#endif

extern char lastValidGame[MAX_OSPATH];
extern char lastValidBase[MAX_OSPATH];

#ifdef __cplusplus
}
#endif

#endif
