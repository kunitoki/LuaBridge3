// https://github.com/kunitoki/LuaBridge3
// Copyright 2026, kunitoki
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include "LuaBridge/Variant.h"

#include <string>
#include <variant>

namespace {
using TestVariant = std::variant<int, std::string>;

std::string describeVariant(const TestVariant& value)
{
    if (const auto* intValue = std::get_if<int>(&value))
        return "int:" + std::to_string(*intValue);

    return "string:" + std::get<std::string>(value);
}

TestVariant echoVariant(const TestVariant& value)
{
    return value;
}
} // namespace

struct VariantTests : TestBase
{
};

TEST_F(VariantTests, PushIntAlternative)
{
    TestVariant value = 42;

    ASSERT_TRUE(luabridge::push(L, value));

    EXPECT_EQ(LUA_TNUMBER, lua_type(L, -1));
    EXPECT_EQ(42, lua_tointeger(L, -1));
}

TEST_F(VariantTests, PushStringAlternative)
{
    TestVariant value = std::string("hello");

    ASSERT_TRUE(luabridge::push(L, value));

    EXPECT_EQ(LUA_TSTRING, lua_type(L, -1));
    EXPECT_STREQ("hello", lua_tostring(L, -1));
}

TEST_F(VariantTests, GetIntAlternative)
{
    lua_pushinteger(L, 42);

    auto result = luabridge::Stack<TestVariant>::get(L, -1);

    ASSERT_TRUE(result);
    ASSERT_TRUE(std::holds_alternative<int>(*result));
    EXPECT_EQ(42, std::get<int>(*result));
}

TEST_F(VariantTests, GetStringAlternative)
{
    lua_pushstring(L, "hello");

    auto result = luabridge::Stack<TestVariant>::get(L, -1);

    ASSERT_TRUE(result);
    ASSERT_TRUE(std::holds_alternative<std::string>(*result));
    EXPECT_EQ("hello", std::get<std::string>(*result));
}

TEST_F(VariantTests, GetInvalidType)
{
    lua_createtable(L, 0, 0);

    auto result = luabridge::Stack<TestVariant>::get(L, -1);

    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(VariantTests, GetMonostateAlternative)
{
    using MaybeInt = std::variant<std::monostate, int>;

    lua_pushnil(L);

    auto result = luabridge::Stack<MaybeInt>::get(L, -1);

    ASSERT_TRUE(result);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(*result));
}

TEST_F(VariantTests, PushMonostateAlternative)
{
    using MaybeInt = std::variant<std::monostate, int>;

    MaybeInt value = std::monostate{};

    ASSERT_TRUE(luabridge::push(L, value));
    EXPECT_TRUE(lua_isnil(L, -1));
}

TEST_F(VariantTests, IsInstance)
{
    lua_pushinteger(L, 42);
    EXPECT_TRUE(luabridge::isInstance<TestVariant>(L, -1));

    lua_pop(L, 1);
    lua_pushstring(L, "hello");
    EXPECT_TRUE(luabridge::isInstance<TestVariant>(L, -1));

    lua_pop(L, 1);
    lua_createtable(L, 0, 0);
    EXPECT_FALSE(luabridge::isInstance<TestVariant>(L, -1));
}

TEST_F(VariantTests, StackOverflow)
{
    exhaustStackSpace();

    TestVariant value = 42;

    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(VariantTests, PassFromLua)
{
    luabridge::getGlobalNamespace(L)
        .addFunction("describeVariant", &describeVariant);

    resetResult();
    runLua("result = describeVariant(42)");

    EXPECT_EQ("int:42", result<std::string>());

    resetResult();
    runLua("result = describeVariant('hello')");

    EXPECT_EQ("string:hello", result<std::string>());
}

TEST_F(VariantTests, ReturnToLua)
{
    luabridge::getGlobalNamespace(L)
        .addFunction("echoVariant", &echoVariant);

    resetResult();
    runLua("result = echoVariant(42)");

    EXPECT_EQ(42, result<int>());

    resetResult();
    runLua("result = echoVariant('hello')");

    EXPECT_EQ("hello", result<std::string>());
}
