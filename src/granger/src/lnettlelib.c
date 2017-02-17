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

#include <string.h>

#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "nettle/md5.h"
#include "nettle/sha2.h"

#define MD5_CTX "md5_ctx*"
#define SHA256_CTX "sha256_ctx*"

static int lmd5(lua_State *L)
{
	struct md5_ctx *ctx;

	ctx = (struct md5_ctx *)lua_newuserdata(L, sizeof(struct md5_ctx));
	luaL_setmetatable(L, MD5_CTX);
	md5_init(ctx);
	return 1;
}

static int lmd5_digest(lua_State *L)
{
	struct md5_ctx *ctx;
	char digest[MD5_DIGEST_SIZE];

	ctx = luaL_checkudata(L, 1, MD5_CTX);
	md5_digest(ctx, sizeof(digest), digest);
	lua_pushlstring(L, digest, sizeof(digest));
	return 1;
}

static int lmd5_update(lua_State *L)
{
	struct md5_ctx *ctx;
	const char *data;
	size_t len;

	ctx = luaL_checkudata(L, 1, MD5_CTX);
	if (lua_isnil(L, 2)) {
		return 0;
	}
	data = luaL_checklstring(L, 2, &len);
	nettle_md5_update(ctx, len, data);
	return 0;
}

static int lmd5_tostring(lua_State *L)
{
	struct md5_ctx *ctx, ctx2;
	char digest[MD5_DIGEST_SIZE];
	char hex[MD5_DIGEST_SIZE*2+1];
	int i;

	ctx = luaL_checkudata(L, 1, MD5_CTX);
	memcpy(&ctx2, ctx, sizeof(struct md5_ctx));

	md5_digest(&ctx2, sizeof(digest), digest);
	for (i = 0; i < sizeof(digest); i++) {
		sprintf(hex + 2 * i, "%02hhx", digest[i]);
	}

	lua_pushstring(L, hex);
	return 1;
}

static int lsha256(lua_State *L)
{
	struct sha256_ctx *ctx;

	ctx = (struct sha256_ctx *)lua_newuserdata(L, sizeof(struct sha256_ctx));
	luaL_setmetatable(L, SHA256_CTX);
	sha256_init(ctx);
	return 1;
}

static int lsha256_digest(lua_State *L)
{
	struct sha256_ctx *ctx;
	char digest[SHA256_DIGEST_SIZE];

	ctx = luaL_checkudata(L, 1, SHA256_CTX);
	sha256_digest(ctx, sizeof(digest), digest);
	lua_pushlstring(L, digest, sizeof(digest));
	return 1;
}

static int lsha256_update(lua_State *L)
{
	struct sha256_ctx *ctx;
	const char *data;
	size_t len;

	ctx = luaL_checkudata(L, 1, SHA256_CTX);
	if (lua_isnil(L, 2)) {
		return 0;
	}
	data = luaL_checklstring(L, 2, &len);
	nettle_sha256_update(ctx, len, data);
	return 0;
}

static int lsha256_tostring(lua_State *L)
{
	struct sha256_ctx *ctx, ctx2;
	char digest[SHA256_DIGEST_SIZE];
	char hex[SHA256_DIGEST_SIZE*2+1];
	int i;

	ctx = luaL_checkudata(L, 1, SHA256_CTX);
	memcpy(&ctx2, ctx, sizeof(struct sha256_ctx));

	sha256_digest(&ctx2, sizeof(digest), digest);
	for (i = 0; i < sizeof(digest); i++) {
		sprintf(hex + 2 * i, "%02hhx", digest[i]);
	}

	lua_pushstring(L, hex);
	return 1;
}

/* functions for 'nettle' library */
static const luaL_Reg nettlelib[] = {
	{"md5", lmd5},
	{"sha256", lsha256},
	{NULL, NULL}
};

static const luaL_Reg lmd5_methods[] = {
	{"digest", lmd5_digest},
	{"update", lmd5_update},
	{"__tostring", lmd5_tostring},
	{NULL, NULL}
};

/* functions for sha256 objects */
static const luaL_Reg lsha256_methods[] = {
	{"digest", lsha256_digest},
	{"update", lsha256_update},
	{"__tostring", lsha256_tostring},
	{NULL, NULL}
};

static void createmeta (lua_State *L)
{
	luaL_newmetatable(L, MD5_CTX);  /* create metatable for file handles */
	lua_pushvalue(L, -1);  /* push metatable */
	lua_setfield(L, -2, "__index");  /* metatable.__index = metatable */
	luaL_setfuncs(L, lmd5_methods, 0);  /* add file methods to new metatable */
	lua_pop(L, 1);  /* pop new metatable */

	luaL_newmetatable(L, SHA256_CTX);  /* create metatable for file handles */
	lua_pushvalue(L, -1);  /* push metatable */
	lua_setfield(L, -2, "__index");  /* metatable.__index = metatable */
	luaL_setfuncs(L, lsha256_methods, 0);  /* add file methods to new metatable */
	lua_pop(L, 1);  /* pop new metatable */
}

LUAMOD_API int luaopen_nettle (lua_State *L)
{
	luaL_newlib(L, nettlelib);
	createmeta(L);
	return 1;
}
