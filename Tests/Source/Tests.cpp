// https://github.com/kunitoki/LuaBridge3
// Copyright 2021, Lucio Asnaghi
// Copyright 2019, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT

// A set of tests of different types' communication with Lua

#include "TestBase.h"

#include "LuaBridge/Set.h"
#include "LuaBridge/List.h"

#include <gtest/gtest.h>

#include <cstring>
#include <iostream>
#include <memory>
#include <string>

void printValue(lua_State* L, int index)
{
    int type = lua_type(L, index);
    switch (type)
    {
    case LUA_TBOOLEAN:
        std::cerr << std::boolalpha << (lua_toboolean(L, index) != 0);
        break;
    case LUA_TSTRING:
        std::cerr << lua_tostring(L, index);
        break;
    case LUA_TNUMBER:
        std::cerr << lua_tonumber(L, index);
        break;
    case LUA_TTABLE:
    case LUA_TTHREAD:
    case LUA_TFUNCTION:
        std::cerr << lua_topointer(L, index);
        break;
    }
    std::cerr << ": " << lua_typename(L, type) << " (" << type << ")" << std::endl;
}

struct LuaBridgeTest : TestBase
{
};

template<class T>
T identityCFunction(T value)
{
    return value;
}

TEST_F(LuaBridgeTest, LambdaGlobalNamespace)
{
    int x = 100;
    
    luabridge::getGlobalNamespace(L)
        .addFunction("test", [x](int v) -> int { return v + x; })
        .addFunction("test2", [x](lua_State* L, int v) -> int { return v + (L != nullptr ? x : 0); });

    runLua("result = test (255)");
    EXPECT_EQ(true, result().isNumber());
    EXPECT_EQ(355, result<int>());

    resetResult();
    runLua("result = test2 (nil, 255)");
    EXPECT_EQ(true, result().isNumber());
    EXPECT_EQ(355, result<int>());
}

TEST_F(LuaBridgeTest, LambdaClassMethods)
{
    int x = 100;
        
    struct Inner
    {
        Inner() = default;
        
        int normalMethod0() const { return 42; }
        int normalMethod1(int) const { return 42; }
    };
    
    luabridge::getGlobalNamespace(L)
        .beginClass<Inner>("Inner")
        .addConstructor<void (*)()>()
        .addFunction("test", [x](Inner*, int v) -> int { return v + x; })
        .addFunction("test2", [x](const Inner*, int v) -> int { return v + x; })
        .addFunction("normalMethod0", &Inner::normalMethod0)
        .addFunction("normalMethod1", &Inner::normalMethod1)
        .endClass();

    runLua("x = Inner () result = x:test (255)");
    EXPECT_EQ(true, result().isNumber());
    EXPECT_EQ(355, result<int>());

    resetResult();
    runLua("x = Inner () result = x:test (255)");
    EXPECT_EQ(true, result().isNumber());
    EXPECT_EQ(355, result<int>());
}

TEST_F(LuaBridgeTest, CFunction)
{
    luabridge::getGlobalNamespace(L)
        .addFunction("boolFn", &identityCFunction<bool>)
        .addFunction("ucharFn", &identityCFunction<unsigned char>)
        .addFunction("shortFn", &identityCFunction<short>)
        .addFunction("ushortFn", &identityCFunction<unsigned short>)
        .addFunction("intFn", &identityCFunction<int>)
        .addFunction("uintFn", &identityCFunction<unsigned int>)
        .addFunction("longFn", &identityCFunction<long>)
        .addFunction("ulongFn", &identityCFunction<unsigned long>)
        .addFunction("longlongFn", &identityCFunction<long long>)
        .addFunction("ulonglongFn", &identityCFunction<unsigned long long>)
        .addFunction("floatFn", &identityCFunction<float>)
        .addFunction("doubleFn", &identityCFunction<double>)
        .addFunction("charFn", &identityCFunction<char>)
        .addFunction("cstringFn", &identityCFunction<const char*>)
        .addFunction("stringFn", &identityCFunction<std::string>);

    {
        runLua("result = ucharFn (255)");
        EXPECT_EQ(true, result().isNumber());
        EXPECT_EQ(255u, result<unsigned char>());
    }

    {
        runLua("result = boolFn (false)");
        EXPECT_EQ(true, result().isBool());
        EXPECT_EQ(false, result<bool>());
    }
    {
        runLua("result = boolFn (true)");
        EXPECT_EQ(true, result().isBool());
        EXPECT_EQ(true, result<bool>());
    }

    {
        runLua("result = shortFn (-32768)");
        EXPECT_EQ(true, result().isNumber());
        EXPECT_EQ(-32768, result<int>());
    }

    {
        runLua("result = ushortFn (32767)");
        EXPECT_EQ(true, result().isNumber());
        EXPECT_EQ(32767u, result<unsigned int>());
    }
    {
        runLua("result = intFn (-500)");
        EXPECT_EQ(true, result().isNumber());
        EXPECT_EQ(-500, result<int>());
    }

    {
        runLua("result = uintFn (42)");
        EXPECT_EQ(true, result().isNumber());
        EXPECT_EQ(42u, result<unsigned int>());
    }

    {
        runLua("result = longFn (-8000)");
        EXPECT_EQ(true, result().isNumber());
        EXPECT_EQ(-8000, result<long>());
    }

    {
        runLua("result = ulongFn (9000)");
        EXPECT_EQ(true, result().isNumber());
        EXPECT_EQ(9000u, result<unsigned long>());
    }

    {
        runLua("result = longlongFn (-8000)");
        EXPECT_EQ(true, result().isNumber());
        EXPECT_EQ(-8000, result<long long>());
    }

    {
        runLua("result = ulonglongFn (9000)");
        EXPECT_EQ(true, result().isNumber());
        EXPECT_EQ(9000u, result<unsigned long long>());
    }

    {
        runLua("result = floatFn (3.14)");
        EXPECT_EQ(true, result().isNumber());
        EXPECT_FLOAT_EQ(3.14f, result<float>());
    }

    {
        runLua("result = doubleFn (-12.3)");
        EXPECT_EQ(true, result().isNumber());
        EXPECT_DOUBLE_EQ(-12.3, result<double>());
    }

    {
        runLua("result = charFn ('a')");
        EXPECT_EQ(true, result().isString());
        EXPECT_EQ('a', result<char>());
    }

    {
        runLua("result = cstringFn ('abc')");
        EXPECT_EQ(true, result().isString());
        EXPECT_STREQ("abc", result<const char*>());
    }

    {
        runLua("result = stringFn ('lua')");
        EXPECT_EQ(true, result().isString());
        EXPECT_EQ("lua", result<std::string>());
    }
}

TEST_F(LuaBridgeTest, Tuple)
{
    std::tuple<int, float> t = std::make_tuple(1, 2.0f);
    
    luabridge::getGlobalNamespace(L)
        .beginNamespace("tuple")
            .addProperty("t", &t)
        .endNamespace();

    {
        resetResult();
        runLua("result = { 1, 2 }");
        EXPECT_EQ(true, result().isTable());
        EXPECT_EQ((std::make_tuple(1, 2)), (result<std::tuple<int, int>>()));
    }

    {
        resetResult();
        runLua("tuple.t = { 2, 4.0 }");
        EXPECT_EQ(2, std::get<0>(t));
        EXPECT_FLOAT_EQ(4.0f, std::get<1>(t));
    }

    {
        resetResult();
        runLua("result = tuple.t");
        EXPECT_EQ(true, result().isTable());
        EXPECT_EQ(2, std::get<0>(result<std::tuple<int, float>>()));
        EXPECT_FLOAT_EQ(4.0f, std::get<1>(result<std::tuple<int, float>>()));
    }
}

TEST_F(LuaBridgeTest, TupleAsFunctionReturnValue)
{
    int x = 100;
        
    struct Inner
    {
        Inner() = default;
    };
    
    luabridge::getGlobalNamespace(L)
        .beginClass<Inner>("Inner")
        .addConstructor<void (*)()>()
        .addFunction("test", [x](Inner*) { return std::make_tuple(x, 42); })
        .endClass();

    runLua("x = Inner () result = x:test ()");
    EXPECT_EQ(true, result().isTable());
    EXPECT_EQ(std::make_tuple(x, 42), (result<std::tuple<int, int>>()));
}

template<class T>
struct TestClass
{
    TestClass(T data) : data(data), constData(data) {}

    T getValue() { return data; }
    T* getPtr() { return &data; }
    T const* getConstPtr() { return &data; }
    T& getRef() { return data; }
    T const& getConstRef() { return data; }
    T getValueConst() const { return data; }
    T* getPtrConst() const { return &data; }
    T const* getConstPtrConst() const { return &data; }
    T& getRefConst() const { return data; }
    T const& getConstRefConst() const { return data; }

    mutable T data;
    mutable T constData;
};

TEST_F(LuaBridgeTest, ClassFunction)
{
    typedef TestClass<int> Inner;
    typedef TestClass<Inner> Outer;

    luabridge::getGlobalNamespace(L)
        .beginClass<Inner>("Inner")
        .addConstructor<void (*)(int)>()
        .addProperty("data", &Inner::data)
        .endClass()
        .beginClass<Outer>("Outer")
        .addConstructor<void (*)(Inner)>()
        .addFunction("getValue", &Outer::getValue)
        .addFunction("getPtr", &Outer::getPtr)
        .addFunction("getConstPtr", &Outer::getConstPtr)
        .addFunction("getRef", &Outer::getRef)
        .addFunction("getConstRef", &Outer::getConstRef)
        .addFunction("getValueConst", &Outer::getValueConst)
        .addFunction("getPtrConst", &Outer::getPtrConst)
        .addFunction("getConstPtrConst", &Outer::getConstPtrConst)
        .addFunction("getRefConst", &Outer::getRefConst)
        .addFunction("getConstRefConst", &Outer::getConstRefConst)
        .endClass();

    Outer outer(Inner(0));
    luabridge::setGlobal(L, &outer, "outer");

    outer.data.data = 0;
    runLua("outer:getValue ().data = 1");
    EXPECT_EQ(0, outer.data.data);

    outer.data.data = 1;
    runLua("outer:getPtr ().data = 10");
    EXPECT_EQ(10, outer.data.data);

    outer.data.data = 2;
#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("outer:getConstPtr ().data = 20"), std::runtime_error);
#else
    EXPECT_FALSE(runLua("outer:getConstPtr ().data = 20"));
#endif

    outer.data.data = 3;
    runLua("outer:getRef().data = 30");
    EXPECT_EQ(30, outer.data.data);

    outer.data.data = 4;
#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_THROW(runLua("outer:getConstPtr ().data = 40"), std::runtime_error);
#else
    EXPECT_FALSE(runLua("outer:getConstPtr ().data = 40"));
#endif

    outer.data.data = 5;
    runLua("outer:getValueConst ().data = 50");
    EXPECT_EQ(5, outer.data.data);

    outer.data.data = 6;
    runLua("outer:getPtrConst ().data = 60");
    EXPECT_EQ(60, outer.data.data);

    outer.data.data = 7;
#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_THROW(runLua("outer:getConstPtr ().data = 70"), std::runtime_error);
#else
    EXPECT_FALSE(runLua("outer:getConstPtr ().data = 70"));
#endif

    outer.data.data = 8;
    runLua("outer:getRef().data = 80");
    EXPECT_EQ(80, outer.data.data);

    outer.data.data = 9;
#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_THROW(runLua("outer:getConstPtr ().data = 90"), std::runtime_error);
#else
    EXPECT_FALSE(runLua("outer:getConstPtr ().data = 90"));
#endif
}

TEST_F(LuaBridgeTest, PropertyGetterFailOnUnregistredClass)
{
    struct Clazz {} clazz;
    
    luabridge::getGlobalNamespace(L)
        .beginNamespace("ns")
            .addProperty("clazz", &clazz)
        .endNamespace();

#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_THROW(runLua("result = ns.clazz"), std::runtime_error);
#else
    EXPECT_FALSE(runLua("result = ns.clazz"));
#endif
}

TEST_F(LuaBridgeTest, CallReturnLuaResult)
{
    runLua("function f1 (arg0, arg1) end");
    runLua("function f2 (arg0, arg1) return arg0; end");
    runLua("function f3 (arg0, arg1) return arg0, arg1; end");
    runLua("function f4 () error('Something bad happened'); return arg0, arg1; end");

    {
        auto f1 = luabridge::getGlobal(L, "f1");
        auto result = luabridge::call(f1, 1, 2);
        EXPECT_FALSE(result.hasFailed());
        EXPECT_TRUE(result.wasOk());
        EXPECT_EQ(std::error_code(), result.errorCode());
    }

    {
        auto f2 = luabridge::getGlobal(L, "f2");
        auto result = luabridge::call(f2, 1, 2);
        EXPECT_FALSE(result.hasFailed());
        EXPECT_TRUE(result.wasOk());
        EXPECT_EQ(std::error_code(), result.errorCode());
        EXPECT_EQ(1u, result.size());
        EXPECT_EQ(result[0], 1);
    }

    {
        auto f3 = luabridge::getGlobal(L, "f3");
        auto result = luabridge::call(f3, 1, 2);
        EXPECT_FALSE(result.hasFailed());
        EXPECT_TRUE(result.wasOk());
        EXPECT_EQ(std::error_code(), result.errorCode());
        EXPECT_EQ(2u, result.size());
        EXPECT_EQ(result[0], 1);
        EXPECT_EQ(result[1], 2);
    }

#if ! LUABRIDGE_HAS_EXCEPTIONS
    {
        auto f3 = luabridge::getGlobal(L, "f4");
        auto result = luabridge::call(f3);
        EXPECT_TRUE(result.hasFailed());
        EXPECT_FALSE(result.wasOk());
        EXPECT_EQ(0u, result.size());
        EXPECT_NE(std::error_code(), result.errorCode());
        EXPECT_NE(std::string::npos, result.errorMessage().find("Something bad happened"));
    }
#endif
}

TEST_F(LuaBridgeTest, InvokePassingUnregisteredClassShouldThrowAndRestoreStack)
{
    class Unregistered {} unregistered;

    {
        runLua("function f1 (unregistered) end");

        auto f1 = luabridge::getGlobal(L, "f1");

#if LUABRIDGE_HAS_EXCEPTIONS
        luabridge::enableExceptions(L);
        EXPECT_THROW(luabridge::call(f1, unregistered), luabridge::LuaException);
#else
        int stackTop = lua_gettop(L);
        
        auto result = luabridge::call(f1, unregistered);
        EXPECT_TRUE(result.hasFailed());
        EXPECT_FALSE(result.wasOk());
        EXPECT_EQ(luabridge::makeErrorCode(luabridge::ErrorCode::ClassNotRegistered), result.errorCode());

        EXPECT_EQ(stackTop, lua_gettop(L));
#endif
    }
}

class A : public std::enable_shared_from_this<A>
{
public:
    A(int newX) : x(newX) {}
    
    int x = 42;
};

TEST_F(LuaBridgeTest, StdSharedPtr)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("test")
            .beginClass<A>("A")
                .addConstructor<void(*)(int), std::shared_ptr<A>>()
            .endClass()
        .endNamespace();
    
    std::shared_ptr<A> a = std::make_shared<A>(1);
    luabridge::setGlobal(L, a, "a");
    
    std::shared_ptr<A> a2 = luabridge::getGlobal<std::shared_ptr<A>>(L, "a");
    EXPECT_EQ(1, a2->x);

    EXPECT_TRUE(runLua("result = a"));
    auto a3 = result().cast<std::shared_ptr<A>>();
    EXPECT_EQ(1, a3->x);

    EXPECT_TRUE(runLua("result = test.A(2)"));
    auto a4 = result().cast<std::shared_ptr<A>>();
    EXPECT_EQ(2, a4->x);
}
