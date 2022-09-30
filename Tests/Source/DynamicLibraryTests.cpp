// https://github.com/kunitoki/LuaBridge3
// Copyright 2022, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#include "TestBase.h"
#include "SharedCode.h"

#include <filesystem>

#if _WIN32
#include <windows.h>
#elif __APPLE__
#include <dlfcn.h>
#include <mach-o/dyld.h>
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

std::optional<std::filesystem::path> getExecutablePath()
{
#if _WIN32
    TCHAR path[MAX_PATH];
    auto pathLength = GetModuleFileName(NULL, path, MAX_PATH);
    if (pathLength > 0)
        return std::filesystem::canonical(std::string_view(path, static_cast<std::size_t>(pathLength)));

#elif __APPLE__
    char path[1024];
    std::uint32_t pathLength = sizeof(path);
    if (_NSGetExecutablePath(path, &pathLength) == 0)
        return std::filesystem::canonical(std::string_view(path, static_cast<std::size_t>(pathLength)));

#else
    char path[1024];
    auto pathLength = readlink("/proc/self/exe", path, sizeof(path));
    if (pathLength > 0)
        return std::filesystem::canonical(std::string_view(path, static_cast<std::size_t>(pathLength)));

#endif

    return std::nullopt;
}

auto openSharedLibrary(const char* library_path)
{
#if _MSC_VER
    return LoadLibrary(library_path);
#else
    return dlopen(library_path, RTLD_NOW | RTLD_LAZY);
#endif
}

template <class T>
void closeSharedLibrary(T handle)
{
#if _MSC_VER
    FreeLibrary(handle);
#else
    dlclose(handle);
#endif
}

template <class S, class T>
auto lookupSharedLibrarySymbol(T handle, const char* procedure_name)
{
#if _MSC_VER
    return reinterpret_cast<S>(GetProcAddress(handle, procedure_name));
#else
    return reinterpret_cast<S>(dlsym(handle, procedure_name));
#endif
}

int callSharedClassMethod(xyz::ISharedClass* s)
{
    return s->publicMethod("1337");
}
} // namespace

TEST_F(DynamicLibraryTests, ExampleUsageFromLibrary)
{
    auto executablePath = getExecutablePath();
    ASSERT_TRUE(executablePath.has_value());

    auto libraryPath = executablePath->remove_filename() / LUABRIDGEDEMO_SHARED_LIBRARY;
    ASSERT_TRUE(std::filesystem::exists(libraryPath));

    auto dll = openSharedLibrary(LUABRIDGEDEMO_SHARED_LIBRARY);
    ASSERT_NE(nullptr, dll);

    auto unloadDll = ScopedGuard([dll] { closeSharedLibrary(dll); });

    auto allocator = lookupSharedLibrarySymbol<xyz::ISharedClass* (*)()>(dll, "allocator");
    ASSERT_NE(nullptr, allocator);

    auto deallocator = lookupSharedLibrarySymbol<void (*)(xyz::ISharedClass*)>(dll, "deallocator");
    ASSERT_NE(nullptr, deallocator);

    luabridge::getGlobalNamespace(L)
        .beginClass<xyz::ISharedClass>("SharedClass")
            .addFactory(allocator, deallocator)
            .addFunction("publicMethod", &xyz::ISharedClass::publicMethod)
        .endClass();

    luabridge::getGlobalNamespace(L)
        .addFunction("callSharedClassMethod", &callSharedClassMethod);

    xyz::ISharedClass* shared = allocator();
    auto unloadSharedClass = ScopedGuard([shared, deallocator] { deallocator(shared); });
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
