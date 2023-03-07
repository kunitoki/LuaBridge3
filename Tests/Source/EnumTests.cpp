// https://github.com/kunitoki/LuaBridge3
// Copyright 2023, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include "LuaBridge/Map.h"

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
struct luabridge::Stack<C> : luabridge::Enum<C, C::C_ZERO, C::C_ONE, C::C_TWO, C::C_THREE>
{
};

template <>
struct luabridge::Stack<D> : luabridge::Enum<D, D::D_ZERO, D::D_ONE, D::D_TWO, D::D_THREE>
{
};

template <>
struct luabridge::Stack<E> : luabridge::Enum<E>
{
};

template <>
struct luabridge::Stack<F> : luabridge::Enum<F>
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

TEST_F(EnumTests, RegisteredStackInvalidValue)
{
    ASSERT_TRUE(luabridge::push(L, 4));

    EXPECT_FALSE(luabridge::get<C>(L, 1));
    EXPECT_FALSE(luabridge::get<D>(L, 1));

    EXPECT_TRUE(luabridge::get<E>(L, 1));
    EXPECT_TRUE(luabridge::get<F>(L, 1));
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

TEST_F(EnumTests, MethodTakingEnumAsMapKey)
{
    std::map<C, std::string> map_of_c { { C::C_ZERO, "0" }, { C::C_ONE, "1" } };
    std::map<D, std::string> map_of_d { { D::D_ZERO, "0" }, { D::D_ONE, "1" } };

    luabridge::getGlobalNamespace(L)
        .beginNamespace("C")
            .addVariable("C_ZERO", C::C_ZERO)
            .addVariable("C_ONE", C::C_ONE)
            .addVariable("C_TWO", C::C_TWO)
            .addVariable("C_THREE", C::C_THREE)
        .endNamespace()
        .beginNamespace("C1")
            .addProperty("C_ZERO", +[] { return C::C_ZERO; })
            .addProperty("C_ONE", +[] { return C::C_ONE; })
            .addProperty("C_TWO", +[] { return C::C_TWO; })
            .addProperty("C_THREE", +[] { return C::C_THREE; })
        .endNamespace()
        .beginNamespace("D")
            .addVariable("D_ZERO", D::D_ZERO)
            .addVariable("D_ONE", D::D_ONE)
            .addVariable("D_TWO", D::D_TWO)
            .addVariable("D_THREE", D::D_THREE)
        .endNamespace()
        .beginNamespace("D1")
            .addProperty("D_ZERO", +[] { return D::D_ZERO; })
            .addProperty("D_ONE", +[] { return D::D_ONE; })
            .addProperty("D_TWO", +[] { return D::D_TWO; })
            .addProperty("D_THREE", +[] { return D::D_THREE; })
        .endNamespace();

    luabridge::setGlobal(L, map_of_c, "map_of_c");
    luabridge::setGlobal(L, map_of_d, "map_of_d");

    ASSERT_TRUE(runLua("result = map_of_c[0]"));
    EXPECT_EQ("0", result<std::string>());

    ASSERT_TRUE(runLua("result = map_of_c[C.C_ZERO]"));
    EXPECT_EQ("0", result<std::string>());

    ASSERT_TRUE(runLua("result = map_of_c[C1.C_ZERO]"));
    EXPECT_EQ("0", result<std::string>());

    ASSERT_TRUE(runLua("result = map_of_c[1]"));
    EXPECT_EQ("1", result<std::string>());

    ASSERT_TRUE(runLua("result = map_of_c[C.C_ONE]"));
    EXPECT_EQ("1", result<std::string>());

    ASSERT_TRUE(runLua("result = map_of_c[C1.C_ONE]"));
    EXPECT_EQ("1", result<std::string>());

    ASSERT_TRUE(runLua("result = map_of_d[0]"));
    EXPECT_EQ("0", result<std::string>());

    ASSERT_TRUE(runLua("result = map_of_d[D.D_ZERO]"));
    EXPECT_EQ("0", result<std::string>());

    ASSERT_TRUE(runLua("result = map_of_d[D1.D_ZERO]"));
    EXPECT_EQ("0", result<std::string>());

    ASSERT_TRUE(runLua("result = map_of_d[1]"));
    EXPECT_EQ("1", result<std::string>());

    ASSERT_TRUE(runLua("result = map_of_d[D.D_ONE]"));
    EXPECT_EQ("1", result<std::string>());

    ASSERT_TRUE(runLua("result = map_of_d[D1.D_ONE]"));
    EXPECT_EQ("1", result<std::string>());
}
