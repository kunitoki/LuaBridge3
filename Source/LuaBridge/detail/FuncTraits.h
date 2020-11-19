// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2020, Dmitry Tarakanov
// Copyright 2019, George Tokmaji
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "Config.h"
#include "TypeList.h"
#include "TypeTraits.h"

#include <functional>
#include <tuple>

namespace luabridge {
namespace detail {

//=================================================================================================
/**
 * @brief Traits class for unrolling the type list values into function arguments.
 */
template <class ReturnType, size_t NUM_PARAMS>
struct Caller
{
    template <class Fn, class Params>
    static ReturnType f(Fn& fn, TypeListValues<Params>& tvl)
    {
        return std::apply(fn, typeListValuesTuple(tvl));
    }

    template <class T, class MemFn, class Params>
    static ReturnType f(T* obj, MemFn& fn, TypeListValues<Params>& tvl)
    {
        auto func = [obj, fn](auto&&... args) { return (obj->*fn)(std::forward<decltype(args)>(args)...); };

        return std::apply(func, typeListValuesTuple(tvl));
    }
};

template <class ReturnType>
struct Caller<ReturnType, 0>
{
    template <class Fn, class Params>
    static ReturnType f(Fn& fn, TypeListValues<Params>&)
    {
        return fn();
    }

    template <class T, class MemFn, class Params>
    static ReturnType f(T* obj, MemFn& fn, TypeListValues<Params>&)
    {
        return (obj->*fn)();
    }
};

template <class ReturnType, class Fn, class Params>
ReturnType doCall(Fn& fn, TypeListValues<Params>& tvl)
{
    return Caller<ReturnType, TypeListSize<Params>::value>::f(fn, tvl);
}

template <class ReturnType, class T, class MemFn, class Params>
ReturnType doCall(T* obj, MemFn& fn, TypeListValues<Params>& tvl)
{
    return Caller<ReturnType, TypeListSize<Params>::value>::f(obj, fn, tvl);
}

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

template <class F>
struct functor_traits_impl : function_traits_impl<decltype(&F::operator())>
{
};

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

/**
 * @brief Reconstruct a function signature from return type and args.
 */
template <class, class>
struct to_function_type
{
};

template <class ReturnType, typename... Ts>
struct to_function_type<ReturnType, std::tuple<Ts...>>
{
    using type = std::function<ReturnType(Ts...)>;
};

template <class ReturnType, typename... Ts>
using to_function_type_t = typename to_function_type<ReturnType, Ts...>::type;

//=================================================================================================
/**
 * @brief Traits for function pointers.
 *
 * There are three types of functions: global, non-const member, and const member. These templates determine the type of function, which
 * class type it belongs to if it is a class member, the const-ness if it is a member function, and the type information for the return value and
 * argument list.
 */
template <class MemFn, class D = MemFn>
struct FuncTraits
{
};

// Ordinary function pointers.
template <class R, class... ParamList>
struct FuncTraits<R (*)(ParamList...)>
{
    static constexpr bool isMemberFunction = false;
    static constexpr bool isConstMemberFunction = false;
    using DeclType = R (*)(ParamList...);
    using ReturnType = R;
    using Params = typename MakeTypeList<ParamList...>::Result;

    static R call(const DeclType& fp, TypeListValues<Params>& tvl)
    {
        return doCall<R>(fp, tvl);
    }
};

// Windows: WINAPI (a.k.a. __stdcall) function pointers (32bit only).
#ifdef _M_IX86
template <class R, class... ParamList>
struct FuncTraits<R(__stdcall*)(ParamList...)>
{
    static constexpr bool isMemberFunction = false;
    static constexpr bool isConstMemberFunction = false;
    using DeclType = R(__stdcall*)(ParamList...);
    using ReturnType = R;
    using Params = typename MakeTypeList<ParamList...>::Result;

    static R call(const DeclType& fp, TypeListValues<Params>& tvl)
    {
        return doCall<R>(fp, tvl);
    }
};
#endif // _M_IX86

// Non-const member function pointers.
template <class T, class R, class... ParamList>
struct FuncTraits<R (T::*)(ParamList...)>
{
    static constexpr bool isMemberFunction = true;
    static constexpr bool isConstMemberFunction = false;
    using DeclType = R (T::*)(ParamList...);
    using ClassType = T;
    using ReturnType = R;
    using Params = typename MakeTypeList<ParamList...>::Result;

    static R call(ClassType* obj, const DeclType& fp, TypeListValues<Params>& tvl)
    {
        return doCall<R>(obj, fp, tvl);
    }
};

// Const member function pointers.
template <class T, class R, class... ParamList>
struct FuncTraits<R (T::*)(ParamList...) const>
{
    static constexpr bool isMemberFunction = true;
    static constexpr bool isConstMemberFunction = true;
    using DeclType = R (T::*)(ParamList...) const;
    using ClassType = T;
    using ReturnType = R;
    using Params = typename MakeTypeList<ParamList...>::Result;

    static R call(const ClassType* obj, const DeclType& fp, TypeListValues<Params>& tvl)
    {
        return doCall<R>(obj, fp, tvl);
    }
};

// std::function
template <class R, class... ParamList>
struct FuncTraits<std::function<R(ParamList...)>>
{
    static constexpr bool isMemberFunction = false;
    static constexpr bool isConstMemberFunction = false;
    using DeclType = std::function<R(ParamList...)>;
    using ReturnType = R;
    using Params = typename MakeTypeList<ParamList...>::Result;

    static ReturnType call(DeclType& fn, TypeListValues<Params>& tvl)
    {
        return doCall<ReturnType>(fn, tvl);
    }
};

//=================================================================================================
/**
 * @brief Invoke object that unpacks the arguments into stack values then call the functor.
 */
template< class ReturnType, class Params, int startParam>
struct Invoke
{
    template <class Fn>
    static int run(lua_State* L, Fn& fn)
    {
        try
        {
            ArgList<Params, startParam> args(L);
            Stack<ReturnType>::push(L, FuncTraits<Fn>::call(fn, args));
            return 1;
        }
        catch (const std::exception& e)
        {
            return luaL_error(L, e.what());
        }
    }

    template <class T, class MemFn>
    static int run(lua_State* L, T* object, const MemFn& fn)
    {
        try
        {
            ArgList<Params, startParam> args(L);
            Stack<ReturnType>::push(L, FuncTraits<MemFn>::call(object, fn, args));
            return 1;
        }
        catch (const std::exception& e)
        {
            return luaL_error(L, e.what());
        }
    }
};

template <class Params, int startParam>
struct Invoke<void, Params, startParam>
{
    template <class Fn>
    static int run(lua_State* L, Fn& fn)
    {
        try
        {
            ArgList<Params, startParam> args(L);
            FuncTraits<Fn>::call(fn, args);
            return 0;
        }
        catch (const std::exception& e)
        {
            return luaL_error(L, e.what());
        }
    }

    template <class T, class MemFn>
    static int run(lua_State* L, T* object, const MemFn& fn)
    {
        try
        {
            ArgList<Params, startParam> args(L);
            FuncTraits<MemFn>::call(object, fn, args);
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
