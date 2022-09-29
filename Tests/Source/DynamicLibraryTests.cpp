// https://github.com/kunitoki/LuaBridge3
// Copyright 2019, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"
#include "SharedCode.h"

struct DynamicLibraryTests : TestBase
{
};

extern void registerClasses(lua_State*);

TEST_F(DynamicLibraryTests, ExampleUsageFromLibrary)
{
    registerClasses(L);

    xyz::SharedClass shared;
    luabridge::setGlobal(L, &shared, "shared");

    runLua("result = shared:publicMethod('42')");
    ASSERT_EQ(84, result<int>());
}

