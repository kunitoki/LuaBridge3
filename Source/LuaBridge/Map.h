// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// Copyright 2018, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#pragma once

#include "detail/Stack.h"

#include <map>

namespace luabridge {

//=================================================================================================
/**
 * @brief Stack specialization for `std::map`.
 */
template <class K, class V>
struct Stack<std::map<K, V>>
{
    using Type = std::map<K, V>;

    static bool push(lua_State* L, const Type& map, std::error_code& ec)
    {
        const int initialStackSize = lua_gettop(L);

        lua_createtable(L, 0, static_cast<int>(map.size()));

        for (auto it = map.begin(); it != map.end(); ++it)
        {
            std::error_code errorCodeKey;
            if (! Stack<K>::push(L, it->first, errorCodeKey))
            {
                ec = errorCodeKey;
                lua_pop(L, lua_gettop(L) - initialStackSize);
                return false;
            }

            std::error_code errorCodeValue;
            if (! Stack<V>::push(L, it->second, errorCodeValue))
            {
                ec = errorCodeValue;
                lua_pop(L, lua_gettop(L) - initialStackSize);
                return false;
            }

            lua_settable(L, -3);
        }
        
        return true;
    }

    static Type get(lua_State* L, int index)
    {
        if (!lua_istable(L, index))
            luaL_error(L, "#%d argument must be a table", index);

        Type map;

        int absIndex = lua_absindex(L, index);
        lua_pushnil(L);

        while (lua_next(L, absIndex) != 0)
        {
            map.emplace(Stack<K>::get(L, -2), Stack<V>::get(L, -1));
            lua_pop(L, 1);
        }

        return map;
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_istable(L, index);
    }
};

} // namespace luabridge
