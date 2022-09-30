// https://github.com/kunitoki/LuaBridge3
// Copyright 2019, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"
#include "SharedCode.h"

#if _MSC_VER
#include <windows.h>
#else
#include <dlfcn.h>
#endif

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

auto lb_dlopen(const char* library_path)
{
#if _MSC_VER
    return LoadLibrary("MyPuts.dll");
#else
    return dlopen(library_path, RTLD_NOW | RTLD_LAZY);
#endif
}

template <class T>
void lb_dlclose(T handle)
{
#if _MSC_VER
    FreeLibrary(handle);
#else
    dlclose(handle);
#endif
}

template <class T>
auto lb_dlsym(T handle, const char* procedure_name)
{
#if _MSC_VER
    return GetProcAddress(handle, procedure_name);
#else
    return dlsym(handle, procedure_name);
#endif
}

int callSharedClassMethod(xyz::ISharedClass* s)
{
    return s->publicMethod("1337");
}
} // namespace

TEST_F(DynamicLibraryTests, ExampleUsageFromLibrary)
{
    auto dll = lb_dlopen(LUABRIDGEDEMO_SHARED_LIBRARY);
    ASSERT_NE(nullptr, dll);

    auto unloadDll = ScopedGuard([dll] { lb_dlclose(dll); });

    auto allocator = reinterpret_cast<xyz::ISharedClass* (*)()>(lb_dlsym(dll, "allocator"));
    ASSERT_NE(nullptr, allocator);

    auto deleter = reinterpret_cast<void (*)(xyz::ISharedClass*)>(lb_dlsym(dll, "deleter"));
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

    lua_close(L); // Force garbage collection before we unload the deleter
    L = nullptr;
}
