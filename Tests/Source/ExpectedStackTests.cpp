// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, kunitoki
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include <system_error>

struct ExpectedStackTests : TestBase
{
};

TEST_F(ExpectedStackTests, PushWithValue)
{
    using ExpectedInt = luabridge::Expected<int, std::error_code>;

    ExpectedInt value(42);
    ASSERT_TRUE(luabridge::push(L, value));
    EXPECT_EQ(LUA_TNUMBER, lua_type(L, -1));

    auto result = luabridge::Stack<int>::get(L, -1);
    ASSERT_TRUE(result);
    EXPECT_EQ(42, *result);
    lua_pop(L, 1);
}

TEST_F(ExpectedStackTests, PushWithError)
{
    using ExpectedInt = luabridge::Expected<int, std::error_code>;

    ExpectedInt value(luabridge::makeUnexpected(luabridge::makeErrorCode(luabridge::ErrorCode::InvalidTypeCast)));
    ASSERT_TRUE(luabridge::push(L, value));
    EXPECT_EQ(LUA_TNIL, lua_type(L, -1));
    lua_pop(L, 1);
}

TEST_F(ExpectedStackTests, GetFromValue)
{
    using ExpectedInt = luabridge::Expected<int, std::error_code>;

    lua_pushinteger(L, 100);
    auto result = luabridge::Stack<ExpectedInt>::get(L, -1);
    ASSERT_TRUE(result);
    ASSERT_TRUE((*result).hasValue());
    EXPECT_EQ(100, (*result).value());
    lua_pop(L, 1);
}

TEST_F(ExpectedStackTests, GetFromNil)
{
    using ExpectedInt = luabridge::Expected<int, std::error_code>;

    lua_pushnil(L);
    auto result = luabridge::Stack<ExpectedInt>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
    lua_pop(L, 1);
}

TEST_F(ExpectedStackTests, IsInstance)
{
    using ExpectedInt = luabridge::Expected<int, std::error_code>;

    lua_pushinteger(L, 1);
    EXPECT_TRUE((luabridge::Stack<ExpectedInt>::isInstance(L, -1)));
    lua_pop(L, 1);

    lua_pushnil(L);
    EXPECT_FALSE((luabridge::Stack<ExpectedInt>::isInstance(L, -1)));
    lua_pop(L, 1);
}

TEST_F(ExpectedStackTests, PushWithStringValue)
{
    using ExpectedString = luabridge::Expected<std::string, std::error_code>;

    ExpectedString value(std::string("hello"));
    ASSERT_TRUE(luabridge::push(L, value));

    auto result = luabridge::Stack<std::string>::get(L, -1);
    ASSERT_TRUE(result);
    EXPECT_EQ("hello", *result);
    lua_pop(L, 1);
}
