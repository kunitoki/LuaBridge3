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
    
    [[nodiscard]] static Result push(lua_State* L, const Type& set)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 3))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        const int initialStackSize = lua_gettop(L);
        
        lua_createtable(L, 0, static_cast<int>(set.size()));

        for (auto it = set.begin(); it != set.end(); ++it)
        {
            auto result = Stack<K>::push(L, it->first);
            if (! result)
            {
                lua_pop(L, lua_gettop(L) - initialStackSize);
                return result;
            }

            result = Stack<V>::push(L, it->second);
            if (! result)
            {
                lua_pop(L, lua_gettop(L) - initialStackSize);
                return result;
            }

            lua_settable(L, -3);
        }
        
        return {};
    }

    [[nodiscard]] static Type get(lua_State* L, int index)
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

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        return lua_istable(L, index);
    }
};

} // namespace luabridge
