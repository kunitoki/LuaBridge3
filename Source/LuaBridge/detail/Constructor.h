// https://github.com/kunitoki/LuaBridge
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT

#pragma once

#include "TypeList.h"

namespace luabridge {
namespace detail {

/** Constructor generators.

    These templates call operator new with the contents of a type/value
    list passed to the Constructor with up to 8 parameters. Two versions
    of call() are provided. One performs a regular new, the other performs
    a placement new.
*/
template <class T, class Params>
struct Constructor;

template <class T>
struct Constructor<T, None>
{
    static T* call(const TypeListValues<None>&)
    {
        return new T;
    }

    static T* call(void* ptr, const TypeListValues<None>&)
    {
        return new (ptr) T;
    }
};

template<class T, class Params>
struct Constructor
{
    static T* call(const TypeListValues<Params>& tvl)
    {
        auto alloc = [](auto&&... args) { return new T(std::forward<decltype(args)>(args)...); };
        
        return std::apply(alloc, typeListValuesTuple(tvl));
    }
    
    static T* call(void* ptr, const TypeListValues<Params>& tvl)
    {
        auto alloc = [ptr](auto&&... args) { return new (ptr) T(std::forward<decltype(args)>(args)...); };

        return std::apply(alloc, typeListValuesTuple(tvl));
    }
};

} // namespace detail
} // namespace luabridge
