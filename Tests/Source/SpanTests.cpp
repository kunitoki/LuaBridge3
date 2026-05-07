// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, kunitoki
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include "LuaBridge/Span.h"
#include "LuaBridge/Vector.h"

#if LUABRIDGE_HAS_CXX20_SPAN

#include <span>
#include <vector>

struct SpanTests : TestBase
{
};

TEST_F(SpanTests, PushIntSpan)
{
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::span<int> span(data);

    ASSERT_TRUE(luabridge::push(L, span));
    EXPECT_TRUE(lua_istable(L, -1));

    auto result = luabridge::Stack<std::vector<int>>::get(L, -1);
    ASSERT_TRUE(result);
    EXPECT_EQ(data, *result);
    lua_pop(L, 1);
}

TEST_F(SpanTests, PushConstSpan)
{
    const std::vector<int> data = {10, 20, 30};
    std::span<const int> span(data);

    ASSERT_TRUE(luabridge::push(L, span));

    auto result = luabridge::Stack<std::vector<int>>::get(L, -1);
    ASSERT_TRUE(result);
    EXPECT_EQ(std::vector<int>({10, 20, 30}), *result);
    lua_pop(L, 1);
}

TEST_F(SpanTests, PushEmptySpan)
{
    std::vector<int> data;
    std::span<int> span(data);

    ASSERT_TRUE(luabridge::push(L, span));
    EXPECT_TRUE(lua_istable(L, -1));

    auto result = luabridge::Stack<std::vector<int>>::get(L, -1);
    ASSERT_TRUE(result);
    EXPECT_TRUE(result->empty());
    lua_pop(L, 1);
}

TEST_F(SpanTests, IsInstance)
{
    std::vector<int> data = {1, 2, 3};
    ASSERT_TRUE(luabridge::push(L, std::span<int>(data)));
    EXPECT_TRUE((luabridge::Stack<std::span<int>>::isInstance(L, -1)));
    lua_pop(L, 1);

    lua_pushnumber(L, 1.0);
    EXPECT_FALSE((luabridge::Stack<std::span<int>>::isInstance(L, -1)));
    lua_pop(L, 1);
}

TEST_F(SpanTests, StackOverflow)
{
    exhaustStackSpace();

    std::vector<int> data = {1, 2, 3};
    std::span<int> span(data);

    ASSERT_FALSE(luabridge::push(L, span));
}

#endif // LUABRIDGE_HAS_CXX20_SPAN
