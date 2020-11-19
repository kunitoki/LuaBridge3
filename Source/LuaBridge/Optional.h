// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#pragma once

#include "detail/Stack.h"

#include <optional>

namespace luabridge {

//=================================================================================================
/**
 * @brief Stack specialization for `std::optional`.
 */
template <class T>
struct Stack<std::optional<T>>
{
    static void push(lua_State* L, const std::optional<T>& value)
    {
        if (value)
            Stack<T>::push(L, *value);
        else
            lua_pushnil(L);
    }

    static std::optional<T> get(lua_State* L, int index)
    {
        if (lua_type(L, index) == LUA_TNIL)
            return std::nullopt;
        
        return Stack<T>::get(L, index);
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_isnil(L, index) || Stack<T>::isInstance(L, index);
    }
};

} // namespace luabridge
