// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// Copyright 2019, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT

#pragma once

#include "LuaHelpers.h"
#include "Errors.h"
#include "Result.h"
#include "Userdata.h"

#include <any>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <tuple>

namespace luabridge {

//=================================================================================================
/**
 * @brief Lua stack traits for C++ types.
 *
 * @tparam T A C++ type.
 */
template <class T, class>
struct Stack;

//=================================================================================================
/**
 * @brief Specialization for void type.
 */
template <>
struct Stack<void>
{
    [[nodiscard]] static Result push(lua_State*)
    {
        return {};
    }
};

//=================================================================================================
/**
 * @brief Specialization for nullptr_t.
 */
template <>
struct Stack<std::nullptr_t>
{
    [[nodiscard]] static Result push(lua_State* L, std::nullptr_t)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        lua_pushnil(L);
        return {};
    }

    [[nodiscard]] static std::nullptr_t get(lua_State*, int)
    {
        return nullptr;
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        return lua_isnil(L, index);
    }
};

//=================================================================================================
/**
 * @brief Receive the lua_State* as an argument.
 */
template <>
struct Stack<lua_State*>
{
    [[nodiscard]] static lua_State* get(lua_State* L, int)
    {
        return L;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for a lua_CFunction.
 */
template <>
struct Stack<lua_CFunction>
{
    [[nodiscard]] static Result push(lua_State* L, lua_CFunction f)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        lua_pushcfunction_x(L, f);
        return {};
    }

    [[nodiscard]] static lua_CFunction get(lua_State* L, int index)
    {
        return lua_tocfunction(L, index);
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        return lua_iscfunction(L, index);
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `bool`.
 */
template <>
struct Stack<bool>
{
    [[nodiscard]] static Result push(lua_State* L, bool value)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        lua_pushboolean(L, value ? 1 : 0);
        return {};
    }

    [[nodiscard]] static bool get(lua_State* L, int index)
    {
        return lua_toboolean(L, index) ? true : false;
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        return lua_isboolean(L, index);
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `std::byte`.
 */
template <>
struct Stack<std::byte>
{
    static_assert(sizeof(std::byte) < sizeof(lua_Integer));

    [[nodiscard]] static Result push(lua_State* L, std::byte value)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        pushunsigned(L, std::to_integer<std::make_unsigned_t<lua_Integer>>(value));
        return {};
    }

    [[nodiscard]] static std::byte get(lua_State* L, int index)
    {
        return static_cast<std::byte>(luaL_checkinteger(L, index));
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        if (lua_type(L, index) == LUA_TNUMBER)
            return is_integral_representable_by<unsigned char>(L, index);

        return false;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `char`.
 */
template <>
struct Stack<char>
{
    [[nodiscard]] static Result push(lua_State* L, char value)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        lua_pushlstring(L, &value, 1);
        return {};
    }

    [[nodiscard]] static char get(lua_State* L, int index)
    {
        if (lua_type(L, index) == LUA_TSTRING)
        {
            std::size_t length = 0;
            const char* str = lua_tolstring(L, index, &length);

            if (str != nullptr && length >= 1)
                return str[0];
        }

        return char(0);
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        if (lua_type(L, index) == LUA_TSTRING)
        {
            std::size_t len;
            luaL_checklstring(L, index, &len);
            return len == 1;
        }

        return false;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `int8_t`.
 */
template <>
struct Stack<int8_t>
{
    static_assert(sizeof(int8_t) < sizeof(lua_Integer));

    [[nodiscard]] static Result push(lua_State* L, int8_t value)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        lua_pushinteger(L, static_cast<lua_Integer>(value));
        return {};
    }

    [[nodiscard]] static int8_t get(lua_State* L, int index)
    {
        return static_cast<int8_t>(luaL_checkinteger(L, index));
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        if (lua_type(L, index) == LUA_TNUMBER)
            return is_integral_representable_by<int8_t>(L, index);

        return false;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `unsigned char`.
 */
template <>
struct Stack<unsigned char>
{
    static_assert(sizeof(unsigned char) < sizeof(lua_Integer));

    [[nodiscard]] static Result push(lua_State* L, unsigned char value)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        pushunsigned(L, value);
        return {};
    }

    [[nodiscard]] static unsigned char get(lua_State* L, int index)
    {
        return static_cast<unsigned char>(luaL_checkinteger(L, index));
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        if (lua_type(L, index) == LUA_TNUMBER)
            return is_integral_representable_by<unsigned char>(L, index);

        return false;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `short`.
 */
template <>
struct Stack<short>
{
    static_assert(sizeof(short) < sizeof(lua_Integer));

    [[nodiscard]] static Result push(lua_State* L, short value)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        lua_pushinteger(L, static_cast<lua_Integer>(value));
        return {};
    }

    [[nodiscard]] static short get(lua_State* L, int index)
    {
        return static_cast<short>(luaL_checkinteger(L, index));
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        if (lua_type(L, index) == LUA_TNUMBER)
            return is_integral_representable_by<short>(L, index);

        return false;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `unsigned short`.
 */
template <>
struct Stack<unsigned short>
{
    static_assert(sizeof(unsigned short) < sizeof(lua_Integer));

    [[nodiscard]] static Result push(lua_State* L, unsigned short value)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        pushunsigned(L, value);
        return {};
    }

    [[nodiscard]] static unsigned short get(lua_State* L, int index)
    {
        return static_cast<unsigned short>(luaL_checkinteger(L, index));
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        if (lua_type(L, index) == LUA_TNUMBER)
            return is_integral_representable_by<unsigned short>(L, index);

        return false;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `int`.
 */
template <>
struct Stack<int>
{
    [[nodiscard]] static Result push(lua_State* L, int value)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        if (! is_integral_representable_by(value))
            return makeErrorCode(ErrorCode::IntegerDoesntFitIntoLuaInteger);

        lua_pushinteger(L, static_cast<lua_Integer>(value));
        return {};
    }

    [[nodiscard]] static int get(lua_State* L, int index)
    {
        return static_cast<int>(luaL_checkinteger(L, index));
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        if (lua_type(L, index) == LUA_TNUMBER)
            return is_integral_representable_by<int>(L, index);

        return false;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `unsigned int`.
 */
template <>
struct Stack<unsigned int>
{
    [[nodiscard]] static Result push(lua_State* L, unsigned int value)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        if (! is_integral_representable_by(value))
            return makeErrorCode(ErrorCode::IntegerDoesntFitIntoLuaInteger);

        pushunsigned(L, value);
        return {};
    }

    [[nodiscard]] static uint32_t get(lua_State* L, int index)
    {
        return static_cast<unsigned int>(luaL_checkinteger(L, index));
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        if (lua_type(L, index) == LUA_TNUMBER)
            return is_integral_representable_by<unsigned int>(L, index);

        return false;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `long`.
 */
template <>
struct Stack<long>
{
    [[nodiscard]] static Result push(lua_State* L, long value)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        if (! is_integral_representable_by(value))
            return makeErrorCode(ErrorCode::IntegerDoesntFitIntoLuaInteger);

        lua_pushinteger(L, static_cast<lua_Integer>(value));
        return {};
    }

    [[nodiscard]] static long get(lua_State* L, int index)
    {
        return static_cast<long>(luaL_checkinteger(L, index));
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        if (lua_type(L, index) == LUA_TNUMBER)
            return is_integral_representable_by<long>(L, index);

        return false;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `unsigned long`.
 */
template <>
struct Stack<unsigned long>
{
    [[nodiscard]] static Result push(lua_State* L, unsigned long value)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        if (! is_integral_representable_by(value))
            return makeErrorCode(ErrorCode::IntegerDoesntFitIntoLuaInteger);

        pushunsigned(L, value);
        return {};
    }

    [[nodiscard]] static unsigned long get(lua_State* L, int index)
    {
        return static_cast<unsigned long>(luaL_checkinteger(L, index));
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        if (lua_type(L, index) == LUA_TNUMBER)
            return is_integral_representable_by<unsigned long>(L, index);

        return false;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `long long`.
 */
template <>
struct Stack<long long>
{
    [[nodiscard]] static Result push(lua_State* L, long long value)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        if (! is_integral_representable_by(value))
            return makeErrorCode(ErrorCode::IntegerDoesntFitIntoLuaInteger);

        lua_pushinteger(L, static_cast<lua_Integer>(value));
        return {};
    }

    [[nodiscard]] static long long get(lua_State* L, int index)
    {
        return static_cast<long long>(luaL_checkinteger(L, index));
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        if (lua_type(L, index) == LUA_TNUMBER)
            return is_integral_representable_by<long long>(L, index);

        return false;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `unsigned long long`.
 */
template <>
struct Stack<unsigned long long>
{
    [[nodiscard]] static Result push(lua_State* L, unsigned long long value)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        if (! is_integral_representable_by(value))
            return makeErrorCode(ErrorCode::IntegerDoesntFitIntoLuaInteger);

        pushunsigned(L, value);
        return {};
    }

    [[nodiscard]] static unsigned long long get(lua_State* L, int index)
    {
        return static_cast<unsigned long long>(luaL_checkinteger(L, index));
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        if (lua_type(L, index) == LUA_TNUMBER)
            return is_integral_representable_by<unsigned long long>(L, index);

        return false;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `float`.
 */
template <>
struct Stack<float>
{
    [[nodiscard]] static Result push(lua_State* L, float value)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        if (! is_floating_point_representable_by(value))
            return makeErrorCode(ErrorCode::FloatingPointDoesntFitIntoLuaNumber);

        lua_pushnumber(L, static_cast<lua_Number>(value));
        return {};
    }

    [[nodiscard]] static float get(lua_State* L, int index)
    {
        return static_cast<float>(luaL_checknumber(L, index));
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        if (lua_type(L, index) == LUA_TNUMBER)
            return is_floating_point_representable_by<float>(L, index);

        return false;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `double`.
 */
template <>
struct Stack<double>
{
    [[nodiscard]] static Result push(lua_State* L, double value)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        if (! is_floating_point_representable_by(value))
            return makeErrorCode(ErrorCode::FloatingPointDoesntFitIntoLuaNumber);

        lua_pushnumber(L, static_cast<lua_Number>(value));
        return {};
    }

    [[nodiscard]] static double get(lua_State* L, int index)
    {
        return static_cast<double>(luaL_checknumber(L, index));
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        if (lua_type(L, index) == LUA_TNUMBER)
            return is_floating_point_representable_by<double>(L, index);

        return false;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `long double`.
 */
template <>
struct Stack<long double>
{
    [[nodiscard]] static Result push(lua_State* L, long double value)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        if (! is_floating_point_representable_by(value))
            return makeErrorCode(ErrorCode::FloatingPointDoesntFitIntoLuaNumber);

        lua_pushnumber(L, static_cast<lua_Number>(value));
        return {};
    }

    [[nodiscard]] static long double get(lua_State* L, int index)
    {
        return static_cast<long double>(luaL_checknumber(L, index));
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        if (lua_type(L, index) == LUA_TNUMBER)
            return is_floating_point_representable_by<long double>(L, index);

        return false;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `const char*`.
 */
template <>
struct Stack<const char*>
{
    [[nodiscard]] static Result push(lua_State* L, const char* str)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        if (str != nullptr)
            lua_pushstring(L, str);
        else
            lua_pushlstring(L, "", 0);

        return {};
    }

    [[nodiscard]] static const char* get(lua_State* L, int index)
    {
        if (lua_type(L, index) == LUA_TSTRING)
        {
            std::size_t length = 0;
            const char* str = lua_tolstring(L, index, &length);

            if (str != nullptr)
                return str;
        }

        return "";
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TSTRING;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `std::string_view`.
 */
template <>
struct Stack<std::string_view>
{
    [[nodiscard]] static Result push(lua_State* L, std::string_view str)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        lua_pushlstring(L, str.data(), str.size());
        return {};
    }

    [[nodiscard]] static std::string_view get(lua_State* L, int index)
    {
        if (lua_type(L, index) == LUA_TSTRING)
        {
            std::size_t length = 0;
            const char* str = lua_tolstring(L, index, &length);

            if (str != nullptr)
                return { str, length };
        }

        return {};
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TSTRING;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `std::string`.
 */
template <>
struct Stack<std::string>
{
    [[nodiscard]] static Result push(lua_State* L, const std::string& str)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        lua_pushlstring(L, str.data(), str.size());
        return {};
    }

    [[nodiscard]] static std::string get(lua_State* L, int index)
    {
        std::size_t length = 0;

        if (lua_type(L, index) == LUA_TSTRING)
        {
            const char* str = lua_tolstring(L, index, &length);

            if (str != nullptr)
                return { str, length };
        }

        // Lua reference manual:
        // If the value is a number, then lua_tolstring also changes the actual value in the stack
        // to a string. (This change confuses lua_next when lua_tolstring is applied to keys during
        // a table traversal)
        lua_pushvalue(L, index);
        const char* str = lua_tolstring(L, -1, &length);
        lua_pop(L, 1);

        if (str != nullptr)
            return { str, length };

        return {};
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TSTRING;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `std::optional`.
 */
template <class T>
struct Stack<std::optional<T>>
{
    using Type = std::optional<T>;

    [[nodiscard]] static Result push(lua_State* L, const Type& value)
    {
        if (value)
            return Stack<T>::push(L, *value);

#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        lua_pushnil(L);
        return {};
    }

    [[nodiscard]] static Type get(lua_State* L, int index)
    {
        if (lua_type(L, index) == LUA_TNIL)
            return std::nullopt;

        return Stack<T>::get(L, index);
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        return lua_isnil(L, index) || Stack<T>::isInstance(L, index);
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `std::any`.
 */
template <>
struct Stack<std::any>
{
    [[nodiscard]] static Result push(lua_State* L, const std::any& value)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 1))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        lua_newuserdata_aligned<std::any>(L, value);
        return {};
    }

    [[nodiscard]] static std::any get(lua_State* L, int index)
    {
        if (lua_type(L, index) != LUA_TUSERDATA)
            return {};

        auto any = static_cast<std::any*>(lua_touserdata(L, index));
        if (any == nullptr)
            return {};

        return *any;
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        return lua_isuserdata(L, index);
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `std::tuple`.
 */
template <class... Types>
struct Stack<std::tuple<Types...>>
{
    [[nodiscard]] static Result push(lua_State* L, const std::tuple<Types...>& t)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 3))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        lua_createtable(L, static_cast<int>(Size), 0);

        return push_element(L, t);
    }

    [[nodiscard]] static std::tuple<Types...> get(lua_State* L, int index)
    {
        if (!lua_istable(L, index))
            luaL_error(L, "#%d argment must be a table", index);

        if (get_length(L, index) != static_cast<int>(Size))
            luaL_error(L, "table size should be %d but is %d", static_cast<unsigned>(Size), get_length(L, index));

        std::tuple<Types...> value;

        int absIndex = lua_absindex(L, index);
        lua_pushnil(L);

        pop_element(L, absIndex, value);

        return value;
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TTABLE && get_length(L, index) == static_cast<int>(Size);
    }

private:
    static constexpr std::size_t Size = std::tuple_size_v<std::tuple<Types...>>;

    template <std::size_t Index = 0>
    static auto push_element(lua_State*, const std::tuple<Types...>&)
        -> std::enable_if_t<Index == sizeof...(Types), Result>
    {
        return {};
    }

    template <std::size_t Index = 0>
    static auto push_element(lua_State* L, const std::tuple<Types...>& t)
        -> std::enable_if_t<Index < sizeof...(Types), Result>
    {
        using T = std::tuple_element_t<Index, std::tuple<Types...>>;

        lua_pushinteger(L, static_cast<lua_Integer>(Index + 1));

        Result r = Stack<T>::push(L, std::get<Index>(t));
        if (!r)
        {
            lua_pushnil(L);
            lua_settable(L, -3);
            return r;
        }

        lua_settable(L, -3);

        return push_element<Index + 1>(L, t);
    }

    template <std::size_t Index = 0>
    static auto pop_element(lua_State*, int, std::tuple<Types...>&)
        -> std::enable_if_t<Index == sizeof...(Types)>
    {
    }

    template <std::size_t Index = 0>
    static auto pop_element(lua_State* L, int absIndex, std::tuple<Types...>& t)
        -> std::enable_if_t<Index < sizeof...(Types)>
    {
        using T = std::tuple_element_t<Index, std::tuple<Types...>>;

        if (lua_next(L, absIndex) == 0)
            return;

        std::get<Index>(t) = Stack<T>::get(L, -1);
        lua_pop(L, 1);

        pop_element<Index + 1>(L, absIndex, t);
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `T[N]`.
 */
template <class T, std::size_t N>
struct Stack<T[N]>
{
    static_assert(N > 0, "Unsupported zero sized array");

    [[nodiscard]] static Result push(lua_State* L, const T (&value)[N])
    {
        if constexpr (std::is_same_v<T, char>)
        {
#if LUABRIDGE_SAFE_STACK_CHECKS
            if (! lua_checkstack(L, 1))
                return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

            lua_pushlstring(L, value, N - 1);
            return {};
        }

#if LUABRIDGE_SAFE_STACK_CHECKS
        if (! lua_checkstack(L, 2))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        const int initialStackSize = lua_gettop(L);

        lua_createtable(L, static_cast<int>(N), 0);

        for (std::size_t i = 0; i < N; ++i)
        {
            lua_pushinteger(L, static_cast<lua_Integer>(i + 1));

            Result r = Stack<T>::push(L, value[i]);
            if (!r)
            {
                lua_pop(L, lua_gettop(L) - initialStackSize);
                return r;
            }

            lua_settable(L, -3);
        }

        return {};
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TTABLE && get_length(L, index) == static_cast<int>(N);
    }
};

namespace detail {

template <class T>
struct StackOpSelector<T&, false>
{
    using ReturnType = T;

    static Result push(lua_State* L, T& value) { return Stack<T>::push(L, value); }

    static ReturnType get(lua_State* L, int index) { return Stack<T>::get(L, index); }

    static bool isInstance(lua_State* L, int index) { return Stack<T>::isInstance(L, index); }
};

template <class T>
struct StackOpSelector<const T&, false>
{
    using ReturnType = T;

    static Result push(lua_State* L, const T& value) { return Stack<T>::push(L, value); }

    static auto get(lua_State* L, int index) { return Stack<T>::get(L, index); }

    static bool isInstance(lua_State* L, int index) { return Stack<T>::isInstance(L, index); }
};

template <class T>
struct StackOpSelector<T*, false>
{
    using ReturnType = T;

    static Result push(lua_State* L, T* value) { return Stack<T>::push(L, *value); }

    static ReturnType get(lua_State* L, int index) { return Stack<T>::get(L, index); }

    static bool isInstance(lua_State* L, int index) { return Stack<T>::isInstance(L, index); }
};

template <class T>
struct StackOpSelector<const T*, false>
{
    using ReturnType = T;

    static Result push(lua_State* L, const T* value) { return Stack<T>::push(L, *value); }

    static ReturnType get(lua_State* L, int index) { return Stack<T>::get(L, index); }

    static bool isInstance(lua_State* L, int index) { return Stack<T>::isInstance(L, index); }
};

} // namespace detail

template <class T>
struct Stack<T&, std::enable_if_t<!std::is_array_v<T&>>>
{
    using Helper = detail::StackOpSelector<T&, detail::IsUserdata<T>::value>;
    using ReturnType = typename Helper::ReturnType;

    [[nodiscard]] static Result push(lua_State* L, T& value) { return Helper::push(L, value); }

    [[nodiscard]] static ReturnType get(lua_State* L, int index) { return Helper::get(L, index); }

    [[nodiscard]] static bool isInstance(lua_State* L, int index) { return Helper::template isInstance<T>(L, index); }
};

template <class T>
struct Stack<const T&, std::enable_if_t<!std::is_array_v<const T&>>>
{
    using Helper = detail::StackOpSelector<const T&, detail::IsUserdata<T>::value>;
    using ReturnType = typename Helper::ReturnType;

    [[nodiscard]] static Result push(lua_State* L, const T& value) { return Helper::push(L, value); }

    [[nodiscard]] static auto get(lua_State* L, int index) { return Helper::get(L, index); }

    [[nodiscard]] static bool isInstance(lua_State* L, int index) { return Helper::template isInstance<T>(L, index); }
};

template <class T>
struct Stack<T*>
{
    using Helper = detail::StackOpSelector<T*, detail::IsUserdata<T>::value>;
    using ReturnType = typename Helper::ReturnType;

    [[nodiscard]] static Result push(lua_State* L, T* value) { return Helper::push(L, value); }

    [[nodiscard]] static ReturnType get(lua_State* L, int index) { return Helper::get(L, index); }

    [[nodiscard]] static bool isInstance(lua_State* L, int index) { return Helper::template isInstance<T>(L, index); }
};

template<class T>
struct Stack<const T*>
{
    using Helper = detail::StackOpSelector<const T*, detail::IsUserdata<T>::value>;
    using ReturnType = typename Helper::ReturnType;

    [[nodiscard]] static Result push(lua_State* L, const T* value) { return Helper::push(L, value); }

    [[nodiscard]] static ReturnType get(lua_State* L, int index) { return Helper::get(L, index); }

    [[nodiscard]] static bool isInstance(lua_State* L, int index) { return Helper::template isInstance<T>(L, index); }
};

//=================================================================================================
/**
 * @brief Push an object onto the Lua stack.
 */
template <class T>
[[nodiscard]] Result push(lua_State* L, const T& t)
{
    return Stack<T>::push(L, t);
}

//=================================================================================================
/**
 * @brief Get an object from the Lua stack.
 */
template <class T>
[[nodiscard]] T get(lua_State* L, int index)
{
    return Stack<T>::get(L, index);
}

//=================================================================================================
/**
 * @brief Check whether an object on the Lua stack is of type T.
 */
template <class T>
[[nodiscard]] bool isInstance(lua_State* L, int index)
{
    return Stack<T>::isInstance(L, index);
}

//=================================================================================================
/**
 * @brief Stack restorer.
 */
class StackRestore final
{
public:
    StackRestore(lua_State* L)
        : m_L(L)
        , m_stackTop(lua_gettop(L))
    {
    }

    ~StackRestore()
    {
        lua_settop(m_L, m_stackTop);
    }

private:
    lua_State* const m_L = nullptr;
    int m_stackTop = 0;
};

} // namespace luabridge
