#ifndef __LUA_RAPIDJSON_USERDATA_HPP__
#define __LUA_RAPIDJSON_USERDATA_HPP__


#include "q3_lauxlib.h"

#include "luax.hpp"

#include "lua.hpp"
#include "rapidjson.h"

template <typename T>
struct Userdata
{
    static int create(lua_State* L)
    {
        push(L, construct(L));
        return 1;
    }

    static T* construct(lua_State* L);

    static void luaopen(lua_State* L)
    {
        luaL_newmetatable(L, metatable);
        lua_pushvalue(L, -1);
        luax::setfuncs(L, methods());
        lua_setfield(L, -2, "__index");
        lua_pop(L, 1);
    }

    static const luaL_Reg* methods();

    static void push(lua_State* L, T* c)
    {
        if (!c)
        {
            lua_pushnil(L);
            return;
        }

        T** ud = reinterpret_cast<T**>(lua_newuserdata(L, sizeof(*ud)));
        if (!ud)
            luaL_error(L, "Out of memory");

        *ud = c;

        luaL_getmetatable(L, metatable);
        lua_setmetatable(L, -2);
    }

    static T** getUserdata(lua_State* L, int idx)
    {
        return reinterpret_cast<T**>(lua_touserdata(L, idx));
    }


    static T* get(lua_State* L, int idx)
    {
        auto p = getUserdata(L, idx);
        if (p && *p )
        {
            if (lua_getmetatable(L, idx))
            {  /* does it have a metatable? */
                luaL_getmetatable(L, metatable);  /* get correct metatable */
                if (lua_rawequal(L, -1, -2))
                {  /* does it have the correct mt? */
                    lua_pop(L, 2);  /* remove both metatables */
                    return *p;
                }
            }
        }
        return nullptr;
    }

    static T* check(lua_State* L, int idx)
    {
        auto ud = reinterpret_cast<T**>(luaL_checkudata(L, idx, metatable));
        if (!*ud)
            luaL_error(L, "%s already closed", metatable);
        return *ud;
    }

    static int metamethod_gc(lua_State* L)
    {
        T** ud = reinterpret_cast<T**>(luaL_checkudata(L, 1, metatable));
        if (*ud)
        {
            delete *ud;
            *ud = nullptr;
        }
        return 0;
    }

    static int metamethod_tostring(lua_State* L)
    {
        auto ud = getUserdata(L, 1);
        if (*ud)
        {
            lua_pushfstring(L, "%s (%p)", metatable, *ud);
        }
        else
        {
            lua_pushfstring(L, "%s (closed)", metatable);
        }
        return 1;
    }

    static const char* const metatable;
};

#endif
