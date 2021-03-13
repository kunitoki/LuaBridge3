// https://github.com/kunitoki/LuaBridge3
// Copyright 2019, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"
#include "TestTypes.h"

#include "LuaBridge/Vector.h"

#include <vector>

template<class T>
struct VectorTest : TestBase
{
};

TYPED_TEST_CASE_P(VectorTest);

TYPED_TEST_P(VectorTest, LuaRef)
{
    using Traits = TypeTraits<TypeParam>;

    this->runLua("result = {" + Traits::list() + "}");

    std::vector<TypeParam> expected(Traits::values());
    std::vector<TypeParam> actual = this->result();
    ASSERT_EQ(expected, actual);
}

REGISTER_TYPED_TEST_CASE_P(VectorTest, LuaRef);

INSTANTIATE_TYPED_TEST_CASE_P(VectorTest, VectorTest, TestTypes);

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
    runLua("result = processValues ({Data (-3), Data (4)})");

    ASSERT_EQ(std::vector<Data>({-3, 4}), result<std::vector<Data>>());
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
    
    std::error_code ec;
    auto result = luabridge::Stack<decltype(v)>::push(L, v, ec);
    EXPECT_FALSE(result);

    EXPECT_EQ(1, lua_gettop(L) - initialStackSize);

    lua_pop(L, 1);
    EXPECT_EQ(0, lua_gettop(L) - initialStackSize);
}
#endif
