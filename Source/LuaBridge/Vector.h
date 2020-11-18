// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2018, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#pragma once

#include "detail/Stack.h"

#include <vector>

namespace luabridge {

//-------------------------------------------------------------------------------------------------
/**
 * @brief Stack specialization for `std::vector`.
 */
template <class T>
struct Stack<std::vector<T>>
{
    static void push(lua_State* L, const std::vector<T>& vector)
    {
        lua_createtable(L, static_cast<int>(vector.size()), 0);

        for (std::size_t i = 0; i < vector.size(); ++i)
        {
            lua_pushinteger(L, static_cast<lua_Integer>(i + 1));
            Stack<T>::push(L, vector[i]);
            lua_settable(L, -3);
        }
    }

    static std::vector<T> get(lua_State* L, int index)
    {
        if (!lua_istable(L, index))
            luaL_error(L, "#%d argument must be a table", index);

        std::vector<T> vector;
        vector.reserve(static_cast<std::size_t>(get_length(L, index)));

        int absIndex = lua_absindex(L, index);
        lua_pushnil(L);

        while (lua_next(L, absIndex) != 0)
        {
            vector.emplace_back(Stack<T>::get(L, -1));
            lua_pop(L, 1);
        }

        return vector;
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_istable(L, index);
    }
};

} // namespace luabridge
