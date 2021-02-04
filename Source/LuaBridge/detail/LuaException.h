// https://github.com/kunitoki/LuaBridge3
// Copyright 2021, Lucio Asnaghi
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2008, Nigel Atkinson <suprapilot+LuaCode@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "Config.h"
#include "LuaHelpers.h"

#include <string>
#include <sstream>

#if LUABRIDGE_HAS_EXCEPTIONS
#include <exception>
#endif

namespace luabridge {

class LuaException : public std::exception
{
private:
    lua_State* m_L = nullptr;
    int m_code = LUA_ERRERR;
    std::string m_what;

public:
    //=============================================================================================
    /**
     * @brief Construct a LuaException after a lua_pcall().
     *
     * Assumes the error string is on top of the stack, but provides a generic error message otherwise.
     */
    LuaException(lua_State* L, int code)
        : m_L(L)
        , m_code(code)
    {
        whatFromStack();
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
     * This centralizes all the exceptions thrown, so that we can set breakpoints before the stack is unwound, or otherwise customize the behavior.
     */
    template <class Exception>
    static void raise(lua_State* L, const Exception& e)
    {
#if LUABRIDGE_HAS_EXCEPTIONS
        (void) L;

        throw e;
#else
        luaL_error(L, "%s", e);
#endif
    }
    
    //=============================================================================================
    /**
     * @brief Wrapper for lua_pcall that throws if exceptions are enabled.
     */
    static int pcall(lua_State* L, int nargs = 0, int nresults = 0, int msgh = 0)
    {
        int code = lua_pcall(L, nargs, nresults, msgh);

#if LUABRIDGE_HAS_EXCEPTIONS
        if (code != LUABRIDGE_LUA_OK)
            raise(L, LuaException(L, code));
#endif
        
        return code;
    }

    //=============================================================================================
    /**
     * @brief Initializes error handling.
     *
     * Subsequent Lua errors are translated to C++ exceptions, or logging and abort if exceptions are disabled.
     */
    static void enableExceptions(lua_State* L) noexcept
    {
        lua_atpanic(L, panicHandlerCallback);
    }

protected:
    void whatFromStack()
    {
        std::stringstream ss;

        const char* errorText = nullptr;
        
        if (lua_gettop(m_L) > 0)
            errorText = lua_tostring(m_L, -1);

        ss << (errorText ? errorText : "Unknown error") << " (code=" << m_code << ")";

        m_what = std::move(ss).str();
    }

private:
    static int panicHandlerCallback(lua_State* L)
    {
#if LUABRIDGE_HAS_EXCEPTIONS
        throw LuaException(L, -1);
#else
        writestringerror("Unprotected error in call (%s)", lua_tostring(L, -1));
        return 0;
#endif
    }
};

//=================================================================================================
/**
 * @brief Initializes error handling.
 *
 * Subsequent Lua errors are translated to C++ exceptions, or logging and abort if exceptions are disabled.
 */
inline void enableExceptions(lua_State* L) noexcept
{
    LuaException::enableExceptions(L);
}

} // namespace luabridge
