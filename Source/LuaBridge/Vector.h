// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// Copyright 2018, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#pragma once

#include "detail/Stack.h"

#include <vector>

namespace luabridge {

//=================================================================================================
/**
 * @brief Stack specialization for `std::vector`.
 */
template <class T>
struct Stack<std::vector<T>>
{
    using Type = std::vector<T>;

    static bool push(lua_State* L, const Type& vector, std::error_code& ec)
    {
        const int initialStackSize = lua_gettop(L);
        
        lua_createtable(L, static_cast<int>(vector.size()), 0);

        for (std::size_t i = 0; i < vector.size(); ++i)
        {
            lua_pushinteger(L, static_cast<lua_Integer>(i + 1));
            
            std::error_code errorCode;
            if (! Stack<T>::push(L, vector[i], errorCode))
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
            luaL_error(L, "#%d argument must be a table", index);

        Type vector;
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
