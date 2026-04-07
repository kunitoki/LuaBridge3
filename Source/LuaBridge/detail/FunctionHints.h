// https://github.com/kunitoki/LuaBridge3
// Copyright 2024, kunitoki
// SPDX-License-Identifier: MIT

#pragma once

#include "ClassInfo.h"
#include "FuncTraits.h"

#include <string>
#include <type_traits>
#include <vector>

namespace luabridge {

//=================================================================================================
/**
 * @brief A wrapper that attaches Lua parameter name hints to a function or callable.
 *
 * Use luabridge::withHints() to create one. Transparently forwards all function trait queries
 * to the wrapped function type, so it can be used anywhere a plain function is accepted.
 *
 * @code
 * .addFunction("attack", luabridge::withHints(&Enemy::attack, "target", "damage"))
 * @endcode
 */
template <class F>
struct FunctionWithHints
{
    using func_type = F;

    F func;
    std::vector<std::string> hints; ///< Optional parameter name hints, one per Lua-visible parameter
};

/**
 * @brief Attach Lua parameter name hints to a function for use with addFunction / addStaticFunction.
 *
 * @param func       The function, member function pointer, or lambda to wrap.
 * @param paramNames Parameter names in the same order as the Lua-visible parameters.
 *
 * @returns A FunctionWithHints wrapper accepted by all addFunction / addStaticFunction overloads.
 */
template <class F, class... Names>
[[nodiscard]] auto withHints(F&& func, Names&&... paramNames) -> FunctionWithHints<std::decay_t<F>>
{
    return { std::forward<F>(func), { std::string(std::forward<Names>(paramNames))... } };
}

namespace detail {

//=================================================================================================
// Trait: safely unwrap FunctionWithHints<F> to F, identity otherwise.
// Unlike conditional_t<is_function_with_hints_v<T>, typename T::func_type, T>,
// this trait never instantiates the ::func_type member for non-FunctionWithHints types.

template <class T>
struct unwrap_fn_type
{
    using type = T;
};

template <class F>
struct unwrap_fn_type<FunctionWithHints<F>>
{
    using type = F;
};

template <class T>
using unwrap_fn_type_t = typename unwrap_fn_type<T>::type;

//=================================================================================================
// Trait: detect FunctionWithHints<F>

template <class T>
struct is_function_with_hints : std::false_type
{
};

template <class F>
struct is_function_with_hints<FunctionWithHints<F>> : std::true_type
{
};

template <class T>
inline static constexpr bool is_function_with_hints_v = is_function_with_hints<T>::value;

//=================================================================================================
// Make function_traits<FunctionWithHints<F>> delegate to function_traits<F>
// This makes function_result_t<>, function_arguments_t<>, function_arity_v<>, etc. all work
// transparently through a FunctionWithHints wrapper.

template <class F>
struct function_traits<FunctionWithHints<F>> : function_traits<F>
{
};

//=================================================================================================
// Make is_callable<FunctionWithHints<F>> true when F is callable

template <class F>
struct is_callable<FunctionWithHints<F>>
{
    static constexpr bool value = is_callable_v<F>;
};

//=================================================================================================
// Make const-member-pointer detection transparent through FunctionWithHints<F>

template <class F>
struct is_const_member_function_pointer<FunctionWithHints<F>>
{
    static constexpr bool value = is_const_member_function_pointer_v<F>;
};

//=================================================================================================
// Helper: unwrap FunctionWithHints to its inner callable, or pass through unchanged.

template <class F>
[[nodiscard]] decltype(auto) get_underlying(F&& f) noexcept
{
    return std::forward<F>(f);
}

template <class F>
[[nodiscard]] F&& get_underlying(FunctionWithHints<F>&& f) noexcept
{
    return std::move(f.func);
}

template <class F>
[[nodiscard]] F& get_underlying(FunctionWithHints<F>& f) noexcept
{
    return f.func;
}

//=================================================================================================
// Reflection helpers — only compiled when LUABRIDGE_ENABLE_REFLECT is defined.

#if defined(LUABRIDGE_ENABLE_REFLECT)

/**
 * @brief Build a vector of C++ type name strings from a tuple of types.
 *
 * lua_State* parameters are filtered out (they are auto-injected by LuaBridge, not
 * visible from Lua).
 */
template <class Tuple, std::size_t... I>
void reflect_param_type_names_impl(std::vector<std::string>& result, std::index_sequence<I...>)
{
    // Fold expression over comma operator (C++17): invoke a lambda for each index
    (
        [&]()
        {
            using ParamT = std::tuple_element_t<I, Tuple>;

            constexpr bool isLuaState =
                std::is_pointer_v<ParamT> &&
                std::is_same_v<std::remove_const_t<std::remove_pointer_t<ParamT>>, lua_State>;

            if constexpr (!isLuaState)
                result.push_back(std::string(typeName<std::remove_cvref_t<ParamT>>()));
        }(),
        ...);
}

template <class Tuple>
[[nodiscard]] std::vector<std::string> reflect_param_type_names()
{
    std::vector<std::string> result;
    reflect_param_type_names_impl<Tuple>(result, std::make_index_sequence<std::tuple_size_v<Tuple>>{});
    return result;
}

#endif // LUABRIDGE_ENABLE_REFLECT

} // namespace detail
} // namespace luabridge
