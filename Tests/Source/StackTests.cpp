// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"

struct StackTests : TestBase
{
};

TEST_F(StackTests, NullptrType)
{
    std::error_code ec;
    bool result = luabridge::push(L, nullptr, ec);
    EXPECT_TRUE(result);
    EXPECT_TRUE(luabridge::isInstance<std::nullptr_t>(L, -1));
}

TEST_F(StackTests, IntegralTypes)
{
    {
        std::error_code ec;
        bool result = luabridge::push(L, true, ec);
        EXPECT_TRUE(result);
        EXPECT_TRUE(luabridge::isInstance<bool>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<int>(L, -1));
    }

    {
        std::error_code ec;
        bool result = luabridge::push(L, 5, ec);
        EXPECT_TRUE(result);
        EXPECT_TRUE(luabridge::isInstance<int>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
        EXPECT_TRUE(luabridge::isInstance<bool>(L, -2));
    }
}

TEST_F(StackTests, CharLiteralType)
{
    std::error_code ec;
    bool result = luabridge::push(L, "xyz", ec);
    EXPECT_TRUE(result);
    EXPECT_TRUE(luabridge::isInstance<const char*>(L, -1));
}
