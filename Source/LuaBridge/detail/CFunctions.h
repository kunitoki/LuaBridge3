// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// Copyright 2019, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "Config.h"
#include "Errors.h"
#include "FuncTraits.h"
#include "LuaHelpers.h"

#include <string>

namespace luabridge {
namespace detail {

//=================================================================================================
/**
 * @brief __index metamethod for a namespace or class static and non-static members.
 *
 * Retrieves functions from metatables and properties from propget tables. Looks through the class hierarchy if inheritance is present.
 */
inline int index_metamethod(lua_State* L)
{
#if LUABRIDGE_SAFE_STACK_CHECKS
    luaL_checkstack(L, 3, detail::error_lua_stack_overflow);
#endif

    assert(lua_istable(L, 1) || lua_isuserdata(L, 1)); // Stack (further not shown): table | userdata, name

    lua_getmetatable(L, 1); // Stack: class/const table (mt)
    assert(lua_istable(L, -1));

    for (;;)
    {
        lua_pushvalue(L, 2); // Stack: mt, field name
        lua_rawget(L, -2); // Stack: mt, field | nil

        if (lua_iscfunction(L, -1)) // Stack: mt, field
        {
            lua_remove(L, -2); // Stack: field
            return 1;
        }

        assert(lua_isnil(L, -1)); // Stack: mt, nil
        lua_pop(L, 1); // Stack: mt

        lua_rawgetp(L, -1, getPropgetKey()); // Stack: mt, propget table (pg)
        assert(lua_istable(L, -1));

        lua_pushvalue(L, 2); // Stack: mt, pg, field name
        lua_rawget(L, -2); // Stack: mt, pg, getter | nil
        lua_remove(L, -2); // Stack: mt, getter | nil

        if (lua_iscfunction(L, -1)) // Stack: mt, getter
        {
            lua_remove(L, -2); // Stack: getter
            lua_pushvalue(L, 1); // Stack: getter, table | userdata
            lua_call(L, 1, 1); // Stack: value
            return 1;
        }

        assert(lua_isnil(L, -1)); // Stack: mt, nil
        lua_pop(L, 1); // Stack: mt

        // It may mean that the field may be in const table and it's constness violation.
        // Don't check that, just return nil

        // Repeat the lookup in the parent metafield,
        // or return nil if the field doesn't exist.
        lua_rawgetp(L, -1, getParentKey()); // Stack: mt, parent mt | nil

        if (lua_isnil(L, -1)) // Stack: mt, nil
        {
            lua_pop(L, 1); // Stack: mt
            lua_rawgetp(L, -1, getIndexFallbackKey()); // Stack: mt, ifb (may be nil)
            lua_remove(L, -2); // Stack: ifb
            if (lua_iscfunction(L, -1))
            {
                lua_pushvalue(L, 1); // Stack: ifb, arg1
                lua_pushvalue(L, 2); // Stack: ifb, arg2
                lua_call(L, 2, 1); // Stack: ifbresult
            }
            else
            {
                lua_pop(L, 1);
                lua_pushnil(L);
            }

            return 1;
        }

        // Remove the metatable and repeat the search in the parent one.
        assert(lua_istable(L, -1)); // Stack: mt, parent mt
        lua_remove(L, -2); // Stack: parent mt
    }

    // no return
}

//=================================================================================================
/**
 * @brief __newindex metamethod for non-static members.
 *
 * Retrieves properties from propset tables.
 */
inline int newindex_metamethod(lua_State* L, bool pushSelf)
{
#if LUABRIDGE_SAFE_STACK_CHECKS
    luaL_checkstack(L, 3, detail::error_lua_stack_overflow);
#endif

    assert(lua_istable(L, 1) || lua_isuserdata(L, 1)); // Stack (further not shown): table | userdata, name, new value

    lua_getmetatable(L, 1); // Stack: metatable (mt)
    assert(lua_istable(L, -1));

    for (;;)
    {
        lua_rawgetp(L, -1, getPropsetKey()); // Stack: mt, propset table (ps) | nil

        if (lua_isnil(L, -1)) // Stack: mt, nil
        {
            lua_pop(L, 2); // Stack: -
            luaL_error(L, "No member named '%s'", lua_tostring(L, 2));
        }

        assert(lua_istable(L, -1));

        lua_pushvalue(L, 2); // Stack: mt, ps, field name
        lua_rawget(L, -2); // Stack: mt, ps, setter | nil
        lua_remove(L, -2); // Stack: mt, setter | nil

        if (lua_iscfunction(L, -1)) // Stack: mt, setter
        {
            lua_remove(L, -2); // Stack: setter
            if (pushSelf)
                lua_pushvalue(L, 1); // Stack: setter, table | userdata
            lua_pushvalue(L, 3); // Stack: setter, table | userdata, new value
            lua_call(L, pushSelf ? 2 : 1, 0); // Stack: -
            return 0;
        }

        assert(lua_isnil(L, -1)); // Stack: mt, nil
        lua_pop(L, 1); // Stack: mt

        lua_rawgetp(L, -1, getParentKey()); // Stack: mt, parent mt | nil

        if (lua_isnil(L, -1)) // Stack: mt, nil
        {
            lua_pop(L, 1); // Stack: mt
            lua_rawgetp(L, -1, getNewIndexFallbackKey()); // Stack: mt, nifb (may be nil)
            if (lua_iscfunction(L, -1))
            {
                lua_pushvalue(L, 1); // stack: nifb, arg1
                lua_pushvalue(L, 2); // stack: nifb, arg2
                lua_pushvalue(L, 3); // stack: nifb, arg3
                lua_call(L, 3, 1); // stack: nifbresult
                return 0;
            }

            lua_pop(L, 1); // Stack: mt
            lua_pop(L, 1); // Stack: -
            luaL_error(L, "No writable member '%s'", lua_tostring(L, 2));
            return 0;
        }

        assert(lua_istable(L, -1)); // Stack: mt, parent mt
        lua_remove(L, -2); // Stack: parent mt

        // Repeat the search in the parent
    }

    return 0;
}

//=================================================================================================
/**
 * @brief __newindex metamethod for objects.
 */
inline int newindex_object_metamethod(lua_State* L)
{
    return newindex_metamethod(L, true);
}

//=================================================================================================
/**
 * @brief __newindex metamethod for namespace or class static members.
 *
 * Retrieves properties from propset tables.
 */
inline int newindex_static_metamethod(lua_State* L)
{
    return newindex_metamethod(L, false);
}

//=================================================================================================
/**
 * @brief lua_CFunction to report an error writing to a read-only value.
 *
 * The name of the variable is in the first upvalue.
 */
inline int read_only_error(lua_State* L)
{
    std::string s;

    s = s + "'" + lua_tostring(L, lua_upvalueindex(1)) + "' is read-only";

    luaL_error(L, "%s", s.c_str());

    return 0;
}

//=================================================================================================
/**
 * @brief __gc metamethod for a class.
 */
template <class C>
static int gc_metamethod(lua_State* L)
{
    Userdata* ud = Userdata::getExact<C>(L, 1);
    assert(ud);

    ud->~Userdata();

    return 0;
}

//=================================================================================================

template <class T, class C = void>
struct property_getter;

/**
 * @brief lua_CFunction to get a variable.
 *
 * This is used for global variables or class static data members. The pointer to the data is in the first upvalue.
 */
template <class T>
struct property_getter<T, void>
{
    static int call(lua_State* L)
    {
        assert(lua_islightuserdata(L, lua_upvalueindex(1)));

        T* ptr = static_cast<T*>(lua_touserdata(L, lua_upvalueindex(1)));
        assert(ptr != nullptr);

        auto result = Stack<T&>::push(L, *ptr);
        if (! result)
            raise_lua_error(L, "%s", result.message().c_str());

        return 1;
    }
};

#if 0
template <class T>
struct property_getter<std::reference_wrapper<T>, void>
{
    static int call(lua_State* L)
    {
        assert(lua_islightuserdata(L, lua_upvalueindex(1)));

        std::reference_wrapper<T>* ptr = static_cast<std::reference_wrapper<T>*>(lua_touserdata(L, lua_upvalueindex(1)));
        assert(ptr != nullptr);

        auto result = Stack<T&>::push(L, ptr->get());
        if (! result)
            luaL_error(L, "%s", result.message().c_str());

        return 1;
    }
};
#endif

/**
 * @brief lua_CFunction to get a class data member.
 *
 * The pointer-to-member is in the first upvalue. The class userdata object is at the top of the Lua stack.
 */
template <class T, class C>
struct property_getter
{
    static int call(lua_State* L)
    {
        C* c = Userdata::get<C>(L, 1, true);

        T C::** mp = static_cast<T C::**>(lua_touserdata(L, lua_upvalueindex(1)));

        Result result;

#if LUABRIDGE_HAS_EXCEPTIONS
        try
        {
#endif
            result = Stack<T&>::push(L, c->**mp);

#if LUABRIDGE_HAS_EXCEPTIONS
        }
        catch (const std::exception& e)
        {
            raise_lua_error(L, "%s", e.what());
        }
#endif

        if (! result)
            raise_lua_error(L, "%s", result.message().c_str());

        return 1;
    }
};

/**
 * @brief Helper function to push a property getter on a table at a specific index.
 */
inline void add_property_getter(lua_State* L, const char* name, int tableIndex)
{
#if LUABRIDGE_SAFE_STACK_CHECKS
    luaL_checkstack(L, 2, detail::error_lua_stack_overflow);
#endif

    assert(name != nullptr);
    assert(lua_istable(L, tableIndex));
    assert(lua_iscfunction(L, -1)); // Stack: getter

    lua_rawgetp(L, tableIndex, getPropgetKey()); // Stack: getter, propget table (pg)
    lua_pushvalue(L, -2); // Stack: getter, pg, getter
    rawsetfield(L, -2, name); // Stack: getter, pg
    lua_pop(L, 2); // Stack: -
}

//=================================================================================================

template <class T, class C = void>
struct property_setter;

/**
 * @brief lua_CFunction to set a variable.
 *
 * This is used for global variables or class static data members. The pointer to the data is in the first upvalue.
 */
template <class T>
struct property_setter<T, void>
{
    static int call(lua_State* L)
    {
        assert(lua_islightuserdata(L, lua_upvalueindex(1)));

        T* ptr = static_cast<T*>(lua_touserdata(L, lua_upvalueindex(1)));
        assert(ptr != nullptr);

        auto result = Stack<T>::get(L, 1);
        if (! result)
            raise_lua_error(L, "%s", result.error().message().c_str());

        *ptr = *result;

        return 0;
    }
};

#if 0
template <class T>
struct property_setter<std::reference_wrapper<T>, void>
{
    static int call(lua_State* L)
    {
        assert(lua_islightuserdata(L, lua_upvalueindex(1)));

        std::reference_wrapper<T>* ptr = static_cast<std::reference_wrapper<T>*>(lua_touserdata(L, lua_upvalueindex(1)));
        assert(ptr != nullptr);

        ptr->get() = Stack<T>::get(L, 1);

        return 0;
    }
};
#endif

/**
 * @brief lua_CFunction to set a class data member.
 *
 * The pointer-to-member is in the first upvalue. The class userdata object is at the top of the Lua stack.
 */
template <class T, class C>
struct property_setter
{
    static int call(lua_State* L)
    {
        C* c = Userdata::get<C>(L, 1, false);

        T C::** mp = static_cast<T C::**>(lua_touserdata(L, lua_upvalueindex(1)));

#if LUABRIDGE_HAS_EXCEPTIONS
        try
        {
#endif
            auto result = Stack<T>::get(L, 2);
            if (! result)
                raise_lua_error(L, "%s", result.error().message().c_str());

            c->** mp = *result;

#if LUABRIDGE_HAS_EXCEPTIONS
        }
        catch (const std::exception& e)
        {
            raise_lua_error(L, "%s", e.what());
        }
#endif

        return 0;
    }
};

/**
 * @brief Helper function to push a property setter on a table at a specific index.
 */
inline void add_property_setter(lua_State* L, const char* name, int tableIndex)
{
#if LUABRIDGE_SAFE_STACK_CHECKS
    luaL_checkstack(L, 2, detail::error_lua_stack_overflow);
#endif

    assert(name != nullptr);
    assert(lua_istable(L, tableIndex));
    assert(lua_iscfunction(L, -1)); // Stack: setter

    lua_rawgetp(L, tableIndex, getPropsetKey()); // Stack: setter, propset table (ps)
    lua_pushvalue(L, -2); // Stack: setter, ps, setter
    rawsetfield(L, -2, name); // Stack: setter, ps
    lua_pop(L, 2); // Stack: -
}

//=================================================================================================
/**
 * @brief lua_CFunction to call a class member function with a return value.
 *
 * The member function pointer is in the first upvalue. The class userdata object is at the top of the Lua stack.
 */
template <class F, class T>
int invoke_member_function(lua_State* L)
{
    using FnTraits = detail::function_traits<F>;

    assert(isfulluserdata(L, lua_upvalueindex(1)));

    T* ptr = Userdata::get<T>(L, 1, false);

    const F& func = *static_cast<const F*>(lua_touserdata(L, lua_upvalueindex(1)));
    assert(func != nullptr);

    return function<typename FnTraits::result_type, typename FnTraits::argument_types, 2>::call(L, ptr, func);
}

template <class F, class T>
int invoke_const_member_function(lua_State* L)
{
    using FnTraits = detail::function_traits<F>;

    assert(isfulluserdata(L, lua_upvalueindex(1)));

    const T* ptr = Userdata::get<T>(L, 1, true);

    const F& func = *static_cast<const F*>(lua_touserdata(L, lua_upvalueindex(1)));
    assert(func != nullptr);

    return function<typename FnTraits::result_type, typename FnTraits::argument_types, 2>::call(L, ptr, func);
}

//=================================================================================================
/**
 * @brief lua_CFunction to call a class member lua_CFunction.
 *
 * The member function pointer is in the first upvalue. The object userdata ('this') value is at top ot the Lua stack.
 */
template <class T>
int invoke_member_cfunction(lua_State* L)
{
    using F = int (T::*)(lua_State * L);

    assert(isfulluserdata(L, lua_upvalueindex(1)));

    T* t = Userdata::get<T>(L, 1, false);

    const F& func = *static_cast<const F*>(lua_touserdata(L, lua_upvalueindex(1)));
    assert(func != nullptr);

    return (t->*func)(L);
}

template <class T>
int invoke_const_member_cfunction(lua_State* L)
{
    using F = int (T::*)(lua_State * L) const;

    assert(isfulluserdata(L, lua_upvalueindex(1)));

    const T* t = Userdata::get<T>(L, 1, true);

    const F& func = *static_cast<const F*>(lua_touserdata(L, lua_upvalueindex(1)));
    assert(func != nullptr);

    return (t->*func)(L);
}

//=================================================================================================
/**
 * @brief lua_CFunction to call on a object via function pointer.
 *
 * The proxy function pointer (lightuserdata) is in the first upvalue. The class userdata object is at the top of the Lua stack.
 */
template <class F>
int invoke_proxy_function(lua_State* L)
{
    using FnTraits = detail::function_traits<F>;

    assert(lua_islightuserdata(L, lua_upvalueindex(1)));

    auto func = reinterpret_cast<F>(lua_touserdata(L, lua_upvalueindex(1)));
    assert(func != nullptr);

    return function<typename FnTraits::result_type, typename FnTraits::argument_types, 1>::call(L, func);
}

//=================================================================================================
/**
 * @brief lua_CFunction to call on a object via functor (lambda wrapped in a std::function).
 *
 * The proxy std::function (lightuserdata) is in the first upvalue. The class userdata object is at the top of the Lua stack.
 */
template <class F>
int invoke_proxy_functor(lua_State* L)
{
    using FnTraits = detail::function_traits<F>;

    assert(isfulluserdata(L, lua_upvalueindex(1)));

    auto& func = *align<F>(lua_touserdata(L, lua_upvalueindex(1)));

    return function<typename FnTraits::result_type, typename FnTraits::argument_types, 1>::call(L, func);
}

//=================================================================================================

inline int try_overload_functions(lua_State* L)
{
    int nargs = lua_gettop(L);

    // get the list of overloads
    lua_pushvalue(L, lua_upvalueindex(1));
    assert(lua_istable(L, -1));
    const int idx_overloads = nargs + 1;
    const int num_overloads = get_length(L, idx_overloads);

    // create table to hold error messages
    lua_createtable(L, num_overloads, 0);
    const int idx_errors = nargs + 2;
    int nerrors = 0;

    // iterate through table, snippet taken from Lua docs
    lua_pushnil(L);  // first key
    while (lua_next(L, idx_overloads) != 0)
    {
        assert(lua_istable(L, -1));

        // check matching arity
        lua_rawgeti(L, -1, 1);
        assert(lua_isnumber(L, -1));

        const int overload_arity = lua_tointeger(L, -1);
        if (overload_arity != nargs)
        {
            // store error message and try next overload
            lua_pushfstring(L, "Skipped overload #%d with unmatched arity of %d instead of %d", nerrors, overload_arity, nargs);
            lua_rawseti(L, idx_errors, ++nerrors);

            lua_pop(L, 2); // pop arity, value (table)
            continue;
        }

        lua_pop(L, 1); // pop arity

        // push function
        lua_pushnumber(L, 2);
        lua_gettable(L, -2);
        assert(lua_isfunction(L, -1));

        // push arguments
        for (int i = 1; i <= nargs; ++i)
            lua_pushvalue(L, i);

        // call f, this pops the function and its args, pushes result(s)
        const int err = lua_pcall(L, nargs, LUA_MULTRET, 0);
        if (err == LUABRIDGE_LUA_OK)
        {
            // calculate number of return values and return
            const int nresults = lua_gettop(L) - nargs - 4; // 4: overloads, errors, key, table
            return nresults;
        }
        else if (err == LUA_ERRRUN)
        {
            // store error message and try next overload
            lua_rawseti(L, idx_errors, ++nerrors);
        }
        else
        {
            return lua_error(L);  // critical error: rethrow
        }

        lua_pop(L, 1); // pop value (table)
    }

    lua_Debug debug;
    lua_getstack(L, 0, &debug);
    lua_getinfo(L, "n", &debug);
    lua_pushfstring(L, "All %d overloads of %s returned an error:", nerrors, debug.name);

    // Concatenate error messages of each overload
    for (int i = 1; i <= nerrors; ++i)
    {
        lua_pushfstring(L, "\n%d: ", i);
        lua_rawgeti(L, idx_errors, i);
    }
    lua_concat(L, nerrors * 2 + 1);

    return lua_error(L); // throw error message just built
}

} // namespace detail
} // namespace luabridge
