// https://github.com/kunitoki/LuaBridge3
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
    using Type = std::optional<T>;
    
    static bool push(lua_State* L, const Type& value, std::error_code& ec)
    {
        if (value)
            return Stack<T>::push(L, *value, ec);

        lua_pushnil(L);
        return true;
    }

    static Type get(lua_State* L, int index)
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
