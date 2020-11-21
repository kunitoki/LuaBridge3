// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT

#pragma once

#include <tuple>

namespace luabridge {
namespace detail {

//=================================================================================================
/**
 * @brief Constructor generators.
 *
 * These templates call operator new with the contents of a type/value list passed to the Constructor
 * with up to 8 parameters. Two versions of call() are provided. One performs a regular new, the other
 * performs a placement new.
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


} // namespace detail
} // namespace luabridge
