// https://github.com/kunitoki/LuaBridge3
// Copyright 2021, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#pragma once

#include "Config.h"
#include "Errors.h"
#include "Stack.h"
#include "LuaRef.h"

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
     * @brief
     */
    explicit operator bool() const noexcept
    {
        return !m_ec;
    }

    /**
     * @brief
     */
    bool wasOk() const noexcept
    {
        return !m_ec;
    }

    /**
     * @brief
     */
    bool hasFailed() const noexcept
    {
        return !!m_ec;
    }

    /**
     * @brief
     */
    std::error_code errorCode() const noexcept
    {
        return m_ec;
    }

    /**
     * @brief
     */
    std::string errorMessage() const noexcept
    {
        return std::holds_alternative<std::string>(m_data) ? std::get<std::string>(m_data) : std::string();
    }

    /**
     * @brief
     */
    std::size_t getNumValues() const noexcept
    {
        if (std::holds_alternative<std::vector<LuaRef>>(m_data))
            return std::get<std::vector<LuaRef>>(m_data).size();
        
        return 0;
    }
    
    /**
     * @brief
     */
    LuaRef getValue(std::size_t index) const
    {
        assert(m_ec == std::error_code());

        if (std::holds_alternative<std::vector<LuaRef>>(m_data))
        {
            const auto& values = std::get<std::vector<LuaRef>>(m_data);

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
        auto errorString = lua_tostring(L, -1);

        lua_pop(L, 1);
        
        return LuaResult(L, ec, errorString ? errorString : "");
    }

    static LuaResult valuesFromStack(lua_State* L)
    {
        std::vector<LuaRef> values;
        
        const int numReturnedValues = lua_gettop(L) - 1;
        if (numReturnedValues > 0)
        {
            values.reserve(numReturnedValues);

            for (int index = 0; index < numReturnedValues; ++index)
            {
                values.insert(std::begin(values), LuaRef::fromStack(L, -1));
                lua_pop(L, 1);
            }
        }
        
        return LuaResult(L, std::move(values));
    }

    LuaResult(lua_State* L, std::error_code ec, std::string_view errorString)
        : m_L(L)
        , m_ec(ec)
        , m_data(std::string(errorString))
    {
    }

    explicit LuaResult(lua_State* L, std::vector<LuaRef> values) noexcept
        : m_L(L)
        , m_data(std::move(values))
    {
    }
    
    lua_State* m_L = nullptr;
    std::error_code m_ec;
    std::variant<std::vector<LuaRef>, std::string> m_data;
};

//=================================================================================================
/**
 * @brief Safely call Lua code.
 *
 * These overloads allow Lua code to be called throught lua_pcall.  The return value is provided as a LuaResult which will hold the return
 * values or an error if the call failed.
 *
 * If an error occurs, a LuaException is thrown or if exceptions are disabled the FunctionResult will contain a error code and evaluate false.
 *
 * @returns A result of the call.
*/
template <class... Args>
LuaResult call(const LuaRef& object, Args&&... args)
{
    lua_State* L = object.state();
    
    object.push();

    {
        std::error_code ec;
        auto pushedArgs = detail::push_arguments(L, std::forward_as_tuple(args...), ec);
        if (ec)
        {
            lua_pop(L, pushedArgs + 1);
            return LuaResult(L, ec, ec.message());
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
        return LuaResult(L, ec, ec.message());
#endif
    }

    return LuaResult::valuesFromStack(L);
}

//=============================================================================================
/**
 * @brief Wrapper for lua_pcall that throws if exceptions are enabled.
 */
static int pcall(lua_State* L, int nargs = 0, int nresults = 0, int msgh = 0)
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
