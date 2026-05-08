// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, kunitoki
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include "LuaBridge/MultiMap.h"
#include "LuaBridge/UnorderedMultiMap.h"

#include <map>
#include <unordered_map>

//=================================================================================================
// MultiMapTests
//=================================================================================================

struct MultiMapTests : TestBase
{
};

TEST_F(MultiMapTests, GetNonTable)
{
    lua_pushnumber(L, 42.0);

    auto result = luabridge::Stack<std::multimap<int, int>>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(MultiMapTests, PushAndGet)
{
    using MM = std::multimap<int, std::string>;

    MM mm;
    mm.emplace(1, "a");
    mm.emplace(1, "b");
    mm.emplace(2, "c");

    ASSERT_TRUE(luabridge::push(L, mm));

    auto result = luabridge::Stack<MM>::get(L, -1);
    ASSERT_TRUE(result);
    EXPECT_EQ(mm, *result);

    lua_pop(L, 1);
}

TEST_F(MultiMapTests, PushAndGetSingleKey)
{
    using MM = std::multimap<std::string, int>;

    MM mm;
    mm.emplace("x", 10);
    mm.emplace("x", 20);
    mm.emplace("x", 30);

    ASSERT_TRUE(luabridge::push(L, mm));

    auto result = luabridge::Stack<MM>::get(L, -1);
    ASSERT_TRUE(result);
    EXPECT_EQ(mm, *result);

    lua_pop(L, 1);
}

TEST_F(MultiMapTests, PushAndGetEmpty)
{
    using MM = std::multimap<int, std::string>;

    MM mm;

    ASSERT_TRUE(luabridge::push(L, mm));

    auto result = luabridge::Stack<MM>::get(L, -1);
    ASSERT_TRUE(result);
    EXPECT_EQ(mm, *result);

    lua_pop(L, 1);
}

TEST_F(MultiMapTests, GetInvalidInnerValue)
{
    // Create outer table with key=1, inner table has a non-int value
    lua_createtable(L, 0, 1);
    lua_pushinteger(L, 1);
    lua_createtable(L, 1, 0);
    lua_pushstring(L, "not_an_int");
    lua_rawseti(L, -2, 1);
    lua_settable(L, -3);

    auto result = luabridge::Stack<std::multimap<int, int>>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(MultiMapTests, GetInvalidInnerTable)
{
    // Create outer table with key=1, value is not a table
    lua_createtable(L, 0, 1);
    lua_pushinteger(L, 1);
    lua_pushstring(L, "not_a_table");
    lua_settable(L, -3);

    auto result = luabridge::Stack<std::multimap<int, std::string>>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(MultiMapTests, IsInstance)
{
    using MM = std::multimap<std::string, int>;

    MM mm;
    mm.emplace("x", 1);
    mm.emplace("y", 2);
    mm.emplace("y", 3);

    ASSERT_TRUE(luabridge::push(L, mm));
    EXPECT_TRUE((luabridge::isInstance<MM>(L, -1)));

    lua_pop(L, 1);

    ASSERT_TRUE(luabridge::push(L, 1));
    EXPECT_FALSE((luabridge::isInstance<MM>(L, -1)));
}

TEST_F(MultiMapTests, StackOverflow)
{
    exhaustStackSpace();

    std::multimap<std::string, int> mm;
    mm.emplace("x", 1);
    mm.emplace("y", 2);
    mm.emplace("y", 3);

    ASSERT_FALSE(luabridge::push(L, mm));
}

//=================================================================================================
// UnorderedMultiMapTests
//=================================================================================================

struct UnorderedMultiMapTests : TestBase
{
};

TEST_F(UnorderedMultiMapTests, GetNonTable)
{
    lua_pushnumber(L, 42.0);

    auto result = luabridge::Stack<std::unordered_multimap<int, int>>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(UnorderedMultiMapTests, PushAndGet)
{
    using UMM = std::unordered_multimap<int, std::string>;

    UMM umm;
    umm.emplace(1, "a");
    umm.emplace(1, "b");
    umm.emplace(2, "c");

    ASSERT_TRUE(luabridge::push(L, umm));

    auto result = luabridge::Stack<UMM>::get(L, -1);
    ASSERT_TRUE(result);
    EXPECT_EQ(umm, *result);

    lua_pop(L, 1);
}

TEST_F(UnorderedMultiMapTests, PushAndGetSingleKey)
{
    using UMM = std::unordered_multimap<std::string, int>;

    UMM umm;
    umm.emplace("x", 10);
    umm.emplace("x", 20);
    umm.emplace("x", 30);

    ASSERT_TRUE(luabridge::push(L, umm));

    auto result = luabridge::Stack<UMM>::get(L, -1);
    ASSERT_TRUE(result);
    EXPECT_EQ(umm, *result);

    lua_pop(L, 1);
}

TEST_F(UnorderedMultiMapTests, PushAndGetEmpty)
{
    using UMM = std::unordered_multimap<int, std::string>;

    UMM umm;

    ASSERT_TRUE(luabridge::push(L, umm));

    auto result = luabridge::Stack<UMM>::get(L, -1);
    ASSERT_TRUE(result);
    EXPECT_EQ(umm, *result);

    lua_pop(L, 1);
}

TEST_F(UnorderedMultiMapTests, GetInvalidInnerValue)
{
    // Create outer table with key=1, inner table has a non-int value
    lua_createtable(L, 0, 1);
    lua_pushinteger(L, 1);
    lua_createtable(L, 1, 0);
    lua_pushstring(L, "not_an_int");
    lua_rawseti(L, -2, 1);
    lua_settable(L, -3);

    auto result = luabridge::Stack<std::unordered_multimap<int, int>>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(UnorderedMultiMapTests, GetInvalidInnerTable)
{
    // Create outer table with key=1, value is not a table
    lua_createtable(L, 0, 1);
    lua_pushinteger(L, 1);
    lua_pushstring(L, "not_a_table");
    lua_settable(L, -3);

    auto result = luabridge::Stack<std::unordered_multimap<int, std::string>>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(UnorderedMultiMapTests, IsInstance)
{
    using UMM = std::unordered_multimap<std::string, int>;

    UMM umm;
    umm.emplace("x", 1);
    umm.emplace("y", 2);
    umm.emplace("y", 3);

    ASSERT_TRUE(luabridge::push(L, umm));
    EXPECT_TRUE((luabridge::isInstance<UMM>(L, -1)));

    lua_pop(L, 1);

    ASSERT_TRUE(luabridge::push(L, 1));
    EXPECT_FALSE((luabridge::isInstance<UMM>(L, -1)));
}

TEST_F(UnorderedMultiMapTests, StackOverflow)
{
    exhaustStackSpace();

    std::unordered_multimap<std::string, int> umm;
    umm.emplace("x", 1);
    umm.emplace("y", 2);
    umm.emplace("y", 3);

    ASSERT_FALSE(luabridge::push(L, umm));
}
