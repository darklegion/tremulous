/**
 * \file   premake.c
 * \brief  Program entry point.
 * \author Copyright (c) 2002-2013 Jason Perkins and the Premake project
 */

#include <stdlib.h>
#include <string.h>
#include "premake.h"

#if PLATFORM_MACOSX
#include <CoreFoundation/CFBundle.h>
#endif

int premake_locate(lua_State* L, const char* argv0);

/* Built-in functions */
static const luaL_Reg path_functions[] = {
	{ "getabsolute", path_getabsolute },
	{ "getrelative", path_getrelative },
	{ "isabsolute",  path_isabsolute },
	{ "join", path_join },
	{ "normalize", path_normalize },
	{ "translate", path_translate },
	{ NULL, NULL }
};

static const luaL_Reg os_functions[] = {
	{ "access",      os_access      },
	{ "chdir",       os_chdir       },
	{ "copyfile",    os_copyfile    },
	{ "elevate",     os_elevate     },
	{ "getcwd",      os_getcwd      },
	{ "_is64bit",    os_is64bit     },
	{ "isdir",       os_isdir       },
	{ "isfile",      os_isfile      },
	{ "matchdone",   os_matchdone   },
	{ "matchisfile", os_matchisfile },
	{ "matchname",   os_matchname   },
	{ "matchnext",   os_matchnext   },
	{ "matchstart",  os_matchstart  },
	{ "mkdir",       os_mkdir       },
	{ "pathsearch",  os_pathsearch  },
	{ "rmdir",       os_rmdir       },
	{ "stat",        os_stat        },
	{ NULL, NULL }
};

static const luaL_Reg string_functions[] = {
	{ "endswith",  string_endswith },
	{ NULL, NULL }
};


/**
 * Initialize the Premake Lua environment.
 */
int premake_init(lua_State* L)
{
	if (lua_getglobal(L, "path") != LUA_TTABLE) {
		lua_pop(L, 1);
		lua_newtable(L);
	}
	luaL_setfuncs(L, path_functions, 0);
	lua_setglobal(L, "path");

	if (lua_getglobal(L, "os") != LUA_TTABLE) {
		lua_pop(L, 1);
		lua_newtable(L);
	}
	luaL_setfuncs(L, os_functions, 0);
	lua_setglobal(L, "os");

	if (lua_getglobal(L, "string") != LUA_TTABLE) {
		lua_pop(L, 1);
		lua_newtable(L);
	}
	luaL_setfuncs(L, string_functions, 0);
	lua_setglobal(L, "string");

	lua_pushstring(L, PLATFORM_STRING);
	lua_setglobal(L, "_OS");

	return OKAY;
}

/**
 * Locate the Premake executable, and push its full path to the Lua stack.
 * Based on:
 * http://sourceforge.net/tracker/index.php?func=detail&aid=3351583&group_id=71616&atid=531880
 * http://stackoverflow.com/questions/933850/how-to-find-the-location-of-the-executable-in-c
 * http://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe
 */
int premake_locate(lua_State* L, const char* argv0)
{
#if !defined(PATH_MAX)
#define PATH_MAX  (4096)
#endif

	char buffer[PATH_MAX];
	const char* path = NULL;

#if PLATFORM_WINDOWS
	DWORD len = GetModuleFileName(NULL, buffer, PATH_MAX);
	if (len > 0)
		path = buffer;
#endif

#if PLATFORM_MACOSX
	CFURLRef bundleURL = CFBundleCopyExecutableURL(CFBundleGetMainBundle());
	CFStringRef pathRef = CFURLCopyFileSystemPath(bundleURL, kCFURLPOSIXPathStyle);
	if (CFStringGetCString(pathRef, buffer, PATH_MAX - 1, kCFStringEncodingUTF8))
		path = buffer;
#endif

#if PLATFORM_LINUX
	int len = readlink("/proc/self/exe", buffer, PATH_MAX);
	if (len > 0)
		path = buffer;
#endif

#if PLATFORM_BSD
	int len = readlink("/proc/curproc/file", buffer, PATH_MAX);
	if (len < 0)
		len = readlink("/proc/curproc/exe", buffer, PATH_MAX);
	if (len > 0)
		path = buffer;
#endif

#if PLATFORM_SOLARIS
	int len = readlink("/proc/self/path/a.out", buffer, PATH_MAX);
	if (len > 0)
		path = buffer;
#endif

	/* As a fallback, search the PATH with argv[0] */
	if (!path)
	{
		lua_pushcfunction(L, os_pathsearch);
		lua_pushstring(L, argv0);
		lua_pushstring(L, getenv("PATH"));
		if (lua_pcall(L, 2, 1, 0) == OKAY && !lua_isnil(L, -1))
		{
			lua_pushstring(L, "/");
			lua_pushstring(L, argv0);
			lua_concat(L, 3);
			path = lua_tostring(L, -1);
		}
	}

	/* If all else fails, use argv[0] as-is and hope for the best */
	if (!path)
	{
		/* make it absolute, if needed */
		os_getcwd(L);
		lua_pushstring(L, "/");
		lua_pushstring(L, argv0);

		if (!path_isabsolute(L)) {
			lua_concat(L, 3);
		}
		else {
			lua_pop(L, 1);
		}

		path = lua_tostring(L, -1);
	}

	lua_pushstring(L, path);
	return 1;
}
