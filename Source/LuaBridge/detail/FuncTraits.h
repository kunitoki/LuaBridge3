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
#include "Userdata.h"

#include <functional>
#include <tuple>

namespace luabridge {
namespace detail {
//=================================================================================================
/**
 * @brief Provides the member typedef type which is the type referred to by T with its topmost cv-qualifiers removed.
 */
template< class T >
struct remove_cvref
{
    typedef std::remove_cv_t<std::remove_reference_t<T>> type;
};

template <class T>
using remove_cvref_t = typename remove_cvref<T>::type;

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
 * @brief Deduces the argument type of a callble object or void in case it has no argument.
 *
 * @tparam I Argument index.
 * @tparam F Callable object.
 */
template <std::size_t I, class F, class = void>
struct function_argument_or_void
{
    using type = void;
};

template <std::size_t I, class F>
struct function_argument_or_void<I, F, std::enable_if_t<I < std::tuple_size_v<typename function_traits<F>::argument_types>>>
{
    using type = std::tuple_element_t<I, typename function_traits<F>::argument_types>;
};

template <std::size_t I, class F>
using function_argument_or_void_t = typename function_argument_or_void<I, F>::type;

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
 * @brief Detect if we T is a callable object.
 *
 * @tparam T Potentially callable object.
 */
template <class T, class = void>
struct is_callable
{
    static constexpr bool value = false;
};

template <class T>
struct is_callable<T, std::void_t<decltype(&T::operator())>>
{
    static constexpr bool value = true;
};

template <class T>
struct is_callable<T, std::enable_if_t<std::is_pointer_v<T> && std::is_function_v<std::remove_pointer_t<T>>>>
{
    static constexpr bool value = true;
};

template <class T>
struct is_callable<T, std::enable_if_t<std::is_member_function_pointer_v<T>>>
{
    static constexpr bool value = true;
};

template <class T>
inline static constexpr bool is_callable_v = is_callable<T>::value;

//=================================================================================================
/**
 * @brief Detect if we T is a const member function pointer.
 *
 * @tparam T Potentially const member function pointer.
 */
template <class T>
struct is_const_member_function_pointer
{
    static constexpr bool value = false;
};

template <class T, class R, class... Args>
struct is_const_member_function_pointer<R (T::*)(Args...)>
{
    static constexpr bool value = false;
};

template <class T, class R, class... Args>
struct is_const_member_function_pointer<R (T::*)(Args...) const>
{
    static constexpr bool value = true;
};

template <class T>
inline static constexpr bool is_const_member_function_pointer_v = is_const_member_function_pointer<T>::value;

//=================================================================================================
/**
 * @brief Detect if we T is a member lua cfunction pointer.
 *
 * @tparam T Potentially member lua cfunction pointer.
 */
template <class T>
struct is_member_cfunction_pointer
{
    static constexpr bool value = false;
};

template <class T>
struct is_member_cfunction_pointer<int (T::*)(lua_State*)>
{
    static constexpr bool value = true;
};

template <class T>
struct is_member_cfunction_pointer<int (T::*)(lua_State*) const>
{
    static constexpr bool value = true;
};

template <class T>
inline static constexpr bool is_member_cfunction_pointer_v = is_member_cfunction_pointer<T>::value;

/**
 * @brief Detect if we T is a const member lua cfunction pointer.
 *
 * @tparam T Potentially const member lua cfunction pointer.
 */
template <class T>
struct is_const_member_cfunction_pointer
{
    static constexpr bool value = false;
};

template <class T>
struct is_const_member_cfunction_pointer<int (T::*)(lua_State*)>
{
    static constexpr bool value = false;
};

template <class T>
struct is_const_member_cfunction_pointer<int (T::*)(lua_State*) const>
{
    static constexpr bool value = true;
};

template <class T>
inline static constexpr bool is_const_member_cfunction_pointer_v = is_const_member_cfunction_pointer<T>::value;

//=================================================================================================
/**
 * @brief A constexpr check for proxy_member functions.
 *
 * @tparam T Type where the callable should be able to operate.
 * @tparam F Callable object.
 */

template <class T, class F>
inline static constexpr bool is_proxy_member_function_v =
    std::is_same_v<T, remove_cvref_t<std::remove_pointer_t<function_argument_or_void_t<0, F>>>>;

template <class T, class F>
inline static constexpr bool is_const_proxy_function_v =
    is_proxy_member_function_v<T, F> &&
    std::is_const_v<std::remove_pointer_t<function_argument_or_void_t<0, F>>>;

//=================================================================================================
/**
 * @brief An integral constant expression that gives the number of arguments excluding one type (usually used with lua_State*) accepted by the callable object.
 *
 * @tparam F Callable object.
 */
template <class, class>
struct function_arity_excluding
{
};

template < class... Ts, class ExclusionType>
struct function_arity_excluding<std::tuple<Ts...>, ExclusionType>
    : std::integral_constant<std::size_t, (0 + ... + (std::is_same_v<std::decay_t<Ts>, ExclusionType> ? 0 : 1))>
{
};

template <class F, class ExclusionType>
inline static constexpr std::size_t function_arity_excluding_v = function_arity_excluding<function_arguments_t<F>, ExclusionType>::value;

/**
 * @brief An integral constant expression that gives the number of arguments excluding one type (usually used with lua_State*) accepted by the callable object.
 *
 * @tparam F Callable object.
 */
template <class, class, class, class, class = void>
struct member_function_arity_excluding
{
};

template <class T, class F, class... Ts, class ExclusionType>
struct member_function_arity_excluding<T, F, std::tuple<Ts...>, ExclusionType, std::enable_if_t<!is_proxy_member_function_v<T, F>>>
    : std::integral_constant<std::size_t, (0 + ... + (std::is_same_v<std::decay_t<Ts>, ExclusionType> ? 0 : 1))>
{
};

template <class T, class F, class... Ts, class ExclusionType>
struct member_function_arity_excluding<T, F, std::tuple<Ts...>, ExclusionType, std::enable_if_t<is_proxy_member_function_v<T, F>>>
    : std::integral_constant<std::size_t, (0 + ... + (std::is_same_v<std::decay_t<Ts>, ExclusionType> ? 0 : 1)) - 1>
{
};

template <class T, class F, class ExclusionType>
inline static constexpr std::size_t member_function_arity_excluding_v = member_function_arity_excluding<T, F, function_arguments_t<F>, ExclusionType>::value;

//=================================================================================================
template <class T, class F>
static constexpr bool is_const_function =
    detail::is_const_member_function_pointer_v<F> ||
        (detail::function_arity_v<F> > 0 && detail::is_const_proxy_function_v<T, F>);

template <class T, class... Fs>
inline static constexpr std::size_t const_functions_count = (0 + ... + (is_const_function<T, Fs> ? 1 : 0));

template <class T, class... Fs>
inline static constexpr std::size_t non_const_functions_count = (0 + ... + (is_const_function<T, Fs> ? 0 : 1));

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

template <class ReturnType, class... Args>
struct to_std_function_type<ReturnType, std::tuple<Args...>>
{
    using type = std::function<ReturnType(Args...)>;
};

template <class ReturnType, class... Args>
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
template <class T>
auto unwrap_argument_or_error(lua_State* L, std::size_t index)
{
    auto result = Stack<T>::get(L, index);
    if (! result)
        luaL_error(L, "Error decoding argument #%d: %s", static_cast<int>(index), result.message().c_str());

    return *result;
}

template <class ArgsPack, std::size_t Start, std::size_t... Indices>
auto make_arguments_list_impl(lua_State* L, std::index_sequence<Indices...>)
{
    return tupleize(unwrap_argument_or_error<std::tuple_element_t<Indices, ArgsPack>>(L, Start + Indices)...);
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
template <std::size_t Index = 0, class... Types>
auto push_arguments(lua_State*, std::tuple<Types...>)
    -> std::enable_if_t<Index == sizeof...(Types), std::tuple<Result, std::size_t>>
{
    return std::make_tuple(Result(), Index + 1);
}

template <std::size_t Index = 0, class... Types>
auto push_arguments(lua_State* L, std::tuple<Types...> t)
    -> std::enable_if_t<Index < sizeof...(Types), std::tuple<Result, std::size_t>>
{
    using T = std::tuple_element_t<Index, std::tuple<Types...>>;

    auto result = Stack<T>::push(L, std::get<Index>(t));
    if (! result)
        return std::make_tuple(result, Index + 1);

    return push_arguments<Index + 1, Types...>(L, std::move(t));
}

//=================================================================================================
/**
 * @brief Helpers for iterating through tuple arguments, popping each argument from the lua stack.
 */
template <std::ptrdiff_t Start, std::ptrdiff_t Index = 0, class... Types>
auto pop_arguments(lua_State*, std::tuple<Types...>&)
    -> std::enable_if_t<Index == sizeof...(Types), std::size_t>
{
    return sizeof...(Types);
}

template <std::ptrdiff_t Start, std::ptrdiff_t Index = 0, class... Types>
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
        Result result;

#if LUABRIDGE_HAS_EXCEPTIONS
        try
        {
#endif
            result = Stack<ReturnType>::push(L, std::apply(func, make_arguments_list<ArgsPack, Start>(L)));

#if LUABRIDGE_HAS_EXCEPTIONS
        }
        catch (const std::exception& e)
        {
            raise_lua_error(L, "%s", e.what());
        }
#endif

        if (! result)
            raise_lua_error(L, "%s", result.message().c_str());

        return 1;
    }

    template <class T, class F>
    static int call(lua_State* L, T* ptr, F func)
    {
        Result result;

#if LUABRIDGE_HAS_EXCEPTIONS
        try
        {
#endif
            auto f = [ptr, func](auto&&... args) -> ReturnType { return (ptr->*func)(std::forward<decltype(args)>(args)...); };

            result = Stack<ReturnType>::push(L, std::apply(f, make_arguments_list<ArgsPack, Start>(L)));

#if LUABRIDGE_HAS_EXCEPTIONS
        }
        catch (const std::exception& e)
        {
            raise_lua_error(L, "%s", e.what());
        }
#endif

        if (! result)
            raise_lua_error(L, "%s", result.message().c_str());

        return 1;
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

#if LUABRIDGE_HAS_EXCEPTIONS
        }
        catch (const std::exception& e)
        {
            raise_lua_error(L, "%s", e.what());
        }
#endif

        return 0;
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

#if LUABRIDGE_HAS_EXCEPTIONS
        }
        catch (const std::exception& e)
        {
            raise_lua_error(L, "%s", e.what());
        }
#endif

        return 0;
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
        auto alloc = [](auto&&... args) { return new T(std::forward<decltype(args)>(args)...); };

        return std::apply(alloc, args);
    }

    static T* call(void* ptr, const Args& args)
    {
        auto alloc = [ptr](auto&&... args) { return new (ptr) T(std::forward<decltype(args)>(args)...); };

        return std::apply(alloc, args);
    }
};

//=================================================================================================
/**
 * @brief Placement constructor generators.
 */
template <class T>
struct placement_constructor
{
    template <class F, class Args>
    static T* construct(void* ptr, const F& func, const Args& args)
    {
        auto alloc = [ptr, &func](auto&&... args) { return func(ptr, std::forward<decltype(args)>(args)...); };

        return std::apply(alloc, args);
    }

    template <class F>
    static T* construct(void* ptr, const F& func)
    {
        return func(ptr);
    }
};

//=================================================================================================
/**
 * @brief External allocator generators.
 */
template <class T>
struct external_constructor
{
    template <class F, class Args>
    static T* construct(const F& func, const Args& args)
    {
        auto alloc = [&func](auto&&... args) { return func(std::forward<decltype(args)>(args)...); };

        return std::apply(alloc, args);
    }

    template <class F>
    static T* construct(const F& func)
    {
        return func();
    }
};

//=================================================================================================
/**
 * @brief lua_CFunction to construct a class object wrapped in a container.
 */
template <class C, class Args>
int constructor_container_proxy(lua_State* L)
{
    using T = typename ContainerTraits<C>::Type;

    T* object = detail::constructor<T, Args>::call(detail::make_arguments_list<Args, 2>(L));

    auto result = detail::UserdataSharedHelper<C, false>::push(L, object);
    if (! result)
        luaL_error(L, "%s", result.message().c_str());

    return 1;
}

/**
 * @brief lua_CFunction to construct a class object in-place in the userdata.
 */
template <class T, class Args>
int constructor_placement_proxy(lua_State* L)
{
    std::error_code ec;
    auto* value = detail::UserdataValue<T>::place(L, ec);
    if (! value)
        luaL_error(L, "%s", ec.message().c_str());

    detail::constructor<T, Args>::call(value->getObject(), detail::make_arguments_list<Args, 2>(L));

    value->commit();

    return 1;
}

//=================================================================================================
/**
 * @brief Constructor forwarder.
 */
template <class T, class F>
struct constructor_forwarder
{
    explicit constructor_forwarder(F f)
        : m_func(std::move(f))
    {
    }

    T* operator()(lua_State* L)
    {
        std::error_code ec;
        auto* value = UserdataValue<T>::place(L, ec);
        if (! value)
            luaL_error(L, "%s", ec.message().c_str());

        using FnTraits = function_traits<F>;
        using FnArgs = remove_first_type_t<typename FnTraits::argument_types>;

        T* obj = placement_constructor<T>::construct(
            value->getObject(), m_func, make_arguments_list<FnArgs, 2>(L));

        value->commit();

        return obj;
    }

private:
    F m_func;
};

//=================================================================================================
/**
 * @brief Constructor forwarder.
 */
template <class T, class Alloc, class Dealloc>
struct factory_forwarder
{
    explicit factory_forwarder(Alloc alloc, Dealloc dealloc)
        : m_alloc(std::move(alloc))
        , m_dealloc(std::move(dealloc))
    {
    }

    T* operator()(lua_State* L)
    {
        using FnTraits = function_traits<Alloc>;
        using FnArgs = typename FnTraits::argument_types;

        T* obj = external_constructor<T>::construct(m_alloc, make_arguments_list<FnArgs, 0>(L));

        std::error_code ec;
        auto* value = UserdataValueExternal<T>::place(L, obj, m_dealloc, ec);
        if (! value)
            luaL_error(L, "%s", ec.message().c_str());

        return obj;
    }

private:
    Alloc m_alloc;
    Dealloc m_dealloc;
};

} // namespace detail
} // namespace luabridge
