// https://github.com/kunitoki/LuaBridge3
// Copyright 2026, kunitoki
// Inspired from https://github.com/ThePhD/lua-bindings-shootout by ThePhD
// SPDX-License-Identifier: MIT

#include "benchmark_common.hpp"

#include <LuaBridge/LuaBridge.h>

#include <benchmark/benchmark.h>

namespace {

using namespace lbsbench;

int vanilla_multi_return(lua_State* L)
{
    const double i = lua_tonumber(L, 1);
    luabridge::push(L, i);
    luabridge::push(L, i * 2.0);
    return 2;
}

lua_State* makeLua()
{
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    return L;
}

void registerBasic(lua_State* L)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Basic>("c")
            .addConstructor<void (*)()>()
            .addFunction("set", &Basic::set)
            .addFunction("get", &Basic::get)
            .addData("var", &Basic::var)
        .endClass();
}

void table_global_string_get_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    luabridge::setGlobal(L, kMagicValue, "value");

    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        x += static_cast<double>(luabridge::getGlobal(L, "value"));
    }

    benchmark::DoNotOptimize(x);
}

void table_global_string_set_measure(benchmark::State& state)
{
    lua_State* L = makeLua();

    double v = 0;
    for (auto _ : state)
    {
        (void) _;
        v += kMagicValue;
        luabridge::setGlobal(L, v, "value");
    }

    benchmark::DoNotOptimize(v);
}

void table_get_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    luaDoStringOrThrow(L, "warble = { value = 24.0 }", "vanilla table_get setup");
    luabridge::LuaRef t = luabridge::getGlobal(L, "warble");

    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        x += static_cast<double>(t["value"]);
    }

    benchmark::DoNotOptimize(x);
}

void table_set_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    luaDoStringOrThrow(L, "warble = { value = 24.0 }", "vanilla table_set setup");
    luabridge::LuaRef t = luabridge::getGlobal(L, "warble");

    double v = 0;
    for (auto _ : state)
    {
        (void) _;
        v += kMagicValue;
        t["value"] = v;
    }

    benchmark::DoNotOptimize(v);
}

void table_chained_get_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    luaDoStringOrThrow(L, "ulahibe = { warble = { value = 24.0 } }", "vanilla chained_get setup");

    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        luabridge::LuaRef tw = luabridge::getGlobal(L, "ulahibe")["warble"];
        x += static_cast<double>(tw["value"]);
    }

    benchmark::DoNotOptimize(x);
}

void table_chained_set_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    luaDoStringOrThrow(L, "ulahibe = { warble = { value = 24.0 } }", "vanilla chained_set setup");

    double v = 0;
    for (auto _ : state)
    {
        (void) _;
        v += kMagicValue;
        luabridge::LuaRef tw = luabridge::getGlobal(L, "ulahibe")["warble"];
        tw["value"] = v;
    }

    benchmark::DoNotOptimize(v);
}

void c_function_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    luabridge::getGlobalNamespace(L).addFunction("f", +[](double v) { return v; });
    luaDoStringOrThrow(L, "function invoke_f() return f(24.0) end", "vanilla c_function setup");

    for (auto _ : state)
    {
        (void) _;
        lua_getglobal(L, "invoke_f");
        luaCheckOrThrow(L, lua_pcall(L, 0, 1, 0), "vanilla invoke_f");
        lua_pop(L, 1);
    }
}

void lua_function_in_c_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    luaDoStringOrThrow(L, "function f(i) return i end", "vanilla lua_function setup");

    luabridge::LuaRef f = luabridge::getGlobal(L, "f");
    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        x += static_cast<double>(f(kMagicValue));
    }

    benchmark::DoNotOptimize(x);
}

void c_function_through_lua_in_c_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    luabridge::getGlobalNamespace(L).addFunction("f", +[](double v) { return v; });

    luabridge::LuaRef f = luabridge::getGlobal(L, "f");
    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        x += static_cast<double>(f(kMagicValue));
    }

    benchmark::DoNotOptimize(x);
}

void member_function_call_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    registerBasic(L);
    luaDoStringOrThrow(L, "b = c()", "vanilla member setup");
    luaDoStringOrThrow(L, "function call_member() b:set(b:get() + 1.0) end", "vanilla member closure setup");

    for (auto _ : state)
    {
        (void) _;
        lua_getglobal(L, "call_member");
        luaCheckOrThrow(L, lua_pcall(L, 0, 0, 0), "vanilla call_member");
    }
}

void userdata_variable_access_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    registerBasic(L);
    luaDoStringOrThrow(L, "b = c()", "vanilla userdata setup");
    luaDoStringOrThrow(L, "function access_var() b.var = b.var + 1.0 return b.var end", "vanilla userdata closure setup");

    for (auto _ : state)
    {
        (void) _;
        lua_getglobal(L, "access_var");
        luaCheckOrThrow(L, lua_pcall(L, 0, 1, 0), "vanilla access_var");
        lua_pop(L, 1);
    }
}

void userdata_variable_access_large_measure(benchmark::State& state)
{
    setSkipped(state, "unsupported in LuaBridge vanilla benchmark parity mode");
}

void userdata_variable_access_last_measure(benchmark::State& state)
{
    setSkipped(state, "unsupported in LuaBridge vanilla benchmark parity mode");
}

void stateful_function_object_measure(benchmark::State& state)
{
    setSkipped(state, "unsupported in LuaBridge vanilla benchmark parity mode");
}

void multi_return_lua_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    luabridge::getGlobalNamespace(L).addCFunction("f", vanilla_multi_return);
    luaDoStringOrThrow(L, "function invoke_multi() local a,b=f(24.0) return a+b end", "vanilla multi_return_lua setup");

    for (auto _ : state)
    {
        (void) _;
        lua_getglobal(L, "invoke_multi");
        luaCheckOrThrow(L, lua_pcall(L, 0, 1, 0), "vanilla invoke_multi");
        lua_pop(L, 1);
    }
}

void multi_return_measure(benchmark::State& state)
{
    setSkipped(state, "unsupported conceptual multi-return conversion in LuaBridge vanilla");
}

void base_derived_measure(benchmark::State& state)
{
    setSkipped(state, "unsupported for multi inheritance in LuaBridge vanilla");
}

void return_userdata_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    registerBasic(L);
    luabridge::getGlobalNamespace(L)
        .addFunction("f", &basic_return)
        .addFunction("h", &basic_get_var);
    luaDoStringOrThrow(L, "function invoke_userdata() return h(f()) end", "vanilla return_userdata setup");

    for (auto _ : state)
    {
        (void) _;
        lua_getglobal(L, "invoke_userdata");
        luaCheckOrThrow(L, lua_pcall(L, 0, 1, 0), "vanilla invoke_userdata");
        lua_pop(L, 1);
    }
}

void optional_success_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    luaDoStringOrThrow(L, "warble = { value = 24.0 }", "vanilla optional_success setup");

    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        luabridge::LuaRef tt = luabridge::getGlobal(L, "warble");
        if (tt.isTable())
        {
            luabridge::LuaRef tv = tt["value"];
            x += tv.isNumber() ? static_cast<double>(tv) : 1.0;
        }
        else
        {
            x += 1.0;
        }
    }

    benchmark::DoNotOptimize(x);
}

void optional_half_failure_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    luaDoStringOrThrow(L, "warble = { value = 'x' }", "vanilla optional_half_failure setup");

    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        luabridge::LuaRef tt = luabridge::getGlobal(L, "warble");
        if (tt.isTable())
        {
            luabridge::LuaRef tv = tt["value"];
            x += tv.isNumber() ? static_cast<double>(tv) : 1.0;
        }
        else
        {
            x += 1.0;
        }
    }

    benchmark::DoNotOptimize(x);
}

void optional_failure_measure(benchmark::State& state)
{
    lua_State* L = makeLua();

    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        luabridge::LuaRef tt = luabridge::getGlobal(L, "warble");
        if (tt.isTable())
        {
            luabridge::LuaRef tv = tt["value"];
            x += tv.isNumber() ? static_cast<double>(tv) : 1.0;
        }
        else
        {
            x += 1.0;
        }
    }

    benchmark::DoNotOptimize(x);
}

void implicit_inheritance_measure(benchmark::State& state)
{
    setSkipped(state, "unsupported for multi inheritance in LuaBridge vanilla");
}

} // namespace

BENCHMARK(table_global_string_get_measure)->Name("table_global_string_get_measure");
BENCHMARK(table_global_string_set_measure)->Name("table_global_string_set_measure");
BENCHMARK(table_get_measure)->Name("table_get_measure");
BENCHMARK(table_set_measure)->Name("table_set_measure");
BENCHMARK(table_chained_get_measure)->Name("table_chained_get_measure");
BENCHMARK(table_chained_set_measure)->Name("table_chained_set_measure");
BENCHMARK(c_function_measure)->Name("c_function_measure");
BENCHMARK(c_function_through_lua_in_c_measure)->Name("c_function_through_lua_in_c_measure");
BENCHMARK(lua_function_in_c_measure)->Name("lua_function_in_c_measure");
BENCHMARK(member_function_call_measure)->Name("member_function_call_measure");
BENCHMARK(userdata_variable_access_measure)->Name("userdata_variable_access_measure");
BENCHMARK(userdata_variable_access_large_measure)->Name("userdata_variable_access_large_measure");
BENCHMARK(userdata_variable_access_last_measure)->Name("userdata_variable_access_last_measure");
BENCHMARK(multi_return_lua_measure)->Name("multi_return_lua_measure");
BENCHMARK(multi_return_measure)->Name("multi_return_measure");
BENCHMARK(stateful_function_object_measure)->Name("stateful_function_object_measure");
BENCHMARK(base_derived_measure)->Name("base_derived_measure");
BENCHMARK(return_userdata_measure)->Name("return_userdata_measure");
BENCHMARK(optional_success_measure)->Name("optional_success_measure");
BENCHMARK(optional_half_failure_measure)->Name("optional_half_failure_measure");
BENCHMARK(optional_failure_measure)->Name("optional_failure_measure");
BENCHMARK(implicit_inheritance_measure)->Name("implicit_inheritance_measure");
