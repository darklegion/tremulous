//
// Restclient wrapper abusively modified to be a one-off donwloader.
// -wtfbbqhax
//
// TODO Verify sha256 of file if exists

#include "cl_rest.h"

#include <iostream>
#include <fstream>
#include <vector>

#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "restclient.h"

static bool is_good(std::string filename, int permissions = (R_OK|W_OK))
{
    int ret = access(filename.c_str(), permissions);
    if (ret)
        std::cerr << filename << ": " << strerror(errno) << std::endl;

    return !ret;
}

bool GetTremulousPk3s(const char* destdir, const char* basegame)
{
    std::string baseuri = "https://github.com/wtfbbqhax/tremulous-data/raw/master/";
    std::vector<std::string> files = { 
        "data-gpp1.pk3",
        "data-1.1.0.pk3",
        "map-atcs-1.1.0.pk3",
        "map-karith-1.1.0.pk3",
        "map-nexus6-1.1.0.pk3",
        "map-niveus-1.1.0.pk3",
        "map-transit-1.1.0.pk3",
        "map-tremor-1.1.0.pk3",
        "map-uncreation-1.1.0.pk3"
    };

    RestClient::init();

    for (auto f : files )
    {
        std::string destpath(destdir);
        destpath += "/";
        destpath += basegame;
        destpath += "/";
        destpath += f;

        if ( is_good(destpath) )
            continue;

        std::cout << "Downloading " << baseuri << f << std::endl;

        RestClient::Response resp = RestClient::get(baseuri + f);

        std::ofstream dl;
        dl.open(destpath);
        dl << resp.body;
        dl.close();
    }

    return true;
}
