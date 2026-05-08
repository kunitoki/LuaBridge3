// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, kunitoki
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include "LuaBridge/detail/Config.h"

#if LUABRIDGE_HAS_CXX17_FILESYSTEM

#include <filesystem>

struct FilesystemTests : TestBase
{
};

TEST_F(FilesystemTests, PushAndGet)
{
    std::filesystem::path p("/some/path/to/file.txt");

    ASSERT_TRUE(luabridge::push(L, p));

    auto result = luabridge::Stack<std::filesystem::path>::get(L, -1);
    ASSERT_TRUE(result);
    EXPECT_EQ(p, *result);
    lua_pop(L, 1);
}

TEST_F(FilesystemTests, GetNonString)
{
    lua_pushnumber(L, 42.0);
    auto result = luabridge::Stack<std::filesystem::path>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
    lua_pop(L, 1);
}

TEST_F(FilesystemTests, IsInstance)
{
    ASSERT_TRUE(luabridge::push(L, std::filesystem::path("/tmp/test")));
    EXPECT_TRUE(luabridge::Stack<std::filesystem::path>::isInstance(L, -1));
    lua_pop(L, 1);

    lua_pushnumber(L, 1.0);
    EXPECT_FALSE(luabridge::Stack<std::filesystem::path>::isInstance(L, -1));
    lua_pop(L, 1);
}

TEST_F(FilesystemTests, LuaRef)
{
    runLua("result = '/tmp/test.lua'");
    auto path = result<std::filesystem::path>();
    EXPECT_EQ(std::filesystem::path("/tmp/test.lua"), path);
}

TEST_F(FilesystemTests, StackOverflow)
{
    exhaustStackSpace();
    ASSERT_FALSE(luabridge::push(L, std::filesystem::path("/some/path")));
}

TEST_F(FilesystemTests, ExpectedPushAndGet)
{
    using ExpectedPath = luabridge::Expected<std::filesystem::path, std::error_code>;

    std::filesystem::path p("/some/path/to/file.txt");
    ExpectedPath value(p);
    ASSERT_TRUE(luabridge::push(L, value));
    EXPECT_EQ(LUA_TSTRING, lua_type(L, -1));

    auto result = luabridge::Stack<ExpectedPath>::get(L, -1);
    ASSERT_TRUE(result);
    ASSERT_TRUE((*result).hasValue());
    EXPECT_EQ(p, (*result).value());
    lua_pop(L, 1);
}

TEST_F(FilesystemTests, ExpectedPushError)
{
    using ExpectedPath = luabridge::Expected<std::filesystem::path, std::error_code>;

    ExpectedPath value(luabridge::makeUnexpected(luabridge::makeErrorCode(luabridge::ErrorCode::InvalidTypeCast)));
    ASSERT_TRUE(luabridge::push(L, value));
    EXPECT_EQ(LUA_TNIL, lua_type(L, -1));
    lua_pop(L, 1);
}

TEST_F(FilesystemTests, ExpectedGetFromNil)
{
    using ExpectedPath = luabridge::Expected<std::filesystem::path, std::error_code>;

    lua_pushnil(L);
    auto result = luabridge::Stack<ExpectedPath>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
    lua_pop(L, 1);
}

TEST_F(FilesystemTests, ExpectedGetInnerTypeMismatch)
{
    using ExpectedPath = luabridge::Expected<std::filesystem::path, std::error_code>;

    lua_pushnumber(L, 42.0);
    auto result = luabridge::Stack<ExpectedPath>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
    lua_pop(L, 1);
}

#endif // LUABRIDGE_HAS_CXX17_FILESYSTEM
