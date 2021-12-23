// https://github.com/vinniefalco/LuaBridge
// Copyright 2021, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#pragma once

#include "Config.h"

#include <type_traits>
#include <memory>
#include <utility>
#include <any>

#if LUABRIDGE_HAS_EXCEPTIONS
#include <stdexcept>
#endif

namespace luabridge {
namespace detail {
using std::swap;

template <class T>
struct static_const
{
    static constexpr T value{};
};

template <class T>
constexpr T static_const<T>::value;

template <class T, class... Args>
T* construct_at(T* ptr, Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value)
{
    return static_cast<T*>(::new (const_cast<void*>(static_cast<const void*>(ptr))) T(std::forward<Args>(args)...));
}

template <class T, class U, class = void>
struct is_swappable_with_impl : std::false_type
{
};

template <class T, class U>
struct is_swappable_with_impl<T, U, std::void_t<decltype(swap(std::declval<T>(), std::declval<U>()))>>
    : std::true_type
{
};

template <class T, class U>
struct is_nothrow_swappable_with_impl
{
    static constexpr bool value = noexcept(swap(std::declval<T>(), std::declval<U>())) && noexcept(swap(std::declval<U>(), std::declval<T>()));

    using type = std::bool_constant<value>;
};

template <class T, class U>
struct is_swappable_with
    : std::conjunction<
          is_swappable_with_impl<std::add_lvalue_reference_t<T>, std::add_lvalue_reference_t<U>>,
          is_swappable_with_impl<std::add_lvalue_reference_t<U>, std::add_lvalue_reference_t<T>>>::type
{
};

template <class T, class U>
struct is_nothrow_swappable_with
    : std::conjunction<is_swappable_with<T, U>, is_nothrow_swappable_with_impl<T, U>>::type
{
};

template <class T>
struct is_nothrow_swappable
    : std::is_nothrow_swappable_with<std::add_lvalue_reference_t<T>, std::add_lvalue_reference_t<T>>
{
};
} // namespace detail

/**
 * @brief Forward for expected.
 */
template <class T, class E>
class expected;

/**
 * @brief Tag type for unexpected outcome.
 */
struct unexpect_t
{
    constexpr unexpect_t() = default;
};

/**
 * @brief Tag instance for unexpected outcome.
 */
static constexpr const auto& unexpect = detail::static_const<unexpect_t>::value;

namespace detail {
template <class T, class E, bool = (std::is_void_v<T> || std::is_trivial_v<T>) && std::is_trivial_v<E>>
union expected_storage
{
public:
    constexpr expected_storage() noexcept
        : value_()
    {
    }

    template <class... Args>
    constexpr explicit expected_storage(std::in_place_t, Args&&... args) noexcept
        : value_(std::forward<Args>(args)...)
    {
    }

    template <class... Args>
    constexpr explicit expected_storage(unexpect_t, Args&&... args) noexcept
        : error_(std::forward<Args>(args)...)
    {
    }

    ~expected_storage() = default;

    constexpr const T& value() const noexcept
    {
        return value_;
    }

    constexpr T& value() noexcept
    {
        return value_;
    }

    constexpr const E& error() const noexcept
    {
        return error_;
    }

    constexpr E& error() noexcept
    {
        return error_;
    }

private:
    T value_;
    E error_;
};

template <class E>
union expected_storage<void, E, true>
{
public:
    constexpr expected_storage() noexcept
        : dummy_(0)
    {
    }

    template <class... Args>
    constexpr explicit expected_storage(unexpect_t, Args&&... args) noexcept
        : error_(std::forward<Args>(args)...)
    {
    }

    ~expected_storage() = default;

    constexpr const E& error() const noexcept
    {
        return error_;
    }

    constexpr E& error() noexcept
    {
        return error_;
    }

private:
    char dummy_;
    E error_;
};

template <class T, class E>
union expected_storage<T, E, false>
{
public:
    constexpr expected_storage() noexcept(std::is_nothrow_default_constructible_v<T>)
        : value_()
    {
    }

    template <class... Args>
    constexpr explicit expected_storage(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : value_(std::forward<Args>(args)...)
    {
    }

    template <class... Args>
    constexpr explicit expected_storage(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<E, Args...>)
        : error_(std::forward<Args>(args)...)
    {
    }

    ~expected_storage()
    {
    }

    constexpr const T& value() const noexcept
    {
        return value_;
    }

    constexpr T& value() noexcept
    {
        return value_;
    }

    constexpr const E& error() const noexcept
    {
        return error_;
    }

    constexpr E& error() noexcept
    {
        return error_;
    }

private:
    T value_;
    E error_;
};

template <class E>
union expected_storage<void, E, false>
{
public:
    constexpr expected_storage() noexcept
        : dummy_(0)
    {
    }

    template <class... Args>
    constexpr explicit expected_storage(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<E, Args...>)
        : error_(std::forward<Args>(args)...)
    {
    }

    ~expected_storage()
    {
    }

    constexpr const E& error() const noexcept
    {
        return error_;
    }

    constexpr E& error() noexcept
    {
        return error_;
    }

private:
    char dummy_;
    E error_;
};

template <class T, class E, bool IsDefaultConstructible, bool IsCopyConstructible, bool IsMoveConstructible>
class expected_base_trivial
{
    using this_type = expected_base_trivial<T, E, IsDefaultConstructible, IsCopyConstructible, IsMoveConstructible>;

protected:
    using storage_type = expected_storage<T, E>;

    constexpr expected_base_trivial() noexcept
        : valid_(true)
    {
    }

    template <class... Args>
    constexpr expected_base_trivial(std::in_place_t, Args&&... args) noexcept
        : storage_(std::in_place, std::forward<Args>(args)...)
        , valid_(true)
    {
    }

    template <class... Args>
    constexpr expected_base_trivial(unexpect_t, Args&&... args) noexcept
        : storage_(unexpect, std::forward<Args>(args)...)
        , valid_(false)
    {
    }

    expected_base_trivial(const expected_base_trivial& other) noexcept
    {
        if (other.valid_)
        {
            construct(std::in_place, other.value());
        }
        else
        {
            construct(unexpect, other.error());
        }
    }

    expected_base_trivial(expected_base_trivial&& other) noexcept
    {
        if (other.valid_)
        {
            construct(std::in_place, std::move(other.value()));
        }
        else
        {
            construct(unexpect, std::move(other.error()));
        }
    }

    ~expected_base_trivial() noexcept = default;

    constexpr const T& value() const noexcept
    {
        return storage_.value();
    }

    constexpr T& value() noexcept
    {
        return storage_.value();
    }

    constexpr const E& error() const noexcept
    {
        return storage_.error();
    }

    constexpr E& error() noexcept
    {
        return storage_.error();
    }

    constexpr const T* value_ptr() const noexcept
    {
        return std::addressof(value());
    }

    constexpr T* value_ptr() noexcept
    {
        return std::addressof(value());
    }

    constexpr const E* error_ptr() const noexcept
    {
        return std::addressof(error());
    }

    constexpr E* error_ptr() noexcept
    {
        return std::addressof(error());
    }

    constexpr bool valid() const noexcept
    {
        return valid_;
    }

    template <class... Args>
    inline T& construct(std::in_place_t, Args&&... args) noexcept
    {
        valid_ = true;
        return *detail::construct_at(value_ptr(), std::forward<Args>(args)...);
    }

    template <class... Args>
    inline E& construct(unexpect_t, Args&&... args) noexcept
    {
        valid_ = false;
        return *detail::construct_at(error_ptr(), std::forward<Args>(args)...);
    }

    inline void destroy() noexcept
    {
    }

private:
    storage_type storage_;
    bool valid_;
};

template <class T, class E, bool IsCopyConstructible, bool IsMoveConstructible>
class expected_base_trivial<T, E, false, IsCopyConstructible, IsMoveConstructible>
{
    using this_type = expected_base_trivial<T, E, false, IsCopyConstructible, IsMoveConstructible>;

protected:
    using storage_type = expected_storage<T, E>;

    template <class... Args>
    constexpr expected_base_trivial(std::in_place_t, Args&&... args) noexcept
        : storage_(std::in_place, std::forward<Args>(args)...)
        , valid_(true)
    {
    }

    template <class... Args>
    constexpr expected_base_trivial(unexpect_t, Args&&... args) noexcept
        : storage_(unexpect, std::forward<Args>(args)...)
        , valid_(false)
    {
    }

    expected_base_trivial(const expected_base_trivial& other) noexcept
    {
        if (other.valid_)
        {
            construct(std::in_place, other.value());
        }
        else
        {
            construct(unexpect, other.error());
        }
    }

    expected_base_trivial(expected_base_trivial&& other) noexcept
    {
        if (other.valid_)
        {
            construct(std::in_place, std::move(other.value()));
        }
        else
        {
            construct(unexpect, std::move(other.error()));
        }
    }

    ~expected_base_trivial() noexcept = default;

    constexpr const T& value() const noexcept
    {
        return storage_.value();
    }

    constexpr T& value() noexcept
    {
        return storage_.value();
    }

    constexpr const E& error() const noexcept
    {
        return storage_.error();
    }

    constexpr E& error() noexcept
    {
        return storage_.error();
    }

    constexpr const T* value_ptr() const noexcept
    {
        return std::addressof(value());
    }

    constexpr T* value_ptr() noexcept
    {
        return std::addressof(value());
    }

    constexpr const E* error_ptr() const noexcept
    {
        return std::addressof(error());
    }

    constexpr E* error_ptr() noexcept
    {
        return std::addressof(error());
    }

    constexpr bool valid() const noexcept
    {
        return valid_;
    }

    template <class... Args>
    inline T& construct(std::in_place_t, Args&&... args) noexcept
    {
        valid_ = true;
        return *detail::construct_at(value_ptr(), std::forward<Args>(args)...);
    }

    template <class... Args>
    inline E& construct(unexpect_t, Args&&... args) noexcept
    {
        valid_ = false;
        return *detail::construct_at(error_ptr(), std::forward<Args>(args)...);
    }

    inline void destroy() noexcept
    {
    }

private:
    storage_type storage_;
    bool valid_;
};

template <class E, bool IsCopyConstructible, bool IsMoveConstructible>
class expected_base_trivial<void, E, true, IsCopyConstructible, IsMoveConstructible>
{
    using this_type = expected_base_trivial<void, E, true, IsCopyConstructible, IsMoveConstructible>;

protected:
    using storage_type = expected_storage<void, E>;

    constexpr expected_base_trivial() noexcept
        : valid_(true)
    {
    }

    template <class... Args>
    constexpr expected_base_trivial(unexpect_t, Args&&... args) noexcept
        : storage_(unexpect, std::forward<Args>(args)...)
        , valid_(false)
    {
    }

    expected_base_trivial(const expected_base_trivial& other) noexcept
    {
        if (other.valid_)
        {
            destroy();
            valid_ = true;
        }
        else
        {
            construct(unexpect, other.error());
        }
    }

    expected_base_trivial(expected_base_trivial&& other) noexcept
    {
        if (other.valid_)
        {
            destroy();
            valid_ = true;
        }
        else
        {
            construct(unexpect, std::move(other.error()));
        }
    }

    ~expected_base_trivial() noexcept = default;

    constexpr const E& error() const noexcept
    {
        return storage_.error();
    }

    constexpr E& error() noexcept
    {
        return storage_.error();
    }

    constexpr const E* error_ptr() const noexcept
    {
        return std::addressof(error());
    }

    constexpr E* error_ptr() noexcept
    {
        return std::addressof(error());
    }

    constexpr bool valid() const noexcept
    {
        return valid_;
    }

    template <class... Args>
    inline void construct(std::in_place_t, Args&&... args) noexcept
    {
        unused(args...);
        valid_ = true;
    }

    template <class... Args>
    inline E& construct(unexpect_t, Args&&... args) noexcept
    {
        valid_ = false;
        return *detail::construct_at(error_ptr(), std::forward<Args>(args)...);
    }

    inline void destroy() noexcept
    {
    }

private:
    storage_type storage_;
    bool valid_;
};

template <class T, class E, bool IsDefaultConstructible, bool IsCopyConstructible, bool IsMoveConstructible>
class expected_base_non_trivial
{
    using this_type = expected_base_non_trivial<T, E, IsDefaultConstructible, IsCopyConstructible, IsMoveConstructible>;

protected:
    using storage_type = expected_storage<T, E>;

    constexpr expected_base_non_trivial() noexcept(std::is_nothrow_default_constructible_v<storage_type>)
        : valid_(true)
    {
    }

    template <class... Args>
    constexpr expected_base_non_trivial(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<storage_type, std::in_place_t, Args...>)
        : storage_(std::in_place, std::forward<Args>(args)...)
        , valid_(true)
    {
    }

    template <class... Args>
    constexpr expected_base_non_trivial(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<storage_type, unexpect_t, Args...>)
        : storage_(unexpect, std::forward<Args>(args)...)
        , valid_(false)
    {
    }

    expected_base_non_trivial(const expected_base_non_trivial& other)
    {
        if (other.valid_)
        {
            construct(std::in_place, other.value());
        }
        else
        {
            construct(unexpect, other.error());
        }
    }

    expected_base_non_trivial(expected_base_non_trivial&& other)
    {
        if (other.valid_)
        {
            construct(std::in_place, std::move(other.value()));
        }
        else
        {
            construct(unexpect, std::move(other.error()));
        }
    }

    ~expected_base_non_trivial() noexcept(noexcept(std::declval<this_type>().destroy()))
    {
        destroy();
    }

    constexpr const T& value() const noexcept
    {
        return storage_.value();
    }

    constexpr T& value() noexcept
    {
        return storage_.value();
    }

    constexpr const E& error() const noexcept
    {
        return storage_.error();
    }

    constexpr E& error() noexcept
    {
        return storage_.error();
    }

    constexpr const T* value_ptr() const noexcept
    {
        return std::addressof(value());
    }

    constexpr T* value_ptr() noexcept
    {
        return std::addressof(value());
    }

    constexpr const E* error_ptr() const noexcept
    {
        return std::addressof(error());
    }

    constexpr E* error_ptr() noexcept
    {
        return std::addressof(error());
    }

    constexpr bool valid() const noexcept
    {
        return valid_;
    }

    template <class... Args>
    inline T& construct(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        valid_ = true;
        return *detail::construct_at(value_ptr(), std::forward<Args>(args)...);
    }

    template <class... Args>
    inline E& construct(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<E, Args...>)
    {
        valid_ = false;
        return *detail::construct_at(error_ptr(), std::forward<Args>(args)...);
    }

    inline void destroy() noexcept(std::is_nothrow_destructible_v<T>&& std::is_nothrow_destructible_v<E>)
    {
        if (valid_)
        {
            std::destroy_at(value_ptr());
        }
        else
        {
            std::destroy_at(error_ptr());
        }
    }

private:
    storage_type storage_;
    bool valid_;
};

template <class T, class E, bool IsDefaultConstructible, bool IsMoveConstructible>
class expected_base_non_trivial<T, E, IsDefaultConstructible, false, IsMoveConstructible>
{
    using this_type = expected_base_non_trivial<T, E, IsDefaultConstructible, false, IsMoveConstructible>;

protected:
    using storage_type = expected_storage<T, E>;

    constexpr expected_base_non_trivial() noexcept(std::is_nothrow_default_constructible_v<storage_type>)
        : valid_(true)
    {
    }

    template <class... Args>
    constexpr expected_base_non_trivial(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<storage_type, std::in_place_t, Args...>)
        : storage_(std::in_place, std::forward<Args>(args)...)
        , valid_(true)
    {
    }

    template <class... Args>
    constexpr expected_base_non_trivial(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<storage_type, unexpect_t, Args...>)
        : storage_(unexpect, std::forward<Args>(args)...)
        , valid_(false)
    {
    }

    expected_base_non_trivial(const expected_base_non_trivial& other) = delete;

    expected_base_non_trivial(expected_base_non_trivial&& other)
    {
        if (other.valid_)
        {
            construct(std::in_place, std::move(other.value()));
        }
        else
        {
            construct(unexpect, std::move(other.error()));
        }
    }

    ~expected_base_non_trivial() noexcept(noexcept(std::declval<this_type>().destroy()))
    {
        destroy();
    }

    constexpr const T& value() const noexcept
    {
        return storage_.value();
    }

    constexpr T& value() noexcept
    {
        return storage_.value();
    }

    constexpr const E& error() const noexcept
    {
        return storage_.error();
    }

    constexpr E& error() noexcept
    {
        return storage_.error();
    }

    constexpr const T* value_ptr() const noexcept
    {
        return std::addressof(value());
    }

    constexpr T* value_ptr() noexcept
    {
        return std::addressof(value());
    }

    constexpr const E* error_ptr() const noexcept
    {
        return std::addressof(error());
    }

    constexpr E* error_ptr() noexcept
    {
        return std::addressof(error());
    }

    constexpr bool valid() const noexcept
    {
        return valid_;
    }

    template <class... Args>
    inline T& construct(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        valid_ = true;
        return *detail::construct_at(value_ptr(), std::forward<Args>(args)...);
    }

    template <class... Args>
    inline E& construct(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<E, Args...>)
    {
        valid_ = false;
        return *detail::construct_at(error_ptr(), std::forward<Args>(args)...);
    }

    inline void destroy() noexcept(std::is_nothrow_destructible_v<T>&& std::is_nothrow_destructible_v<E>)
    {
        if (valid_)
        {
            std::destroy_at(value_ptr());
        }
        else
        {
            std::destroy_at(error_ptr());
        }
    }

private:
    storage_type storage_;
    bool valid_;
};

template <class T, class E, bool IsMoveConstructible>
class expected_base_non_trivial<T, E, false, false, IsMoveConstructible>
{
    using this_type = expected_base_non_trivial<T, E, false, false, IsMoveConstructible>;

protected:
    using storage_type = expected_storage<T, E>;

    constexpr expected_base_non_trivial() = delete;
    
    template <class... Args>
    constexpr expected_base_non_trivial(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<storage_type, std::in_place_t, Args...>)
        : storage_(std::in_place, std::forward<Args>(args)...)
        , valid_(true)
    {
    }

    template <class... Args>
    constexpr expected_base_non_trivial(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<storage_type, unexpect_t, Args...>)
        : storage_(unexpect, std::forward<Args>(args)...)
        , valid_(false)
    {
    }

    expected_base_non_trivial(const expected_base_non_trivial& other) = delete;

    expected_base_non_trivial(expected_base_non_trivial&& other)
    {
        if (other.valid_)
        {
            construct(std::in_place, std::move(other.value()));
        }
        else
        {
            construct(unexpect, std::move(other.error()));
        }
    }

    ~expected_base_non_trivial() noexcept(noexcept(std::declval<this_type>().destroy()))
    {
        destroy();
    }

    constexpr const T& value() const noexcept
    {
        return storage_.value();
    }

    constexpr T& value() noexcept
    {
        return storage_.value();
    }

    constexpr const E& error() const noexcept
    {
        return storage_.error();
    }

    constexpr E& error() noexcept
    {
        return storage_.error();
    }

    constexpr const T* value_ptr() const noexcept
    {
        return std::addressof(value());
    }

    constexpr T* value_ptr() noexcept
    {
        return std::addressof(value());
    }

    constexpr const E* error_ptr() const noexcept
    {
        return std::addressof(error());
    }

    constexpr E* error_ptr() noexcept
    {
        return std::addressof(error());
    }

    constexpr bool valid() const noexcept
    {
        return valid_;
    }

    template <class... Args>
    inline T& construct(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        valid_ = true;
        return *detail::construct_at(value_ptr(), std::forward<Args>(args)...);
    }

    template <class... Args>
    inline E& construct(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<E, Args...>)
    {
        valid_ = false;
        return *detail::construct_at(error_ptr(), std::forward<Args>(args)...);
    }

    inline void destroy() noexcept(std::is_nothrow_destructible_v<T>&& std::is_nothrow_destructible_v<E>)
    {
        if (valid_)
        {
            std::destroy_at(value_ptr());
        }
        else
        {
            std::destroy_at(error_ptr());
        }
    }

private:
    storage_type storage_;
    bool valid_;
};

template <class T, class E, bool IsDefaultConstructible, bool IsCopyConstructible>
class expected_base_non_trivial<T, E, IsDefaultConstructible, IsCopyConstructible, false>
{
    using this_type = expected_base_non_trivial<T, E, IsDefaultConstructible, IsCopyConstructible, false>;

protected:
    using storage_type = expected_storage<T, E>;

    constexpr expected_base_non_trivial() noexcept(std::is_nothrow_default_constructible_v<storage_type>)
        : valid_(true)
    {
    }

    template <class... Args>
    constexpr expected_base_non_trivial(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<storage_type, std::in_place_t, Args...>)
        : storage_(std::in_place, std::forward<Args>(args)...)
        , valid_(true)
    {
    }

    template <class... Args>
    constexpr expected_base_non_trivial(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<storage_type, unexpect_t, Args...>)
        : storage_(unexpect, std::forward<Args>(args)...)
        , valid_(false)
    {
    }

    expected_base_non_trivial(const expected_base_non_trivial& other)
    {
        if (other.valid_)
        {
            construct(std::in_place, other.value());
        }
        else
        {
            construct(unexpect, other.error());
        }
    }

    expected_base_non_trivial(expected_base_non_trivial&& other) = delete;

    ~expected_base_non_trivial() noexcept(noexcept(std::declval<this_type>().destroy()))
    {
        destroy();
    }

    constexpr const T& value() const noexcept
    {
        return storage_.value();
    }

    constexpr T& value() noexcept
    {
        return storage_.value();
    }

    constexpr const E& error() const noexcept
    {
        return storage_.error();
    }

    constexpr E& error() noexcept
    {
        return storage_.error();
    }

    constexpr const T* value_ptr() const noexcept
    {
        return std::addressof(value());
    }

    constexpr T* value_ptr() noexcept
    {
        return std::addressof(value());
    }

    constexpr const E* error_ptr() const noexcept
    {
        return std::addressof(error());
    }

    constexpr E* error_ptr() noexcept
    {
        return std::addressof(error());
    }

    constexpr bool valid() const noexcept
    {
        return valid_;
    }

    template <class... Args>
    inline T& construct(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        valid_ = true;
        return *detail::construct_at(value_ptr(), std::forward<Args>(args)...);
    }

    template <class... Args>
    inline E& construct(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<E, Args...>)
    {
        valid_ = false;
        return *detail::construct_at(error_ptr(), std::forward<Args>(args)...);
    }

    inline void destroy() noexcept(std::is_nothrow_destructible_v<T>&& std::is_nothrow_destructible_v<E>)
    {
        if (valid_)
        {
            std::destroy_at(value_ptr());
        }
        else
        {
            std::destroy_at(error_ptr());
        }
    }

private:
    storage_type storage_;
    bool valid_;
};

template <class T, class E, bool IsCopyConstructible>
class expected_base_non_trivial<T, E, false, IsCopyConstructible, false>
{
    using this_type = expected_base_non_trivial<T, E, false, IsCopyConstructible, false>;

protected:
    using storage_type = expected_storage<T, E>;

    constexpr expected_base_non_trivial() = delete;

    template <class... Args>
    constexpr expected_base_non_trivial(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<storage_type, std::in_place_t, Args...>)
        : storage_(std::in_place, std::forward<Args>(args)...)
        , valid_(true)
    {
    }

    template <class... Args>
    constexpr expected_base_non_trivial(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<storage_type, unexpect_t, Args...>)
        : storage_(unexpect, std::forward<Args>(args)...)
        , valid_(false)
    {
    }

    expected_base_non_trivial(const expected_base_non_trivial& other)
    {
        if (other.valid_)
        {
            construct(std::in_place, other.value());
        }
        else
        {
            construct(unexpect, other.error());
        }
    }

    expected_base_non_trivial(expected_base_non_trivial&& other) = delete;

    ~expected_base_non_trivial() noexcept(noexcept(std::declval<this_type>().destroy()))
    {
        destroy();
    }

    constexpr const T& value() const noexcept
    {
        return storage_.value();
    }

    constexpr T& value() noexcept
    {
        return storage_.value();
    }

    constexpr const E& error() const noexcept
    {
        return storage_.error();
    }

    constexpr E& error() noexcept
    {
        return storage_.error();
    }

    constexpr const T* value_ptr() const noexcept
    {
        return std::addressof(value());
    }

    constexpr T* value_ptr() noexcept
    {
        return std::addressof(value());
    }

    constexpr const E* error_ptr() const noexcept
    {
        return std::addressof(error());
    }

    constexpr E* error_ptr() noexcept
    {
        return std::addressof(error());
    }

    constexpr bool valid() const noexcept
    {
        return valid_;
    }

    template <class... Args>
    inline T& construct(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        valid_ = true;
        return *detail::construct_at(value_ptr(), std::forward<Args>(args)...);
    }

    template <class... Args>
    inline E& construct(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<E, Args...>)
    {
        valid_ = false;
        return *detail::construct_at(error_ptr(), std::forward<Args>(args)...);
    }

    inline void destroy() noexcept(std::is_nothrow_destructible_v<T>&& std::is_nothrow_destructible_v<E>)
    {
        if (valid_)
        {
            std::destroy_at(value_ptr());
        }
        else
        {
            std::destroy_at(error_ptr());
        }
    }

private:
    storage_type storage_;
    bool valid_;
};

template <class T, class E, bool IsDefaultConstructible>
class expected_base_non_trivial<T, E, IsDefaultConstructible, false, false>
{
    using this_type = expected_base_non_trivial<T, E, IsDefaultConstructible, false, false>;

protected:
    using storage_type = expected_storage<T, E>;

    constexpr expected_base_non_trivial() noexcept(std::is_nothrow_default_constructible_v<storage_type>)
        : valid_(true)
    {
    }

    template <class... Args>
    constexpr expected_base_non_trivial(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<storage_type, std::in_place_t, Args...>)
        : storage_(std::in_place, std::forward<Args>(args)...)
        , valid_(true)
    {
    }

    template <class... Args>
    constexpr expected_base_non_trivial(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<storage_type, unexpect_t, Args...>)
        : storage_(unexpect, std::forward<Args>(args)...)
        , valid_(false)
    {
    }

    expected_base_non_trivial(const expected_base_non_trivial& other) = delete;

    expected_base_non_trivial(expected_base_non_trivial&& other) = delete;

    ~expected_base_non_trivial() noexcept(noexcept(std::declval<this_type>().destroy()))
    {
        destroy();
    }

    constexpr const T& value() const noexcept
    {
        return storage_.value();
    }

    constexpr T& value() noexcept
    {
        return storage_.value();
    }

    constexpr const E& error() const noexcept
    {
        return storage_.error();
    }

    constexpr E& error() noexcept
    {
        return storage_.error();
    }

    constexpr const T* value_ptr() const noexcept
    {
        return std::addressof(value());
    }

    constexpr T* value_ptr() noexcept
    {
        return std::addressof(value());
    }

    constexpr const E* error_ptr() const noexcept
    {
        return std::addressof(error());
    }

    constexpr E* error_ptr() noexcept
    {
        return std::addressof(error());
    }

    constexpr bool valid() const noexcept
    {
        return valid_;
    }

    template <class... Args>
    inline T& construct(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        valid_ = true;
        return *detail::construct_at(value_ptr(), std::forward<Args>(args)...);
    }

    template <class... Args>
    inline E& construct(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<E, Args...>)
    {
        valid_ = false;
        return *detail::construct_at(error_ptr(), std::forward<Args>(args)...);
    }

    inline void destroy() noexcept(std::is_nothrow_destructible_v<T>&& std::is_nothrow_destructible_v<E>)
    {
        if (valid_)
        {
            std::destroy_at(value_ptr());
        }
        else
        {
            std::destroy_at(error_ptr());
        }
    }

private:
    storage_type storage_;
    bool valid_;
};

template <class T, class E>
class expected_base_non_trivial<T, E, false, false, false>
{
    using this_type = expected_base_non_trivial<T, E, false, false, false>;

protected:
    using storage_type = expected_storage<T, E>;

    constexpr expected_base_non_trivial() = delete;

    template <class... Args>
    constexpr expected_base_non_trivial(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<storage_type, std::in_place_t, Args...>)
        : storage_(std::in_place, std::forward<Args>(args)...)
        , valid_(true)
    {
    }

    template <class... Args>
    constexpr expected_base_non_trivial(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<storage_type, unexpect_t, Args...>)
        : storage_(unexpect, std::forward<Args>(args)...)
        , valid_(false)
    {
    }

    expected_base_non_trivial(const expected_base_non_trivial& other) = delete;

    expected_base_non_trivial(expected_base_non_trivial&& other) = delete;

    ~expected_base_non_trivial() noexcept(noexcept(std::declval<this_type>().destroy()))
    {
        destroy();
    }

    constexpr const T& value() const noexcept
    {
        return storage_.value();
    }

    constexpr T& value() noexcept
    {
        return storage_.value();
    }

    constexpr const E& error() const noexcept
    {
        return storage_.error();
    }

    constexpr E& error() noexcept
    {
        return storage_.error();
    }

    constexpr const T* value_ptr() const noexcept
    {
        return std::addressof(value());
    }

    constexpr T* value_ptr() noexcept
    {
        return std::addressof(value());
    }

    constexpr const E* error_ptr() const noexcept
    {
        return std::addressof(error());
    }

    constexpr E* error_ptr() noexcept
    {
        return std::addressof(error());
    }

    constexpr bool valid() const noexcept
    {
        return valid_;
    }

    template <class... Args>
    inline T& construct(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        valid_ = true;
        return *detail::construct_at(value_ptr(), std::forward<Args>(args)...);
    }

    template <class... Args>
    inline E& construct(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<E, Args...>)
    {
        valid_ = false;
        return *detail::construct_at(error_ptr(), std::forward<Args>(args)...);
    }

    inline void destroy() noexcept(std::is_nothrow_destructible_v<T>&& std::is_nothrow_destructible_v<E>)
    {
        if (valid_)
        {
            std::destroy_at(value_ptr());
        }
        else
        {
            std::destroy_at(error_ptr());
        }
    }

private:
    storage_type storage_;
    bool valid_;
};

template <class E, bool IsCopyConstructible, bool IsMoveConstructible>
class expected_base_non_trivial<void, E, true, IsCopyConstructible, IsMoveConstructible>
{
    using this_type = expected_base_non_trivial<void, E, true, IsCopyConstructible, IsMoveConstructible>;

protected:
    using storage_type = expected_storage<void, E>;

    constexpr expected_base_non_trivial() noexcept(std::is_nothrow_default_constructible_v<storage_type>)
        : valid_(true)
    {
    }

    template <class... Args>
    constexpr expected_base_non_trivial(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<storage_type, unexpect_t, Args...>)
        : storage_(unexpect, std::forward<Args>(args)...)
        , valid_(false)
    {
    }

    expected_base_non_trivial(const expected_base_non_trivial& other)
    {
        if (other.valid_)
        {
            valid_ = true;
        }
        else
        {
            construct(unexpect, other.error());
        }
    }

    expected_base_non_trivial(expected_base_non_trivial&& other)
    {
        if (other.valid_)
        {
            valid_ = true;
        }
        else
        {
            construct(unexpect, std::move(other.error()));
        }
    }

    ~expected_base_non_trivial() noexcept(noexcept(std::declval<this_type>().destroy()))
    {
        destroy();
    }

    constexpr const E& error() const noexcept
    {
        return storage_.error();
    }

    constexpr E& error() noexcept
    {
        return storage_.error();
    }

    constexpr const E* error_ptr() const noexcept
    {
        return std::addressof(error());
    }

    constexpr E* error_ptr() noexcept
    {
        return std::addressof(error());
    }

    constexpr bool valid() const noexcept
    {
        return valid_;
    }

    template <class... Args>
    inline void construct(std::in_place_t, Args&&... args) noexcept
    {
        unused(args...);
        valid_ = true;
    }

    template <class... Args>
    inline E& construct(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<E, Args...>)
    {
        valid_ = false;
        return *detail::construct_at(error_ptr(), std::forward<Args>(args)...);
    }

    inline void destroy() noexcept(std::is_nothrow_destructible_v<E>)
    {
        if (!valid_)
            std::destroy_at(error_ptr());
    }

private:
    storage_type storage_;
    bool valid_;
};

template <class T, class E, bool IsDefaultConstructible, bool IsCopyConstructible, bool IsMoveConstructible>
using expected_base = std::conditional_t<
    (std::is_void_v<T> || std::is_trivially_destructible_v<T>) && std::is_trivially_destructible_v<E>,
    expected_base_trivial<T, E, IsDefaultConstructible, IsCopyConstructible, IsMoveConstructible>,
    expected_base_non_trivial<T, E, IsDefaultConstructible, IsCopyConstructible, IsMoveConstructible>>;

}  // namespace detail

/**
 * @brief Representing an unexpected outcome.
 *
 * @tparam E Error type.
 */
template <class E>
class unexpected
{
    static_assert(!std::is_reference_v<E> && !std::is_void_v<E>, "Unexpected type can't be a reference or void");

public:
    unexpected() = delete;

    /**
     * @brief Constructs an unexpected object by moving the error into it.
     */
    constexpr explicit unexpected(E&& e) noexcept(std::is_nothrow_move_constructible_v<E>)
        : error_(std::move(e))
    {
    }

    /**
     * @brief Constructs an unexpected object by copying the error into it.
     */
    constexpr explicit unexpected(const E& e) noexcept(std::is_nothrow_copy_constructible_v<E>)
        : error_(e)
    {
    }

    /**
     * @brief Returns a reference to the underlaying error.
     *
     * @return Const l-value reference to the underlaying error.
     */
    constexpr const E& value() const& noexcept
    {
        return error_;
    }

    /**
     * @brief Returns a reference to the underlaying error.
     *
     * @return Mutable l-value reference to the underlaying error.
     */
    constexpr E& value() & noexcept
    {
        return error_;
    }

    /**
     * @brief Returns a reference to the underlaying error.
     *
     * @return Const r-value reference to the underlaying error.
     */
    constexpr const E&& value() const&& noexcept
    {
        return std::move(error_);
    }

    /**
     * @brief Returns a reference to the underlaying error.
     *
     * @return Mutable r-value reference to the underlaying error.
     */
    constexpr E&& value() && noexcept
    {
        return std::move(error_);
    }

private:
    E error_;
};

/**
 * @brief Equal operator for unexpected types.
 *
 * @tparam E Error type.
 *
 * @param [in] lhs Left-hand-side object.
 * @param [in] rhs Right-hand-side object.
 *
 * @return true if the two objects are equal, otherwise false.
 */
template <class E>
constexpr bool operator==(const unexpected<E>& lhs, const unexpected<E>& rhs) noexcept
{
    return lhs.value() == rhs.value();
}

/**
 * @brief Not equal operator for unexpected types.
 *
 * @tparam E Error type.
 *
 * @param [in] lhs Left-hand-side object.
 * @param [in] rhs Right-hand-side object.
 *
 * @return true if the two objects are not equal, otherwise false.
 */
template <class E>
constexpr bool operator!=(const unexpected<E>& lhs, const unexpected<E>& rhs) noexcept
{
    return lhs.value() != rhs.value();
}

/**
 * @brief Helper method to create an unexpected object.
 *
 * @tparam E Error type.
 *
 * @param [in] error Error that occured.
 *
 * @return An unexpected object wrapping the error.
 */
template <class E>
constexpr inline unexpected<std::decay_t<E>> make_unexpected(E&& error) noexcept(std::is_nothrow_constructible_v<unexpected<std::decay_t<E>>, E>)
{
    return unexpected<std::decay_t<E>>{ std::forward<E>(error) };
}

#if LUABRIDGE_HAS_EXCEPTIONS
template <class E>
class bad_expected_access;

/**
 * @brief Thrown as exceptions to report the situation where an attempt is made to access the value of expected object that contains an unexpected<E>.
 */
template <>
class bad_expected_access<void> : public std::exception
{
public:
    explicit bad_expected_access() noexcept
    {
    }
};

/**
 * @brief Thrown as exceptions to report the situation where an attempt is made to access the value of expected object that contains an unexpected<E>.
 *
 * @tparam E Error type.
 */
template <class E>
class bad_expected_access : public bad_expected_access<void>
{
public:
    /**
     * @brief Initialization constructor.
     *
     * @param [in] error Error contained at the time of the illegal access.
     */
    explicit bad_expected_access(E error) noexcept(std::is_nothrow_constructible_v<E, E&&>)
        : error_(std::move(error))
    {
    }

    /**
     * @brief Returns a reference to the underlaying error.
     *
     * @return Const l-value reference to the underlaying error.
     */
    const E& error() const& noexcept
    {
        return error_;
    }

    /**
     * @brief Returns a reference to the underlaying error.
     *
     * @return Mutable l-value reference to the underlaying error.
     */
    E& error() & noexcept
    {
        return error_;
    }

    /**
     * @brief Returns a reference to the underlaying error.
     *
     * @return Mutable r-value reference to the underlaying error.
     */
    E&& error() && noexcept
    {
        return std::move(error_);
    }

private:
    E error_;
};
#endif

/**
 * @brief Trait to check if a type T is of type expected.
 * @{
 */
template <class T>
struct is_expected : std::false_type
{
};

template <class T, class E>
struct is_expected<expected<T, E>> : std::true_type
{
};
/**
 * @}
 */

/**
 * @brief Trait to check if a type T is of type unexpected.
 * @{
 */
template <class T>
struct is_unexpected : std::false_type
{
};

template <class E>
struct is_unexpected<unexpected<E>> : std::true_type
{
};
/**
 * @}
 */

/**
 * @brief Utility class to represent an expected object.
 *
 * @tparam T Value type.
 * @tparam E Error type.
 */
template <class T, class E>
class expected : public detail::expected_base<T, E, std::is_default_constructible_v<T>, std::is_copy_constructible_v<T>, std::is_move_constructible_v<T>>
{
    static_assert(!std::is_reference_v<E> && !std::is_void_v<E>, "Unexpected type can't be a reference or void");

    using base_type = detail::expected_base<T, E, std::is_default_constructible_v<T>, std::is_copy_constructible_v<T>, std::is_move_constructible_v<T>>;
    using this_type = expected<T, E>;

public:
    /**
     * @brief Value type.
     */
    using value_type = T;

    /**
     * @brief Error type.
     */
    using error_type = E;

    /**
     * @brief Unexpected type.
     */
    using unexpected_type = unexpected<E>;

    template <class U>
    struct rebind
    {
        using type = expected<U, error_type>;
    };

    /**
     * @brief Default constructor.
     *
     * Default constructs the value type.
     */
    constexpr expected() noexcept(std::is_nothrow_default_constructible_v<base_type>) = default;

    /**
     * @brief Copy constructor.
     *
     * @param [in] other Expected object to copy from.
     */
    constexpr expected(const expected& other) noexcept(std::is_nothrow_copy_constructible_v<base_type>) = default;

    /**
     * @brief Move constructor.
     *
     * @param [in] other Expected object to move from.
     */
    constexpr expected(expected&& other) noexcept(std::is_nothrow_move_constructible_v<base_type>) = default;

    /**
     * @brief Copy-convert constructor.
     *
     * @tparam U Value type on the source.
     * @tparam G Error type on the source.
     *
     * @param [in] other Expected to copy from.
     */
    template <class U, class G>
    expected(const expected<U, G>& other)
    {
        if (other.has_value())
        {
            this->construct(std::in_place, other.value());
        }
        else
        {
            this->construct(unexpect, other.error());
        }
    }

    /**
     * @brief Move-convert constructor.
     *
     * @tparam U Value type on the source.
     * @tparam G Error type on the source.
     *
     * @param [in] other Expected to move from.
     */
    template <class U, class G>
    expected(expected<U, G>&& other)
    {
        if (other.has_value())
        {
            this->construct(std::in_place, std::move(other.value()));
        }
        else
        {
            this->construct(unexpect, std::move(other.error()));
        }
    }

    /**
     * @brief Value initialize constructor.
     *
     * Forwards value and initializes the expected object.
     *
     * @tparam U Type of value.
     *
     * @param [in] value Value to initialize with.
     */
    template <class U = T, std::enable_if_t<!std::is_void_v<T> && std::is_constructible_v<T, U&&> && !std::is_same_v<std::decay_t<U>, std::in_place_t> && !is_expected<std::decay_t<U>>::value && !is_unexpected<std::decay_t<U>>::value, int> = 0>
    constexpr expected(U&& value) noexcept(std::is_nothrow_constructible_v<base_type, std::in_place_t, U>)
        : base_type(std::in_place, std::forward<U>(value))
    {
    }

    /**
     * @brief In-place value construction constructor.
     *
     * @tparam Args Argument types.
     *
     * @param [in] args Arguments to initialize the expected with.
     */
    template <class... Args>
    constexpr explicit expected(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<base_type, std::in_place_t, Args...>)
        : base_type(std::in_place, std::forward<Args>(args)...)
    {
    }

    /**
     * @brief In-place value construction constructor.
     *
     * @tparam U Value type of the initializer list.
     * @tparam Args Argument types.
     *
     * @param [in] ilist Initializer list to initialize with.
     * @param [in] args Arguments to initialize the expected with.
     */
    template <class U, class... Args>
    constexpr explicit expected(std::in_place_t, std::initializer_list<U> ilist, Args&&... args) noexcept(std::is_nothrow_constructible_v<base_type, std::in_place_t, std::initializer_list<U>, Args...>)
        : base_type(std::in_place, ilist, std::forward<Args>(args)...)
    {
    }

    /**
     * @brief Error copy-convert constructor.
     *
     * @tparam G Error type on the source.
     *
     * @param [in] u Unexpected to copy from.
     */
    template <class G = E>
    constexpr expected(const unexpected<G>& u) noexcept(std::is_nothrow_constructible_v<base_type, unexpect_t, const G&>)
        : base_type(unexpect, u.value())
    {
    }

    /**
     * @brief Error move-convert constructor.
     *
     * @tparam G Error type on the source.
     *
     * @param [in] u Unexpected to move from.
     */
    template <class G = E>
    constexpr expected(unexpected<G>&& u) noexcept(std::is_nothrow_constructible_v<base_type, unexpect_t, G&&>)
        : base_type(unexpect, std::move(u.value()))
    {
    }

    /**
     * @brief In-place error construction constructor.
     *
     * @tparam Args Argument types.
     *
     * @param [in] args Arguments to initialize the error with.
     */
    template <class... Args>
    constexpr explicit expected(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<base_type, unexpect_t, Args...>)
        : base_type(unexpect, std::forward<Args>(args)...)
    {
    }

    /**
     * @brief In-place error construction constructor.
     *
     * @tparam U Value type of the initializer list.
     * @tparam Args Argument types.
     *
     * @param [in] ilist Initializer list to initialize with.
     * @param [in] args Arguments to initialize the error with.
     */
    template <class U, class... Args>
    constexpr explicit expected(unexpect_t, std::initializer_list<U> ilist, Args&&... args) noexcept(std::is_nothrow_constructible_v<base_type, unexpect_t, std::initializer_list<U>, Args...>)
        : base_type(unexpect, ilist, std::forward<Args>(args)...)
    {
    }

    /**
     * @brief Copy assignment operator.
     *
     * @param [in] other Expected to copy from.
     *
     * @return Reference to this.
     */
    expected& operator=(const expected& other)
    {
        if (other.has_value())
        {
            assign(std::in_place, other.value());
        }
        else
        {
            assign(unexpect, other.error());
        }

        return *this;
    }

    /**
     * @brief Move assignment operator.
     *
     * @param [in] other Expected to move from.
     *
     * @return Reference to this.
     */
    expected& operator=(expected&& other)
    {
        if (other.has_value())
        {
            assign(std::in_place, std::move(other.value()));
        }
        else
        {
            assign(unexpect, std::move(other.error()));
        }

        return *this;
    }

    /**
     * @brief Value assignment operator.
     *
     * @tparam U Value type.
     *
     * @param [in] value Value to be assigned.
     *
     * @return Reference to this.
     */
    template <class U = T, std::enable_if_t<!is_expected<std::decay_t<U>>::value && !is_unexpected<std::decay_t<U>>::value, int> = 0>
    expected& operator=(U&& value)
    {
        assign(std::in_place, std::forward<U>(value));
        return *this;
    }

    /**
     * @brief Unexpected copy assignment operator.
     *
     * @tparam G Error type.
     *
     * @param [in] u Unexpected to copy from.
     *
     * @return Reference to this.
     */
    template <class G = E>
    expected& operator=(const unexpected<G>& u)
    {
        assign(unexpect, u.value());
        return *this;
    }

    /**
     * @brief Unexpected move assignment operator.
     *
     * @tparam G Error type.
     *
     * @param [in] u Unexpected to move from.
     *
     * @return Reference to this.
     */
    template <class G = E>
    expected& operator=(unexpected<G>&& u)
    {
        assign(unexpect, std::move(u.value()));
        return *this;
    }

    /**
     * @brief Constructs the value in-place.
     *
     * @tparam Args Argument types.
     *
     * @param [in] args The arguments to pass to the constructor.
     *
     * @return Reference to the value.
     */
    template <class... Args>
    T& emplace(Args&&... args) noexcept(noexcept(std::declval<this_type>().assign(std::in_place, std::forward<Args>(args)...)))
    {
        return assign(std::in_place, std::forward<Args>(args)...);
    }

    /**
     * @brief Constructs the value in-place.
     *
     * @tparam U Initializer list type.
     * @tparam Args Argument types.
     *
     * @param [in] ilist The initializer list to pass to the constructor.
     * @param [in] args The arguments to pass to the constructor.
     *
     * @return Reference to the value.
     */
    template <class U, class... Args>
    T& emplace(std::initializer_list<U> ilist, Args&&... args) noexcept(noexcept(std::declval<this_type>().assign(std::in_place, ilist, std::forward<Args>(args)...)))
    {
        return assign(std::in_place, ilist, std::forward<Args>(args)...);
    }

    /**
     * @brief Swaps the contents with those of other.
     *
     * @param [in|out] other The expected object to exchange the contents with.
     */
    void swap(expected& other) noexcept(detail::is_nothrow_swappable<value_type>::value && detail::is_nothrow_swappable<error_type>::value)
    {
        using std::swap;

        if (has_value())
        {
            if (other.has_value())
            {
                swap(value(), other.value());
            }
            else
            {
                E error = std::move(other.error());
                other.assign(std::in_place, std::move(value()));
                assign(unexpect, std::move(error));
            }
        }
        else
        {
            if (other.has_value())
            {
                other.swap(*this);
            }
            else
            {
                swap(error(), other.error());
            }
        }
    }

    /**
     * @brief Accesses the contained value.
     *
     * @return Returns a pointer to the contained value.
     */
    constexpr const T* operator->() const
    {
        return base_type::value_ptr();
    }

    /**
     * @brief Accesses the contained value.
     *
     * @return Returns a pointer to the contained value.
     */
    constexpr T* operator->()
    {
        return base_type::value_ptr();
    }

    /**
     * @brief Accesses the contained value.
     *
     * @return Returns a reference to the contained value.
     */
    constexpr const T& operator*() const&
    {
        return value();
    }

    /**
     * @brief Accesses the contained value.
     *
     * @return Returns a reference to the contained value.
     */
    constexpr T& operator*() &
    {
        return value();
    }

    /**
     * @brief Accesses the contained value.
     *
     * @return Returns a reference to the contained value.
     */
    constexpr const T&& operator*() const&&
    {
        return std::move(value());
    }

    /**
     * @brief Accesses the contained value.
     *
     * @return Returns a reference to the contained value.
     */
    constexpr T&& operator*() &&
    {
        return std::move(value());
    }

    /**
     * @brief Checks whether this contains a value.
     *
     * @return true if this contains a value, otherwise false.
     */
    constexpr explicit operator bool() const noexcept
    {
        return has_value();
    }

    /**
     * @brief Checks whether this contains a value.
     *
     * @return true if this contains a value, otherwise false.
     */
    constexpr bool has_value() const noexcept
    {
        return base_type::valid();
    }

/**
     * @brief Accesses the contained value.
     *
     * @return Returns a reference to the contained value.
     */
    constexpr const T& value() const&
    {
#if LUABRIDGE_HAS_EXCEPTIONS
        if (!has_value())
            throw bad_expected_access<E>(error());
#endif

        return base_type::value();
    }

    /**
     * @brief Accesses the contained value.
     *
     * @return Returns a reference to the contained value.
     */
    constexpr T& value() &
    {
#if LUABRIDGE_HAS_EXCEPTIONS
        if (!has_value())
            throw bad_expected_access<E>(error());
#endif

        return base_type::value();
    }

/**
     * @brief Accesses the contained value.
     *
     * @return Returns a reference to the contained value.
     */
    constexpr const T&& value() const&& noexcept
    {
#if LUABRIDGE_HAS_EXCEPTIONS
        if (!has_value())
            throw bad_expected_access<E>(error());
#endif

        return std::move(base_type::value());
    }

    /**
     * @brief Accesses the contained value.
     *
     * @return Returns a reference to the contained value.
     */
    constexpr T&& value() &&
    {
#if LUABRIDGE_HAS_EXCEPTIONS
        if (!has_value())
            throw bad_expected_access<E>(error());
#endif
        return std::move(base_type::value());
    }

    /**
     * @brief Accesses the contained error.
     *
     * @return Returns a reference to the contained error.
     */
    constexpr const E& error() const& noexcept
    {
        return base_type::error();
    }

    /**
     * @brief Accesses the contained error.
     *
     * @return Returns a reference to the contained error.
     */
    constexpr E& error() & noexcept
    {
        return base_type::error();
    }

    /**
     * @brief Accesses the contained error.
     *
     * @return Returns a reference to the contained error.
     */
    constexpr const E&& error() const&& noexcept
    {
        return std::move(base_type::error());
    }

    /**
     * @brief Accesses the contained error.
     *
     * @return Returns a reference to the contained error.
     */
    constexpr E&& error() && noexcept
    {
        return std::move(base_type::error());
    }

    /**
     * @brief Returns the contained value if this has a value, otherwise returns default_value.
     *
     * @tparam U Type of default_value.
     *
     * @param [in|out] default_value The value to use in case this is empty.
     *
     * @return The current value if this has a value, or default_value otherwise.
     */
    template <class U>
    constexpr T value_or(U&& defaultValue) const&
    {
        return has_value() ? value() : static_cast<T>(std::forward<U>(defaultValue));
    }

    /**
     * @brief Returns the contained value if this has a value, otherwise returns default_value.
     *
     * @tparam U Type of default_value.
     *
     * @param [in|out] default_value The value to use in case this is empty.
     *
     * @return The current value if this has a value, or default_value otherwise.
     */
    template <class U>
    T value_or(U&& defaultValue) &&
    {
        return has_value() ? std::move(value()) : static_cast<T>(std::forward<U>(defaultValue));
    }

private:
    template <class Tag, class... Args>
    auto assign(Tag tag, Args&&... args) noexcept(noexcept(std::declval<this_type>().destroy()) && noexcept(std::declval<this_type>().construct(tag, std::forward<Args>(args)...)))
        -> decltype(std::declval<this_type>().construct(tag, std::forward<Args>(args)...))
    {
        this->destroy();

        return this->construct(tag, std::forward<Args>(args)...);
    }
};

/**
 * @brief Utility class to represent an expected object. Specialization for void type.
 *
 * @tparam E Error type.
 */
template <class E>
class expected<void, E> : public detail::expected_base<void, E, true, std::is_copy_constructible_v<E>, std::is_move_constructible_v<E>>
{
    static_assert(!std::is_reference_v<E> && !std::is_void_v<E>, "Unexpected type can't be a reference or void");

    using base_type = detail::expected_base<void, E, true, std::is_copy_constructible_v<E>, std::is_move_constructible_v<E>>;
    using this_type = expected<void, E>;

public:
    /**
     * @brief Value type.
     */
    using value_type = void;

    /**
     * @brief Error type.
     */
    using error_type = E;

    /**
     * @brief Unexpected type.
     */
    using unexpected_type = unexpected<E>;

    template <class U>
    struct rebind
    {
        using type = expected<U, error_type>;
    };

    /**
     * @brief Default constructor.
     */
    constexpr expected() = default;

    /**
     * @brief Copy assignment operator.
     *
     * @param [in] other Expected to copy from.
     *
     * @return Reference to this.
     */
    constexpr expected(const expected& other) = default;

    /**
     * @brief Move assignment operator.
     *
     * @param [in] other Expected to move from.
     *
     * @return Reference to this.
     */
    constexpr expected(expected&& other) = default;

    /**
     * @brief Copy-convert constructor.
     *
     * @tparam G Error type on the source.
     *
     * @param [in] other Expected to copy from.
     */
    template <class G>
    expected(const expected<void, G>& other)
    {
        if (other.has_value())
        {
            this->valid_ = true;
        }
        else
        {
            this->construct(unexpect, other.error());
        }
    }

    /**
     * @brief Move-convert constructor.
     *
     * @tparam G Error type on the source.
     *
     * @param [in] other Expected to move from.
     */
    template <class G>
    expected(expected<void, G>&& other)
    {
        if (other.has_value())
        {
            this->valid_ = true;
        }
        else
        {
            this->construct(unexpect, std::move(other.error()));
        }
    }

    /**
     * @brief Error copy-convert constructor.
     *
     * @tparam G Error type on the source.
     *
     * @param [in] u Unexpected to copy from.
     */
    template <class G = E>
    constexpr expected(const unexpected<G>& u)
        : base_type(unexpect, u.value())
    {
    }

    /**
     * @brief Error move-convert constructor.
     *
     * @tparam G Error type on the source.
     *
     * @param [in] u Unexpected to move from.
     */
    template <class G = E>
    constexpr expected(unexpected<G>&& u)
        : base_type(unexpect, std::move(u.value()))
    {
    }

    /**
     * @brief In-place error construction constructor.
     *
     * @tparam Args Argument types.
     *
     * @param [in] args Arguments to initialize the error with.
     */
    template <class... Args>
    constexpr explicit expected(unexpect_t, Args&&... args)
        : base_type(unexpect, std::forward<Args>(args)...)
    {
    }

    /**
     * @brief In-place error construction constructor.
     *
     * @tparam U Value type of the initializer list.
     * @tparam Args Argument types.
     *
     * @param [in] ilist Initializer list to initialize with.
     * @param [in] args Arguments to initialize the error with.
     */
    template <class U, class... Args>
    constexpr explicit expected(unexpect_t, std::initializer_list<U> ilist, Args&&... args)
        : base_type(unexpect, ilist, std::forward<Args>(args)...)
    {
    }

    /**
     * @brief Copy assignment operator.
     *
     * @param [in] other Expected to copy from.
     *
     * @return Reference to this.
     */
    expected& operator=(const expected& other)
    {
        if (other.has_value())
        {
            assign(std::in_place);
        }
        else
        {
            assign(unexpect, other.error());
        }

        return *this;
    }

    /**
     * @brief Move assignment operator.
     *
     * @param [in] other Expected to move from.
     *
     * @return Reference to this.
     */
    expected& operator=(expected&& other)
    {
        if (other.has_value())
        {
            assign(std::in_place);
        }
        else
        {
            assign(unexpect, std::move(other.error()));
        }

        return *this;
    }

    /**
     * @brief Unexpected copy assignment operator.
     *
     * @tparam G Error type.
     *
     * @param [in] u Unexpected to copy from.
     *
     * @return Reference to this.
     */
    template <class G = E>
    expected& operator=(const unexpected<G>& u)
    {
        assign(unexpect, u.value());
        return *this;
    }

    /**
     * @brief Unexpected move assignment operator.
     *
     * @tparam G Error type.
     *
     * @param [in] u Unexpected to move from.
     *
     * @return Reference to this.
     */
    template <class G = E>
    expected& operator=(unexpected<G>&& u)
    {
        assign(unexpect, std::move(u.value()));
        return *this;
    }

    /**
     * @brief Swaps the contents with those of other.
     *
     * @param [in|out] other The expected object to exchange the contents with.
     */
    void swap(expected& other) noexcept(detail::is_nothrow_swappable<error_type>::value)
    {
        using std::swap;

        if (has_value())
        {
            if (!other.has_value())
            {
                assign(unexpect, std::move(other.error()));
                other.assign(std::in_place);
            }
        }
        else
        {
            if (other.has_value())
            {
                other.swap(*this);
            }
            else
            {
                swap(error(), other.error());
            }
        }
    }

    /**
     * @brief Checks whether this contains a value.
     *
     * @return true if this contains a value, otherwise false.
     */
    constexpr explicit operator bool() const noexcept
    {
        return has_value();
    }

    /**
     * @brief Checks whether this contains a value.
     *
     * @return true if this contains a value, otherwise false.
     */
    constexpr bool has_value() const noexcept
    {
        return base_type::valid();
    }

    /**
     * @brief Accesses the contained error.
     *
     * @return Returns a reference to the contained error.
     */
    constexpr const E& error() const& noexcept
    {
        return base_type::error();
    }

    /**
     * @brief Accesses the contained error.
     *
     * @return Returns a reference to the contained error.
     */
    constexpr E& error() & noexcept
    {
        return base_type::error();
    }

    /**
     * @brief Accesses the contained error.
     *
     * @return Returns a reference to the contained error.
     */
    constexpr const E&& error() const&& noexcept
    {
        return std::move(base_type::error());
    }

    /**
     * @brief Accesses the contained error.
     *
     * @return Returns a reference to the contained error.
     */
    constexpr E&& error() && noexcept
    {
        return std::move(base_type::error());
    }

private:
    template <class Tag, class... Args>
    void assign(Tag tag, Args&&... args) noexcept(noexcept(std::declval<this_type>().destroy()) && noexcept(std::declval<this_type>().construct(tag, std::forward<Args>(args)...)))
    {
        this->destroy();
        this->construct(tag, std::forward<Args>(args)...);
    }
};

template <class T, class E>
constexpr bool operator==(const expected<T, E>& lhs, const expected<T, E>& rhs)
{
    return (lhs && rhs) ? *lhs == *rhs : ((!lhs && !rhs) ? lhs.error() == rhs.error() : false);
}

template <class E>
constexpr bool operator==(const expected<void, E>& lhs, const expected<void, E>& rhs)
{
    return (lhs && rhs) ? true : ((!lhs && !rhs) ? lhs.error() == rhs.error() : false);
}

template <class T, class E>
constexpr bool operator!=(const expected<T, E>& lhs, const expected<T, E>& rhs)
{
    return !(lhs == rhs);
}

template <class T, class E>
constexpr bool operator==(const expected<T, E>& lhs, const T& rhs)
{
    return lhs ? *lhs == rhs : false;
}

template <class T, class E>
constexpr bool operator==(const T& lhs, const expected<T, E>& rhs)
{
    return rhs == lhs;
}

template <class T, class E>
constexpr bool operator!=(const expected<T, E>& lhs, const T& rhs)
{
    return !(lhs == rhs);
}

template <class T, class E>
constexpr bool operator!=(const T& lhs, const expected<T, E>& rhs)
{
    return rhs != lhs;
}

template <class T, class E>
constexpr bool operator==(const expected<T, E>& lhs, const unexpected<E>& rhs)
{
    return lhs ? false : lhs.error() == rhs.value();
}

template <class T, class E>
constexpr bool operator==(const unexpected<E>& lhs, const expected<T, E>& rhs)
{
    return rhs == lhs;
}

template <class T, class E>
constexpr bool operator!=(const expected<T, E>& lhs, const unexpected<E>& rhs)
{
    return !(lhs == rhs);
}

template <class T, class E>
constexpr bool operator!=(const unexpected<E>& lhs, const expected<T, E>& rhs)
{
    return rhs != lhs;
}

/**
 * @brief Move constructs an any from a expected.
 *
 * @param [in] e Expected to move from.
 *
 * @return A new any instance constructed from the expected.
 */
template <class T, class E>
inline std::enable_if_t<!std::is_void_v<T>, std::any> to_any(expected<T, E>&& e)
{
    if (!e)
        return {};

    return std::move(e).value();
}

/**
 * @brief Copy constructs an any from a expected.
 *
 * @param [in] e Expected to copy from.
 *
 * @return A new any instance constructed from the expected.
 */
template <class T, class E>
inline std::enable_if_t<!std::is_void_v<T>, std::any> to_any(const expected<T, E>& e)
{
    if (!e)
        return {};

    return e.value();
}

/**
 * @brief Tries to move constructs an any from a expected if it contains a value, otherwise uses a default value.
 *
 * @param [in] e Expected to move from.
 * @param [in] default_value Default value to use if e doesn't contain a value.
 *
 * @return A new any instance constructed from the expected.
 */
template <class T, class E, class U>
inline std::enable_if_t<!std::is_void_v<T>, std::any> to_any(expected<T, E>&& e, U&& defaultValue)
{
    return std::move(e).value_or(std::forward<U>(defaultValue));
}

/**
 * @brief Tries to copy constructs an any from a expected if it contains a value, otherwise uses a default value.
 *
 * @param [in] e Expected to copy from.
 * @param [in] default_value Default value to use if e doesn't contain a value.
 *
 * @return A new any instance constructed from the expected.
 */
template <class T, class E, class U>
inline std::enable_if_t<!std::is_void_v<T>, std::any> to_any(const expected<T, E>& e, U&& defaultValue)
{
    return e.value_or(std::forward<U>(defaultValue));
}

} // namespace luabridge
