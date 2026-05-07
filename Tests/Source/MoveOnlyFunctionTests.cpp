// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, kunitoki
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#if LUABRIDGE_HAS_CXX23_MOVE_ONLY_FUNCTION

#include <functional>

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
        .addFunction("double_it", std::function<int(int)>([](int x) { return x * 2; }));

    runLua("result = double_it(21)");
    EXPECT_EQ(42, result<int>());
}

TEST_F(MoveOnlyFunctionTests, HasFunctionTraits)
{
    using F = std::move_only_function<int(double)>;
    EXPECT_TRUE((luabridge::detail::has_function_traits_v<F>));
}

#endif // LUABRIDGE_HAS_CXX23_MOVE_ONLY_FUNCTION
