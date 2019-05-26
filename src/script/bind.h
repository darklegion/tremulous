//
// This file is part of Tremulous.
// Copyright © 2016 Victor Roemer (blowfish) <victor@badsec.org>
// Copyright (C) 2015-2019 GrangerHub
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
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

#ifndef SCRIPT_KEY_H
#define SCRIPT_KEY_H

#include "client/client.h"
#include "client/keys.h"

namespace sol
{ 
    class state;
};

namespace script 
{
    class InvalidKey : public std::exception {
    public:
        virtual const char* what() const throw()
        { return "Invalid Key"; }
    };

    struct Bind {
    private:
        qkey_t *my = nullptr;
        int keynum = 0;

        Bind() = delete;

    public:
        Bind(qkey_t* key)
            : my(key)
        { }

        Bind(std::string key)
        {
            keynum = Key_StringToKeynum(key.c_str());
            if ( keynum < 0 ) throw InvalidKey(); 
            my = &keys[ keynum ];
        }

        Bind(std::string key, std::string cmd)
        {
            keynum = Key_StringToKeynum(key.c_str());
            if ( keynum < 0 ) throw InvalidKey(); 
            my = &keys[ keynum ];
            Key_SetBinding(keynum, cmd.c_str());
        }

        Bind(std::string key, std::string cmd, bool overwrite)
        {
            keynum = Key_StringToKeynum(key.c_str());
            if ( keynum < 0 ) throw InvalidKey(); 
            my = &keys[ keynum ];
            if ( !my->binding || overwrite )
                Key_SetBinding(keynum, cmd.c_str());
        }

        std::string get_value()
        { return my->binding; }

        void set_value(std::string cmd) {
            if ( my->binding ) Z_Free( my->binding );
            my->binding = CopyString(cmd.c_str());
            // consider this like modifying an archived cvar, so the file
            // write will be triggered at the next oportunity
            cvar_modifiedFlags |= CVAR_ARCHIVE;
        }

    };

    namespace keybind 
    {
        static inline void init(sol::state&& lua)
        {
            lua.new_usertype<Bind>(
                    "bind", sol::constructors<sol::types<std::string>,
                             sol::types<std::string, std::string>,
                             sol::types<std::string, std::string, bool>>(),
                    "value", sol::property(&Bind::get_value, &Bind::set_value)
                    //"key", sol::property(&Bind::get_key)
                );
        }
    };
};

#endif
