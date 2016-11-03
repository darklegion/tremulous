#include "cl_updates.h"

#include <iostream>
#include <fstream>
#include <vector>

#include "../restclient/restclient.h"
#include "../restclient/connection.h"
#include "../rapidjson/document.h"

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

using namespace std;

constexpr auto url =
   "https://api.github.com/repos/wtfbbqhax/tremulous/releases";

static int nextCheckTime = 0;

void CL_GetLatestRelease()
{
	auto currentTime = Sys_Milliseconds();

    if ( nextCheckTime > currentTime )
        return;

    nextCheckTime = currentTime + 10000;

    RestClient::Response r = RestClient::get(url);

    if ( r.code != 200 )
    {
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

    if ( release["prerelease"].IsTrue() )
        txt += S_COLOR_YELLOW " (Prerelease)";

    txt += '\n';
    txt += S_COLOR_CYAN "Released:" S_COLOR_WHITE;
    txt += release["published_at"].GetString();

    txt += '\n';
    txt += S_COLOR_RED "Release Notes:\n" S_COLOR_WHITE;

    if ( !release["body"].IsNull() )
    {
        txt += '\n';
        txt += release["body"].GetString();
    }
        
    Cvar_Set("cl_latestRelease", txt.c_str());
}
