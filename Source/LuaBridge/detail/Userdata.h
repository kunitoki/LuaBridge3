// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// Copyright 2019, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "Config.h"
#include "Errors.h"
#include "LuaException.h"
#include "ClassInfo.h"
#include "TypeTraits.h"

#include <cassert>
#include <stdexcept>
#include <type_traits>

namespace luabridge {
namespace detail {

//=================================================================================================
/**
 * @brief Return the identity pointer for our lightuserdata tokens.
 *
 * Because of Lua's dynamic typing and our improvised system of imposing C++ class structure, there is the possibility that executing
 * scripts may knowingly or unknowingly cause invalid data to get passed to the C functions created by LuaBridge.
 *
 * In particular, our security model addresses the following:
 *
 *   1. Scripts cannot create a userdata (ignoring the debug lib).
 *
 *   2. Scripts cannot create a lightuserdata (ignoring the debug lib).
 *
 *   3. Scripts cannot set the metatable on a userdata.
 */

/**
 * @brief Interface to a class pointer retrievable from a userdata.
 */
class Userdata
{
private:
    //=============================================================================================
    /**
     * @brief Validate and retrieve a Userdata on the stack.
     *
     * The Userdata must exactly match the corresponding class table or const table, or else a Lua error is raised. This is used for the
     * __gc metamethod.
     */
    static Userdata* getExactClass(lua_State* L, int index, const void* classKey)
    {
        return (void)classKey, static_cast<Userdata*>(lua_touserdata(L, lua_absindex(L, index)));
    }

    //=============================================================================================
    /**
     * @brief Validate and retrieve a Userdata on the stack.
     *
     * The Userdata must be derived from or the same as the given base class, identified by the key. If canBeConst is false, generates
     * an error if the resulting Userdata represents to a const object. We do the type check first so that the error message is informative.
     */
    static Userdata* getClass(lua_State* L,
                              int index,
                              const void* registryConstKey,
                              const void* registryClassKey,
                              bool canBeConst)
    {
        index = lua_absindex(L, index);

        lua_getmetatable(L, index); // Stack: object metatable (ot) | nil
        if (!lua_istable(L, -1))
        {
            lua_rawgetp(L, LUA_REGISTRYINDEX, registryClassKey); // Stack: registry metatable (rt) | nil
            return throwBadArg(L, index);
        }

        lua_rawgetp(L, -1, getConstKey()); // Stack: ot | nil, const table (co) | nil
        assert(lua_istable(L, -1) || lua_isnil(L, -1));

        // If const table is NOT present, object is const. Use non-const registry table
        // if object cannot be const, so constness validation is done automatically.
        // E.g. nonConstFn (constObj)
        // -> canBeConst = false, isConst = true
        // -> 'Class' registry table, 'const Class' object table
        // -> 'expected Class, got const Class'
        bool isConst = lua_isnil(L, -1); // Stack: ot | nil, nil, rt
        if (isConst && canBeConst)
        {
            lua_rawgetp(L, LUA_REGISTRYINDEX, registryConstKey); // Stack: ot, nil, rt
        }
        else
        {
            lua_rawgetp(L, LUA_REGISTRYINDEX, registryClassKey); // Stack: ot, co, rt
        }

        lua_insert(L, -3); // Stack: rt, ot, co | nil
        lua_pop(L, 1); // Stack: rt, ot

        for (;;)
        {
            if (lua_rawequal(L, -1, -2)) // Stack: rt, ot
            {
                lua_pop(L, 2); // Stack: -
                return static_cast<Userdata*>(lua_touserdata(L, index));
            }

            // Replace current metatable with it's base class.
            lua_rawgetp(L, -1, getParentKey()); // Stack: rt, ot, parent ot (pot) | nil

            if (lua_isnil(L, -1)) // Stack: rt, ot, nil
            {
                // Drop the object metatable because it may be some parent metatable
                lua_pop(L, 2); // Stack: rt
                return throwBadArg(L, index);
            }

            lua_remove(L, -2); // Stack: rt, pot
        }

        // no return
    }

    static bool isInstance(lua_State* L, int index, const void* registryClassKey)
    {
        index = lua_absindex(L, index);

        int result = lua_getmetatable(L, index); // Stack: object metatable (ot) | nothing
        if (result == 0)
            return false; // Nothing was pushed on the stack

        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1); // Stack: -
            return false;
        }

        lua_rawgetp(L, LUA_REGISTRYINDEX, registryClassKey); // Stack: ot, rt
        lua_insert(L, -2); // Stack: rt, ot

        for (;;)
        {
            if (lua_rawequal(L, -1, -2)) // Stack: rt, ot
            {
                lua_pop(L, 2); // Stack: -
                return true;
            }

            // Replace current metatable with it's base class.
            lua_rawgetp(L, -1, getParentKey()); // Stack: rt, ot, parent ot (pot) | nil

            if (lua_isnil(L, -1)) // Stack: rt, ot, nil
            {
                lua_pop(L, 3); // Stack: -
                return false;
            }

            lua_remove(L, -2); // Stack: rt, pot
        }
    }

    static Userdata* throwBadArg(lua_State* L, int index)
    {
        assert(lua_istable(L, -1) || lua_isnil(L, -1)); // Stack: rt | nil

        const char* expected = 0;
        if (lua_isnil(L, -1)) // Stack: nil
        {
            expected = "unregistered class";
        }
        else
        {
            lua_rawgetp(L, -1, getTypeKey()); // Stack: rt, registry type
            expected = lua_tostring(L, -1);
        }

        const char* got = 0;
        if (lua_isuserdata(L, index))
        {
            lua_getmetatable(L, index); // Stack: ..., ot | nil
            if (lua_istable(L, -1)) // Stack: ..., ot
            {
                lua_rawgetp(L, -1, getTypeKey()); // Stack: ..., ot, object type | nil
                if (lua_isstring(L, -1))
                {
                    got = lua_tostring(L, -1);
                }
            }
        }

        if (!got)
        {
            got = lua_typename(L, lua_type(L, index));
        }

        luaL_argerror(L, index, lua_pushfstring(L, "%s expected, got %s", expected, got));
        return 0;
    }

public:
    virtual ~Userdata() {}

    //=============================================================================================
    /**
     * @brief Returns the Userdata* if the class on the Lua stack matches.
     *
     * If the class does not match, a Lua error is raised.
     *
     * @tparam T  A registered user class.
     *
     * @param L A Lua state.
     * @param index The index of an item on the Lua stack.
     *
     * @return A userdata pointer if the class matches.
     */
    template <class T>
    static Userdata* getExact(lua_State* L, int index)
    {
        return getExactClass(L, index, detail::getClassRegistryKey<T>());
    }

    //=============================================================================================
    /**
     * @brief Get a pointer to the class from the Lua stack.
     *
     * If the object is not the class or a subclass, or it violates the const-ness, a Lua error is raised.
     *
     * @tparam T A registered user class.
     *
     * @param L A Lua state.
     * @param index The index of an item on the Lua stack.
     * @param canBeConst TBD
     *
     * @return A pointer if the class and constness match.
     */
    template <class T>
    static T* get(lua_State* L, int index, bool canBeConst)
    {
        if (lua_isnil(L, index))
            return nullptr;

        return static_cast<T*>(getClass(L,
                                        index,
                                        detail::getConstRegistryKey<T>(),
                                        detail::getClassRegistryKey<T>(),
                                        canBeConst)
                                   ->getPointer());
    }

    template <class T>
    static bool isInstance(lua_State* L, int index)
    {
        return isInstance(L, index, detail::getClassRegistryKey<T>());
    }

protected:
    Userdata() = default;

    /**
     * @brief Get an untyped pointer to the contained class.
     */
    void* getPointer() const noexcept
    {
        return m_p;
    }

    void* m_p = nullptr; // subclasses must set this
};

//=================================================================================================
/**
 * @brief Wraps a class object stored in a Lua userdata.
 *
 * The lifetime of the object is managed by Lua. The object is constructed inside the userdata using placement new.
 */
template <class T>
class UserdataValue : public Userdata
{
public:
    /**
     * @brief Push a T via placement new.
     *
     * The caller is responsible for calling placement new using the returned uninitialized storage.
     *
     * @param L A Lua state.
     *
     * @return An object referring to the newly created userdata value.
     */
    static UserdataValue<T>* place(lua_State* L, std::error_code& ec)
    {
        auto* ud = new (lua_newuserdata(L, sizeof(UserdataValue<T>))) UserdataValue<T>();

        lua_rawgetp(L, LUA_REGISTRYINDEX, detail::getClassRegistryKey<T>());

        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1); // possibly: a nil

            ud->~UserdataValue<T>();

            ec = throw_or_error_code<LuaException>(L, ErrorCode::ClassNotRegistered);

            return nullptr;
        }

        lua_setmetatable(L, -2);

        return ud;
    }

    /**
     * @brief Push T via copy construction from U.
     *
     * @tparam U A container type.
     *
     * @param L A Lua state.
     * @param u A container object reference.
     * @param ec Error code that will be set in case of failure to push on the lua stack.
     */
    template <class U>
    static bool push(lua_State* L, const U& u, std::error_code& ec)
    {
        auto* ud = place(L, ec);

        if (!ud)
            return false;

        new (ud->getObject()) U(u);

        ud->commit();

        return true;
    }

    /**
     * @brief Confirm object construction.
     */
    void commit() noexcept
    {
        m_p = getObject();
    }

    T* getObject() noexcept
    {
        // If this fails to compile it means you forgot to provide
        // a Container specialization for your container!
        return reinterpret_cast<T*>(&m_storage);
    }

private:
    /**
     * @brief Used for placement construction.
     */
    UserdataValue() noexcept
    {
        m_p = nullptr;
    }

    ~UserdataValue()
    {
        if (getPointer() != nullptr)
        {
            getObject()->~T();
        }
    }

    UserdataValue<T>(const UserdataValue<T>&);
    UserdataValue<T> operator=(const UserdataValue<T>&);

    std::aligned_storage_t<sizeof(T), alignof(T)> m_storage;
};

//=================================================================================================
/**
 * @brief Wraps a pointer to a class object inside a Lua userdata.
 *
 * The lifetime of the object is managed by C++.
 */
class UserdataPtr : public Userdata
{
public:
    /**
     * @brief Push non-const pointer to object.
     *
     * @tparam T A user registered class.
     *
     * @param L A Lua state.
     * @param p A pointer to the user class instance.
     * @param ec Error code that will be set in case of failure to push on the lua stack.
     */
    template <class T>
    static bool push(lua_State* L, T* p, std::error_code& ec)
    {
        if (p)
            return push(L, p, getClassRegistryKey<T>(), ec);

        lua_pushnil(L);
        return true;
    }

    /**
     * @brief Push const pointer to object.
     *
     * @tparam T A user registered class.
     *
     * @param L A Lua state.
     * @param p A pointer to the user class instance.
     * @param ec Error code that will be set in case of failure to push on the lua stack.
     */
    template <class T>
    static bool push(lua_State* L, const T* p, std::error_code& ec)
    {
        if (p)
            return push(L, p, getConstRegistryKey<T>(), ec);

        lua_pushnil(L);
        return true;
    }

private:
    /**
     * @brief Push a pointer to object using metatable key.
     */
    static bool push(lua_State* L, const void* p, const void* key, std::error_code& ec)
    {
        auto* ptr = new (lua_newuserdata(L, sizeof(UserdataPtr))) UserdataPtr(const_cast<void*>(p));

        lua_rawgetp(L, LUA_REGISTRYINDEX, key);

        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1); // possibly: a nil

            ptr->~UserdataPtr();

            ec = throw_or_error_code<LuaException>(L, ErrorCode::ClassNotRegistered);

            return false;
        }

        lua_setmetatable(L, -2);

        return true;
    }

    explicit UserdataPtr(void* p)
    {
        // Can't construct with a null pointer!
        assert(p != nullptr);

        m_p = p;
    }

    UserdataPtr(const UserdataPtr&);
    UserdataPtr operator=(const UserdataPtr&);
};

//============================================================================
/**
 * @brief Wraps a container that references a class object.
 *
 * The template argument C is the container type, ContainerTraits must be specialized on C or else a compile error will result.
 */
template <class C>
class UserdataShared : public Userdata
{
public:
    ~UserdataShared() = default;

    /**
     * @brief Construct from a container to the class or a derived class.
     *
     * @tparam U A container type.
     *
     * @param  u A container object reference.
     */
    template <class U>
    explicit UserdataShared(const U& u) : m_c(u)
    {
        m_p = const_cast<void*>(reinterpret_cast<const void*>((ContainerTraits<C>::get(m_c))));
    }

    /**
     * @brief Construct from a pointer to the class or a derived class.
     *
     * @tparam U A container type.
     *
     * @param u A container object pointer.
     */
    template <class U>
    explicit UserdataShared(U* u) : m_c(u)
    {
        m_p = const_cast<void*>(reinterpret_cast<const void*>((ContainerTraits<C>::get(m_c))));
    }

private:
    UserdataShared(const UserdataShared<C>&);
    UserdataShared<C>& operator=(const UserdataShared<C>&);

    C m_c;
};

//=================================================================================================
/**
 * @brief SFINAE helper for non-const objects.
 */
template <class C, bool MakeObjectConst>
struct UserdataSharedHelper
{
    using T = std::remove_const_t<typename ContainerTraits<C>::Type>;

    static bool push(lua_State* L, const C& c, std::error_code& ec)
    {
        if (ContainerTraits<C>::get(c) != nullptr)
        {
            auto* us = new (lua_newuserdata(L, sizeof(UserdataShared<C>))) UserdataShared<C>(c);

            lua_rawgetp(L, LUA_REGISTRYINDEX, getClassRegistryKey<T>());

            if (!lua_istable(L, -1))
            {
                lua_pop(L, 1); // possibly: a nil

                us->~UserdataShared<C>();

                ec = throw_or_error_code<LuaException>(L, ErrorCode::ClassNotRegistered);

                return false;
            }

            lua_setmetatable(L, -2);
        }
        else
        {
            lua_pushnil(L);
        }

        return true;
    }

    static bool push(lua_State* L, T* t, std::error_code& ec)
    {
        if (t)
        {
            auto* us = new (lua_newuserdata(L, sizeof(UserdataShared<C>))) UserdataShared<C>(t);

            lua_rawgetp(L, LUA_REGISTRYINDEX, getClassRegistryKey<T>());

            if (!lua_istable(L, -1))
            {
                lua_pop(L, 1); // possibly: a nil

                us->~UserdataShared<C>();

                ec = throw_or_error_code<LuaException>(L, ErrorCode::ClassNotRegistered);

                return false;
            }

            lua_setmetatable(L, -2);
        }
        else
        {
            lua_pushnil(L);
        }

        return true;
    }
};

/**
 * @brief SFINAE helper for const objects.
 */
template <class C>
struct UserdataSharedHelper<C, true>
{
    using T = std::remove_const_t<typename ContainerTraits<C>::Type>;

    static bool push(lua_State* L, const C& c, std::error_code& ec)
    {
        if (ContainerTraits<C>::get(c) != nullptr)
        {
            auto* us = new (lua_newuserdata(L, sizeof(UserdataShared<C>))) UserdataShared<C>(c);

            lua_rawgetp(L, LUA_REGISTRYINDEX, getConstRegistryKey<T>());

            if (!lua_istable(L, -1))
            {
                lua_pop(L, 1); // possibly: a nil

                us->~UserdataShared<C>();

                ec = throw_or_error_code<LuaException>(L, ErrorCode::ClassNotRegistered);

                return false;
            }

            lua_setmetatable(L, -2);
        }
        else
        {
            lua_pushnil(L);
        }

        return true;
    }

    static bool push(lua_State* L, T* t, std::error_code& ec)
    {
        if (t)
        {
            auto* us = new (lua_newuserdata(L, sizeof(UserdataShared<C>))) UserdataShared<C>(t);

            lua_rawgetp(L, LUA_REGISTRYINDEX, getConstRegistryKey<T>());

            if (!lua_istable(L, -1))
            {
                lua_pop(L, 1); // possibly: a nil

                us->~UserdataShared<C>();

                ec = throw_or_error_code<LuaException>(L, ErrorCode::ClassNotRegistered);

                return false;
            }

            lua_setmetatable(L, -2);
        }
        else
        {
            lua_pushnil(L);
        }

        return true;
    }
};

//=================================================================================================
/**
 * @brief Pass by container.
 *
 * The container controls the object lifetime. Typically this will be a lifetime shared by C++ and Lua using a reference count. Because of type
 * erasure, containers like std::shared_ptr will not work, unless the type hold by them is derived from std::enable_shared_from_this.
 */
template <class T, bool ByContainer>
struct StackHelper
{
    using ReturnType = std::remove_const_t<typename ContainerTraits<T>::Type>;

    static bool push(lua_State* L, const T& t, std::error_code& ec)
    {
        return UserdataSharedHelper<T, std::is_const_v<typename ContainerTraits<T>::Type>>::push(L, t, ec);
    }

    static T get(lua_State* L, int index)
    {
        return ContainerTraits<T>::construct(Userdata::get<ReturnType>(L, index, true));
    }
};

/**
 * @brief Pass by value.
 *
 * Lifetime is managed by Lua. A C++ function which accesses a pointer or reference to an object outside the activation record in which it was
 * retrieved may result in undefined behavior if Lua garbage collected it.
 */
template <class T>
struct StackHelper<T, false>
{
    static bool push(lua_State* L, const T& t, std::error_code& ec)
    {
        return UserdataValue<T>::push(L, t, ec);
    }

    static T const& get(lua_State* L, int index)
    {
        return *Userdata::get<T>(L, index, true);
    }
};

//=================================================================================================
/**
 * @brief Lua stack conversions for pointers and references to class objects.
 *
 * Lifetime is managed by C++. Lua code which remembers a reference to the value may result in undefined behavior if C++ destroys the object.
 * The handling of the const and volatile qualifiers happens in UserdataPtr.
 */
template <class C, bool ByContainer>
struct RefStackHelper
{
    using ReturnType = C;
    using T = std::remove_const_t<typename ContainerTraits<C>::Type>;

    static inline bool push(lua_State* L, const C& t, std::error_code& ec)
    {
        return UserdataSharedHelper<C, std::is_const_v<typename ContainerTraits<C>::Type>>::push(L, t, ec);
    }

    static ReturnType get(lua_State* L, int index)
    {
        return ContainerTraits<C>::construct(Userdata::get<T>(L, index, true));
    }
};

template <class T>
struct RefStackHelper<T, false>
{
    using ReturnType = T&;

    static bool push(lua_State* L, const T& t, std::error_code& ec)
    {
        return UserdataPtr::push(L, &t, ec);
    }

    static ReturnType get(lua_State* L, int index)
    {
        T* t = Userdata::get<T>(L, index, true);

        if (!t)
            luaL_error(L, "nil passed to reference");

        return *t;
    }
};

//=================================================================================================
/**
 * @brief Trait class that selects whether to return a user registered class object by value or by reference.
 */
template <class T, class Enable = void>
struct UserdataGetter
{
    using ReturnType = T*;

    static ReturnType get(lua_State* L, int index)
    {
        return Userdata::get<T>(L, index, false);
    }
};

template <class T>
struct UserdataGetter<T, std::void_t<T (*)()>>
{
    using ReturnType = T;

    static ReturnType get(lua_State* L, int index)
    {
        return StackHelper<T, IsContainer<T>::value>::get(L, index);
    }
};

} // namespace detail

//=================================================================================================
/**
 * @brief Lua stack conversions for class objects passed by value.
 */
template <class T>
struct Stack
{
    using IsUserdata = void;

    using Getter = detail::UserdataGetter<T>;
    using ReturnType = typename Getter::ReturnType;

    static bool push(lua_State* L, const T& value, std::error_code& ec)
    {
        return detail::StackHelper<T, detail::IsContainer<T>::value>::push(L, value, ec);
    }

    static ReturnType get(lua_State* L, int index)
    {
        return Getter::get(L, index);
    }

    static bool isInstance(lua_State* L, int index)
    {
        return detail::Userdata::isInstance<T>(L, index);
    }
};

namespace detail {

//=================================================================================================
/**
 * @brief Trait class indicating whether the parameter type must be a user registered class.
 *
 * The trait checks the existence of member type Stack::IsUserdata specialization for detection.
 */
template <class T, class Enable = void>
struct IsUserdata : std::false_type
{
};

template <class T>
struct IsUserdata<T, std::void_t<typename Stack<T>::IsUserdata>> : std::true_type
{
};

//=================================================================================================
/**
 * @brief Trait class that selects a specific push/get implemenation.
 */
template <class T, bool IsUserdata>
struct StackOpSelector;

// pointer
template <class T>
struct StackOpSelector<T*, true>
{
    using ReturnType = T*;

    static bool push(lua_State* L, T* value, std::error_code& ec) { return UserdataPtr::push(L, value, ec); }

    static T* get(lua_State* L, int index) { return Userdata::get<T>(L, index, false); }

    static bool isInstance(lua_State* L, int index) { return Userdata::isInstance<T>(L, index); }
};

// pointer to const
template <class T>
struct StackOpSelector<const T*, true>
{
    using ReturnType = const T*;

    static bool push(lua_State* L, const T* value, std::error_code& ec) { return UserdataPtr::push(L, value, ec); }

    static const T* get(lua_State* L, int index) { return Userdata::get<T>(L, index, true); }

    static bool isInstance(lua_State* L, int index) { return Userdata::isInstance<T>(L, index); }
};

// l-value reference
template <class T>
struct StackOpSelector<T&, true>
{
    using Helper = RefStackHelper<T, IsContainer<T>::value>;
    using ReturnType = typename Helper::ReturnType;

    static bool push(lua_State* L, T& value, std::error_code& ec) { return UserdataPtr::push(L, &value, ec); }

    static ReturnType get(lua_State* L, int index) { return Helper::get(L, index); }

    static bool isInstance(lua_State* L, int index) { return Userdata::isInstance<T>(L, index); }
};

// l-value reference to const
template <class T>
struct StackOpSelector<const T&, true>
{
    using Helper = RefStackHelper<T, IsContainer<T>::value>;
    using ReturnType = typename Helper::ReturnType;

    static bool push(lua_State* L, const T& value, std::error_code& ec) { return Helper::push(L, value, ec); }

    static ReturnType get(lua_State* L, int index) { return Helper::get(L, index); }

    static bool isInstance(lua_State* L, int index) { return Userdata::isInstance<T>(L, index); }
};

} // namespace detail
} // namespace luabridge
