// https://github.com/kunitoki/LuaBridge3
// Copyright 2021, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#include "Lua/LuaLibrary.h"

#define luabridge luabridge_amalgamated
#include "../../Distribution/LuaBridge/LuaBridge.h"
#undef luabridge

#if _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4099) /* Type name first seen using 'class' now seen using 'struct' */
#endif

#include <gtest/gtest.h>

TEST(AmalgamateTests, CompilationWorks)
{
    auto L = luaL_newstate();
    luaL_openlibs(L);

    luabridge_amalgamated::getGlobalNamespace(L)
        .beginClass<struct Test>("Test")
        .endClass();

    lua_close(L);
}

#if _MSC_VER
#pragma warning (pop)
#endif
