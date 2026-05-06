// https://github.com/kunitoki/LuaBridge3
// Copyright 2026, kunitoki
// Inspired from https://github.com/ThePhD/lua-bindings-shootout by ThePhD
// SPDX-License-Identifier: MIT

#include "benchmark_common.hpp"

#include "LuaBridge/LuaBridge.h"

#include <benchmark/benchmark.h>

#include <cmath>
#include <tuple>

namespace {

using namespace lbsbench;

std::tuple<double, double> lb3_multi_return(double value)
{
    return { value, value * 2.0 };
}

void registerBasic(lua_State* L)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Basic>("c")
            .addConstructor<void (*)()>()
            .addFunction("set", &Basic::set)
            .addFunction("get", &Basic::get)
            .addProperty("var", &Basic::var)
        .endClass();
}

void registerBasicLarge(lua_State* L)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<BasicLarge>("cl")
            .addConstructor<void (*)()>()
            .addProperty("var", &BasicLarge::var)
            .addProperty("var0", &BasicLarge::var0)
            .addProperty("var1", &BasicLarge::var1)
            .addProperty("var2", &BasicLarge::var2)
            .addProperty("var3", &BasicLarge::var3)
            .addProperty("var4", &BasicLarge::var4)
            .addProperty("var5", &BasicLarge::var5)
            .addProperty("var6", &BasicLarge::var6)
            .addProperty("var7", &BasicLarge::var7)
            .addProperty("var8", &BasicLarge::var8)
            .addProperty("var9", &BasicLarge::var9)
            .addProperty("var10", &BasicLarge::var10)
            .addProperty("var11", &BasicLarge::var11)
            .addProperty("var12", &BasicLarge::var12)
            .addProperty("var13", &BasicLarge::var13)
            .addProperty("var14", &BasicLarge::var14)
            .addProperty("var15", &BasicLarge::var15)
            .addProperty("var16", &BasicLarge::var16)
            .addProperty("var17", &BasicLarge::var17)
            .addProperty("var18", &BasicLarge::var18)
            .addProperty("var19", &BasicLarge::var19)
            .addProperty("var20", &BasicLarge::var20)
            .addProperty("var21", &BasicLarge::var21)
            .addProperty("var22", &BasicLarge::var22)
            .addProperty("var23", &BasicLarge::var23)
            .addProperty("var24", &BasicLarge::var24)
            .addProperty("var25", &BasicLarge::var25)
            .addProperty("var26", &BasicLarge::var26)
            .addProperty("var27", &BasicLarge::var27)
            .addProperty("var28", &BasicLarge::var28)
            .addProperty("var29", &BasicLarge::var29)
            .addProperty("var30", &BasicLarge::var30)
            .addProperty("var31", &BasicLarge::var31)
            .addProperty("var32", &BasicLarge::var32)
            .addProperty("var33", &BasicLarge::var33)
            .addProperty("var34", &BasicLarge::var34)
            .addProperty("var35", &BasicLarge::var35)
            .addProperty("var36", &BasicLarge::var36)
            .addProperty("var37", &BasicLarge::var37)
            .addProperty("var38", &BasicLarge::var38)
            .addProperty("var39", &BasicLarge::var39)
            .addProperty("var40", &BasicLarge::var40)
            .addProperty("var41", &BasicLarge::var41)
            .addProperty("var42", &BasicLarge::var42)
            .addProperty("var43", &BasicLarge::var43)
            .addProperty("var44", &BasicLarge::var44)
            .addProperty("var45", &BasicLarge::var45)
            .addProperty("var46", &BasicLarge::var46)
            .addProperty("var47", &BasicLarge::var47)
            .addProperty("var48", &BasicLarge::var48)
            .addProperty("var49", &BasicLarge::var49)
        .endClass();
}

lua_State* makeLua()
{
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    luabridge::registerMainThread(L);
#if LUABRIDGE_HAS_EXCEPTIONS
    luabridge::enableExceptions(L);
#endif
    return L;
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
    luaDoStringOrThrow(L, "warble = { value = 24.0 }", "table_get setup");
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
    luaDoStringOrThrow(L, "warble = { value = 24.0 }", "table_set setup");
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
    luaDoStringOrThrow(L, "ulahibe = { warble = { value = 24.0 } }", "table_chained_get setup");

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
    luaDoStringOrThrow(L, "ulahibe = { warble = { value = 24.0 } }", "table_chained_set setup");

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

    luaDoStringOrThrow(L, "function invoke_f() return f(24.0) end", "c_function setup");

    for (auto _ : state)
    {
        (void) _;
        lua_getglobal(L, "invoke_f");
        luaCheckOrThrow(L, lua_pcall(L, 0, 1, 0), "invoke_f");
        lua_pop(L, 1);
    }
}

void lua_function_in_c_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    luaDoStringOrThrow(L, "function f(i) return i end", "lua_function setup");

    luabridge::LuaRef f = luabridge::getGlobal(L, "f");
    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        x += f.call<double>(kMagicValue).valueOr(0.0);
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
        x += f.call<double>(kMagicValue).valueOr(0.0);
    }

    benchmark::DoNotOptimize(x);
}

void member_function_call_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    registerBasic(L);
    luaDoStringOrThrow(L, "b = c()", "member_function setup");
    luaDoStringOrThrow(L, "function call_member() b:set(b:get() + 1.0) end", "member_function closure setup");

    for (auto _ : state)
    {
        (void) _;
        lua_getglobal(L, "call_member");
        luaCheckOrThrow(L, lua_pcall(L, 0, 0, 0), "call_member");
    }
}

void userdata_variable_access_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    registerBasic(L);
    luaDoStringOrThrow(L, "b = c()", "userdata_variable_access setup");
    luaDoStringOrThrow(L, "function access_var() return b.var end", "userdata_variable_access closure setup");

    for (auto _ : state)
    {
        (void) _;
        lua_getglobal(L, "access_var");
        luaCheckOrThrow(L, lua_pcall(L, 0, 1, 0), "access_var");
        lua_pop(L, 1);
    }
}

void userdata_variable_access_large_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    registerBasicLarge(L);
    luaDoStringOrThrow(L, "b = cl()", "userdata_variable_access_large setup");
    luaDoStringOrThrow(L, "function access_var_large() return b.var0 end", "userdata_variable_access_large closure setup");

    for (auto _ : state)
    {
        (void) _;
        lua_getglobal(L, "access_var_large");
        luaCheckOrThrow(L, lua_pcall(L, 0, 1, 0), "access_var_large");
        lua_pop(L, 1);
    }
}

void userdata_variable_access_last_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    registerBasicLarge(L);
    luaDoStringOrThrow(L, "b = cl()", "userdata_variable_access_last setup");
    luaDoStringOrThrow(L, "function access_var_last() return b.var49 end", "userdata_variable_access_last closure setup");

    for (auto _ : state)
    {
        (void) _;
        lua_getglobal(L, "access_var_last");
        luaCheckOrThrow(L, lua_pcall(L, 0, 1, 0), "access_var_last");
        lua_pop(L, 1);
    }
}

void stateful_function_object_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    luabridge::getGlobalNamespace(L).addFunction("f", StatefulFunction{});

    luabridge::LuaRef f = luabridge::getGlobal(L, "f");
    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        x += f.call<double>(kMagicValue).valueOr(0.0);
    }

    benchmark::DoNotOptimize(x);
}

void multi_return_lua_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    luabridge::getGlobalNamespace(L).addFunction("f", &lb3_multi_return);
    luaDoStringOrThrow(L, "function invoke_multi() local a,b=f(24.0) return a+b end", "multi_return_lua setup");

    for (auto _ : state)
    {
        (void) _;
        lua_getglobal(L, "invoke_multi");
        luaCheckOrThrow(L, lua_pcall(L, 0, 1, 0), "invoke_multi");
        lua_pop(L, 1);
    }
}

void multi_return_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    luabridge::getGlobalNamespace(L).addFunction("f", &lb3_multi_return);
    luabridge::LuaRef f = luabridge::getGlobal(L, "f");

    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        auto result = f.call<std::tuple<double, double>>(kMagicValue).valueOr(std::make_tuple(0.0, 0.0));
        x += std::get<0>(result);
        x += std::get<1>(result);
    }

    benchmark::DoNotOptimize(x);
}

void base_derived_measure(benchmark::State& state)
{
    lua_State* L = makeLua();

    luabridge::getGlobalNamespace(L)
        .beginClass<ComplexBaseA>("ComplexBaseA")
            .addFunction("a_func", &ComplexBaseA::a_func)
            .addProperty("a", &ComplexBaseA::a)
        .endClass()
        .beginClass<ComplexBaseB>("ComplexBaseB")
            .addFunction("b_func", &ComplexBaseB::b_func)
            .addProperty("b", &ComplexBaseB::b)
        .endClass()
        .deriveClass<ComplexAB, ComplexBaseA, ComplexBaseB>("ComplexAB")
            .addConstructor<void (*)()>()
            .addFunction("ab_func", &ComplexAB::ab_func)
            .addProperty("ab", &ComplexAB::ab)
        .endClass();

    luaDoStringOrThrow(L, "obj = ComplexAB()", "base_derived setup");
    luaDoStringOrThrow(L, "function call_base() return obj:a_func() + obj:b_func() end", "base_derived closure setup");

    for (auto _ : state)
    {
        (void) _;
        lua_getglobal(L, "call_base");
        luaCheckOrThrow(L, lua_pcall(L, 0, 1, 0), "call_base");
        lua_pop(L, 1);
    }
}

void optional_success_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    luaDoStringOrThrow(L, "warble = { value = 24.0 }", "optional_success setup");

    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        auto result = luabridge::tryGetGlobalField<double>(L, "warble", "value");
        x += result ? *result : 1.0;
    }

    benchmark::DoNotOptimize(x);
}

void optional_half_failure_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    luaDoStringOrThrow(L, "warble = { value = 'x' }", "optional_half_failure setup");

    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        auto result = luabridge::tryGetGlobalField<double>(L, "warble", "value");
        x += result ? *result : 1.0;
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
        auto result = luabridge::tryGetGlobalField<double>(L, "warble", "value");
        x += result ? *result : 1.0;
    }

    benchmark::DoNotOptimize(x);
}

void return_userdata_measure(benchmark::State& state)
{
    lua_State* L = makeLua();
    registerBasic(L);
    luabridge::getGlobalNamespace(L)
        .addFunction("f", &basic_return)
        .addFunction("h", &basic_get_var);
    luaDoStringOrThrow(L, "function invoke_userdata() return h(f()) end", "return_userdata setup");

    for (auto _ : state)
    {
        (void) _;
        lua_getglobal(L, "invoke_userdata");
        luaCheckOrThrow(L, lua_pcall(L, 0, 1, 0), "invoke_userdata");
        lua_pop(L, 1);
    }
}

void implicit_inheritance_measure(benchmark::State& state)
{
    lua_State* L = makeLua();

    luabridge::getGlobalNamespace(L)
        .beginClass<ComplexBaseA>("ComplexBaseA")
            .addFunction("a_func", &ComplexBaseA::a_func)
        .endClass()
        .deriveClass<ComplexAB, ComplexBaseA>("ComplexAB")
            .addConstructor<void (*)()>()
            .addFunction("ab_func", &ComplexAB::ab_func)
        .endClass()
        .addFunction("call_a", +[](ComplexBaseA* obj) -> double { return obj->a_func(); });

    luaDoStringOrThrow(L, "obj = ComplexAB()", "implicit_inheritance setup");
    luaDoStringOrThrow(L, "function test_implicit() return call_a(obj) end", "implicit_inheritance closure setup");

    for (auto _ : state)
    {
        (void) _;
        lua_getglobal(L, "test_implicit");
        luaCheckOrThrow(L, lua_pcall(L, 0, 1, 0), "test_implicit");
        lua_pop(L, 1);
    }
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
