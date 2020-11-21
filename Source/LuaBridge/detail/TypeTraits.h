// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// Copyright 2019, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "Config.h"

#include <string>

namespace luabridge {

//------------------------------------------------------------------------------
/**
    Container traits.

    Unspecialized ContainerTraits has the isNotContainer typedef for SFINAE.
    All user defined containers must supply an appropriate specialization for
    ContinerTraits (without the typedef isNotContainer). The containers that
    come with LuaBridge also come with the appropriate ContainerTraits
    specialization. See the corresponding declaration for details.

    A specialization of ContainerTraits for some generic type ContainerType
    looks like this:

        template <class T>
        struct ContainerTraits <ContainerType <T>>
        {
          typedef typename T Type;

          static T* get (ContainerType <T> const& c)
          {
            return c.get (); // Implementation-dependent on ContainerType
          }
        };
*/
template<class T>
struct ContainerTraits
{
    typedef bool isNotContainer;

    typedef T Type;
};

namespace detail {

//------------------------------------------------------------------------------
/**
    Type traits.

    Specializations return information about a type.
*/
struct TypeTraits
{
    /** Determine if type T is a container.

        To be considered a container, there must be a specialization of
        ContainerTraits with the required fields.
    */
    template<typename T>
    class isContainer
    {
    private:
        typedef char yes[1]; // sizeof (yes) == 1
        typedef char no[2]; // sizeof (no)  == 2

        template <typename C>
        static constexpr no& test(typename C::isNotContainer*);

        template <typename>
        static constexpr yes& test(...);

    public:
        static constexpr bool value = sizeof(test<ContainerTraits<T>>(0)) == sizeof(yes);
    };
};

} // namespace detail

} // namespace luabridge
