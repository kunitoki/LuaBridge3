// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2019, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#pragma once

#include "detail/Stack.h"

#include <unordered_map>

namespace luabridge {

//-------------------------------------------------------------------------------------------------
/**
 * @brief Stack specialization for `std::unordered_map`.
 */
template <class K, class V>
struct Stack<std::unordered_map<K, V>>
{
    typedef std::unordered_map<K, V> Map;

    static void push(lua_State* L, const std::unordered_map<K, V>& map)
    {
        lua_createtable(L, 0, static_cast<int>(map.size()));

        for (auto it = map.begin(); it != map.end(); ++it)
        {
            Stack<K>::push(L, it->first);
            Stack<V>::push(L, it->second);
            lua_settable(L, -3);
        }
    }

    static std::unordered_map<K, V> get(lua_State* L, int index)
    {
        if (!lua_istable(L, index))
            luaL_error(L, "#%d argument must be a table", index);

        std::unordered_map<K, V> map;

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
