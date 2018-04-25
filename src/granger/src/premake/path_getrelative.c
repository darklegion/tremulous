/**
 * \file   path_getrelative.c
 * \brief  Returns a path relative to another.
 * \author Copyright (c) 2002-2013 Jason Perkins and the Premake project
 */

#include "premake.h"
#include <string.h>


int path_getrelative(lua_State* L)
{
	int i, last, count;
	char src[0x4000];
	char dst[0x4000];

	const char* p1 = luaL_checkstring(L, 1);
	const char* p2 = luaL_checkstring(L, 2);

	/* normalize the paths */
	do_getabsolute(src, p1, NULL);
	do_getabsolute(dst, p2, NULL);

	/* same directory? */
	if (strcmp(src, dst) == 0) {
		lua_pushstring(L, ".");
		return 1;
	}

	/* dollar macro? Can't tell what the real path might be, so treat
	 * as absolute. This enables paths like $(SDK_ROOT)/include to
	 * work as expected. */
	if (dst[0] == '$') {
		lua_pushstring(L, dst);
		return 1;
	}

	/* find the common leading directories */
	strcat(src, "/");
	strcat(dst, "/");

	last = -1;
	i = 0;
	while (src[i] && dst[i] && src[i] == dst[i]) {
		if (src[i] == '/') {
			last = i;
		}
		++i;
	}

	/* if I end up with just the root of the filesystem, either a single
	 * slash (/) or a drive letter (c:) then return the absolute path. */
	if (last <= 0 || (last == 2 && src[1] == ':')) {
		dst[strlen(dst) - 1] = '\0';
		lua_pushstring(L, dst);
		return 1;
	}

	/* count remaining levels in src */
	count = 0;
	for (i = last + 1; src[i] != '\0'; ++i) {
		if (src[i] == '/') {
			++count;
		}
	}

	/* start my result by backing out that many levels */
	src[0] = '\0';
	for (i = 0; i < count; ++i) {
		strcat(src, "../");
	}

	/* append what's left */
	strcat(src, dst + last + 1);

	/* remove trailing slash and done */
	src[strlen(src) - 1] = '\0';
	lua_pushstring(L, src);
	return 1;
}
