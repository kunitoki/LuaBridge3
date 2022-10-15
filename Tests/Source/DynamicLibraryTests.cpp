// https://github.com/kunitoki/LuaBridge3
// Copyright 2022, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#include "TestBase.h"
#include "SharedCode.h"

#include <filesystem>
#include <set>

#if _WIN32
#include <windows.h>
#elif __APPLE__
#include <dlfcn.h>
#include <mach-o/dyld.h>
#else
#include <dlfcn.h>
#endif

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
#if _WIN32
    return LoadLibrary(library_path);
#else
    return dlopen(library_path, RTLD_NOW | RTLD_LAZY);
#endif
}

template <class T>
void closeSharedLibrary(T handle)
{
#if _WIN32
    FreeLibrary(handle);
#else
    dlclose(handle);
#endif
}

template <class S, class T>
auto lookupSharedLibrarySymbol(T handle, const char* procedure_name)
{
#if _WIN32
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

struct DynamicLibraryTests : TestBase
{
    auto loadSharedLibrary() -> decltype(openSharedLibrary(std::declval<const char*>()))
    {
        auto executablePath = getExecutablePath();
        if (! executablePath.has_value())
            return nullptr;

        auto libraryPath = executablePath->remove_filename() / LUABRIDGEDEMO_SHARED_LIBRARY;
        if (! std::filesystem::exists(libraryPath))
            return nullptr;

        return openSharedLibrary(libraryPath.string().c_str());
    }
};

TEST_F(DynamicLibraryTests, ExampleUsageFromLibrary)
{
    auto dll = loadSharedLibrary();
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

    closeLuaState(); // Force garbage collection before we unload the deleter
}

#if !LUABRIDGE_ON_LUAU
namespace {
void* allocFunction(void*, void* ptr, std::size_t osize, std::size_t nsize)
{
    static std::set<void*> allocs;

    void* nptr = nullptr;

    if (nsize > 0)
    {
        nptr = new uint8_t[nsize];

        if (ptr != nullptr)
            std::memcpy(nptr, ptr, nsize < osize ? nsize : osize);

        allocs.emplace(nptr);
    }

    auto node = allocs.extract(ptr);
    if (! node.empty() && ptr != nullptr)
        delete[] static_cast<uint8_t*>(ptr);

    return nptr;
}
} // namespace

TEST_F(DynamicLibraryTests, ExampleRegistrationFromLibrary)
{
    auto dll = loadSharedLibrary();
    ASSERT_NE(nullptr, dll);

    auto unloadDll = ScopedGuard([dll] { closeSharedLibrary(dll); });

    auto registerAnotherClass = lookupSharedLibrarySymbol<void (*)(lua_State*)>(dll, "registerAnotherClass");
    ASSERT_NE(nullptr, registerAnotherClass);

    closeLuaState();
    L = createNewLuaState(&allocFunction);

    registerAnotherClass(L);

    runLua("a = dll.AnotherClass(); result = a.value");
    EXPECT_EQ(30, result<int>());

    runLua("b = dll.AnotherClass(); b:publicMethod(12); result = b.value");
    EXPECT_EQ(12, result<int>());

    runLua("c = dll.AnotherClass(); result = c:publicMethod(12)");
    EXPECT_EQ(12, result<int>());

    runLua("d = dll.AnotherClass(); result = d:publicConstMethod(12)");
    EXPECT_EQ(42, result<int>());

    closeLuaState(); // Force garbage collection
}
#endif
