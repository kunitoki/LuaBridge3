// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// Copyright 2020, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#pragma once

#include "detail/Stack.h"

#include <list>

namespace luabridge {

//=================================================================================================
/**
 * @brief Stack specialization for `std::array`.
 */
template <class T>
struct Stack<std::list<T>>
{
    using Type = std::list<T>;
    
    static bool push(lua_State* L, const Type& list, std::error_code& ec)
    {
        const int initialStackSize = lua_gettop(L);

        lua_createtable(L, static_cast<int>(list.size()), 0);

        auto it = list.cbegin();
        for (lua_Integer tableIndex = 1; it != list.cend(); ++tableIndex, ++it)
        {
            lua_pushinteger(L, tableIndex);

            std::error_code errorCode;
            if (! Stack<T>::push(L, *it, errorCode))
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

        Type list;

        int absIndex = lua_absindex(L, index);
        lua_pushnil(L);

        while (lua_next(L, absIndex) != 0)
        {
            list.emplace_back(Stack<T>::get(L, -1));
            lua_pop(L, 1);
        }

        return list;
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_istable(L, index);
    }
};

} // namespace luabridge
