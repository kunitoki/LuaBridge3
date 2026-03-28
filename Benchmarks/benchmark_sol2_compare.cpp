// SPDX-License-Identifier: MIT

#include "Lua/LuaLibrary.h"
#include "LuaBridge/LuaBridge.h"

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

#if LUABRIDGE_BENCHMARK_WITH_SOL2
#include <sol/sol.hpp>
#endif

namespace {

struct Counter
{
    int value = 0;

    void inc()
    {
        ++value;
    }

    int add(int x)
    {
        value += x;
        return value;
    }

    int get() const
    {
        return value;
    }

    void set(int v)
    {
        value = v;
    }
};

using Clock = std::chrono::steady_clock;

struct CaseResult
{
    std::string name;
    double nsPerOp = 0.0;
    double mops = 0.0;
};

template <class Fn>
CaseResult runCase(const std::string& name, std::int64_t iterations, Fn&& fn)
{
    fn(); // warm-up

    const auto start = Clock::now();
    fn();
    const auto end = Clock::now();

    const auto elapsedNs = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    const double nsPerOp = static_cast<double>(elapsedNs) / static_cast<double>(iterations);
    const double mops = 1'000.0 / nsPerOp;

    return { name, nsPerOp, mops };
}

void printResult(const char* family, const CaseResult& r)
{
    std::cout << std::left << std::setw(12) << family
              << "  " << std::setw(30) << r.name
              << "  " << std::fixed << std::setprecision(2)
              << std::setw(12) << r.nsPerOp
              << "  " << std::setw(10) << r.mops
              << '\n';
}

void runLuaBridgeBenchmarks(std::int64_t iterations)
{
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    luabridge::registerMainThread(L);

#if LUABRIDGE_HAS_EXCEPTIONS
    luabridge::enableExceptions(L);
#endif

    luabridge::getGlobalNamespace(L)
        .beginClass<Counter>("Counter")
            .addConstructor<void (*)()>()
            .addFunction("inc", &Counter::inc)
            .addFunction("add", &Counter::add)
            .addProperty("value", &Counter::get, &Counter::set)
        .endClass()
        .addFunction("add", +[](int a, int b) { return a + b; });

    const auto doLua = [L](const char* chunk)
    {
        if (luaL_dostring(L, chunk) != LUABRIDGE_LUA_OK)
        {
            const char* message = lua_tostring(L, -1);
            std::cerr << "Lua error: " << (message ? message : "unknown") << '\n';
            std::exit(1);
        }
    };

    doLua("obj = Counter()");

    const std::string emptyLoop = "for i = 1, " + std::to_string(iterations) + " do end";
    const std::string freeCallLoop = "for i = 1, " + std::to_string(iterations) + " do add(i, i) end";
    const std::string memberCallLoop = "for i = 1, " + std::to_string(iterations) + " do obj:inc() end";
    const std::string propertyLoop = "for i = 1, " + std::to_string(iterations) + " do obj.value = i; local x = obj.value end";

    const auto empty = runCase("lua_empty_loop", iterations, [&] { doLua(emptyLoop.c_str()); });
    const auto freeCall = runCase("lua_to_cpp_free_fn", iterations, [&] { doLua(freeCallLoop.c_str()); });
    const auto memberCall = runCase("lua_to_cpp_member", iterations, [&] { doLua(memberCallLoop.c_str()); });
    const auto property = runCase("lua_to_cpp_property", iterations, [&] { doLua(propertyLoop.c_str()); });

    CaseResult cppToLua;
    {
        doLua("function f(a, b) return a + b end");
        auto f = luabridge::getGlobal(L, "f");
        cppToLua = runCase("cpp_to_lua_call", iterations, [&]
        {
            for (std::int64_t i = 0; i < iterations; ++i)
                (void) f(static_cast<int>(i), static_cast<int>(i));
        });
    }

    printResult("LuaBridge", empty);
    printResult("LuaBridge", freeCall);
    printResult("LuaBridge", memberCall);
    printResult("LuaBridge", property);
    printResult("LuaBridge", cppToLua);

    lua_close(L);
}

#if LUABRIDGE_BENCHMARK_WITH_SOL2
void runSol2Benchmarks(std::int64_t iterations)
{
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::string);

    Counter counter;

    lua.new_usertype<Counter>("Counter",
        sol::call_constructor,
        sol::constructors<Counter()>(),
        "inc", &Counter::inc,
        "add", &Counter::add,
        "value", sol::property(&Counter::get, &Counter::set));

    lua.set_function("add", +[](int a, int b) { return a + b; });
    lua["counter"] = &counter;
    lua.script("obj = Counter()");

    const std::string emptyLoop = "for i = 1, " + std::to_string(iterations) + " do end";
    const std::string freeCallLoop = "for i = 1, " + std::to_string(iterations) + " do add(i, i) end";
    const std::string memberCallLoop = "for i = 1, " + std::to_string(iterations) + " do obj:inc() end";
    const std::string propertyLoop = "for i = 1, " + std::to_string(iterations) + " do obj.value = i; local x = obj.value end";

    const auto empty = runCase("lua_empty_loop", iterations, [&] { lua.script(emptyLoop); });
    const auto freeCall = runCase("lua_to_cpp_free_fn", iterations, [&] { lua.script(freeCallLoop); });
    const auto memberCall = runCase("lua_to_cpp_member", iterations, [&] { lua.script(memberCallLoop); });
    const auto property = runCase("lua_to_cpp_property", iterations, [&] { lua.script(propertyLoop); });

    lua.script("function f(a, b) return a + b end");
    sol::function f = lua["f"];
    const auto cppToLua = runCase("cpp_to_lua_call", iterations, [&]
    {
        for (std::int64_t i = 0; i < iterations; ++i)
            (void) f(static_cast<int>(i), static_cast<int>(i));
    });

    printResult("sol2", empty);
    printResult("sol2", freeCall);
    printResult("sol2", memberCall);
    printResult("sol2", property);
    printResult("sol2", cppToLua);
}
#endif

} // namespace

int main(int argc, char** argv)
{
    std::int64_t iterations = 2'000'000;

    if (argc > 1)
    {
        const auto parsed = std::strtoll(argv[1], nullptr, 10);
        if (parsed > 0)
            iterations = parsed;
    }

    std::cout << "Iterations: " << iterations << '\n';
    std::cout << std::left << std::setw(12) << "Family"
              << "  " << std::setw(30) << "Case"
              << "  " << std::setw(12) << "ns/op"
              << "  " << std::setw(10) << "Mop/s"
              << '\n';

    runLuaBridgeBenchmarks(iterations);

#if LUABRIDGE_BENCHMARK_WITH_SOL2
    runSol2Benchmarks(iterations);
#else
    std::cout << "sol2 not enabled (set LUABRIDGE_BENCHMARK_WITH_SOL2=ON and SOL2_INCLUDE_DIR)." << '\n';
#endif

    return 0;
}
