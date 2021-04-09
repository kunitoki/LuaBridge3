// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include <functional>

struct NamespaceTests : TestBase
{
    template<class T>
    T variable(const std::string& name)
    {
        runLua("result = " + name);
        return result<T>();
    }
};

TEST_F(NamespaceTests, Variables)
{
    int int_ = -10;
    int stored = 42;
    auto any = luabridge::newTable(L);
    any["a"] = 1;

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(luabridge::getGlobalNamespace(L).addProperty("int", &int_), std::logic_error);
#endif
    
    runLua("result = int");
    ASSERT_TRUE(result().isNil());

    luabridge::getGlobalNamespace(L)
        .beginNamespace("ns")
        .addProperty("int", &int_)
        .addProperty("any", &any)
        .addProperty("fnc_get", [stored] { return stored; })
        .addProperty("fnc_getset", [stored] { return stored; }, [&stored](int v) { stored = v; })
        .endNamespace();

    ASSERT_EQ(-10, variable<int>("ns.int"));
    ASSERT_EQ(any, variable<luabridge::LuaRef>("ns.any"));

    runLua("ns.int = -20");
    ASSERT_EQ(-20, int_);

    runLua("ns.any = {b = 2}");
    ASSERT_TRUE(any.isTable());
    ASSERT_TRUE(any["b"].isNumber());
    ASSERT_EQ(2, any["b"].cast<int>());

    runLua("result = ns.fnc_get");
    ASSERT_EQ(stored, result<int>());

    runLua("result = ns.fnc_getset");
    ASSERT_EQ(stored, result<int>());

    runLua("ns.fnc_getset = 1337");
    ASSERT_EQ(1337, stored);
}

TEST_F(NamespaceTests, ReadOnlyVariables)
{
    int int_ = -10;
    auto any = luabridge::newTable(L);
    any["a"] = 1;

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(luabridge::getGlobalNamespace(L).addProperty("int", &int_), std::logic_error);
#endif
    
    runLua("result = int");
    ASSERT_TRUE(result().isNil());

    luabridge::getGlobalNamespace(L)
        .beginNamespace("ns")
        .addProperty("int", &int_, false)
        .addProperty("any", &any, false)
        .endNamespace();

    ASSERT_EQ(-10, variable<int>("ns.int"));
    ASSERT_EQ(any, variable<luabridge::LuaRef>("ns.any"));

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("ns.int = -20"), std::runtime_error);
#else
    ASSERT_FALSE(runLua("ns.int = -20"));
#endif
    
    ASSERT_EQ(-10, variable<int>("ns.int"));

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("ns.any = {b = 2}"), std::runtime_error);
#else
    ASSERT_FALSE(runLua("ns.any = {b = 2}"));
#endif

    ASSERT_EQ(any, variable<luabridge::LuaRef>("ns.any"));
}

namespace {

template<class T>
struct Property
{
    static T value;
};

template<class T>
T Property<T>::value;

template<class T>
void setProperty(const T& value)
{
    Property<T>::value = value;
}

template<class T>
const T& getProperty()
{
    return Property<T>::value;
}

} // namespace

TEST_F(NamespaceTests, Properties)
{
    setProperty<int>(-10);

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(
        luabridge::getGlobalNamespace(L).addProperty("int", &getProperty<int>, &setProperty<int>),
        std::logic_error);
#endif
    
    runLua("result = int");
    ASSERT_TRUE(result().isNil());

    luabridge::getGlobalNamespace(L)
        .beginNamespace("ns")
        .addProperty("int", &getProperty<int>, &setProperty<int>)
        .endNamespace();

    ASSERT_EQ(-10, variable<int>("ns.int"));

    runLua("ns.int = -20");
    ASSERT_EQ(-20, getProperty<int>());
}

TEST_F(NamespaceTests, ReadOnlyProperties)
{
    setProperty<int>(-10);

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(luabridge::getGlobalNamespace(L).addProperty("int", &getProperty<int>),
                 std::logic_error);
#endif
    
    runLua("result = int");
    ASSERT_TRUE(result().isNil());

    luabridge::getGlobalNamespace(L)
        .beginNamespace("ns")
        .addProperty("int", &getProperty<int>)
        .endNamespace();

    ASSERT_EQ(-10, variable<int>("ns.int"));

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("ns.int = -20"), std::runtime_error);
#else
    ASSERT_FALSE(runLua("ns.int = -20"));
#endif

    ASSERT_EQ(-10, getProperty<int>());
}

namespace {

template<class T>
struct Storage
{
    static T value;
};

template<class T>
T Storage<T>::value;

template<class T>
int getDataC(lua_State* L)
{
    std::error_code ec;
    luabridge::Stack<T>::push(L, Storage<T>::value, ec);
    return 1;
}

template<class T>
int setDataC(lua_State* L)
{
    Storage<T>::value = luabridge::Stack<T>::get(L, -1);
    return 0;
}

} // namespace

TEST_F(NamespaceTests, NamespaceFromStack)
{
    // Create environment table
    lua_newtable(L);

    luabridge::getNamespaceFromStack(L)
        .addFunction("Function", [this](int x) { return x; });

    int tableReference = luaL_ref(L, LUA_REGISTRYINDEX);

    // Load a script
    std::string script = "result = Function (42)";
    auto success = luaL_loadstring(L, script.c_str()) == LUABRIDGE_LUA_OK;
    ASSERT_TRUE(success);
    
    // Register
    lua_rawgeti(L, LUA_REGISTRYINDEX, tableReference);

    // Set as up value
#if LUA_VERSION_NUM < 502
    ASSERT_EQ(1, lua_setfenv(L, -2));
#else
    auto newUpvalueName = lua_setupvalue(L, -2, 1);

    ASSERT_TRUE(newUpvalueName);
    ASSERT_STREQ("_ENV", newUpvalueName);

    if (! newUpvalueName)
        lua_pop(L, -1);
#endif

    success = lua_pcall(L, 0, 0, 0) == LUABRIDGE_LUA_OK;
    ASSERT_TRUE(success);

    lua_rawgeti(L, LUA_REGISTRYINDEX, tableReference);
    auto resultTable = luabridge::LuaRef::fromStack(L);

    ASSERT_TRUE(resultTable.isTable());
    ASSERT_TRUE(resultTable["result"].cast<int>());
    ASSERT_EQ(42, resultTable["result"].cast<int>());
}


TEST_F(NamespaceTests, Properties_ProxyCFunctions)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("ns")
        .addProperty("value", &getDataC<int>, &setDataC<int>)
        .endNamespace();

    Storage<int>::value = 1;
    runLua("ns.value = 2");
    ASSERT_EQ(2, Storage<int>::value);

    Storage<int>::value = 3;
    runLua("result = ns.value");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(3, result().cast<int>());
}

TEST_F(NamespaceTests, Properties_ProxyCFunctions_ReadOnly)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("ns")
        .addProperty("value", &getDataC<int>)
        .endNamespace();

    Storage<int>::value = 1;

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("ns.value = 2"), std::exception);
#else
    ASSERT_FALSE(runLua("ns.value = 2"));
#endif

    ASSERT_EQ(1, Storage<int>::value);

    Storage<int>::value = 3;
    runLua("result = ns.value");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(3, result().cast<int>());
}

namespace {
struct Class
{
};
} // namespace

TEST_F(NamespaceTests, LuaStackIntegrity)
{
    ASSERT_EQ(1, lua_gettop(L)); // Stack: ...

    {
        auto ns2 =
            luabridge::getGlobalNamespace(L).beginNamespace("namespace").beginNamespace("ns2");

        ASSERT_EQ(
            4,
            lua_gettop(L)); // Stack: ..., global namespace table (gns), namespace table (ns), ns2

        ns2.endNamespace(); // Stack: ...
        ASSERT_EQ(1, lua_gettop(L)); // Stack: ...
    }
    ASSERT_EQ(1, lua_gettop(L)); // Stack: ...

    {
        auto globalNs = luabridge::getGlobalNamespace(L);
        ASSERT_EQ(2, lua_gettop(L)); // Stack: ..., gns

        {
            auto ns = luabridge::getGlobalNamespace(L).beginNamespace("namespace");
            // both globalNs an ns are active
            ASSERT_EQ(4, lua_gettop(L)); // Stack: ..., gns, gns, ns
        }
        ASSERT_EQ(2, lua_gettop(L)); // Stack: ..., gns

        {
            auto ns = globalNs.beginNamespace("namespace");
            // globalNs became inactive
            ASSERT_EQ(3, lua_gettop(L)); // Stack: ..., gns, ns
        }
        ASSERT_EQ(1, lua_gettop(L)); // Stack: ...

#if LUABRIDGE_HAS_EXCEPTIONS
        ASSERT_THROW(globalNs.beginNamespace("namespace"), std::exception);
        ASSERT_THROW(globalNs.beginClass<Class>("Class"), std::exception);
#endif
    }

    {
        auto globalNs = luabridge::getGlobalNamespace(L).beginNamespace("namespace").endNamespace();
        // globalNs is active
        ASSERT_EQ(2, lua_gettop(L)); // Stack: ..., gns
    }
    ASSERT_EQ(1, lua_gettop(L)); // StacK: ...

    {
        auto cls =
            luabridge::getGlobalNamespace(L).beginNamespace("namespace").beginClass<Class>("Class");
        ASSERT_EQ(6, lua_gettop(L)); // Stack: ..., gns, ns, const table, class table, static table
        {
            auto ns = cls.endClass();
            ASSERT_EQ(3, lua_gettop(L)); // Stack: ..., gns, ns
        }
        ASSERT_EQ(1, lua_gettop(L)); // Stack: ...
    }
    ASSERT_EQ(1, lua_gettop(L)); // StacK: ...

    // Test class continuation
    {
        auto cls =
            luabridge::getGlobalNamespace(L).beginNamespace("namespace").beginClass<Class>("Class");
        ASSERT_EQ(6, lua_gettop(L)); // Stack: ..., gns, ns, const table, class table, static table
    }
    ASSERT_EQ(1, lua_gettop(L)); // Stack: ...
}

namespace {

template<class T>
T Function(T param)
{
    return param;
}

} // namespace

TEST_F(NamespaceTests, Functions)
{
    luabridge::getGlobalNamespace(L).addFunction("Function", &Function<double>);

    runLua("result = Function (3.14)");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(3.14, result<double>());
}

TEST_F(NamespaceTests, StdFunctions)
{
    luabridge::getGlobalNamespace(L).addFunction("Function",
                                                 std::function<int(int)>(&Function<int>));

    runLua("result = Function (12)");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(12, result<int>());
}

#ifdef _M_IX86 // Windows 32bit only

namespace {

int __stdcall StdCall(int i)
{
    return i + 10;
}

} // namespace

TEST_F(NamespaceTests, StdCallFunctions)
{
    luabridge::getGlobalNamespace(L).addFunction("StdCall", &StdCall);

    runLua("result = StdCall (2)");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(12, result<int>());
}

#endif // _M_IX86
