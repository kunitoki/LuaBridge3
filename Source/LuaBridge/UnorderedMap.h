// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// Copyright 2019, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#pragma once

#include "detail/Stack.h"

#include <unordered_map>

namespace luabridge {

//=================================================================================================
/**
 * @brief Stack specialization for `std::unordered_map`.
 */
template <class K, class V>
struct Stack<std::unordered_map<K, V>>
{
    using Type = std::unordered_map<K, V>;

    [[nodiscard]] static Result push(lua_State* L, const Type& map)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 3))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        const int initialStackSize = lua_gettop(L);
        
        lua_createtable(L, 0, static_cast<int>(map.size()));

        for (auto it = map.begin(); it != map.end(); ++it)
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

    [[nodiscard]] static Expected<Type, std::error_code> get(lua_State* L, int index)
    {
        const StackRestore stackRestore(L);

        if (!lua_istable(L, index))
            return makeUnexpected(makeErrorCode(ErrorCode::InvalidTypeCast));
            //luaL_error(L, "#%d argument must be a table", index);

        Type map;

        int absIndex = lua_absindex(L, index);
        lua_pushnil(L);

        while (lua_next(L, absIndex) != 0)
        {
            auto value = Stack<V>::get(L, -1);
            if (! value)
                return makeUnexpected(makeErrorCode(ErrorCode::InvalidTypeCast));

            auto key = Stack<K>::get(L, -2);
            if (! key)
                return makeUnexpected(makeErrorCode(ErrorCode::InvalidTypeCast));

            map.emplace(*key, *value);
            lua_pop(L, 1);
        }

        return map;
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        return lua_istable(L, index);
    }
};

} // namespace luabridge
