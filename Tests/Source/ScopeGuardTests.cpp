// https://github.com/kunitoki/LuaBridge3
// Copyright 2022, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#include "TestBase.h"

TEST(ScopeGuardTests, ExecuteAtScopeExit)
{
    bool executed = false;

    {
        const luabridge::detail::ScopeGuard sg([&] { executed = true; });
        luabridge::unused(sg);
    }

    EXPECT_TRUE(executed);
}

TEST(ScopeGuardTests, ResetDoesNotExecuteAtScopeExit)
{
    bool executed = false;

    {
        luabridge::detail::ScopeGuard sg([&] { executed = true; });
        sg.reset();
    }

    EXPECT_FALSE(executed);
}
