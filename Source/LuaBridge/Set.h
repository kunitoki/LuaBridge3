// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#pragma once

#include "detail/Stack.h"

#include <set>

namespace luabridge {

//=================================================================================================
/**
 * @brief Stack specialization for `std::set`.
 */
template <class K, class V>
struct Stack<std::set<K, V>>
{
    using Type = std::set<K, V>;
    
    static bool push(lua_State* L, const Type& set, std::error_code& ec)
    {
        lua_createtable(L, 0, static_cast<int>(set.size()));

        bool result;
        for (auto it = set.begin(); it != set.end(); ++it)
        {
            result = Stack<K>::push(L, it->first);
            if (!result)
                return false; // TODO - must pop ?

            result = Stack<V>::push(L, it->second);
            if (!result)
                return false; // TODO - must pop ?

            lua_settable(L, -3);
        }
        
        return true;
    }

    static Type get(lua_State* L, int index)
    {
        if (!lua_istable(L, index))
            luaL_error(L, "#%d argument must be a table", index);

        Type set;

        int absIndex = lua_absindex(L, index);
        lua_pushnil(L);

        while (lua_next(L, absIndex) != 0)
        {
            set.emplace(Stack<K>::get(L, -2), Stack<V>::get(L, -1));
            lua_pop(L, 1);
        }

        return set;
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_istable(L, index);
    }
};

} // namespace luabridge
