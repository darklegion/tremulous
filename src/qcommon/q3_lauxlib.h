// This file is part of Tremulous.
// Copyright © 2016 Victor Roemer (blowfish) <victor@badsec.org>
// Copyright (C) 2015-2018 GrangerHub
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.

#ifndef OVERRIDE_LAUXLIB_H
#define OVERRIDE_LAUXLIB_H

#include <stdarg.h>
#include <sys/types.h>

#include "q_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

SO_PUBLIC size_t lua_writestring(const char* string, size_t n);
SO_PUBLIC int lua_writeline(void);
SO_PUBLIC int lua_writestringerror(const char *fmt, ...);

#define LUA_TMPNAMTEMPLATE	"/tmp/tremulous_XXXXXX"

#ifdef __cplusplus
}
#endif
#endif
