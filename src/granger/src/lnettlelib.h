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

#ifndef lnettlelib_h
#define lnettlelib_h

#define LUA_NETTLELIBNAME	"nettle"
LUAMOD_API int (luaopen_nettle) (lua_State *L);

#endif /* lnettlelib_h */
