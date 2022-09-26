// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// Copyright 2020, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "Config.h"

#include <memory>

namespace luabridge {
namespace detail {

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
    static char value;
    return std::addressof(value);
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
    static char value;
    return std::addressof(value);
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
    static char value;
    return std::addressof(value);
}

} // namespace detail
} // namespace luabridge
