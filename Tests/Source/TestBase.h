// https://github.com/kunitoki/LuaBridge3
// Copyright 2022, Lucio Asnaghi
// Copyright 2019, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT

#pragma once

#include "Lua/LuaLibrary.h"

#include "LuaBridge/LuaBridge.h"
#include "LuaBridge/Dump.h"

#include <gtest/gtest.h>

#if LUABRIDGE_ON_LUAU
#include "../../ThirdParty/luau/Compiler/include/luacode.h"
#include "../../ThirdParty/luau/Common/include/Luau/Common.h"
#endif

#if LUABRIDGE_HAS_EXCEPTIONS
#include <stdexcept>
#endif

#include <cstring>

// Uncomment this if you want errors to be printed when lua fails to compile or run
//#define LUABRIDGE_TESTS_PRINT_ERRORS 1

// Traceback function, when a runtime error occurs, this will append the call stack to the error message
inline int traceback(lua_State* L)
{
    // look up Lua's 'debug.traceback' function
    lua_getglobal(L, "debug");
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        return 1;
    }

    lua_getfield(L, -1, "traceback");
    if (!lua_isfunction(L, -1))
    {
        lua_pop(L, 2);
        return 1;
    }

    lua_pushvalue(L, 1);
    lua_pushinteger(L, 2);
    lua_call(L, 2, 1);

    lua_getglobal(L, "print");
    if (!lua_isfunction(L, -1))
    {
        lua_pop(L, 1);
        return 1;
    }

    lua_pushvalue(L, 1);
    lua_call(L, 1, 0);

    return 1;
}

#if LUABRIDGE_ON_LUAU
inline int luaL_loadstring(lua_State *L, const char *s)
{
    std::size_t bytecodeSize = 0;

    auto bytecode = std::shared_ptr<char>(
        luau_compile(s, std::strlen(s), nullptr, &bytecodeSize),
        [](char* x) { std::free(x); }
    );

    return luau_load(L, "...", bytecode.get(), bytecodeSize, 0);
}
#endif

// Base test class. Introduces the global 'result' variable, used for checking of C++ - Lua interoperation.
struct TestBase : public ::testing::Test
{
    lua_State* L = nullptr;

    void SetUp() override
    {
        L = createNewLuaState();
    }

    void TearDown() override
    {
        closeLuaState();
    }

    lua_State* createNewLuaState(lua_Alloc alloc = nullptr) const
    {
        lua_State* l;

        if (alloc)
            l = lua_newstate(alloc, nullptr);
        else
            l = luaL_newstate();

        luaL_openlibs(l);

        luabridge::registerMainThread(l);

#if LUABRIDGE_HAS_EXCEPTIONS
        luabridge::enableExceptions(l);
#endif

        return l;
    }

    void closeLuaState()
    {
        if (L != nullptr)
        {
            lua_close(L);
            L = nullptr;
        }
    }

    bool runLua(const std::string& script, lua_State* overrideState = nullptr) const
    {
        auto stateToUse = overrideState ? overrideState : L;

        lua_settop(stateToUse, 0);

        luabridge::lua_pushcfunction_x(stateToUse, &traceback, "traceback");

        if (luaL_loadstring(stateToUse, script.c_str()) != LUABRIDGE_LUA_OK)
        {
            [[maybe_unused]] auto errorString = lua_tostring(stateToUse, -1);

#if LUABRIDGE_HAS_EXCEPTIONS
            throw std::runtime_error(errorString ? errorString : "Unknown lua compilation error");
#else
#if LUABRIDGE_TESTS_PRINT_ERRORS
            std::cerr << "===== Lua Compile Error =====\n";
            std::cerr << errorString ? errorString : "Unknown lua compilation error" << "\n";
#endif // LUABRIDGE_TESTS_PRINT_ERRORS
            return false;
#endif // LUABRIDGE_HAS_EXCEPTIONS
        }

        if (lua_pcall(stateToUse, 0, 0, -2) != LUABRIDGE_LUA_OK)
        {
            [[maybe_unused]] auto errorString = lua_tostring(stateToUse, -1);

#if LUABRIDGE_HAS_EXCEPTIONS
            throw std::runtime_error(errorString ? errorString : "Unknown lua runtime error");
#else
#if LUABRIDGE_TESTS_PRINT_ERRORS
            std::cerr << "===== Lua Call Error =====\n";
            std::cerr << (errorString ? errorString : "Unknown lua runtime error") << "\n";
#endif // LUABRIDGE_TESTS_PRINT_ERRORS
            return false;
#endif // LUABRIDGE_HAS_EXCEPTIONS
        }

        return true;
    }

    std::tuple<bool, std::string> runLuaCaptureError(const std::string& script) const
    {
        lua_settop(L, 0);

        if (luaL_loadstring(L, script.c_str()) != LUABRIDGE_LUA_OK)
        {
            auto errorString = lua_tostring(L, -1);

#if LUABRIDGE_TESTS_PRINT_ERRORS
            std::cerr << "===== Lua Compile Error =====\n";
            std::cerr << errorString ? errorString : "Unknown lua compilation error" << "\n";
#endif // LUABRIDGE_TESTS_PRINT_ERRORS

            return std::make_tuple(false, errorString);
        }

        if (lua_pcall(L, 0, 0, 0) != LUABRIDGE_LUA_OK)
        {
            auto errorString = lua_tostring(L, -1);

#if LUABRIDGE_TESTS_PRINT_ERRORS
            std::cerr << "===== Lua Call Error =====\n";
            std::cerr << (errorString ? errorString : "Unknown lua runtime error") << "\n";
#endif // LUABRIDGE_TESTS_PRINT_ERRORS

            return std::make_tuple(false, errorString);
        }

        return std::make_tuple(true, "");
    }

    template <class T = luabridge::LuaRef>
    T result() const
    {
        if constexpr (std::is_same_v<T, luabridge::LuaRef>)
            return luabridge::getGlobal(L, "result");

        return luabridge::getGlobal(L, "result").unsafe_cast<T>();
    }

    void resetResult() const
    {
        luabridge::setGlobal(L, luabridge::LuaRef(L), "result");
    }

    void exhaustStackSpace()
    {
        // Exhaust LUAI_MAXSTACK = 1000000
        for (int i = 0; i < 1000000; ++i)
        {
            if (!lua_checkstack(L, 1))
                break;

            lua_pushnil(L);
        }
        
        // Exhaust ERRORSTACKSIZE = LUAI_MAXSTACK + 200
        for (int i = 0; i < 200; ++i)
        {
            if (!lua_checkstack(L, 1))
                break;

            lua_pushnil(L);
        }
    }

    void printStack() const
    {
        std::cerr << "===== Stack =====\n";
        for (int i = 1; i <= lua_gettop(L); ++i)
        {
            std::cerr << "@" << i << " = " << luabridge::LuaRef::fromStack(L, i) << "\n";
        }
    }
};
