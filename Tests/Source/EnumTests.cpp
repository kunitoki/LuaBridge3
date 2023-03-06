// https://github.com/kunitoki/LuaBridge3
// Copyright 2023, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include <string>

namespace {
enum A
{
    A_ZERO,
    A_ONE,
    A_TWO,
    A_THREE
};

enum class B
{
    B_ZERO,
    B_ONE,
    B_TWO,
    B_THREE
};

enum C
{
    C_ZERO,
    C_ONE,
    C_TWO,
    C_THREE
};

enum class D
{
    D_ZERO,
    D_ONE,
    D_TWO,
    D_THREE
};

enum E
{
    E_ZERO,
    E_ONE,
    E_TWO,
    E_THREE
};

enum class F
{
    F_ZERO,
    F_ONE,
    F_TWO,
    F_THREE
};
} // namespace

struct EnumTests : TestBase
{
};

template <>
struct luabridge::Stack<C> : luabridge::Enum<C>
{
};

template <>
struct luabridge::Stack<D> : luabridge::Enum<D>
{
};

template <>
struct luabridge::Stack<E> : luabridge::EnumType<E>
{
};

template <>
struct luabridge::Stack<F> : luabridge::EnumType<F>
{
};

TEST_F(EnumTests, Unregistered)
{
#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_ANY_THROW([[maybe_unused]] auto result = luabridge::push(L, A::A_ZERO));
#else
    ASSERT_FALSE(luabridge::push(L, A::A_ZERO));
#endif

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_ANY_THROW([[maybe_unused]] auto result = luabridge::push(L, B::B_ZERO));
#else
    ASSERT_FALSE(luabridge::push(L, B::B_ZERO));
#endif
}

TEST_F(EnumTests, RegisteredStack)
{
    {
        ASSERT_TRUE(luabridge::push(L, C::C_ZERO));

        auto result = luabridge::get<C>(L, 1);
        ASSERT_TRUE(result);
        EXPECT_EQ(C::C_ZERO, *result);
    }

    {
        ASSERT_TRUE(luabridge::push(L, D::D_ZERO));

        auto result = luabridge::get<D>(L, 1);
        ASSERT_TRUE(result);
        EXPECT_EQ(D::D_ZERO, *result);
    }
}

TEST_F(EnumTests, MethodTakingEnum)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("test")
        .addFunction("takeEnum", +[](C value) { return static_cast<int>(value); })
        .addFunction("takeEnumClass", +[](D value) { return static_cast<int>(value); })
        .endNamespace();

    luabridge::setGlobal(L, C::C_ONE, "C_ONE");
    luabridge::setGlobal(L, D::D_ONE, "D_ONE");

    ASSERT_TRUE(runLua("result = test.takeEnum(C_ONE)"));
    EXPECT_EQ(1, result<int>());

    ASSERT_TRUE(runLua("result = test.takeEnum(1)"));
    EXPECT_EQ(1, result<int>());

    ASSERT_TRUE(runLua("result = test.takeEnum(D_ONE)"));
    EXPECT_EQ(D::D_ONE, result<D>());

    ASSERT_TRUE(runLua("result = test.takeEnumClass(D_ONE)"));
    EXPECT_EQ(1, result<int>());

    ASSERT_TRUE(runLua("result = test.takeEnumClass(1)"));
    EXPECT_EQ(1, result<int>());

    ASSERT_TRUE(runLua("result = test.takeEnumClass(C_ONE)"));
    EXPECT_EQ(C::C_ONE, result<C>());
}

TEST_F(EnumTests, MethodTakingEnumType)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("test")
        .addFunction("takeEnum", +[](E value) { return static_cast<int>(value); })
        .addFunction("takeEnumClass", +[](F value) { return static_cast<int>(value); })
        .endNamespace();

    luabridge::setGlobal(L, E::E_ONE, "E_ONE");
    luabridge::setGlobal(L, F::F_ONE, "F_ONE");

    ASSERT_TRUE(runLua("result = test.takeEnum(E_ONE)"));
    EXPECT_EQ(1, result<int>());

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_ANY_THROW(runLua("result = test.takeEnum(F_ONE)"));
    ASSERT_ANY_THROW(runLua("result = test.takeEnum(1)"));
#else
    ASSERT_FALSE(runLua("result = test.takeEnum(F_ONE)"));
    ASSERT_FALSE(runLua("result = test.takeEnum(1)"));
#endif

    ASSERT_TRUE(runLua("result = test.takeEnumClass(F_ONE)"));
    EXPECT_EQ(1, result<int>());

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_ANY_THROW(runLua("result = test.takeEnumClass(E_ONE)"));
    ASSERT_ANY_THROW(runLua("result = test.takeEnumClass(1)"));
#else
    ASSERT_FALSE(runLua("result = test.takeEnumClass(E_ONE)"));
    ASSERT_FALSE(runLua("result = test.takeEnumClass(1)"));
#endif
}
