// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, kunitoki
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#if LUABRIDGE_HAS_CXX23_MOVE_ONLY_FUNCTION

#include <functional>

namespace {
class TestClass
{
public:
    void method1(std::function<void()> fn) { fn(); }
    void method2(std::move_only_function<void()> fn) { fn(); }
};
} // namespace

struct MoveOnlyFunctionTests : TestBase
{
};

TEST_F(MoveOnlyFunctionTests, TraitsNonConst)
{
    using F = std::move_only_function<int(double, std::string)>;

    EXPECT_FALSE((luabridge::detail::function_traits<F>::is_member));
    EXPECT_FALSE((luabridge::detail::function_traits<F>::is_const));
    EXPECT_EQ(2u, (luabridge::detail::function_traits<F>::arity));

    using RetType = typename luabridge::detail::function_traits<F>::result_type;
    static_assert(std::is_same_v<RetType, int>);
}

TEST_F(MoveOnlyFunctionTests, TraitsConst)
{
    using F = std::move_only_function<void(int) const>;

    EXPECT_FALSE((luabridge::detail::function_traits<F>::is_member));
    EXPECT_TRUE((luabridge::detail::function_traits<F>::is_const));
    EXPECT_EQ(1u, (luabridge::detail::function_traits<F>::arity));
}

TEST_F(MoveOnlyFunctionTests, TraitsNoexcept)
{
    using F = std::move_only_function<double() noexcept>;

    EXPECT_EQ(0u, (luabridge::detail::function_traits<F>::arity));

    using RetType = typename luabridge::detail::function_traits<F>::result_type;
    static_assert(std::is_same_v<RetType, double>);
}

TEST_F(MoveOnlyFunctionTests, RegisterAndCall)
{
    std::move_only_function<int(int)> fn = [](int x) { return x * 2; };

    luabridge::getGlobalNamespace(L)
        .addFunction("double_it", std::move(fn));

    runLua("result = double_it(21)");
    EXPECT_EQ(42, result<int>());
}

TEST_F(MoveOnlyFunctionTests, HasFunctionTraits)
{
    using F = std::move_only_function<int(double)>;
    EXPECT_TRUE((luabridge::detail::has_function_traits_v<F>));
}

TEST_F(MoveOnlyFunctionTests, RegisterAsArgument)
{
    using Function1 = std::function<void()>;
    using Function2 = std::move_only_function<void()>;

    luabridge::getGlobalNamespace(L)
        .beginClass<TestClass>("TestClass")
            .addFunction("Method1", &TestClass::method1)
            .addFunction("Method2", &TestClass::method2)
        .endClass()
        .beginClass<Function1>("Function1")
        .endClass()
        .beginClass<Function2>("Function2")
        .endClass();

    TestClass object;
    luabridge::setGlobal(L, &object, "object");

    bool called1 = false;
    Function1 fn1 = [&called1] { called1 = true; };
    luabridge::setGlobal(L, &fn1, "fn1");
    runLua("object:Method1(fn1)");
    EXPECT_TRUE(called1);

    bool called2 = false;
    Function2 fn2 = [&called2] { called2 = true; };
    luabridge::setGlobal(L, &fn2, "fn2");
    runLua("object:Method2(fn2)");
    EXPECT_TRUE(called2);
}

#endif // LUABRIDGE_HAS_CXX23_MOVE_ONLY_FUNCTION
