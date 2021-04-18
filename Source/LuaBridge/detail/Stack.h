// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// Copyright 2019, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT

#pragma once

#include "LuaHelpers.h"
#include "Userdata.h"

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>

#include <iostream>

namespace luabridge {

//=================================================================================================
/**
 * @brief Lua stack traits for C++ types.
 *
 * @tparam T A C++ type.
 */
template <class T>
struct Stack;

//=================================================================================================
/**
 * @brief Specialization for void type.
 */
template <>
struct Stack<void>
{
    static bool push(lua_State*, std::error_code&)
    {
        return true;
    }
};

//=================================================================================================
/**
 * @brief Specialization for nullptr_t.
 */
template <>
struct Stack<std::nullptr_t>
{
    static bool push(lua_State* L, std::nullptr_t, std::error_code&)
    {
        lua_pushnil(L);
        return true;
    }

    static std::nullptr_t get(lua_State*, int)
    {
        return nullptr;
    }

    static bool isInstance(lua_State* L, int index)
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
    static lua_State* get(lua_State* L, int)
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
    static bool push(lua_State* L, lua_CFunction f, std::error_code&)
    {
        lua_pushcfunction(L, f);
        return true;
    }

    static lua_CFunction get(lua_State* L, int index)
    {
        return lua_tocfunction(L, index);
    }

    static bool isInstance(lua_State* L, int index)
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
    static bool push(lua_State* L, bool value, std::error_code&)
    {
        lua_pushboolean(L, value ? 1 : 0);
        return true;
    }

    static bool get(lua_State* L, int index)
    {
        return lua_toboolean(L, index) ? true : false;
    }

    static bool isInstance(lua_State* L, int index)
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
    static bool push(lua_State* L, std::byte value, std::error_code&)
    {
        pushunsigned(L, std::to_integer<lua_Unsigned>(value));
        return true;
    }

    static std::byte get(lua_State* L, int index)
    {
        return static_cast<std::byte>(luaL_checkinteger(L, index));
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TNUMBER;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `char`.
 */
template <>
struct Stack<char>
{
    static bool push(lua_State* L, char value, std::error_code&)
    {
        lua_pushlstring(L, &value, 1);
        return true;
    }

    static char get(lua_State* L, int index)
    {
        return luaL_checkstring(L, index)[0];
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TSTRING;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `unsigned char`.
 */
template <>
struct Stack<unsigned char>
{
    static bool push(lua_State* L, unsigned char value, std::error_code&)
    {
        pushunsigned(L, value);
        return true;
    }

    static unsigned char get(lua_State* L, int index)
    {
        return static_cast<unsigned char>(luaL_checkinteger(L, index));
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TNUMBER;
    }
};

//=================================================================================================
/**
    Stack specialization for `short`.
*/
template <>
struct Stack<short>
{
    static bool push(lua_State* L, short value, std::error_code&)
    {
        lua_pushinteger(L, static_cast<lua_Integer>(value));
        return true;
    }

    static short get(lua_State* L, int index)
    {
        return static_cast<short>(luaL_checkinteger(L, index));
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TNUMBER;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `unsigned short`.
 */
template <>
struct Stack<unsigned short>
{
    static bool push(lua_State* L, unsigned short value, std::error_code&)
    {
        pushunsigned(L, value);
        return true;
    }

    static unsigned short get(lua_State* L, int index)
    {
        return static_cast<unsigned short>(luaL_checkinteger(L, index));
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TNUMBER;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `int`.
 */
template <>
struct Stack<int>
{
    static bool push(lua_State* L, int value, std::error_code&)
    {
        lua_pushinteger(L, static_cast<lua_Integer>(value));
        return true;
    }

    static int get(lua_State* L, int index)
    {
        return static_cast<int>(luaL_checkinteger(L, index));
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TNUMBER;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `unsigned int`.
 */
template <>
struct Stack<unsigned int>
{
    static bool push(lua_State* L, unsigned int value, std::error_code&)
    {
        pushunsigned(L, value);
        return true;
    }

    static unsigned int get(lua_State* L, int index)
    {
        return static_cast<unsigned int>(luaL_checkinteger(L, index));
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TNUMBER;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `long`.
 */
template <>
struct Stack<long>
{
    static bool push(lua_State* L, long value, std::error_code&)
    {
        lua_pushinteger(L, static_cast<lua_Integer>(value));
        return true;
    }

    static long get(lua_State* L, int index)
    {
        return static_cast<long>(luaL_checkinteger(L, index));
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TNUMBER;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `unsigned long`.
 */
template <>
struct Stack<unsigned long>
{
    static bool push(lua_State* L, unsigned long value, std::error_code&)
    {
        pushunsigned(L, value);
        return true;
    }

    static unsigned long get(lua_State* L, int index)
    {
        return static_cast<unsigned long>(luaL_checkinteger(L, index));
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TNUMBER;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `long long`.
 */
template <>
struct Stack<long long>
{
    static bool push(lua_State* L, long long value, std::error_code&)
    {
        lua_pushinteger(L, static_cast<lua_Integer>(value));
        return true;
    }

    static long long get(lua_State* L, int index)
    {
        return static_cast<long long>(luaL_checkinteger(L, index));
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TNUMBER;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `unsigned long long`.
 */
template <>
struct Stack<unsigned long long>
{
    static bool push(lua_State* L, unsigned long long value, std::error_code&)
    {
        pushunsigned(L, value);
        return true;
    }

    static unsigned long long get(lua_State* L, int index)
    {
        return static_cast<unsigned long long>(luaL_checkinteger(L, index));
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TNUMBER;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `float`.
 */
template <>
struct Stack<float>
{
    static bool push(lua_State* L, float value, std::error_code&)
    {
        lua_pushnumber(L, static_cast<lua_Number>(value));
        return true;
    }

    static float get(lua_State* L, int index)
    {
        return static_cast<float>(luaL_checknumber(L, index));
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TNUMBER;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `double`.
 */
template <>
struct Stack<double>
{
    static bool push(lua_State* L, double value, std::error_code&)
    {
        lua_pushnumber(L, static_cast<lua_Number>(value));
        return true;
    }

    static double get(lua_State* L, int index)
    {
        return static_cast<double>(luaL_checknumber(L, index));
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TNUMBER;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `long double`.
 */
template <>
struct Stack<long double>
{
    static bool push(lua_State* L, long double value, std::error_code&)
    {
        lua_pushnumber(L, static_cast<lua_Number>(value));
        return true;
    }

    static long double get(lua_State* L, int index)
    {
        return static_cast<long double>(luaL_checknumber(L, index));
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TNUMBER;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `const char*`.
 */
template <>
struct Stack<const char*>
{
    static bool push(lua_State* L, const char* str, std::error_code&)
    {
        if (str != nullptr)
            lua_pushstring(L, str);
        else
            lua_pushnil(L);

        return true;
    }

    static const char* get(lua_State* L, int index)
    {
        return lua_isnil(L, index) ? nullptr : luaL_checkstring(L, index);
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_isnil(L, index) || lua_type(L, index) == LUA_TSTRING;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `std::string_view`.
 */
template <>
struct Stack<std::string_view>
{
    static bool push(lua_State* L, std::string_view str, std::error_code&)
    {
        lua_pushlstring(L, str.data(), str.size());
        return true;
    }

    static std::string_view get(lua_State* L, int index)
    {
        return lua_isnil(L, index) ? std::string_view() : luaL_checkstring(L, index);
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_isnil(L, index) || lua_type(L, index) == LUA_TSTRING;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `std::string`.
 */
template <>
struct Stack<std::string>
{
    static bool push(lua_State* L, const std::string& str, std::error_code&)
    {
        lua_pushlstring(L, str.data(), str.size());
        return true;
    }

    static std::string get(lua_State* L, int index)
    {
        std::size_t len;
        if (lua_type(L, index) == LUA_TSTRING)
        {
            const char* str = lua_tolstring(L, index, &len);
            return std::string(str, len);
        }

        // Lua reference manual:
        // If the value is a number, then lua_tolstring also changes the actual value in the stack
        // to a string. (This change confuses lua_next when lua_tolstring is applied to keys during
        // a table traversal.)
        lua_pushvalue(L, index);
        const char* str = lua_tolstring(L, -1, &len);
        std::string string(str, len);
        lua_pop(L, 1); // Pop the temporary string
        return string;
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TSTRING;
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `std::tuple`.
 */
template <class... Types>
struct Stack<std::tuple<Types...>>
{
    static bool push(lua_State* L, const std::tuple<Types...>& t, std::error_code& ec)
    {
        lua_createtable(L, static_cast<int>(Size), 0);

        return push_element(L, t, ec);
    }

    static std::tuple<Types...> get(lua_State* L, int index)
    {
        if (!lua_istable(L, index))
            luaL_error(L, "#%d argment must be a table", index);

        if (get_length(L, index) != Size)
            luaL_error(L, "table size should be %d but is %d", Size, get_length(L, index));

        std::tuple<Types...> value;

        int absIndex = lua_absindex(L, index);
        lua_pushnil(L);

        pop_element(L, absIndex, value);

        return value;
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TTABLE;
    }

private:
    static constexpr std::size_t Size = std::tuple_size_v<std::tuple<Types...>>;

    template <std::size_t Index = 0>
    static auto push_element(lua_State*, const std::tuple<Types...>&, std::error_code&)
        -> std::enable_if_t<Index == sizeof...(Types), bool>
    {
        return true;
    }

    template <std::size_t Index = 0>
    static auto push_element(lua_State* L, const std::tuple<Types...>& t, std::error_code& ec)
        -> std::enable_if_t<Index < sizeof...(Types), bool>
    {
        using T = std::tuple_element_t<Index, std::tuple<Types...>>;

        lua_pushinteger(L, static_cast<lua_Integer>(Index + 1));

        std::error_code push_ec;
        bool result = Stack<T>::push(L, std::get<Index>(t), push_ec);
        if (! result)
        {
            lua_pushnil(L);
            lua_settable(L, -3);
            ec = push_ec;
            return false;
        }

        lua_settable(L, -3);

        return push_element<Index + 1>(L, t, ec);
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

namespace detail {

template <class T>
struct StackOpSelector<T&, false>
{
    using ReturnType = T;

    static bool push(lua_State* L, T& value, std::error_code& ec) { return Stack<T>::push(L, value, ec); }

    static ReturnType get(lua_State* L, int index) { return Stack<T>::get(L, index); }

    static bool isInstance(lua_State* L, int index) { return Stack<T>::isInstance(L, index); }
};

template <class T>
struct StackOpSelector<const T&, false>
{
    using ReturnType = T;

    static bool push(lua_State* L, const T& value, std::error_code& ec) { return Stack<T>::push(L, value, ec); }

    static ReturnType get(lua_State* L, int index) { return Stack<T>::get(L, index); }

    static bool isInstance(lua_State* L, int index) { return Stack<T>::isInstance(L, index); }
};

template <class T>
struct StackOpSelector<T*, false>
{
    using ReturnType = T;

    static bool push(lua_State* L, T* value, std::error_code& ec) { return Stack<T>::push(L, *value, ec); }

    static ReturnType get(lua_State* L, int index) { return Stack<T>::get(L, index); }

    static bool isInstance(lua_State* L, int index) { return Stack<T>::isInstance(L, index); }
};

template <class T>
struct StackOpSelector<const T*, false>
{
    using ReturnType = T;

    static bool push(lua_State* L, const T* value, std::error_code& ec) { return Stack<T>::push(L, *value, ec); }

    static ReturnType get(lua_State* L, int index) { return Stack<T>::get(L, index); }

    static bool isInstance(lua_State* L, int index) { return Stack<T>::isInstance(L, index); }
};

} // namespace detail

template <class T>
struct Stack<T&>
{
    using Helper = detail::StackOpSelector<T&, detail::IsUserdata<T>::value>;
    using ReturnType = typename Helper::ReturnType;

    static bool push(lua_State* L, T& value, std::error_code& ec) { return Helper::push(L, value, ec); }

    static ReturnType get(lua_State* L, int index) { return Helper::get(L, index); }

    static bool isInstance(lua_State* L, int index) { return Helper::template isInstance<T>(L, index); }
};

template <class T>
struct Stack<const T&>
{
    using Helper = detail::StackOpSelector<const T&, detail::IsUserdata<T>::value>;
    using ReturnType = typename Helper::ReturnType;

    static bool push(lua_State* L, const T& value, std::error_code& ec) { return Helper::push(L, value, ec); }

    static ReturnType get(lua_State* L, int index) { return Helper::get(L, index); }

    static bool isInstance(lua_State* L, int index) { return Helper::template isInstance<T>(L, index); }
};

template <class T>
struct Stack<T*>
{
    using Helper = detail::StackOpSelector<T*, detail::IsUserdata<T>::value>;
    using ReturnType = typename Helper::ReturnType;

    static bool push(lua_State* L, T* value, std::error_code& ec) { return Helper::push(L, value, ec); }

    static ReturnType get(lua_State* L, int index) { return Helper::get(L, index); }
    
    static bool isInstance(lua_State* L, int index) { return Helper::template isInstance<T>(L, index); }
};

template<class T>
struct Stack<const T*>
{
    using Helper = detail::StackOpSelector<const T*, detail::IsUserdata<T>::value>;
    using ReturnType = typename Helper::ReturnType;

    static bool push(lua_State* L, const T* value, std::error_code& ec) { return Helper::push(L, value, ec); }

    static ReturnType get(lua_State* L, int index) { return Helper::get(L, index); }

    static bool isInstance(lua_State* L, int index) { return Helper::template isInstance<T>(L, index); }
};

//------------------------------------------------------------------------------
/**
 * @brief Push an object onto the Lua stack.
 */
template <class T>
bool push(lua_State* L, T t, std::error_code& ec)
{
    return Stack<T>::push(L, t, ec);
}

//------------------------------------------------------------------------------
/**
 * @brief Check whether an object on the Lua stack is of type T.
 */
template <class T>
bool isInstance(lua_State* L, int index)
{
    return Stack<T>::isInstance(L, index);
}

} // namespace luabridge
