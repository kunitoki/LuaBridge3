// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, kunitoki
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include "LuaBridge/Any.h"

#if LUABRIDGE_HAS_CXX17_ANY

#include <any>
#include <string>

struct AnyTests : TestBase
{
};

TEST_F(AnyTests, PushInt)
{
    luabridge::registerAnyPush<int>(L);

    std::any value = 42;
    ASSERT_TRUE(luabridge::push(L, value));

    EXPECT_EQ(42, luabridge::Stack<int>::get(L, -1).value());
    lua_pop(L, 1);
}

TEST_F(AnyTests, PushString)
{
    luabridge::registerAnyPush<std::string>(L);

    std::any value = std::string("hello");
    ASSERT_TRUE(luabridge::push(L, value));

    EXPECT_EQ("hello", luabridge::Stack<std::string>::get(L, -1).value());
    lua_pop(L, 1);
}

TEST_F(AnyTests, PushEmpty)
{
    std::any value;
    ASSERT_TRUE(luabridge::push(L, value));
    EXPECT_TRUE(lua_isnil(L, -1));
    lua_pop(L, 1);
}

TEST_F(AnyTests, PushUnregisteredType)
{
    struct MyStruct { int x; };
    std::any value = MyStruct{42};

    auto result = luabridge::push(L, value);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(AnyTests, PushDouble)
{
    luabridge::registerAnyPush<double>(L);

    std::any value = 3.14;
    ASSERT_TRUE(luabridge::push(L, value));

    auto result = luabridge::Stack<double>::get(L, -1);
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(3.14, *result);
    lua_pop(L, 1);
}

#endif // LUABRIDGE_HAS_CXX17_ANY
