// https://github.com/kunitoki/LuaBridge3
// Copyright 2019, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"
#include "TestTypes.h"

#include "LuaBridge/ForwardList.h"

#include <forward_list>

namespace {
template <class T>
std::forward_list<T> toForwardList(const std::vector<T>& vector)
{
    return {vector.begin(), vector.end()};
}

template <class T>
void checkEquals(const std::forward_list<T>& expected, const std::forward_list<T>& actual)
{
    using U = std::decay_t<T>;

    if constexpr (std::is_same_v<U, float>)
    {
        auto expectedIt = expected.begin();
        auto actualIt = actual.begin();
        while (expectedIt != expected.end() && actualIt != actual.end())
        {
            ASSERT_FLOAT_EQ((*expectedIt), (*actualIt));
            ++expectedIt;
            ++actualIt;
        }
        ASSERT_EQ(expectedIt, expected.end());
        ASSERT_EQ(actualIt, actual.end());
    }
    else if constexpr (std::is_same_v<U, double> || std::is_same_v<U, long double>)
    {
        auto expectedIt = expected.begin();
        auto actualIt = actual.begin();
        while (expectedIt != expected.end() && actualIt != actual.end())
        {
            ASSERT_DOUBLE_EQ((*expectedIt), (*actualIt));
            ++expectedIt;
            ++actualIt;
        }
        ASSERT_EQ(expectedIt, expected.end());
        ASSERT_EQ(actualIt, actual.end());
    }
    else if constexpr (std::is_same_v<U, const char*>)
    {
        auto expectedIt = expected.begin();
        auto actualIt = actual.begin();
        while (expectedIt != expected.end() && actualIt != actual.end())
        {
            ASSERT_STREQ((*expectedIt), (*actualIt));
            ++expectedIt;
            ++actualIt;
        }
        ASSERT_EQ(expectedIt, expected.end());
        ASSERT_EQ(actualIt, actual.end());
    }
    else
    {
        // Convert to vectors for equality comparison
        std::vector<T> expectedVec(expected.begin(), expected.end());
        std::vector<T> actualVec(actual.begin(), actual.end());
        ASSERT_EQ(expectedVec, actualVec);
    }
}
} // namespace

template<class T>
struct ForwardListTest : TestBase
{
};

TYPED_TEST_SUITE_P(ForwardListTest);

TYPED_TEST_P(ForwardListTest, LuaRef)
{
    using Traits = TypeTraits<TypeParam>;

    this->runLua("result = {" + Traits::list() + "}");

    std::forward_list<TypeParam> expected = toForwardList(Traits::values());
    std::forward_list<TypeParam> actual = this->result();

    checkEquals(expected, actual);
}

REGISTER_TYPED_TEST_SUITE_P(ForwardListTest, LuaRef);

INSTANTIATE_TYPED_TEST_SUITE_P(ForwardListTest, ForwardListTest, TestTypes);

struct ForwardListTests : TestBase
{
};

TEST_F(ForwardListTests, GetNonTable)
{
    lua_pushnumber(L, 42.0);

    auto result = luabridge::Stack<std::forward_list<int>>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(ForwardListTests, GetWithInvalidItem)
{
    lua_createtable(L, 2, 0);
    lua_pushinteger(L, 1);
    lua_pushstring(L, "not_an_int");
    lua_settable(L, -3);
    lua_pushinteger(L, 2);
    lua_pushstring(L, "also_not_an_int");
    lua_settable(L, -3);

    auto result = luabridge::Stack<std::forward_list<int>>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(ForwardListTests, PassToFunction)
{
    runLua("function foo (forwardList) "
           "  result = forwardList "
           "end");

    auto foo = luabridge::getGlobal(L, "foo");

    resetResult();

    std::forward_list<int> lvalue{ 10, 20, 30 };
    ASSERT_TRUE(foo.call(lvalue));
    ASSERT_TRUE(result().isTable());

    std::vector<int> expectedVec(lvalue.begin(), lvalue.end());
    std::forward_list<int> actualForwardList = result<std::forward_list<int>>();
    std::vector<int> actualVec(actualForwardList.begin(), actualForwardList.end());
    ASSERT_EQ(expectedVec, actualVec);

    resetResult();

    const std::forward_list<int> constLvalue = lvalue;
    ASSERT_TRUE(foo.call(constLvalue));
    ASSERT_TRUE(result().isTable());

    std::forward_list<int> actualForwardList2 = result<std::forward_list<int>>();
    std::vector<int> actualVec2(actualForwardList2.begin(), actualForwardList2.end());
    ASSERT_EQ(expectedVec, actualVec2);
}

TEST_F(ForwardListTests, UnregisteredClass)
{
    struct Unregistered {};

#if LUABRIDGE_HAS_EXCEPTIONS
    [[maybe_unused]] luabridge::Result r;
    ASSERT_THROW((r = luabridge::push(L, std::forward_list<Unregistered>{ Unregistered() })), std::exception);
#else
    ASSERT_FALSE((luabridge::push(L, std::forward_list<Unregistered>{ Unregistered() })));
#endif
}

TEST_F(ForwardListTests, IsInstance)
{
    ASSERT_TRUE((luabridge::push(L, std::forward_list<int>{ 1, 2, 3 })));
    EXPECT_TRUE(luabridge::isInstance<std::forward_list<int>>(L, -1));

    lua_pop(L, 1);

    ASSERT_TRUE((luabridge::push(L, 1)));
    EXPECT_FALSE(luabridge::isInstance<std::forward_list<int>>(L, -1));
}

TEST_F(ForwardListTests, StackOverflow)
{
    exhaustStackSpace();

    std::forward_list<int> value = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    ASSERT_FALSE(luabridge::push(L, value));
}

#if !LUABRIDGE_HAS_EXCEPTIONS
TEST_F(ForwardListTests, PushUnregisteredWithNoExceptionsShouldFailButRestoreStack)
{
    class Unregistered {};

    const int initialStackSize = lua_gettop(L);

    lua_pushnumber(L, 1);
    EXPECT_EQ(1, lua_gettop(L) - initialStackSize);

    std::forward_list<Unregistered> v;
    v.emplace_front();
    v.emplace_front();

    auto result = luabridge::Stack<decltype(v)>::push(L, v);
    EXPECT_FALSE(result);

    EXPECT_EQ(1, lua_gettop(L) - initialStackSize);

    lua_pop(L, 1);
    EXPECT_EQ(0, lua_gettop(L) - initialStackSize);
}
#endif
