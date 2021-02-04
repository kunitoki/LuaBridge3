// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"

struct StackTests : TestBase
{
};

TEST_F(StackTests, IntegralTypes)
{
    {
        std::error_code ec;
        luabridge::push(L, true, ec);
        ASSERT_TRUE(luabridge::isInstance<bool>(L, -1));
        ASSERT_FALSE(luabridge::isInstance<int>(L, -1));
    }

    {
        std::error_code ec;
        luabridge::push(L, 5, ec);
        ASSERT_TRUE(luabridge::isInstance<int>(L, -1));
        ASSERT_FALSE(luabridge::isInstance<bool>(L, -1));
        ASSERT_TRUE(luabridge::isInstance<bool>(L, -2));
    }
}
