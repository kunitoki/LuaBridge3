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

TEST_F(OverloadTests, UnregisteredClass)
{
    struct Unregistered {};

    luabridge::getGlobalNamespace(L)
        .addFunction("testUnregistered",
            [](Unregistered*) { return 1; },
            [](int) { return 2; });

#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_ANY_THROW(runLua("result = testUnregistered ({})"));
#else
    EXPECT_FALSE(runLua("result = testUnregistered ({})"));
#endif

    runLua("result = testUnregistered (1)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(2, result<int>());
}

TEST_F(OverloadTests, BaseAndDerivedClassOverloads)
{
    struct Base
    {
        virtual ~Base() {}

        virtual int test1() { return 1; }
        virtual int test2(int) { return 2; }

        int test3() { return 3; }
        int test4(int) { return 4; }
    };

    struct Derived : Base
    {
        int test1() override { return 5; }
        int test2(int) override { return 6; }

        int test5() { return 7; }
        int test6(int) { return 8; }
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<Base>("Base")
            .addConstructor<void (*)()>()
            .addFunction("testx", &Base::test1, &Base::test2)
            .addFunction("testy", &Base::test3, &Base::test4)
        .endClass()
        .deriveClass<Derived, Base>("Derived")
            .addConstructor<void (*)()>()
            .addFunction("testx", &Derived::test1, &Derived::test2)
            .addFunction("testz", &Derived::test5, &Derived::test6)
        .endClass()
        .addFunction("testBase",
            [](Base* b) { return b->test3(); },
            [](Derived* d) { return d->test5(); })
        .addFunction("testDerived",
            [](Derived* d) { return d->test5(); },
            [](Base* b) { return b->test3(); });

    runLua("b = Base(); result = b:testx ()");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(1, result<int>());

    runLua("b = Base(); result = b:testx (1)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(2, result<int>());

    runLua("b = Derived(); result = b:testx ()");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(5, result<int>());

    runLua("b = Derived(); result = b:testx (1)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(6, result<int>());

    runLua("b = Derived(); result = b:testy ()");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(3, result<int>());

    runLua("b = Derived(); result = b:testy (1)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(4, result<int>());

    runLua("b = Base(); result = testBase (b)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(3, result<int>());

    runLua("b = Derived(); result = testBase (b)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(3, result<int>());

    runLua("b = Base(); result = testDerived (b)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(3, result<int>());

    runLua("b = Derived(); result = testDerived (b)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(7, result<int>());
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

TEST_F(OverloadTests, NoMatchingArityStaticFunctionClass)
{
    int x = 100;

    struct X {};

    luabridge::getGlobalNamespace(L)
        .beginClass<X>("X")
            .addStaticFunction("test",
                [x](int v) -> int {
                    return v + x;
                },
                [x](std::string v) -> int {
                    return v.size() ? int(v[0]) : x;
                })
        .endClass();

    X y;
    luabridge::setGlobal(L, &y, "y");

#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_ANY_THROW(runLua("result = X.test ()"));
    EXPECT_ANY_THROW(runLua("result = X.test (255, 255)"));
    EXPECT_ANY_THROW(runLua("result = X.test ('', '')"));
    EXPECT_ANY_THROW(runLua("result = X.test ('0', '0')"));
#else
    EXPECT_FALSE(runLua("result = X.test ()"));
    EXPECT_FALSE(runLua("result = X.test (255, 255)"));
    EXPECT_FALSE(runLua("result = X.test ('', '')"));
    EXPECT_FALSE(runLua("result = X.test ('0', '0')"));
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

TEST_F(OverloadTests, NoMatchingArityWithStateStaticFunctionClass)
{
    int x = 100;

    struct X {};

    luabridge::getGlobalNamespace(L)
        .beginClass<X>("X")
            .addStaticFunction("test",
                [x](int v, lua_State*) -> int {
                    return v + x;
                },
                [x](std::string v, lua_State*) -> int {
                    return v.size() ? int(v[0]) : x;
                })
        .endClass();

    X y;
    luabridge::setGlobal(L, &y, "y");

#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_ANY_THROW(runLua("result = X.test ()"));
    EXPECT_ANY_THROW(runLua("result = X.test (255, 255)"));
    EXPECT_ANY_THROW(runLua("result = X.test ('', '')"));
    EXPECT_ANY_THROW(runLua("result = X.test ('0', '0')"));
#else
    EXPECT_FALSE(runLua("result = X.test ()"));
    EXPECT_FALSE(runLua("result = X.test (255, 255)"));
    EXPECT_FALSE(runLua("result = X.test ('', '')"));
    EXPECT_FALSE(runLua("result = X.test ('0', '0')"));
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

TEST_F(OverloadTests, MixedStaticFunctionTypesClass)
{
    int x = 100;

    struct X
    {
        static int test()
        {
            return 42;
        }
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<X>("X")
            .addStaticFunction("test",
                [x](int v, lua_State*) -> int {
                    return v + x;
                },
                +[](std::string v, lua_State*) -> int {
                    return v.size() ? int(v[0]) : 1337;
                },
                &X::test)
        .endClass();

    X y;
    luabridge::setGlobal(L, &y, "y");

    runLua("result = X.test ()");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(42, result<int>());

    runLua("result = X.test (255)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(355, result<int>());

    runLua("result = X.test ('')");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(1337, result<int>());

    runLua("result = X.test ('0')");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(48, result<int>());
}

TEST_F(OverloadTests, OverloadOperatorClass)
{
    struct X
    {
        X(int x) : value(x) {}

        X operator*(const X& rhs) const
        {
            return { value * rhs.value };
        }

        X operator*(int x) const
        {
            return { value * x };
        }

        int value = 0;
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<X>("X")
            .addConstructor<void(int)>()
            .addProperty("value", &X::value)
            .addFunction("__mul",
                luabridge::overload<const X&>(&X::operator*),
                luabridge::overload<int>(&X::operator*))
        .endClass();

    runLua("x1 = X(2); x2 = X(3); result = x1 * x2");
    ASSERT_TRUE(result().isUserdata());
    auto result1 = result<X>();
    EXPECT_EQ(6, result1.value);

    runLua("x1 = X(2); result = x1 * 3");
    ASSERT_TRUE(result().isUserdata());
    auto result2 = result<X>();
    EXPECT_EQ(6, result2.value);
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

TEST_F(OverloadTests, LuaCStaticFunctionFallback)
{
    struct X
    {
        static int test(std::string)
        {
            return 3;
        }

        static int testLua(lua_State* L)
        {
            lua_pushnumber(L, 4);
            return 1;
        }
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<X>("X")
            .addStaticFunction("test",
                [](char v, lua_State*) -> int {
                    return 1;
                },
                +[](int v) -> int {
                    return 2;
                },
                &X::test,
                &X::testLua)
        .endClass();

    X y;
    luabridge::setGlobal(L, &y, "y");

    runLua("result = X.test ('2')");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(1, result<int>());

    runLua("result = X.test (1)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(2, result<int>());

    runLua("result = X.test ('123456')");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(3, result<int>());

    runLua("result = X.test ()");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(4, result<int>());
}

TEST_F(OverloadTests, ConstAndNonConstOverloadClass)
{
    struct X
    {
        int test1(std::string)              { return 1; }
        int test1(std::string) const        { return 2; }
        int test2(std::string)              { return 3; }
        int test2(std::string, int)         { return 4; }
        int test3(std::string) const        { return 5; }
        int test3(std::string, int) const   { return 6; }
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<X>("X")
            .addFunction("test1",
                luabridge::nonConstOverload<std::string>(&X::test1),
                luabridge::constOverload<std::string>(&X::test1))
            .addFunction("test2",
                luabridge::overload<std::string>(&X::test2),
                luabridge::overload<std::string, int>(&X::test2))
            .addFunction("test3",
                luabridge::constOverload<std::string>(&X::test3),
                luabridge::constOverload<std::string, int>(&X::test3))
        .endClass();

    // Non const object
    X x;
    luabridge::setGlobal(L, &x, "x");

    runLua("result = x:test1 ('abc')");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(1, result<int>());

    runLua("result = x.test2");
    EXPECT_FALSE(result().isNil());
    runLua("result = x:test2 ('abc')");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(3, result<int>());

    runLua("result = x.test3");
    EXPECT_FALSE(result().isNil());
    runLua("result = x:test3 ('abc')");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(5, result<int>());
    runLua("result = x:test3 ('abc', 1)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(6, result<int>());

    // Const object
    const X y;
    luabridge::setGlobal(L, &y, "y");

    runLua("result = y:test1 ('abc')");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(2, result<int>());

    runLua("result = y.test2");
    EXPECT_TRUE(result().isNil());
#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_ANY_THROW(runLua("result = y:test2 ('abc')"));
    ASSERT_ANY_THROW(runLua("result = y:test2 ('abc', 1)"));
#else
    ASSERT_FALSE(runLua("result = y:test2 ('abc')"));
    ASSERT_FALSE(runLua("result = y:test2 ('abc', 1)"));
#endif

    runLua("result = y.test3");
    EXPECT_FALSE(result().isNil());
    runLua("result = y:test3 ('abc')");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(5, result<int>());
    runLua("result = y:test3 ('abc', 1)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(6, result<int>());
}

namespace {
struct Vec
{
public:
	Vec operator+(const Vec& v) const
	{
		Vec a;
		return a;
	}

	Vec operator+(const Vec& v)
	{
		Vec a;
		return a;
	}

	template <class T, typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
	Vec operator+(T v) const
	{
		Vec a;
		return a;
	}

	template <class T, typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
	Vec operator+(T v)
	{
		Vec a;
		return a;
	}
};
} // namespace

TEST_F(OverloadTests, ConstAndNonConstOverloadClassTemplate)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("test")
            .beginClass<Vec>("Vec")
                .addFunction("__add",
                    luabridge::constOverload<double>(&Vec::operator+<double>),
                    luabridge::ConstOverload<const Vec&>::with<Vec, Vec>(&Vec::operator+),
                    luabridge::nonConstOverload<double>(&Vec::operator+<double>),
                    luabridge::NonConstOverload<const Vec&>::with<Vec, Vec>(&Vec::operator+),
                    luabridge::Overload<double>::with<Vec, Vec>(&Vec::operator+<double>),
                    luabridge::Overload<const Vec&>::with<Vec, Vec>(&Vec::operator+))
            .endClass()
        .endNamespace();

    SUCCEED();
}

TEST_F(OverloadTests, LuaCFunctionArityCheck)
{
    struct X
    {
        int test1(int)
        {
            return 1;
        }

        int test2(lua_State* L)
        {
            lua_pushinteger(L, 2);
            return 1;
        }

        static int test3(int)
        {
            return 3;
        }

        static int test4(lua_State* L)
        {
            lua_pushinteger(L, 4);
            return 1;
        }
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<X>("X")
            .addFunction("test_a", &X::test2, &X::test1)
            .addFunction("test_b", &X::test1, &X::test2)
            .addStaticFunction("test_c", &X::test4, &X::test3)
            .addStaticFunction("test_d", &X::test3, &X::test4)
        .endClass();

    X x;
    luabridge::setGlobal(L, &x, "x");

    // Non-static
    runLua("result = x:test_a ('abc')");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(2, result<int>());

    runLua("result = x:test_a (1)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(2, result<int>());

    runLua("result = x:test_a (1, 2, 3)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(2, result<int>());

    runLua("result = x:test_b ('abc')");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(2, result<int>());

    runLua("result = x:test_b (1)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(1, result<int>());

    runLua("result = x:test_b (1, 2, 3)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(2, result<int>());

    // Static
    runLua("result = X.test_c ('abc')");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(4, result<int>());

    runLua("result = X.test_c (1)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(4, result<int>());

    runLua("result = X.test_c (1, 2, 3)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(4, result<int>());

    runLua("result = X.test_d ('abc')");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(4, result<int>());

    runLua("result = X.test_d (1)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(3, result<int>());

    runLua("result = X.test_d (1, 2, 3)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(4, result<int>());
}

TEST_F(OverloadTests, ConstructorOverloading)
{
    struct X
    {
        X(int x)
            : value(x)
        {
        }

        X(int x, int y)
            : value(x + y)
        {
        }

        int value = 0;
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<X>("X")
            .addConstructor(
                [](void* ptr, int x) { return new (ptr) X(x); },
                [](void* ptr, int x, int y) { return new (ptr) X(x, y); })
            .addProperty("value", &X::value)
        .endClass()
        .beginClass<X>("Y")
            .addConstructor<void(int), void(int, int)>()
            .addProperty("value", &X::value)
        .endClass();

    runLua("x = X(1); result = x.value");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(1, result<int>());

    runLua("x = X(1, 10); result = x.value");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(11, result<int>());

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_ANY_THROW(runLua("x = X(1, 10, 100); result = x.value"));
#else
    ASSERT_FALSE(runLua("x = X(1, 10, 100); result = x.value"));
#endif

    runLua("y = Y(1); result = y.value");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(1, result<int>());

    runLua("y = Y(1, 10); result = y.value");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(11, result<int>());

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_ANY_THROW(runLua("y = Y(1, 10, 100); result = y.value"));
#else
    ASSERT_FALSE(runLua("y = Y(1, 10, 100); result = y.value"));
#endif
}

TEST_F(OverloadTests, FunctionFailureVsArgumentConversion)
{
    luabridge::getGlobalNamespace(L)
        .addFunction("test",
            +[](int, lua_State* L) -> int {
                lua_pushinteger(L, 1);
                luaL_error(L, "failure");
                return 1;
            },
            +[](lua_State* L) -> int {
                lua_pushinteger(L, 2);
                return 1;
            });

    runLua("result = test ('abcdefg')");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(2, result<int>());

    runLua("result = test (1)");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(2, result<int>());
}
