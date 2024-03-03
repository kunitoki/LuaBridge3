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

namespace {
enum class A { x, y };
static int fncPointerGetSetValue = 42;
} // namespace

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
        .addProperty("fnc_ptr_get", +[] { return fncPointerGetSetValue; })
        .addProperty("fnc_ptr_getset", +[] { return fncPointerGetSetValue; }, +[](int v) { fncPointerGetSetValue = v; })
        .addProperty("fnc_c_get",
            +[](lua_State* L) { luabridge::getGlobal(L, "xyz").push(); return 1; })
        .addProperty("fnc_c_getset",
            +[](lua_State* L) { luabridge::getGlobal(L, "xyz").push(); return 1; },
            +[](lua_State* L) { luabridge::setGlobal(L, luabridge::LuaRef::fromStack(L, 1).unsafe_cast<int>(), "xyz"); return 0; })
        .addVariable("A_x", A::x)
        .addVariable("A_y", A::y)
        .endNamespace();

    luabridge::setGlobal(L, 666, "xyz");

    runLua("result = int");

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

    runLua("result = ns.fnc_ptr_get");
    ASSERT_EQ(fncPointerGetSetValue, result<int>());

    runLua("result = ns.fnc_ptr_getset");
    ASSERT_EQ(fncPointerGetSetValue, result<int>());

    runLua("ns.fnc_ptr_getset = 1337");
    ASSERT_EQ(1337, fncPointerGetSetValue);

    runLua("result = ns.fnc_c_get");
    ASSERT_EQ(666, result<int>());

    runLua("result = ns.fnc_c_getset");
    ASSERT_EQ(666, result<int>());

    runLua("ns.fnc_c_getset = 1337");
    ASSERT_EQ(1337, luabridge::getGlobal(L, "xyz").unsafe_cast<int>());

    runLua("result = ns.A_x");
    ASSERT_EQ(A::x, static_cast<A>(result<int>()));

    runLua("result = ns.A_y");
    ASSERT_EQ(A::y, static_cast<A>(result<int>()));
}

TEST_F(NamespaceTests, ReadOnlyVariables)
{
    int int_ = -10;
    const int const_int_ = 1337;
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
        .addProperty("const_int", &const_int_)
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
    ASSERT_THROW(runLua("ns.const_int = 100"), std::runtime_error);
#else
    ASSERT_FALSE(runLua("ns.const_int = 100"));
#endif

    ASSERT_EQ(1337, variable<int>("ns.const_int"));

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("ns.any = {b = 2}"), std::runtime_error);
#else
    ASSERT_FALSE(runLua("ns.any = {b = 2}"));
#endif

    ASSERT_EQ(any, variable<luabridge::LuaRef>("ns.any"));
}

namespace {
template <class T>
struct Property
{
    static T value;
};

template <class T>
T Property<T>::value;

template <class T>
void setProperty(const T& value)
{
    Property<T>::value = value;
}

template <class T>
void setPropertyNoexcept(const T& value) noexcept
{
    Property<T>::value = value;
}

template <class T>
const T& getProperty()
{
    return Property<T>::value;
}

template <class T>
const T& getPropertyNoexcept() noexcept
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
        .addProperty("int2", &getPropertyNoexcept<int>, &setPropertyNoexcept<int>)
        .endNamespace();

    ASSERT_EQ(-10, variable<int>("ns.int"));
    ASSERT_EQ(-10, variable<int>("ns.int2"));

    runLua("ns.int = -20");
    ASSERT_EQ(-20, getProperty<int>());

    runLua("ns.int2 = -20");
    ASSERT_EQ(-20, getPropertyNoexcept<int>());
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
        .addProperty("int2", &getPropertyNoexcept<int>)
        .endNamespace();

    ASSERT_EQ(-10, variable<int>("ns.int"));

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("ns.int = -20"), std::runtime_error);
    ASSERT_THROW(runLua("ns.int2 = -20"), std::runtime_error);
#else
    ASSERT_FALSE(runLua("ns.int = -20"));
    ASSERT_FALSE(runLua("ns.int2 = -20"));
#endif

    ASSERT_EQ(-10, getProperty<int>());
    ASSERT_EQ(-10, getPropertyNoexcept<int>());
}

TEST_F(NamespaceTests, AddVariable)
{
    int int_ = -10;
    auto any = luabridge::newTable(L);
    any["a"] = 1;

    enum class A { a, b, c, d };

    luabridge::getGlobalNamespace(L)
        .addVariable("int", &int_);

    runLua("result = int");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(int_, result<int>());

    luabridge::getGlobalNamespace(L)
        .beginNamespace("ns")
            .addVariable("int", int_)
            .addVariable("any", any)
            .beginNamespace("A")
                .addVariable("a", A::a)
                .addVariable("b", A::b)
                .addVariable("c", A::c)
                .addVariable("d", A::d)
            .endNamespace()
        .endNamespace();

    ASSERT_EQ(-10, variable<int>("ns.int"));
    ASSERT_EQ(any, variable<luabridge::LuaRef>("ns.any"));

    runLua("ns.A.a = 2; result = ns.A.a");
    ASSERT_EQ(A::c, static_cast<A>(result<int>()));

    runLua("ns.int = -20");
    ASSERT_EQ(-10, int_);

    runLua("result = ns.int");
    ASSERT_EQ(-20, result<int>());

    runLua("ns.any = {a = 42, b = 2}");
    ASSERT_TRUE(any.isTable());
    ASSERT_TRUE(any["b"].isNil());

    runLua("result = ns.any.a");
    ASSERT_EQ(42, result<int>());
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
    [[maybe_unused]] auto result = luabridge::Stack<T>::push(L, Storage<T>::value);

    return 1;
}

template<class T>
int setDataC(lua_State* L)
{
    auto result = luabridge::Stack<T>::get(L, -1);
    if (! result)
        luaL_error(L, "%s", result.error().message().c_str());

    Storage<T>::value = *result;

    return 0;
}
} // namespace

TEST_F(NamespaceTests, NamespaceFromStack)
{
    // Create environment table
    lua_newtable(L);
    luabridge::getNamespaceFromStack(L)
        .addFunction("Function", [](int x) { return x; });

    int tableReference = luabridge::luaL_ref(L, LUA_REGISTRYINDEX);

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

TEST_F(NamespaceTests, NamespaceFromStackProperties)
{
    int x = 0;
    int wx = 10;
    const int cx = 100;

    lua_newtable(L);
    luabridge::getNamespaceFromStack(L)
        .addProperty("valueX", &x, false)
        .addProperty("valueWX", &wx, true)
        .addProperty("valueCX", &cx)
        .addProperty("value1", +[] { return 1; })
        .addProperty("value2", +[] { return 2; }, +[](int) {})
        .addProperty("value_getX", [&x] { return x; })
        .addProperty("value_getSetX", [&x] { return x; }, [&x](int v) { x = v; })
    ;

    auto table = luabridge::LuaRef::fromStack(L);

    luabridge::setGlobal(L, table, "tab");

    runLua("result = tab.valueX");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(0, result<int>());

    runLua("result = tab.valueWX");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(10, result<int>());

    runLua("result = tab.valueCX");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(100, result<int>());

    runLua("result = tab.value1");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(1, result<int>());

    runLua("result = tab.value2");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(2, result<int>());

    runLua("result = tab.value_getX");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(0, result<int>());

    runLua("result = tab.value_getSetX");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(0, result<int>());

    //runLua("tab.value_getSetX = 111");
    //EXPECT_EQ(111, x);
}

TEST_F(NamespaceTests, Properties_ProxyCFunctions)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("ns")
        .addProperty("value", &getDataC<int>, &setDataC<int>)
        .endNamespace();

    Storage<int>::value = 1;
    runLua("ns.value = 2");
    EXPECT_EQ(2, Storage<int>::value);

    Storage<int>::value = 3;
    runLua("result = ns.value");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(3, result<int>());
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
    ASSERT_EQ(3, result<int>());
}

namespace {
struct Class {};
} // namespace

TEST_F(NamespaceTests, LuaStackIntegrity)
{
    const int initialStackIndex = 0;

    EXPECT_EQ(initialStackIndex, lua_gettop(L)); // Stack: ...

    {
        auto ns2 =
            luabridge::getGlobalNamespace(L).beginNamespace("namespace").beginNamespace("ns2");

        EXPECT_EQ(
            initialStackIndex + 3,
            lua_gettop(L)); // Stack: ..., global namespace table (gns), namespace table (ns), ns2

        ns2.endNamespace(); // Stack: ...
        EXPECT_EQ(initialStackIndex, lua_gettop(L)); // Stack: ...
    }

    EXPECT_EQ(initialStackIndex, lua_gettop(L)); // Stack: ...

    {
        auto globalNs = luabridge::getGlobalNamespace(L);
        EXPECT_EQ(initialStackIndex + 1, lua_gettop(L)); // Stack: ..., gns

        {
            auto ns = luabridge::getGlobalNamespace(L).beginNamespace("namespace");
            // both globalNs an ns are active
            EXPECT_EQ(initialStackIndex + 3, lua_gettop(L)); // Stack: ..., gns, gns, ns
        }

        EXPECT_EQ(initialStackIndex + 1, lua_gettop(L)); // Stack: ..., gns

        {
            auto ns = globalNs.beginNamespace("namespace");
            // globalNs became inactive
            EXPECT_EQ(initialStackIndex + 2, lua_gettop(L)); // Stack: ..., gns, ns
        }

        EXPECT_EQ(initialStackIndex, lua_gettop(L)); // Stack: ...

#if LUABRIDGE_HAS_EXCEPTIONS
        EXPECT_THROW(globalNs.beginNamespace("namespace"), std::exception);
        EXPECT_THROW(globalNs.beginClass<Class>("Class"), std::exception);
#endif
    }

    {
        auto globalNs = luabridge::getGlobalNamespace(L).beginNamespace("namespace").endNamespace();
        // globalNs is active
        EXPECT_EQ(initialStackIndex + 1, lua_gettop(L)); // Stack: ..., gns
    }

    EXPECT_EQ(initialStackIndex, lua_gettop(L)); // StacK: ...

    {
        auto cls =
            luabridge::getGlobalNamespace(L).beginNamespace("namespace").beginClass<Class>("Class");
        EXPECT_EQ(initialStackIndex + 5, lua_gettop(L)); // Stack: ..., gns, ns, const table, class table, static table

        {
            auto ns = cls.endClass();
            EXPECT_EQ(initialStackIndex + 2, lua_gettop(L)); // Stack: ..., gns, ns
        }

        EXPECT_EQ(initialStackIndex, lua_gettop(L)); // Stack: ...
    }

    EXPECT_EQ(initialStackIndex, lua_gettop(L)); // StacK: ...

    // Test class continuation
    {
        auto cls =
            luabridge::getGlobalNamespace(L).beginNamespace("namespace").beginClass<Class>("Class");
        EXPECT_EQ(initialStackIndex + 5, lua_gettop(L)); // Stack: ..., gns, ns, const table, class table, static table
    }

    EXPECT_EQ(initialStackIndex, lua_gettop(L)); // Stack: ...
}

TEST_F(NamespaceTests, LuaStackAdditionalIntegrity)
{
    [[maybe_unused]] auto globalNs = luabridge::getGlobalNamespace(L);

#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_THROW(globalNs.endNamespace(), std::exception);
#endif
}

namespace {
template<class T>
T Function(T param)
{
    return param;
}

int LuaFunction(lua_State* L)
{
    lua_pushnumber(L, 42);
    return 1;
}
} // namespace

TEST_F(NamespaceTests, Functions)
{
    luabridge::getGlobalNamespace(L).addFunction("Function", &Function<double>);

    runLua("result = Function (3.14)");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(3.14, result<double>());
}

TEST_F(NamespaceTests, LuaFunctions)
{
    luabridge::getGlobalNamespace(L).addFunction("Function", &LuaFunction);

    runLua("result = Function ()");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(42, result<int>());
}

TEST_F(NamespaceTests, StdFunctions)
{
    luabridge::getGlobalNamespace(L).addFunction("Function", std::function<int(int)>(&Function<int>));

    runLua("result = Function (12)");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(12, result<int>());
}

TEST_F(NamespaceTests, CapturingLambdas)
{
    int x = 30;

    luabridge::getGlobalNamespace(L).addFunction("Function", [x](int v) -> int { return x + Function(v); });

    runLua("result = Function (12)");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(42, result<int>());
}

TEST_F(NamespaceTests, CapturingMutableLambdas)
{
    int x = 30;

    luabridge::getGlobalNamespace(L).addFunction("Function", [x](int v) mutable -> int { x += Function(v); return x; });

    runLua("Function (12); result = Function (12)");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(54, result<int>());
}

namespace {
class SystemDestroyer {};
} // namespacw

TEST_F(NamespaceTests, IndexAccessByNonStringShouldNotCrash)
{
    luabridge::getGlobalNamespace(L)
       .beginNamespace("test")
          .beginClass<SystemDestroyer>("SystemDestroyer")
          .endClass()
       .endNamespace();

    runLua("result = test[SystemDestroyer]");
    EXPECT_TRUE(result().isNil());
}

TEST_F(NamespaceTests, NamespaceAsRValueReferenceShouldWork)
{
    auto ns = luabridge::getGlobalNamespace(L);

    ns = ns.beginNamespace("test");
    
    auto cls = ns.beginClass<SystemDestroyer>("SystemDestroyer");
    ns = cls.endClass();
    
    ns.endNamespace();

    runLua("result = test[SystemDestroyer]");
    EXPECT_TRUE(result().isNil());
}

TEST_F(NamespaceTests, BugWithLuauNotPrintingMethodNameInErrors)
{
    auto strangelyNamedMethod = [](lua_State* L)
    {
        luaL_argerror(L, 1, "test message");
        return 0;
    };

    luabridge::getGlobalNamespace(L)
        .beginNamespace("foo")
            .addFunction("strangelyNamedMethod", strangelyNamedMethod)
        .endNamespace();

    auto [result, error] = runLuaCaptureError(R"(
        local bar = function() foo.strangelyNamedMethod() end
        bar()
    )");

    EXPECT_TRUE(error.find("strangelyNamedMethod") != std::string::npos);
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

namespace {
int __fastcall FastCall(int i)
{
    return i + 10;
}
} // namespace

TEST_F(NamespaceTests, FastCallFunctions)
{
    luabridge::getGlobalNamespace(L).addFunction("FastCall", &FastCall);

    runLua("result = FastCall (2)");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(12, result<int>());
}

#endif // _M_IX86
