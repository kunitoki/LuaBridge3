// https://github.com/kunitoki/LuaBridge3
// Copyright 2022, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include <gtest/gtest.h>

#include <cstring>
#include <iostream>
#include <memory>
#include <string>

struct OverloadTests : TestBase
{
};

TEST_F(OverloadTests, NoMatchingArity)
{
    int x = 100;

    luabridge::getGlobalNamespace(L)
        .addFunction("test",
            [x](int v) -> int {
                return v + x;
            },
            [x](std::string v) -> int {
                return v.size() ? int(v[0]) : x;
            });

#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_ANY_THROW(runLua("result = test ()"));
    EXPECT_ANY_THROW(runLua("result = test (255, 255)"));
    EXPECT_ANY_THROW(runLua("result = test ('', '')"));
    EXPECT_ANY_THROW(runLua("result = test ('0', '0')"));
#else
    EXPECT_FALSE(runLua("result = test ()"));
    EXPECT_FALSE(runLua("result = test (255, 255)"));
    EXPECT_FALSE(runLua("result = test ('', '')"));
    EXPECT_FALSE(runLua("result = test ('0', '0')"));
#endif
}

TEST_F(OverloadTests, NoMatchingArityWithState)
{
    int x = 100;

    luabridge::getGlobalNamespace(L)
        .addFunction("test",
            [x](int v, lua_State*) -> int {
                return v + x;
            },
            [x](std::string v, lua_State*) -> int {
                return v.size() ? int(v[0]) : x;
            });

#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_ANY_THROW(runLua("result = test ()"));
    EXPECT_ANY_THROW(runLua("result = test (255, 255)"));
    EXPECT_ANY_THROW(runLua("result = test ('', '')"));
    EXPECT_ANY_THROW(runLua("result = test ('0', '0')"));
#else
    EXPECT_FALSE(runLua("result = test ()"));
    EXPECT_FALSE(runLua("result = test (255, 255)"));
    EXPECT_FALSE(runLua("result = test ('', '')"));
    EXPECT_FALSE(runLua("result = test ('0', '0')"));
#endif
}

TEST_F(OverloadTests, SingleArgumentOverloads)
{
    int x = 100;

    luabridge::getGlobalNamespace(L)
        .addFunction("test",
            [x](int v) -> int {
                return v + x;
            },
            [x](std::string v) -> int {
                return v.size() ? int(v[0]) : x;
            });

    runLua("result = test (255)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(355, result<int>());

    runLua("result = test ('')");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(100, result<int>());

    runLua("result = test ('0')");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(48, result<int>());
}

TEST_F(OverloadTests, SingleArgumentOverloadsWithState)
{
    int x = 100;

    luabridge::getGlobalNamespace(L)
        .addFunction("test",
            [x](int v, lua_State*) -> int {
                return v + x;
            },
            [x](std::string v, lua_State*) -> int {
                return v.size() ? int(v[0]) : x;
            });

    runLua("result = test (255)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(355, result<int>());

    runLua("result = test ('')");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(100, result<int>());

    runLua("result = test ('0')");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(48, result<int>());
}

TEST_F(OverloadTests, IntegerTypeFallbackOverloads)
{
    luabridge::getGlobalNamespace(L)
        .addFunction("test",
            [](int8_t v) -> int {
                return 1;
            },
            [](int16_t v) -> int {
                return 2;
            },
            [](int32_t v) -> int {
                return 3;
            },
            [](int64_t v) -> int {
                return 4;
            });

    runLua("result = test (1)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(1, result<int>());

    runLua("result = test (127)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(1, result<int>());

    runLua("result = test (128)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(2, result<int>());

    runLua("result = test (32767)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(2, result<int>());

    runLua("result = test (32768)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(3, result<int>());

    runLua("result = test (2147483647)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(3, result<int>());

    if constexpr (sizeof(lua_Integer) >= sizeof(int64_t))
    {
        runLua("result = test (2147483648)");
        ASSERT_TRUE(result().isNumber());
        EXPECT_EQ(4, result<int>());
    }
}

TEST_F(OverloadTests, NoMatchingArityClass)
{
    int x = 100;

    struct X {};

    luabridge::getGlobalNamespace(L)
        .beginClass<X>("X")
            .addFunction("test",
                [x](X*, int v) -> int {
                    return v + x;
                },
                [x](X*, std::string v) -> int {
                    return v.size() ? int(v[0]) : x;
                })
        .endClass();

    X y;
    luabridge::setGlobal(L, &y, "y");

#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_ANY_THROW(runLua("result = y:test ()"));
    EXPECT_ANY_THROW(runLua("result = y:test (255, 255)"));
    EXPECT_ANY_THROW(runLua("result = y:test ('', '')"));
    EXPECT_ANY_THROW(runLua("result = y:test ('0', '0')"));
#else
    EXPECT_FALSE(runLua("result = y:test ()"));
    EXPECT_FALSE(runLua("result = y:test (255, 255)"));
    EXPECT_FALSE(runLua("result = y:test ('', '')"));
    EXPECT_FALSE(runLua("result = y:test ('0', '0')"));
#endif
}

TEST_F(OverloadTests, NoMatchingArityWithStateClass)
{
    int x = 100;

    struct X {};

    luabridge::getGlobalNamespace(L)
        .beginClass<X>("X")
            .addFunction("test",
                [x](X*, int v, lua_State*) -> int {
                    return v + x;
                },
                [x](X*, std::string v, lua_State*) -> int {
                    return v.size() ? int(v[0]) : x;
                })
        .endClass();

    X y;
    luabridge::setGlobal(L, &y, "y");

#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_ANY_THROW(runLua("result = y:test ()"));
    EXPECT_ANY_THROW(runLua("result = y:test (255, 255)"));
    EXPECT_ANY_THROW(runLua("result = y:test ('', '')"));
    EXPECT_ANY_THROW(runLua("result = y:test ('0', '0')"));
#else
    EXPECT_FALSE(runLua("result = y:test ()"));
    EXPECT_FALSE(runLua("result = y:test (255, 255)"));
    EXPECT_FALSE(runLua("result = y:test ('', '')"));
    EXPECT_FALSE(runLua("result = y:test ('0', '0')"));
#endif
}

TEST_F(OverloadTests, MixedFunctionTypesClass)
{
    int x = 100;

    struct X
    {
        int test()
        {
            return 42;
        }
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<X>("X")
            .addFunction("test",
                [x](X*, int v, lua_State*) -> int {
                    return v + x;
                },
                +[](X*, std::string v, lua_State*) -> int {
                    return v.size() ? int(v[0]) : 1337;
                },
                &X::test)
        .endClass();

    X y;
    luabridge::setGlobal(L, &y, "y");

    runLua("result = y:test ()");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(42, result<int>());

    runLua("result = y:test (255)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(355, result<int>());

    runLua("result = y:test ('')");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(1337, result<int>());

    runLua("result = y:test ('0')");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(48, result<int>());
}

TEST_F(OverloadTests, LuaCFunctionFallback)
{
    struct X
    {
        int test(std::string)
        {
            return 3;
        }

        int testLua(lua_State* L)
        {
            lua_pushnumber(L, 4);
            return 1;
        }
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<X>("X")
            .addFunction("test",
                [](X*, char v, lua_State*) -> int {
                    return 1;
                },
                +[](X*, int v) -> int {
                    return 2;
                },
                &X::test,
                &X::testLua)
        .endClass();

    X y;
    luabridge::setGlobal(L, &y, "y");

    runLua("result = y:test ('2')");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(1, result<int>());

    runLua("result = y:test (1)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(2, result<int>());

    runLua("result = y:test ('123456')");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(3, result<int>());

    runLua("result = y:test ()");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(4, result<int>());
}
