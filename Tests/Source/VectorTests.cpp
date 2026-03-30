// https://github.com/kunitoki/LuaBridge3
// Copyright 2019, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"
#include "TestTypes.h"

#include "LuaBridge/Vector.h"

#include <vector>

namespace {
template <class T>
void checkEquals(const std::vector<T>& expected, const std::vector<T>& actual)
{
    using U = std::decay_t<T>;

    if constexpr (std::is_same_v<U, float>)
    {
        for (std::size_t i = 0; i < expected.size(); ++i)
            ASSERT_FLOAT_EQ(expected[i], actual[i]);
    }
    else if constexpr (std::is_same_v<U, double> || std::is_same_v<U, long double>)
    {
        for (std::size_t i = 0; i < expected.size(); ++i)
            ASSERT_DOUBLE_EQ(expected[i], actual[i]);
    }
    else if constexpr (std::is_same_v<U, const char*>)
    {
        for (std::size_t i = 0; i < expected.size(); ++i)
            ASSERT_STREQ(expected[i], actual[i]);
    }
    else
    {
        ASSERT_EQ(expected, actual);
    }
}
} // namespace

template<class T>
struct VectorTest : TestBase
{
};

TYPED_TEST_SUITE_P(VectorTest);

TYPED_TEST_P(VectorTest, LuaRef)
{
    using Traits = TypeTraits<TypeParam>;

    this->runLua("result = {" + Traits::list() + "}");

    std::vector<TypeParam> expected(Traits::values());
    std::vector<TypeParam> actual = this->result();

    checkEquals(expected, actual);
}

REGISTER_TYPED_TEST_SUITE_P(VectorTest, LuaRef);

INSTANTIATE_TYPED_TEST_SUITE_P(VectorTest, VectorTest, TestTypes);

namespace {
struct Data
{
    /* explicit */ Data(int i) : i(i) {}

    int i;
};

bool operator==(const Data& lhs, const Data& rhs)
{
    return lhs.i == rhs.i;
}

std::ostream& operator<<(std::ostream& lhs, const Data& rhs)
{
    lhs << "{" << rhs.i << "}";
    return lhs;
}

std::vector<Data> processValues(const std::vector<Data>& data)
{
    return data;
}

std::vector<Data> processPointers(const std::vector<const Data*>& data)
{
    std::vector<Data> result;
    for (const auto* item : data)
    {
        result.emplace_back(*item);
    }
    return result;
}
} // namespace

struct VectorTests : TestBase
{
};

TEST_F(VectorTests, PassFromLua)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Data>("Data")
        .addConstructor<void (*)(int)>()
        .endClass()
        .addFunction("processValues", &processValues)
        .addFunction("processPointers", &processPointers);

    resetResult();
    runLua("result = processValues ({Data (-1), Data (2)})");

    ASSERT_EQ(std::vector<Data>({-1, 2}), result<std::vector<Data>>());

    resetResult();
    runLua("result = processPointers ({Data (-3), Data (4)})");

    ASSERT_EQ(std::vector<Data>({-3, 4}), result<std::vector<Data>>());
}

TEST_F(VectorTests, GetNonTable)
{
    lua_pushnumber(L, 42.0);

    auto result = luabridge::Stack<std::vector<int>>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(VectorTests, GetWithInvalidItem)
{
    lua_createtable(L, 2, 0);
    lua_pushinteger(L, 1);
    lua_pushstring(L, "not_an_int");
    lua_settable(L, -3);
    lua_pushinteger(L, 2);
    lua_pushstring(L, "also_not_an_int");
    lua_settable(L, -3);

    auto result = luabridge::Stack<std::vector<int>>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(VectorTests, IsInstance)
{
    ASSERT_TRUE((luabridge::push(L, std::vector<int>{ 1, 2, 3 })));
    EXPECT_TRUE(luabridge::isInstance<std::vector<int>>(L, -1));

    lua_pop(L, 1);

    ASSERT_TRUE((luabridge::push(L, 1)));
    EXPECT_FALSE(luabridge::isInstance<std::vector<int>>(L, -1));
}

TEST_F(VectorTests, StackOverflow)
{
    exhaustStackSpace();

    std::vector<int> value{ 1, 2, 3 };

    ASSERT_FALSE(luabridge::push(L, value));
}

#if !LUABRIDGE_HAS_EXCEPTIONS
TEST_F(VectorTests, PushUnregisteredWithNoExceptionsShouldFailButRestoreStack)
{
    class Unregistered {};
    
    const int initialStackSize = lua_gettop(L);

    lua_pushnumber(L, 1);
    EXPECT_EQ(1, lua_gettop(L) - initialStackSize);

    std::vector<Unregistered> v;
    v.emplace_back();
    v.emplace_back();
    
    auto result = luabridge::Stack<decltype(v)>::push(L, v);
    EXPECT_FALSE(result);

    EXPECT_EQ(1, lua_gettop(L) - initialStackSize);

    lua_pop(L, 1);
    EXPECT_EQ(0, lua_gettop(L) - initialStackSize);
}
#endif
