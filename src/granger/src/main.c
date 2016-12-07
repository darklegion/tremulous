/*
 * This file is part of Granger.
 * Copyright (c) 2016 Jeff Kent <jeff@jkent.net>
 *
 * Granger is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Granger is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Granger.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
#include "premake/premake.h"

#if defined(PLATFORM_WINDOWS)
#include "getopt.h"
#endif

#include "lnettlelib.h"

extern const luaL_Reg extra_os_functions[];

int main (int argc, char *argv[])
{
	char *script_path;
	lua_State *L;
	int c;

	while ((c = getopt(argc, argv, "C:")) != -1) {
		switch (c) {
		case 'C':
			if (chdir(optarg)) {
				fprintf(stderr, "could not change working directory\n");
				return EXIT_FAILURE;
			}
			break;
		case '?':
		default:
			return EXIT_FAILURE;
		}
	}

	if (optind >= argc || argv[optind][0] == '-') {
		fprintf(stderr, "lua script not provided\n");
		return EXIT_FAILURE;
	}
	script_path = argv[optind];
	optind++;

	L = luaL_newstate();
	if (L == NULL) {
		fprintf(stderr, "cannot create lua state\n");
		return EXIT_FAILURE;
	}

	luaL_openlibs(L);
	luaL_requiref(L, "nettle", luaopen_nettle, 1);
	lua_pop(L, 1);

	premake_init(L);
	premake_locate(L, argv[0]);
	lua_setglobal(L, "_EXE_PATH");

	lua_pushstring(L, script_path);
	lua_setglobal(L, "_GRANGER_SCRIPT");

	lua_newtable(L);
	for (int i = 1; optind < argc; i++, optind++) {
		lua_pushinteger(L, i);
		lua_pushstring(L, argv[optind]);
		lua_settable(L, 1);
	}
	lua_setglobal(L, "argv");

	if (luaL_dofile(L, script_path)) {
		fprintf(stderr, "Error: %s\n", lua_tostring(L, -1));
		lua_close(L);
		return EXIT_FAILURE;
	}

	lua_close(L);
	return EXIT_SUCCESS;
}
