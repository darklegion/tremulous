#include "cl_updates.h"
#include "cl_rest.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <array>

#include <libgen.h>
#include <unistd.h>

#include "restclient/restclient.h"
#include "restclient/connection.h"

#include "rapidjson.h"

#include "lua.hpp"
//#include "../sol/sol.h"

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "../qcommon/unzip.h"

#include "../qcommon/q_platform.h"

using namespace std;

std::vector<std::string> release_package;
static std::string granger_exe;
static std::string granger_main_lua;
static int nextCheckTime = 0;

struct UpdateManager {

    static void refresh();
    static void download();
    static void extract(std::string extract_path, std::string path);
    static void execute();

private:
    static constexpr auto url = "https://api.github.com/repos/wtfbbqhax/tremulous/releases";
    static constexpr auto package_name = RELEASE_PACKAGE_NAME;
    static constexpr auto granger_binary_name = "granger" EXE_EXT;
};

#define AU_ACT_NIL 0
#define AU_ACT_GET 1
#define AU_ACT_RUN 2

void UpdateManager::refresh()
{
	auto currentTime = Sys_Milliseconds();

    if ( nextCheckTime > currentTime )
        return;

    nextCheckTime = currentTime + 10000;

    Cvar_SetValue("ui_autoupdate_action", 0);
    Cvar_Set("cl_latestDownload", "");
    Cvar_Set("cl_latestRelease", "");

    RestClient::Response r = RestClient::get(url);
    if ( r.code != 200 )
    {
        std::string msg { va("Server did not return OK status code: %d", r.code) };
        
        Cvar_Set("cl_latestRelease", 
                S_COLOR_RED "ERROR:\n"
                S_COLOR_WHITE "Server did not return OK status code");

        return;
    }

    rapidjson::Document d;
    d.Parse( r.body.c_str() );

    rapidjson::Value &release = d[0];
    std::string txt;
    
    txt += release["tag_name"].GetString();
    if (release["prerelease"].IsTrue())
        txt += S_COLOR_YELLOW " (Prerelease)";

    txt += '\n';
    txt += S_COLOR_CYAN "Released:" S_COLOR_WHITE;
    txt += release["published_at"].GetString();

    txt += '\n';
    txt += S_COLOR_RED "Release Notes:\n" S_COLOR_WHITE;

    if (!release["body"].IsNull())
    {
        txt += '\n';
        txt += release["body"].GetString();
    }

    rapidjson::Value &assets = release["assets"];
    for ( rapidjson::SizeType i = 0; i < assets.Size(); ++i )
    {
        auto &a = assets[i];
        std::string name = a["name"].GetString();

        if ( name == package_name )
        {
            std::string dl = a["browser_download_url"].GetString();
            Cvar_Set("cl_latestDownload", dl.c_str());
            Cvar_Set("cl_latestPackage", package_name);
            Cvar_Set("cl_latestRelease", txt.c_str());
            Cvar_SetValue("ui_autoupdate_action", AU_ACT_GET);
        }
    }
}

void UpdateManager::extract(std::string extract_path, std::string path)
{
    // Extract the release package
    auto z = unzOpen(path.c_str());
    assert( z != nullptr );

    unz_global_info zi;
    unzGetGlobalInfo(z, &zi);

    // Iterate through all files in the package
    unzGoToFirstFile(z);
    for (int i = 0; i < zi.number_entry; ++i)
    {
        unz_file_info fi;
        char filename[256] = ""; // 256 == MAX_ZPATH
        int err;
        
        err = unzGetCurrentFileInfo(z, &fi, filename, sizeof(filename),
                nullptr, 0, nullptr, 0);
        assert( err == UNZ_OK ); // != OK means corrupt archive

        std::string fn = { filename };

        // FIXME stop doing dumb string stuff
        std::string fullpath = extract_path;
        fullpath += PATH_SEP;
        fullpath += fn;
        release_package.emplace_back(fn);

        // Bad assumption of a directory?
        if ( !fi.compressed_size && !fi.uncompressed_size )
        {
            Com_DPrintf(S_COLOR_CYAN"ARCHIVED DIR: "
                        S_COLOR_WHITE"%s\n",
                        fn.c_str());

            MakeDir(extract_path, fn);

            unzGoToNextFile(z);
            continue;
        }

        // Must be a file
        Com_DPrintf(S_COLOR_CYAN"ARCHIVED FILE: "
                    S_COLOR_WHITE"%s (%lu/%lu)\n",
                    filename,
                    fi.compressed_size,
                    fi.uncompressed_size);

        if ( fn.rfind(granger_binary_name) != std::string::npos )
            granger_exe = fn;

        else if ( fn.rfind("main.lua") != std::string::npos )
            granger_main_lua = fn;

        err = unzOpenCurrentFile(z);
        assert( err == UNZ_OK );

        // FIXME cleanup all this shit string stuff
        std::string path = extract_path;
        path += PATH_SEP;
        path += fn;

        Com_DPrintf(S_COLOR_YELLOW"Extracted FILE: "
                    S_COLOR_WHITE"%s\n",
                    path.c_str());

        std::fstream dl;
        dl.open(path, fstream::out|ios::binary);
    
        // Extract the release package
        size_t numwrote;
        do {
            // Extract 16k at a time
            unsigned blocksiz = 16384; 

            if ( blocksiz > fi.uncompressed_size - numwrote )
                blocksiz = fi.uncompressed_size - numwrote;

            uint8_t block[blocksiz];
            unzReadCurrentFile(z, static_cast<void*>(&block), blocksiz);

            dl.write((const char*)block, blocksiz);
            numwrote += blocksiz;

        } while ( fi.uncompressed_size > numwrote ); 
        dl.close();

        unzCloseCurrentFile(z);
        unzGoToNextFile(z);
    }

    unzClose(z);
}

void UpdateManager::download()
{
    // Download the latest release
    auto url = Cvar_VariableString("cl_latestDownload");
    auto r = RestClient::get(url);
    if ( r.code != 200 )
        throw exception();

    Com_DPrintf(S_COLOR_CYAN "URL: " S_COLOR_WHITE "%s\n", url);

    // FIXME Cleanup this string bullshit
    std::string extract_path, path { Cvar_VariableString("fs_homepath") };
    path += PATH_SEP;
    extract_path = path;
    path += package_name;

    MakeDir(extract_path, "extract");
    extract_path += "extract";

    Com_DPrintf(S_COLOR_CYAN"PATH: " S_COLOR_WHITE "%s\n",
            path.c_str());

    // Write the release package to disk
    std::fstream dl;
    dl.open( path, fstream::out|ios::binary );
    dl.write( r.body.c_str(), r.body.length() );
    dl.close();

    // Extract the contents of the release package
    UpdateManager::extract(extract_path, path);

    // Delete the release package
    unlink(path.c_str());

    Cvar_SetValue("au_autoupdate_action", AU_ACT_RUN);
}

extern char** environ;

class FailInstaller : public std::exception {
    std::string msg;
public:
    FailInstaller(int e)
    { msg = strerror(e); }

    virtual const char* what() throw()
    { return msg.c_str(); }
};

void UpdateManager::execute()
{
    granger_exe = "";
    granger_main_lua = "";

    if ( granger_exe == "" || granger_main_lua == "" )
    {
        for ( auto const& i : release_package )
        {
            Com_Printf(S_COLOR_RED " Unlink %s\n", i.c_str());
            unlink(i.c_str());
        }

        Com_Error( ERR_DROP, "Missing Granger or GrangerScript\n" );
        return;
    }

	std::array<const char*, 4> argv{};

    char* tmp = strdup(granger_exe.c_str());
    const char* dir = dirname(tmp);

	argv[0] = granger_exe.c_str();
    argv[1] = va(" -C %s", dir);
    argv[2] = granger_main_lua.c_str();

    // Dump the details
	Com_Printf(S_COLOR_YELLOW"Launching "
               S_COLOR_WHITE"%s %s %s\n",
               argv[0], argv[1], argv[2]);

#ifndef _WIN32
    // Fork solely to try cleanup SDL2/GL states
	auto pid = fork();
	if (pid == -1)
		throw FailInstaller(errno);

	if (pid == 0)
	{
		execve(argv[0],
			const_cast<char **>(argv.data()),
			environ);

		throw FailInstaller(errno);
	}
	else
	{
		Engine_Exit("");
	}
#else
    // FIXME Dirty exit on Windows...
	execve(argv[0],
		const_cast<char **>(argv.data()),
		environ);

	throw FailInstaller(errno);
#endif
}

void CL_GetLatestRelease() { UpdateManager::refresh(); }
void CL_DownloadRelease() { UpdateManager::download(); }
void CL_ExecuteInstaller() { UpdateManager::execute(); }
