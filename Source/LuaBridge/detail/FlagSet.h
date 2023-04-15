// https://github.com/kunitoki/LuaBridge3
// Copyright 2023, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#pragma once

#include "Config.h"

#include <type_traits>
#include <algorithm>
#include <string>

namespace luabridge {

template <class T, class... Ts>
class FlagSet
{
    static_assert(std::is_integral_v<T>);

public:
    constexpr FlagSet() noexcept = default;

    constexpr void set(FlagSet other) noexcept
    {
        flags |= other.flags;
    }

    constexpr FlagSet withSet(FlagSet other) const noexcept
    {
        FlagSet result { flags };
        result.flags |= other.flags;
        return result;
    }

    constexpr void unset(FlagSet other) noexcept
    {
        flags &= ~other.flags;
    }

    constexpr FlagSet withUnset(FlagSet other) const noexcept
    {
        FlagSet result { flags };
        result.flags &= ~other.flags;
        return result;
    }

    constexpr bool test(FlagSet other) const noexcept
    {
        return (flags & other.flags) != 0;
    }

    constexpr FlagSet operator|(FlagSet other) const noexcept
    {
        return FlagSet(flags | other.flags);
    }

    constexpr FlagSet operator&(FlagSet other) const noexcept
    {
        return FlagSet(flags & other.flags);
    }

    constexpr FlagSet operator~() const noexcept
    {
        return FlagSet(~flags);
    }

    std::string toString() const
    {
        std::string result;

        ((result.append(mask<Ts>() & flags) ? '1' : '0'), ...);

        std::reverse(result.begin(), result.end());

        return result;
    }

    template <class... Us>
    static constexpr FlagSet Value() noexcept
    {
        return FlagSet{ mask<Us...>() };
    }

private:
    template <class U, class V, class... Us>
    static constexpr T indexOf() noexcept
    {
        if constexpr (std::is_same_v<U, V>)
            return static_cast<T>(0);
        else
            return static_cast<T>(1) + indexOf<U, Us...>();
    }

    template <class... Us>
    static constexpr T mask() noexcept
    {
        return ((static_cast<T>(1) << indexOf<Us, Ts...>()) | ...);
    }

    constexpr FlagSet(T flags) noexcept
        : flags(flags)
    {
    }

    T flags = 0;
};

} // namespace luabridge