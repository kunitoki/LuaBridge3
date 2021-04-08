// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// Copyright 2020, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#pragma once

#include "detail/Stack.h"

#include <array>

namespace luabridge {

//=================================================================================================
/**
 * @brief Stack specialization for `std::array`.
 */
template <class T, std::size_t Size>
struct Stack<std::array<T, Size>>
{
    using Type = std::array<T, Size>;

    static bool push(lua_State* L, const Type& array, std::error_code& ec)
    {
        const int initialStackSize = lua_gettop(L);
        
        lua_createtable(L, static_cast<int>(Size), 0);

        for (std::size_t i = 0; i < Size; ++i)
        {
            lua_pushinteger(L, static_cast<lua_Integer>(i + 1));

            std::error_code errorCode;
            bool result = Stack<T>::push(L, array[i], errorCode);
            if (!result)
            {
                ec = errorCode;
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
            luaL_error(L, "#%d argment must be a table", index);

        if (get_length(L, index) != Size)
            luaL_error(L, "table size should be %d but is %d", Size, get_length(L, index));

        Type array;

        int absIndex = lua_absindex(L, index);
        lua_pushnil(L);

        int arrayIndex = 0;
        while (lua_next(L, absIndex) != 0)
        {
            array[arrayIndex++] = Stack<T>::get(L, -1);
            lua_pop(L, 1);
        }

        return array;
    }
};

} // namespace luabridge
