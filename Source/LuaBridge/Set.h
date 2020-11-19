// https://github.com/kunitoki/LuaBridge
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
    static void push(lua_State* L, const std::set<K, V>& set)
    {
        lua_createtable(L, 0, static_cast<int>(set.size()));

        for (auto it = set.begin(); it != set.end(); ++it)
        {
            Stack<K>::push(L, it->first);
            Stack<V>::push(L, it->second);
            lua_settable(L, -3);
        }
    }

    static std::set<K, V> get(lua_State* L, int index)
    {
        if (!lua_istable(L, index))
            luaL_error(L, "#%d argument must be a table", index);

        std::set<K, V> set;

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
