// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT

#pragma once

#include "Config.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <type_traits>
#include <utility>

namespace luabridge {

/**
 * @brief Helper for unused vars.
 */
template <class... Args>
constexpr void unused(Args&&...)
{
}

// These functions and defines are for Luau.
#if LUABRIDGE_ON_LUAU
inline int luaL_ref(lua_State* L, int idx)
{
    assert(idx == LUA_REGISTRYINDEX);

    const int ref = lua_ref(L, -1);

    lua_pop(L, 1);

    return ref;
}

inline void luaL_unref(lua_State* L, int idx, int ref)
{
    unused(idx);

    lua_unref(L, ref);
}

template <class T>
inline void* lua_newuserdata_x(lua_State* L, size_t sz)
{
    return lua_newuserdatadtor(L, sz, [](void* x)
    {
        T* object = static_cast<T*>(x);
        object->~T();
    });
}

inline void lua_pushcfunction_x(lua_State *L, lua_CFunction fn)
{
    lua_pushcfunction(L, fn, "");
}

inline void lua_pushcclosure_x(lua_State* L, lua_CFunction fn, int n)
{
    lua_pushcclosure(L, fn, "", n);
}

#else
using ::luaL_ref;
using ::luaL_unref;

template <class T>
inline void* lua_newuserdata_x(lua_State* L, size_t sz)
{
    return lua_newuserdata(L, sz);
}

inline void lua_pushcfunction_x(lua_State *L, lua_CFunction fn)
{
    lua_pushcfunction(L, fn);
}

inline void lua_pushcclosure_x(lua_State* L, lua_CFunction fn, int n)
{
    lua_pushcclosure(L, fn, n);
}

#endif // LUABRIDGE_ON_LUAU

// These are for Lua versions prior to 5.3.0.
#if LUA_VERSION_NUM < 503
inline lua_Number to_numberx(lua_State* L, int idx, int* isnum)
{
    lua_Number n = lua_tonumber(L, idx);

    if (isnum)
        *isnum = (n != 0 || lua_isnumber(L, idx));

    return n;
}

inline lua_Integer to_integerx(lua_State* L, int idx, int* isnum)
{
    int ok = 0;
    lua_Number n = to_numberx(L, idx, &ok);

    if (ok)
    {
        const auto int_n = static_cast<lua_Integer>(n);
        if (n == static_cast<lua_Number>(int_n))
        {
            if (isnum)
                *isnum = 1;
            
            return int_n;
        }
    }

    if (isnum)
        *isnum = 0;
    
    return 0;
}

#endif // LUA_VERSION_NUM < 503

// These are for Lua versions prior to 5.2.0.
#if LUA_VERSION_NUM < 502
using lua_Unsigned = std::make_unsigned_t<lua_Integer>;

#if ! LUABRIDGE_ON_LUAU
inline int lua_absindex(lua_State* L, int idx)
{
    if (idx > LUA_REGISTRYINDEX && idx < 0)
        return lua_gettop(L) + idx + 1;
    else
        return idx;
}
#endif

inline void lua_rawgetp(lua_State* L, int idx, const void* p)
{
    idx = lua_absindex(L, idx);
    luaL_checkstack(L, 1, "not enough stack slots");
    lua_pushlightuserdata(L, const_cast<void*>(p));
    lua_rawget(L, idx);
}

inline void lua_rawsetp(lua_State* L, int idx, const void* p)
{
    idx = lua_absindex(L, idx);
    luaL_checkstack(L, 1, "not enough stack slots");
    lua_pushlightuserdata(L, const_cast<void*>(p));
    lua_insert(L, -2);
    lua_rawset(L, idx);
}

#define LUA_OPEQ 1
#define LUA_OPLT 2
#define LUA_OPLE 3

inline int lua_compare(lua_State* L, int idx1, int idx2, int op)
{
    switch (op)
    {
    case LUA_OPEQ:
        return lua_equal(L, idx1, idx2);

    case LUA_OPLT:
        return lua_lessthan(L, idx1, idx2);

    case LUA_OPLE:
        return lua_equal(L, idx1, idx2) || lua_lessthan(L, idx1, idx2);

    default:
        return 0;
    }
}

inline int get_length(lua_State* L, int idx)
{
    return static_cast<int>(lua_objlen(L, idx));
}

#else // LUA_VERSION_NUM >= 502
inline int get_length(lua_State* L, int idx)
{
    lua_len(L, idx);
    const int len = static_cast<int>(luaL_checknumber(L, -1));
    lua_pop(L, 1);
    return len;
}

#endif // LUA_VERSION_NUM < 502

#ifndef LUA_OK
#define LUABRIDGE_LUA_OK 0
#else
#define LUABRIDGE_LUA_OK LUA_OK
#endif

/**
 * @brief Helper to throw or return an error code.
 */
template <class T, class ErrorType>
std::error_code throw_or_error_code(ErrorType error)
{
#if LUABRIDGE_HAS_EXCEPTIONS
    throw T(makeErrorCode(error).message().c_str());
#else
    return makeErrorCode(error);
#endif
}

template <class T, class ErrorType>
std::error_code throw_or_error_code(lua_State* L, ErrorType error)
{
#if LUABRIDGE_HAS_EXCEPTIONS
    throw T(L, makeErrorCode(error));
#else
    return unused(L), makeErrorCode(error);
#endif
}

/**
 * @brief Helper to throw or assert.
 */
template <class T, class... Args>
void throw_or_assert(Args&&... args)
{
#if LUABRIDGE_HAS_EXCEPTIONS
    throw T(std::forward<Args>(args)...);
#else
    unused(std::forward<Args>(args)...);
    assert(false);
#endif
}

/**
 * @brief Helper to set unsigned.
 */
template <class T>
void pushunsigned(lua_State* L, T value)
{
    static_assert(std::is_unsigned_v<T>);

    lua_pushinteger(L, static_cast<lua_Integer>(value));
}

/**
 * @brief Helper to convert to integer.
 */
inline lua_Number tonumber(lua_State* L, int idx, int* isnum)
{
#if ! LUABRIDGE_ON_LUAU && LUA_VERSION_NUM > 502
    return lua_tonumberx(L, idx, isnum);
#else
    return to_numberx(L, idx, isnum);
#endif
}

/**
 * @brief Helper to convert to integer.
 */
inline lua_Integer tointeger(lua_State* L, int idx, int* isnum)
{
#if ! LUABRIDGE_ON_LUAU && LUA_VERSION_NUM > 502
    return lua_tointegerx(L, idx, isnum);
#else
    return to_integerx(L, idx, isnum);
#endif
}

/**
 * @brief Get a table value, bypassing metamethods.
 */
inline void rawgetfield(lua_State* L, int index, char const* key)
{
    assert(lua_istable(L, index));
    index = lua_absindex(L, index);
    lua_pushstring(L, key);
    lua_rawget(L, index);
}

/**
 * @brief Set a table value, bypassing metamethods.
 */
inline void rawsetfield(lua_State* L, int index, char const* key)
{
    assert(lua_istable(L, index));
    index = lua_absindex(L, index);
    lua_pushstring(L, key);
    lua_insert(L, -2);
    lua_rawset(L, index);
}

/**
 * @brief Returns true if the value is a full userdata (not light).
 */
[[nodiscard]] inline bool isfulluserdata(lua_State* L, int index)
{
    return lua_isuserdata(L, index) && !lua_islightuserdata(L, index);
}

/**
 * @brief Test lua_State objects for global equality.
 *
 * This can determine if two different lua_State objects really point
 * to the same global state, such as when using coroutines.
 *
 * @note This is used for assertions.
 */
[[nodiscard]] inline bool equalstates(lua_State* L1, lua_State* L2)
{
    return lua_topointer(L1, LUA_REGISTRYINDEX) == lua_topointer(L2, LUA_REGISTRYINDEX);
}

/**
 * @brief Return an aligned pointer of type T.
 */
template <class T>
[[nodiscard]] T* align(void* ptr) noexcept
{
    const auto address = reinterpret_cast<size_t>(ptr);

    const auto offset = address % alignof(T);
    const auto aligned_address = (offset == 0) ? address : (address + alignof(T) - offset);

    return reinterpret_cast<T*>(aligned_address);
}

/**
 * @brief Return the space needed to align the type T on an unaligned address.
 */
template <class T>
[[nodiscard]] constexpr size_t maximum_space_needed_to_align() noexcept
{
    return sizeof(T) + alignof(T) - 1;
}

/**
 * @brief Deallocate lua userdata taking into account alignment.
 */
template <class T>
int lua_deleteuserdata_aligned(lua_State* L)
{
    assert(isfulluserdata(L, 1));

    T* aligned = align<T>(lua_touserdata(L, 1));
    aligned->~T();

    return 0;
}

/**
 * @brief Allocate lua userdata taking into account alignment.
 *
 * Using this instead of lua_newuserdata directly prevents alignment warnings on 64bits platforms.
 */
template <class T, class... Args>
void* lua_newuserdata_aligned(lua_State* L, Args&&... args)
{
#if LUABRIDGE_ON_LUAU
    void* pointer = lua_newuserdatadtor(L, maximum_space_needed_to_align<T>(), [](void* x)
    {
        T* aligned = align<T>(x);
        aligned->~T();
    });
#else
    void* pointer = lua_newuserdata_x<T>(L, maximum_space_needed_to_align<T>());

    lua_newtable(L);
    lua_pushcfunction_x(L, &lua_deleteuserdata_aligned<T>);
    rawsetfield(L, -2, "__gc");
    lua_setmetatable(L, -2);
#endif

    T* aligned = align<T>(pointer);

    new (aligned) T(std::forward<Args>(args)...);

    return pointer;
}

/**
 * @brief Checks if the value on the stack is a number type and can fit into the corresponding c++ integral type..
 */
template <class U = lua_Integer, class T>
constexpr bool is_integral_representable_by(T value)
{
    constexpr bool same_signedness = (std::is_unsigned_v<T> && std::is_unsigned_v<U>)
        || (!std::is_unsigned_v<T> && !std::is_unsigned_v<U>);

    if constexpr (sizeof(T) == sizeof(U))
    {
        if constexpr (same_signedness)
            return true;

        if constexpr (std::is_unsigned_v<T>)
            return value <= static_cast<T>(std::numeric_limits<U>::max());
        
        return value >= static_cast<T>(std::numeric_limits<U>::min())
            && static_cast<U>(value) <= std::numeric_limits<U>::max();
    }

    if constexpr (sizeof(T) < sizeof(U))
    {
        return static_cast<U>(value) >= std::numeric_limits<U>::min()
            && static_cast<U>(value) <= std::numeric_limits<U>::max();
    }

    if constexpr (std::is_unsigned_v<T>)
        return value <= static_cast<T>(std::numeric_limits<U>::max());

    return value >= static_cast<T>(std::numeric_limits<U>::min())
        && value <= static_cast<T>(std::numeric_limits<U>::max());
}

template <class U = lua_Integer>
bool is_integral_representable_by(lua_State* L, int index)
{
    int isValid = 0;

    const auto value = tointeger(L, index, &isValid);

    return isValid ? is_integral_representable_by<U>(value) : false;
}

/**
 * @brief Checks if the value on the stack is a number type and can fit into the corresponding c++ numerical type..
 */
template <class U = lua_Number, class T>
constexpr bool is_floating_point_representable_by(T value)
{
    if constexpr (sizeof(T) == sizeof(U))
        return true;

    if constexpr (sizeof(T) < sizeof(U))
        return static_cast<U>(value) >= -std::numeric_limits<U>::max()
            && static_cast<U>(value) <= std::numeric_limits<U>::max();

    return value >= static_cast<T>(-std::numeric_limits<U>::max())
        && value <= static_cast<T>(std::numeric_limits<U>::max());
}

template <class U = lua_Number>
bool is_floating_point_representable_by(lua_State* L, int index)
{
    int isValid = 0;

    const auto value = tonumber(L, index, &isValid);

    return isValid ? is_floating_point_representable_by<U>(value) : false;
}

} // namespace luabridge
