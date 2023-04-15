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
struct OptionAllowOverridingMethods;
struct OptionVisibleMetatables;
} // namespace Detail

/**
 * @brief
 */
using Options = FlagSet<uint32_t,
    detail::OptionExtensibleClass,
    detail::OptionAllowOverridingMethods,
    detail::OptionVisibleMetatables>;

/**
 * @brief
 */
static inline constexpr Options defaultOptions = Options();

/**
 * @brief
 */
static inline constexpr Options extensibleClass = Options::Value<detail::OptionExtensibleClass>();

/**
 * @brief
 */
static inline constexpr Options allowOverridingMethods = Options::Value<detail::OptionAllowOverridingMethods>();

/**
 * @brief
 */
static inline constexpr Options visibleMetatables = Options::Value<detail::OptionVisibleMetatables>();

} // namespace luabridge
