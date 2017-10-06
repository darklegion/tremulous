#ifndef __LUA_RAPIDJSION_LUACOMPAT_H__
#define __LUA_RAPIDJSION_LUACOMPAT_H__

#include <cmath>
#include <limits>
#include <cstdint>

#include "qcommon/q3_lauxlib.h"

#include "lua.hpp"

namespace luax
{
    inline void setfuncs(lua_State* L, const luaL_Reg* funcs)
    { luaL_setfuncs(L, funcs, 0); }

    inline size_t rawlen(lua_State* L, int idx)
    { return lua_rawlen(L, idx); }

    inline bool isinteger(lua_State* L, int idx, int64_t* out = nullptr)
    {
        if (lua_isinteger(L, idx)) // but it maybe not detect all integers.
        {
            if (out)
                *out = lua_tointeger(L, idx);
            return true;
        }

        double intpart;
        if (std::modf(lua_tonumber(L, idx), &intpart) == 0.0)
        {
            if (std::numeric_limits<lua_Integer>::min() <= intpart
                    && intpart <= std::numeric_limits<lua_Integer>::max())
            {
                if (out)
                    *out = static_cast<int64_t>(intpart);
                return true;
            }
        }
        return false;
    }

    inline int typerror(lua_State* L, int narg, const char* tname)
    {
        const char *msg = lua_pushfstring(L, "%s expected, got %s", tname, luaL_typename(L, narg));
        return luaL_argerror(L, narg, msg);
    }

    inline bool optboolfield(lua_State* L, int idx, const char* name, bool def)
    {
        auto v = def;
        auto t = lua_type(L, idx);

        if (t != LUA_TTABLE && t != LUA_TNONE)
            luax::typerror(L, idx, "table");

        if (t != LUA_TNONE)
        {
            lua_getfield(L, idx, name);
            if (!lua_isnoneornil(L, -1))
                v = lua_toboolean(L, -1) != 0;
            lua_pop(L, 1);
        }

        return v;
    }

    inline int optintfield(lua_State* L, int idx, const char* name, int def)
    {
        auto v = def;
        lua_getfield(L, idx, name);

        if (lua_isnumber(L, -1))
            v = static_cast<int>(lua_tointeger(L, -1));

        lua_pop(L, 1);
        return v;
    }

}

#endif
