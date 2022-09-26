// https://github.com/vinniefalco/LuaBridge
// Copyright 2021, Lucio Asnaghi
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "Config.h"

namespace luabridge {

//=================================================================================================
/**
 * @brief Security options.
 */
class Security
{
public:
    static bool hideMetatables() noexcept
    {
        return getSettings().hideMetatables;
    }

    static void setHideMetatables(bool shouldHide) noexcept
    {
        getSettings().hideMetatables = shouldHide;
    }

private:
    struct Settings
    {
        Settings() noexcept
            : hideMetatables(true)
        {
        }

        bool hideMetatables;
    };

    static Settings& getSettings() noexcept
    {
        static Settings settings;
        return settings;
    }
};

//=================================================================================================
/**
 * @brief Get a global value from the lua_State.
 *
 * @note This works on any type specialized by `Stack`, including `LuaRef` and its table proxies.
*/
template <class T>
T getGlobal(lua_State* L, const char* name)
{
    lua_getglobal(L, name);

    auto result = luabridge::Stack<T>::get(L, -1);
    
    lua_pop(L, 1);
    
    return result;
}

//=================================================================================================
/**
 * @brief Set a global value in the lua_State.
 *
 * @note This works on any type specialized by `Stack`, including `LuaRef` and its table proxies.
*/
template <class T>
bool setGlobal(lua_State* L, T&& t, const char* name)
{
    std::error_code ec;
    if (push(L, std::forward<T>(t), ec))
    {
        lua_setglobal(L, name);
        return true;
    }

    return false;
}

//=================================================================================================
/**
 * @brief Change whether or not metatables are hidden (on by default).
 */
inline void setHideMetatables(bool shouldHide) noexcept
{
    Security::setHideMetatables(shouldHide);
}

} // namespace luabridge
