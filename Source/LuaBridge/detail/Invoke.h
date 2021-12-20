// https://github.com/kunitoki/LuaBridge3
// Copyright 2021, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#pragma once

#include "Config.h"
#include "Errors.h"
#include "Expected.h"
#include "Stack.h"
#include "LuaRef.h"
#include "LuaException.h"

#include <vector>
#include <functional>
#include <optional>
#include <variant>
#include <tuple>

namespace luabridge {

//=================================================================================================
/**
 * @brief Result of a lua invocation.
 */
class LuaResult
{
public:
    /**
     * @brief Get if the result was ok and didn't raise a lua error.
     */
    explicit operator bool() const noexcept
    {
        return m_data.has_value();
    }

    /**
     * @brief Return if the invocation was ok and didn't raise a lua error.
     */
    bool wasOk() const noexcept
    {
        return m_data.has_value();
    }

    /**
     * @brief Return if the invocation did raise a lua error.
     */
    bool hasFailed() const noexcept
    {
        return !m_data.has_value();
    }

    /**
     * @brief Return the error message, if any.
     */
    std::string errorMessage() const noexcept
    {
        if (!m_data.has_value())
            return m_data.error();

        return {};
    }

    /**
     * @brief Return the number of return values.
     */
    std::size_t size() const noexcept
    {
        if (m_data)
            return m_data->size();

        return 0;
    }

    /**
     * @brief Get a return value at a specific index.
     */
    LuaRef operator[](std::size_t index) const
    {
        assert(m_data);
        if (m_data)
        {
            const auto& values = m_data.value();

            assert(index < values.size());
            return values[index];
        }

        return LuaRef(m_L);
    }

private:
    template <class... Args>
    friend LuaResult call(const LuaRef&, Args&&...);

    static LuaResult errorFromStack(lua_State* L, std::error_code ec)
    {
        const auto errorString = lua_tostring(L, -1);
        lua_pop(L, 1);

        return LuaResult(L, errorString ? errorString : ec.message());
    }

    static LuaResult valuesFromStack(lua_State* L, int stackTop)
    {
        std::vector<LuaRef> values;

        const int numReturnedValues = lua_gettop(L) - stackTop;
        if (numReturnedValues > 0)
        {
            values.reserve(numReturnedValues);

            for (int index = numReturnedValues; index > 0; --index)
                values.emplace_back(LuaRef::fromStack(L, -index));

            lua_pop(L, numReturnedValues);
        }

        return LuaResult(L, std::move(values));
    }

    LuaResult(lua_State* L, std::string_view errorString)
        : m_L(L)
        , m_data(make_unexpected(errorString))
    {
    }

    explicit LuaResult(lua_State* L, std::vector<LuaRef> values) noexcept
        : m_L(L)
        , m_data(std::move(values))
    {
    }

    lua_State* m_L = nullptr;
    expected<std::vector<LuaRef>, std::string> m_data;
};

//=================================================================================================
/**
 * @brief Safely call Lua code.
 *
 * These overloads allow Lua code to be called throught lua_pcall.  The return value is provided as
 * a LuaResult which will hold the return values or an error if the call failed.
 *
 * If an error occurs, a LuaException is thrown or if exceptions are disabled the FunctionResult will
 * contain a error code and evaluate false.
 *
 * @note The function might throw a LuaException if the application is compiled with exceptions on
 * and they are enabled globally by calling `enableExceptions` in two cases:
 * - one of the arguments passed cannot be pushed in the stack, for example a unregistered C++ class
 * - the lua invaction calls the panic handler, which is converted to a C++ exception
 *
 * @return A result object.
*/
template <class... Args>
LuaResult call(const LuaRef& object, Args&&... args)
{
    lua_State* L = object.state();
    const int stackTop = lua_gettop(L);

    object.push();

    {
        std::error_code ec;
        auto pushedArgs = detail::push_arguments(L, std::forward_as_tuple(args...), ec);
        if (ec)
        {
            lua_pop(L, static_cast<int>(pushedArgs) + 1);
            return LuaResult(L, ec.message());
        }
    }

    int code = lua_pcall(L, sizeof...(Args), LUA_MULTRET, 0);
    if (code != LUABRIDGE_LUA_OK)
    {
        auto ec = makeErrorCode(ErrorCode::LuaFunctionCallFailed);

#if LUABRIDGE_HAS_EXCEPTIONS
        if (LuaException::areExceptionsEnabled())
            LuaException::raise(LuaException(L, ec));
#else
        return LuaResult::errorFromStack(L, ec);
#endif
    }

    return LuaResult::valuesFromStack(L, stackTop);
}

//=============================================================================================
/**
 * @brief Wrapper for lua_pcall that throws if exceptions are enabled.
 */
inline int pcall(lua_State* L, int nargs = 0, int nresults = 0, int msgh = 0)
{
    const int code = lua_pcall(L, nargs, nresults, msgh);

#if LUABRIDGE_HAS_EXCEPTIONS
    if (code != LUABRIDGE_LUA_OK && LuaException::areExceptionsEnabled())
        LuaException::raise(LuaException(L, makeErrorCode(ErrorCode::LuaFunctionCallFailed)));
#endif

    return code;
}

//=============================================================================================
template <class Impl, class LuaRef>
template <class... Args>
LuaResult LuaRefBase<Impl, LuaRef>::operator()(Args&&... args) const
{
    return call(*this, std::forward<Args>(args)...);
}

} // namespace luabridge
