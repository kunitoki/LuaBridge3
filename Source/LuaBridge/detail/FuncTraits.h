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
