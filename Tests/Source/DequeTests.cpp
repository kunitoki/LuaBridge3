// https://github.com/kunitoki/LuaBridge3
// Copyright 2019, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"
#include "TestTypes.h"

#include "LuaBridge/Deque.h"

#include <deque>

namespace {
template <class T>
std::deque<T> toDeque(const std::vector<T>& vector)
{
    return {vector.begin(), vector.end()};
}

template <class T>
void checkEquals(const std::deque<T>& expected, const std::deque<T>& actual)
{
    using U = std::decay_t<T>;

    if constexpr (std::is_same_v<U, float>)
    {
        for (std::size_t i = 0; i < expected.size(); ++i)
            ASSERT_FLOAT_EQ((*std::next(expected.begin(), i)), (*std::next(actual.begin(), i)));
    }
    else if constexpr (std::is_same_v<U, double> || std::is_same_v<U, long double>)
    {
        for (std::size_t i = 0; i < expected.size(); ++i)
            ASSERT_DOUBLE_EQ((*std::next(expected.begin(), i)), (*std::next(actual.begin(), i)));
    }
    else if constexpr (std::is_same_v<U, const char*>)
    {
        for (std::size_t i = 0; i < expected.size(); ++i)
            ASSERT_STREQ((*std::next(expected.begin(), i)), (*std::next(actual.begin(), i)));
    }
    else
    {
        ASSERT_EQ(expected, actual);
    }
}
} // namespace

template<class T>
struct DequeTest : TestBase
{
};

TYPED_TEST_SUITE_P(DequeTest);

TYPED_TEST_P(DequeTest, LuaRef)
{
    using Traits = TypeTraits<TypeParam>;

    this->runLua("result = {" + Traits::list() + "}");

    std::deque<TypeParam> expected = toDeque(Traits::values());
    std::deque<TypeParam> actual = this->result();

    checkEquals(expected, actual);
}

REGISTER_TYPED_TEST_SUITE_P(DequeTest, LuaRef);

INSTANTIATE_TYPED_TEST_SUITE_P(DequeTest, DequeTest, TestTypes);

struct DequeTests : TestBase
{
};

TEST_F(DequeTests, GetNonTable)
{
    lua_pushnumber(L, 42.0);

    auto result = luabridge::Stack<std::deque<int>>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(DequeTests, GetWithInvalidItem)
{
    lua_createtable(L, 2, 0);
    lua_pushinteger(L, 1);
    lua_pushstring(L, "not_an_int");
    lua_settable(L, -3);
    lua_pushinteger(L, 2);
    lua_pushstring(L, "also_not_an_int");
    lua_settable(L, -3);

    auto result = luabridge::Stack<std::deque<int>>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(DequeTests, PassToFunction)
{
    runLua("function foo (deque) "
           "  result = deque "
           "end");

    auto foo = luabridge::getGlobal(L, "foo");

    resetResult();

    std::deque<int> lvalue{ 10, 20, 30 };
    ASSERT_TRUE(foo.call(lvalue));
    ASSERT_TRUE(result().isTable());
    ASSERT_EQ(lvalue, result<std::deque<int>>());

    resetResult();

    const std::deque<int> constLvalue = lvalue;
    ASSERT_TRUE(foo.call(constLvalue));
    ASSERT_TRUE(result().isTable());
    ASSERT_EQ(lvalue, result<std::deque<int>>());
}

TEST_F(DequeTests, UnregisteredClass)
{
    struct Unregistered {};

#if LUABRIDGE_HAS_EXCEPTIONS
    [[maybe_unused]] luabridge::Result r;
    ASSERT_THROW((r = luabridge::push(L, std::deque<Unregistered>{ Unregistered() })), std::exception);
#else
    ASSERT_FALSE((luabridge::push(L, std::deque<Unregistered>{ Unregistered() })));
#endif
}

TEST_F(DequeTests, IsInstance)
{
    ASSERT_TRUE((luabridge::push(L, std::deque<int>{ 1, 2, 3 })));
    EXPECT_TRUE(luabridge::isInstance<std::deque<int>>(L, -1));

    lua_pop(L, 1);

    ASSERT_TRUE((luabridge::push(L, 1)));
    EXPECT_FALSE(luabridge::isInstance<std::deque<int>>(L, -1));
}

TEST_F(DequeTests, StackOverflow)
{
    exhaustStackSpace();

    std::deque<int> value = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    ASSERT_FALSE(luabridge::push(L, value));
}

#if !LUABRIDGE_HAS_EXCEPTIONS
TEST_F(DequeTests, PushUnregisteredWithNoExceptionsShouldFailButRestoreStack)
{
    class Unregistered {};

    const int initialStackSize = lua_gettop(L);

    lua_pushnumber(L, 1);
    EXPECT_EQ(1, lua_gettop(L) - initialStackSize);

    std::deque<Unregistered> v;
    v.emplace_back();
    v.emplace_back();

    auto result = luabridge::Stack<decltype(v)>::push(L, v);
    EXPECT_FALSE(result);

    EXPECT_EQ(1, lua_gettop(L) - initialStackSize);

    lua_pop(L, 1);
    EXPECT_EQ(0, lua_gettop(L) - initialStackSize);
}
#endif
