// https://github.com/kunitoki/LuaBridge3
// Copyright 2019, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"
#include "TestTypes.h"

#include "LuaBridge/List.h"

#include <list>

namespace {
template <class T>
std::list<T> toList(const std::vector<T>& vector)
{
    return {vector.begin(), vector.end()};
}

template <class T>
void checkEquals(const std::list<T>& expected, const std::list<T>& actual)
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
struct ListTest : TestBase
{
};

TYPED_TEST_SUITE_P(ListTest);

TYPED_TEST_P(ListTest, LuaRef)
{
    using Traits = TypeTraits<TypeParam>;

    this->runLua("result = {" + Traits::list() + "}");

    std::list<TypeParam> expected = toList(Traits::values());
    std::list<TypeParam> actual = this->result();

    checkEquals(expected, actual);
}

REGISTER_TYPED_TEST_SUITE_P(ListTest, LuaRef);

INSTANTIATE_TYPED_TEST_SUITE_P(ListTest, ListTest, TestTypes);

struct ListTests : TestBase
{
};

TEST_F(ListTests, PassToFunction)
{
    runLua("function foo (list) "
           "  result = list "
           "end");

    auto foo = luabridge::getGlobal(L, "foo");

    resetResult();

    std::list<int> lvalue{ 10, 20, 30 };
    foo(lvalue);
    ASSERT_TRUE(result().isTable());
    ASSERT_EQ(lvalue, result<std::list<int>>());

    resetResult();

    const std::list<int> constLvalue = lvalue;
    foo(constLvalue);
    ASSERT_TRUE(result().isTable());
    ASSERT_EQ(lvalue, result<std::list<int>>());
}

TEST_F(ListTests, UnregisteredClass)
{
    struct Unregistered {};
    
#if LUABRIDGE_HAS_EXCEPTIONS
    [[maybe_unused]] luabridge::Result r;
    ASSERT_THROW((r = luabridge::push(L, std::list<Unregistered>{ Unregistered() })), std::exception);
#else
    ASSERT_FALSE((luabridge::push(L, std::list<Unregistered>{ Unregistered() })));
#endif
}

TEST_F(ListTests, IsInstance)
{
    ASSERT_TRUE((luabridge::push(L, std::list<int>{ 1, 2, 3 })));
    EXPECT_TRUE(luabridge::isInstance<std::list<int>>(L, -1));
    
    lua_pop(L, 1);
    
    ASSERT_TRUE((luabridge::push(L, 1)));
    EXPECT_FALSE(luabridge::isInstance<std::list<int>>(L, -1));
}

TEST_F(ListTests, StackOverflow)
{
    exhaustStackSpace();
    
    std::list<int> value = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    
    ASSERT_FALSE(luabridge::push(L, value));
}

#if !LUABRIDGE_HAS_EXCEPTIONS
TEST_F(ListTests, PushUnregisteredWithNoExceptionsShouldFailButRestoreStack)
{
    class Unregistered {};

    const int initialStackSize = lua_gettop(L);

    lua_pushnumber(L, 1);
    EXPECT_EQ(1, lua_gettop(L) - initialStackSize);

    std::list<Unregistered> v;
    v.emplace_back();
    v.emplace_back();

    auto result = luabridge::Stack<decltype(v)>::push(L, v);
    EXPECT_FALSE(result);

    EXPECT_EQ(1, lua_gettop(L) - initialStackSize);

    lua_pop(L, 1);
    EXPECT_EQ(0, lua_gettop(L) - initialStackSize);
}
#endif
