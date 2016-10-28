#include "cl_updates.h"

#include <iostream>
#include <fstream>
#include <vector>

#include "../restclient/restclient.h"
#include "../restclient/connection.h"
#include "../rapidjson/document.h"
//#include "semver/cpp/semver.h"

//#include "platform.h"

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
        Cvar_Set("cl_latestRelease", "^1ERROR:\n^7Server did not return OK status code");
        return;
    }

    rapidjson::Document d;
    d.Parse( r.body.c_str() );

    rapidjson::Value &release = d[0];
    std::string txt;
    
    txt += release["tag_name"].GetString();

    if ( release["prerelease"].IsTrue() )
        txt += " ^3(PRE RELEASE)";

    txt += "\n^5RELEASE DATE: ^7";
    txt += release["published_at"].GetString();

    if ( !release["body"].IsNull() )
    {
        txt += '\n';
        txt += "^1RELEASE NOTES:";
        txt += '\n';
        txt += release["body"].GetString();
    }
        
    Cvar_Set("cl_latestRelease", txt.c_str());

#if 0
    rapidjson::Value &assets = release["assets"];
    for ( rapidjson::SizeType i = 0; i < assets.Size(); ++i )
    {
        auto &a = assets[i];
        std::string name = a["name"].GetString();

        if ( name == package_name )
        {
            std::string url = a["browser_download_url"].GetString();
            auto r = RestClient::get(url);

            if ( r.code != 200 )
                throw exception();

            std::fstream dl;
            dl.open( package_name, fstream::out|ios::binary );
            dl.write( r.body.c_str(), r.body.length() );
            dl.close();
        }
    }
#endif
}
