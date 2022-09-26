// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// Copyright 2019, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "Config.h"

#include <memory>

namespace luabridge {

//=================================================================================================
/**
 * @brief Container traits.
 *
 * Unspecialized ContainerTraits has the isNotContainer typedef for SFINAE. All user defined containers must supply an appropriate
 * specialization for ContinerTraits (without the alias isNotContainer). The containers that come with LuaBridge also come with the
 * appropriate ContainerTraits specialization.
 *
 * @note See the corresponding declaration for details.
 *
 * A specialization of ContainerTraits for some generic type ContainerType looks like this:
 *
 * @code
 *
 *  template <class T>
 *  struct ContainerTraits<ContainerType<T>>
 *  {
 *    using Type = T;
 *
 *    static ContainerType<T> construct(T* c)
 *    {
 *      return c; // Implementation-dependent on ContainerType
 *    }
 *
 *    static T* get(const ContainerType<T>& c)
 *    {
 *      return c.get(); // Implementation-dependent on ContainerType
 *    }
 *  };
 *
 * @endcode
 */
template <class T>
struct ContainerTraits
{
    using IsNotContainer = bool;

    using Type = T;
};

/**
 * @brief Register shared_ptr support as container.
 *
 * @tparam T Class that is hold by the shared_ptr, must inherit from std::enable_shared_from_this.
 */
template <class T>
struct ContainerTraits<std::shared_ptr<T>>
{
    static_assert(std::is_base_of_v<std::enable_shared_from_this<T>, T>);
    
    using Type = T;

    static std::shared_ptr<T> construct(T* t)
    {
        return t->shared_from_this();
    }

    static T* get(const std::shared_ptr<T>& c)
    {
        return c.get();
    }
};

namespace detail {

//=================================================================================================
/**
 * @brief Determine if type T is a container.
 *
 * To be considered a container, there must be a specialization of ContainerTraits with the required fields.
 */
template <class T>
class IsContainer
{
private:
    typedef char yes[1]; // sizeof (yes) == 1
    typedef char no[2]; // sizeof (no)  == 2

    template <class C>
    static constexpr no& test(typename C::IsNotContainer*);

    template <class>
    static constexpr yes& test(...);

public:
    static constexpr bool value = sizeof(test<ContainerTraits<T>>(0)) == sizeof(yes);
};

} // namespace detail
} // namespace luabridge
