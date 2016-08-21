
//

// _                    _       _             __                
//| |   _   _  __ _    (_)_ __ | |_ ___ _ __ / _| __ _  ___ ___ 
//| |  | | | |/ _` |   | | '_ \| __/ _ \ '__| |_ / _` |/ __/ _ \
//| |__| |_| | (_| |   | | | | | ||  __/ |  |  _| (_| | (_|  __/
//|_____\__,_|\__,_|   |_|_| |_|\__\___|_|  |_|  \__,_|\___\___|
//                                                            

//
//#if 0 && FEAT_LUA
//# error "Lua feature is not enabled."
//#endif
#include <iostream>

#include "client.h"

#include "../sol-v2.11.5/sol.hpp"

// The global lua state
extern sol::state* glua;

#if 0
#include "module.hpp"
class LuaModule : public Module
{
public:
    bool begin() override;
    bool end() override;
};

bool LuaModule::begin()
{
    lua.open_libraries(sol::lib::base, sol::lib::package);
    lua.script(R"(print "LuaModule::begin")");

    lua.new_usertype<clientConnection_t>( "clc",
            "state", sol::readonly( &clc.state ),
            "newsString", sol::readonly( &clc.newsString )
            );

    return true;
}

bool LuaModule::end()
{
    lua.script(R"(print "LuaModule::end")");
    return true;
}
#endif
