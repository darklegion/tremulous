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

#include "premake.h"

#if defined(PLATFORM_WINDOWS)
#include <io.h>
#define access _access
#define X_OK 1
#define W_OK 2
#define R_OK 4
#endif

int os_access(lua_State *L)
{
	const char *filename = luaL_checkstring(L, 1);
	const char *mode = luaL_optstring(L, 2, "r");
	int nmode = 0;
	int result;

	while (*mode) {
		switch (*mode) {
		case 'x':
			nmode |= X_OK;
			break;
		case 'w':
			nmode |= W_OK;
			break;
		case 'r':
			nmode |= R_OK;
			break;
		default:
			lua_pushboolean(L, 0);
			return 1;
		}
		mode++;
	}

	result = access(filename, nmode);

	lua_pushboolean(L, result == 0);
	return 1;
}
