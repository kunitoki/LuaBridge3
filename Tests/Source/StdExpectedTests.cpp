// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, kunitoki
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include "LuaBridge/StdExpected.h"

#if LUABRIDGE_HAS_CXX23_EXPECTED

#include <expected>
#include <string>
#include <system_error>

struct StdExpectedTests : TestBase
{
};

TEST_F(StdExpectedTests, PushWithValue)
{
    using StdExpectedInt = std::expected<int, std::error_code>;

    StdExpectedInt value(42);
    ASSERT_TRUE(luabridge::push(L, value));
    EXPECT_EQ(LUA_TNUMBER, lua_type(L, -1));

    auto result = luabridge::Stack<int>::get(L, -1);
    ASSERT_TRUE(result);
    EXPECT_EQ(42, *result);
    lua_pop(L, 1);
}

TEST_F(StdExpectedTests, PushWithError)
{
    using StdExpectedInt = std::expected<int, std::error_code>;

    StdExpectedInt value(std::unexpected(std::make_error_code(std::errc::invalid_argument)));
    ASSERT_TRUE(luabridge::push(L, value));
    EXPECT_EQ(LUA_TNIL, lua_type(L, -1));
    lua_pop(L, 1);
}

TEST_F(StdExpectedTests, GetFromValue)
{
    using StdExpectedInt = std::expected<int, std::error_code>;

    lua_pushinteger(L, 100);
    auto result = luabridge::Stack<StdExpectedInt>::get(L, -1);
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->has_value());
    EXPECT_EQ(100, result->value());
    lua_pop(L, 1);
}

TEST_F(StdExpectedTests, GetFromNil)
{
    using StdExpectedInt = std::expected<int, std::error_code>;

    lua_pushnil(L);
    auto result = luabridge::Stack<StdExpectedInt>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
    lua_pop(L, 1);
}

TEST_F(StdExpectedTests, IsInstance)
{
    using StdExpectedInt = std::expected<int, std::error_code>;

    lua_pushinteger(L, 1);
    EXPECT_TRUE((luabridge::Stack<StdExpectedInt>::isInstance(L, -1)));
    lua_pop(L, 1);

    lua_pushnil(L);
    EXPECT_FALSE((luabridge::Stack<StdExpectedInt>::isInstance(L, -1)));
    lua_pop(L, 1);
}

TEST_F(StdExpectedTests, PushWithStringValue)
{
    using StdExpectedString = std::expected<std::string, std::error_code>;

    StdExpectedString value(std::string("hello"));
    ASSERT_TRUE(luabridge::push(L, value));

    auto result = luabridge::Stack<std::string>::get(L, -1);
    ASSERT_TRUE(result);
    EXPECT_EQ("hello", *result);
    lua_pop(L, 1);
}

#endif // LUABRIDGE_HAS_CXX23_EXPECTED
