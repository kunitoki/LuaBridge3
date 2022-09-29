// https://github.com/kunitoki/LuaBridge3
// Copyright 2019, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"
#include "SharedCode.h"

#include <dlfcn.h>

struct DynamicLibraryTests : TestBase
{
};

namespace {
int callSharedClassMethod(xyz::SharedClass* s)
{
    return s->publicMethod("1337");
}
} // namespace

TEST_F(DynamicLibraryTests, ExampleUsageFromLibrary)
{
    auto dll = dlopen("libLuaBridgeTests54_DynamicLibrary.dylib", RTLD_NOW);
    ASSERT_NE(nullptr, dll);

    auto registerClasses = reinterpret_cast<void (*)(lua_State*)>(dlsym(dll, "registerClasses"));
    ASSERT_NE(nullptr, registerClasses);

    registerClasses(L);

    luabridge::getGlobalNamespace(L)
        .addFunction("callSharedClassMethod", &callSharedClassMethod);

    xyz::SharedClass shared;
    luabridge::setGlobal(L, &shared, "shared");

    runLua("result = shared:publicMethod('42')");
    EXPECT_EQ(84, result<int>());

    runLua("result = callSharedClassMethod(shared)");
    EXPECT_EQ(1379, result<int>());

    runLua("a = SharedClass(); result = callSharedClassMethod(a)");
    EXPECT_EQ(1379, result<int>());

    dlclose(dll);
}

