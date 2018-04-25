//
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
//

#ifndef __cplusplus
#error __file__ " is only available to C++"
#endif

#ifndef SCRIPT_NETTLE_H
#define SCRIPT_NETTLE_H

#include <iostream>

#define LUA_BUILD_AS_DLL
#define LUA_CORE
#include "lua.hpp"
#include "sol.hpp"

#include "lnettlelib.h"

namespace script {
    namespace nettle {
        static inline void init(sol::state&& lua)
        { lua.require("nettle", luaopen_nettle, 1); }
    };
};

#endif
