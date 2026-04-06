// https://github.com/kunitoki/LuaBridge3
// Copyright 2026, kunitoki
// Inspired from https://github.com/ThePhD/lua-bindings-shootout by ThePhD
// SPDX-License-Identifier: MIT

#pragma once

#include "Lua/LuaLibrary.h"

#include <benchmark/benchmark.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

namespace lbsbench {

inline constexpr double kMagicValue = 24.0;

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

struct Basic
{
    double var = 0.0;

    double get() const
    {
        return var;
    }

    void set(double v)
    {
        var = v;
    }
};

struct BasicLarge
{
    std::int64_t var = 0;
    std::int64_t var0 = 0;
    std::int64_t var1 = 0;
    std::int64_t var2 = 0;
    std::int64_t var3 = 0;
    std::int64_t var4 = 0;
    std::int64_t var5 = 0;
    std::int64_t var6 = 0;
    std::int64_t var7 = 0;
    std::int64_t var8 = 0;
    std::int64_t var9 = 0;
    std::int64_t var10 = 0;
    std::int64_t var11 = 0;
    std::int64_t var12 = 0;
    std::int64_t var13 = 0;
    std::int64_t var14 = 0;
    std::int64_t var15 = 0;
    std::int64_t var16 = 0;
    std::int64_t var17 = 0;
    std::int64_t var18 = 0;
    std::int64_t var19 = 0;
    std::int64_t var20 = 0;
    std::int64_t var21 = 0;
    std::int64_t var22 = 0;
    std::int64_t var23 = 0;
    std::int64_t var24 = 0;
    std::int64_t var25 = 0;
    std::int64_t var26 = 0;
    std::int64_t var27 = 0;
    std::int64_t var28 = 0;
    std::int64_t var29 = 0;
    std::int64_t var30 = 0;
    std::int64_t var31 = 0;
    std::int64_t var32 = 0;
    std::int64_t var33 = 0;
    std::int64_t var34 = 0;
    std::int64_t var35 = 0;
    std::int64_t var36 = 0;
    std::int64_t var37 = 0;
    std::int64_t var38 = 0;
    std::int64_t var39 = 0;
    std::int64_t var40 = 0;
    std::int64_t var41 = 0;
    std::int64_t var42 = 0;
    std::int64_t var43 = 0;
    std::int64_t var44 = 0;
    std::int64_t var45 = 0;
    std::int64_t var46 = 0;
    std::int64_t var47 = 0;
    std::int64_t var48 = 0;
    std::int64_t var49 = 0;
};

struct ComplexBaseA
{
    double a = kMagicValue;

    double a_func() const
    {
        return a;
    }
};

struct ComplexBaseB
{
    double b = kMagicValue;

    double b_func() const
    {
        return b;
    }
};

struct ComplexAB : ComplexBaseA, ComplexBaseB
{
    double ab = kMagicValue;

    double ab_func() const
    {
        return ab;
    }
};

struct StatefulFunction
{
    double operator()(double v) const
    {
        return v;
    }
};

inline Basic* basic_return()
{
    static Basic value{};
    return &value;
}

inline double basic_get_var(Basic* b)
{
    return b ? b->var : 0.0;
}

void luaCheckOrThrow(lua_State* L, int status, std::string_view where);
void luaDoStringOrThrow(lua_State* L, std::string_view code, std::string_view where);

inline void setSkipped(benchmark::State& state, std::string_view reason)
{
    state.SkipWithError(std::string(reason).c_str());
}

} // namespace lbsbench
