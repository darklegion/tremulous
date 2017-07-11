#ifndef _CL_REST_H_
#define _CL_REST_H_

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

bool    GetPermissions(const char*execpath);

// Download a fresh copy of the "required" base game files to the top-most fs_basegame.
//
// Returns true(1) on success
// Return false(0) on error
//
bool    GetTremulousPk3s(const char* destdir, const char* basegame);

#ifdef __cplusplus
};
#endif

// XXX Hack
#ifdef __cplusplus
#include <string>

//bool is_good(std::string filename, int permissions = (R_OK|W_OK));
bool MakeDir(std::string destdir, std::string basegame);
#endif

#endif
