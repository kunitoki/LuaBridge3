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
 *   template <>
 *   struct luabridge::Stack<EnumType> : luabridge::Enum<EnumType>
 *   {
 *   };
 *
 * @endcode
 */
template <class T>
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

        return static_cast<T>(*result);
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TNUMBER;
    }
};

//=================================================================================================
/**
 * @brief LuaBridge enum wrapper for enums as userdata.
 *
 * Use this when you need maximum safety and could sacrifice speed. An enum exposed with this class will retain its own userdata
 * and trying to construct a C++ enum from an integer will fail..
 *
 * Example of usage for exporting an enum from C++ to lua:
 *
 * @code
 *
 *   template <>
 *   struct luabridge::Stack<EnumType> : luabridge::EnumType<EnumType>
 *   {
 *   };
 *
 * @endcode
 */

template <class T>
struct EnumType
{
    static_assert(std::is_enum_v<T>);

    [[nodiscard]] static Result push(lua_State* L, T value)
    {
        lua_newuserdata_aligned<T>(L, value);

        luaL_newmetatable(L, typeName());
        lua_setmetatable(L, -2);

        return {};
    }

    [[nodiscard]] static TypeResult<T> get(lua_State* L, int index)
    {
        auto ptr = luaL_testudata(L, index, typeName());
        if (ptr == nullptr)
            return makeErrorCode(ErrorCode::InvalidTypeCast);

        auto reference = reinterpret_cast<T*>(ptr);
        if (reference == nullptr)
            return makeErrorCode(ErrorCode::InvalidTypeCast);

        return *reference;
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        return luaL_testudata(L, index, typeName()) != nullptr;
    }

private:
    static const char* typeName()
    {
        static const std::string s{ detail::typeName<T>() };
        return s.c_str();
    }
};

} // namespace luabridge
