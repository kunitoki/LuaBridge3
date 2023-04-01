// https://github.com/vinniefalco/LuaBridge
// Copyright 2021, Stefan Frings
// Copyright 2019, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT

#pragma once

#include <cassert>
#include <unordered_map>
#include <utility>

namespace luabridge {

//==============================================================================
/**
  A reference counted smart pointer.

  The api is compatible with boost::RefCountedPtr and std::RefCountedPtr, in the
  sense that it implements a strict subset of the functionality.

  This implementation wraps a std::shared_ptr.

  @tparam T The class type.
*/
template<class T>
class RefCountedPtr // : private detail::RefCountedPtrBase
{
public:
    template<typename Other>
    struct rebind
    {
        typedef RefCountedPtr<Other> other;
    };

    /** Construct as nullptr or from existing pointer to T.

        @param p The optional, existing pointer to assign from.
    */
    RefCountedPtr(T* const p = nullptr) : m_p(p)
    {
    }

    RefCountedPtr<T>& operator=(T* const p)
    {
        if (p != m_p.get())
        {
            RefCountedPtr<T> tmp(p);
            std::swap(m_p, tmp.m_p);
        }

        return *this;
    }

    /** Retrieve the raw pointer.

        @returns A pointer to the object.
    */
    T* get() const { return m_p.get(); }

    /** Retrieve the raw pointer by conversion.

        @returns A pointer to the object.
    */
    operator T*() const { return m_p.get(); }

    /** Retrieve the raw pointer.

        @returns A pointer to the object.
    */
    T* operator*() const { return m_p.get(); }

    /** Retrieve the raw pointer.

        @returns A pointer to the object.
    */
    T* operator->() const { return m_p.get(); }

    /** Determine the number of references.

        @note This is not thread-safe.

        @returns The number of active references.
    */
    long use_count() const
    {
       return m_p.use_count();
    }
   
private:
    std::shared_ptr<T> m_p;
};

//==============================================================================

// forward declaration
template<class T>
struct ContainerTraits;

template<class T>
struct ContainerTraits<RefCountedPtr<T>>
{
    typedef T Type;

    static T* get(RefCountedPtr<T> const& c) { return c.get(); }
};

} // namespace luabridge
