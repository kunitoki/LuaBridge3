// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// Copyright 2019, George Tokmaji
// Copyright 2018, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2008, Nigel Atkinson <suprapilot+LuaCode@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "Config.h"
#include "Errors.h"
#include "Stack.h"

#include <iostream>
#include <exception>
#include <map>
#include <string>
#include <optional>
#include <vector>

namespace luabridge {

class LuaResult;

//=================================================================================================
/**
 * @brief Type tag for representing LUA_TNIL.
 *
 * Construct one of these using `LuaNil ()` to represent a Lua nil. This is faster than creating a reference in the registry to nil.
 * Example:
 *
 * @code
 *     LuaRef t (LuaRef::createTable (L));
 *     ...
 *     t ["k"] = LuaNil (); // assign nil
 * @endcode
 */
struct LuaNil
{
};

/**
 * @brief Stack specialization for LuaNil.
 */
template <>
struct Stack<LuaNil>
{
    static bool push(lua_State* L, const LuaNil&, std::error_code&)
    {
        lua_pushnil(L);
        return true;
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_type(L, index) == LUA_TNIL;
    }
};

//=================================================================================================
/**
 * @brief Base class for Lua variables and table item reference classes.
 */
template <class Impl, class LuaRef>
class LuaRefBase
{
protected:
    //=============================================================================================
    /**
     * @brief Pop the Lua stack.
     *
     * Pops the specified number of stack items on destruction. We use this when returning objects, to avoid an explicit temporary
     * variable, since the destructor executes after the return statement. For example:
     *
     * @code
     *     template <class U>
     *     U cast (lua_State* L)
     *     {
     *         StackPop p (L, 1);
     *         ...
     *         return U (); // Destructor called after this line
     *     }
     * @endcode
     *
     * @note The `StackPop` object must always be a named local variable.
     */
    class StackPop
    {
    public:
        /**
         * @brief Create a StackPop object.
         *
         * @param L  A Lua state.
         * @param count The number of stack entries to pop on destruction.
         */
        StackPop(lua_State* L, int count)
            : m_L(L)
            , m_count(count) {
        }

        /**
         * @brief Destroy a StackPop object.
         *
         * In case an exception is in flight before the destructor is called, stack is potentially cleared by lua. So we never pop more than
         * the actual size of the stack.
         */
        ~StackPop()
        {
            const int stackSize = lua_gettop(m_L);

            lua_pop(m_L, stackSize < m_count ? stackSize : m_count);
        }

        /**
         * @brief Set a new number to pop.
         *
         * @param newCount The new number of stack entries to pop on destruction.
         */
        void popCount(int newCount)
        {
            m_count = newCount;
        }

    private:
        lua_State* m_L = nullptr;
        int m_count = 0;
    };

    friend struct Stack<LuaRef>;

    //=============================================================================================
    /**
     * @brief Type tag for stack construction.
     */
    struct FromStack
    {
    };

    LuaRefBase(lua_State* L)
        : m_L(L)
    {
    }

    //=============================================================================================
    /**
     * @brief Create a reference to this reference.
     *
     * @returns An index in the Lua registry.
     */
    int createRef() const
    {
        impl().push();

        return luaL_ref(m_L, LUA_REGISTRYINDEX);
    }

public:
    //=============================================================================================
    /**
     * @brief Convert to a string using lua_tostring function.
     *
     * @returns A string representation of the referred Lua value.
     */
    std::string tostring() const
    {
        StackPop p(m_L, 1);
        
        lua_getglobal(m_L, "tostring");

        impl().push();

        lua_call(m_L, 1, 1);

        const char* str = lua_tostring(m_L, -1);
        return str != nullptr ? str : "";
    }

    //=============================================================================================
    /**
     * @brief Print a text description of the value to a stream.
     *
     * This is used for diagnostics.
     *
     * @param os An output stream.
     */
    void print(std::ostream& os) const
    {
        switch (type())
        {
        case LUA_TNIL:
            os << "nil";
            break;

        case LUA_TNUMBER:
            os << cast<lua_Number>();
            break;

        case LUA_TBOOLEAN:
            os << (cast<bool>() ? "true" : "false");
            break;

        case LUA_TSTRING:
            os << '"' << cast<const char*>() << '"';
            break;

        case LUA_TTABLE:
            os << "table: " << tostring();
            break;

        case LUA_TFUNCTION:
            os << "function: " << tostring();
            break;

        case LUA_TUSERDATA:
            os << "userdata: " << tostring();
            break;

        case LUA_TTHREAD:
            os << "thread: " << tostring();
            break;

        case LUA_TLIGHTUSERDATA:
            os << "lightuserdata: " << tostring();
            break;

        default:
            os << "unknown";
            break;
        }
    }

    //=============================================================================================
    /**
     * @brief Insert a Lua value or table item reference to a stream.
     *
     * @param os  An output stream.
     * @param ref A Lua reference.
     *
     * @returns The output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const LuaRefBase& ref)
    {
        ref.print(os);
        return os;
    }

    //=============================================================================================
    /**
     * @brief Retrieve the lua_State associated with the reference.
     *
     * @returns A Lua state.
     */
    lua_State* state() const
    {
        return m_L;
    }

    //=============================================================================================
    /**
     * @brief Place the object onto the Lua stack.
     *
     * @param L A Lua state.
     */
    void push(lua_State* L) const
    {
        assert(equalstates(L, m_L));
        (void) L;

        impl().push();
    }

    //=============================================================================================
    /**
     * @brief Pop the top of Lua stack and assign it to the reference.
     *
     * @param L A Lua state.
     */
    void pop(lua_State* L)
    {
        assert(equalstates(L, m_L));
        (void) L;

        impl().pop();
    }

    //=============================================================================================
    /**
     * @brief Return the Lua type of the referred value.
     *
     * This invokes lua_type().
     *
     * @returns The type of the referred value.
     *
     * @see lua_type()
     */
    int type() const
    {
        StackPop p(m_L, 1);

        impl().push();

        const int refType = lua_type(m_L, -1);
        
        return refType;
    }

    /**
     * @brief Indicate whether it is a nil reference.
     *
     * @returns True if this is a nil reference, false otherwise.
     */
    bool isNil() const { return type() == LUA_TNIL; }

    /**
     * @brief Indicate whether it is a reference to a boolean.
     *
     * @returns True if it is a reference to a boolean, false otherwise.
     */
    bool isBool() const { return type() == LUA_TBOOLEAN; }

    /**
     * @brief Indicate whether it is a reference to a number.
     *
     * @returns True if it is a reference to a number, false otherwise.
     */
    bool isNumber() const { return type() == LUA_TNUMBER; }

    /**
     * @brief Indicate whether it is a reference to a string.
     *
     * @returns True if it is a reference to a string, false otherwise.
     */
    bool isString() const { return type() == LUA_TSTRING; }

    /**
     * @brief Indicate whether it is a reference to a table.
     *
     * @returns True if it is a reference to a table, false otherwise.
     */
    bool isTable() const { return type() == LUA_TTABLE; }

    /**
     * @brief Indicate whether it is a reference to a function.
     *
     * @returns True if it is a reference to a function, false otherwise.
     */
    bool isFunction() const { return type() == LUA_TFUNCTION; }

    /**
     * @brief Indicate whether it is a reference to a full userdata.
     *
     * @returns True if it is a reference to a full userdata, false otherwise.
     */
    bool isUserdata() const { return type() == LUA_TUSERDATA; }

    /**
     * @brief Indicate whether it is a reference to a lua thread (coroutine).
     *
     * @returns True if it is a reference to a lua thread, false otherwise.
     */
    bool isThread() const { return type() == LUA_TTHREAD; }

    /**
     * @brief Indicate whether it is a reference to a light userdata.
     *
     * @returns True if it is a reference to a light userdata, false otherwise.
     */
    bool isLightUserdata() const { return type() == LUA_TLIGHTUSERDATA; }

    /**
     * @brief Indicate whether it is a callable.
     *
     * @returns True if it is a callable, false otherwise.
     */
    bool isCallable() const
    {
        if (isFunction())
            return true;

        auto metatable = getMetatable();
        return metatable.isTable() && metatable["__call"].isFunction();
    }

    //=============================================================================================
    /**
     * @brief Perform an explicit conversion to the type T.
     *
     * @returns A value of the type T converted from this reference.
     */
    template <class T>
    T cast() const
    {
        StackPop p(m_L, 1);

        impl().push();

        return Stack<T>::get(m_L, -1);
    }

    //=============================================================================================
    /**
     * @brief Indicate if this reference is convertible to the type T.
     *
     * @returns True if the referred value is convertible to the type T, false otherwise.
     */
    template <class T>
    bool isInstance() const
    {
        StackPop p(m_L, 1);

        impl().push();

        return Stack<T>::isInstance(m_L, -1);
    }

    //=============================================================================================
    /**
     * @brief Type cast operator.
     *
     * @returns A value of the type T converted from this reference.
     */
    template <class T>
    operator T() const
    {
        return cast<T>();
    }

    //=============================================================================================
    /**
     * @brief Get the metatable for the LuaRef.
     *
     * @returns A LuaRef holding the metatable of the lua object.
     */
    LuaRef getMetatable() const
    {
        if (isNil())
            return LuaRef(m_L);

        StackPop p(m_L, 2);

        impl().push();

        if (! lua_getmetatable(m_L, -1))
        {
            p.popCount(1);
            return LuaRef(m_L);
        }

        return LuaRef::fromStack(m_L);
    }

    //=============================================================================================
    /**
     * @brief Compare this reference with a specified value using lua_compare().
     *
     * This invokes metamethods.
     *
     * @param rhs A value to compare with.
     *
     * @returns True if the referred value is equal to the specified one.
     */
    template <class T>
    bool operator==(T rhs) const
    {
        StackPop p(m_L, 2);

        impl().push();

        std::error_code ec;
        if (! Stack<T>::push(m_L, rhs, ec))
        {
            p.popCount(1);
            return false;
        }

        return lua_compare(m_L, -2, -1, LUA_OPEQ) == 1;
    }

    /**
     * @brief Compare this reference with a specified value using lua_compare().
     *
     * This invokes metamethods.
     *
     * @param rhs A value to compare with.
     *
     * @returns True if the referred value is not equal to the specified one.
     */
    template <class T>
    bool operator!=(T rhs) const
    {
        return !(*this == rhs);
    }

    /**
     * @brief Compare this reference with a specified value using lua_compare().
     *
     * This invokes metamethods.
     *
     * @param rhs A value to compare with.
     *
     * @returns True if the referred value is less than the specified one.
     */
    template <class T>
    bool operator<(T rhs) const
    {
        StackPop p(m_L, 2);

        impl().push();

        std::error_code ec;
        if (! Stack<T>::push(m_L, rhs, ec))
        {
            p.popCount(1);
            return false;
        }

        const int lhsType = lua_type(m_L, -2);
        const int rhsType = lua_type(m_L, -1);
        if (lhsType != rhsType)
            return lhsType < rhsType;

        return lua_compare(m_L, -2, -1, LUA_OPLT) == 1;
    }

    /**
     * @brief Compare this reference with a specified value using lua_compare().
     *
     * This invokes metamethods.
     *
     * @param rhs A value to compare with.
     *
     * @returns True if the referred value is less than or equal to the specified one.
     */
    template <class T>
    bool operator<=(T rhs) const
    {
        StackPop p(m_L, 2);

        impl().push();

        std::error_code ec;
        if (! Stack<T>::push(m_L, rhs, ec))
        {
            p.popCount(1);
            return false;
        }

        const int lhsType = lua_type(m_L, -2);
        const int rhsType = lua_type(m_L, -1);
        if (lhsType != rhsType)
            return lhsType <= rhsType;

        return lua_compare(m_L, -2, -1, LUA_OPLE) == 1;
    }

    /**
     * @brief Compare this reference with a specified value using lua_compare().
     *
     * This invokes metamethods.
     *
     * @param rhs A value to compare with.
     *
     * @returns True if the referred value is greater than the specified one.
     */
    template <class T>
    bool operator>(T rhs) const
    {
        StackPop p(m_L, 2);

        impl().push();

        std::error_code ec;
        if (! Stack<T>::push(m_L, rhs, ec))
        {
            p.popCount(1);
            return false;
        }

        const int lhsType = lua_type(m_L, -2);
        const int rhsType = lua_type(m_L, -1);
        if (lhsType != rhsType)
            return lhsType > rhsType;

        return lua_compare(m_L, -1, -2, LUA_OPLT) == 1;
    }

    /**
     * @brief Compare this reference with a specified value using lua_compare().
     *
     * This invokes metamethods.
     *
     * @param rhs A value to compare with.
     *
     * @returns True if the referred value is greater than or equal to the specified one.
     */
    template <class T>
    bool operator>=(T rhs) const
    {
        StackPop p(m_L, 2);

        impl().push();

        std::error_code ec;
        if (! Stack<T>::push(m_L, rhs, ec))
        {
            p.popCount(1);
            return false;
        }

        const int lhsType = lua_type(m_L, -2);
        const int rhsType = lua_type(m_L, -1);
        if (lhsType != rhsType)
            return lhsType >= rhsType;

        return lua_compare(m_L, -1, -2, LUA_OPLE) == 1;
    }

    /**
     * @brief Compare this reference with a specified value using lua_compare().
     *
     * This does not invoke metamethods.
     *
     * @param rhs A value to compare with.
     *
     * @returns True if the referred value is equal to the specified one.
     */
    template <class T>
    bool rawequal(T v) const
    {
        StackPop p(m_L, 2);

        impl().push();

        std::error_code ec;
        if (! Stack<T>::push(m_L, v, ec))
        {
            p.popCount(1);
            return false;
        }

        return lua_rawequal(m_L, -1, -2) == 1;
    }

    //=============================================================================================
    /**
     * @brief Append a value to a referred table.
     *
     * If the table is a sequence this will add another element to it.
     *
     * @param v A value to append to the table.
     */
    template <class T>
    void append(T v) const
    {
        StackPop p(m_L, 1);

        impl().push();

        std::error_code ec;
        if (! Stack<T>::push(m_L, v, ec))
            return;

        luaL_ref(m_L, -2);
    }

    //=============================================================================================
    /**
     * @brief Return the length of a referred array.
     *
     * This is identical to applying the Lua # operator.
     *
     * @returns The length of the referred array.
     */
    int length() const
    {
        StackPop p(m_L, 1);

        impl().push();

        return get_length(m_L, -1);
    }

    //=============================================================================================
    /**
     * @brief Call Lua code.
     *
     * The return value is provided as a LuaRef (which may be LUA_REFNIL).
     *
     * If an error occurs, a LuaException is thrown (only if exceptions are enabled).
     *
     * @returns A result of the call.
     */
    template <class... Args>
    LuaResult operator()(Args&&... args) const;

protected:
    lua_State* m_L = nullptr;

private:
    const Impl& impl() const { return static_cast<const Impl&>(*this); }

    Impl& impl() { return static_cast<Impl&>(*this); }
};

//=================================================================================================
/**
 * @brief Lightweight reference to a Lua object.
 *
 * The reference is maintained for the lifetime of the C++ object.
 */
class LuaRef : public LuaRefBase<LuaRef, LuaRef>
{
    //=============================================================================================
    /**
     * @brief A proxy for representing table values.
     */
    class TableItem : public LuaRefBase<TableItem, LuaRef>
    {
        friend class LuaRef;

    public:
        //=========================================================================================
        /**
         * @brief Construct a TableItem from a table value.
         *
         * The table is in the registry, and the key is at the top of the stack.
         * The key is popped off the stack.
         *
         * @param L A lua state.
         * @param tableRef The index of a table in the Lua registry.
         */
        TableItem(lua_State* L, int tableRef)
            : LuaRefBase(L)
            , m_tableRef(LUA_NOREF)
            , m_keyRef(luaL_ref(L, LUA_REGISTRYINDEX))
        {
            lua_rawgeti(m_L, LUA_REGISTRYINDEX, tableRef);
            m_tableRef = luaL_ref(L, LUA_REGISTRYINDEX);
        }

        //=========================================================================================
        /**
         * @brief Create a TableItem via copy constructor.
         *
         * It is best to avoid code paths that invoke this, because it creates an extra temporary Lua reference. Typically this is done by
         * passing the TableItem parameter as a `const` reference.
         *
         * @param other Another Lua table item reference.
         */
        TableItem(const TableItem& other)
            : LuaRefBase(other.m_L)
            , m_tableRef(LUA_NOREF)
            , m_keyRef(LUA_NOREF)
        {
            lua_rawgeti(m_L, LUA_REGISTRYINDEX, other.m_tableRef);
            m_tableRef = luaL_ref(m_L, LUA_REGISTRYINDEX);

            lua_rawgeti(m_L, LUA_REGISTRYINDEX, other.m_keyRef);
            m_keyRef = luaL_ref(m_L, LUA_REGISTRYINDEX);
        }

        //=========================================================================================
        /**
         * @brief Destroy the proxy.
         *
         * This does not destroy the table value.
         */
        ~TableItem()
        {
            if (m_keyRef != LUA_NOREF)
                luaL_unref(m_L, LUA_REGISTRYINDEX, m_keyRef);
            
            if (m_tableRef != LUA_NOREF)
                luaL_unref(m_L, LUA_REGISTRYINDEX, m_tableRef);
        }

        //=========================================================================================
        /**
         * @brief Assign a new value to this table key.
         *
         * This may invoke metamethods.
         *
         * @tparam T The type of a value to assing.
         *
         * @param v A value to assign.
         *
         * @returns This reference.
         */
        template <class T>
        TableItem& operator=(T v)
        {
            StackPop p(m_L, 1);
            lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_tableRef);
            lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_keyRef);

            std::error_code ec;
            if (! Stack<T>::push(m_L, v, ec))
                return *this;

            lua_settable(m_L, -3);
            return *this;
        }

        //=========================================================================================
        /**
         * @brief Assign a new value to this table key.
         *
         * The assignment is raw, no metamethods are invoked.
         *
         * @tparam T The type of a value to assing.
         *
         * @param v A value to assign.
         *
         * @returns This reference.
         */
        template <class T>
        TableItem& rawset(T v)
        {
            StackPop p(m_L, 1);
            lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_tableRef);
            lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_keyRef);

            std::error_code ec;
            if (! Stack<T>::push(m_L, v, ec))
                return *this;

            lua_rawset(m_L, -3);
            return *this;
        }

        //=========================================================================================
        /**
         * @brief Push the value onto the Lua stack.
         */
        using LuaRefBase::push;

        void push() const
        {
            lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_tableRef);
            lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_keyRef);
            lua_gettable(m_L, -2);
            lua_remove(m_L, -2); // remove the table
        }

        //=========================================================================================
        /**
         * @brief Access a table value using a key.
         *
         * This invokes metamethods.
         *
         * @tparam T The type of a key.
         *
         * @param key A key value.
         *
         * @returns A Lua table item reference.
         */
        template <class T>
        TableItem operator[](T key) const
        {
            return LuaRef(*this)[key];
        }

        //=========================================================================================
        /**
         * @brief Access a table value using a key.
         *
         * The operation is raw, metamethods are not invoked. The result is passed by value and may not be modified.
         *
         * @tparam T The type of a key.
         *
         * @param key A key value.
         *
         * @returns A Lua value reference.
         */
        template <class T>
        LuaRef rawget(T key) const
        {
            return LuaRef(*this).rawget(key);
        }

    private:
        int m_tableRef;
        int m_keyRef;
    };

    friend struct Stack<TableItem>;
    friend struct Stack<TableItem&>;

    //=========================================================================================
    /**
     * @brief Create a reference to an object at the top of the Lua stack and pop it.
     *
     * This constructor is private and not invoked directly. Instead, use the `fromStack` function.
     *
     * @param L A Lua state.
     *
     * @note The object is popped.
     */
    LuaRef(lua_State* L, FromStack)
        : LuaRefBase(L)
        , m_ref(luaL_ref(m_L, LUA_REGISTRYINDEX))
    {
    }

    //=========================================================================================
    /**
     * @brief Create a reference to an object on the Lua stack.
     *
     * This constructor is private and not invoked directly. Instead, use the `fromStack` function.
     *
     * @param L A Lua state.
     *
     * @param index The index of the value on the Lua stack.
     *
     * @note The object is not popped.
     */
    LuaRef(lua_State* L, int index, FromStack)
        : LuaRefBase(L)
        , m_ref(LUA_NOREF)
    {
        lua_pushvalue(m_L, index);
        m_ref = luaL_ref(m_L, LUA_REGISTRYINDEX);
    }

public:
    //=============================================================================================
    /**
     * @brief Create an invalid reference that will be treated as nil.
     *
     * The Lua reference may be assigned later.
     *
     * @param L A Lua state.
     */
    LuaRef(lua_State* L)
        : LuaRefBase(L)
        , m_ref(LUA_NOREF)
    {
    }

    //=============================================================================================
    /**
     * @brief Push a value onto a Lua stack and return a reference to it.
     *
     * @param L A Lua state.
     * @param v A value to push.
     */
    template <class T>
    LuaRef(lua_State* L, T v)
        : LuaRefBase(L)
        , m_ref(LUA_NOREF)
    {
        std::error_code ec;
        if (! Stack<T>::push(m_L, v, ec))
            return;

        m_ref = luaL_ref(m_L, LUA_REGISTRYINDEX);
    }

    //=============================================================================================
    /**
     * @brief Create a reference to a table item.
     *
     * @param v A table item reference.
     */
    LuaRef(const TableItem& v)
        : LuaRefBase(v.state())
        , m_ref(v.createRef())
    {
    }

    //=============================================================================================
    /**
     * @brief Create a new reference to an existing Lua value.
     *
     * @param other An existing reference.
     */
    LuaRef(const LuaRef& other)
        : LuaRefBase(other.m_L)
        , m_ref(other.createRef())
    {
    }

    //=============================================================================================
    /**
     * @brief Destroy a reference.
     *
     * The corresponding Lua registry reference will be released.
     *
     * @note If the state refers to a thread, it is the responsibility of the caller to ensure that the thread still exists when the LuaRef is destroyed.
     */
    ~LuaRef()
    {
        if (m_ref != LUA_NOREF)
            luaL_unref(m_L, LUA_REGISTRYINDEX, m_ref);
    }

    //=============================================================================================
    /**
     * @brief Return a reference to a top Lua stack item.
     *
     * The stack item is not popped.
     *
     * @param L A Lua state.
     *
     * @returns A reference to a value on the top of a Lua stack.
     */
    static LuaRef fromStack(lua_State* L)
    {
        return LuaRef(L, FromStack());
    }

    //=============================================================================================
    /**
     * @brief Return a reference to a Lua stack item with a specified index.
     *
     * The stack item is not removed.
     *
     * @param L     A Lua state.
     * @param index An index in the Lua stack.
     *
     * @returns A reference to a value in a Lua stack.
     */
    static LuaRef fromStack(lua_State* L, int index)
    {
        lua_pushvalue(L, index);
        return LuaRef(L, FromStack());
    }

    //=============================================================================================
    /**
     * @brief Create a new empty table on the top of a Lua stack and return a reference to it.
     *
     * @param L A Lua state.
     *
     * @returns A reference to the newly created table.
     *
     * @see luabridge::newTable()
     */
    static LuaRef newTable(lua_State* L)
    {
        lua_newtable(L);
        return LuaRef(L, FromStack());
    }

    //=============================================================================================
    /**
     * @brief Return a reference to a named global Lua variable.
     *
     * @param L    A Lua state.
     * @param name The name of a global variable.
     *
     * @returns A reference to the Lua variable.
     *
     * @see luabridge::getGlobal()
     */
    static LuaRef getGlobal(lua_State* L, const char* name)
    {
        lua_getglobal(L, name);
        return LuaRef(L, FromStack());
    }

    //=============================================================================================
    /**
     * @brief Indicate whether it is an invalid reference.
     *
     * @returns True if this is an invalid reference, false otherwise.
     */
    bool isValid() const { return m_ref != LUA_NOREF; }

    //=============================================================================================
    /**
     * @brief Assign another LuaRef to this LuaRef.
     *
     * @param rhs A reference to assign from.
     *
     * @returns This reference.
     */
    LuaRef& operator=(const LuaRef& rhs)
    {
        LuaRef ref(rhs);
        swap(ref);
        return *this;
    }

    //=============================================================================================
    /**
     * @brief Assign a table item reference.
     *
     * @param rhs A table item reference.
     *
     * @returns This reference.
     */
    LuaRef& operator=(const LuaRef::TableItem& rhs)
    {
        LuaRef ref(rhs);
        swap(ref);
        return *this;
    }

    //=============================================================================================
    /**
     * @brief Assign nil to this reference.
     *
     * @returns This reference.
     */
    LuaRef& operator=(const LuaNil&)
    {
        LuaRef ref(m_L);
        swap(ref);
        return *this;
    }

    //=============================================================================================
    /**
     * @brief Assign a different value to this reference.
     *
     * @param rhs A value to assign.
     *
     * @returns This reference.
     */
    template <class T>
    LuaRef& operator=(T rhs)
    {
        LuaRef ref(m_L, rhs);
        swap(ref);
        return *this;
    }

    //=============================================================================================
    /**
     * @brief Place the object onto the Lua stack.
     */
    using LuaRefBase::push;

    void push() const
    {
        lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_ref);
    }

    //=============================================================================================
    /**
     * @brief Pop the top of Lua stack and assign the ref to m_ref
     */
    void pop()
    {
        luaL_unref(m_L, LUA_REGISTRYINDEX, m_ref);
        m_ref = luaL_ref(m_L, LUA_REGISTRYINDEX);
    }

    //=============================================================================================
    /**
     * @brief Access a table value using a key.
     *
     * This invokes metamethods.
     *
     * @param key A key in the table.
     *
     * @returns A reference to the table item.
     */
    template <class T>
    TableItem operator[](T key) const
    {
        std::error_code ec;
        if (! Stack<T>::push(m_L, key, ec))
            return TableItem(m_L, m_ref);

        return TableItem(m_L, m_ref);
    }

    //=============================================================================================
    /**
     * @brief Access a table value using a key.
     *
     * The operation is raw, metamethods are not invoked. The result is passed by value and may not be modified.
     *
     * @param key A key in the table.
     *
     * @returns A reference to the table item.
     */
    template <class T>
    LuaRef rawget(T key) const
    {
        StackPop(m_L, 1);

        push(m_L);

        std::error_code ec;
        if (! Stack<T>::push(m_L, key, ec))
            return LuaRef(m_L);

        lua_rawget(m_L, -2);
        return LuaRef(m_L, FromStack());
    }

private:
    void swap(LuaRef& other)
    {
        using std::swap;
        
        swap(m_L, other.m_L);
        swap(m_ref, other.m_ref);
    }

    int m_ref = LUA_NOREF;
};

//=================================================================================================
/**
 * @brief Stack specialization for `LuaRef`.
 */
template <>
struct Stack<LuaRef>
{
    static bool push(lua_State* L, const LuaRef& v, std::error_code&)
    {
        return v.push(L), true;
    }

    static LuaRef get(lua_State* L, int index)
    {
        return LuaRef::fromStack(L, index);
    }
};

//=================================================================================================
/**
 * @brief Stack specialization for `TableItem`.
 */
template <>
struct Stack<LuaRef::TableItem>
{
    static bool push(lua_State* L, const LuaRef::TableItem& v, std::error_code&)
    {
        return v.push(L), true;
    }
};

//=================================================================================================
/**
 * @brief Create a reference to a new, empty table.
 *
 * This is a syntactic abbreviation for LuaRef::newTable ().
 */
inline LuaRef newTable(lua_State* L)
{
    return LuaRef::newTable(L);
}

//=================================================================================================
/**
 * @brief Create a reference to a value in the global table.
 *
 * This is a syntactic abbreviation for LuaRef::getGlobal ().
 */
inline LuaRef getGlobal(lua_State* L, const char* name)
{
    return LuaRef::getGlobal(L, name);
}

//=================================================================================================
/**
 * @brief C++ like cast syntax.
 */
template <class T>
T cast(const LuaRef& ref)
{
    return ref.cast<T>();
}

} // namespace luabridge
