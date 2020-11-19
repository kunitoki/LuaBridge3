// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include "LuaBridge/Optional.h"

#include <optional>

struct OptionalTests : TestBase
{
};

TEST_F(OptionalTests, PassToFunction)
{
    runLua("function foo (opt) "
           "  result = opt "
           "end");

    auto foo = luabridge::getGlobal(L, "foo");

    {
        resetResult();

        std::optional<int> lvalue{ 10 };
        foo(lvalue);
        EXPECT_FALSE(result().isNil());
        EXPECT_EQ(lvalue, result<std::optional<int>>());

        resetResult();

        const std::optional<int> constLvalue = lvalue;
        foo(constLvalue);
        EXPECT_FALSE(result().isNil());
        EXPECT_EQ(lvalue, result<std::optional<int>>());
    }

    {
        resetResult();

        std::optional<std::string> lvalue;
        foo(lvalue);
        EXPECT_TRUE(result().isNil());
        EXPECT_FALSE(result<std::optional<std::string>>());

        resetResult();

        const std::optional<std::string> constLvalue = lvalue;
        foo(constLvalue);
        EXPECT_TRUE(result().isNil());
        EXPECT_FALSE(result<std::optional<std::string>>());
    }
}

TEST_F(OptionalTests, FromCppApi)
{
    struct OptionalClass
    {
        OptionalClass() = default;
        
        void testOptionalAsArg(std::optional<int> v)
        {
            x = v;
        }

        std::optional<std::string> testOptionalAsReturn()
        {
            return "abcdef";
        }

        std::optional<int> x;
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<OptionalClass>("OptionalClass")
        .addConstructor<void (*)()>()
        .addFunction("testOptionalAsArg", &OptionalClass::testOptionalAsArg)
        .addFunction("testOptionalAsReturn", &OptionalClass::testOptionalAsReturn)
        .endClass();

    resetResult();
    runLua("result = OptionalClass () result:testOptionalAsArg (1337)");
    
    EXPECT_TRUE(result().isUserdata());
    EXPECT_EQ(1337, *result<OptionalClass>().x);

    resetResult();
    runLua("x = OptionalClass () result = x:testOptionalAsReturn ()");
    
    EXPECT_FALSE(result().isNil());
    EXPECT_EQ("abcdef", *result<std::optional<std::string>>());
}
