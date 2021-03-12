// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// Copyright 2019, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT

#pragma once

#include "Config.h"
#include "ClassInfo.h"
#include "LuaException.h"
#include "Security.h"
#include "TypeTraits.h"

#include <stdexcept>
#include <string_view>
#include <string>
#include <type_traits>
#include <utility>

namespace luabridge {
namespace detail {

/**
 * Base for class and namespace registration.
 * Maintains Lua stack in the proper state.
 * Once beginNamespace, beginClass or deriveClass is called the parent
 * object upon its destruction may no longer clear the Lua stack.
 * Then endNamespace or endClass is called, a new parent is created
 * and the child transfers the responsibility for clearing stack to it.
 * So there can be maximum one "active" registrar object.
 */
class Registrar
{
protected:
    lua_State* const L;
    int mutable m_stackSize;

    Registrar(lua_State* L)
        : L(L)
        , m_stackSize(0)
    {
    }

    Registrar(const Registrar& rhs)
        : L(rhs.L)
        , m_stackSize(std::exchange(rhs.m_stackSize, 0))
    {
    }

    Registrar& operator=(const Registrar& rhs)
    {
        using std::swap;
        
        Registrar tmp(rhs);
        
        swap(m_stackSize, tmp.m_stackSize);
        
        return *this;
    }

    ~Registrar()
    {
        if (m_stackSize > 0)
        {
            assert(m_stackSize <= lua_gettop(L));
            lua_pop(L, m_stackSize);
        }
    }

    void assertIsActive() const
    {
        if (m_stackSize == 0)
        {
            throw_or_assert<std::logic_error>("Unable to continue registration");
        }
    }
};

} // namespace detail

/** Provides C++ to Lua registration capabilities.

    This class is not instantiated directly, call `getGlobalNamespace` to start
    the registration process.
*/
class Namespace : public detail::Registrar
{
    //============================================================================
#if 0
  /**
    Error reporting.

    VF: This function looks handy, why aren't we using it?
  */
  static int luaError (lua_State* L, std::string message)
  {
    assert (lua_isstring (L, lua_upvalueindex (1)));
    std::string s;

    // Get information on the caller's caller to format the message,
    // so the error appears to originate from the Lua source.
    lua_Debug ar;
    int result = lua_getstack (L, 2, &ar);
    if (result != 0)
    {
      lua_getinfo (L, "Sl", &ar);
      s = ar.short_src;
      if (ar.currentline != -1)
      {
        // poor mans int to string to avoid <strstrream>.
        lua_pushnumber (L, ar.currentline);
        s = s + ":" + lua_tostring (L, -1) + ": ";
        lua_pop (L, 1);
      }
    }

    s = s + message;

    return luaL_error (L, s.c_str ());
  }
#endif

    /**
      Factored base to reduce template instantiations.
    */
    class ClassBase : public detail::Registrar
    {
    public:
        explicit ClassBase(Namespace& parent) : Registrar(parent) {}

        using Registrar::operator=;

    protected:
        //--------------------------------------------------------------------------
        /**
          Create the const table.
        */
        void createConstTable(const char* name, bool trueConst = true)
        {
            assert(name != nullptr);
            
            std::string type_name = std::string(trueConst ? "const " : "") + name;

            // Stack: namespace table (ns)
            lua_newtable(L); // Stack: ns, const table (co)
            lua_pushvalue(L, -1); // Stack: ns, co, co
            lua_setmetatable(L, -2); // co.__metatable = co. Stack: ns, co

            lua_pushstring(L, type_name.c_str());
            lua_rawsetp(L, -2, detail::getTypeKey()); // co [typeKey] = name. Stack: ns, co

            lua_pushcfunction(L, &detail::index_metamethod);
            rawsetfield(L, -2, "__index");

            lua_pushcfunction(L, &detail::newindex_object_metamethod);
            rawsetfield(L, -2, "__newindex");

            lua_newtable(L);
            lua_rawsetp(L, -2, detail::getPropgetKey());

            if (Security::hideMetatables())
            {
                lua_pushnil(L);
                rawsetfield(L, -2, "__metatable");
            }
        }

        //--------------------------------------------------------------------------
        /**
          Create the class table.

          The Lua stack should have the const table on top.
        */
        void createClassTable(char const* name)
        {
            assert(name != nullptr);

            // Stack: namespace table (ns), const table (co)

            // Class table is the same as const table except the propset table
            createConstTable(name, false); // Stack: ns, co, cl

            lua_newtable(L); // Stack: ns, co, cl, propset table (ps)
            lua_rawsetp(L, -2, detail::getPropsetKey()); // cl [propsetKey] = ps. Stack: ns, co, cl

            lua_pushvalue(L, -2); // Stack: ns, co, cl, co
            lua_rawsetp(L, -2, detail::getConstKey()); // cl [constKey] = co. Stack: ns, co, cl

            lua_pushvalue(L, -1); // Stack: ns, co, cl, cl
            lua_rawsetp(L, -3, detail::getClassKey()); // co [classKey] = cl. Stack: ns, co, cl
        }

        //--------------------------------------------------------------------------
        /**
          Create the static table.
        */
        void createStaticTable(char const* name)
        {
            assert(name != nullptr);

            // Stack: namespace table (ns), const table (co), class table (cl)
            lua_newtable(L); // Stack: ns, co, cl, visible static table (vst)
            lua_newtable(L); // Stack: ns, co, cl, st, static metatable (st)
            lua_pushvalue(L, -1); // Stack: ns, co, cl, vst, st, st
            lua_setmetatable(L, -3); // st.__metatable = mt. Stack: ns, co, cl, vst, st
            lua_insert(L, -2); // Stack: ns, co, cl, st, vst
            rawsetfield(L, -5, name); // ns [name] = vst. Stack: ns, co, cl, st

#if 0
            lua_pushlightuserdata (L, this);
            lua_pushcclosure (L, &tostringMetaMethod, 1);
            rawsetfield (L, -2, "__tostring");
#endif

            lua_pushcfunction(L, &detail::index_metamethod);
            rawsetfield(L, -2, "__index");

            lua_pushcfunction(L, &detail::newindex_static_metamethod);
            rawsetfield(L, -2, "__newindex");

            lua_newtable(L); // Stack: ns, co, cl, st, proget table (pg)
            lua_rawsetp(L, -2, detail::getPropgetKey()); // st [propgetKey] = pg. Stack: ns, co, cl, st

            lua_newtable(L); // Stack: ns, co, cl, st, propset table (ps)
            lua_rawsetp(L, -2, detail::getPropsetKey()); // st [propsetKey] = pg. Stack: ns, co, cl, st

            lua_pushvalue(L, -2); // Stack: ns, co, cl, st, cl
            lua_rawsetp(L, -2, detail::getClassKey()); // st [classKey] = cl. Stack: ns, co, cl, st

            if (Security::hideMetatables())
            {
                lua_pushnil(L);
                rawsetfield(L, -2, "__metatable");
            }
        }

        //==========================================================================
        /**
          lua_CFunction to construct a class object wrapped in a container.
        */
        template <class Args, class C>
        static int ctorContainerProxy(lua_State* L)
        {
            using T = typename ContainerTraits<C>::Type;
            
            T* object = detail::constructor<T, Args>::call(detail::make_arguments_list<Args, 2>(L));

            std::error_code ec;
            if (! detail::UserdataSharedHelper<C, false>::push(L, object, ec))
                luaL_error(L, ec.message().c_str());

            return 1;
        }

        //--------------------------------------------------------------------------
        /**
          lua_CFunction to construct a class object in-place in the userdata.
        */
        template <class Args, class T>
        static int ctorPlacementProxy(lua_State* L)
        {
            std::error_code ec;
            detail::UserdataValue<T>* value = detail::UserdataValue<T>::place(L, ec);
            if (! value)
                luaL_error(L, ec.message().c_str());

            detail::constructor<T, Args>::call(value->getObject(), detail::make_arguments_list<Args, 2>(L));

            value->commit();

            return 1;
        }

        void assertStackState() const
        {
            // Stack: const table (co), class table (cl), static table (st)
            assert(lua_istable(L, -3));
            assert(lua_istable(L, -2));
            assert(lua_istable(L, -1));
        }
    };

    //============================================================================
    //
    // Class
    //
    //============================================================================
    /**
      Provides a class registration in a lua_State.

      After construction the Lua stack holds these objects:
        -1 static table
        -2 class table
        -3 const table
        -4 enclosing namespace table
    */
    template<class T>
    class Class : public ClassBase
    {
    public:
        //==========================================================================
        /**
          Register a new class or add to an existing class registration.

          @param name   The new class name.
          @param parent A parent namespace object.
        */
        Class(char const* name, Namespace& parent) : ClassBase(parent)
        {
            assert(name != nullptr);
            assert(lua_istable(L, -1)); // Stack: namespace table (ns)

            rawgetfield(L, -1, name); // Stack: ns, static table (st) | nil

            if (lua_isnil(L, -1)) // Stack: ns, nil
            {
                lua_pop(L, 1); // Stack: ns

                createConstTable(name); // Stack: ns, const table (co)
                lua_pushcfunction(L, &detail::gc_metamethod<T>); // Stack: ns, co, function
                rawsetfield(L, -2, "__gc"); // co ["__gc"] = function. Stack: ns, co
                ++m_stackSize;

                createClassTable(name); // Stack: ns, co, class table (cl)
                lua_pushcfunction(L, &detail::gc_metamethod<T>); // Stack: ns, co, cl, function
                rawsetfield(L, -2, "__gc"); // cl ["__gc"] = function. Stack: ns, co, cl
                ++m_stackSize;

                createStaticTable(name); // Stack: ns, co, cl, st
                ++m_stackSize;

                // Map T back to its tables.
                lua_pushvalue(L, -1); // Stack: ns, co, cl, st, st
                lua_rawsetp(L, LUA_REGISTRYINDEX, detail::getStaticRegistryKey<T>()); // Stack: ns, co, cl, st
                lua_pushvalue(L, -2); // Stack: ns, co, cl, st, cl
                lua_rawsetp(L, LUA_REGISTRYINDEX, detail::getClassRegistryKey<T>()); // Stack: ns, co, cl, st
                lua_pushvalue(L, -3); // Stack: ns, co, cl, st, co
                lua_rawsetp(L, LUA_REGISTRYINDEX, detail::getConstRegistryKey<T>()); // Stack: ns, co, cl, st
            }
            else
            {
                assert(lua_istable(L, -1)); // Stack: ns, st
                ++m_stackSize;

                // Map T back from its stored tables

                lua_rawgetp(L, LUA_REGISTRYINDEX, detail::getConstRegistryKey<T>()); // Stack: ns, st, co
                lua_insert(L, -2); // Stack: ns, co, st
                ++m_stackSize;

                lua_rawgetp(L, LUA_REGISTRYINDEX, detail::getClassRegistryKey<T>()); // Stack: ns, co, st, cl
                lua_insert(L, -2); // Stack: ns, co, cl, st
                ++m_stackSize;
            }
        }

        //==========================================================================
        /**
          Derive a new class.

          @param name     The class name.
          @param parent A parent namespace object.
          @param staticKey
        */
        Class(char const* name, Namespace& parent, void const* const staticKey) : ClassBase(parent)
        {
            assert(name != nullptr);
            assert(lua_istable(L, -1)); // Stack: namespace table (ns)

            createConstTable(name); // Stack: ns, const table (co)
            lua_pushcfunction(L, &detail::gc_metamethod<T>); // Stack: ns, co, function
            rawsetfield(L, -2, "__gc"); // co ["__gc"] = function. Stack: ns, co
            ++m_stackSize;

            createClassTable(name); // Stack: ns, co, class table (cl)
            lua_pushcfunction(L, &detail::gc_metamethod<T>); // Stack: ns, co, cl, function
            rawsetfield(L, -2, "__gc"); // cl ["__gc"] = function. Stack: ns, co, cl
            ++m_stackSize;

            createStaticTable(name); // Stack: ns, co, cl, st
            ++m_stackSize;

            lua_rawgetp( L, LUA_REGISTRYINDEX, staticKey); // Stack: ns, co, cl, st, parent st (pst) | nil
            if (lua_isnil(L, -1)) // Stack: ns, co, cl, st, nil
            {
                ++m_stackSize;

                throw_or_assert<std::logic_error>("Base class is not registered");
                return;
            }

            assert(lua_istable(L, -1)); // Stack: ns, co, cl, st, pst

            lua_rawgetp(L, -1, detail::getClassKey()); // Stack: ns, co, cl, st, pst, parent cl (pcl)
            assert(lua_istable(L, -1));

            lua_rawgetp(L, -1, detail::getConstKey()); // Stack: ns, co, cl, st, pst, pcl, parent co (pco)
            assert(lua_istable(L, -1));

            lua_rawsetp(L, -6, detail::getParentKey()); // co [parentKey] = pco. Stack: ns, co, cl, st, pst, pcl
            lua_rawsetp(L, -4, detail::getParentKey()); // cl [parentKey] = pcl. Stack: ns, co, cl, st, pst
            lua_rawsetp(L, -2, detail::getParentKey()); // st [parentKey] = pst. Stack: ns, co, cl, st

            lua_pushvalue(L, -1); // Stack: ns, co, cl, st, st
            lua_rawsetp(L, LUA_REGISTRYINDEX, detail::getStaticRegistryKey<T>()); // Stack: ns, co, cl, st
            lua_pushvalue(L, -2); // Stack: ns, co, cl, st, cl
            lua_rawsetp(L, LUA_REGISTRYINDEX, detail::getClassRegistryKey<T>()); // Stack: ns, co, cl, st
            lua_pushvalue(L, -3); // Stack: ns, co, cl, st, co
            lua_rawsetp(L, LUA_REGISTRYINDEX, detail::getConstRegistryKey<T>()); // Stack: ns, co, cl, st
        }

        //--------------------------------------------------------------------------
        /**
          Continue registration in the enclosing namespace.

          @returns A parent registration object.
        */
        Namespace endClass()
        {
            assert(m_stackSize > 3);

            m_stackSize -= 3;
            lua_pop(L, 3);
            return Namespace(*this);
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a static property.

          @tparam U          The type of the property.
          @param  name       The property name.
          @param  value      A property value pointer.
          @param  isWritable True for a read-write, false for read-only property.
          @returns This class registration object.
        */
        template <class U>
        Class<T>& addStaticProperty(char const* name, U* value, bool isWritable = true)
        {
            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_pushlightuserdata(L, value); // Stack: co, cl, st, pointer
            lua_pushcclosure(L, &detail::property_getter<U>::call, 1); // Stack: co, cl, st, getter
            detail::add_property_getter(L, name, -2); // Stack: co, cl, st

            if (isWritable)
            {
                lua_pushlightuserdata(L, value); // Stack: co, cl, st, ps, pointer
                lua_pushcclosure(L, &detail::property_setter<U>::call, 1); // Stack: co, cl, st, ps, setter
            }
            else
            {
                lua_pushstring(L, name); // Stack: co, cl, st, name
                lua_pushcclosure(L, &detail::read_only_error, 1); // Stack: co, cl, st, function
            }

            detail::add_property_setter(L, name, -2); // Stack: co, cl, st

            return *this;
        }

        //--------------------------------------------------------------------------
        /// Add or replace a static property member.
        ///
        /// @tparam U          The type of the property.
        /// @param  name       The property name.
        /// @param  get        A property getter function pointer.
        /// @param  set        A property setter function pointer, optional, nullable.
        ///                    Omit or pass nullptr for a read-only property.
        /// @returns This class registration object.
        ///
        template <class U>
        Class<T>& addStaticProperty(char const* name, U (*get)(), void (*set)(U) = nullptr)
        {
            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_pushlightuserdata(L, reinterpret_cast<void*>(get)); // Stack: co, cl, st, function ptr
            lua_pushcclosure(L, &detail::invoke_proxy_function<U (*)()>, 1); // Stack: co, cl, st, getter
            detail::add_property_getter(L, name, -2); // Stack: co, cl, st

            if (set != nullptr)
            {
                lua_pushlightuserdata(L, reinterpret_cast<void*>(set)); // Stack: co, cl, st, function ptr
                lua_pushcclosure(L, &detail::invoke_proxy_function<void (*)(U)>, 1); // Stack: co, cl, st, setter
            }
            else
            {
                lua_pushstring(L, name); // Stack: co, cl, st, ps, name
                lua_pushcclosure(L, &detail::read_only_error, 1); // Stack: co, cl, st, function
            }

            detail::add_property_setter(L, name, -2); // Stack: co, cl, st

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a static member function.
        */
        template <class FP>
        Class<T>& addStaticFunction(char const* name, FP const fp)
        {
            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_pushlightuserdata(L, reinterpret_cast<void*>(fp)); // Stack: co, cl, st, function ptr
            lua_pushcclosure(L, &detail::invoke_proxy_function<FP>, 1); // co, cl, st, function
            rawsetfield(L, -2, name); // co, cl, st

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a static member function by std::function.
        */
        template <class ReturnType, class... Params>
        Class<T>& addStaticFunction(char const* name, std::function<ReturnType(Params...)> function)
        {
            using FnType = decltype(function);

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_newuserdata_aligned<FnType>(L, std::move(function)); // Stack: co, cl, st, function userdata (ud)
            lua_newtable(L); // Stack: co, cl, st, ud, ud metatable (mt)
            lua_pushcfunction(L, &lua_deleteuserdata_aligned<FnType>); // Stack: co, cl, st, ud, mt, gc function
            rawsetfield(L, -2, "__gc"); // Stack: co, cl, st, ud, mt
            lua_setmetatable(L, -2); // Stack: co, cl, st, ud
            lua_pushcclosure(L, &detail::invoke_proxy_functor<FnType>, 1); // Stack: co, cl, st, function
            rawsetfield(L, -2, name); // Stack: co, cl, st

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a lua_CFunction.

          @param name The name of the function.
          @param fp   A C-function pointer.
          @returns This class registration object.
        */
        Class<T>& addStaticFunction(char const* name, int (*const fp)(lua_State*))
        {
            return addStaticCFunction(name, fp);
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a lua_CFunction.
        */
        Class<T>& addStaticCFunction(char const* name, int (*const fp)(lua_State*))
        {
            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_pushcfunction(L, fp); // co, cl, st, function
            rawsetfield(L, -2, name); // co, cl, st

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a property member.
        */
        template <class U, class V>
        Class<T>& addProperty(char const* name, U V::*mp, bool isWritable = true)
        {
            static_assert(std::is_base_of_v<V, T>);

            using MemberPtrType = decltype(mp);

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            new (lua_newuserdata(L, sizeof(MemberPtrType))) MemberPtrType(mp); // Stack: co, cl, st, field ptr
            lua_pushcclosure(L, &detail::property_getter<U, T>::call, 1); // Stack: co, cl, st, getter
            lua_pushvalue(L, -1); // Stack: co, cl, st, getter, getter
            detail::add_property_getter(L, name, -5); // Stack: co, cl, st, getter
            detail::add_property_getter(L, name, -3); // Stack: co, cl, st

            if (isWritable)
            {
                new (lua_newuserdata(L, sizeof(MemberPtrType))) MemberPtrType(mp); // Stack: co, cl, st, field ptr
                lua_pushcclosure(L, &detail::property_setter<U, T>::call, 1); // Stack: co, cl, st, setter
                detail::add_property_setter(L, name, -3); // Stack: co, cl, st
            }

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a property member.
        */
        template <class TG, class TS = TG>
        Class<T>& addProperty(char const* name, TG (T::*get)() const, void (T::*set)(TS) = nullptr)
        {
            typedef TG (T::*get_t)() const;
            typedef void (T::*set_t)(TS);

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            new (lua_newuserdata(L, sizeof(get_t))) get_t(get); // Stack: co, cl, st, funcion ptr
            lua_pushcclosure(L, &detail::invoke_const_member_function<get_t, T>, 1); // Stack: co, cl, st, getter
            lua_pushvalue(L, -1); // Stack: co, cl, st, getter, getter
            detail::add_property_getter(L, name, -5); // Stack: co, cl, st, getter
            detail::add_property_getter(L, name, -3); // Stack: co, cl, st

            if (set != nullptr)
            {
                new (lua_newuserdata(L, sizeof(set_t))) set_t(set); // Stack: co, cl, st, function ptr
                lua_pushcclosure(L, &detail::invoke_member_function<set_t, T>, 1); // Stack: co, cl, st, setter
                detail::add_property_setter(L, name, -3); // Stack: co, cl, st
            }

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a property member.
        */
        template <class TG, class TS = TG>
        Class<T>& addProperty(char const* name, TG (T::*get)(lua_State*) const, void (T::*set)(TS, lua_State*) = nullptr)
        {
            typedef TG (T::*get_t)(lua_State*) const;
            typedef void (T::*set_t)(TS, lua_State*);

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            new (lua_newuserdata(L, sizeof(get_t))) get_t(get); // Stack: co, cl, st, funcion ptr
            lua_pushcclosure(L, &detail::invoke_const_member_function<get_t, T>, 1); // Stack: co, cl, st, getter
            lua_pushvalue(L, -1); // Stack: co, cl, st, getter, getter
            detail::add_property_getter(L, name, -5); // Stack: co, cl, st, getter
            detail::add_property_getter(L, name, -3); // Stack: co, cl, st

            if (set != nullptr)
            {
                new (lua_newuserdata(L, sizeof(set_t))) set_t(set); // Stack: co, cl, st, function ptr
                lua_pushcclosure(L, &detail::invoke_member_function<set_t, T>, 1); // Stack: co, cl, st, setter
                detail::add_property_setter(L, name, -3); // Stack: co, cl, st
            }

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a property member, by proxy.

          When a class is closed for modification and does not provide (or cannot
          provide) the function signatures necessary to implement get or set for
          a property, this will allow non-member functions act as proxies.

          Both the get and the set functions require a T const* and T* in the first
          argument respectively.
        */
        template <class TG, class TS = TG>
        Class<T>& addProperty(char const* name, TG (*get)(T const*), void (*set)(T*, TS) = nullptr)
        {
            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_pushlightuserdata(L, reinterpret_cast<void*>(get)); // Stack: co, cl, st, function ptr
            lua_pushcclosure(L, &detail::invoke_proxy_function<TG (*)(const T*)>, 1); // Stack: co, cl, st, getter
            lua_pushvalue(L, -1); // Stack: co, cl, st,, getter, getter
            detail::add_property_getter(L, name, -5); // Stack: co, cl, st, getter
            detail::add_property_getter(L, name, -3); // Stack: co, cl, st

            if (set != nullptr)
            {
                lua_pushlightuserdata( L, reinterpret_cast<void*>(set)); // Stack: co, cl, st, function ptr
                lua_pushcclosure(L, &detail::invoke_proxy_function<void (*)(T*, TS)>, 1); // Stack: co, cl, st, setter
                detail::add_property_setter(L, name, -3); // Stack: co, cl, st
            }

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a property member, by proxy C-function.

          When a class is closed for modification and does not provide (or cannot
          provide) the function signatures necessary to implement get or set for
          a property, this will allow non-member functions act as proxies.

          The object userdata ('this') value is at the index 1.
          The new value for set function is at the index 2.
        */
        Class<T>& addProperty(char const* name, int (*get)(lua_State*), int (*set)(lua_State*) = nullptr)
        {
            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_pushcfunction(L, get);
            lua_pushvalue(L, -1); // Stack: co, cl, st,, getter, getter
            detail::add_property_getter(L, name, -5); // Stack: co, cl, st,, getter
            detail::add_property_getter(L, name, -3); // Stack: co, cl, st,

            if (set != nullptr)
            {
                lua_pushcfunction(L, set);
                detail::add_property_setter(L, name, -3); // Stack: co, cl, st,
            }

            return *this;
        }

        template <class TG, class TS = TG>
        Class<T>& addProperty(char const* name, std::function<TG(const T*)> get, std::function<void(T*, TS)> set = nullptr)
        {
            using GetType = decltype(get);

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_newuserdata_aligned<GetType>(L, std::move(get)); // Stack: co, cl, st, function userdata (ud)
            lua_newtable(L); // Stack: co, cl, st, ud, ud metatable (mt)
            lua_pushcfunction(L, &lua_deleteuserdata_aligned<GetType>); // Stack: co, cl, st, ud, mt, gc function
            rawsetfield(L, -2, "__gc"); // Stack: co, cl, st, ud, mt
            lua_setmetatable(L, -2); // Stack: co, cl, st, ud
            lua_pushcclosure(L, &detail::invoke_proxy_functor<GetType>, 1); // Stack: co, cl, st, getter
            lua_pushvalue(L, -1); // Stack: co, cl, st, getter, getter
            detail::add_property_getter(L, name, -4); // Stack: co, cl, st, getter
            detail::add_property_getter(L, name, -4); // Stack: co, cl, st

            if (set != nullptr)
            {
                using SetType = decltype(set);

                lua_newuserdata_aligned<SetType>(L, std::move(set)); // Stack: co, cl, st, function userdata (ud)
                lua_newtable(L); // Stack: co, cl, st, ud, ud metatable (mt)
                lua_pushcfunction(L, &lua_deleteuserdata_aligned<SetType>); // Stack: co, cl, st, ud, mt, gc function
                rawsetfield(L, -2, "__gc"); // Stack: co, cl, st, ud, mt
                lua_setmetatable(L, -2); // Stack: co, cl, st, ud
                lua_pushcclosure(L, &detail::invoke_proxy_functor<SetType>, 1); // Stack: co, cl, st, setter
                detail::add_property_setter(L, name, -3); // Stack: co, cl, st
            }

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
            Add or replace a namespace function by convertible to std::function (capturing lambdas).
        */
        template <class Function, typename = std::enable_if_t<detail::function_arity_v<Function> != 0>>
        Class<T> addFunction(char const* name, Function function)
        {
            using FnTraits = detail::function_traits<Function>;
            
            using FnType = detail::to_std_function_type_t<
                typename FnTraits::result_type,
                typename FnTraits::argument_types>;
            
            using FirstArg = detail::function_argument_t<0, Function>;
            static_assert(std::is_same_v<std::decay_t<std::remove_pointer_t<FirstArg>>, T>);

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_newuserdata_aligned<FnType>(L, std::move(function)); // Stack: co, cl, st, function userdata (ud)
            lua_newtable(L); // Stack: co, cl, st, ud, ud metatable (mt)
            lua_pushcfunction(L, &lua_deleteuserdata_aligned<FnType>); // Stack: co, cl, st, ud, mt, gc function
            rawsetfield(L, -2, "__gc"); // Stack: co, cl, st, ud, mt
            lua_setmetatable(L, -2); // Stack: co, cl, st, ud

            lua_pushcclosure(L, &detail::invoke_proxy_functor<FnType>, 1); // Stack: co, cl, st, function

            if constexpr (! std::is_const_v<std::remove_reference_t<std::remove_pointer_t<FirstArg>>>)
            {
                rawsetfield(L, -3, name); // Stack: co, cl, st
            }
            else
            {
                lua_pushvalue(L, -1); // Stack: co, cl, st, function, function
                rawsetfield(L, -4, name); // Stack: co, cl, st, function
                rawsetfield(L, -4, name); // Stack: co, cl, st
            }
            
            return *this;
        }

        //--------------------------------------------------------------------------
        /**
            Add or replace a member function.
        */
        template <class U, class ReturnType, class... Params>
        Class<T>& addFunction(char const* name, ReturnType (U::*mf)(Params...))
        {
            static_assert(std::is_base_of_v<U, T>);

            using MemFn = decltype(mf);

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            if (name == std::string_view("__gc"))
            {
                throw_or_assert<std::logic_error>("__gc metamethod registration is forbidden");
                return *this;
            }
            
            new (lua_newuserdata(L, sizeof(MemFn))) MemFn(mf);
            lua_pushcclosure(L, &detail::invoke_member_function<MemFn, T>, 1);
            rawsetfield(L, -3, name); // class table

            return *this;
        }

        template <class U, class ReturnType, class... Params>
        Class<T>& addFunction(char const* name, ReturnType (U::*mf)(Params...) const)
        {
            static_assert(std::is_base_of_v<U, T>);

            using MemFn = decltype(mf);

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            if (name == std::string_view("__gc"))
            {
                throw_or_assert<std::logic_error>("__gc metamethod registration is forbidden");
                return *this;
            }
            
            new (lua_newuserdata(L, sizeof(MemFn))) MemFn(mf);
            lua_pushcclosure(L, &detail::invoke_const_member_function<MemFn, T>, 1);
            lua_pushvalue(L, -1);
            rawsetfield(L, -5, name); // const table
            rawsetfield(L, -3, name); // class table

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
            Add or replace a proxy function.
        */
        template <class ReturnType, class... Params>
        Class<T>& addFunction(char const* name, ReturnType (*proxyFn)(T* object, Params...))
        {
            using FnType = decltype(proxyFn);

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            if (name == std::string_view("__gc"))
            {
                throw_or_assert<std::logic_error>("__gc metamethod registration is forbidden");
                return *this;
            }
            
            lua_pushlightuserdata(L, reinterpret_cast<void*>(proxyFn)); // Stack: co, cl, st, function ptr
            lua_pushcclosure(L, &detail::invoke_proxy_function<FnType>, 1); // Stack: co, cl, st, function
            rawsetfield(L, -3, name); // Stack: co, cl, st

            return *this;
        }

        template <class ReturnType, class... Params>
        Class<T>& addFunction(char const* name, ReturnType (*proxyFn)(const T* object, Params...))
        {
            using FnType = decltype(proxyFn);

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            if (name == std::string_view("__gc"))
            {
                throw_or_assert<std::logic_error>("__gc metamethod registration is forbidden");
                return *this;
            }
            
            lua_pushlightuserdata(L, reinterpret_cast<void*>(proxyFn)); // Stack: co, cl, st, function ptr
            lua_pushcclosure(L, &detail::invoke_proxy_function<FnType>, 1); // Stack: co, cl, st, function
            lua_pushvalue(L, -1); // Stack: co, cl, st, function, function
            rawsetfield(L, -4, name); // Stack: co, cl, st, function
            rawsetfield(L, -4, name); // Stack: co, cl, st

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
            Add or replace a member lua_CFunction.
        */
        template <class U>
        Class<T>& addFunction(char const* name, int (U::*mfp)(lua_State*))
        {
            static_assert(std::is_base_of_v<U, T>);

            return addCFunction<U>(name, mfp);
        }

        //--------------------------------------------------------------------------
        /**
            Add or replace a member lua_CFunction.
        */
        template <class U>
        Class<T>& addCFunction(char const* name, int (U::*mfp)(lua_State*))
        {
            static_assert(std::is_base_of_v<U, T>);

            using F = decltype(mfp);

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            new (lua_newuserdata(L, sizeof(mfp))) F(mfp); // Stack: co, cl, st, function ptr
            lua_pushcclosure(L, &detail::invoke_member_cfunction<T>, 1); // Stack: co, cl, st, function
            rawsetfield(L, -3, name); // Stack: co, cl, st

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
            Add or replace a const member lua_CFunction.
        */
        template <class U>
        Class<T>& addFunction(char const* name, int (U::*mfp)(lua_State*) const)
        {
            static_assert(std::is_base_of_v<U, T>);

            return addCFunction<U>(name, mfp);
        }

        //--------------------------------------------------------------------------
        /**
            Add or replace a const member lua_CFunction.
        */
        template <class U>
        Class<T>& addCFunction(char const* name, int (U::*mfp)(lua_State*) const)
        {
            static_assert(std::is_base_of_v<U, T>);
            
            using F = decltype(mfp);

            assert(name != nullptr);
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            new (lua_newuserdata(L, sizeof(mfp))) F(mfp);
            lua_pushcclosure(L, &detail::invoke_const_member_cfunction<T>, 1);
            lua_pushvalue(L, -1); // Stack: co, cl, st, function, function
            rawsetfield(L, -4, name); // Stack: co, cl, st, function
            rawsetfield(L, -4, name); // Stack: co, cl, st

            return *this;
        }

        //--------------------------------------------------------------------------
        /**
          Add or replace a primary Constructor.

          The primary Constructor is invoked when calling the class type table
          like a function.

          The template parameter should be a function pointer type that matches
          the desired Constructor (since you can't take the address of a Constructor
          and pass it as an argument).
        */
        template <class MemFn, class C>
        Class<T>& addConstructor()
        {
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_pushcclosure(L, &ctorContainerProxy<detail::function_arguments_t<MemFn>, C>, 0);
            rawsetfield(L, -2, "__call");

            return *this;
        }

        template <class MemFn>
        Class<T>& addConstructor()
        {
            assertStackState(); // Stack: const table (co), class table (cl), static table (st)

            lua_pushcclosure(L, &ctorPlacementProxy<detail::function_arguments_t<MemFn>, T>, 0);
            rawsetfield(L, -2, "__call");

            return *this;
        }
    };

private:
    //----------------------------------------------------------------------------
    /**
        Open the global namespace for registrations.

        @param L A Lua state.
    */
    explicit Namespace(lua_State* L) : Registrar(L)
    {
        lua_getglobal(L, "_G");
        ++m_stackSize;
    }

    //----------------------------------------------------------------------------
    /**
        Open a namespace for registrations.
        The namespace is created if it doesn't already exist.

        @param name   The namespace name.
        @param parent The parent namespace object.
        @pre The parent namespace is at the top of the Lua stack.
    */
    Namespace(char const* name, Namespace& parent) : Registrar(parent)
    {
        assert(name != nullptr);
        assert(lua_istable(L, -1)); // Stack: parent namespace (pns)

        rawgetfield(L, -1, name); // Stack: pns, namespace (ns) | nil

        if (lua_isnil(L, -1)) // Stack: pns, nil
        {
            lua_pop(L, 1); // Stack: pns

            lua_newtable(L); // Stack: pns, ns
            lua_pushvalue(L, -1); // Stack: pns, ns, ns

            // na.__metatable = ns
            lua_setmetatable(L, -2); // Stack: pns, ns

            // ns.__index = index_metamethod
            lua_pushcfunction(L, &detail::index_metamethod);
            rawsetfield(L, -2, "__index"); // Stack: pns, ns

            // ns.__newindex = newindex_static_metamethod
            lua_pushcfunction(L, &detail::newindex_static_metamethod);
            rawsetfield(L, -2, "__newindex"); // Stack: pns, ns

            lua_newtable(L); // Stack: pns, ns, propget table (pg)
            lua_rawsetp(L, -2, detail::getPropgetKey()); // ns [propgetKey] = pg. Stack: pns, ns

            lua_newtable(L); // Stack: pns, ns, propset table (ps)
            lua_rawsetp(L, -2, detail::getPropsetKey()); // ns [propsetKey] = ps. Stack: pns, ns

            // pns [name] = ns
            lua_pushvalue(L, -1); // Stack: pns, ns, ns
            rawsetfield(L, -3, name); // Stack: pns, ns
        }

        ++m_stackSize;
    }

    //----------------------------------------------------------------------------
    /**
        Close the class and continue the namespace registrations.

        @param child A child class registration object.
    */
    explicit Namespace(ClassBase& child)
        : Registrar(child)
    {
    }

    using Registrar::operator=;

public:
    //----------------------------------------------------------------------------
    /**
      Retrieve the global namespace.
      It is recommended to put your namespace inside the global namespace, and
      then add your classes and functions to it, rather than adding many classes
      and functions directly to the global namespace.

      @param L A Lua state.
      @returns A namespace registration object.
    */
    static Namespace getGlobalNamespace(lua_State* L)
    {
        return Namespace(L);
    }

    //----------------------------------------------------------------------------
    /**
        Open a new or existing namespace for registrations.

        @param name The namespace name.
        @returns A namespace registration object.
    */
    Namespace beginNamespace(char const* name)
    {
        assertIsActive();
        return Namespace(name, *this);
    }

    //----------------------------------------------------------------------------
    /**
        Continue namespace registration in the parent.
        Do not use this on the global namespace.

        @returns A parent namespace registration object.
    */
    Namespace endNamespace()
    {
        if (m_stackSize == 1)
        {
            throw_or_assert<std::logic_error>("endNamespace() called on global namespace");

            return Namespace(*this);
        }
        
        assert(m_stackSize > 1);
        --m_stackSize;
        lua_pop(L, 1);
        return Namespace(*this);
    }

    //----------------------------------------------------------------------------
    /**
        Add or replace a property.

        @param name       The property name.
        @param value      A value pointer.
        @param isWritable True for a read-write, false for read-only property.
        @returns This namespace registration object.
    */
    template <class T>
    Namespace& addProperty(char const* name, T* value, bool isWritable = true)
    {
        if (m_stackSize == 1)
        {
            throw_or_assert<std::logic_error>("addProperty() called on global namespace");

            return *this;
        }
        
        assert(name != nullptr);
        assert(lua_istable(L, -1)); // Stack: namespace table (ns)

        lua_pushlightuserdata(L, value); // Stack: ns, pointer
        lua_pushcclosure(L, &detail::property_getter<T>::call, 1); // Stack: ns, getter
        detail::add_property_getter(L, name, -2); // Stack: ns

        if (isWritable)
        {
            lua_pushlightuserdata(L, value); // Stack: ns, pointer
            lua_pushcclosure(L, &detail::property_setter<T>::call, 1); // Stack: ns, setter
        }
        else
        {
            lua_pushstring(L, name); // Stack: ns, ps, name
            lua_pushcclosure(L, &detail::read_only_error, 1); // Stack: ns, function
        }

        detail::add_property_setter(L, name, -2); // Stack: ns

        return *this;
    }

    //----------------------------------------------------------------------------
    /**
        Add or replace a property.
        If the set function is omitted or null, the property is read-only.

        @param name       The property name.
        @param get  A pointer to a property getter function.
        @param set  A pointer to a property setter function, optional.
        @returns This namespace registration object.
    */
    template <class TG, class TS = TG>
    Namespace& addProperty(char const* name, TG (*get)(), void (*set)(TS) = nullptr)
    {
        if (m_stackSize == 1)
        {
            throw_or_assert<std::logic_error>("addProperty() called on global namespace");

            return *this;
        }
        
        assert(name != nullptr);
        assert(lua_istable(L, -1)); // Stack: namespace table (ns)

        lua_pushlightuserdata(L, reinterpret_cast<void*>(get)); // Stack: ns, function ptr
        lua_pushcclosure(L, &detail::invoke_proxy_function<TG (*)()>, 1); // Stack: ns, getter
        detail::add_property_getter(L, name, -2);

        if (set != nullptr)
        {
            lua_pushlightuserdata(L, reinterpret_cast<void*>(set)); // Stack: ns, function ptr
            lua_pushcclosure(L, &detail::invoke_proxy_function<void (*)(TS)>, 1);
        }
        else
        {
            lua_pushstring(L, name);
            lua_pushcclosure(L, &detail::read_only_error, 1);
        }

        detail::add_property_setter(L, name, -2);

        return *this;
    }

    //----------------------------------------------------------------------------
    /**
        Add or replace a property.
        If the set function is omitted or null, the property is read-only.

        @param name The property name.
        @param get  A pointer to a property getter function.
        @param set  A pointer to a property setter function, optional.
        @returns This namespace registration object.
    */
    Namespace& addProperty(char const* name, int (*get)(lua_State*), int (*set)(lua_State*) = nullptr)
    {
        if (m_stackSize == 1)
        {
            throw_or_assert<std::logic_error>("addProperty() called on global namespace");

            return *this;
        }

        assert(name != nullptr);
        assert(lua_istable(L, -1)); // Stack: namespace table (ns)

        lua_pushcfunction(L, get); // Stack: ns, getter
        detail::add_property_getter(L, name, -2); // Stack: ns

        if (set != nullptr)
        {
            lua_pushcfunction(L, set); // Stack: ns, setter
            detail::add_property_setter(L, name, -2); // Stack: ns
        }
        else
        {
            lua_pushstring(L, name); // Stack: ns, name
            lua_pushcclosure(L, &detail::read_only_error, 1); // Stack: ns, name, function
            detail::add_property_setter(L, name, -2); // Stack: ns
        }

        return *this;
    }

    //----------------------------------------------------------------------------
    /**
        Add or replace a namespace function by convertible to std::function.
    */
    template <class Function>
    Namespace& addFunction(char const* name, Function function)
    {
        using FnTraits = detail::function_traits<Function>;
        
        using FnType = detail::to_std_function_type_t<
            typename FnTraits::result_type,
            typename FnTraits::argument_types>;
        
        assert(name != nullptr);
        assert(lua_istable(L, -1)); // Stack: namespace table (ns)

        lua_newuserdata_aligned<FnType>(L, std::move(function)); // Stack: ns, function userdata (ud)
        lua_newtable(L); // Stack: ns, ud, ud metatable (mt)
        lua_pushcfunction(L, &lua_deleteuserdata_aligned<FnType>); // Stack: ns, ud, mt, gc function
        rawsetfield(L, -2, "__gc"); // Stack: ns, ud, mt
        lua_setmetatable(L, -2); // Stack: ns, ud
        lua_pushcclosure(L, &detail::invoke_proxy_functor<FnType>, 1); // Stack: ns, function
        rawsetfield(L, -2, name); // Stack: ns

        return *this;
    }

    //----------------------------------------------------------------------------
    /**
        Add or replace a free function.
    */
    template <class ReturnType, class... Params>
    Namespace& addFunction(char const* name, ReturnType (*fp)(Params...))
    {
        using FnType = decltype(fp);

        assert(name != nullptr);
        assert(lua_istable(L, -1)); // Stack: namespace table (ns)

        lua_pushlightuserdata(L, reinterpret_cast<void*>(fp)); // Stack: ns, function ptr
        lua_pushcclosure(L, &detail::invoke_proxy_function<FnType>, 1); // Stack: ns, function
        rawsetfield(L, -2, name); // Stack: ns

        return *this;
    }

    //----------------------------------------------------------------------------
    /**
        Add or replace a lua_CFunction.

        @param name The function name.
        @param fp   A C-function pointer.
        @returns This namespace registration object.
    */
    Namespace& addFunction(char const* name, int (*const fp)(lua_State*))
    {
        return addCFunction(name, fp);
    }

    //----------------------------------------------------------------------------
    /**
        Add or replace a lua_CFunction.

        @param name The function name.
        @param fp   A C-function pointer.
        @returns This namespace registration object.
    */
    Namespace& addCFunction(char const* name, int (*const fp)(lua_State*))
    {
        assert(name != nullptr);
        assert(lua_istable(L, -1)); // Stack: namespace table (ns)

        lua_pushcfunction(L, fp); // Stack: ns, function
        rawsetfield(L, -2, name); // Stack: ns

        return *this;
    }

    //----------------------------------------------------------------------------
    /**
        Open a new or existing class for registrations.

        @param name The class name.
        @returns A class registration object.
    */
    template <class T>
    Class<T> beginClass(char const* name)
    {
        assertIsActive();
        return Class<T>(name, *this);
    }

    //----------------------------------------------------------------------------
    /**
        Derive a new class for registrations.
        Call deriveClass() only once.
        To continue registrations for the class later, use beginClass().

        @param name The class name.
        @returns A class registration object.
    */
    template <class Derived, class Base>
    Class<Derived> deriveClass(char const* name)
    {
        assertIsActive();
        return Class<Derived>(name, *this, detail::getStaticRegistryKey<Base>());
    }
};

//------------------------------------------------------------------------------
/**
    Retrieve the global namespace.
    It is recommended to put your namespace inside the global namespace, and
    then add your classes and functions to it, rather than adding many classes
    and functions directly to the global namespace.

    @param L A Lua state.
    @returns A namespace registration object.
*/
inline Namespace getGlobalNamespace(lua_State* L)
{
    return Namespace::getGlobalNamespace(L);
}

} // namespace luabridge
