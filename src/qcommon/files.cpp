/*
   Copyright (C) 2016 Victor Roemer (wtfbbqhax), <victor@badsec.org>.
   Copyright (C) 2000-2013 Darklegion Development
   Copyright (C) 1999-2005 Id Software, Inc.
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
   
*/

#include "files.h"

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#endif

#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "cmd.h"
#include "cvar.h"
#include "md4.h"
#include "q_platform.h"
#include "q_shared.h"
#include "qcommon.h"
#include "unzip.h"
#include "vm.h"

#ifndef DEDICATED
#include "client/cl_rest.h"
#endif
#include "sys/sys_shared.h"

using namespace std;

#define MAX_ZPATH 256
#define MAX_SEARCH_PATHS 4096
#define MAX_FILEHASH_SIZE 1024

static bool FS_IsDemoExt(const char *filename);
static bool FS_IsExt(const char *filename, const char *ext, int namelen);

struct fileInPack_t {
    char* name;
    unsigned long pos;  // file info position in zip
    unsigned long len;  // uncompressed file size
    fileInPack_t* next;
};

struct pack_t {
    char pakPathname[MAX_OSPATH];  // /tremulous/baseq3
    char pakFilename[MAX_OSPATH];  // /tremulous/base/pak0.pk3
    char pakBasename[MAX_OSPATH];  // pak0
    char pakGamename[MAX_OSPATH];  // base
    unzFile handle;  // handle to zip file
    int checksum;  // regular checksum
    int pure_checksum;  // checksum for pure
    int numfiles;  // number of files in pk3
    int referenced;  // referenced file flags
    int hashSize;  // hash table size (power of 2)
    fileInPack_t **hashTable;  // hash table
    fileInPack_t *buildBuffer;  // buffer with the filenames etc.
    // some multiprotocol stuff
    bool onlyPrimary;
    bool onlyAlternate;
    pack_t *primaryVersion;

    // member functions
    inline fileInPack_t* find(string filename);
    inline bool is_pure();
};

struct directory_t {
    char path[MAX_OSPATH];
    char fullpath[MAX_OSPATH];  // /tremulous/base
    char gamedir[MAX_OSPATH];  // base
};

struct searchpath_t {
    pack_t *pack;  // only one of pack / dir will be non nullptr
    directory_t *dir;
    searchpath_t *next;
};

static char fs_gamedir[MAX_OSPATH];  // this will be a single file name with no separators
static cvar_t *fs_debug;
static cvar_t *fs_homepath;

static cvar_t *fs_basepath;
static cvar_t *fs_basegame;
#ifdef __APPLE__
static cvar_t *fs_apppath;  // Also search the .app bundle for .pk3 files
#endif
static cvar_t *fs_gamedirvar;

static searchpath_t *fs_searchpaths;
static int fs_readCount;  // total bytes read
static int fs_loadCount;  // total files read
static int fs_loadStack;  // total files in memory
static int fs_packFiles = 0;  // total number of files in packs

static int fs_checksumFeed;

union qfile_gut {
    FILE *o;
    unzFile z;
};

struct qfile_ut {
    qfile_gut file;
    bool unique;
};

struct fileHandleData_t {
    qfile_ut handleFiles;
    bool handleSync;
    int fileSize;
    int zipFilePos;
    int zipFileLen;
    bool zipFile;
    char name[MAX_ZPATH];

    void close();
};

static fileHandleData_t fsh[MAX_FILE_HANDLES];

// TTimo - https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=540
// wether we did a reorder on the current search path when joining the server

static bool fs_reordered;

// never load anything from pk3 files that are not present at the server when pure

static int fs_numServerPaks = 0;
static int fs_serverPaks[MAX_SEARCH_PATHS];  // checksums
static char *fs_serverPakNames[MAX_SEARCH_PATHS];  // pk3 names

// only used for autodownload, to make sure the client has at least
// all the pk3 files that are referenced at the server side

static int fs_numServerReferencedPaks;
static int fs_serverReferencedPaks[MAX_SEARCH_PATHS];  // checksums
static char *fs_serverReferencedPakNames[MAX_SEARCH_PATHS];  // pk3 names

// last valid game folder used
char lastValidBase[MAX_OSPATH];
char lastValidGame[MAX_OSPATH];

#ifdef FS_MISSING
FILE *missingFiles = nullptr;
#endif

/*
==============
FS_Initialized
==============
*/

bool FS_Initialized(void) { return fs_searchpaths ? true : false; }

/*
=================
pack_t::is_pure()

FIXME: also use hashed file names
=================
*/
inline bool pack_t::is_pure()
{
    if (fs_numServerPaks)
    {
        for (int i = 0; i < fs_numServerPaks; i++)
            if (checksum == fs_serverPaks[i])
                return true;

        return false;
    }
    return true;
}

/*
=================
FS_LoadStack
return load stack
=================
*/
int FS_LoadStack(void) { return fs_loadStack; }

inline fileInPack_t* pack_t::find(string filename)
{
    long hash = 0;
    auto fn = filename.c_str();
    for (long i = 0; fn[i]; i++)
    {
        long c = tolower(fn[i]);
        if (c == '.')
            break; // FIXME probably a bad idea

        if (c == '\\')
            c = '/';
        hash += c * (i + 119);
    }
    hash = (hash ^ (hash >> 10) ^ (hash >> 20));
    hash &= (hashSize - 1);

    for (auto file = hashTable[hash]; file; file = file->next)
    {
        if (FS_FilenameCompare(file->name, fn) == false)
            return file;
    }
    return nullptr;
}
/*
================
return a hash value for the filename
================
*/
static long FS_HashFileName(const char *fname, int hashSize)
{
    int i;
    long hash;
    char letter;

    hash = 0;
    i = 0;
    while (fname[i] != '\0')
    {
        letter = tolower(fname[i]);
        if (letter == '.') break;  // don't include extension
        if (letter == '\\') letter = '/';  // damn path names
        if (letter == PATH_SEP) letter = '/';  // damn path names
        hash += (long)(letter) * (i + 119);
        i++;
    }
    hash = (hash ^ (hash >> 10) ^ (hash >> 20));
    hash &= (hashSize - 1);
    return hash;
}

static fileHandle_t FS_HandleForFile(void)
{
    for (int i = 1; i < MAX_FILE_HANDLES; i++)
    {
        if (fsh[i].handleFiles.file.o == nullptr)
        {
            return i;
        }
    }
    Com_Error(ERR_DROP, "FS_HandleForFile: none free");
    return 0;
}

static FILE *FS_FileForHandle(fileHandle_t f)
{
    if (f < 1 || f >= MAX_FILE_HANDLES)
    {
        Com_Error(ERR_DROP, "FS_FileForHandle: out of range");
    }

    if (fsh[f].zipFile == true)
    {
        Com_Error(ERR_DROP, "FS_FileForHandle: can't get FILE on zip file");
    }

    if (!fsh[f].handleFiles.file.o)
    {
        Com_Error(ERR_DROP, "FS_FileForHandle: nullptr");
    }

    return fsh[f].handleFiles.file.o;
}

void FS_ForceFlush(fileHandle_t f)
{
    FILE *file = FS_FileForHandle(f);
    setvbuf(file, nullptr, _IONBF, 0);
}

/*
================
FS_fplength
================
*/

long FS_fplength(FILE *h)
{
    long pos = ftell(h);
    fseek(h, 0, SEEK_END);

    long end = ftell(h);
    fseek(h, pos, SEEK_SET);

    return end;
}

/*
================
FS_filelength

If this is called on a non-unique FILE (from a pak file),
it will return the size of the pak file, not the expected
size of the file.
================
*/
long FS_filelength(fileHandle_t f)
{
    FILE *h = FS_FileForHandle(f);
    if (h == nullptr) return -1;

    return FS_fplength(h);
}

/*
====================
FS_ReplaceSeparators

Fix things up differently for win/unix/mac
====================
*/
void FS_ReplaceSeparators(char *path)
{
    bool lastCharWasSep = false;

    for (char *s = path; *s; s++)
    {
        if (*s == '/' || *s == '\\')
        {
            if (!lastCharWasSep)
            {
                *s = PATH_SEP;
                lastCharWasSep = true;
            }
            else
            {
                memmove(s, s + 1, strlen(s));
            }
        }
        else
        {
            lastCharWasSep = false;
        }
    }
}

/*
===================
FS_BuildOSPath

Qpath may have either forward or backwards slashes
===================
*/
char *FS_BuildOSPath(const char *base, const char *game, const char *qpath)
{
    char temp[MAX_OSPATH];
    // "FIXME FS_BuildOSPath() returns static buffer with function scope"

    // This code will alternate between 2 different buffers-
    // XXX 3 or more calls to FS_BuildOSPath in a row are not safe.
    static char ospath[2][MAX_OSPATH];
    static bool toggle;

    toggle = !toggle;  // flip-flop to allow two returns without clash

    if (!game || !game[0])
    {
        game = fs_gamedir;
    }

    Com_sprintf(temp, sizeof(temp), "/%s/%s", game, qpath);
    FS_ReplaceSeparators(temp);
    Com_sprintf(ospath[toggle], sizeof(ospath[0]), "%s%s", base, temp);

    Com_DPrintf(S_COLOR_GREEN "%s: returning " S_COLOR_RED "%s\n",
            __FUNCTION__, ospath[toggle]);

    return ospath[toggle];
}

/*
============
FS_OpenWithDefault

Wrapper for Sys_OpenWithDefault()
============
*/
static bool FS_OpenWithDefault( const char *path )
{
    if( Sys_OpenWithDefault( path ) )
    {
        // minimize the client's window
        Cmd_ExecuteString( "minimize" );
        return true;
    }

    return false;
}

/*
============
FS_BrowseHomepath

Opens the homepath in the default file manager
============
*/
bool FS_BrowseHomepath( void )
{
    const char *homePath = Sys_DefaultHomePath( );

    if (!homePath || !homePath[0])
    {
        homePath = fs_basepath->string;
    }

    if( FS_OpenWithDefault( homePath ) )
        return true;

    Com_Printf( S_COLOR_RED "FS_BrowseHomepath: failed to open the homepath with the default file manager.\n" S_COLOR_WHITE );
    return false;
}

/*
============
FS_OpenBaseGamePath

Opens the given path for the
base game in the default file manager
============
*/
bool FS_OpenBaseGamePath( const char *baseGamePath )
{
    const char *homePath = Sys_DefaultHomePath( );
    const char *path;

    if (!homePath || !homePath[0])
    {
        homePath = fs_basepath->string;
    }

    path = FS_BuildOSPath( homePath, fs_basegame->string, baseGamePath);

    if( FS_OpenWithDefault( path ) )
        return true;

    Com_Printf( S_COLOR_RED "FS_BrowseHomepath: failed to open the homepath with the default file manager.\n" S_COLOR_WHITE );
    return false;
}

/*
============
FS_CreatePath

Creates any directories needed to store the given filename
============
*/
bool FS_CreatePath(const char *OSPath)
{
    // make absolutely sure that it can't back up the path
    // FIXME: is c: allowed???
    if (strstr(OSPath, "..") || strstr(OSPath, "::"))
    {
        Com_Printf("WARNING: refusing to create relative path \"%s\"\n", OSPath);
        return true;
    }

    char path[MAX_OSPATH];
    Q_strncpyz(path, OSPath, sizeof(path));
    FS_ReplaceSeparators(path);

    // Skip creation of the root directory as it will always be there
    char *ofs = strchr(path, PATH_SEP);
    if (ofs != nullptr)
    {
        ofs++;
    }

    for (; ofs != nullptr && *ofs; ofs++)
    {
        if (*ofs == PATH_SEP)
        {
            // create the directory
            *ofs = 0;
            if (!Sys_Mkdir(path))
            {
                Com_Error(ERR_FATAL, "FS_CreatePath: failed to create path \"%s\"", path);
            }
            *ofs = PATH_SEP;
        }
    }

    return false;
}

/*
=================
FS_CheckFilenameIsMutable

ERR_FATAL if trying to maniuplate a file with the platform library, QVM, or pk3 extension
=================
 */
static void FS_CheckFilenameIsMutable(const char *filename, const char *function)
{
    // Check if the filename ends with the library, QVM, or pk3 extension
    if (Sys_DllExtension(filename) || COM_CompareExtension(filename, ".qvm") ||
        COM_CompareExtension(filename, ".lua") || COM_CompareExtension(filename, ".pk3"))
    {
        Com_Error(ERR_FATAL, "%s: Not allowed to manipulate '%s' due to %s extension",
                function, filename, COM_GetExtension(filename));
    }
}

/*
===========
FS_Remove

===========
*/
void FS_Remove(const char *osPath)
{
    FS_CheckFilenameIsMutable(osPath, __FUNCTION__);
// RB begin
#if defined(_WIN32)
    ::DeleteFile(osPath);
#else
    remove(osPath);
#endif
}

/*
===========
FS_HomeRemove

===========
*/
void FS_HomeRemove(const char *homePath)
{
    FS_CheckFilenameIsMutable(homePath, __FUNCTION__);
    FS_Remove(FS_BuildOSPath(fs_homepath->string, fs_gamedir, homePath));
}

#if 0
bool FS_RemoveDir(const char* relativePath)
{
    bool success = true;
    success = Sys_Rmdir(FS_BuildOSPath(fs_homepath->string, fs_gamedir, relativePath));
    return success;
}
#endif

/*
================
FS_FileInPathExists

Tests if path and file exists
================
*/
bool FS_FileInPathExists(const char *testpath)
{
    FILE *filep;

    filep = Sys_FOpen(testpath, "rb");

    if (filep)
    {
        fclose(filep);
        return true;
    }

    return false;
}

/*
================
FS_FileExists

Tests if the file exists in the current gamedir, this DOES NOT
search the paths.  This is to determine if opening a file to write
(which always goes into the current gamedir) will cause any overwrites.
NOTE TTimo: this goes with FS_FOpenFileWrite for opening the file afterwards
================
*/
bool FS_FileExists(const char *file)
{
    return FS_FileInPathExists(FS_BuildOSPath(fs_homepath->string, fs_gamedir, file));
}

/*
================
FS_SV_FileExists

Tests if the file exists
================
*/
bool FS_SV_FileExists(const char *file)
{
    char *testpath = FS_BuildOSPath(fs_homepath->string, file, "");
    testpath[strlen(testpath) - 1] = '\0';

    return FS_FileInPathExists(testpath);
}

/*
===========
FS_SV_FOpenFileWrite

===========
*/
fileHandle_t FS_SV_FOpenFileWrite(const char *filename)
{
    if (!fs_searchpaths)
    {
        Com_Error(ERR_FATAL, "Filesystem call made without initialization");
    }

    char *ospath = FS_BuildOSPath(fs_homepath->string, filename, "");
    ospath[strlen(ospath) - 1] = '\0';

    fileHandle_t f = FS_HandleForFile();
    fsh[f].zipFile = false;

    Com_DPrintf("FS_SV_FOpenFileWrite: %s\n", ospath);

    FS_CheckFilenameIsMutable(ospath, __FUNCTION__);

    if (FS_CreatePath(ospath))
    {
        return 0;
    }

    Com_DPrintf("writing to: %s\n", ospath);
    fsh[f].handleFiles.file.o = Sys_FOpen(ospath, "wb");

    Q_strncpyz(fsh[f].name, filename, sizeof(fsh[f].name));

    fsh[f].handleSync = false;
    if (!fsh[f].handleFiles.file.o)
    {
        f = 0;
    }

    return f;
}

/*
===========
FS_SV_FOpenFileRead

Search for a file somewhere below the home path then base path
in that order
===========
*/
long FS_SV_FOpenFileRead(const char *filename, fileHandle_t *fp)
{
    if (!fs_searchpaths)
    {
        Com_Error(ERR_FATAL, "Filesystem call made without initialization");
    }

    fileHandle_t f = FS_HandleForFile();
    fsh[f].zipFile = false;

    Q_strncpyz(fsh[f].name, filename, sizeof(fsh[f].name));

    // don't let sound stutter
    S_ClearSoundBuffer();

    // search homepath
    char *ospath = FS_BuildOSPath(fs_homepath->string, filename, "");
    // remove trailing slash
    ospath[strlen(ospath) - 1] = '\0';

    Com_DPrintf("FS_SV_FOpenFileRead (fs_homepath): %s\n", ospath);

    fsh[f].handleFiles.file.o = Sys_FOpen(ospath, "rb");
    fsh[f].handleSync = false;
    if (!fsh[f].handleFiles.file.o)
    {
        // If fs_homepath == fs_basepath, don't bother
        if (Q_stricmp(fs_homepath->string, fs_basepath->string))
        {
            // search basepath
            ospath = FS_BuildOSPath(fs_basepath->string, filename, "");
            ospath[strlen(ospath) - 1] = '\0';

            Com_DPrintf("FS_SV_FOpenFileRead (fs_basepath): %s\n", ospath);

            fsh[f].handleFiles.file.o = Sys_FOpen(ospath, "rb");
            fsh[f].handleSync = false;
        }

        if (!fsh[f].handleFiles.file.o)
        {
            f = 0;
        }
    }

    *fp = f;
    if (f)
    {
        return FS_filelength(f);
    }

    return -1;
}

/*
===========
FS_SV_Rename

===========
*/
void FS_SV_Rename(const char *from, const char *to, bool safe)
{
    if (!fs_searchpaths)
    {
        Com_Error(ERR_FATAL, "Filesystem call made without initialization");
    }

    // don't let sound stutter
    S_ClearSoundBuffer();

    char *from_ospath = FS_BuildOSPath(fs_homepath->string, from, "");
    char *to_ospath = FS_BuildOSPath(fs_homepath->string, to, "");

    from_ospath[strlen(from_ospath) - 1] = '\0';
    to_ospath[strlen(to_ospath) - 1] = '\0';

    Com_DPrintf("FS_SV_Rename: (%s) %s --> %s\n", safe ? "safe" : "unsafe", from_ospath, to_ospath);

    if (safe)
    {
        FS_CheckFilenameIsMutable(to_ospath, __FUNCTION__);
    }

    rename(from_ospath, to_ospath);
}

/*
===========
FS_Rename

===========
*/
void FS_Rename(const char *from, const char *to)
{
    if (!fs_searchpaths)
    {
        Com_Error(ERR_FATAL, "Filesystem call made without initialization");
    }

    // don't let sound stutter
    S_ClearSoundBuffer();

    char *from_ospath = FS_BuildOSPath(fs_homepath->string, fs_gamedir, from);
    char *to_ospath = FS_BuildOSPath(fs_homepath->string, fs_gamedir, to);

    Com_DPrintf("FS_Rename: %s --> %s\n", from_ospath, to_ospath);

    FS_CheckFilenameIsMutable(to_ospath, __FUNCTION__);

    rename(from_ospath, to_ospath);
}

/*
==============
FS_FCloseFile

If the FILE pointer is an open pak file, leave it open.

For some reason, other dll's can't just cal fclose()
on files returned by FS_FOpenFile...
==============
*/
void fileHandleData_t::close()
{
    if (zipFile == true)
    {
        unzCloseCurrentFile(handleFiles.file.z);

        if (handleFiles.unique)
            unzClose(handleFiles.file.z);
    }
    // we didn't find it as a pak, so close it as a unique file
    else if (handleFiles.file.o)
    {
        ::fclose(handleFiles.file.o);
    }

    ::memset(this, 0, sizeof(*this));
}

void FS_FCloseFile(fileHandle_t f)
{
    if (!fs_searchpaths)
        Com_Error(ERR_FATAL, "Filesystem call made without initialization");

    fsh[f].close();

    ::memset(&fsh[f], 0, sizeof(fsh[f]));
}

/*
===========
FS_FOpenFileWrite

===========
*/
fileHandle_t FS_FOpenFileWrite(const char *filename)
{

    if (!fs_searchpaths)
    {
        Com_Error(ERR_FATAL, "Filesystem call made without initialization");
    }

    fileHandle_t f = FS_HandleForFile();
    fsh[f].zipFile = false;

    char *ospath = FS_BuildOSPath(fs_homepath->string, fs_gamedir, filename);

    Com_DPrintf("FS_FOpenFileWrite: %s\n", ospath);

    FS_CheckFilenameIsMutable(ospath, __FUNCTION__);

    if (FS_CreatePath(ospath))
    {
        return 0;
    }

    // enabling the following line causes a recursive function call loop
    // when running with +set logfile 1 +set developer 1
    // Com_DPrintf( "writing to: %s\n", ospath );
    fsh[f].handleFiles.file.o = Sys_FOpen(ospath, "wb");

    Q_strncpyz(fsh[f].name, filename, sizeof(fsh[f].name));

    fsh[f].handleSync = false;
    if (!fsh[f].handleFiles.file.o)
    {
        f = 0;
    }
    return f;
}

/*
===========
FS_FOpenFileAppend

===========
*/
fileHandle_t FS_FOpenFileAppend(const char *filename)
{
    char *ospath;
    fileHandle_t f;

    if (!fs_searchpaths)
    {
        Com_Error(ERR_FATAL, "Filesystem call made without initialization");
    }

    f = FS_HandleForFile();
    fsh[f].zipFile = false;

    Q_strncpyz(fsh[f].name, filename, sizeof(fsh[f].name));

    // don't let sound stutter
    S_ClearSoundBuffer();

    ospath = FS_BuildOSPath(fs_homepath->string, fs_gamedir, filename);

    Com_DPrintf("FS_FOpenFileAppend: %s\n", ospath);

    FS_CheckFilenameIsMutable(ospath, __FUNCTION__);

    if (FS_CreatePath(ospath))
    {
        return 0;
    }

    fsh[f].handleFiles.file.o = Sys_FOpen(ospath, "ab");
    fsh[f].handleSync = false;
    if (!fsh[f].handleFiles.file.o)
    {
        f = 0;
    }
    return f;
}

/*
===========
FS_FCreateOpenPipeFile

===========
*/
fileHandle_t FS_FCreateOpenPipeFile(const char *filename)
{
    if (!fs_searchpaths)
    {
        Com_Error(ERR_FATAL, "Filesystem call made without initialization");
    }

    fileHandle_t f = FS_HandleForFile();
    fsh[f].zipFile = false;

    Q_strncpyz(fsh[f].name, filename, sizeof(fsh[f].name));

    // don't let sound stutter
    S_ClearSoundBuffer();

    char *ospath = FS_BuildOSPath(fs_homepath->string, fs_gamedir, filename);

    Com_DPrintf("FS_FCreateOpenPipeFile: %s\n", ospath);

    FS_CheckFilenameIsMutable(ospath, __FUNCTION__);

    FILE *fifo = Sys_Mkfifo(ospath);
    if (fifo)
    {
        fsh[f].handleFiles.file.o = fifo;
        fsh[f].handleSync = false;
    }
    else
    {
        Com_Printf(S_COLOR_YELLOW
            "WARNING: Could not create new com_pipefile at %s. "
            "com_pipefile will not be used.\n",
            ospath);
        f = 0;
    }

    return f;
}

/*
===========
FS_FilenameCompare

Ignore case and seprator char distinctions
===========
*/
bool FS_FilenameCompare(const char *s1, const char *s2)
{
    int c1, c2;

    do
    {
        c1 = *s1++;
        c2 = *s2++;

        if (c1 >= 'a' && c1 <= 'z')
        {
            c1 -= ('a' - 'A');
        }
        if (c2 >= 'a' && c2 <= 'z')
        {
            c2 -= ('a' - 'A');
        }

        if (c1 == '\\' || c1 == ':')
        {
            c1 = '/';
        }
        if (c2 == '\\' || c2 == ':')
        {
            c2 = '/';
        }

        if (c1 != c2)
        {
            return true;  // strings not equal
        }
    } while (c1);

    return false;  // strings are equal
}

/*
===========
FS_IsExt

Return true if ext matches file extension filename
===========
*/
static bool FS_IsExt(const char *filename, const char *ext, int namelen)
{
    int extlen = strlen(ext);

    if (extlen > namelen) return false;

    filename += namelen - extlen;

    return !Q_stricmp(filename, ext);
}

/*
===========
FS_IsDemoExt

Return true if filename has a demo extension
===========
*/

static bool FS_IsDemoExt(const char *filename)
{
    const char *ext_test = strrchr(filename, '.');
    if (ext_test && !Q_stricmpn(ext_test + 1, DEMOEXT, ARRAY_LEN(DEMOEXT) - 1))
    {
        int protocol = atoi(ext_test + ARRAY_LEN(DEMOEXT));
        if (protocol == PROTOCOL_VERSION) return true;

        for (int i = 0; demo_protocols[i]; i++)
            if (demo_protocols[i] == protocol) return true;
    }

    return false;
}

/*
===========
FS_FOpenFileReadDir

Tries opening file "filename" in searchpath "search"
Returns filesize and an open FILE pointer.
===========
*/
long FS_FOpenFileReadDir(
    const char *filename, void *_search, fileHandle_t *file, bool uniqueFILE, bool unpure)
{
    pack_t *pak;
    directory_t *dir;
    char *netpath;
    FILE *filep;
    int len;

    searchpath_t *search = static_cast<searchpath_t *>(_search);

    if (filename == nullptr)
        Com_Error(ERR_FATAL, "FS_FOpenFileRead: nullptr 'filename' parameter passed");

    // qpaths are not supposed to have a leading slash
    if (filename[0] == '/' || filename[0] == '\\') filename++;

    // make absolutely sure that it can't back up the path.
    // The searchpaths do guarantee that something will always
    // be prepended, so we don't need to worry about "c:" or "//limbo"
    if (strstr(filename, "..") || strstr(filename, "::"))
    {
        if (file == nullptr) return false;

        *file = 0;
        return -1;
    }

    if (file == nullptr)
    {
        // just wants to see if file is there

        if ( fs_debug->integer )
        {
            Com_Printf(S_COLOR_GREEN "Searching for: " S_COLOR_RED "%s\n", filename);
        }

        // is the element a pak file?
        if (search->pack)
        {
            auto pakfile = search->pack->find(filename);
            if (pakfile)
            {
                // found it!
                if (!pakfile->len)
                {
                    // FIXME: It's not nice, but legacy code depends on
                    // positive value if file exists no matter what size
                    return 1;
                }

                return pakfile->len;
            }
        }
        else if (search->dir)
        {
            dir = search->dir;

            netpath = FS_BuildOSPath(dir->path, dir->gamedir, filename);
            filep = Sys_FOpen(netpath, "rb");

            if (filep)
            {
                len = FS_fplength(filep);
                fclose(filep);

                if (len)
                    return len;
                else
                    return 1;
            }
        }

        return 0;
    }

    *file = FS_HandleForFile();
    fsh[*file].handleFiles.unique = uniqueFILE;

    // is the element a pak file?
    if (search->pack)
    {
        pak = search->pack;
        auto pakfile = pak->find(filename);
        if (pakfile )
        {
            if ( fs_debug->integer == 2 )
                Com_Printf(S_COLOR_GREEN "#2 Searching for: " S_COLOR_RED "%s\n", filename);

            // disregard if it doesn't match one of the allowed pure pak files
            if (!unpure && !pak->is_pure())
            {
                if ( fs_debug->integer == 2 )
                    Com_Printf(S_COLOR_GREEN "Ugh-oh %s found in unpure pk3\n", filename);

                *file = 0;
                return -1;
            }

            len = strlen(filename);

            if (!(pak->referenced & FS_GENERAL_REF))
            {
                if ( !FS_IsExt(filename, ".shader", len)
                        && !FS_IsExt(filename, ".mtr", len)
                        && !FS_IsExt(filename, ".txt", len)
                        && !FS_IsExt(filename, ".cfg", len)
                        && !FS_IsExt(filename, ".config", len)
                        && !FS_IsExt(filename, ".arena", len)
                        && !FS_IsExt(filename, ".menu", len)
                        && !strstr(filename, "levelshots") )
                {
                    pak->referenced |= FS_GENERAL_REF;
                }
            }

            if (strstr(filename, "cgame.qvm"))
                pak->referenced |= FS_CGAME_REF;

            if (strstr(filename, "ui.qvm"))
                pak->referenced |= FS_UI_REF;

            if (uniqueFILE)
            {
                fsh[*file].handleFiles.file.z = unzOpen(pak->pakFilename);
                if ( !fsh[*file].handleFiles.file.z )
                    Com_Error(ERR_FATAL, "Couldn't open %s", pak->pakFilename);
            }
            else
            {
                fsh[*file].handleFiles.file.z = pak->handle;
            }

            Q_strncpyz(fsh[*file].name, filename, sizeof(fsh[*file].name));
            fsh[*file].zipFile = true;

            // set the file position in the zip file (also sets the current file info)
            unzSetOffset(fsh[*file].handleFiles.file.z, pakfile->pos);

            // open the file in the zip
            unzOpenCurrentFile(fsh[*file].handleFiles.file.z);
            fsh[*file].zipFilePos = pakfile->pos;
            fsh[*file].zipFileLen = pakfile->len;

            if ( fs_debug->integer )
            {
                Com_Printf( "FS_FOpenFileRead: %s (found in '%s')\n",
                        filename, pak->pakFilename);
            }

            return pakfile->len;
        }
    }
    else if (search->dir)
    {
        // check a file in the directory tree

        // if we are running restricted, the only files we
        // will allow to come from the directory are .cfg files
        len = strlen(filename);
        // FIXME TTimo I'm not sure about the fs_numServerPaks test
        // if you are using FS_ReadFile to find out if a file exists,
        //   this test can make the search fail although the file is in the directory
        // I had the problem on https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=8
        // turned out I used FS_FileExists instead
        if (!unpure && fs_numServerPaks)
        {
            if (!FS_IsExt(filename, ".cfg", len) &&  // for config files
                !FS_IsExt(filename, ".lua", len) &&  // lua 
                !FS_IsExt(filename, ".menu", len) &&  // menu files
                !FS_IsExt(filename, ".game", len) &&  // menu files
                !FS_IsExt(filename, ".dat", len) &&  // for journal files
                !FS_IsDemoExt(filename))  // demos
            {
                *file = 0;
                return -1;
            }
        }

        dir = search->dir;

        netpath = FS_BuildOSPath(dir->path, dir->gamedir, filename);
        filep = Sys_FOpen(netpath, "rb");

        if (filep == nullptr)
        {
            *file = 0;
            return -1;
        }

        Q_strncpyz(fsh[*file].name, filename, sizeof(fsh[*file].name));
        fsh[*file].zipFile = false;

        if (fs_debug->integer)
        {
            Com_Printf("FS_FOpenFileRead: %s (found in '%s%c%s')\n",
                    filename, dir->path, PATH_SEP, dir->gamedir);
        }

        fsh[*file].handleFiles.file.o = filep;
        return FS_fplength(filep);
    }

    *file = 0;
    return -1;
}

/*
===========
FS_FOpenFileRead

Finds the file in the search path.
Returns filesize and an open FILE pointer.
Used for streaming data out of either a
separate file or a ZIP file.
===========
*/
long FS_FOpenFileRead(const char *filename, fileHandle_t *file, bool uniqueFILE)
{
    searchpath_t *search;
    long len;

    if (!fs_searchpaths) Com_Error(ERR_FATAL, "Filesystem call made without initialization");

    bool isLocalConfig = !strcmp(filename, "autoexec.cfg") || !strcmp(filename, Q3CONFIG_CFG);
    for (search = fs_searchpaths; search; search = search->next)
    {
        // autoexec.cfg and q3config.cfg can only be loaded outside of pk3 files.
        if (isLocalConfig && search->pack) continue;

        len = FS_FOpenFileReadDir(filename, search, file, uniqueFILE, false);

        if (file == nullptr)
        {
            if (len > 0) return len;
        }
        else
        {
            if (len >= 0 && *file) return len;
        }
    }

#ifdef FS_MISSING
    if (missingFiles) fprintf(missingFiles, "%s\n", filename);
#endif

    if (file)
    {
        *file = 0;
        return -1;
    }
    else
    {
        // When file is nullptr, we're querying the existance of the file
        // If we've got here, it doesn't exist
        return 0;
    }
}

/*
=================
FS_FindVM

Find a suitable VM file in search path order.

In each searchpath try:
 - open DLL file if DLL loading enabled
 - open QVM file

Enable search for DLL by setting enableDll to FSVM_ENABLEDLL

write found DLL or QVM to "found" and return VMI_NATIVE if DLL, VMI_COMPILED if QVM
Return the searchpath in "startSearch".
=================
*/

int FS_FindVM(void **startSearch, char *found, int foundlen, const char *name, int enableDll)
{
    searchpath_t *search, *lastSearch;
    directory_t *dir;
    pack_t *pack;
    char dllName[MAX_OSPATH], qvmName[MAX_OSPATH];
    char *netpath;

    if (!fs_searchpaths) Com_Error(ERR_FATAL, "Filesystem call made without initialization");

    if (enableDll) Com_sprintf(dllName, sizeof(dllName), "%s" DLL_EXT, name);

    Com_sprintf(qvmName, sizeof(qvmName), "vm/%s.qvm", name);

    lastSearch = static_cast<searchpath_t *>(*startSearch);
    if (*startSearch == nullptr)
        search = fs_searchpaths;
    else
        search = lastSearch->next;

    while (search)
    {
        if (search->dir && (!fs_numServerPaks || !strcmp(name, "game")))
        {
            dir = search->dir;

            if (enableDll)
            {
                netpath = FS_BuildOSPath(dir->path, dir->gamedir, dllName);

                if (FS_FileInPathExists(netpath))
                {
                    Q_strncpyz(found, netpath, foundlen);
                    *startSearch = search;

                    return VMI_NATIVE;
                }
            }

            if (FS_FOpenFileReadDir(qvmName, search, nullptr, false, false) > 0)
            {
                *startSearch = search;
                return VMI_COMPILED;
            }
        }
        else if (search->pack)
        {
            pack = search->pack;

            if (lastSearch && lastSearch->pack)
            {
                // make sure we only try loading one VM file per game dir
                // i.e. if VM from pak7.pk3 fails we won't try one from pak6.pk3

                if (!FS_FilenameCompare(lastSearch->pack->pakPathname, pack->pakPathname))
                {
                    search = search->next;
                    continue;
                }
            }

            if (FS_FOpenFileReadDir(qvmName, search, nullptr, false, false) > 0)
            {
                *startSearch = search;

                return VMI_COMPILED;
            }
        }

        search = search->next;
    }

    return -1;
}

int FS_Read(void *buffer, int len, fileHandle_t f)
{
    int block, remaining;
    int read;
    byte *buf;
    int tries;

    if (!fs_searchpaths)
    {
        Com_Error(ERR_FATAL, "Filesystem call made without initialization");
    }

    if (!f)
    {
        return 0;
    }

    buf = (byte *)buffer;
    fs_readCount += len;

    if (fsh[f].zipFile == false)
    {
        remaining = len;
        tries = 0;
        while (remaining)
        {
            block = remaining;
            read = fread(buf, 1, block, fsh[f].handleFiles.file.o);
            if (read == 0)
            {
                // we might have been trying to read from a CD, which
                // sometimes returns a 0 read on windows
                if (!tries)
                {
                    tries = 1;
                }
                else
                {
                    return len - remaining;  // Com_Error (ERR_FATAL, "FS_Read: 0 bytes read");
                }
            }

            if (read == -1)
            {
                Com_Error(ERR_FATAL, "FS_Read: -1 bytes read");
            }

            remaining -= read;
            buf += read;
        }
        return len;
    }
    else
    {
        return unzReadCurrentFile(fsh[f].handleFiles.file.z, buffer, len);
    }
}

/*
=================
FS_Write

Properly handles partial writes
=================
*/
int FS_Write(const void *buffer, int len, fileHandle_t h)
{
    if (!fs_searchpaths)
    {
        Com_Error(ERR_FATAL, "Filesystem call made without initialization");
    }

    if (!h)
    {
        return 0;
    }

    FILE *f = FS_FileForHandle(h);
    byte *buf = (byte *)buffer;

    int remaining = len;
    int tries = 0;
    while (remaining)
    {
        int block = remaining;
        int written = fwrite(buf, 1, block, f);
        if (written == 0)
        {
            if (!tries)
            {
                tries = 1;
            }
            else
            {
                Com_Printf("FS_Write: 0 bytes written\n");
                return 0;
            }
        }

        if (written == -1)
        {
            Com_Printf("FS_Write: -1 bytes written\n");
            return 0;
        }

        remaining -= written;
        buf += written;
    }

    if (fsh[h].handleSync)
    {
        fflush(f);
    }
    return len;
}

void QDECL FS_Printf(fileHandle_t h, const char *fmt, ...)
{
    va_list argptr;
    char msg[MAXPRINTMSG];

    va_start(argptr, fmt);
    Q_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    FS_Write(msg, strlen(msg), h);
}

#define PK3_SEEK_BUFFER_SIZE 65536

/*
=================
FS_Seek

=================
*/
int FS_Seek(fileHandle_t f, long offset, enum FS_Origin origin)
{
    if (!fs_searchpaths)
    {
        Com_Error(ERR_FATAL, "Filesystem call made without initialization");
        return -1;
    }

    if (fsh[f].zipFile == true)
    {
        // FIXME: this is really, really crappy
        //(but better than what was here before)
        byte buffer[PK3_SEEK_BUFFER_SIZE];
        int remainder;
        int currentPosition = FS_FTell(f);

        // change negative offsets into FS_SEEK_SET
        if (offset < 0)
        {
            switch (origin)
            {
                case FS_SEEK_END:
                    remainder = fsh[f].zipFileLen + offset;
                    break;

                case FS_SEEK_CUR:
                    remainder = currentPosition + offset;
                    break;

                case FS_SEEK_SET:
                default:
                    remainder = 0;
                    break;
            }

            if (remainder < 0)
            {
                remainder = 0;
            }

            origin = FS_SEEK_SET;
        }
        else
        {
            if (origin == FS_SEEK_END)
            {
                remainder = fsh[f].zipFileLen - currentPosition + offset;
            }
            else
            {
                remainder = offset;
            }
        }

        switch (origin)
        {
            case FS_SEEK_SET:
                if (remainder == currentPosition)
                {
                    return offset;
                }
                unzSetOffset(fsh[f].handleFiles.file.z, fsh[f].zipFilePos);
                unzOpenCurrentFile(fsh[f].handleFiles.file.z);
            // fallthrough

            case FS_SEEK_END:  // fall through
            case FS_SEEK_CUR:
                while (remainder > PK3_SEEK_BUFFER_SIZE)
                {
                    FS_Read(buffer, PK3_SEEK_BUFFER_SIZE, f);
                    remainder -= PK3_SEEK_BUFFER_SIZE;
                }
                FS_Read(buffer, remainder, f);
                return offset;

            default:
                Com_Error(ERR_FATAL, "Bad origin in FS_Seek");
                return -1;
        }
    }
    else
    {
        FILE *file;
        file = FS_FileForHandle(f);
        int _origin;
        switch (origin)
        {
            case FS_SEEK_CUR:
                _origin = SEEK_CUR;
                break;
            case FS_SEEK_END:
                _origin = SEEK_END;
                break;
            case FS_SEEK_SET:
                _origin = SEEK_SET;
                break;
            default:
                Com_Error(ERR_FATAL, "Bad origin in FS_Seek");
                break;
        }

        return fseek(file, offset, _origin);
    }
}

/*
======================================================================================

CONVENIENCE FUNCTIONS FOR ENTIRE FILES

======================================================================================
*/

int FS_FileIsInPAK_A(bool alternate, const char *filename, int *pChecksum)
{
    if (!fs_searchpaths)
        Com_Error(ERR_FATAL, "Filesystem call made without initialization");

    if (!filename)
        Com_Error(ERR_FATAL, "FS_FOpenFileRead: nullptr 'filename' parameter passed");

    // qpaths are not supposed to have a leading slash
    if (filename[0] == '/' || filename[0] == '\\')
        filename++;

    // make absolutely sure that it can't back up the path.
    // The searchpaths do guarantee that something will always
    // be prepended, so we don't need to worry about "c:" or "//limbo"
    if (strstr(filename, "..") || strstr(filename, "::"))
        return -1;

    //
    // search through the path, one element at a time
    //
    for (auto search = fs_searchpaths; search; search = search->next)
    {
        if (!search->pack)
            continue;

        // disregard if it doesn't match one of the allowed pure pak files
        if (!search->pack->is_pure())
            continue;

        if ((alternate && search->pack->onlyPrimary) ||
            (!alternate && search->pack->onlyAlternate))
            continue;

        auto found = search->pack->find(filename);
        if (found)
        {
            if (pChecksum)
                *pChecksum = search->pack->pure_checksum;

            return 1;
        }
    }
    return -1;
}

int FS_FileIsInPAK(const char *filename, int *pChecksum)
{
    return FS_FileIsInPAK_A(false, filename, pChecksum);
}
/*
============
FS_ReadFileDir

Filename are relative to the quake search path
a null buffer will just return the file length without loading
If searchPath is non-nullptr search only in that specific search path
============
*/
long FS_ReadFileDir(const char *qpath, void *searchPath, bool unpure, void **buffer)
{
    fileHandle_t h;
    byte *buf;
    bool isConfig;
    long len;

    if (!fs_searchpaths)
    {
        Com_Error(ERR_FATAL, "Filesystem call made without initialization");
    }

    if (!qpath || !qpath[0])
    {
        Com_Error(ERR_FATAL, "FS_ReadFile with empty name");
    }

    buf = nullptr;  // quiet compiler warning

    // if this is a .cfg file and we are playing back a journal, read
    // it from the journal file
    if (strstr(qpath, ".cfg"))
    {
        isConfig = true;
        if (com_journal && com_journal->integer == 2)
        {
            Com_DPrintf("Loading %s from journal file.\n", qpath);

            int r = FS_Read(&len, sizeof(len), com_journalDataFile);
            if (r != sizeof(len))
            {
                if (buffer != nullptr) *buffer = nullptr;
                return -1;
            }

            // if the file didn't exist when the journal was created
            if (!len)
            {
                if (buffer == nullptr)
                {
                    return 1;  // hack for old journal files
                }
                *buffer = nullptr;
                return -1;
            }

            if (buffer == nullptr)
            {
                return len;
            }

            buf = static_cast<byte *>(Hunk_AllocateTempMemory(len + 1));
            *buffer = buf;

            r = FS_Read(buf, len, com_journalDataFile);
            if (r != len)
            {
                Com_Error(ERR_FATAL, "Read from journalDataFile failed");
            }

            fs_loadCount++;
            fs_loadStack++;

            // guarantee that it will have a trailing 0 for string operations
            buf[len] = 0;

            return len;
        }
    }
    else
    {
        isConfig = false;
    }

    searchpath_t *search = static_cast<searchpath_t *>(searchPath);
    if (search == nullptr)
    {
        // look for it in the filesystem or pack files
        len = FS_FOpenFileRead(qpath, &h, false);
    }
    else
    {
        // look for it in a specific search path only
        len = FS_FOpenFileReadDir(qpath, search, &h, false, unpure);
    }

    if (h == 0)
    {
        if (buffer)
        {
            *buffer = nullptr;
        }
        // if we are journalling and it is a config file, write a zero to the journal file
        if (isConfig && com_journal && com_journal->integer == 1)
        {
            Com_DPrintf("Writing zero for %s to journal file.\n", qpath);
            len = 0;
            FS_Write(&len, sizeof(len), com_journalDataFile);
            FS_Flush(com_journalDataFile);
        }
        return -1;
    }

    if (!buffer)
    {
        if (isConfig && com_journal && com_journal->integer == 1)
        {
            Com_DPrintf("Writing len for %s to journal file.\n", qpath);
            FS_Write(&len, sizeof(len), com_journalDataFile);
            FS_Flush(com_journalDataFile);
        }
        FS_FCloseFile(h);
        return len;
    }

    fs_loadCount++;
    fs_loadStack++;

    buf = static_cast<byte *>(Hunk_AllocateTempMemory(len + 1));
    *buffer = buf;

    FS_Read(buf, len, h);

    // guarantee that it will have a trailing 0 for string operations
    buf[len] = 0;
    FS_FCloseFile(h);

    // if we are journalling and it is a config file, write it to the journal file
    if (isConfig && com_journal && com_journal->integer == 1)
    {
        Com_DPrintf("Writing %s to journal file.\n", qpath);
        FS_Write(&len, sizeof(len), com_journalDataFile);
        FS_Write(buf, len, com_journalDataFile);
        FS_Flush(com_journalDataFile);
    }
    return len;
}

/*
============
FS_ReadFile

Filename are relative to the quake search path
a null buffer will just return the file length without loading
============
*/
long FS_ReadFile(const char *qpath, void **buffer)
{
    return FS_ReadFileDir(qpath, nullptr, false, buffer);
}
/*
=============
FS_FreeFile
=============
*/
void FS_FreeFile(void *buffer)
{
    if (!fs_searchpaths)
    {
        Com_Error(ERR_FATAL, "Filesystem call made without initialization");
    }
    if (!buffer)
    {
        Com_Error(ERR_FATAL, "FS_FreeFile( nullptr )");
    }

    fs_loadStack--;
    Hunk_FreeTempMemory(buffer);

    // if all of our temp files are free, clear all of our space
    if (fs_loadStack == 0)
    {
        Hunk_ClearTempMemory();
    }
}

/*
============
FS_WriteFile

Filename are relative to the quake search path
============
*/
void FS_WriteFile(const char *qpath, const void *buffer, int size)
{
    if (!fs_searchpaths)
    {
        Com_Error(ERR_FATAL, "Filesystem call made without initialization");
    }

    if (!qpath || !buffer)
    {
        Com_Error(ERR_FATAL, "FS_WriteFile: nullptr parameter");
    }

    fileHandle_t f = FS_FOpenFileWrite(qpath);
    if (!f)
    {
        Com_Printf("Failed to open %s\n", qpath);
        return;
    }

    FS_Write(buffer, size, f);
    FS_FCloseFile(f);
}

/*
==========================================================================

ZIP FILE LOADING

==========================================================================
*/

/*
=================
FS_LoadZipFile

Creates a new pack_t in the search chain for the contents of a zip file.
=================
*/
static pack_t *FS_LoadZipFile(const char *zipfile, const char *basename)
{
    int fs_numHeaderLongs = 0;
    unsigned long len = 0;
    char filename[MAX_ZPATH];

    auto z = unzOpen(zipfile);

    unz_global_info gi;
    int err = unzGetGlobalInfo(z, &gi);
    if (err) return nullptr;

    err = unzGoToFirstFile(z);
    if (err) return nullptr;

    for (uLong i = 0; i < gi.number_entry; i++)
    {
        unz_file_info fi;
        err = unzGetCurrentFileInfo(
            z, &fi, filename, sizeof(filename), nullptr, 0, nullptr, 0);

        if (err) break;

        len += strlen(filename) + 1;
        unzGoToNextFile(z);
    }

    fileInPack_t *buildBuffer =
        static_cast<fileInPack_t *>(Z_Malloc((gi.number_entry * sizeof(fileInPack_t)) + len));

    char *namePtr = ((char *)buildBuffer) + gi.number_entry * sizeof(fileInPack_t);

    int *fs_headerLongs = static_cast<int *>(Z_Malloc((gi.number_entry + 1) * sizeof(int)));

    fs_headerLongs[fs_numHeaderLongs] = LittleLong(fs_checksumFeed);
    fs_numHeaderLongs++;

    // get the hash table size from the number of files in the zip
    // because lots of custom pk3 files have less than 32 or 64 files
    uLong hashsiz;
    for (hashsiz = 1; hashsiz <= MAX_FILEHASH_SIZE; hashsiz <<= 1)
    {
        if (hashsiz > gi.number_entry) break;
    }

    pack_t *pack = static_cast<pack_t *>(Z_Malloc(sizeof(pack_t) + hashsiz * sizeof(fileInPack_t *)));

    pack->hashSize = hashsiz;
    pack->hashTable = (fileInPack_t **)(((char *)pack) + sizeof(pack_t));

    for (int i = 0; i < pack->hashSize; i++)
    {
        pack->hashTable[i] = nullptr;
    }

    Q_strncpyz(pack->pakFilename, zipfile, sizeof(pack->pakFilename));
    Q_strncpyz(pack->pakBasename, basename, sizeof(pack->pakBasename));

    // strip .pk3 if needed
    if ( strlen(pack->pakBasename) > 4 &&
        !Q_stricmp(pack->pakBasename + strlen(pack->pakBasename) - 4, ".pk3"))
    {
        pack->pakBasename[strlen(pack->pakBasename) - 4] = '\0';
    }

    pack->handle = z;
    pack->numfiles = gi.number_entry;
    unzGoToFirstFile(z);
    for (uLong i = 0; i < gi.number_entry; i++)
    {
        unz_file_info fi;
        err = unzGetCurrentFileInfo(
            z, &fi, filename, sizeof(filename), nullptr, 0, nullptr, 0);

        if (err) break;

        if (fi.uncompressed_size)
        {
            fs_headerLongs[fs_numHeaderLongs] = LittleLong(fi.crc);
            fs_numHeaderLongs++;
        }

        Q_strlwr(filename);
        long hash = FS_HashFileName(filename, pack->hashSize);

        buildBuffer[i].name = namePtr;
        strcpy(buildBuffer[i].name, filename);
        namePtr += strlen(filename) + 1;

        // store the file position in the zip
        buildBuffer[i].pos = unzGetOffset(z);
        buildBuffer[i].len = fi.uncompressed_size;
        buildBuffer[i].next = pack->hashTable[hash];

        pack->hashTable[hash] = &buildBuffer[i];
        unzGoToNextFile(z);
    }

    pack->checksum =
        Com_BlockChecksum(&fs_headerLongs[1], sizeof(*fs_headerLongs) * (fs_numHeaderLongs - 1));
    pack->pure_checksum =
        Com_BlockChecksum(fs_headerLongs, sizeof(*fs_headerLongs) * fs_numHeaderLongs);
    pack->checksum = LittleLong(pack->checksum);
    pack->pure_checksum = LittleLong(pack->pure_checksum);

    Z_Free(fs_headerLongs);

    pack->buildBuffer = buildBuffer;
    return pack;
}

/*
=================
FS_FreePak

Frees a pak structure and releases all associated resources
=================
*/

static void FS_FreePak(pack_t *thepak)
{
    unzClose(thepak->handle);
    Z_Free(thepak->buildBuffer);
    Z_Free(thepak);
}

/*
=================
FS_GetZipChecksum

Compares whether the given pak file matches a referenced checksum
=================
*/
bool FS_CompareZipChecksum(const char *zipfile)
{
    pack_t *thepak = FS_LoadZipFile(zipfile, "");

    if (!thepak) return false;

    int checksum = thepak->checksum;
    FS_FreePak(thepak);

    for (int i = 0; i < fs_numServerReferencedPaks; i++)
    {
        if (checksum == fs_serverReferencedPaks[i]) return true;
    }

    return false;
}

/*
=================================================================================

DIRECTORY SCANNING FUNCTIONS

=================================================================================
*/

#define MAX_FOUND_FILES 0x1000

static int FS_ReturnPath(const char *zname, char *zpath, int *depth)
{
    int newdep = 0;
    int len = 0;
    int at = 0;
    zpath[0] = 0;

    while (zname[at] != 0)
    {
        if (zname[at] == '/' || zname[at] == '\\')
        {
            len = at;
            newdep++;
        }
        at++;
    }
    strcpy(zpath, zname);
    zpath[len] = 0;
    *depth = newdep;

    return len;
}

/*
==================
FS_AddFileToList
==================
*/
static int FS_AddFileToList(char *name, char *list[MAX_FOUND_FILES], int nfiles)
{
    if (nfiles == MAX_FOUND_FILES - 1)
    {
        return nfiles;
    }

    for (int i = 0; i < nfiles; i++)
    {
        if (!Q_stricmp(name, list[i]))
        {
            return nfiles;  // allready in list
        }
    }
    list[nfiles] = CopyString(name);
    nfiles++;

    return nfiles;
}

/*
===============
FS_ListFilteredFiles

Returns a uniqued list of files that match the given criteria
from all search paths
===============
*/
char **FS_ListFilteredFiles(const char *path, const char *extension, const char *filter,
    int *numfiles, bool allowNonPureFilesOnDisk)
{
    int nfiles;
    char **listCopy;
    char *list[MAX_FOUND_FILES];
    searchpath_t *search;
    int i;
    int pathLength;
    int extensionLength;
    int length, pathDepth, temp;
    pack_t *pak;
    fileInPack_t *buildBuffer;
    char zpath[MAX_ZPATH];

    if (!fs_searchpaths)
    {
        Com_Error(ERR_FATAL, "Filesystem call made without initialization");
    }

    if (!path)
    {
        *numfiles = 0;
        return nullptr;
    }
    if (!extension)
    {
        extension = "";
    }

    pathLength = strlen(path);
    if (path[pathLength - 1] == '\\' || path[pathLength - 1] == '/')
    {
        pathLength--;
    }
    extensionLength = strlen(extension);
    nfiles = 0;
    FS_ReturnPath(path, zpath, &pathDepth);

    //
    // search through the path, one element at a time, adding to list
    //
    for (search = fs_searchpaths; search; search = search->next)
    {
        // is the element a pak file?
        if (search->pack)
        {
            // ZOID:  If we are pure, don't search for files on paks that
            // aren't on the pure list
            if (!search->pack->is_pure())
            {
                continue;
            }

            // look through all the pak file elements
            pak = search->pack;
            buildBuffer = pak->buildBuffer;
            for (i = 0; i < pak->numfiles; i++)
            {
                char *name;
                int zpathLen, depth;

                // check for directory match
                name = buildBuffer[i].name;
                //
                if (filter)
                {
                    // case insensitive
                    if (!Com_FilterPath(filter, name, false)) continue;
                    // unique the match
                    nfiles = FS_AddFileToList(name, list, nfiles);
                }
                else
                {
                    zpathLen = FS_ReturnPath(name, zpath, &depth);

                    if ((depth - pathDepth) > 2 || pathLength > zpathLen ||
                        Q_stricmpn(name, path, pathLength))
                    {
                        continue;
                    }

                    // check for extension match
                    length = strlen(name);
                    if (length < extensionLength)
                    {
                        continue;
                    }

                    if (Q_stricmp(name + length - extensionLength, extension))
                    {
                        continue;
                    }
                    // unique the match

                    temp = pathLength;
                    if (pathLength)
                    {
                        temp++;  // include the '/'
                    }
                    nfiles = FS_AddFileToList(name + temp, list, nfiles);
                }
            }
        }
        else if (search->dir)
        {  // scan for files in the filesystem

            // don't scan directories for files if we are pure or restricted
            if (fs_numServerPaks && !allowNonPureFilesOnDisk)
            {
                continue;
            }
            else
            {
                int numSysFiles;
                char *netpath = FS_BuildOSPath(search->dir->path, search->dir->gamedir, path);
                char **sysFiles = Sys_ListFiles(netpath, extension, filter, &numSysFiles, false);
                for (i = 0; i < numSysFiles; i++)
                {
                    // unique the match
                    char *name = sysFiles[i];
                    nfiles = FS_AddFileToList(name, list, nfiles);
                }
                Sys_FreeFileList(sysFiles);
            }
        }
    }

    // return a copy of the list
    *numfiles = nfiles;

    if (!nfiles)
    {
        return nullptr;
    }

    listCopy = static_cast<char **>(Z_Malloc((nfiles + 1) * sizeof(*listCopy)));
    for (i = 0; i < nfiles; i++)
    {
        listCopy[i] = list[i];
    }
    listCopy[i] = nullptr;

    return listCopy;
}

/*
=================
FS_ListFiles
=================
*/
char **FS_ListFiles(const char *path, const char *extension, int *numfiles)
{
    return FS_ListFilteredFiles(path, extension, nullptr, numfiles, false);
}

/*
=================
FS_FreeFileList
=================
*/
void FS_FreeFileList(char **list)
{
    if (!fs_searchpaths)
    {
        Com_Error(ERR_FATAL, "Filesystem call made without initialization");
    }

    if (!list)
    {
        return;
    }

    for (int i = 0; list[i]; i++)
    {
        Z_Free(list[i]);
    }

    Z_Free(list);
}

/*
================
FS_GetFileList
================
*/
int FS_GetFileList(const char *path, const char *extension, char *listbuf, int bufsize)
{
    int nFiles = 0;
    int nTotal = 0;
    *listbuf = 0;

    if (Q_stricmp(path, "$modlist") == 0)
    {
        return FS_GetModList(listbuf, bufsize);
    }

    char **pFiles = FS_ListFiles(path, extension, &nFiles);

    for (int i = 0; i < nFiles; i++)
    {
        int nLen = strlen(pFiles[i]) + 1;
        if (nTotal + nLen + 1 < bufsize)
        {
            strcpy(listbuf, pFiles[i]);
            listbuf += nLen;
            nTotal += nLen;
        }
        else
        {
            nFiles = i;
            break;
        }
    }

    FS_FreeFileList(pFiles);

    return nFiles;
}

/*
================
FS_GetFilteredFiles
================
*/
int FS_GetFilteredFiles(
    const char *path, const char *extension, const char *filter, char *listbuf, int bufsize)
{
    int nFiles = 0;
    int nTotal = 0;
    *listbuf = 0;

    char **pFiles = FS_ListFilteredFiles(path, extension, filter, &nFiles, false);

    for (int i = 0; i < nFiles; i++)
    {
        int nLen = strlen(pFiles[i]) + 1;
        if (nTotal + nLen + 1 < bufsize)
        {
            strcpy(listbuf, pFiles[i]);
            listbuf += nLen;
            nTotal += nLen;
        }
        else
        {
            nFiles = i;
            break;
        }
    }

    FS_FreeFileList(pFiles);

    return nFiles;
}

/*
=======================
Sys_ConcatenateFileLists

mkv: Naive implementation. Concatenates three lists into a
     new list, and frees the old lists from the heap.
bk001129 - from cvs1.17 (mkv)

FIXME TTimo those two should move to common.c next to Sys_ListFiles
=======================
 */
static unsigned int Sys_CountFileList(char **list)
{
    unsigned int i = 0;
    if (list)
    {
        while (*list)
        {
            list++;
            i++;
        }
    }
    return i;
}

static char **Sys_ConcatenateFileLists(char **list0, char **list1)
{
    char **cat, **dst;

    int totalLength = 0;
    totalLength += Sys_CountFileList(list0);
    totalLength += Sys_CountFileList(list1);

    /* Create new list. */
    dst = cat = static_cast<char **>(Z_Malloc((totalLength + 1) * sizeof(char *)));

    /* Copy over lists. */
    if (list0)
    {
        for (char **src = list0; *src; src++, dst++) *dst = *src;
    }

    if (list1)
    {
        for (char **src = list1; *src; src++, dst++) *dst = *src;
    }

    // Terminate the list
    *dst = nullptr;

    // Free our old lists.
    // NOTE: not freeing their content, it's been merged in dst and still being used
    if (list0) Z_Free(list0);
    if (list1) Z_Free(list1);

    return cat;
}

/*
================
FS_GetModList

Returns a list of mod directory names
A mod directory is a peer to baseq3 with a pk3 or pk3dir in it
================
*/
int	FS_GetModList( char *listbuf, int bufsize )
{
    char * start = listbuf;
    *listbuf = '\0';

    // paths to search for mods
    const char * const paths[] = {
        fs_basepath->string,
        fs_homepath->string
    };

    char **pFiles = nullptr;
    for (int i = 0; i < ARRAY_LEN(paths); i++)
    {
        int dummy;
        char **pFiles0 = Sys_ListFiles(paths[i], NULL, NULL, &dummy, true);
        pFiles = Sys_ConcatenateFileLists(pFiles, pFiles0);
    }

    int nMods = 0;
    int nTotal = 0;
    for (int i = 0; i < Sys_CountFileList(pFiles); i++)
    {
        const char* name = pFiles[i];

        if ( name[0] == '.' )
            continue;

        // In order to be a valid mod the directory must contain at least one
        // .pk3 or .pk3dir we didn't keep the information when we merged the
        // directory names, as to what OS Path it was found under so we will
        // try each of them here.
        int nPaks = 0;
        int nPakDirs = 0;
        for (int j = 0; j < ARRAY_LEN(paths); j++)
        {
            const char* path = FS_BuildOSPath(paths[j], name, "");
            int nDirs = 0; 

            char **pPaks = Sys_ListFiles(path, ".pk3", NULL, &nPaks, false);
            char **pDirs = Sys_ListFiles(path, "/", NULL, &nDirs, false);
            for (int k = 0; k < nDirs; k++)
            {
                // we only want to count directories ending with ".pk3dir"
                if (FS_IsExt(pDirs[k], ".pk3dir", strlen(pDirs[k])))
                    nPakDirs++;
            }

            // we only use Sys_ListFiles to check whether files are present
            Sys_FreeFileList(pPaks);
            Sys_FreeFileList(pDirs);

            if (nPaks > 0 || nPakDirs > 0)
                break;
        }

		if (nPaks > 0 || nPakDirs > 0)
        {
			size_t nLen = strlen(name) + 1;

			if (nTotal + nLen + 1 < bufsize)
            {
				strcpy(listbuf, name);
				listbuf += nLen;
				nTotal += nLen;
				nMods++;
			}
            else
            {
                Com_Printf(S_COLOR_RED "Warning: Too many mods!\n");
				break;
			}
		}
    }

    Sys_FreeFileList( pFiles );
    return nMods;
}

//============================================================================

/*
================
FS_Dir_f
================
*/
void FS_Dir_f(void)
{
    const char *path;
    const char *extension;

    if (Cmd_Argc() < 2 || Cmd_Argc() > 3)
    {
        Com_Printf("usage: dir <directory> [extension]\n");
        return;
    }

    if (Cmd_Argc() == 2)
    {
        path = Cmd_Argv(1);
        extension = "";
    }
    else
    {
        path = Cmd_Argv(1);
        extension = Cmd_Argv(2);
    }

    Com_Printf("Directory of %s %s\n", path, extension);
    Com_Printf("---------------\n");

    int ndirs;
    char **dirnames = FS_ListFiles(path, extension, &ndirs);

    for (int i = 0; i < ndirs; i++)
    {
        Com_Printf("%s\n", dirnames[i]);
    }
    FS_FreeFileList(dirnames);
}

/*
===========
FS_ConvertPath
===========
*/
void FS_ConvertPath(char *s)
{
    while (*s)
    {
        if (*s == '\\' || *s == ':')
        {
            *s = '/';
        }
        s++;
    }
}

/*
===========
FS_PathCmp

Ignore case and seprator char distinctions
===========
*/
int FS_PathCmp(const char *s1, const char *s2)
{
    int c1, c2;

    do
    {
        c1 = *s1++;
        c2 = *s2++;

        if (c1 >= 'a' && c1 <= 'z')
        {
            c1 -= ('a' - 'A');
        }
        if (c2 >= 'a' && c2 <= 'z')
        {
            c2 -= ('a' - 'A');
        }

        if (c1 == '\\' || c1 == ':')
        {
            c1 = '/';
        }
        if (c2 == '\\' || c2 == ':')
        {
            c2 = '/';
        }

        if (c1 < c2)
        {
            return -1;  // strings not equal
        }
        else if (c1 > c2)
        {
            return 1;
        }
    } while (c1);

    return 0;  // strings are equal
}

/*
================
FS_SortFileList
================
*/
void FS_SortFileList(char **filelist, int numfiles)
{
    char **sortedlist = static_cast<char **>(Z_Malloc((numfiles + 1) * sizeof(*sortedlist)));
    sortedlist[0] = nullptr;

    int numsortedfiles = 0;
    for (int i = 0; i < numfiles; i++)
    {
        int j;
        for (j = 0; j < numsortedfiles; j++)
        {
            if (FS_PathCmp(filelist[i], sortedlist[j]) < 0) break;
        }

        int k;
        for (k = numsortedfiles; k > j; k--)
        {
            sortedlist[k] = sortedlist[k - 1];
        }

        sortedlist[j] = filelist[i];
        numsortedfiles++;
    }
    ::memcpy(filelist, sortedlist, numfiles * sizeof(*filelist));
    Z_Free(sortedlist);
}

/*
================
FS_NewDir_f
================
*/
void FS_NewDir_f(void)
{
    if (Cmd_Argc() < 2)
    {
        Com_Printf("usage: fdir <filter>\n");
        Com_Printf("example: fdir *q3dm*.bsp\n");
        return;
    }

    Com_Printf("---------------\n");

    const char *filter = Cmd_Argv(1);

    int ndirs;
    char **dirnames = FS_ListFilteredFiles("", "", filter, &ndirs, false);

    FS_SortFileList(dirnames, ndirs);
    for (int i = 0; i < ndirs; i++)
    {
        FS_ConvertPath(dirnames[i]);
        Com_Printf("%s\n", dirnames[i]);
    }
    Com_Printf("%d files listed\n", ndirs);
    FS_FreeFileList(dirnames);
}

/*
============
FS_Path_f

============
*/
void FS_Path_f(void)
{
    Com_Printf("We are looking in the current search path:\n");

    for (auto s = fs_searchpaths; s; s = s->next)
    {
        if (s->pack)
        {
            Com_Printf("%s (%i files%s)\n",
                    s->pack->pakFilename,
                    s->pack->numfiles,
                    s->pack->onlyPrimary ? ", not for 1.1"
                    : s->pack->onlyAlternate ? ", only for 1.1" : "");

            if (s->pack->primaryVersion)
                Com_Printf("        (the 1.1 version of %s)\n",
                        s->pack->primaryVersion->pakFilename);

            if (fs_numServerPaks)
            {
                if (!s->pack->is_pure())
                {
                    Com_Printf("    not on the pure list\n");
                }
                else
                {
                    Com_Printf("    on the pure list\n");
                }
            }
        }
        else
        {
            Com_Printf("%s%c%s\n", s->dir->path, PATH_SEP, s->dir->gamedir);
        }
    }

    Com_Printf("\n");
    for (int i = 1; i < MAX_FILE_HANDLES; i++)
    {
        if (fsh[i].handleFiles.file.o)
        {
            Com_Printf("handle %i: %s\n", i, fsh[i].name);
        }
    }
}

/*
============
FS_TouchFile_f
============
*/
void FS_TouchFile_f(void)
{
    if (Cmd_Argc() != 2)
    {
        Com_Printf("Usage: touchFile <file>\n");
        return;
    }

    fileHandle_t f;
    FS_FOpenFileRead(Cmd_Argv(1), &f, false);
    if (f)
    {
        FS_FCloseFile(f);
    }
}

/*
============
FS_Which
============
*/

bool FS_Which(const char *filename, void *searchPath)
{
    searchpath_t *search = static_cast<searchpath_t *>(searchPath);

    if (FS_FOpenFileReadDir(filename, search, nullptr, false, false) > 0)
    {
        if (search->pack)
        {
            Com_Printf("File \"%s\" found in \"%s\"\n", filename, search->pack->pakFilename);
            return true;
        }
        else if (search->dir)
        {
            Com_Printf("File \"%s\" found at \"%s\"\n", filename, search->dir->fullpath);
            return true;
        }
    }

    return false;
}

/*
============
FS_Which_f
============
*/
void FS_Which_f(void)
{
    const char *filename = Cmd_Argv(1);

    if (!filename[0])
    {
        Com_Printf("Usage: which <file>\n");
        return;
    }

    // qpaths are not supposed to have a leading slash
    if (filename[0] == '/' || filename[0] == '\\') filename++;

    // just wants to see if file is there
    for (searchpath_t *search = fs_searchpaths; search; search = search->next)
        if (FS_Which(filename, search)) return;

    Com_Printf("File not found: \"%s\"\n", filename);
}

//===========================================================================

static int QDECL paksort(const void *a, const void *b)
{
    char *aa = *(char **)a;
    char *bb = *(char **)b;

    return FS_PathCmp(aa, bb);
}

/*
================
FS_AddGameDirectory

Sets fs_gamedir, adds the directory to the head of the path,
then loads the zip headers
================
*/
void FS_AddGameDirectory(const char *path, const char *dir)
{
    pack_t *pak;
    char curpath[MAX_OSPATH + 1];
    int numfiles;
    char **pakfiles;
    int pakfilesi;
    char **pakfilestmp;
    int numdirs;
    char **pakdirs;
    int pakdirsi;
    char **pakdirstmp;

    int lengths[10][2];

    // Unique
    for (searchpath_t *sp = fs_searchpaths; sp; sp = sp->next)
    {
        if (sp->dir && !Q_stricmp(sp->dir->path, path) && !Q_stricmp(sp->dir->gamedir, dir))
            return;  // we've already got this one
    }

    Q_strncpyz(fs_gamedir, dir, sizeof(fs_gamedir));

    // find all pak files in this directory
    Q_strncpyz(curpath, FS_BuildOSPath(path, dir, ""), sizeof(curpath));
    curpath[strlen(curpath) - 1] = '\0';  // strip the trailing slash

    // Get .pk3 files
    pakfiles = Sys_ListFiles(curpath, ".pk3", nullptr, &numfiles, false);

    qsort(pakfiles, numfiles, sizeof(char *), paksort);

    if (fs_numServerPaks)
    {
        numdirs = 0;
        pakdirs = nullptr;
    }
    else
    {
        // Get top level directories (we'll filter them later since the Sys_ListFiles filtering is
        // terrible)
        pakdirs = Sys_ListFiles(curpath, "/", nullptr, &numdirs, false);
        qsort(pakdirs, numdirs, sizeof(char *), paksort);
    }

    char prefixBuf[MAX_STRING_CHARS];
    Q_strncpyz(prefixBuf, Cvar_VariableString("fs_pk3PrefixPairs"), sizeof(prefixBuf));
    int numPairs = 0;

    char *p = prefixBuf;
    if (!p[0]) p = nullptr;

    const char *prefixes[10][2];
    while (p)
    {
        prefixes[numPairs][0] = p;
        p = strchr(p, '&');
        if (!p)
        {
            Com_Printf(S_COLOR_YELLOW "WARNING: fs_pk3PrefixPairs ends with an incomplete pair\n");
            break;
        }
        lengths[numPairs][0] = (int)(p - prefixes[numPairs][0]);
        *p++ = '\0';
        prefixes[numPairs][1] = p;
        p = strchr(p, '|');
        if (p)
        {
            lengths[numPairs][1] = (int)(p - prefixes[numPairs][1]);
            *p++ = '\0';
        }
        else
        {
            lengths[numPairs][1] = (int)strlen(prefixes[numPairs][1]);
        }
        if (lengths[numPairs][0] == 0 && lengths[numPairs][1] == 0)
        {
            Com_Printf(S_COLOR_YELLOW
                "WARNING: fs_pk3PrefixPairs contains a null-null pair, "
                "skipping this pair\n");
            continue;
        }
        if (lengths[numPairs][0] != 0 && lengths[numPairs][1] != 0 &&
            !Q_stricmpn(prefixes[numPairs][0], prefixes[numPairs][1],
                MIN(lengths[numPairs][0], lengths[numPairs][1])))
        {
            Com_Printf(S_COLOR_YELLOW
                "WARNING: in fs_pk3PrefixPairs, one of '%s' and '%s' is a real prefix "
                "of the other, skipping this pair\n",
                prefixes[numPairs][0], prefixes[numPairs][1]);

            continue;
        }
        ++numPairs;
    }
    searchpath_t *otherSearchpaths = fs_searchpaths;

    pakfilesi = 0;
    pakdirsi = 0;

    while ((pakfilesi < numfiles) || (pakdirsi < numdirs))
    {
        bool pakwhich;
        // Check if a pakfile or pakdir comes next
        if (pakfilesi >= numfiles)
        {
            pakwhich = false;  // We've used all the pak files, it must be a pak directory.
        }
        else if (pakdirsi >= numdirs)
        {
            pakwhich = true;  // We've used all the pak directories, it must be a pak file.
        }
        else
        {
            // Could be either, compare to see which name comes first
            // Need tmp variables for appropriate indirection for paksort()
            pakfilestmp = &pakfiles[pakfilesi];
            pakdirstmp = &pakdirs[pakdirsi];
            pakwhich = (paksort(pakfilestmp, pakdirstmp) < 0);
        }

        if (pakwhich)
        {
            // The next .pk3 file is before the next .pk3dir
            char *pakfile = FS_BuildOSPath(path, dir, pakfiles[pakfilesi]);
            if ((pak = FS_LoadZipFile(pakfile, pakfiles[pakfilesi])) == 0)
            {
                // This isn't a .pk3! Next!
                pakfilesi++;
                continue;
            }

            Q_strncpyz(pak->pakPathname, curpath, sizeof(pak->pakPathname));
            // store the game name for downloading
            Q_strncpyz(pak->pakGamename, dir, sizeof(pak->pakGamename));

            fs_packFiles += pak->numfiles;

            searchpath_t *search = static_cast<searchpath_t *>(Z_Malloc(sizeof(searchpath_t)));
            search->pack = pak;
            search->next = fs_searchpaths;
            fs_searchpaths = search;

            pak->onlyPrimary = false;
            pak->onlyAlternate = false;
            for (int i = 0; i < numPairs; ++i)
            {
                if (lengths[i][0] && !Q_stricmpn(pak->pakBasename, prefixes[i][0], lengths[i][0]))
                {
                    pak->onlyPrimary = true;
                    break;
                }
                else if (lengths[i][1] &&
                         !Q_stricmpn(pak->pakBasename, prefixes[i][1], lengths[i][1]))
                {
                    pak->onlyAlternate = true;
                    break;
                }
            }

            pak->primaryVersion = nullptr;
            pakfilesi++;
        }
        else
        {
            // The next .pk3dir is before the next .pk3 file
            // But wait, this could be any directory, we're filtering to only ending with ".pk3dir"
            // here.
            int len = strlen(pakdirs[pakdirsi]);
            if (!FS_IsExt(pakdirs[pakdirsi], ".pk3dir", len))
            {
                // This isn't a .pk3dir! Next!
                pakdirsi++;
                continue;
            }

            char *pakfile = FS_BuildOSPath(path, dir, pakdirs[pakdirsi]);

            // add the directory to the search path
            searchpath_t *search = static_cast<searchpath_t *>(Z_Malloc(sizeof(searchpath_t)));
            search->dir = static_cast<directory_t *>(Z_Malloc(sizeof(*search->dir)));

            Q_strncpyz(search->dir->path, curpath, sizeof(search->dir->path));  // c:\quake3\baseq3
            Q_strncpyz(search->dir->fullpath, pakfile,
                sizeof(search->dir->fullpath));  // c:\quake3\baseq3\mypak.pk3dir
            Q_strncpyz(search->dir->gamedir, pakdirs[pakdirsi],
                sizeof(search->dir->gamedir));  // mypak.pk3dir

            search->next = fs_searchpaths;
            fs_searchpaths = search;

            pakdirsi++;
        }
    }

    // done
    Sys_FreeFileList(pakfiles);
    Sys_FreeFileList(pakdirs);

    if (numPairs > 0)
    {
        int bnlengths[2];
        for (searchpath_t *search = fs_searchpaths; search != otherSearchpaths;
             search = search->next)
        {
            if (!(search->pack && search->pack->onlyPrimary))
            {
                continue;
            }

            bnlengths[0] = (int)strlen(search->pack->pakBasename);
            for (searchpath_t *srch = fs_searchpaths; srch != otherSearchpaths; srch = srch->next)
            {
                if (!(srch->pack && srch->pack->onlyAlternate))
                {
                    continue;
                }

                bnlengths[1] = (int)strlen(srch->pack->pakBasename);
                for (int i = 0; i < numPairs; ++i)
                {
                    if (lengths[i][0] && lengths[i][1] && bnlengths[0] >= lengths[i][0] &&
                        bnlengths[1] >= lengths[i][1] &&
                        !Q_stricmp(search->pack->pakBasename + lengths[i][0],
                            srch->pack->pakBasename + lengths[i][1]))
                    {
                        srch->pack->primaryVersion = search->pack;
                        break;
                    }
                }
            }
        }
    }

    //
    // add the directory to the search path
    //
    searchpath_t *search = static_cast<searchpath_t *>(Z_Malloc(sizeof(searchpath_t)));
    search->dir = static_cast<directory_t *>(Z_Malloc(sizeof(*search->dir)));

    Q_strncpyz(search->dir->path, path, sizeof(search->dir->path));
    Q_strncpyz(search->dir->fullpath, curpath, sizeof(search->dir->fullpath));
    Q_strncpyz(search->dir->gamedir, dir, sizeof(search->dir->gamedir));

    search->next = fs_searchpaths;
    fs_searchpaths = search;
}

/*
================
FS_CheckDirTraversal

Check whether the string contains stuff like "../" to prevent directory traversal bugs
and return true if it does.
================
*/

bool FS_CheckDirTraversal(const char *checkdir)
{
    if (strstr(checkdir, "../") || strstr(checkdir, "..\\")) return true;

    return false;
}

/*
================
FS_ComparePaks

----------------
dlstring == true

Returns a list of pak files that we should download from the server. They all get stored
in the current gamedir and an FS_Restart will be fired up after we download them all.

The string is the format:

@remotename@localname [repeat]

static int		fs_numServerReferencedPaks;
static int		fs_serverReferencedPaks[MAX_SEARCH_PATHS];
static char		*fs_serverReferencedPakNames[MAX_SEARCH_PATHS];

----------------
dlstring == false

we are not interested in a download string format, we want something human-readable
(this is used for diagnostics while connecting to a pure server)

================
*/
bool FS_ComparePaks(char *neededpaks, int len, bool dlstring)
{
    if (!fs_numServerReferencedPaks)
    {
        return false;  // Server didn't send any pack information along
    }

    char *origpos = neededpaks;
    *neededpaks = '\0';

    bool havepak = false;
    for (int i = 0; i < fs_numServerReferencedPaks; i++)
    {
        // Ok, see if we have this pak file
        havepak = false;

        // Make sure the server cannot make us write to non-quake3 directories.
        if (FS_CheckDirTraversal(fs_serverReferencedPakNames[i]))
        {
            Com_Printf("WARNING: Invalid download name %s\n", fs_serverReferencedPakNames[i]);
            continue;
        }

        for (searchpath_t *sp = fs_searchpaths; sp; sp = sp->next)
        {
            if (sp->pack && sp->pack->checksum == fs_serverReferencedPaks[i])
            {
                havepak = true;  // This is it!
                break;
            }
        }

        if (!havepak && fs_serverReferencedPakNames[i] && *fs_serverReferencedPakNames[i])
        {
            // Don't got it
            if (dlstring)
            {
                // We need this to make sure we won't hit the end of the buffer or the server could
                // overwrite non-pk3 files on clients by writing so much crap into neededpaks that
                // Q_strcat cuts off the .pk3 extension.

                origpos += strlen(origpos);

                // Remote name
                Q_strcat(neededpaks, len, "@");
                Q_strcat(neededpaks, len, fs_serverReferencedPakNames[i]);
                Q_strcat(neededpaks, len, ".pk3");

                // Local name
                Q_strcat(neededpaks, len, "@");
                // Do we have one with the same name?
                if (FS_SV_FileExists(va("%s.pk3", fs_serverReferencedPakNames[i])))
                {
                    char st[MAX_ZPATH];
                    // We already have one called this, we need to download it to another name
                    // Make something up with the checksum in it
                    Com_sprintf(st, sizeof(st), "%s.%08x.pk3", fs_serverReferencedPakNames[i],
                        fs_serverReferencedPaks[i]);
                    Q_strcat(neededpaks, len, st);
                }
                else
                {
                    Q_strcat(neededpaks, len, fs_serverReferencedPakNames[i]);
                    Q_strcat(neededpaks, len, ".pk3");
                }

                // Find out whether it might have overflowed the buffer and don't add this file to
                // the
                // list if that is the case.
                if (strlen(origpos) + (origpos - neededpaks) >= (len - 1))
                {
                    *origpos = '\0';
                    break;
                }
            }
            else
            {
                Q_strcat(neededpaks, len, fs_serverReferencedPakNames[i]);
                Q_strcat(neededpaks, len, ".pk3");
                // Do we have one with the same name?
                if (FS_SV_FileExists(va("%s.pk3", fs_serverReferencedPakNames[i])))
                {
                    Q_strcat(neededpaks, len, " (local file exists with wrong checksum)");
                }
                Q_strcat(neededpaks, len, "\n");
            }
        }
    }

    if (*neededpaks)
    {
        return true;
    }

    return false;  // We have them all
}

/*
================
FS_Shutdown

Frees all resources.
================
*/

void FS_Shutdown(bool closemfp)
{
    for (int i = 0; i < MAX_FILE_HANDLES; i++)
    {
        if (fsh[i].fileSize) FS_FCloseFile(i);
    }

    searchpath_t *next;
    // free everything
    for (auto p = fs_searchpaths; p; p = next)
    {
        next = p->next;
        if (p->pack) FS_FreePak(p->pack);
        if (p->dir) Z_Free(p->dir);
        Z_Free(p);
    }

    // Any FS_ calls will now be an error until reinitialized
    fs_searchpaths = nullptr;

    Cmd_RemoveCommand("path");
    Cmd_RemoveCommand("dir");
    Cmd_RemoveCommand("fdir");
    Cmd_RemoveCommand("touchFile");
    Cmd_RemoveCommand("which");

#ifdef FS_MISSING
    if (closemfp)
    {
        fclose(missingFiles);
    }
#endif
}

/*
================
FS_ReorderPurePaks
NOTE TTimo: the reordering that happens here is not reflected in the cvars (\cvarlist *pak*)
  this can lead to misleading situations, see
https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=540
================
*/
static void FS_ReorderPurePaks(void)
{
    // do this before fs_numServerPaks check?
    fs_reordered = false;

    // only relevant when connected to pure server
    if (!fs_numServerPaks) return;

    // we insert in order at the beginning of the list
    auto p_insert_index = &fs_searchpaths;
    for (int i = 0; i < fs_numServerPaks; i++)
    {
        // track the pointer-to-current-item
        auto p_previous = p_insert_index;
        for (auto s = *p_insert_index; s; s = s->next)
        {
            // the part of the list before p_insert_index has been sorted already
            if (s->pack && fs_serverPaks[i] == s->pack->checksum)
            {
                fs_reordered = true;

                // move this element to the insert list
                *p_previous = s->next;
                s->next = *p_insert_index;
                *p_insert_index = s;

                // increment insert list
                p_insert_index = &s->next;

                // iterate to next server pack
                break;
            }
            p_previous = &s->next;
        }
    }
}

/*
================
FS_Startup
================
*/
static void FS_Startup(const char *gameName)
{
    Com_Printf("----- FS_Startup -----\n");
    fs_packFiles = 0;

    fs_debug = Cvar_Get("fs_debug", "0", 0);
    fs_basepath = Cvar_Get("fs_basepath", Sys_DefaultInstallPath(), CVAR_INIT | CVAR_PROTECTED);
    fs_basegame = Cvar_Get("fs_basegame", BASEGAME, CVAR_INIT);

    const char *homePath = Sys_DefaultHomePath();
    if (!homePath || !homePath[0])
    {
        homePath = fs_basepath->string;
    }

    fs_homepath = Cvar_Get("fs_homepath", homePath, CVAR_INIT | CVAR_PROTECTED);
    fs_gamedirvar = Cvar_Get("fs_game", BASEGAME, CVAR_INIT | CVAR_SYSTEMINFO);

#ifdef DEDICATED
    // add search path elements in reverse priority order
    if (fs_basepath->string[0])
        FS_AddGameDirectory(fs_basepath->string, gameName);

    // NOTE: same filtering below for mods and basegame
    if (fs_homepath->string[0] && Q_stricmp(fs_homepath->string,fs_basepath->string))
    {
        FS_CreatePath(fs_homepath->string);
        FS_AddGameDirectory(fs_homepath->string, gameName);
    }

    // check for additional base game so mods can be based upon other mods
    if ( fs_basegame->string[0] && Q_stricmp( fs_basegame->string, gameName ) )
    {
        if (fs_basepath->string[0])
            FS_AddGameDirectory(fs_basepath->string, fs_basegame->string);
        if (fs_homepath->string[0] && Q_stricmp(fs_homepath->string,fs_basepath->string))
            FS_AddGameDirectory(fs_homepath->string, fs_basegame->string);
    }

    // check for additional game folder for mods
    if ( fs_gamedirvar->string[0] && Q_stricmp( fs_gamedirvar->string, gameName ) )
    {
        if (fs_basepath->string[0])
            FS_AddGameDirectory(fs_basepath->string, fs_gamedirvar->string);
        if (fs_homepath->string[0] && Q_stricmp(fs_homepath->string,fs_basepath->string))
            FS_AddGameDirectory(fs_homepath->string, fs_gamedirvar->string);
    }

#else

    // add search path elements in reverse priority order
    if (fs_basepath->string[0])
    {
        FS_AddGameDirectory(fs_basepath->string, "base");
    }

#ifdef __APPLE__
    // Make MacOSX also include the base path included with the .app bundle
    fs_apppath = Cvar_Get("fs_apppath", Sys_DefaultAppPath(), CVAR_INIT | CVAR_PROTECTED);
    if (fs_apppath->string[0])
    {
        FS_AddGameDirectory(fs_apppath->string, "base");
    }
#endif

    // NOTE: same filtering below for mods and basegame
    if (fs_homepath->string[0] && Q_stricmp(fs_homepath->string, fs_basepath->string))
    {
        FS_CreatePath(fs_homepath->string);
        FS_AddGameDirectory(fs_homepath->string, "base");
    }

    // check for additional base game so mods can be based upon other mods
    if (fs_basegame->string[0] && Q_stricmp(fs_basegame->string, gameName))
    {
        if (fs_basepath->string[0])
            FS_AddGameDirectory(fs_basepath->string, fs_basegame->string);
        if (fs_homepath->string[0] && Q_stricmp(fs_homepath->string, fs_basepath->string))
            FS_AddGameDirectory(fs_homepath->string, fs_basegame->string);
    }

    // check for additional game folder for mods
    if (fs_gamedirvar->string[0] && Q_stricmp(fs_gamedirvar->string, gameName))
    {
        if (fs_basepath->string[0])
            FS_AddGameDirectory(fs_basepath->string, fs_gamedirvar->string);
        if (fs_homepath->string[0] && Q_stricmp(fs_homepath->string, fs_basepath->string))
            FS_AddGameDirectory(fs_homepath->string, fs_gamedirvar->string);
    }

    // NOTE: same filtering below for mods and basegame
    if (fs_homepath->string[0] && Q_stricmp(fs_homepath->string, fs_basepath->string))
    {
        FS_CreatePath(fs_homepath->string);
        FS_AddGameDirectory(fs_homepath->string, gameName);
    }

    // add search path elements in reverse priority order
    if (fs_basepath->string[0])
    {
        FS_AddGameDirectory(fs_basepath->string, gameName);
    }

#ifdef __APPLE__
    // Make MacOSX also include the base path included with the .app bundle
    fs_apppath = Cvar_Get("fs_apppath", Sys_DefaultAppPath(), CVAR_INIT | CVAR_PROTECTED);
    if (fs_apppath->string[0])
    {
        FS_AddGameDirectory(fs_apppath->string, gameName);
    }
#endif
#endif

    // add our commands
    Cmd_AddCommand("path", FS_Path_f);
    Cmd_AddCommand("dir", FS_Dir_f);
    Cmd_AddCommand("fdir", FS_NewDir_f);
    Cmd_AddCommand("touchFile", FS_TouchFile_f);
    Cmd_AddCommand("which", FS_Which_f);

    // reorder the pure pk3 files according to server order
    FS_ReorderPurePaks();

    // print the current search paths
    FS_Path_f();

    // We just loaded, it's not modified
    fs_gamedirvar->modified = false;

    Com_Printf("----------------------\n");

#ifdef FS_MISSING
    if (missingFiles == nullptr)
    {
        missingFiles = Sys_FOpen("\\missing.txt", "ab");
    }
#endif

    Com_Printf("%d files in pk3 files\n", fs_packFiles);
}

/*
=====================
FS_LoadedPakChecksums

Returns a space separated string containing the checksums of all loaded pk3 files.
Servers with sv_pure set will get this string and pass it to clients.
=====================
*/
const char *FS_LoadedPakChecksums(bool alternate)
{
    static char info[BIG_INFO_STRING];
    info[0] = 0;

    for (auto search = fs_searchpaths; search; search = search->next)
    {
        // is the element a pak file?
        if (!search->pack) continue;
        if ((alternate && search->pack->onlyPrimary) || (!alternate && search->pack->onlyAlternate))
            continue;
        Q_strcat(info, sizeof(info), va("%i ", search->pack->checksum));
    }

    return info;
}

/*
=====================
FS_LoadedPakNames

Returns a space separated string containing the names of all loaded pk3 files.
Servers with sv_pure set will get this string and pass it to clients.
=====================
*/
const char *FS_LoadedPakNames(bool alternate)
{
    static char info[BIG_INFO_STRING];
    info[0] = 0;

    for (auto search = fs_searchpaths; search; search = search->next)
    {
        // is the element a pak file?
        if (!search->pack) continue;
        if ((alternate && search->pack->onlyPrimary) || (!alternate && search->pack->onlyAlternate))
            continue;
        if (info[0]) Q_strcat(info, sizeof(info), " ");
        Q_strcat(info, sizeof(info), search->pack->pakBasename);
    }

    return info;
}

/*
=====================
FS_LoadedPakPureChecksums

Returns a space separated string containing the pure checksums of all loaded pk3 files.
Servers with sv_pure use these checksums to compare with the checksums the clients send
back to the server.
=====================
*/
const char *FS_LoadedPakPureChecksums(bool alternate)
{
    static char info[BIG_INFO_STRING];
    info[0] = 0;

    for (auto search = fs_searchpaths; search; search = search->next)
    {
        // is the element a pak file?
        if (!search->pack) continue;
        if ((alternate && search->pack->onlyPrimary) || (!alternate && search->pack->onlyAlternate))
            continue;
        Q_strcat(info, sizeof(info), va("%i ", search->pack->pure_checksum));
    }

    return info;
}

/*
=====================
FS_ReferencedPakChecksums

Returns a space separated string containing the checksums of all referenced pk3 files.
The server will send this to the clients so they can check which files should be auto-downloaded.
=====================
*/
const char *FS_ReferencedPakChecksums(bool alternate)
{
    static char info[BIG_INFO_STRING];
    info[0] = 0;

    for (auto search = fs_searchpaths; search; search = search->next)
    {
        // is the element a pak file?
        if (search->pack)
        {
            if ((alternate and search->pack->onlyPrimary) or
                (!alternate and search->pack->onlyAlternate))
                continue;

            if (search->pack->referenced or
                (search->pack->primaryVersion and search->pack->primaryVersion->referenced) or
                (*fs_gamedirvar->string and Q_stricmp(fs_gamedirvar->string, BASEGAME) and
                    Q_stricmp(search->pack->pakGamename, fs_gamedirvar->string) == 0))
            {
                Q_strcat(info, sizeof(info), va("%i ", search->pack->checksum));
            }
        }
    }

    return info;
}

/*
=====================
FS_ReferencedPakPureChecksums

Returns a space separated string containing the pure checksums of all referenced pk3 files.
Servers with sv_pure set will get this string back from clients for pure validation

The string has a specific order, "cgame ui @ ref1 ref2 ref3 ..."
=====================
*/
const char *FS_ReferencedPakPureChecksums(void)
{
    static char info[BIG_INFO_STRING];
    info[0] = 0;

    int checksum = fs_checksumFeed;
    int numPaks = 0;
    for (int nFlags = FS_CGAME_REF; nFlags; nFlags = nFlags >> 1)
    {
        if (nFlags & FS_GENERAL_REF)
        {
            // add a delimter between must haves and general refs
            // Q_strcat(info, sizeof(info), "@ ");
            info[strlen(info) + 1] = '\0';
            info[strlen(info) + 2] = '\0';
            info[strlen(info)] = '@';
            info[strlen(info)] = ' ';
        }
        for (auto search = fs_searchpaths; search; search = search->next)
        {
            // is the element a pak file and has it been referenced based on flag?
            if (search->pack && (search->pack->referenced & nFlags))
            {
                Q_strcat(info, sizeof(info), va("%i ", search->pack->pure_checksum));
                if (nFlags & (FS_CGAME_REF | FS_UI_REF)) break;

                checksum ^= search->pack->pure_checksum;
                numPaks++;
            }
        }
    }
    // last checksum is the encoded number of referenced pk3s
    checksum ^= numPaks;
    Q_strcat(info, sizeof(info), va("%i ", checksum));

    return info;
}

/*
=====================
FS_ReferencedPakNames

Returns a space separated string containing the names of all referenced pk3 files.
The server will send this to the clients so they can check which files should be auto-downloaded.
=====================
*/
const char *FS_ReferencedPakNames(bool alternate)
{
    static char info[BIG_INFO_STRING];
    info[0] = 0;

    // we want to return ALL pk3's from the fs_game path
    // and referenced one's from base
    for (auto search = fs_searchpaths; search; search = search->next)
    {
        // is the element a pak file?
        if (search->pack)
        {
            if ((alternate && search->pack->onlyPrimary) ||
                (!alternate && search->pack->onlyAlternate))
                continue;

            if (search->pack->referenced ||
                (search->pack->primaryVersion && search->pack->primaryVersion->referenced) ||
                (fs_gamedirvar->string[0] && Q_stricmp(fs_gamedirvar->string, BASEGAME) &&
                    !Q_stricmp(search->pack->pakGamename, fs_gamedirvar->string)))
            {
                if (*info) Q_strcat(info, sizeof(info), " ");

                Q_strcat(info, sizeof(info), search->pack->pakGamename);
                Q_strcat(info, sizeof(info), "/");
                Q_strcat(info, sizeof(info), search->pack->pakBasename);
            }
        }
    }

    return info;
}

/*
=====================
FS_ClearPakReferences
=====================
*/
void FS_ClearPakReferences(int flags)
{
    if (!flags) flags = -1;

    for (auto search = fs_searchpaths; search; search = search->next)
    {
        // is the element a pak file and has it been referenced?
        if (search->pack) search->pack->referenced &= ~flags;
    }
}

/*
=====================
FS_PureServerSetLoadedPaks

If the string is empty, all data sources will be allowed.
If not empty, only pk3 files that match one of the space
separated checksums will be checked for files, with the
exception of .cfg and .dat files.
=====================
*/
void FS_PureServerSetLoadedPaks(const char *pakSums, const char *pakNames)
{
    Cmd_TokenizeString(pakSums);

    int c = Cmd_Argc();
    if (c > MAX_SEARCH_PATHS) c = MAX_SEARCH_PATHS;

    fs_numServerPaks = c;

    for (int i = 0; i < c; i++) fs_serverPaks[i] = atoi(Cmd_Argv(i));

    if (fs_numServerPaks)
    {
        Com_DPrintf("Connected to a pure server.\n");
    }
    else if (fs_reordered)
    {
        // force a restart to make sure the search order will be correct
        Com_DPrintf("FS search reorder is required\n");
        FS_Restart(fs_checksumFeed);
        return;
    }

    for (int i = 0; i < c; i++)
    {
        if (fs_serverPakNames[i]) Z_Free(fs_serverPakNames[i]);
        fs_serverPakNames[i] = nullptr;
    }

    if (pakNames && pakNames[0])
    {
        Cmd_TokenizeString(pakNames);

        int d = Cmd_Argc();
        if (d > MAX_SEARCH_PATHS) d = MAX_SEARCH_PATHS;

        for (int i = 0; i < d; i++) fs_serverPakNames[i] = CopyString(Cmd_Argv(i));
    }
}

/*
=====================
FS_PureServerSetReferencedPaks

The checksums and names of the pk3 files referenced at the server
are sent to the client and stored here. The client will use these
checksums to see if any pk3 files need to be auto-downloaded.
=====================
*/
void FS_PureServerSetReferencedPaks(const char *pakSums, const char *pakNames)
{
    Cmd_TokenizeString(pakSums);

    unsigned c = Cmd_Argc();
    if (c > MAX_SEARCH_PATHS) c = MAX_SEARCH_PATHS;

    for (unsigned i = 0; i < c; i++) fs_serverReferencedPaks[i] = atoi(Cmd_Argv(i));

    for (unsigned i = 0; i < ARRAY_LEN(fs_serverReferencedPakNames); i++)
    {
        if (fs_serverReferencedPakNames[i]) Z_Free(fs_serverReferencedPakNames[i]);
        fs_serverReferencedPakNames[i] = nullptr;
    }

    unsigned d = 0;
    if (pakNames && *pakNames)
    {
        Cmd_TokenizeString(pakNames);

        d = Cmd_Argc();
        if (d > c) d = c;

        for (unsigned i = 0; i < d; i++) fs_serverReferencedPakNames[i] = CopyString(Cmd_Argv(i));
    }

    // ensure that there are as many checksums as there are pak names.
    if (d < c) c = d;

    fs_numServerReferencedPaks = c;
}

/*
================
FS_InitFilesystem

Called only at inital startup, not when the filesystem
is resetting due to a game change
================
*/
void FS_InitFilesystem(void)
{
    // allow command line parms to override our defaults
    // we have to specially handle this, because normal command
    // line variable sets don't happen until after the filesystem
    // has already been initialized
    Com_StartupVariable("fs_basepath");
    Com_StartupVariable("fs_homepath");
    Com_StartupVariable("fs_game");
    Com_StartupVariable("fs_pk3PrefixPairs");

    if (!FS_FilenameCompare(Cvar_VariableString("fs_game"), BASEGAME)) Cvar_Set("fs_game", "");

    // try to start up normally
    FS_Startup(BASEGAME);

    // if we can't find default.cfg, assume that the paths are
    // busted and error out now, rather than getting an unreadable
    // graphics screen when the font fails to load
    if (FS_ReadFile("default.cfg", nullptr) <= 0)
    {
        Com_Error(ERR_FATAL, "Couldn't load default.cfg");
    }

    Q_strncpyz(lastValidBase, fs_basegame->string, sizeof(lastValidBase));
    Q_strncpyz(lastValidGame, fs_gamedirvar->string, sizeof(lastValidGame));
}

/*
================
FS_Restart
================
*/
void FS_Restart(int checksumFeed)
{
    // free anything we currently have loaded
    FS_Shutdown(false);

    // set the checksum feed
    fs_checksumFeed = checksumFeed;

    // clear pak references
    FS_ClearPakReferences(0);

    // try to start up normally
    FS_Startup(BASEGAME);

    // if we can't find default.cfg, assume that the paths are
    // busted and error out now, rather than getting an unreadable
    // graphics screen when the font fails to load
    if (FS_ReadFile("default.cfg", nullptr) <= 0)
    {
        // this might happen when connecting to a pure server not using BASEGAME/pak0.pk3
        // (for instance a TA demo server)
        if (lastValidBase[0])
        {
            FS_PureServerSetLoadedPaks("", "");
            Cvar_Set("fs_basegame", lastValidBase);
            Cvar_Set("fs_game", lastValidGame);
            lastValidBase[0] = lastValidGame[0] = '\0';
            FS_Restart(checksumFeed);
            Com_Error(ERR_DROP, "Invalid game folder");
            return;
        }
        Com_Error(ERR_FATAL, "Couldn't load default.cfg");
    }

    if (Q_stricmp(fs_gamedirvar->string, lastValidGame))
    {
        // skip the autogen.cfg if "safe" is on the command line
        if (!Com_SafeMode())
        {
            Cbuf_AddText("exec " Q3CONFIG_CFG "\n");
        }
    }

    Q_strncpyz(lastValidBase, fs_basegame->string, sizeof(lastValidBase));
    Q_strncpyz(lastValidGame, fs_gamedirvar->string, sizeof(lastValidGame));
}

/*
=================
FS_ConditionalRestart

Restart if necessary
Return true if restarting due to game directory changed, false otherwise
=================
*/
bool FS_ConditionalRestart(int checksumFeed, bool disconnect)
{
    if (fs_gamedirvar->modified)
    {
        if (FS_FilenameCompare(lastValidGame, fs_gamedirvar->string) &&
            (*lastValidGame || FS_FilenameCompare(fs_gamedirvar->string, BASEGAME)) &&
            (*fs_gamedirvar->string || FS_FilenameCompare(lastValidGame, BASEGAME)))
        {
            Com_GameRestart(checksumFeed, disconnect);
            return true;
        }
        fs_gamedirvar->modified = false;
    }

    if (checksumFeed != fs_checksumFeed)
        FS_Restart(checksumFeed);

    else if (fs_numServerPaks && !fs_reordered)
        FS_ReorderPurePaks();

    return false;
}

/*
========================================================================================

Handle based file calls for virtual machines

========================================================================================
*/

int FS_FOpenFileByMode(const char *qpath, fileHandle_t *f, enum FS_Mode mode)
{
    int r;
    bool sync = false;

    switch (mode)
    {
        case FS_READ:
            r = FS_FOpenFileRead(qpath, f, true);
            break;

        case FS_WRITE:
            *f = FS_FOpenFileWrite(qpath);
            r = 0;
            if (*f == 0) r = -1;
            break;

        case FS_APPEND_SYNC:
            sync = true;
        // fall through

        case FS_APPEND:
            *f = FS_FOpenFileAppend(qpath);
            r = 0;
            if (*f == 0) r = -1;
            break;

        default:
            Com_Error(ERR_FATAL, "FS_FOpenFileByMode: bad mode");
            return -1;
    }

    if (!f) return r;

    if (*f)
    {
        fsh[*f].fileSize = r;
    }
    fsh[*f].handleSync = sync;

    return r;
}

int FS_FTell(fileHandle_t f)
{
    if (fsh[f].zipFile == true) return unztell(fsh[f].handleFiles.file.z);
    return ftell(fsh[f].handleFiles.file.o);
}

void FS_Flush(fileHandle_t f)
{
    fflush(fsh[f].handleFiles.file.o);
}

void FS_FilenameCompletion(const char *dir, const char *ext, bool stripExt,
    void (*callback)(const char *s), bool allowNonPureFilesOnDisk)
{
    int nfiles;
    char filename[MAX_STRING_CHARS];
    char **filenames = FS_ListFilteredFiles(dir, ext, nullptr, &nfiles, allowNonPureFilesOnDisk);

    FS_SortFileList(filenames, nfiles);

    for (int i = 0; i < nfiles; i++)
    {
        FS_ConvertPath(filenames[i]);
        Q_strncpyz(filename, filenames[i], MAX_STRING_CHARS);
        if (stripExt) COM_StripExtension(filename, filename, sizeof(filename));
        callback(filename);
    }
    FS_FreeFileList(filenames);
}

const char *FS_GetCurrentGameDir(void)
{
    if (fs_gamedirvar->string[0]) return fs_gamedirvar->string;
    return BASEGAME;
}
