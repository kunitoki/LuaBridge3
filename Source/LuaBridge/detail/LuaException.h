// https://github.com/kunitoki/LuaBridge3
// Copyright 2021, Lucio Asnaghi
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2008, Nigel Atkinson <suprapilot+LuaCode@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "Config.h"

#include "LuaHelpers.h"

#include <cassert>
#include <string>
#include <sstream>
#include <exception>

namespace luabridge {

//================================================================================================
class LuaException : public std::exception
{
public:
    //=============================================================================================
    /**
     * @brief Construct a LuaException after a lua_pcall().
     *
     * Assumes the error string is on top of the stack, but provides a generic error message otherwise.
     */
    LuaException(lua_State* L, std::error_code code)
        : m_L(L)
        , m_code(code)
    {
    }

    ~LuaException() noexcept override
    {
    }

    //=============================================================================================
    /**
     * @brief Return the error message.
     */
    const char* what() const noexcept override
    {
        return m_what.c_str();
    }

    //=============================================================================================
    /**
     * @brief Throw an exception or raises a luaerror when exceptions are disabled.
     *
     * This centralizes all the exceptions thrown, so that we can set breakpoints before the stack is
     * unwound, or otherwise customize the behavior.
     */
    template <class Exception>
    static void raise(const Exception& e)
    {
        assert(areExceptionsEnabled());

#if LUABRIDGE_HAS_EXCEPTIONS
        throw e;
#else
        unused(e);

        std::abort();
#endif
    }

    //=============================================================================================
    /**
     * @brief Check if exceptions are enabled.
     */
    static bool areExceptionsEnabled() noexcept
    {
        return exceptionsEnabled();
    }

    /**
     * @brief Initializes error handling.
     *
     * Subsequent Lua errors are translated to C++ exceptions, or logging and abort if exceptions are disabled.
     */
    static void enableExceptions(lua_State* L) noexcept
    {
        exceptionsEnabled() = true;

        lua_atpanic(L, panicHandlerCallback);
    }

private:
    struct FromLua {};

    LuaException(lua_State* L, std::error_code code, FromLua)
        : m_L(L)
        , m_code(code)
    {
        whatFromStack();
    }

    void whatFromStack()
    {
        std::stringstream ss;

        const char* errorText = nullptr;

        if (lua_gettop(m_L) > 0)
        {
            errorText = lua_tostring(m_L, -1);
            lua_pop(m_L, 1);
        }

        ss << (errorText ? errorText : "Unknown error") << " (code=" << m_code.message() << ")";

        m_what = std::move(ss).str();
    }

    static int panicHandlerCallback(lua_State* L)
    {
#if LUABRIDGE_HAS_EXCEPTIONS
        throw LuaException(L, makeErrorCode(ErrorCode::LuaFunctionCallFailed), FromLua{});
#else
        unused(L);

        std::abort();
#endif
    }

    static bool& exceptionsEnabled()
    {
        static bool areExceptionsEnabled = false;
        return areExceptionsEnabled;
    }

    lua_State* m_L = nullptr;
    std::error_code m_code;
    std::string m_what;
};

//=================================================================================================
/**
 * @brief Initializes error handling using C++ exceptions.
 *
 * Subsequent Lua errors are translated to C++ exceptions. It aborts the application if called when no exceptions.
 */
inline void enableExceptions(lua_State* L) noexcept
{
#if LUABRIDGE_HAS_EXCEPTIONS
    LuaException::enableExceptions(L);
#else
    unused(L);

    assert(false); // Never call this function when exceptions are not enabled.
#endif
}

} // namespace luabridge
