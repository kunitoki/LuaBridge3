// https://github.com/kunitoki/LuaBridge3
// Copyright 2026, kunitoki
// Inspired from https://github.com/ThePhD/lua-bindings-shootout by ThePhD
// SPDX-License-Identifier: MIT

#include "benchmark_common.hpp"

#include <sol/sol.hpp>

#include <benchmark/benchmark.h>

#include <tuple>

namespace {

using namespace lbsbench;

std::tuple<double, double> sol3_multi_return(double value)
{
    return { value, value * 2.0 };
}

void registerBasic(sol::state& lua)
{
    lua.new_usertype<Basic>("c",
        sol::constructors<Basic()>(),
        "set", &Basic::set,
        "get", &Basic::get,
        "var", &Basic::var);
}

void registerBasicLarge(sol::state& lua)
{
    lua.new_usertype<BasicLarge>("cl",
        sol::constructors<BasicLarge()>(),
        "var", &BasicLarge::var,
        "var0", &BasicLarge::var0,
        "var1", &BasicLarge::var1,
        "var2", &BasicLarge::var2,
        "var3", &BasicLarge::var3,
        "var4", &BasicLarge::var4,
        "var5", &BasicLarge::var5,
        "var6", &BasicLarge::var6,
        "var7", &BasicLarge::var7,
        "var8", &BasicLarge::var8,
        "var9", &BasicLarge::var9,
        "var10", &BasicLarge::var10,
        "var11", &BasicLarge::var11,
        "var12", &BasicLarge::var12,
        "var13", &BasicLarge::var13,
        "var14", &BasicLarge::var14,
        "var15", &BasicLarge::var15,
        "var16", &BasicLarge::var16,
        "var17", &BasicLarge::var17,
        "var18", &BasicLarge::var18,
        "var19", &BasicLarge::var19,
        "var20", &BasicLarge::var20,
        "var21", &BasicLarge::var21,
        "var22", &BasicLarge::var22,
        "var23", &BasicLarge::var23,
        "var24", &BasicLarge::var24,
        "var25", &BasicLarge::var25,
        "var26", &BasicLarge::var26,
        "var27", &BasicLarge::var27,
        "var28", &BasicLarge::var28,
        "var29", &BasicLarge::var29,
        "var30", &BasicLarge::var30,
        "var31", &BasicLarge::var31,
        "var32", &BasicLarge::var32,
        "var33", &BasicLarge::var33,
        "var34", &BasicLarge::var34,
        "var35", &BasicLarge::var35,
        "var36", &BasicLarge::var36,
        "var37", &BasicLarge::var37,
        "var38", &BasicLarge::var38,
        "var39", &BasicLarge::var39,
        "var40", &BasicLarge::var40,
        "var41", &BasicLarge::var41,
        "var42", &BasicLarge::var42,
        "var43", &BasicLarge::var43,
        "var44", &BasicLarge::var44,
        "var45", &BasicLarge::var45,
        "var46", &BasicLarge::var46,
        "var47", &BasicLarge::var47,
        "var48", &BasicLarge::var48,
        "var49", &BasicLarge::var49);
}

void table_global_string_get_measure(benchmark::State& state)
{
    sol::state lua;
    lua["value"] = kMagicValue;

    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        x += lua["value"].get<double>();
    }
    benchmark::DoNotOptimize(x);
}

void table_global_string_set_measure(benchmark::State& state)
{
    sol::state lua;
    double v = 0;

    for (auto _ : state)
    {
        (void) _;
        v += kMagicValue;
        lua["value"] = v;
    }

    benchmark::DoNotOptimize(v);
}

void table_get_measure(benchmark::State& state)
{
    sol::state lua;
    lua.script("warble = { value = 24.0 }");
    sol::table t = lua["warble"];

    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        x += t["value"].get<double>();
    }

    benchmark::DoNotOptimize(x);
}

void table_set_measure(benchmark::State& state)
{
    sol::state lua;
    lua.script("warble = { value = 24.0 }");
    sol::table t = lua["warble"];

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
    sol::state lua;
    lua.script("ulahibe = { warble = { value = 24.0 } }");

    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        x += lua["ulahibe"]["warble"]["value"].get<double>();
    }

    benchmark::DoNotOptimize(x);
}

void table_chained_set_measure(benchmark::State& state)
{
    sol::state lua;
    lua.script("ulahibe = { warble = { value = 24.0 } }");

    double v = 0;
    for (auto _ : state)
    {
        (void) _;
        v += kMagicValue;
        lua["ulahibe"]["warble"]["value"] = v;
    }

    benchmark::DoNotOptimize(v);
}

void c_function_measure(benchmark::State& state)
{
    sol::state lua;
    lua.set_function("f", +[](double value) { return value; });
    lua.script("function invoke_f() return f(24.0) end");

    for (auto _ : state)
    {
        (void) _;
        lua["invoke_f"]();
    }
}

void lua_function_in_c_measure(benchmark::State& state)
{
    sol::state lua;
    lua.script("function f(i) return i end");
    sol::function f = lua["f"];

    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        x += f.call<double>(kMagicValue);
    }

    benchmark::DoNotOptimize(x);
}

void c_function_through_lua_in_c_measure(benchmark::State& state)
{
    sol::state lua;
    lua.set_function("f", +[](double value) { return value; });
    sol::function f = lua["f"];

    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        x += f.call<double>(kMagicValue);
    }

    benchmark::DoNotOptimize(x);
}

void member_function_call_measure(benchmark::State& state)
{
    sol::state lua;
    registerBasic(lua);
    lua.script("b = c.new()\nfunction call_member() b:set(b:get() + 1.0) end");

    for (auto _ : state)
    {
        (void) _;
        lua["call_member"]();
    }
}

void userdata_variable_access_measure(benchmark::State& state)
{
    sol::state lua;
    registerBasic(lua);
    lua.script("b = c.new()\nfunction access_var() b.var = b.var + 1.0 return b.var end");

    for (auto _ : state)
    {
        (void) _;
        lua["access_var"]();
    }
}

void userdata_variable_access_large_measure(benchmark::State& state)
{
    sol::state lua;
    registerBasicLarge(lua);
    lua.script("b = cl.new()\nfunction access_var_large() b.var0 = b.var0 + 1 return b.var0 end");

    for (auto _ : state)
    {
        (void) _;
        lua["access_var_large"]();
    }
}

void userdata_variable_access_last_measure(benchmark::State& state)
{
    sol::state lua;
    registerBasicLarge(lua);
    lua.script("b = cl.new()\nfunction access_var_last() b.var49 = b.var49 + 1 return b.var49 end");

    for (auto _ : state)
    {
        (void) _;
        lua["access_var_last"]();
    }
}

void stateful_function_object_measure(benchmark::State& state)
{
    sol::state lua;
    lua.set_function("f", StatefulFunction{});
    sol::function f = lua["f"];

    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        x += f.call<double>(kMagicValue);
    }

    benchmark::DoNotOptimize(x);
}

void multi_return_lua_measure(benchmark::State& state)
{
    sol::state lua;
    lua.set_function("f", &sol3_multi_return);
    lua.script("function invoke_multi() local a,b=f(24.0) return a+b end");

    for (auto _ : state)
    {
        (void) _;
        lua["invoke_multi"]();
    }
}

void multi_return_measure(benchmark::State& state)
{
    sol::state lua;
    lua.set_function("f", &sol3_multi_return);
    sol::function f = lua["f"];

    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        std::tuple<double, double> values = f.call<double, double>(kMagicValue);
        x += std::get<0>(values);
        x += std::get<1>(values);
    }

    benchmark::DoNotOptimize(x);
}

void base_derived_measure(benchmark::State& state)
{
    sol::state lua;

    lua.new_usertype<ComplexBaseA>("ComplexBaseA",
        "a_func", &ComplexBaseA::a_func,
        "a", &ComplexBaseA::a);

    lua.new_usertype<ComplexBaseB>("ComplexBaseB",
        "b_func", &ComplexBaseB::b_func,
        "b", &ComplexBaseB::b);

    lua.new_usertype<ComplexAB>("ComplexAB",
        sol::base_classes, sol::bases<ComplexBaseA, ComplexBaseB>(),
        "ab_func", &ComplexAB::ab_func,
        "ab", &ComplexAB::ab);

    ComplexAB ab;
    lua["b"] = &ab;

    lua.script("function call_base() return b:a_func() + b:b_func() end");

    for (auto _ : state)
    {
        (void) _;
        lua["call_base"]();
    }
}

void optional_success_measure(benchmark::State& state)
{
    sol::state lua;
    lua.script("warble = { value = 24.0 }");

    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        sol::optional<double> value = lua["warble"]["value"];
        x += value.value_or(1.0);
    }

    benchmark::DoNotOptimize(x);
}

void optional_half_failure_measure(benchmark::State& state)
{
    sol::state lua;
    lua.script("warble = { value = 'x' }");

    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        sol::optional<double> value = lua["warble"]["value"];
        x += value.value_or(1.0);
    }

    benchmark::DoNotOptimize(x);
}

void optional_failure_measure(benchmark::State& state)
{
    sol::state lua;

    double x = 0;
    for (auto _ : state)
    {
        (void) _;
        sol::optional<double> value = lua["warble"]["value"];
        x += value.value_or(1.0);
    }

    benchmark::DoNotOptimize(x);
}

void return_userdata_measure(benchmark::State& state)
{
    sol::state lua;
    registerBasic(lua);
    lua.set_function("f", &basic_return);
    lua.set_function("h", &basic_get_var);
    lua.script("function invoke_userdata() return h(f()) end");

    for (auto _ : state)
    {
        (void) _;
        lua["invoke_userdata"]();
    }
}

void implicit_inheritance_measure(benchmark::State& state)
{
    sol::state lua;

    lua.new_usertype<ComplexBaseA>("ComplexBaseA",
        "a_func", &ComplexBaseA::a_func);

    lua.new_usertype<ComplexAB>("ComplexAB",
        sol::constructors<ComplexAB()>(),
        sol::base_classes, sol::bases<ComplexBaseA>(),
        "ab_func", &ComplexAB::ab_func);

    lua.set_function("call_a", +[](ComplexBaseA* obj) -> double { return obj->a_func(); });

    lua.script("obj = ComplexAB.new()");
    lua.script("function test_implicit() return call_a(obj) end");

    for (auto _ : state)
    {
        (void) _;
        lua["test_implicit"]();
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
