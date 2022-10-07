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

template <class... Args>
struct non_const_overload
{
    template <class R, class T>
    constexpr auto operator()(R (T::*ptr)(Args...)) const noexcept -> decltype(ptr)
    {
        return ptr;
    }
};

template <class... Args>
struct const_overload
{
    template <class R, class T>
    constexpr auto operator()(R (T::*ptr)(Args...) const) const noexcept -> decltype(ptr)
    {
        return ptr;
    }
};

template <class... Args>
struct overload : const_overload<Args...>, non_const_overload<Args...>
{
    using const_overload<Args...>::operator();
    using non_const_overload<Args...>::operator();

    template <class R>
    constexpr auto operator()(R (*ptr)(Args...)) const noexcept -> decltype(ptr)
    {
        return ptr;
    }
};

} // namespace detail

//=================================================================================================
/**
 * @brief Overload resolution.
 */
template <class... Args> constexpr detail::overload<Args...> overload = {};
template <class... Args> constexpr detail::const_overload<Args...> constOverload = {};
template <class... Args> constexpr detail::non_const_overload<Args...> nonConstOverload = {};

} // namespace luabridge
