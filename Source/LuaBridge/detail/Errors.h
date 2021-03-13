// https://github.com/vinniefalco/LuaBridge
// Copyright 2021, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#pragma once

#include "Config.h"

#include <system_error>

namespace luabridge {

//=================================================================================================
/**
 * @brief LuaBridge error codes.
 */
enum class ErrorCode
{
    ClassNotRegistered = 1,

    LuaFunctionCallFailed,
};

//=================================================================================================
namespace detail {
struct ErrorCategory : std::error_category
{
    const char* name() const noexcept override
    {
        return "luabridge";
    }

    std::string message(int ev) const override
    {
        switch (static_cast<ErrorCode>(ev))
        {
        case ErrorCode::ClassNotRegistered:
            return "The class is not registered in LuaBridge";

        case ErrorCode::LuaFunctionCallFailed:
            return "The lua function invocation raised an error";

        default:
            return "Unknown error";
        }
    }

    static const ErrorCategory& getInstance() noexcept
    {
        static ErrorCategory category;
        return category;
    }
};
} // namespace detail

//=================================================================================================
/**
 * @brief Construct an error code from the error enum.
 */
inline std::error_code makeErrorCode(ErrorCode e)
{
  return { static_cast<int>(e), detail::ErrorCategory::getInstance() };
}

} // namespace luabridge

namespace std {
template <> struct is_error_code_enum<luabridge::ErrorCode> : true_type {};
} // namespace std
