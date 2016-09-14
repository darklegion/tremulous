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

#include <iostream>
#include <exception>

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

namespace sol
{
    class state;
};

namespace script
{
    ////////////////////////////////////////////// Exceptions ////////////
    class CvarInvalidName : public std::exception {
    public:
        virtual const char* what() const throw()
        { return "cvar name is invalid"; }
    };

    class CvarNotFound : public std::exception {
    public:
        virtual const char* what() const throw()
        { return "cvar does not exist"; }
    };

    class CvarReadOnly : public std::exception { 
    public:
        virtual const char *what() const throw()
        { return "cvar is readonly"; }
    };

    class CvarWriteProtected : public std::exception { 
    public:
        virtual const char *what() const throw()
        { return "cvar is write protected"; }
    };

    class CvarCheatProtected : public std::exception {
    public:
        virtual const char *what() const throw()
        { return "cvar is cheat protected"; }
    };

    class CvarLatchedUnsupported : public std::exception {
    public:
        virtual const char *what() const throw()
        { return "modifying latched cvars is unsupported"; }
    };

    extern "C" cvar_t *cvar_cheats;
    ////////////////////////////////////////////// class Cvar ////////////
    struct Cvar {
    private:
        cvar_s *my = nullptr;

        Cvar() = delete;

    public:
        Cvar(cvar_s* var)
            : my(var)
        { }
 
        Cvar(std::string name)
        { my = Cvar_Get(name.c_str(), "", 0); }
    
        Cvar(std::string name, std::string value)
        { my = Cvar_Get(name.c_str(), value.c_str(), CVAR_ARCHIVE); }
    
        Cvar(std::string name, std::string value, int flags)
        { my = Cvar_Get(name.c_str(), value.c_str(), flags); }
    
    	static Cvar set(std::string name, const char* value, bool force)
        {
            if ( !Cvar_ValidateString(name.c_str()) )
                throw CvarInvalidName();

            cvar_s *my = Cvar_FindVar(name.c_str());
            if ( !my )
                throw CvarNotFound();
            else if ( my->flags & CVAR_ROM )
                throw CvarReadOnly();
            else if ( my->flags & CVAR_INIT )
                throw CvarWriteProtected();
            else if ( my->flags & CVAR_CHEAT && !cvar_cheats->integer )
                throw CvarCheatProtected();
            else if ( (my->flags & CVAR_LATCH) )
                throw CvarLatchedUnsupported();
            else if ( !value )
                value = my->resetString;
            value = Cvar_Validate(my, value, qfalse);
            cvar_modifiedFlags |= my->flags;
            
            // FIXME Set the value

            return Cvar(my);
        }

        void set_value(const char*value)
        {
            if ( !my )
                throw CvarNotFound();
            else if ( my->flags & CVAR_ROM )
                throw CvarReadOnly();
            else if ( my->flags & CVAR_INIT )
                throw CvarWriteProtected();
            else if ( my->flags & CVAR_CHEAT && !cvar_cheats->integer )
                throw CvarCheatProtected();
            else if ( (my->flags & CVAR_LATCH) )
                throw CvarLatchedUnsupported();
            else if ( !value )
                value = my->resetString;
            value = Cvar_Validate(my, value, qfalse);

            cvar_modifiedFlags |= my->flags;

            my->modified = qtrue;
            my->modificationCount++;
            my->string = CopyString(value);
            my->value = atof(my->string);
            my->integer = atoi(my->string);
        }

        std::string get_value()
        { return my->string; }
    };
    
    namespace cvar_api {
        static inline void init(sol::state* lua)
        {
            lua->new_usertype<Cvar>(
                    "cvar", sol::constructors<sol::types<std::string>,
    								  sol::types<std::string, std::string>,
    								  sol::types<std::string, std::string, int>>(),
                    "value", sol::property(&Cvar::get_value, &Cvar::set_value)
            );
        }
    };
};
#endif
