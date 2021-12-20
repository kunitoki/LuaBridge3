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
    if constexpr (std::is_same_v<T, float>)
    {
        for (std::size_t i = 0; i < expected.size(); ++i)
            ASSERT_FLOAT_EQ((*std::next(expected.begin(), i)), (*std::next(actual.begin(), i)));
    }
    else if constexpr (std::is_same_v<T, double> || std::is_same_v<T, long double>)
    {
        for (std::size_t i = 0; i < expected.size(); ++i)
            ASSERT_DOUBLE_EQ((*std::next(expected.begin(), i)), (*std::next(actual.begin(), i)));
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
    
    std::error_code ec;
#if LUABRIDGE_HAS_EXCEPTIONS
    bool result;
    ASSERT_THROW((result = luabridge::push(L, std::list<Unregistered>{ Unregistered() }, ec)), std::exception);
#else
    ASSERT_FALSE((luabridge::push(L, std::list<Unregistered>{ Unregistered() }, ec)));
#endif
}

TEST_F(ListTests, IsInstance)
{
    std::error_code ec;

    ASSERT_TRUE((luabridge::push(L, std::list<int>{ 1, 2, 3 }, ec)));
    EXPECT_TRUE(luabridge::isInstance<std::list<int>>(L, -1));
    
    lua_pop(L, 1);
    
    ASSERT_TRUE((luabridge::push(L, 1, ec)));
    EXPECT_FALSE(luabridge::isInstance<std::list<int>>(L, -1));
}

TEST_F(ListTests, StackOverflow)
{
    exhaustStackSpace();
    
    std::list<int> value = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}
