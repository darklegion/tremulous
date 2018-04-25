#include "cl_updates.h"

#include <libgen.h>
#include <unistd.h>

#include "nettle/rsa.h"
#include "nettle/sha2.h"
#include "rapidjson.h"
#include "restclient/connection.h"
#include "restclient/restclient.h"
#include "semantic_version.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <array>

#include "qcommon/cvar.h"
#include "qcommon/q_platform.h"
#include "qcommon/q_shared.h"
#include "qcommon/qcommon.h"
#include "qcommon/unzip.h"
#include "sys/sys_shared.h"

#include "cl_rest.h"

using namespace std;

std::vector<std::string> release_package;
static std::string granger_exe;
static std::string granger_main_lua;
static int nextCheckTime = 0;

static const uint8_t release_key_pub[] = {
  0x28, 0x31, 0x30, 0x3a, 0x70, 0x75, 0x62, 0x6c, 0x69, 0x63, 0x2d, 0x6b,
  0x65, 0x79, 0x28, 0x39, 0x3a, 0x72, 0x73, 0x61, 0x2d, 0x70, 0x6b, 0x63,
  0x73, 0x31, 0x28, 0x31, 0x3a, 0x6e, 0x35, 0x31, 0x33, 0x3a, 0x00, 0xaa,
  0x4a, 0xe1, 0xdc, 0x08, 0xed, 0x90, 0x92, 0x66, 0x4a, 0xc2, 0x00, 0x43,
  0x3e, 0x89, 0x40, 0x4d, 0x11, 0xaa, 0x98, 0x2d, 0x55, 0x08, 0xec, 0x43,
  0x9f, 0x73, 0xfe, 0xc9, 0x67, 0xcb, 0xb0, 0x4a, 0x52, 0x89, 0x1d, 0x1a,
  0xc1, 0x86, 0x29, 0x26, 0x32, 0xb2, 0x34, 0xb6, 0xa5, 0x42, 0x16, 0x08,
  0x60, 0x8d, 0xf1, 0xe4, 0x45, 0xd4, 0x90, 0x2b, 0xfb, 0x98, 0xb2, 0x2e,
  0xa7, 0xa9, 0x79, 0xff, 0x5e, 0xaa, 0xb5, 0xd6, 0xf9, 0xf9, 0x2e, 0x67,
  0x5c, 0xb6, 0x6c, 0x36, 0x70, 0xf3, 0xea, 0x3f, 0xd8, 0xc8, 0xd0, 0xda,
  0xd8, 0xfb, 0x1b, 0x55, 0xe9, 0x69, 0x9d, 0x4a, 0xea, 0xf2, 0xb1, 0xdd,
  0x6b, 0xea, 0xb4, 0x99, 0xd7, 0x5e, 0xca, 0x5f, 0x34, 0xcf, 0xee, 0xbb,
  0xc6, 0x07, 0xa8, 0x3b, 0x59, 0xc3, 0xc1, 0x53, 0x15, 0x2b, 0xe4, 0x2f,
  0x17, 0x2d, 0x0c, 0x0f, 0x4e, 0xee, 0x2e, 0xef, 0x97, 0x9c, 0xff, 0x58,
  0xec, 0x6a, 0xf1, 0x12, 0x21, 0x5a, 0xc5, 0xbb, 0x6a, 0xef, 0xb0, 0xc8,
  0x4b, 0x7d, 0x33, 0xca, 0x53, 0x03, 0x31, 0x4a, 0xbd, 0x82, 0x01, 0x56,
  0x8f, 0x4e, 0x1d, 0x21, 0x7d, 0x2e, 0x8d, 0xaa, 0x0d, 0xcd, 0xe6, 0x19,
  0x09, 0x79, 0x76, 0x11, 0x16, 0xb5, 0xd2, 0xc1, 0x03, 0xdb, 0xaa, 0x11,
  0xd5, 0x89, 0x70, 0xc8, 0x71, 0xbf, 0x1a, 0xea, 0x9d, 0xd6, 0xaa, 0x77,
  0x97, 0xa4, 0xcc, 0x1d, 0xd6, 0x1c, 0x1a, 0x14, 0x6f, 0x35, 0x34, 0x6a,
  0x10, 0x32, 0x13, 0x33, 0x41, 0x27, 0xad, 0x1b, 0x59, 0x41, 0xff, 0xb0,
  0x1b, 0x64, 0x14, 0xea, 0x29, 0x44, 0x2b, 0x62, 0xdc, 0x63, 0xbc, 0xc7,
  0xf8, 0xac, 0xf7, 0x5c, 0x9d, 0x3c, 0xbd, 0x17, 0x84, 0xd9, 0x56, 0x09,
  0x44, 0x1b, 0xdd, 0x72, 0x16, 0xf6, 0x67, 0xc4, 0x49, 0xd3, 0x56, 0x74,
  0x42, 0x2d, 0x08, 0xe2, 0x0b, 0x0c, 0x40, 0x97, 0x62, 0x97, 0xb3, 0xd6,
  0x0c, 0x69, 0x04, 0x6a, 0xd7, 0x7b, 0xf0, 0xe1, 0x37, 0x0b, 0xe0, 0xee,
  0x6b, 0x2d, 0x79, 0xa0, 0x72, 0xa5, 0x75, 0x97, 0xb1, 0x6b, 0x01, 0xa2,
  0xb8, 0xe3, 0xc7, 0x3c, 0x10, 0x50, 0x59, 0xe4, 0xda, 0x9e, 0x8d, 0xe6,
  0x1b, 0xef, 0xa6, 0xae, 0xc4, 0xd7, 0xd8, 0x9b, 0x57, 0x4c, 0xa3, 0xd7,
  0x9f, 0x37, 0x17, 0x72, 0x7f, 0x60, 0xd6, 0x4c, 0x42, 0x14, 0x54, 0x66,
  0x88, 0xc6, 0xa2, 0x1e, 0xdf, 0x55, 0xfc, 0xef, 0xb9, 0x05, 0x63, 0x8a,
  0xc4, 0x5b, 0xe7, 0x45, 0x28, 0x0e, 0x55, 0x8e, 0xcc, 0x74, 0x25, 0xf2,
  0xea, 0x2a, 0x66, 0x51, 0xf2, 0x8f, 0x72, 0x1d, 0x97, 0x5a, 0xfb, 0x1c,
  0x95, 0x01, 0x93, 0x6b, 0x2b, 0xa5, 0x87, 0x5d, 0xd8, 0xea, 0xb2, 0x24,
  0x78, 0xb1, 0x58, 0x63, 0x47, 0x86, 0xed, 0xd9, 0xbc, 0xe9, 0xd2, 0xea,
  0xa8, 0x90, 0x5b, 0xe8, 0x82, 0x89, 0xa2, 0xe2, 0x52, 0x1f, 0x78, 0x00,
  0x93, 0x64, 0x54, 0xdb, 0x9b, 0x93, 0xc3, 0xee, 0xa2, 0x37, 0xab, 0x2e,
  0x7e, 0x8f, 0xec, 0xb1, 0x0b, 0x69, 0xad, 0x21, 0xda, 0xa9, 0xaf, 0xd2,
  0x22, 0x52, 0x2f, 0x1a, 0x6b, 0xb9, 0x21, 0x5e, 0xe9, 0x1a, 0xe1, 0x4c,
  0x33, 0x26, 0x46, 0xa1, 0xde, 0x52, 0xf3, 0x87, 0xf4, 0x8c, 0x99, 0xe4,
  0x5d, 0xe7, 0x5b, 0x76, 0x6c, 0xf5, 0xe1, 0xae, 0x5b, 0xaa, 0xbb, 0x3a,
  0xf4, 0x90, 0xa7, 0x5c, 0x5c, 0x72, 0xab, 0xb8, 0x71, 0xdc, 0x47, 0xb1,
  0x75, 0x0d, 0xc8, 0xcc, 0xfd, 0xce, 0x62, 0xbe, 0xe5, 0x9f, 0x85, 0x00,
  0x53, 0x35, 0x91, 0x44, 0x29, 0x5f, 0x64, 0x8d, 0x7c, 0x37, 0x32, 0x28,
  0x45, 0x9d, 0xa3, 0x49, 0xbb, 0xe3, 0xd3, 0x1e, 0x0a, 0xae, 0x53, 0xbd,
  0x66, 0x86, 0x8c, 0x4d, 0xa6, 0xd3, 0x85, 0x29, 0x28, 0x31, 0x3a, 0x65,
  0x33, 0x3a, 0x01, 0x00, 0x01, 0x29, 0x29, 0x29
};
static const size_t release_key_pub_len = 560;

struct UpdateManager {

    static void refresh();
    static void download();
    static void validate_signature(std::string path, std::string signature_path);
    static void extract(std::string extract_path, std::string path);
    static void execute();

private:
    static constexpr auto url = "https://api.github.com/repos/GrangerHub/tremulous/releases";
    static constexpr auto package_name = RELEASE_PACKAGE_NAME;
    static constexpr auto signature_name = RELEASE_SIGNATURE_NAME;
    static constexpr auto granger_binary_name = GRANGER_EXE;
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
    Cvar_Set("cl_latestSignature", "");

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
    
    semver::v2::Version current { PRODUCT_VERSION + 1 };
    semver::v2::Version latest { release["tag_name"].GetString() + 1 };

    if ( current == latest )
    {
        txt += "You are up to date\n\n";
    }
    else if ( current < latest )
    {
        txt += "A new release is available!\n\n";
    }
    else if ( current > latest )
    { 
        txt += "Wow! You are ahead of the release.\n\n";
    }
    
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
        else if ( name == signature_name )
        {
            std::string dl = a["browser_download_url"].GetString();
            Cvar_Set("cl_latestSignature", dl.c_str());
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
    cvar_t *cl_enableSignatureCheck = Cvar_Get("cl_enableSignatureCheck", "0", CVAR_ARCHIVE | CVAR_PROTECTED);

    // Check for and download signature
    std::string signature_url = Cvar_VariableString("cl_latestSignature");
    if ( signature_url.empty() && cl_enableSignatureCheck->integer)
        throw exception();

    // Download the latest release
    auto url = Cvar_VariableString("cl_latestDownload");
    auto r = RestClient::get(url);
    if ( r.code != 200 )
        throw exception();

    Com_DPrintf(S_COLOR_CYAN "URL: " S_COLOR_WHITE "%s\n", url);

    // FIXME Cleanup this string bullshit
    std::string extract_path, signature_path, path { Cvar_VariableString("fs_homepath") };
    path += PATH_SEP;
    extract_path = path;
    signature_path = path + signature_name;
    path += package_name;

    MakeDir(extract_path, "extract");
    extract_path += "extract";

    Com_DPrintf(S_COLOR_CYAN"PATH: " S_COLOR_WHITE "%s\n",
            path.c_str());

    // Write the release package to disk
    {
        std::fstream dl;
        dl.open( path, fstream::out|ios::binary );
        dl.write( r.body.c_str(), r.body.length() );
        dl.close();
    }

    if (cl_enableSignatureCheck->integer)
    {
        auto r = RestClient::get(signature_url);
        if ( r.code != 200 )
            throw exception();

        std::fstream dl;
        dl.open( signature_path, fstream::out|ios::binary );
        dl.write( r.body.c_str(), r.body.length() );
        dl.close();

        // Validate the signature of the package if enabled
        UpdateManager::validate_signature(path, signature_path);
    }

    // Extract the contents of the release package
    UpdateManager::extract(extract_path, path);

    // Delete the release package
    unlink(path.c_str());

    Cvar_SetValue("au_autoupdate_action", AU_ACT_RUN);
}

void UpdateManager::validate_signature(std::string path, std::string signature_path)
{
    // Load public key
    rsa_public_key public_key;
    rsa_public_key_init(&public_key);
    rsa_keypair_from_sexp(&public_key, NULL, 0, release_key_pub_len, release_key_pub);

    auto min = [](auto a, auto b) { return a < b ? a : b; };

    // Read in signature
    mpz_t signature;
    {
        std::ifstream f(signature_path, ios::binary);
        f.seekg (0, f.end);
        size_t length = f.tellg();
        f.seekg (0, f.beg);

        std::vector<char> buffer(512, 0);
        f.read(buffer.data(), min(length, buffer.size()));
        nettle_mpz_init_set_str_256_u(signature, f.gcount(), (uint8_t *)buffer.data());
    }

    // Hash file
    sha256_ctx ctx;
    sha256_init(&ctx);
    {
        std::ifstream f(path, ios::binary);
        f.seekg (0, f.end);
        size_t length = f.tellg();
        f.seekg (0, f.beg);

        std::vector<unsigned char> buffer(16384, 0);
        while (f.read((char *)buffer.data(), min(length, buffer.size()))) {
            auto nbytes = f.gcount();
            sha256_update(&ctx, nbytes, buffer.data());
            length -= nbytes;
            if (length <= 0) {
                break;
            }
        }
    }

    // Verify signature
    if (!rsa_sha256_verify(&public_key, &ctx, signature)) {
        rsa_public_key_clear(&public_key);
        mpz_clear(signature);
        Com_Error( ERR_DROP, "Update signature was not verified\n" );
        return;
    }

    unlink(signature_path.c_str());
    rsa_public_key_clear(&public_key);
    mpz_clear(signature);
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
