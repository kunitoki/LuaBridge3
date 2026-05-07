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

#endif // LUABRIDGE_HAS_CXX17_FILESYSTEM
