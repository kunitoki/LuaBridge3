// https://github.com/kunitoki/LuaBridge3
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2008, Nigel Atkinson <suprapilot+LuaCode@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "Config.h"
#include "LuaHelpers.h"

#include <string>

#if LUABRIDGE_HAS_EXCEPTIONS
#include <exception>
#endif

namespace luabridge {

class LuaException : public std::exception
{
private:
    lua_State* m_L;
    std::string m_what;

public:
    //----------------------------------------------------------------------------
    /**
        Construct a LuaException after a lua_pcall().
    */
    LuaException(lua_State* L, int /*code*/) : m_L(L) { whatFromStack(); }

    //----------------------------------------------------------------------------

    LuaException(lua_State* L, char const*, char const*, long) : m_L(L) { whatFromStack(); }

    //----------------------------------------------------------------------------

    ~LuaException() throw() {}

    //----------------------------------------------------------------------------

    char const* what() const throw() { return m_what.c_str(); }

    //============================================================================
    /**
     * @brief Throw an exception or raises a luaerror when exceptions are disabled.
     *
     * This centralizes all the exceptions thrown, so that we can set breakpoints before the stack is unwound, or otherwise customize the behavior.
     */
    template <class Exception>
    static void Throw(lua_State* L, const Exception& e)
    {
#if LUABRIDGE_HAS_EXCEPTIONS
        throw e;
#else
        luaL_error(L, "%s", e);
#endif
    }
    
    //----------------------------------------------------------------------------
    /**
     * @brief Wrapper for lua_pcall that throws if exceptions are enabled.
     */
    static int pcall(lua_State* L, int nargs = 0, int nresults = 0, int msgh = 0)
    {
        int code = lua_pcall(L, nargs, nresults, msgh);

#if LUABRIDGE_HAS_EXCEPTIONS
        if (code != LUABRIDGE_LUA_OK)
            Throw(L, LuaException(L, code));
#endif
        
        return code;
    }

    //----------------------------------------------------------------------------
    /**
     * @brief Initializes error handling.
     *
     * Subsequent Lua errors are translated to C++ exceptions, or logging only if exceptions are disabled.
     */
    static void enableExceptions(lua_State* L)
    {
#if LUABRIDGE_HAS_EXCEPTIONS
        lua_atpanic(L, throwAtPanic);
#else
        lua_atpanic(L, logAtPanic);
#endif
    }

protected:
    void whatFromStack()
    {
        if (lua_gettop(m_L) > 0)
        {
            char const* s = lua_tostring(m_L, -1);
            m_what = s ? s : "";
        }
        else
        {
            // stack is empty
            m_what = "missing error";
        }
    }

private:
#if LUABRIDGE_HAS_EXCEPTIONS
    static int throwAtPanic(lua_State* L)
    {
        throw LuaException(L, -1);
    }
#endif
    
    static int logAtPanic(lua_State* L)
    {
        detail::writestringerror("Unprotected error in call (%s)", lua_tostring(L, -1));
        return 0;
    }
};

//----------------------------------------------------------------------------
/**
    Initializes error handling. Subsequent Lua errors are translated to C++ exceptions.
*/
static void enableExceptions(lua_State* L)
{
    LuaException::enableExceptions(L);
}

} // namespace luabridge
