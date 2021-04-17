// https://github.com/kunitoki/LuaBridge3
// Copyright 2021, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#include "Lua/LuaLibrary.h"
#include "../../Distribution/LuaBridge/LuaBridge.h"

#include <gtest/gtest.h>

TEST(AmalgamateTests, CompilationWorks)
{
    auto L = luaL_newstate();
    luaL_openlibs(L);

    luabridge::getGlobalNamespace(L)
        .beginClass<struct Test>("Test")
        .endClass();

    lua_close(L);
}
