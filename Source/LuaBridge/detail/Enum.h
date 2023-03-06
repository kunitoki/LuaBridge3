// https://github.com/kunitoki/LuaBridge3
// Copyright 2023, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#pragma once

#include "Config.h"
#include "LuaHelpers.h"
#include "Stack.h"

#include <type_traits>

namespace luabridge {

//=================================================================================================
/**
 * @brief LuaBridge enum wrapper for enums as integers.
 *
 * Use this when you need maximum speed and could sacrifice safety. An enum exposed with this class will just be decayed to lua
 * as integer. It's responsibility of the developer to make sure that a lua integer could be converted back to C++. Failing to validate a lua
 * integer before converting to the corresponding C++ enum value could lead to a C++ enum that has no defined value.
 *
 * Example of usage for exporting an enum from C++ to lua:
 *
 * @code
 *
 *   enum class MyEnum
 *   {
 *      A,
 *      B,
 *      C
 *   };
 *
 *   template <>
 *   struct luabridge::Stack<EnumType> : luabridge::Enum<EnumType,
 *                                                       EnumType::A,
 *                                                       EnumType::B,
 *                                                       EnumType::C>
 *   {
 *   };
 *
 * @endcode
 */
template <class T, T... Values>
struct Enum
{
    static_assert(std::is_enum_v<T>);

    using Type = std::underlying_type_t<T>;

    [[nodiscard]] static Result push(lua_State* L, T value)
    {
        return Stack<Type>::push(L, static_cast<Type>(value));
    }

    [[nodiscard]] static TypeResult<T> get(lua_State* L, int index)
    {
        const auto result = Stack<Type>::get(L, index);
        if (! result)
            return result.error();

        constexpr Type values[] = { static_cast<Type>(Values)... };
        for (std::size_t i = 0; i < sizeof...(Values); ++i)
        {
            if (values[i] == *result)
                return static_cast<T>(*result);
        }

        return makeErrorCode(ErrorCode::InvalidTypeCast);
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TNUMBER;
    }
};

} // namespace luabridge
