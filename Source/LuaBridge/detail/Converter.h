// https://github.com/kunitoki/LuaBridge3
// Copyright 2024, kunitoki
// SPDX-License-Identifier: MIT

#pragma once

#include "ClassInfo.h"
#include "Errors.h"
#include "LuaHelpers.h"
#include "Result.h"

#include <cstring>
#include <type_traits>
#include <unordered_map>

namespace luabridge {

// Forward declaration (no default — default is provided by the primary template in Userdata.h).
template <class T, class>
struct Stack;

//=================================================================================================
/**
 * @brief Opt-in trait for enabling custom type converters for type T.
 *
 * Specialize with enabled = true so that Stack<T>::get consults the metatable
 * converter registry as a Phase 3 fallback after Phase 1 (exact match) and
 * Phase 2 (inheritance) both fail.
 *
 * Example:
 *   template <> struct luabridge::StackConversion<MyType> { static constexpr bool enabled = true; };
 */
template <class T>
struct StackConversion
{
    static constexpr bool enabled = false;
};

//=================================================================================================
/**
 * @brief User-defined conversion hook from source type From to target type To.
 *
 * Specialize this template for each (To, From) pair and provide:
 *   static To convert(const From& from);
 *
 * Example:
 *   template <>
 *   struct luabridge::StackConverter<Vec3, glm::vec3> {
 *       static Vec3 convert(const glm::vec3& v) { return {v.x, v.y, v.z}; }
 *   };
 */
template <class To, class From>
struct StackConverter;

namespace detail {

//=================================================================================================
/**
 * @brief C++ registry of converter function pointers, stored as a Lua full userdata in the
 *        FROM class's metatable at getConvertersKey().
 *
 * Each entry maps getClassRegistryKey<To>() → void* (type-erased function pointer).
 */
struct ConverterRegistry
{
    std::unordered_map<const void*, void*> converters;
};

//=================================================================================================
/**
 * @brief Get or create a ConverterRegistry in the class metatable at metatableIdx.
 *
 * If the entry already exists it is returned directly.
 * Otherwise a new Lua full userdata is created (with __gc to call the destructor),
 * stored in the metatable, and returned.
 *
 * Stack-neutral: leaves the stack exactly as it was on entry.
 */
inline ConverterRegistry* getOrCreateConverterRegistry(lua_State* L, int metatableIdx)
{
    const int absIdx = lua_absindex(L, metatableIdx);

    lua_rawgetp_x(L, absIdx, getConvertersKey());
    if (lua_isuserdata(L, -1) && !lua_islightuserdata(L, -1))
    {
        auto* reg = align<ConverterRegistry>(lua_touserdata(L, -1));
        lua_pop(L, 1);
        return reg;
    }
    lua_pop(L, 1); // pop nil or unexpected value

    // Create ConverterRegistry as an aligned Lua full userdata with automatic __gc
    lua_newuserdata_aligned<ConverterRegistry>(L);
    auto* reg = align<ConverterRegistry>(lua_touserdata(L, -1));

    // Store the userdata in the class metatable
    lua_pushvalue(L, -1); // dup userdata
    lua_rawsetp_x(L, absIdx, getConvertersKey()); // store, pops dup
    lua_pop(L, 1); // pop the original userdata

    return reg;
}

} // namespace detail

//=================================================================================================
/**
 * @brief Internal converter dispatch: extracts a From* from the Lua stack and applies
 *        StackConverter<To, From>::convert.  Stored as a type-erased function pointer
 *        in the FROM class's ConverterRegistry under getClassRegistryKey<To>().
 */
template <class To, class From>
TypeResult<To> convertFromStack(lua_State* L, int index)
{
    auto ptr_result = detail::Userdata::get<From>(L, index, true);
    if (!ptr_result || !*ptr_result)
        return makeErrorCode(ErrorCode::InvalidTypeCast);
    return StackConverter<To, From>::convert(**ptr_result);
}

//=================================================================================================
/**
 * @brief Stack specialization for converter-enabled value types.
 *
 * Replaces the default Stack<T, void> for types where StackConversion<T>::enabled == true.
 * Returns TypeResult<T> (owned value) so Phase 3 can return a freshly-constructed T.
 *
 * Extraction order:
 *   Phase 1+2: Standard userdata extraction via Userdata::get<T> (exact match + inheritance).
 *   Phase 3:   Falls back to the ConverterRegistry stored by addConverter<T>().
 */
template <class T>
struct Stack<T, std::enable_if_t<StackConversion<T>::enabled>>
{
    using IsUserdata = void;
    using ReturnType = TypeResult<T>;

    [[nodiscard]] static Result push(lua_State* L, const T& value)
    {
        return detail::StackHelper<T, detail::IsContainer<T>::value>::push(L, value);
    }

    [[nodiscard]] static Result push(lua_State* L, T&& value)
    {
        return detail::StackHelper<T, detail::IsContainer<T>::value>::push(L, std::move(value));
    }

    [[nodiscard]] static ReturnType get(lua_State* L, int index)
    {
        // Phase 1 + 2: exact match and inheritance path — guard with isInstance to
        // avoid LuaException thrown by Userdata::get when the type doesn't match.
        if (detail::Userdata::isInstance<T>(L, index))
        {
            auto ptr_result = detail::Userdata::get<T>(L, index, true);
            if (ptr_result)
            {
                if (T* ptr = *ptr_result)
                    return *ptr;
            }
            return ptr_result.error();
        }

        // Phase 3: look up a registered converter in the value's metatable ConverterRegistry
        if (lua_getmetatable(L, index))
        {
            lua_rawgetp_x(L, -1, detail::getConvertersKey());
            if (lua_isuserdata(L, -1) && !lua_islightuserdata(L, -1))
            {
                auto* reg = align<detail::ConverterRegistry>(lua_touserdata(L, -1));
                lua_pop(L, 2); // registry userdata + metatable

                auto it = reg->converters.find(detail::getClassRegistryKey<T>());
                if (it != reg->converters.end() && it->second)
                {
                    using FnType = TypeResult<T>(*)(lua_State*, int);
                    static_assert(sizeof(FnType) == sizeof(void*),
                        "Function pointer size must match void* for type-erased storage");
                    FnType fn;
                    std::memcpy(&fn, &it->second, sizeof(fn));
                    return fn(L, index);
                }
            }
            else
            {
                lua_pop(L, 2); // nil/other + metatable
            }
        }

        return makeErrorCode(ErrorCode::InvalidTypeCast);
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        return detail::Userdata::isInstance<T>(L, index);
    }
};

//=================================================================================================
/**
 * @brief Stack<const T&> specialization for converter-enabled types.
 *
 * Returns TypeResult<T> (owned value) rather than TypeResult<std::reference_wrapper<T>>,
 * so that Phase 3 converted temporaries have stable lifetime in the calling frame's ArgStorage.
 * Delegates all phases to Stack<T>::get.
 */
template <class T>
struct Stack<const T&, std::enable_if_t<StackConversion<T>::enabled && !std::is_array_v<T>>>
{
    using ReturnType = TypeResult<T>;

    [[nodiscard]] static Result push(lua_State* L, const T& value)
    {
        return Stack<T>::push(L, value);
    }

    [[nodiscard]] static ReturnType get(lua_State* L, int index)
    {
        return Stack<T>::get(L, index);
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        return Stack<T>::isInstance(L, index);
    }
};

} // namespace luabridge
