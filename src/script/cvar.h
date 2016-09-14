// This file is part of Tremulous.
// Copyright © 2016 Victor Roemer (blowfish) <victor@badsec.org>
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

#ifndef __cplusplus
#error __file__ " is only available to C++"
#endif

#ifndef SCRIPT_CVAR_H
#define SCRIPT_CVAR_H

namespace sol {
    class state;
};

//class CvarApi : public ScriptApi {
//private:
//    CvarApi() { };
//public:
//
#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

struct Cvar : cvar_s {
private:
    Cvar() { }
public:
    Cvar(std::string name)
    {
        Cvar_Get(name.c_str(), "", 0);
    }

    Cvar(std::string name, std::string value)
    {
        Cvar_Get(name.c_str(), value.c_str(), CVAR_ARCHIVE);
    }

    Cvar(std::string name, std::string value, int flags)
    {
        Cvar_Get(name.c_str(), value.c_str(), flags);
    }

	static void Set(std::string name, std::string value)
    {
        Cvar_Set(name.c_str(), value.c_str());
    }
};

namespace CvarApi {

    static inline void init(sol::state* lua)
    {
        lua->new_usertype<Cvar>( "cvar",
                sol::constructors<sol::types<std::string>,
								  sol::types<std::string, std::string>,
								  sol::types<std::string, std::string, int>>(),
                "int", &Cvar::integer,
                "string", &Cvar::string,
                "value", &Cvar::value
		);
    }
};

#endif
