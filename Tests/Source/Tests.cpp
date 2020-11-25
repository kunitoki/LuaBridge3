// https://github.com/kunitoki/LuaBridge3
// Copyright 2019, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT

// A set of tests of different types' communication with Lua

#include "TestBase.h"

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

TEST_F(LuaBridgeTest, FactoryConstructorNoArgs)
{
    struct Inner
    {
        Inner(int x) : value(x) {}
        
        int value = 0;
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<Inner>("Inner")
        .addFactory([](void* ptr) { new (ptr) Inner(42); })
        .addProperty("value", &Inner::value)
        .endClass();

    runLua("x = Inner () result = x.value");
    EXPECT_EQ(true, result().isNumber());
    EXPECT_EQ(42, result<int>());
}

TEST_F(LuaBridgeTest, FactoryConstructorArgs)
{
    struct Inner
    {
        Inner(int a, int b, int c) : value(a + b + c) {}
        
        int value = 0;
    };

    int y = 1;
    
    luabridge::getGlobalNamespace(L)
        .beginClass<Inner>("Inner")
        .addFactory([y](void* ptr, int a, int b, int c) { new (ptr) Inner(a + y, b, c); })
        .addProperty("value", &Inner::value)
        .endClass();

    runLua("x = Inner (0, 10, 100) result = x.value");
    EXPECT_EQ(true, result().isNumber());
    EXPECT_EQ(111, result<int>());
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
    ASSERT_THROW(runLua("outer:getConstPtr ().data = 20"), std::runtime_error);

    outer.data.data = 3;
    runLua("outer:getRef().data = 30");
    EXPECT_EQ(30, outer.data.data);

    outer.data.data = 4;
    EXPECT_THROW(runLua("outer:getConstPtr ().data = 40"), std::runtime_error);

    outer.data.data = 5;
    runLua("outer:getValueConst ().data = 50");
    EXPECT_EQ(5, outer.data.data);

    outer.data.data = 6;
    runLua("outer:getPtrConst ().data = 60");
    EXPECT_EQ(60, outer.data.data);

    outer.data.data = 7;
    EXPECT_THROW(runLua("outer:getConstPtr ().data = 70"), std::runtime_error);

    outer.data.data = 8;
    runLua("outer:getRef().data = 80");
    EXPECT_EQ(80, outer.data.data);

    outer.data.data = 9;
    EXPECT_THROW(runLua("outer:getConstPtr ().data = 90"), std::runtime_error);
}
