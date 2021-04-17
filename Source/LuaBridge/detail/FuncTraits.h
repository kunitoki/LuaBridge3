// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// Copyright 2020, Dmitry Tarakanov
// Copyright 2019, George Tokmaji
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "Config.h"
#include "Errors.h"
#include "Stack.h"
#include "TypeTraits.h"

#include <functional>
#include <tuple>

namespace luabridge {
namespace detail {

//=================================================================================================
/**
 * @brief Generic function traits.
 *
 * @tparam IsMember True if the function is a member function pointer.
 * @tparam IsConst True if the function is const.
 * @tparam R Return type of the function.
 * @tparam Args Arguments types as variadic parameter pack.
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
struct function_traits : std::conditional_t<std::is_class_v<F>,
                                            detail::functor_traits_impl<F>,
                                            detail::function_traits_impl<F>>
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
 *
 * @tparam F Callable object.
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
template <class, class...>
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
 *
 * @tparam Types Argument types that will compose the tuple.
 */
template <class... Types>
constexpr auto tupleize(Types&&... types)
{
    return std::tuple<Types...>(std::forward<Types>(types)...);
}

//=================================================================================================
/**
 * @brief Make argument lists extracting them from the lua state, starting at a stack index.
 *
 * @tparam ArgsPack Arguments pack to extract from the lua stack.
 * @tparam Start Start index where stack variables are located in the lua stack.
 */
template <class ArgsPack, std::size_t Start, std::size_t... Indices>
auto make_arguments_list_impl(lua_State* L, std::index_sequence<Indices...>)
{
    return tupleize(Stack<std::tuple_element_t<Indices, ArgsPack>>::get(L, Start + Indices)...);
}

template <class ArgsPack, std::size_t Start>
auto make_arguments_list(lua_State* L)
{
    return make_arguments_list_impl<ArgsPack, Start>(L, std::make_index_sequence<std::tuple_size_v<ArgsPack>>());
}

//=================================================================================================
/**
 * @brief Helpers for iterating through tuple arguments, pushing each argument to the lua stack.
 */
template <std::size_t Index = 0, typename... Types>
auto push_arguments(lua_State*, const std::tuple<Types...>&, std::error_code&)
    -> std::enable_if_t<Index == sizeof...(Types), std::size_t>
{
    return Index + 1;
}

template <std::size_t Index = 0, typename... Types>
auto push_arguments(lua_State* L, const std::tuple<Types...>& t, std::error_code& ec)
    -> std::enable_if_t<Index < sizeof...(Types), std::size_t>
{
    using T = std::tuple_element_t<Index, std::tuple<Types...>>;

    std::error_code pec;
    bool result = Stack<T>::push(L, std::get<Index>(t), pec);
    if (! result)
    {
        ec = pec;
        return Index + 1;
    }

    return push_arguments<Index + 1, Types...>(L, t, ec);
}

//=================================================================================================
/**
 * @brief Helpers for iterating through tuple arguments, popping each argument from the lua stack.
 */
template <std::ptrdiff_t Start, std::ptrdiff_t Index = 0, typename... Types>
auto pop_arguments(lua_State*, std::tuple<Types...>&)
    -> std::enable_if_t<Index == sizeof...(Types), std::size_t>
{
    return sizeof...(Types);
}

template <std::ptrdiff_t Start, std::ptrdiff_t Index = 0, typename... Types>
auto pop_arguments(lua_State* L, std::tuple<Types...>& t)
    -> std::enable_if_t<Index < sizeof...(Types), std::size_t>
{
    using T = std::tuple_element_t<Index, std::tuple<Types...>>;

    std::get<Index>(t) = Stack<T>::get(L, Start - Index);

    return pop_arguments<Start, Index + 1, Types...>(L, t);
}

//=================================================================================================
/**
 * @brief Remove first type from tuple.
 */
template <class T>
struct remove_first_type
{
};

template <class T, class... Ts>
struct remove_first_type<std::tuple<T, Ts...>>
{
    using type = std::tuple<Ts...>;
};

template <class T>
using remove_first_type_t = typename remove_first_type<T>::type;

//=================================================================================================
/**
 * @brief Function generator.
 */
template <class ReturnType, class ArgsPack, std::size_t Start = 1u>
struct function
{
    template <class F>
    static int call(lua_State* L, F func)
    {
#if LUABRIDGE_HAS_EXCEPTIONS
        try
        {
#endif
            std::error_code ec;
            bool result = Stack<ReturnType>::push(L, std::apply(func, make_arguments_list<ArgsPack, Start>(L)), ec);
            if (! result)
                return luaL_error(L, ec.message().c_str());

            return 1;

#if LUABRIDGE_HAS_EXCEPTIONS
        }
        catch (const std::exception& e)
        {
            return luaL_error(L, e.what());
        }
        catch (...)
        {
            return luaL_error(L, "Error while calling function");
        }
#endif
    }

    template <class T, class F>
    static int call(lua_State* L, T* ptr, F func)
    {
#if LUABRIDGE_HAS_EXCEPTIONS
        try
        {
#endif
            auto f = [ptr, func](auto&&... args) -> ReturnType { return (ptr->*func)(std::forward<decltype(args)>(args)...); };

            std::error_code ec;
            bool result = Stack<ReturnType>::push(L, std::apply(f, make_arguments_list<ArgsPack, Start>(L)), ec);
            if (! result)
                return luaL_error(L, ec.message().c_str());

            return 1;

#if LUABRIDGE_HAS_EXCEPTIONS
        }
        catch (const std::exception& e)
        {
            return luaL_error(L, e.what());
        }
        catch (...)
        {
            return luaL_error(L, "Error while calling method");
        }
#endif
    }
};

template <class ArgsPack, std::size_t Start>
struct function<void, ArgsPack, Start>
{
    template <class F>
    static int call(lua_State* L, F func)
    {
#if LUABRIDGE_HAS_EXCEPTIONS
        try
        {
#endif
            std::apply(func, make_arguments_list<ArgsPack, Start>(L));

            return 0;

#if LUABRIDGE_HAS_EXCEPTIONS
        }
        catch (const std::exception& e)
        {
            return luaL_error(L, e.what());
        }
        catch (...)
        {
            return luaL_error(L, "Error while calling function");
        }
#endif
    }

    template <class T, class F>
    static int call(lua_State* L, T* ptr, F func)
    {
#if LUABRIDGE_HAS_EXCEPTIONS
        try
        {
#endif
            auto f = [ptr, func](auto&&... args) { (ptr->*func)(std::forward<decltype(args)>(args)...); };

            std::apply(f, make_arguments_list<ArgsPack, Start>(L));

            return 0;

#if LUABRIDGE_HAS_EXCEPTIONS
        }
        catch (const std::exception& e)
        {
            return luaL_error(L, e.what());
        }
        catch (...)
        {
            return luaL_error(L, "Error while calling method");
        }
#endif
    }
};

//=================================================================================================
/**
 * @brief Constructor generators.
 *
 * These templates call operator new with the contents of a type/value list passed to the constructor. Two versions of call() are provided.
 * One performs a regular new, the other performs a placement new.
 */
template <class T, class Args>
struct constructor;

template <class T>
struct constructor<T, void>
{
    using empty = std::tuple<>;

    static T* call(const empty&)
    {
        return new T;
    }

    static T* call(void* ptr, const empty&)
    {
        return new (ptr) T;
    }
};

template <class T, class Args>
struct constructor
{
    static T* call(const Args& args)
    {
        auto alloc = [](auto&&... args) { return new T{ std::forward<decltype(args)>(args)... }; };

        return std::apply(alloc, args);
    }

    static T* call(void* ptr, const Args& args)
    {
        auto alloc = [ptr](auto&&... args) { return new (ptr) T{ std::forward<decltype(args)>(args)... }; };

        return std::apply(alloc, args);
    }
};

//=================================================================================================
/**
 * @brief Factory generators.
 */
template <class T>
struct factory
{
    template <class F, class Args>
    static T* call(void* ptr, const F& func, const Args& args)
    {
        auto alloc = [ptr, &func](auto&&... args) { return func(ptr, std::forward<decltype(args)>(args)...); };

        return std::apply(alloc, args);
    }

    template <class F>
    static T* call(void* ptr, const F& func)
    {
        return func(ptr);
    }
};

} // namespace detail
} // namespace luabridge
