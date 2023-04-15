// https://github.com/kunitoki/LuaBridge3
// Copyright 2023, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#pragma once

#include "FlagSet.h"

#include <cstdint>

namespace luabridge {

//=================================================================================================
namespace detail {
struct OptionExtensibleClass;
struct OptionVisibleMetatables;
} // namespace Detail

/**
 * @brief
 */
using Options = FlagSet<uint32_t,
    detail::OptionExtensibleClass,
    detail::OptionVisibleMetatables>;

static inline constexpr Options defaultOptions = Options();
static inline constexpr Options extensibleClass = Options::Value<detail::OptionExtensibleClass>();
static inline constexpr Options visibleMetatables = Options::Value<detail::OptionVisibleMetatables>();

} // namespace luabridge
