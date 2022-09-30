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
template <class F>
struct ScopedGuard
{
    ScopedGuard(F func)
        : func_(std::move(func))
    {
    }

    ~ScopedGuard()
    {
        func_();
    }

private:
    F func_;
};

template <class F>
ScopedGuard(F) -> ScopedGuard<F>;

int callSharedClassMethod(xyz::ISharedClass* s)
{
    return s->publicMethod("1337");
}
} // namespace

TEST_F(DynamicLibraryTests, ExampleUsageFromLibrary)
{
    auto dll = dlopen(LUABRIDGEDEMO_DYNAMIC_LIBRARY, RTLD_NOW);
    ASSERT_NE(nullptr, dll);

    auto unloadDll = ScopedGuard([dll] { dlclose(dll); });

    auto allocator = reinterpret_cast<xyz::ISharedClass* (*)()>(dlsym(dll, "allocator"));
    ASSERT_NE(nullptr, allocator);

    auto deleter = reinterpret_cast<void (*)(xyz::ISharedClass*)>(dlsym(dll, "deleter"));
    ASSERT_NE(nullptr, deleter);

    luabridge::getGlobalNamespace(L)
        .beginClass<xyz::ISharedClass>("SharedClass")
            .addFactory(allocator, deleter)
            .addFunction("publicMethod", &xyz::ISharedClass::publicMethod)
        .endClass();

    luabridge::getGlobalNamespace(L)
        .addFunction("callSharedClassMethod", &callSharedClassMethod);

    xyz::ISharedClass* shared = allocator();
    auto unloadSharedClass = ScopedGuard([shared, deleter] { deleter(shared); });
    luabridge::setGlobal(L, shared, "shared");

    runLua("result = shared:publicMethod('42')");
    EXPECT_EQ(84, result<int>());

    runLua("result = callSharedClassMethod(shared)");
    EXPECT_EQ(1379, result<int>());

    runLua("a = SharedClass(); result = callSharedClassMethod(a)");
    EXPECT_EQ(1379, result<int>());

    runLua("b = SharedClass(); result = b");
    auto ptr = result<xyz::ISharedClass*>();
    ASSERT_NE(nullptr, ptr);
    EXPECT_EQ(1379, callSharedClassMethod(ptr));
}

