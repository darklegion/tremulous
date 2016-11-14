#include "cl_updates.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <array>

#include <unistd.h>

#include "restclient/restclient.h"
#include "restclient/connection.h"

#include "rapidjson.h"

#include "lua.hpp"
//#include "../sol/sol.h"

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

using namespace std;


struct UpdateManager {

    static void refresh();
    static void download();
    static void execute(const char *path);

private:
    static constexpr auto url = "https://api.github.com/repos/wtfbbqhax/tremulous/releases";
    static constexpr auto package_name = "release-mingw32-x86_64.zip";
    // WIN32 "release-mingw32-x86_64.zip";
    // __linux__ "release-linux-x86_64.zip";
    // __APPLE__ "release-darwin-x86_64.zip";
};

static int nextCheckTime = 0;

void UpdateManager::refresh()
{
	auto currentTime = Sys_Milliseconds();

    if ( nextCheckTime > currentTime )
        return;

    nextCheckTime = currentTime + 10000;

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
        }
    }
}

void UpdateManager::download()
{
    auto url = Cvar_VariableString("cl_latestDownload");
    auto r = RestClient::get(url);
    if ( r.code != 200 )
        throw exception();

    std::string path { Cvar_VariableString("fs_homepath") };
    path += PATH_SEP;
    path += package_name;

    std::fstream dl;
    dl.open( path, fstream::out|ios::binary );
    dl.write( r.body.c_str(), r.body.length() );
    dl.close();
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

// TODO
void UpdateManager::execute(const char *path)
{
	std::string cmd{ Sys_DefaultInstallPath() };
	cmd += PATH_SEP;
	cmd += "tremulous-installer";

#warning "path is needs to be sanitized!"

	std::array<const char*, 256> argv{};

	argv[0] = cmd.c_str();
	if (path && path[0])
		argv[1] = path;

	Com_Printf(S_COLOR_YELLOW "Executing %s\n", cmd.c_str());

#ifndef _WIN32
	auto pid = fork();
	if (pid == -1)
		throw FailInstaller(errno);

	if (pid == 0)
	{
		execve(cmd.c_str(),
			const_cast<char **>(argv.data()),
			environ);

		throw FailInstaller(errno);
	}
	else
	{
		Engine_Exit("");
	}
#else
	execve(cmd.c_str(),
		const_cast<char **>(argv.data()),
		environ);

	throw FailInstaller(errno);
#endif
}

void CL_GetLatestRelease() { UpdateManager::refresh(); }
void DownloadRelease() { UpdateManager::download(); }
void ExecuteInstaller(const char*path) { UpdateManager::execute(path); }
