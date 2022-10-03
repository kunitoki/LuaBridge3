// https://github.com/kunitoki/LuaBridge3
// Copyright 2022, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#pragma once

#include "Errors.h"

namespace luabridge {

//=================================================================================================
/**
 * @brief Simple result class containing an error code.
 */
struct Result
{
    Result() = default;

    Result(std::error_code ec) noexcept
        : m_ec(ec)
    {
    }

    explicit operator bool() const noexcept
    {
        return !m_ec;
    }

    operator std::error_code() const noexcept
    {
        return m_ec;
    }

    std::string message() const
    {
        return m_ec.message();
    }

private:
    std::error_code m_ec;
};

} // namespace luabridge
