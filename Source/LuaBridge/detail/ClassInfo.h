// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// Copyright 2020, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "Config.h"

#include <cstdint>
#include <memory>
#include <string_view>

#if defined __clang__ || defined __GNUC__
#define LUABRIDGE_PRETTY_FUNCTION __PRETTY_FUNCTION__
#define LUABRIDGE_PRETTY_FUNCTION_PREFIX '='
#define LUABRIDGE_PRETTY_FUNCTION_SUFFIX ']'
#elif defined _MSC_VER
#define LUABRIDGE_PRETTY_FUNCTION __FUNCSIG__
#define LUABRIDGE_PRETTY_FUNCTION_PREFIX '<'
#define LUABRIDGE_PRETTY_FUNCTION_SUFFIX '>'
#endif

namespace luabridge {
namespace detail {

[[nodiscard]] static constexpr auto fnv1a(const char* s, std::size_t count) noexcept
{
    if constexpr (sizeof(void*) == 4)
    {
        uint32_t seed = 2166136261u;

        for (std::size_t i = 0; i < count; ++i)
            seed ^= static_cast<uint32_t>(*s++) * 16777619u;

        return seed;
    }
    else
    {
        uint64_t seed = 14695981039346656037ull;

        for (std::size_t i = 0; i < count; ++i)
            seed ^= static_cast<uint64_t>(*s++) * 1099511628211ull;

        return seed;
    }
}

template <class T>
[[nodiscard]] static constexpr auto typeName() noexcept
{
    constexpr std::string_view prettyName{ LUABRIDGE_PRETTY_FUNCTION };

    constexpr auto first = prettyName.find_first_not_of(' ', prettyName.find_first_of(LUABRIDGE_PRETTY_FUNCTION_PREFIX) + 1);

    return prettyName.substr(first, prettyName.find_last_of(LUABRIDGE_PRETTY_FUNCTION_SUFFIX) - first);
}

template <class T, auto = typeName<T>().find_first_of('.')>
[[nodiscard]] static constexpr auto typeHash() noexcept
{
    constexpr auto stripped = typeName<T>();

    return fnv1a(stripped.data(), stripped.size());
}

//=================================================================================================
/**
 * @brief A unique key for a type name in a metatable.
 */
inline const void* getTypeKey() noexcept
{
    return reinterpret_cast<void*>(0x71);
}

//=================================================================================================
/**
 * @brief The key of a const table in another metatable.
 */
inline const void* getConstKey() noexcept
{
    return reinterpret_cast<void*>(0xc07);
}

//=================================================================================================
/**
 * @brief The key of a class table in another metatable.
 */
inline const void* getClassKey() noexcept
{
    return reinterpret_cast<void*>(0xc1a);
}

//=================================================================================================
/**
 * @brief The key of a propget table in another metatable.
 */
inline const void* getPropgetKey() noexcept
{
    return reinterpret_cast<void*>(0x6e7);
}

//=================================================================================================
/**
 * @brief The key of a propset table in another metatable.
 */
inline const void* getPropsetKey() noexcept
{
    return reinterpret_cast<void*>(0x5e7);
}

//=================================================================================================
/**
 * @brief The key of a static table in another metatable.
 */
inline const void* getStaticKey() noexcept
{
    return reinterpret_cast<void*>(0x57a);
}

//=================================================================================================
/**
 * @brief The key of a parent table in another metatable.
 */
inline const void* getParentKey() noexcept
{
    return reinterpret_cast<void*>(0xdad);
}

//=================================================================================================
/**
 * @brief Get the key for the static table in the Lua registry.
 *
 * The static table holds the static data members, static properties, and static member functions for a class.
 */
template <class T>
const void* getStaticRegistryKey() noexcept
{
#if LUABRIDGE_ON_OBJECTIVE_C
    static char value;
#else
    static auto value = typeHash<T>();
#endif

    return reinterpret_cast<void*>(value);
}

//=================================================================================================
/**
 * @brief Get the key for the class table in the Lua registry.
 *
 * The class table holds the data members, properties, and member functions of a class. Read-only data and properties, and const
 * member functions are also placed here (to save a lookup in the const table).
 */
template <class T>
const void* getClassRegistryKey() noexcept
{
#if LUABRIDGE_ON_OBJECTIVE_C
    static char value;
#else
    static auto value = typeHash<T>() ^ 1;
#endif

    return reinterpret_cast<void*>(value);
}

//=================================================================================================
/**
 * @brief Get the key for the const table in the Lua registry.
 *
 * The const table holds read-only data members and properties, and const member functions of a class.
 */
template <class T>
const void* getConstRegistryKey() noexcept
{
#if LUABRIDGE_ON_OBJECTIVE_C
    static char value;
#else
    static auto value = typeHash<T>() ^ 2;
#endif

    return reinterpret_cast<void*>(value);
}

} // namespace detail
} // namespace luabridge
