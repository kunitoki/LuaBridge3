// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// Copyright 2020, Dmitry Tarakanov
// Copyright 2019, George Tokmaji
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "Config.h"
#include "Stack.h"
#include "TypeTraits.h"

#include <functional>
#include <tuple>

namespace luabridge {
namespace detail {

//=================================================================================================
/**
 * @brief Generic function traits.
 */
template <bool IsMember, bool IsConst, class R, class... Args>
struct function_traits_base
{
    using result_type = R;

    using argument_types = std::tuple<Args...>;

    static constexpr auto arity = sizeof...(Args);

    static constexpr auto is_member = IsMember;

    static constexpr auto is_const = IsConst;
};

template <class...>
struct function_traits_impl;

template <class R, class... Args>
struct function_traits_impl<R(Args...)> : function_traits_base<false, false, R, Args...>
{
};

template <class R, class... Args>
struct function_traits_impl<R (*)(Args...)> : function_traits_base<false, false, R, Args...>
{
};

template <class C, class R, class... Args>
struct function_traits_impl<R (C::*)(Args...)> : function_traits_base<true, false, R, Args...>
{
};

template <class C, class R, class... Args>
struct function_traits_impl<R (C::*)(Args...) const> : function_traits_base<true, true, R, Args...>
{
};

template <class R, class... Args>
struct function_traits_impl<R(Args...) noexcept> : function_traits_base<false, false, R, Args...>
{
};

template <class R, class... Args>
struct function_traits_impl<R (*)(Args...) noexcept> : function_traits_base<false, false, R, Args...>
{
};

template <class C, class R, class... Args>
struct function_traits_impl<R (C::*)(Args...) noexcept> : function_traits_base<true, false, R, Args...>
{
};

template <class C, class R, class... Args>
struct function_traits_impl<R (C::*)(Args...) const noexcept> : function_traits_base<true, true, R, Args...>
{
};

#if _MSC_VER && _M_IX86 // Windows: WINAPI (a.k.a. __stdcall) function pointers (32bit only).
template <class R, class... Args>
struct function_traits_impl<R __stdcall(Args...)> : function_traits_base<false, false, R, Args...>
{
};

template <class R, class... Args>
struct function_traits_impl<R (__stdcall *)(Args...)> : function_traits_base<false, false, R, Args...>
{
};

template <class C, class R, class... Args>
struct function_traits_impl<R (__stdcall C::*)(Args...)> : function_traits_base<true, false, R, Args...>
{
};

template <class C, class R, class... Args>
struct function_traits_impl<R (__stdcall C::*)(Args...) const> : function_traits_base<true, true, R, Args...>
{
};

template <class R, class... Args>
struct function_traits_impl<R __stdcall(Args...) noexcept> : function_traits_base<false, false, R, Args...>
{
};

template <class R, class... Args>
struct function_traits_impl<R (__stdcall *)(Args...) noexcept> : function_traits_base<false, false, R, Args...>
{
};

template <class C, class R, class... Args>
struct function_traits_impl<R (__stdcall C::*)(Args...) noexcept> : function_traits_base<true, false, R, Args...>
{
};

template <class C, class R, class... Args>
struct function_traits_impl<R (__stdcall C::*)(Args...) const noexcept> : function_traits_base<true, true, R, Args...>
{
};
#endif

template <class F>
struct functor_traits_impl : function_traits_impl<decltype(&F::operator())>
{
};

//=================================================================================================
/**
 * @brief Traits class for callable objects (e.g. function pointers, lambdas)
 *
 * @tparam F Callable object.
 */
template <class F>
struct function_traits : std::conditional<std::is_class<F>::value,
                                          detail::functor_traits_impl<F>,
                                          detail::function_traits_impl<F>>::type
{
};

//=================================================================================================
/**
 * @brief Deduces the return type of a callble object.
 *
 * @tparam F Callable object.
 */
template <class F>
using function_result_t = typename function_traits<F>::result_type;

/**
 * @brief Deduces the argument type of a callble object.
 *
 * @tparam I Argument index.
 * @tparam F Callable object.
 */
template <std::size_t I, class F>
using function_argument_t = std::tuple_element_t<I, typename function_traits<F>::argument_types>;

/**
 * @brief Deduces the arguments type of a callble object.
 *
 * @tparam F Callable object.
 */
template <class F>
using function_arguments_t = typename function_traits<F>::argument_types;

/**
 * @brief An integral constant expression that gives the number of arguments accepted by the callable object.
 *
 * @tparam F Callable object.
 */
template <class F>
static constexpr std::size_t function_arity_v = function_traits<F>::arity;

/**
 * @brief An boolean constant expression that checks if the callable object is a member function.
 *
 * @tparam F Callable object.
 */
template <class F>
static constexpr bool function_is_member_v = function_traits<F>::is_member;

/**
 * @brief An boolean constant expression that checks if the callable object is const.
 *
 * @tparam F Callable object.
 */
template <class F>
static constexpr bool function_is_const_v = function_traits<F>::is_const;

//=================================================================================================
/**
 * @brief Detect if we are a `std::function`.
 */
template <class F, class...>
struct is_std_function : std::false_type
{
};

template <class ReturnType, class... Args>
struct is_std_function<std::function<ReturnType(Args...)>> : std::true_type
{
};

template <class Signature>
struct is_std_function<std::function<Signature>> : std::true_type
{
};

template <class F>
static constexpr bool is_std_function_v = is_std_function<F>::value;

//=================================================================================================
/**
 * @brief Reconstruct a function signature from return type and args.
 */
template <class, class>
struct to_std_function_type
{
};

template <class ReturnType, typename... Args>
struct to_std_function_type<ReturnType, std::tuple<Args...>>
{
    using type = std::function<ReturnType(Args...)>;
};

template <class ReturnType, typename... Args>
using to_std_function_type_t = typename to_std_function_type<ReturnType, Args...>::type;

//=================================================================================================
/**
 * @brief Simple make_tuple alternative that doesn't decay the types.
 */
template <class... Types>
auto tupleize(Types&&... args)
{
    return std::tuple<Types...>(std::forward<Types>(args)...);
}

//=================================================================================================
/**
 * @brief Make argument lists extracting them from the lua state, starting at a stack index.
 */
template <class Args, std::size_t Start, std::size_t... Indices>
auto make_args_list_impl(lua_State* L, std::index_sequence<Indices...>)
{
    return tupleize(Stack<std::tuple_element_t<Indices, Args>>::get(L, Start + Indices)...);
}

template <class Args, std::size_t Start>
auto make_args_list(lua_State* L)
{
    return make_args_list_impl<Args, Start>(L, std::make_index_sequence<std::tuple_size_v<Args>>());
}

//=================================================================================================
/**
 * @brief Dispatcher object that unpacks the arguments into stack values then call the functor.
 */
template <std::size_t Start, class ReturnType, class Args>
struct dispatcher
{
    template <class F>
    static int call(lua_State* L, F func)
    {
        try
        {
            Stack<ReturnType>::push(L, std::apply(func, make_args_list<Args, Start>(L)));
            
            return 1;
        }
        catch (const std::exception& e)
        {
            return luaL_error(L, e.what());
        }
    }

    template <class T, class F>
    static int call(lua_State* L, T* ptr, F func)
    {
        try
        {
            auto f = [ptr, func](auto&&... args) -> ReturnType { return (ptr->*func)(std::forward<decltype(args)>(args)...); };

            Stack<ReturnType>::push(L, std::apply(f, make_args_list<Args, Start>(L)));

            return 1;
        }
        catch (const std::exception& e)
        {
            return luaL_error(L, e.what());
        }
    }
};

template <std::size_t Start, class Args>
struct dispatcher<Start, void, Args>
{
    template <class F>
    static int call(lua_State* L, F func)
    {
        try
        {
            std::apply(func, make_args_list<Args, Start>(L));

            return 0;
        }
        catch (const std::exception& e)
        {
            return luaL_error(L, e.what());
        }
    }

    template <class T, class F>
    static int call(lua_State* L, T* ptr, F func)
    {
        try
        {
            auto f = [ptr, func](auto&&... args) { (ptr->*func)(std::forward<decltype(args)>(args)...); };

            std::apply(f, make_args_list<Args, Start>(L));

            return 0;
        }
        catch (const std::exception& e)
        {
            return luaL_error(L, e.what());
        }
    }
};

} // namespace detail
} // namespace luabridge
