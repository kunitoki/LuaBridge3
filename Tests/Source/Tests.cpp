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

namespace {
template <class T>
T identityCFunction(T value)
{
    return value;
}
} // namespace

struct LuaBridgeTest : TestBase
{
};

class FCStaffSystem {};
class FCSystemStaff {};

TEST_F(LuaBridgeTest, TypeName)
{
    auto x1 = luabridge::detail::typeName<FCStaffSystem>();
    auto x2 = luabridge::detail::typeName<FCSystemStaff>();
    EXPECT_NE(x1, x2);

    {
        auto y1 = luabridge::detail::typeHash<FCStaffSystem>();
        auto y2 = luabridge::detail::typeHash<FCSystemStaff>();
        EXPECT_NE(y1, y2);
    }

    {
        auto y1 = luabridge::detail::typeHash<FCStaffSystem>() ^ 1;
        auto y2 = luabridge::detail::typeHash<FCSystemStaff>() ^ 1;
        EXPECT_NE(y1, y2);
    }

    {
        auto y1 = luabridge::detail::typeHash<FCStaffSystem>() ^ 2;
        auto y2 = luabridge::detail::typeHash<FCSystemStaff>() ^ 2;
        EXPECT_NE(y1, y2);
    }
}

TEST_F(LuaBridgeTest, LambdaGlobalNamespace)
{
    int x = 100;
    
    luabridge::getGlobalNamespace(L)
        .addFunction("test", [x](int v) -> int {
            return v + x;
        })
        .addFunction("test2", [x](lua_State* L, int v) -> int {
            return v + (L != nullptr ? x : 0);
        });

    runLua("result = test (255)");
    
    ASSERT_EQ(true, result().isNumber());
    EXPECT_EQ(355, result<int>());

    resetResult();
    runLua("result = test2 (nil, 255)");
    ASSERT_EQ(true, result().isNumber());
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

namespace {
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
} // namespace

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

TEST_F(LuaBridgeTest, PropertyGetterFailOnUnregisteredClass)
{
    struct Clazz {} clazz;
    
    luabridge::getGlobalNamespace(L)
        .beginNamespace("ns")
            .addProperty("clazz", &clazz)
        .endNamespace();

#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_ANY_THROW(runLua("result = ns.clazz"));
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

namespace {
struct Foo {};
} // namespace

TEST_F(LuaBridgeTest, NonConstMethodOnConstPointer)
{
   luabridge::getGlobalNamespace(L)
      .beginNamespace("test")
         .addFunction("constCall", [](const Foo*) { return "const"; })
         .addFunction("mutableCall", [](Foo*) { return "mutable"; })
         .beginClass<Foo>("Foo")
            .addFunction("constCall", [](const Foo*) { return "const"; })
            .addFunction("mutableCall", [](Foo*) { return "mutable"; })
         .endClass()
      .endNamespace();

    Foo a, b;
    luabridge::setGlobal(L, std::addressof(a), "a");
    luabridge::setGlobal(L, std::addressof(std::as_const(b)), "b");

    runLua("result = a:constCall()");
    EXPECT_EQ("const", result<std::string>());
    runLua("result = test.constCall(a)");
    EXPECT_EQ("const", result<std::string>());

    runLua("result = a:mutableCall()");
    EXPECT_EQ("mutable", result<std::string>());
    runLua("result = test.mutableCall(a)");
    EXPECT_EQ("mutable", result<std::string>());

    runLua("result = b:constCall()");
    EXPECT_EQ("const", result<std::string>());
    runLua("result = test.constCall(b)");
    EXPECT_EQ("const", result<std::string>());

#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_ANY_THROW(runLua("result = b:mutableCall()"));
    EXPECT_ANY_THROW(runLua("result = test.mutableCall(b)"));
#else
    EXPECT_FALSE(runLua("result = b:mutableCall()"));
    EXPECT_FALSE(runLua("result = test.mutableCall(b)"));
#endif
}

namespace {
class A : public std::enable_shared_from_this<A>
{
public:
    A() = default;

    A(int newX)
        : x(newX)
    {
    }
    
    int x = 42;
};

class B : public A
{
public:
    B() = default;

    B(int newX)
        : A(newX)
    {
    }
};

class VirtualA : public std::enable_shared_from_this<VirtualA>
{
public:
    VirtualA() = default;
    virtual ~VirtualA() = default;

    VirtualA(int newX)
        : x(newX)
    {
    }

    virtual std::string myNameIs() const
    {
        return "VirtualA";
    }

    int x = 42;
};

class VirtualB : public VirtualA
{
public:
    VirtualB() = default;

    VirtualB(int newX)
        : VirtualA(newX)
    {
    }

    std::string myNameIs() const override
    {
        return "VirtualB";
    }
};

class VirtualC : public VirtualB
{
public:
    VirtualC() = default;

    VirtualC(int newX)
        : VirtualB(newX)
    {
    }

    std::string myNameIs() const override
    {
        return "VirtualC";
    }
};
} // namespace

TEST_F(LuaBridgeTest, StdSharedPtrSingle)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("test")
            .beginClass<A>("A")
                .addConstructorFrom<std::shared_ptr<A>, void(*)(int)>()
            .endClass()
        .endNamespace();
    
    std::shared_ptr<A> a = std::make_shared<A>(1);
    luabridge::setGlobal(L, a, "a");
    
    std::shared_ptr<A> a2 = *luabridge::getGlobal<std::shared_ptr<A>>(L, "a");
    EXPECT_EQ(1, a2->x);

    EXPECT_TRUE(runLua("result = a"));
    auto a3 = result<std::shared_ptr<A>>();
    EXPECT_EQ(1, a3->x);

    EXPECT_TRUE(runLua("result = test.A(2)"));
    auto a4 = result<std::shared_ptr<A>>();
    EXPECT_EQ(2, a4->x);
}

TEST_F(LuaBridgeTest, StdSharedPtrSingleCustomConstructor)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("test")
            .beginClass<A>("A1")
                .addConstructorFrom<std::shared_ptr<A>>(
                    [] { return std::make_shared<A>(); })
            .endClass()
        .endNamespace();

    luabridge::getGlobalNamespace(L)
        .beginNamespace("test")
            .beginClass<A>("A2")
                .addConstructorFrom<std::shared_ptr<A>>(
                    [] { return std::make_shared<A>(); },
                    [](int x) { return std::make_shared<A>(x); },
                    [](int x, int y) { return std::make_shared<A>(x + y); })
            .endClass()
        .endNamespace();

    EXPECT_TRUE(runLua("result = test.A1()"));
    auto a0 = result<std::shared_ptr<A>>();
    EXPECT_EQ(42, a0->x);

    EXPECT_TRUE(runLua("result = test.A2()"));
    auto a1 = result<std::shared_ptr<A>>();
    EXPECT_EQ(42, a1->x);

    EXPECT_TRUE(runLua("result = test.A2(1337)"));
    auto a2 = result<std::shared_ptr<A>>();
    EXPECT_EQ(1337, a2->x);

    EXPECT_TRUE(runLua("result = test.A2(11, 22)"));
    auto a3 = result<std::shared_ptr<A>>();
    EXPECT_EQ(33, a3->x);
}

TEST_F(LuaBridgeTest, StdSharedPtrMultiple)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("test")
            .beginClass<A>("A")
                .addConstructorFrom<std::shared_ptr<A>, void(*)(), void(*)(int)>()
            .endClass()
        .endNamespace();

    EXPECT_TRUE(runLua("result = test.A()"));
    auto a1 = result<std::shared_ptr<A>>();
    EXPECT_EQ(42, a1->x);

    EXPECT_TRUE(runLua("result = test.A(2)"));
    auto a2 = result<std::shared_ptr<A>>();
    EXPECT_EQ(2, a2->x);
}

TEST_F(LuaBridgeTest, StdSharedPtrDerived)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("test")
            .beginClass<A>("A")
                .addConstructorFrom<std::shared_ptr<A>, void(*)(int)>()
            .endClass()
            .deriveClass<B, A>("B")
                .addConstructorFrom<std::shared_ptr<B>, void(*)(int)>()
            .endClass()
        .endNamespace();

    auto b = std::make_shared<B>(1);
    luabridge::setGlobal(L, b, "b");

    {
        auto a1 = *luabridge::getGlobal<std::shared_ptr<A>>(L, "b");
        EXPECT_EQ(1, a1->x);

        EXPECT_TRUE(runLua("result = b"));
        auto a2 = result<std::shared_ptr<A>>();
        EXPECT_EQ(1, a2->x);

        EXPECT_TRUE(runLua("result = test.B(2)"));
        auto a3 = result<std::shared_ptr<A>>();
        EXPECT_EQ(2, a3->x);
    }

    {
        auto b1 = *luabridge::getGlobal<std::shared_ptr<B>>(L, "b");
        EXPECT_EQ(1, b1->x);

        EXPECT_TRUE(runLua("result = b"));
        auto b2 = result<std::shared_ptr<B>>();
        EXPECT_EQ(1, b2->x);

        EXPECT_TRUE(runLua("result = test.B(2)"));
        auto b3 = result<std::shared_ptr<B>>();
        EXPECT_EQ(2, b3->x);
    }
}

TEST_F(LuaBridgeTest, StdSharedPtrDerivedPolymorphic)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("test")
            .beginClass<VirtualA>("A")
                .addConstructorFrom<std::shared_ptr<VirtualA>, void(*)(int)>()
                .addFunction("myNameIs", &VirtualA::myNameIs)
            .endClass()
            .deriveClass<VirtualB, VirtualA>("B")
                .addConstructorFrom<std::shared_ptr<VirtualB>, void(*)(int)>()
                .addFunction("myNameIs", &VirtualB::myNameIs)
            .endClass()
            .deriveClass<VirtualC, VirtualB>("C")
                .addConstructorFrom<std::shared_ptr<VirtualC>, void(*)(int)>()
                .addFunction("myNameIs", &VirtualC::myNameIs)
            .endClass()
        .endNamespace();

    auto b = std::make_shared<VirtualB>(1);
    luabridge::setGlobal(L, b, "b");

    {
        auto a1 = *luabridge::getGlobal<std::shared_ptr<VirtualA>>(L, "b");
        EXPECT_EQ(1, a1->x);

        EXPECT_TRUE(runLua("result = b"));
        auto a2 = result<std::shared_ptr<VirtualA>>();
        EXPECT_EQ(1, a2->x);

        EXPECT_TRUE(runLua("result = test.B(2)"));
        auto a3 = result<std::shared_ptr<VirtualA>>();
        EXPECT_EQ(2, a3->x);
    }

    {
        auto b1 = *luabridge::getGlobal<std::shared_ptr<VirtualB>>(L, "b");
        EXPECT_EQ(1, b1->x);

        EXPECT_TRUE(runLua("result = b"));
        auto b2 = result<std::shared_ptr<VirtualB>>();
        EXPECT_EQ(1, b2->x);

        EXPECT_TRUE(runLua("result = test.B(2)"));
        auto b3 = result<std::shared_ptr<VirtualB>>();
        EXPECT_EQ(2, b3->x);
    }

    {
#if LUABRIDGE_HAS_EXCEPTIONS
        ASSERT_ANY_THROW(luabridge::getGlobal<std::shared_ptr<VirtualC>>(L, "b"));
#else
        // TODO - Userdata::get is still not safe to use outside of lua, as it will call the panic handler.
        //auto c1 = luabridge::getGlobal<std::shared_ptr<VirtualC>>(L, "b");
        //EXPECT_FALSE(!!c1);
#endif
    }

#if LUABRIDGE_ON_RAVI
    return; // TODO - Ravi asserts on the lua state being invalid because of the previous exception
#endif

    EXPECT_TRUE(runLua("local x = test.A(2); result = x:myNameIs()"));
    auto x1 = result<std::string>();
    EXPECT_EQ("VirtualA", x1);

    EXPECT_TRUE(runLua("local x = test.B(2); result = x:myNameIs()"));
    auto x2 = result<std::string>();
    EXPECT_EQ("VirtualB", x2);

    EXPECT_TRUE(runLua("local x = test.C(2); result = x:myNameIs()"));
    auto x3 = result<std::string>();
    EXPECT_EQ("VirtualC", x3);
}

namespace {
class TestClassInner : public std::enable_shared_from_this<TestClassInner>
{
public:
    inline int getValue() const { return 42; }
};

class TestClassOuter
{
public:
    TestClassOuter()
        : sharedPointerNested(std::make_shared<TestClassInner>())
    {
    }

    inline std::shared_ptr<TestClassInner> getNested() const { return sharedPointerNested; }

    TestClassInner valueNested;
    std::shared_ptr<TestClassInner> sharedPointerNested;
};
} // namespace

TEST_F(LuaBridgeTest, StdSharedPtrAsProperty)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("test")
            .beginClass<TestClassInner>("TestClassInner")
                .addFunction("getValue", &TestClassInner::getValue)
            .endClass()
            .beginClass<TestClassOuter>("TestClassOuter")
                .addConstructor<void()>()
                .addProperty("getNested", &TestClassOuter::getNested)
                .addProperty("valueNested", &TestClassOuter::valueNested)
                .addProperty("sharedPointerNested", &TestClassOuter::sharedPointerNested)
            .endClass()
        .endNamespace();

    ASSERT_TRUE(runLua("local x = test.TestClassOuter(); result = x.getNested:getValue()"));
    EXPECT_EQ(42, result<int>());

    ASSERT_TRUE(runLua("local x = test.TestClassOuter(); result = x.valueNested:getValue()"));
    EXPECT_EQ(42, result<int>());

    ASSERT_TRUE(runLua("local x = test.TestClassOuter(); result = x.sharedPointerNested:getValue()"));
    EXPECT_EQ(42, result<int>());
}

TEST_F(LuaBridgeTest, SharedPtrNoEnableSharedFromThis)
{
    using TestC = TestClass<int>;

    luabridge::getGlobalNamespace(L)
        .beginNamespace("ns")
            .beginClass<TestC>("TestClass")
                .addFunction("getValue", &TestC::getValue)
                .addFunction("getValueConst", &TestC::getValueConst)
            .endClass()
        .endNamespace();

    {
        std::shared_ptr<TestC> a = std::make_shared<TestC>(42);
        auto result = luabridge::setGlobal(L, a, "a");
        ASSERT_TRUE(result);
    }

    runLua("result = a");
    ASSERT_TRUE(result().isInstance<TestC*>());
    ASSERT_TRUE(result().isInstance<const TestC*>());
    ASSERT_TRUE(result().isInstance<TestC&>());
    ASSERT_TRUE(result().isInstance<const TestC&>());

    auto x1 = result<TestC*>();
    EXPECT_EQ(42, x1->getValue());

    auto x2 = result<const TestC*>();
    EXPECT_EQ(42, x2->getValueConst());

    auto& x3 = result<TestC&>();
    EXPECT_EQ(42, x3.getValue());

    auto x4 = result<const TestC&>();
    EXPECT_EQ(42, x4.getValueConst());
}

namespace {
class BoomyClass
{
public:
    BoomyClass() = default;

    const char* nullconst() { return nullptr; }
    char* null() { return nullptr; }

    std::string twochars(const char* one, const char* two = nullptr)
    {
        std::string one_two = one;

        if (two)
            one_two += two;

        return one_two;
   }
};
} // namespace

TEST_F(LuaBridgeTest, PointersBoom)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("test")
            .beginClass<BoomyClass>("BoomyClass")
                .addConstructor<void(*)(void)>()
                .addFunction("nullconst", &BoomyClass::nullconst)
                .addFunction("null", &BoomyClass::null)
                .addFunction("twochars", &BoomyClass::twochars)
            .endClass()
        .endNamespace();

    runLua(R"(
        local foo = test.BoomyClass()
        result = foo:nullconst()
    )");
    EXPECT_TRUE(result().isNil());

    runLua(R"(
        local foo = test.BoomyClass()
        result = foo:null()
    )");
    EXPECT_TRUE(result().isNil());

    runLua(R"(
        local foo = test.BoomyClass()
        result = foo:twochars("aaa", nil)
    )");
    ASSERT_TRUE(result().isString());
    EXPECT_EQ("aaa", result<std::string>());

    runLua(R"(
        local foo = test.BoomyClass()
        result = foo:twochars("aaa", "bbb")
    )");
    ASSERT_TRUE(result().isString());
    EXPECT_EQ("aaabbb", result<std::string>());
}

namespace {
class ConstructibleFromBool
{
public:
    ConstructibleFromBool(bool param) : val_(param) {}

    bool val() const { return val_; }

private:
    bool val_;
};
} // namespace

TEST_F(LuaBridgeTest, BooleanNoValue)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("test")
            .beginClass<ConstructibleFromBool>("ConstructibleFromBool")
                .addConstructor<void(*)(bool)>()
                .addFunction("val", &ConstructibleFromBool::val)
            .endClass()
        .endNamespace();

    runLua("local foo = test.ConstructibleFromBool(); result = foo:val()");
    ASSERT_TRUE(result().isBool());
    EXPECT_FALSE(result<bool>());
}

#if LUABRIDGE_HAS_EXCEPTIONS
TEST_F(LuaBridgeTest, StackExceptionWithMessage)
{
    try {
        auto result = luabridge::Stack<std::string>::get(L, 1);
        result.value();
        EXPECT_FALSE(true);
    } catch (const std::exception& e) {
        EXPECT_STREQ("The lua object can't be cast to desired type", e.what());
    }
}

TEST_F(LuaBridgeTest, ExpectedExceptionWithoutMessage)
{
    try {
        luabridge::Expected<std::string, int> result = luabridge::makeUnexpected(5);
        result.value();
        EXPECT_FALSE(true);
    } catch (const std::exception& e) {
        EXPECT_STREQ("Bad access to expected value", e.what());
    }
}

namespace {
template <class... Args>
std::string call_callback_get_exception(const luabridge::LuaRef& fn, Args&&... args)
{
    assert(fn.isCallable());

    try {
        fn(std::forward<Args>(args)...);
        return {};
    } catch (const std::exception& e) {
        return e.what();
    }
}
} // namespace

TEST_F(LuaBridgeTest, Exception)
{
    luabridge::LuaRef cb1(L);
    luabridge::LuaRef cb2(L);

    luabridge::getGlobalNamespace(L)
        .beginNamespace("ns")
            .addProperty("cb1", &cb1)
            .addProperty("cb2", &cb2)
        .endNamespace();

    auto text = R"(
        function ns.cb1()
            local x = 42
            return x - 1337
        end

        function ns.cb2()
            local y = 42
            this.will.fail()
            return y - 1337
        end
    )";

    EXPECT_TRUE(runLua(text));

    EXPECT_EQ("", call_callback_get_exception(cb1));

    const auto error = call_callback_get_exception(cb2);
    EXPECT_NE(std::string::npos, error.find("The lua function invocation raised an error"));
    EXPECT_NE(std::string::npos, error.find("attempt to index"));
    EXPECT_NE(std::string::npos, error.find(" nil "));
}
#endif
