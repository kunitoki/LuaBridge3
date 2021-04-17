// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include <exception>
#include <functional>
#include <map>
#include <memory>

struct ClassTests : TestBase
{
    template<class T>
    T variable(const std::string& name)
    {
        runLua("result = " + name);
        return result<T>();
    }
};

namespace {

struct EmptyBase
{
};

template<class T, class Base>
struct Class : Base
{
    Class() : data() {}

    Class(T data) : data(data) {}

    static Class<T, Base> staticFunction(Class<T, Base> value) { return value; }

    std::string toString() const
    {
        std::ostringstream stream;
        stream << data;
        return stream.str();
    }

    bool operator==(const Class<T, Base>& rhs) const { return data == rhs.data; }

    bool operator<(const Class<T, Base>& rhs) const { return data < rhs.data; }

    bool operator<=(const Class<T, Base>& rhs) const { return data <= rhs.data; }

    Class<T, Base> operator+(const Class<T, Base>& rhs) const
    {
        return Class<T, Base>(data + rhs.data);
    }

    Class<T, Base> operator-(const Class<T, Base>& rhs) const
    {
        return Class<T, Base>(data - rhs.data);
    }

    Class<T, Base> operator*(const Class<T, Base>& rhs) const
    {
        return Class<T, Base>(data * rhs.data);
    }

    Class<T, Base> operator/(const Class<T, Base>& rhs) const
    {
        return Class<T, Base>(data / rhs.data);
    }

    Class<T, Base> operator%(const Class<T, Base>& rhs) const
    {
        return Class<T, Base>(data % rhs.data);
    }

    Class<T, Base> operator()(T param) { return Class<T, Base>(param); }

    int len() const { return data; }

    Class<T, Base> negate() const { return Class<T, Base>(-data); }

    T method(T value) { return value; }

    T methodState(T value, lua_State*) { return value; }

    T constMethod(T value) const { return value; }

    T getData() const { return data; }

    void setData(T data) { this->data = data; }

    T getDataState(lua_State*) const { return data; }

    void setDataState(T data, lua_State*) { this->data = data; }

    mutable T data;
    static T staticData;
};

template<class T, class Base>
T Class<T, Base>::staticData = {};

} // namespace

TEST_F(ClassTests, IsInstance)
{
    using BaseClass = Class<int, EmptyBase>;
    using OtherClass = Class<float, EmptyBase>;
    using DerivedClass = Class<float, BaseClass>;

    luabridge::getGlobalNamespace(L)
        .beginClass<BaseClass>("BaseClass")
        .endClass()
        .deriveClass<DerivedClass, BaseClass>("DerivedClass")
        .endClass()
        .beginClass<OtherClass>("OtherClass")
        .endClass();

    BaseClass base;
    std::error_code ec1;
    luabridge::push(L, base, ec1);

    DerivedClass derived;
    std::error_code ec2;
    luabridge::push(L, derived, ec2);

    OtherClass other;
    std::error_code ec3;
    luabridge::push(L, other, ec3);

    ASSERT_TRUE(luabridge::isInstance<BaseClass>(L, -3));
    ASSERT_FALSE(luabridge::isInstance<DerivedClass>(L, -3)); // BaseClass is not DerivedClass
    ASSERT_FALSE(luabridge::isInstance<OtherClass>(L, -3));

    ASSERT_TRUE(luabridge::isInstance<BaseClass>(L, -2));
    ASSERT_TRUE(luabridge::isInstance<DerivedClass>(L, -2)); // DerivedClass is BaseClass
    ASSERT_FALSE(luabridge::isInstance<OtherClass>(L, -2));

    ASSERT_FALSE(luabridge::isInstance<BaseClass>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<DerivedClass>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<OtherClass>(L, -1));
}

TEST_F(ClassTests, PassingUnregisteredClassToLuaThrows)
{
    using Unregistered = Class<int, EmptyBase>;

    runLua("function process_fn (value) end");

    auto process_fn = luabridge::getGlobal(L, "process_fn");
    ASSERT_TRUE(process_fn.isFunction());

    Unregistered value(1);
    const Unregistered constValue(2);
    
#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(process_fn(value), std::exception);
    ASSERT_THROW(process_fn(constValue), std::exception);
    ASSERT_THROW(process_fn(&value), std::exception);
    ASSERT_THROW(process_fn(&constValue), std::exception);
#else
    EXPECT_FALSE(process_fn(value));
    EXPECT_FALSE(process_fn(constValue));
    EXPECT_FALSE(process_fn(&value));
    EXPECT_FALSE(process_fn(&constValue));
#endif
}

TEST_F(ClassTests, PassWrongClassFromLuaThrows)
{
    using Right = Class<int, EmptyBase>;
    using WrongBase = Class<float, EmptyBase>;
    using Wrong = Class<int, WrongBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Right>("Right")
        .endClass()
        .beginClass<WrongBase>("WrongBase")
        .endClass()
        .beginClass<Wrong>("Wrong")
        .addConstructor<void (*)(int)>()
        .endClass()
        .addFunction("processRight", &Right::staticFunction);

    // bad argument #1 to 'processRight' (Right expected, got Wrong)
#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("result = processRight (Wrong (5))"), std::exception);
#else
    ASSERT_FALSE(runLua("result = processRight (Wrong (5))"));
#endif

    ASSERT_TRUE(result().isNil());
}

TEST_F(ClassTests, PassDerivedClassInsteadOfBase)
{
    using Base = Class<int, EmptyBase>;
    using Derived = Class<float, Base>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Base>("Base")
        .endClass()
        .deriveClass<Derived, Base>("Derived")
        .addConstructor<void (*)(float)>()
        .endClass()
        .addFunction("processBase", &Base::staticFunction);

    runLua("result = processBase (Derived (3.14))");
    ASSERT_EQ(0, result<Base>().data);
}

namespace {

template<class T, class Base>
T processNonConst(Class<T, Base>* object)
{
    return object->data;
}

} // namespace

TEST_F(ClassTests, PassConstClassInsteadOfNonConstThrows)
{
    using Base = Class<int, EmptyBase>;
    using Derived = Class<float, Base>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Base>("Base")
        .endClass()
        .deriveClass<Derived, Base>("Derived")
        .endClass()
        .addFunction("processNonConst", &processNonConst<float, Base>);

    const Derived constObject(1.2f);
    luabridge::setGlobal(L, &constObject, "constObject");

    // bad argument #1 to 'processNonConst' (Derived expected, got const Derived)
#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("result = processNonConst (constObject)"), std::exception);
#else
    ASSERT_FALSE(runLua("result = processNonConst (constObject)"));
#endif

    ASSERT_TRUE(result().isNil());
}

TEST_F(ClassTests, PassOtherTypeInsteadOfNonConstThrows)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>() // Show that it does't matter
        .endClass()
        .addFunction("processNonConst", &processNonConst<int, EmptyBase>);

    // bad argument #1 to 'processNonConst' (Int expected, got number)
#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("result = processNonConst (1)"), std::exception);
#else
    ASSERT_FALSE(runLua("result = processNonConst (1)"));
#endif

    ASSERT_TRUE(result().isNil());
}

TEST_F(ClassTests, PassRegisteredClassInsteadOfUnregisteredThrows)
{
    using Int = Class<int, EmptyBase>;
    using Float = Class<float, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Float>("Float")
        .addConstructor<void (*)(float)>()
        .endClass()
        .addFunction("processUnregisteredInt", &Int::staticFunction);

    // bad argument #1 to 'processUnregisteredInt' (unregistered class expected, got Float)
#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("result = processUnregisteredInt (Float (1.2))"), std::exception);
#else
    ASSERT_FALSE(runLua("result = processUnregisteredInt (Float (1.2))"));
#endif

    ASSERT_TRUE(result().isNil());
}

namespace {

Class<int, EmptyBase>& returnRef()
{
    static Class<int, EmptyBase> value(1);
    return value;
}

const Class<int, EmptyBase>& returnConstRef()
{
    return returnRef();
}

Class<int, EmptyBase>* returnPtr()
{
    return &returnRef();
}

const Class<int, EmptyBase>* returnConstPtr()
{
    return &returnConstRef();
}

Class<int, EmptyBase> returnValue()
{
    return Class<int, EmptyBase>(2);
}

void addHelperFunctions(lua_State* L)
{
    luabridge::getGlobalNamespace(L)
        .addFunction("returnRef", &returnRef)
        .addFunction("returnConstRef", &returnConstRef)
        .addFunction("returnPtr", &returnPtr)
        .addFunction("returnConstPtr", &returnConstPtr)
        .addFunction("returnValue", &returnValue);
}

} // namespace

TEST_F(ClassTests, PassingUnregisteredClassFromLuaThrows)
{
    using Unregistered = Class<int, EmptyBase>;

    addHelperFunctions(L);

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("result = returnRef ()"), std::exception);
    ASSERT_THROW(runLua("result = returnConstRef ()"), std::exception);
    ASSERT_THROW(runLua("result = returnPtr ()"), std::exception);
    ASSERT_THROW(runLua("result = returnConstPtr ()"), std::exception);
    ASSERT_THROW(runLua("result = returnValue ()"), std::exception);
#else
    ASSERT_FALSE(runLua("result = returnRef ()"));
    ASSERT_FALSE(runLua("result = returnConstRef ()"));
    ASSERT_FALSE(runLua("result = returnPtr ()"));
    ASSERT_FALSE(runLua("result = returnConstPtr ()"));
    ASSERT_FALSE(runLua("result = returnValue ()"));
#endif
}

TEST_F(ClassTests, DeriveFromUnregisteredClassThrows)
{
#if LUABRIDGE_HAS_EXCEPTIONS
    using Base = Class<int, EmptyBase>;
    using Derived = Class<float, Base>;

    ASSERT_THROW((luabridge::getGlobalNamespace(L).deriveClass<Derived, Base>("Derived")), std::exception);
    ASSERT_EQ(1, lua_gettop(L));
#endif
}

struct ClassFunctions : ClassTests
{
};

TEST_F(ClassFunctions, MemberFunctions)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addFunction("method", &Int::method)
        .endClass();

    addHelperFunctions(L);

    runLua("result = returnRef ():method (1)");
    ASSERT_EQ(1, result<int>());

    runLua("result = returnConstRef ().method"); // Don't call, just get
    ASSERT_TRUE(result().isNil());

    runLua("result = returnPtr ():method (2)");
    ASSERT_EQ(2, result<int>());

    runLua("result = returnConstPtr ().method"); // Don't call, just get
    ASSERT_TRUE(result().isNil());

    runLua("result = returnValue ():method (3)");
    ASSERT_EQ(3, result<int>());
}

TEST_F(ClassFunctions, MemberFunctions_PassState)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addFunction("method", &Int::methodState)
        .endClass();

    addHelperFunctions(L);

    runLua("result = returnRef ():method (1)");
    ASSERT_EQ(1, result<int>());

    runLua("result = returnConstRef ().method"); // Don't call, just get
    ASSERT_TRUE(result().isNil());

    runLua("result = returnPtr ():method (2)");
    ASSERT_EQ(2, result<int>());

    runLua("result = returnConstPtr ().method"); // Don't call, just get
    ASSERT_TRUE(result().isNil());

    runLua("result = returnValue ():method (3)");
    ASSERT_EQ(3, result<int>());
}

TEST_F(ClassFunctions, ConstMemberFunctions)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addFunction("constMethod", &Int::constMethod)
        .endClass();

    addHelperFunctions(L);

    runLua("result = returnRef ():constMethod (1)");
    ASSERT_EQ(1, result<int>());

    runLua("result = returnConstRef ():constMethod (2)");
    ASSERT_EQ(2, result<int>());

    runLua("result = returnPtr ():constMethod (3)");
    ASSERT_EQ(3, result<int>());

    runLua("result = returnConstPtr ():constMethod (4)");
    ASSERT_EQ(4, result<int>());

    runLua("result = returnValue ():constMethod (5)");
    ASSERT_EQ(5, result<int>());
}

namespace {

template<class T, class Base>
T proxyFunction(Class<T, Base>* object, T value)
{
    object->data = value;
    return value;
}

template<class T, class Base>
T proxyFunctionState(Class<T, Base>* object, T value, lua_State*)
{
    object->data = value;
    return value;
}

template<class T, class Base>
T proxyConstFunction(const Class<T, Base>* object, T value)
{
    return value;
}

int proxyCFunctionState(lua_State* L)
{
    using Int = Class<int, EmptyBase>;
    
    auto ref = luabridge::LuaRef::fromStack(L, 1);
    if (!ref.isUserdata() || !ref.isInstance<Int>()) {
        return 0;
    }
    
    auto arg = luabridge::LuaRef::fromStack(L, 2);
    if (!arg.isNumber()) {
        return 0;
    }
    
    std::error_code ec;
    luabridge::push(L, arg.cast<int>() + 1000, ec);
    return 1;
}

} // namespace

TEST_F(ClassFunctions, ProxyFunctions)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addFunction("method", &proxyFunction<int, EmptyBase>)
        .endClass();

    addHelperFunctions(L);

    runLua("result = returnRef ():method (1)");
    ASSERT_EQ(1, result<int>());

    runLua("result = returnConstRef ().method"); // Don't call, just get
    ASSERT_TRUE(result().isNil());

    runLua("result = returnPtr ():method (2)");
    ASSERT_EQ(2, result<int>());

    runLua("result = returnConstPtr ().method"); // Don't call, just get
    ASSERT_TRUE(result().isNil());

    runLua("result = returnValue ():method (3)");
    ASSERT_EQ(3, result<int>());
}

TEST_F(ClassFunctions, ProxyFunctions_PassState)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addFunction("method", &proxyFunctionState<int, EmptyBase>)
        .endClass();

    addHelperFunctions(L);

    runLua("result = returnRef ():method (1)");
    ASSERT_EQ(1, result<int>());

    runLua("result = returnConstRef ().method"); // Don't call, just get
    ASSERT_TRUE(result().isNil());

    runLua("result = returnPtr ():method (2)");
    ASSERT_EQ(2, result<int>());

    runLua("result = returnConstPtr ().method"); // Don't call, just get
    ASSERT_TRUE(result().isNil());

    runLua("result = returnValue ():method (3)");
    ASSERT_EQ(3, result<int>());
}

TEST_F(ClassFunctions, ConstProxyFunctions)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addFunction("constMethod", &proxyConstFunction<int, EmptyBase>)
        .endClass();

    addHelperFunctions(L);

    runLua("result = returnRef ():constMethod (1)");
    ASSERT_EQ(1, result<int>());

    runLua("result = returnConstRef ():constMethod (2)");
    ASSERT_EQ(2, result<int>());

    runLua("result = returnPtr ():constMethod (3)");
    ASSERT_EQ(3, result<int>());

    runLua("result = returnConstPtr ():constMethod (4)");
    ASSERT_EQ(4, result<int>());

    runLua("result = returnValue ():constMethod (5)");
    ASSERT_EQ(5, result<int>());
}

TEST_F(ClassFunctions, ProxyCFunction)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addFunction("method", &proxyCFunctionState)
        .endClass();

    addHelperFunctions(L);

    runLua("result = returnRef ():method (1000)");
    ASSERT_EQ(2000, result<int>());
}

TEST_F(ClassFunctions, StdFunctions)
{
    using Int = Class<int, EmptyBase>;

    auto sharedData = std::make_shared<int>();
    std::weak_ptr<int> data = sharedData; // Check __gc meta-method

    std::function<int(Int*, int)> function = [sharedData](Int* object, int value) {
        object->data = value;
        return value;
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addFunction("method", std::move(function))
        .endClass();

    function = nullptr;
    sharedData.reset();
    ASSERT_FALSE(data.expired());

    addHelperFunctions(L);

    runLua("result = returnRef ():method (1)");
    ASSERT_EQ(1, result<int>());

    runLua("result = returnConstRef ().method"); // Don't call, just get
    ASSERT_TRUE(result().isNil());

    runLua("result = returnPtr ():method (2)");
    ASSERT_EQ(2, result<int>());

    runLua("result = returnConstPtr ().method"); // Don't call, just get
    ASSERT_TRUE(result().isNil());

    runLua("result = returnValue ():method (3)");
    ASSERT_EQ(3, result<int>());

    runLua("result = nil");
    lua_close(L); // Force garbage collection
    L = nullptr;

    ASSERT_TRUE(data.expired());
}

TEST_F(ClassFunctions, StdFunctions_PassState)
{
    using Int = Class<int, EmptyBase>;

    auto sharedData = std::make_shared<int>();
    std::weak_ptr<int> data = sharedData; // Check __gc meta-method

    std::function<int(Int*, int, lua_State*)> function =
        [sharedData](Int* object, int value, lua_State*) {
            object->data = value;
            return value;
        };

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addFunction("method", std::move(function))
        .endClass();

    function = nullptr;
    sharedData.reset();
    ASSERT_FALSE(data.expired());

    addHelperFunctions(L);

    runLua("result = returnRef ():method (1)");
    ASSERT_EQ(1, result<int>());

    runLua("result = returnConstRef ().method"); // Don't call, just get
    ASSERT_TRUE(result().isNil());

    runLua("result = returnPtr ():method (2)");
    ASSERT_EQ(2, result<int>());

    runLua("result = returnConstPtr ().method"); // Don't call, just get
    ASSERT_TRUE(result().isNil());

    runLua("result = returnValue ():method (3)");
    ASSERT_EQ(3, result<int>());

    runLua("result = nil");
    lua_close(L); // Force garbage collection
    L = nullptr;

    ASSERT_TRUE(data.expired());
}

TEST_F(ClassFunctions, ConstStdFunctions)
{
    using Int = Class<int, EmptyBase>;

    auto sharedData = std::make_shared<int>();
    std::weak_ptr<int> data = sharedData; // Check __gc meta-method

    std::function<int(const Int*, int)> function = [sharedData](const Int* object, int value) {
        object->data = value;
        return value;
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addFunction("constMethod", std::move(function))
        .endClass();

    function = nullptr;
    sharedData.reset();
    ASSERT_FALSE(data.expired());

    addHelperFunctions(L);

    runLua("result = returnRef ():constMethod (1)");
    ASSERT_EQ(1, result<int>());

    runLua("result = returnConstRef ():constMethod (2)");
    ASSERT_EQ(2, result<int>());

    runLua("result = returnPtr ():constMethod (3)");
    ASSERT_EQ(3, result<int>());

    runLua("result = returnConstPtr ():constMethod (4)");
    ASSERT_EQ(4, result<int>());

    runLua("result = returnValue ():constMethod (5)");
    ASSERT_EQ(5, result<int>());

    runLua("result = nil");
    lua_close(L); // Force garbage collection
    L = nullptr;

    ASSERT_TRUE(data.expired());
}

struct ClassProperties : ClassTests
{
};

TEST_F(ClassProperties, FieldPointers)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addProperty("data", &Int::data, true)
        .endClass();

    runLua("result = Int (501)");
    ASSERT_TRUE(result()["data"].isNumber());
    ASSERT_EQ(501, result()["data"].cast<int>());

    runLua("result.data = 2");
    ASSERT_TRUE(result()["data"].isNumber());
    ASSERT_EQ(2, result()["data"].cast<int>());

    runLua("result = Int (42).data");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(42, result<int>());
}

TEST_F(ClassProperties, FieldPointers_ReadOnly)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addProperty("data", &Int::data, false)
        .endClass();

    runLua("result = Int (501)");
    ASSERT_TRUE(result()["data"].isNumber());
    ASSERT_EQ(501, result()["data"].cast<int>());

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("result.data = 2"), std::exception);
#else
    ASSERT_FALSE(runLua("result.data = 2"));
#endif

    runLua("result = Int (42).data");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(42, result<int>());
}

TEST_F(ClassProperties, MemberFunctions)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addProperty("data", &Int::getData, &Int::setData)
        .endClass();

    runLua("result = Int (501)");
    ASSERT_TRUE(result()["data"].isNumber());
    ASSERT_EQ(501, result()["data"].cast<int>());

    runLua("result.data = -2");
    ASSERT_TRUE(result()["data"].isNumber());
    ASSERT_EQ(-2, result()["data"].cast<int>());
}

TEST_F(ClassProperties, MemberFunctions_PassState)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addProperty("data", &Int::getDataState, &Int::setDataState)
        .endClass();

    runLua("result = Int (501)");
    ASSERT_TRUE(result()["data"].isNumber());
    ASSERT_EQ(501, result()["data"].cast<int>());

    runLua("result.data = -2");
    ASSERT_TRUE(result()["data"].isNumber());
    ASSERT_EQ(-2, result()["data"].cast<int>());
}

TEST_F(ClassProperties, MemberFunctions_ReadOnly)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addProperty("data", &Int::getData)
        .endClass();

    runLua("result = Int (501)");
    ASSERT_TRUE(result()["data"].isNumber());
    ASSERT_EQ(501, result()["data"].cast<int>());

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("result.data = -2"), std::exception);
#else
    ASSERT_FALSE(runLua("result.data = -2"));
#endif

    ASSERT_EQ(501, result()["data"].cast<int>());
}

TEST_F(ClassProperties, MemberFunctions_Derived)
{
    using Base = Class<std::string, EmptyBase>;
    using Derived = Class<int, Base>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Base>("Base")
        .addProperty("data", &Base::getData, &Base::setData)
        .endClass()
        .deriveClass<Derived, Base>("Derived")
        .endClass();

    Derived derived(12);
    derived.Base::data = "abc";
    luabridge::setGlobal(L, &derived, "derived");

    runLua("result = derived.data");
    ASSERT_TRUE(result().isString());
    ASSERT_EQ("abc", result<std::string>());

    runLua("derived.data = 5"); // Lua just casts integer to string
    ASSERT_EQ("5", derived.Base::data);
    ASSERT_EQ(12, derived.data);

    runLua("derived.data = '123'");
    ASSERT_EQ("123", derived.Base::data);
    ASSERT_EQ(12, derived.data);
}

TEST_F(ClassProperties, MemberFunctions_Overridden)
{
    using Base = Class<float, EmptyBase>;
    using Derived = Class<int, Base>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Base>("Base")
        .addProperty("data", &Base::getData, &Base::setData)
        .endClass()
        .deriveClass<Derived, Base>("Derived")
        .addProperty("data", &Derived::getData, &Derived::setData)
        .endClass();

    Derived derived(50);
    derived.Base::data = 1.23f;
    luabridge::setGlobal(L, static_cast<Base*>(&derived), "base");
    luabridge::setGlobal(L, &derived, "derived");

    runLua("result = base.data");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(1.23f, result<float>());

    runLua("result = derived.data");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(50, result<int>());

    runLua("base.data = -3.14");
    ASSERT_EQ(-3.14f, derived.Base::data);
    ASSERT_EQ(50, derived.data);

    runLua("derived.data = 7");
    ASSERT_EQ(-3.14f, derived.Base::data);
    ASSERT_EQ(7, derived.data);
}

namespace {

template<class T, class BaseClass>
T getData(const Class<T, BaseClass>* object)
{
    return object->data;
}

template<class T, class BaseClass>
void setData(Class<T, BaseClass>* object, T data)
{
    object->data = data;
}

} // namespace

TEST_F(ClassProperties, ProxyFunctions)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addProperty("data", &getData<int, EmptyBase>, &setData<int, EmptyBase>)
        .endClass();

    runLua("result = Int (501)");
    ASSERT_TRUE(result()["data"].isNumber());
    ASSERT_EQ(501, result()["data"].cast<int>());

    runLua("result.data = -2");
    ASSERT_TRUE(result()["data"].isNumber());
    ASSERT_EQ(-2, result()["data"].cast<int>());
}

TEST_F(ClassProperties, ProxyFunctions_ReadOnly)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addProperty("data", &getData<int, EmptyBase>)
        .endClass();

    runLua("result = Int (501)");
    ASSERT_TRUE(result()["data"].isNumber());
    ASSERT_EQ(501, result()["data"].cast<int>());

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("result.data = -2"), std::exception);
#else
    ASSERT_FALSE(runLua("result.data = -2"));
#endif

    ASSERT_EQ(501, result()["data"].cast<int>());
}

TEST_F(ClassProperties, ProxyFunctions_Derived)
{
    using Base = Class<std::string, EmptyBase>;
    using Derived = Class<int, Base>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Base>("Base")
        .addProperty("data", &getData<std::string, EmptyBase>, &setData<std::string, EmptyBase>)
        .endClass()
        .deriveClass<Derived, Base>("Derived")
        .endClass();

    Derived derived(12);
    derived.Base::data = "abc";
    luabridge::setGlobal(L, &derived, "derived");

    runLua("result = derived.data");
    ASSERT_TRUE(result().isString());
    ASSERT_EQ("abc", result<std::string>());

    runLua("derived.data = 5"); // Lua just casts integer to string
    ASSERT_EQ("5", derived.Base::data);
    ASSERT_EQ(12, derived.data);

    runLua("derived.data = '123'");
    ASSERT_EQ("123", derived.Base::data);
    ASSERT_EQ(12, derived.data);
}

TEST_F(ClassProperties, ProxyFunctions_Overridden)
{
    using Base = Class<float, EmptyBase>;
    using Derived = Class<int, Base>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Base>("Base")
        .addProperty("data", &getData<float, EmptyBase>, &setData<float, EmptyBase>)
        .endClass()
        .deriveClass<Derived, Base>("Derived")
        .addProperty("data", &getData<int, Base>, &setData<int, Base>)
        .endClass();

    Derived derived(50);
    derived.Base::data = 1.23f;
    luabridge::setGlobal(L, static_cast<Base*>(&derived), "base");
    luabridge::setGlobal(L, &derived, "derived");

    runLua("result = base.data");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(1.23f, result<float>());

    runLua("result = derived.data");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(50, result<int>());

    runLua("base.data = -3.14");
    ASSERT_EQ(-3.14f, derived.Base::data);
    ASSERT_EQ(50, derived.data);

    runLua("derived.data = 7");
    ASSERT_EQ(-3.14f, derived.Base::data);
    ASSERT_EQ(7, derived.data);
}

namespace {

template<class T, class BaseClass>
int getDataC(lua_State* L)
{
    auto objectRef = luabridge::LuaRef::fromStack(L, 1);
    auto* object = objectRef.cast<const Class<T, BaseClass>*>();
    std::error_code ec;
    luabridge::Stack<T>::push(L, object->data, ec);
    return 1;
}

template<class T, class BaseClass>
int setDataC(lua_State* L)
{
    auto objectRef = luabridge::LuaRef::fromStack(L, 1);
    auto* object = objectRef.cast<const Class<T, BaseClass>*>();
    auto valueRef = luabridge::LuaRef::fromStack(L, 2);
    T value = valueRef.cast<T>();
    object->data = value;
    return 0;
}

} // namespace

TEST_F(ClassProperties, ProxyCFunctions)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addProperty("data", &getDataC<int, EmptyBase>, &setDataC<int, EmptyBase>)
        .endClass();

    runLua("result = Int (501)");
    ASSERT_TRUE(result()["data"].isNumber());
    ASSERT_EQ(501, result()["data"].cast<int>());

    runLua("result.data = -2");
    ASSERT_TRUE(result()["data"].isNumber());
    ASSERT_EQ(-2, result()["data"].cast<int>());
}

TEST_F(ClassProperties, ProxyCFunctions_ReadOnly)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addProperty("data", &getDataC<int, EmptyBase>)
        .endClass();

    runLua("result = Int (501)");
    ASSERT_TRUE(result()["data"].isNumber());
    ASSERT_EQ(501, result()["data"].cast<int>());

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("result.data = -2"), std::exception);
#else
    ASSERT_FALSE(runLua("result.data = -2"));
#endif

    ASSERT_EQ(501, result()["data"].cast<int>());
}

TEST_F(ClassProperties, ProxyCFunctions_Derived)
{
    using Base = Class<std::string, EmptyBase>;
    using Derived = Class<int, Base>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Base>("Base")
        .addProperty("data", &getDataC<std::string, EmptyBase>, &setDataC<std::string, EmptyBase>)
        .endClass()
        .deriveClass<Derived, Base>("Derived")
        .endClass();

    Derived derived(12);
    derived.Base::data = "abc";
    luabridge::setGlobal(L, &derived, "derived");

    runLua("result = derived.data");
    ASSERT_TRUE(result().isString());
    ASSERT_EQ("abc", result<std::string>());

    runLua("derived.data = 5"); // Lua just casts integer to string
    ASSERT_EQ("5", derived.Base::data);
    ASSERT_EQ(12, derived.data);

    runLua("derived.data = '123'");
    ASSERT_EQ("123", derived.Base::data);
    ASSERT_EQ(12, derived.data);
}

TEST_F(ClassProperties, ProxyCFunctions_Overridden)
{
    using Base = Class<float, EmptyBase>;
    using Derived = Class<int, Base>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Base>("Base")
        .addProperty("data", &getDataC<float, EmptyBase>, &setDataC<float, EmptyBase>)
        .endClass()
        .deriveClass<Derived, Base>("Derived")
        .addProperty("data", &getData<int, Base>, &setData<int, Base>)
        .endClass();

    Derived derived(50);
    derived.Base::data = 1.23f;
    luabridge::setGlobal(L, static_cast<Base*>(&derived), "base");
    luabridge::setGlobal(L, &derived, "derived");

    runLua("result = base.data");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(1.23f, result<float>());

    runLua("result = derived.data");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(50, result<int>());

    runLua("base.data = -3.14");
    ASSERT_EQ(-3.14f, derived.Base::data);
    ASSERT_EQ(50, derived.data);

    runLua("derived.data = 7");
    ASSERT_EQ(-3.14f, derived.Base::data);
    ASSERT_EQ(7, derived.data);
}

TEST_F(ClassProperties, StdFunctions)
{
    using Int = Class<int, EmptyBase>;

    auto sharedGetterData = std::make_shared<int>();
    std::weak_ptr<int> getterData = sharedGetterData; // Check __gc meta-method

    auto sharedSetterData = std::make_shared<int>();
    std::weak_ptr<int> setterData = sharedGetterData; // Check __gc meta-method

    std::function<int(const Int*)> getter = [sharedGetterData](const Int* object) {
        return object->data;
    };

    std::function<void(Int*, int)> setter = [sharedSetterData](Int* object, int value) {
        object->data = value;
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addProperty("data", std::move(getter), std::move(setter))
        .endClass();

    getter = nullptr;
    sharedGetterData.reset();
    ASSERT_FALSE(getterData.expired());

    setter = nullptr;
    sharedSetterData.reset();
    ASSERT_FALSE(setterData.expired());

    runLua("result = Int (501)");
    ASSERT_EQ(501, result()["data"].cast<int>());

    runLua("result.data = -2");
    ASSERT_TRUE(result()["data"].isNumber());
    ASSERT_EQ(-2, result()["data"].cast<int>());

    runLua("result = nil");
    lua_close(L); // Force garbage collection
    L = nullptr;

    ASSERT_TRUE(getterData.expired());
    ASSERT_TRUE(setterData.expired());
}

TEST_F(ClassProperties, StdFunctions_ReadOnly)
{
    using Int = Class<int, EmptyBase>;

    auto sharedGetterData = std::make_shared<int>();
    std::weak_ptr<int> getterData = sharedGetterData; // Check __gc meta-method

    std::function<int(const Int*)> getter = [sharedGetterData](const Int* object) {
        return object->data;
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addProperty("data", std::move(getter))
        .endClass();

    getter = nullptr;
    sharedGetterData.reset();
    ASSERT_FALSE(getterData.expired());

    runLua("result = Int (501)");
    ASSERT_TRUE(result()["data"].isNumber());
    ASSERT_EQ(501, result()["data"].cast<int>());

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("result.data = -2"), std::exception);
#else
    ASSERT_FALSE(runLua("result.data = -2"));
#endif

    ASSERT_EQ(501, result()["data"].cast<int>());

    runLua("result = nil");
    lua_close(L); // Force garbage collection
    L = nullptr;

    ASSERT_TRUE(getterData.expired());
}

struct ClassStaticFunctions : ClassTests
{
};

TEST_F(ClassStaticFunctions, Functions)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addStaticFunction("static", &Int::staticFunction)
        .endClass();

    runLua("result = Int.static (Int (35))");
    ASSERT_EQ(35, result<Int>().data);
}

TEST_F(ClassStaticFunctions, Functions_Derived)
{
    using Base = Class<std::string, EmptyBase>;
    using Derived = Class<int, Base>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Base>("Base")
        .addConstructor<void (*)(std::string)>()
        .addStaticFunction("static", &Base::staticFunction)
        .endClass()
        .deriveClass<Derived, Base>("Derived")
        .endClass();

    runLua("result = Derived.static (Base ('abc'))");
    ASSERT_EQ("abc", result<Base>().data);
}

TEST_F(ClassStaticFunctions, Functions_Overridden)
{
    using Base = Class<std::string, EmptyBase>;
    using Derived = Class<int, Base>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Base>("Base")
        .addConstructor<void (*)(std::string)>()
        .addStaticFunction("staticFunction", &Base::staticFunction)
        .endClass()
        .deriveClass<Derived, Base>("Derived")
        .addConstructor<void (*)(int)>()
        .addStaticFunction("staticFunction", &Derived::staticFunction)
        .endClass();

    runLua("result = Base.staticFunction (Base ('abc'))");
    ASSERT_EQ("abc", result<Base>().data);

    runLua("result = Derived.staticFunction (Derived (123))");
    ASSERT_EQ(123, result<Derived>().data);
}

TEST_F(ClassStaticFunctions, StdFunctions)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addStaticFunction("static", std::function<Int(Int)>(&Int::staticFunction))
        .endClass();

    runLua("result = Int.static (Int (35))");
    ASSERT_EQ(35, result<Int>().data);
}

struct ClassStaticProperties : ClassTests
{
};

TEST_F(ClassStaticProperties, FieldPointers)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addStaticProperty("staticData", &Int::staticData, true)
        .endClass();

    Int::staticData = 10;

    runLua("result = Int.staticData");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(10, result<int>());

    runLua("Int.staticData = 20");
    ASSERT_EQ(20, Int::staticData);
}

TEST_F(ClassStaticProperties, FieldPointers_ReadOnly)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addStaticProperty("staticData", &Int::staticData, false)
        .endClass();

    Int::staticData = 10;

    runLua("result = Int.staticData");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(10, result<int>());

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("Int.staticData = 20"), std::exception);
#else
    ASSERT_FALSE(runLua("Int.staticData = 20"));
#endif

    ASSERT_EQ(10, Int::staticData);
}

TEST_F(ClassStaticProperties, FieldPointers_Derived)
{
    using Base = Class<float, EmptyBase>;
    using Derived = Class<int, Base>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Base>("Base")
        .addStaticProperty("staticData", &Base::staticData, true)
        .endClass()
        .deriveClass<Derived, Base>("Derived")
        .endClass();

    Base::staticData = 1.23f;
    Derived::staticData = 50;

    runLua("result = Derived.staticData");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(1.23f, result<float>());

    runLua("Derived.staticData = -3.14");
    ASSERT_EQ(-3.14f, Base::staticData);
    ASSERT_EQ(50, Derived::staticData);
}

TEST_F(ClassStaticProperties, FieldPointers_Overridden)
{
    using Base = Class<float, EmptyBase>;
    using Derived = Class<int, Base>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Base>("Base")
        .addStaticProperty("staticData", &Base::staticData, true)
        .endClass()
        .deriveClass<Derived, Base>("Derived")
        .addStaticProperty("staticData", &Derived::staticData, true)
        .endClass();

    Base::staticData = 1.23f;
    Derived::staticData = 50;

    runLua("result = Base.staticData");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(1.23f, result<float>());

    runLua("result = Derived.staticData");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(50, result<int>());

    runLua("Base.staticData = -3.14");
    ASSERT_EQ(-3.14f, Base::staticData);
    ASSERT_EQ(50, Derived::staticData);

    runLua("Derived.staticData = 7");
    ASSERT_EQ(-3.14f, Base::staticData);
    ASSERT_EQ(7, Derived::staticData);
}

struct ClassMetaMethods : ClassTests
{
};

TEST_F(ClassMetaMethods, __call)
{
    typedef Class<int, EmptyBase> Int;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addFunction("__call", &Int::operator())
        .endClass();

    runLua("result = Int (1) (-1)");
    ASSERT_TRUE(result().isUserdata());
    ASSERT_EQ(-1, result<Int>().data);

    runLua("result = Int (2) (5)");
    ASSERT_TRUE(result().isUserdata());
    ASSERT_EQ(5, result<Int>().data);
}

TEST_F(ClassMetaMethods, __tostring)
{
    typedef Class<int, EmptyBase> Int;
    typedef Class<std::string, EmptyBase> StringClass;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addFunction("__tostring", &Int::toString)
        .endClass()
        .beginClass<StringClass>("String")
        .addConstructor<void (*)(std::string)>()
        .addFunction("__tostring", &StringClass::toString)
        .endClass();

    runLua("result = tostring (Int (-123))");
    ASSERT_EQ("-123", result<std::string>());

#if LUA_VERSION_NUM >= 502
    // Lua 5.1 string.format doesn't use __tostring
    runLua("result = string.format ('%s%s', String ('abc'), Int (-123))");
    ASSERT_EQ("abc-123", result<std::string>());
#endif
}

TEST_F(ClassMetaMethods, __eq)
{
    typedef Class<int, EmptyBase> Int;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addFunction("__eq", &Int::operator==)
        .endClass();

    runLua("result = Int (1) == Int (1)");
    ASSERT_EQ(true, result<bool>());

    runLua("result = Int (1) ~= Int (1)");
    ASSERT_EQ(false, result<bool>());

    runLua("result = Int (1) == Int (2)");
    ASSERT_EQ(false, result<bool>());

    runLua("result = Int (1) ~= Int (2)");
    ASSERT_EQ(true, result<bool>());
}

TEST_F(ClassMetaMethods, __lt)
{
    typedef Class<int, EmptyBase> Int;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addFunction("__lt", &Int::operator<)
        .endClass();

    runLua("result = Int (1) < Int (1)");
    ASSERT_EQ(false, result<bool>());

    runLua("result = Int (1) < Int (2)");
    ASSERT_EQ(true, result<bool>());

    runLua("result = Int (2) < Int (1)");
    ASSERT_EQ(false, result<bool>());
}

TEST_F(ClassMetaMethods, __le)
{
    typedef Class<int, EmptyBase> Int;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addFunction("__le", &Int::operator<=)
        .endClass();

    runLua("result = Int (1) <= Int (1)");
    ASSERT_EQ(true, result<bool>());

    runLua("result = Int (1) <= Int (2)");
    ASSERT_EQ(true, result<bool>());

    runLua("result = Int (2) <= Int (1)");
    ASSERT_EQ(false, result<bool>());
}

TEST_F(ClassMetaMethods, __add)
{
    typedef Class<int, EmptyBase> Int;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addFunction("__add", &Int::operator+)
        .endClass();

    runLua("result = Int (1) + Int (2)");
    ASSERT_TRUE(result().isUserdata());
    ASSERT_EQ(3, result<Int>().data);
}

TEST_F(ClassMetaMethods, __sub)
{
    typedef Class<int, EmptyBase> Int;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addFunction("__sub", &Int::operator-)
        .endClass();

    runLua("result = Int (1) - Int (2)");
    ASSERT_TRUE(result().isUserdata());
    ASSERT_EQ(-1, result<Int>().data);
}

TEST_F(ClassMetaMethods, __mul)
{
    typedef Class<int, EmptyBase> Int;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addFunction("__mul", &Int::operator*)
        .endClass();

    runLua("result = Int (-2) * Int (-5)");
    ASSERT_TRUE(result().isUserdata());
    ASSERT_EQ(10, result<Int>().data);
}

TEST_F(ClassMetaMethods, __div)
{
    typedef Class<int, EmptyBase> Int;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addFunction("__div", &Int::operator/)
        .endClass();

    runLua("result = Int (10) / Int (2)");
    ASSERT_TRUE(result().isUserdata());
    ASSERT_EQ(5, result<Int>().data);
}

TEST_F(ClassMetaMethods, __mod)
{
    typedef Class<int, EmptyBase> Int;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addFunction("__mod", &Int::operator%)
        .endClass();

    runLua("result = Int (7) % Int (2)");
    ASSERT_TRUE(result().isUserdata());
    ASSERT_EQ(1, result<Int>().data);
}

TEST_F(ClassMetaMethods, __pow)
{
    typedef Class<int, EmptyBase> Int;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addFunction("__pow", &Int::operator-)
        .endClass();

    runLua("result = Int (5) ^ Int (2)");
    ASSERT_TRUE(result().isUserdata());
    ASSERT_EQ(3, result<Int>().data);
}

TEST_F(ClassMetaMethods, __unm)
{
    typedef Class<int, EmptyBase> Int;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addFunction("__unm", &Int::negate)
        .endClass();

    runLua("result = -Int (-3)");
    ASSERT_TRUE(result().isUserdata());
    ASSERT_EQ(3, result<Int>().data);
}

TEST_F(ClassMetaMethods, __concat)
{
    typedef Class<std::string, EmptyBase> String;

    luabridge::getGlobalNamespace(L)
        .beginClass<String>("String")
        .addConstructor<void (*)(std::string)>()
        .addFunction("__concat", &String::operator+)
        .endClass();

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("result = String ('a') + String ('b')"), std::exception);
#else
    ASSERT_FALSE(runLua("result = String ('a') + String ('b')"));
#endif

    runLua("result = String ('ab') .. String ('cd')");
    ASSERT_TRUE(result().isUserdata());
    ASSERT_EQ("abcd", result<String>().data);
}

TEST_F(ClassMetaMethods, __len)
{
    typedef Class<int, EmptyBase> Int;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addFunction("__len", &Int::len)
        .endClass();

    runLua("result = #Int (1)");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(1, result<int>());

    runLua("result = #Int (5)");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(5, result<int>());
}

namespace {

struct Table
{
    int index(const std::string& key)
    {
#if LUABRIDGE_HAS_EXCEPTIONS
        return map.at(key);
#else
        auto it = map.find(key);
        return it != map.end() ? it->second : -1;
#endif
    }

    void newIndex(const std::string& key, int value)
    {
        map.emplace(key, value);
    }

    std::map<std::string, int> map;
};

} // namespace

TEST_F(ClassMetaMethods, __index)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Table>("Table")
        .addFunction("__index", &Table::index)
        .endClass();

    Table t{{{"a", 1}, {"b", 2}}};

    luabridge::setGlobal(L, &t, "t");

    runLua("result = t.a");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(1, result<int>());

    runLua("result = t.b");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(2, result<int>());

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("result = t.c"), std::exception); // at ("c") throws
#else
    ASSERT_TRUE(runLua("result = t.c"));
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(-1, result<int>());
#endif
}

TEST_F(ClassMetaMethods, __newindex)
{
    typedef Class<int, EmptyBase> Int;

    luabridge::getGlobalNamespace(L)
        .beginClass<Table>("Table")
        .addFunction("__newindex", &Table::newIndex)
        .endClass();

    Table t;

    luabridge::setGlobal(L, &t, "t");

    runLua("t.a = 1\n"
           "t ['b'] = 2");

    ASSERT_EQ((std::map<std::string, int>{{"a", 1}, {"b", 2}}), t.map);
}

TEST_F(ClassMetaMethods, __gcForbidden)
{
    typedef Class<int, EmptyBase> Int;

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(luabridge::getGlobalNamespace(L)
                     .beginClass<Int>("Int")
                     .addFunction("__gc", &Int::method)
                     .endClass(),
                 std::exception);
#endif
}

TEST_F(ClassTests, EnclosedClassProperties)
{
    typedef Class<int, EmptyBase> Inner;
    typedef Class<Inner, EmptyBase> Outer;

    luabridge::getGlobalNamespace(L)
        .beginClass<Inner>("Inner")
        .addProperty("data", &Inner::data)
        .endClass()
        .beginClass<Outer>("Outer")
        .addProperty("data", &Outer::data)
        .endClass();

    Outer outer(Inner(0));
    luabridge::setGlobal(L, &outer, "outer");

    outer.data.data = 1;
    runLua("outer.data.data = 10");
    ASSERT_EQ(10, outer.data.data);

    runLua("result = outer.data.data");
    ASSERT_EQ(10, result<int>());
}

// namespace {

struct InnerClass
{
    ~InnerClass() { ++destructorCallCount; }

    static unsigned destructorCallCount;
};

unsigned InnerClass::destructorCallCount;

struct OuterClass
{
    OuterClass()
    {
#if LUABRIDGE_HAS_EXCEPTIONS
        throw std::runtime_error("Exception");
#endif
    }

    ~OuterClass() { ++destructorCallCount; }

    static unsigned destructorCallCount;
    InnerClass inner;
};

unsigned OuterClass::destructorCallCount;

//} // namespace

TEST_F(ClassTests, ConstructorWithReferences)
{
    struct InnerClass
    {
        InnerClass() = default;
    };

    struct OuterClass
    {
        OuterClass(const InnerClass& x) : y(x) {}
        
    private:
        InnerClass y;
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<InnerClass>("InnerClass")
            .addConstructor<void (*)()>()
        .endClass()
        .beginClass<OuterClass>("OuterClass")
            .addConstructor<void (*)(const InnerClass&)>()
        .endClass();

    runLua("x = InnerClass () result = OuterClass (x)");
}

TEST_F(ClassTests, DestructorIsNotCalledIfConstructorThrows)
{
#if LUABRIDGE_HAS_EXCEPTIONS
    luabridge::getGlobalNamespace(L)
        .beginClass<OuterClass>("OuterClass")
        .addConstructor<void (*)()>()
        .endClass();

    InnerClass::destructorCallCount = 0;
    OuterClass::destructorCallCount = 0;
    ASSERT_THROW(runLua("result = OuterClass ()"), std::exception);
    ASSERT_EQ(1, InnerClass::destructorCallCount);

    lua_close(L);
    L = nullptr;

    ASSERT_EQ(1, InnerClass::destructorCallCount);
    ASSERT_EQ(0, OuterClass::destructorCallCount);
#endif
}

TEST_F(ClassTests, DestructorIsCalledOnce)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<InnerClass>("InnerClass")
        .addConstructor<void (*)()>()
        .endClass();

    InnerClass::destructorCallCount = 0;
    runLua("result = InnerClass ()");

    lua_close(L);
    L = nullptr;

    ASSERT_EQ(1, InnerClass::destructorCallCount);
}

TEST_F(ClassTests, ConstructorTakesMoreThanEightArgs)
{
    struct WideClass
    {
        WideClass(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10)
        {
            a1_ = a1;
            a2_ = a2;
            a3_ = a3;
            a4_ = a4;
            a5_ = a5;
            a6_ = a6;
            a7_ = a7;
            a8_ = a8;
            a9_ = a9;
            a10_ = a10;
        }
        
        int a1_, a2_, a3_, a4_, a5_, a6_, a7_, a8_, a9_, a10_;
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<WideClass>("WideClass")
        .addConstructor<void (*)(int, int, int, int, int, int, int, int, int, int)>()
        .endClass();

    runLua("result = WideClass (1, 2, 3, 4, 5, 6, 7, 8, 9, 10)");

    ASSERT_TRUE(result().isUserdata());
    ASSERT_EQ(1, result<WideClass>().a1_);
    ASSERT_EQ(2, result<WideClass>().a2_);
    ASSERT_EQ(3, result<WideClass>().a3_);
    ASSERT_EQ(4, result<WideClass>().a4_);
    ASSERT_EQ(5, result<WideClass>().a5_);
    ASSERT_EQ(6, result<WideClass>().a6_);
    ASSERT_EQ(7, result<WideClass>().a7_);
    ASSERT_EQ(8, result<WideClass>().a8_);
    ASSERT_EQ(9, result<WideClass>().a9_);
    ASSERT_EQ(10, result<WideClass>().a10_);
}

TEST_F(ClassTests, MethodTakesMoreThanEightArgs)
{
    struct WideClass
    {
        WideClass() = default;
        
        void testLotsOfArgs(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10)
        {
            a1_ = a1;
            a2_ = a2;
            a3_ = a3;
            a4_ = a4;
            a5_ = a5;
            a6_ = a6;
            a7_ = a7;
            a8_ = a8;
            a9_ = a9;
            a10_ = a10;
        }
        
        int a1_, a2_, a3_, a4_, a5_, a6_, a7_, a8_, a9_, a10_;
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<WideClass>("WideClass")
        .addConstructor<void (*)()>()
        .addFunction("testLotsOfArgs", &WideClass::testLotsOfArgs)
        .endClass();

    runLua("result = WideClass () result:testLotsOfArgs (1, 2, 3, 4, 5, 6, 7, 8, 9, 10)");

    ASSERT_TRUE(result().isUserdata());
    ASSERT_EQ(1, result<WideClass>().a1_);
    ASSERT_EQ(2, result<WideClass>().a2_);
    ASSERT_EQ(3, result<WideClass>().a3_);
    ASSERT_EQ(4, result<WideClass>().a4_);
    ASSERT_EQ(5, result<WideClass>().a5_);
    ASSERT_EQ(6, result<WideClass>().a6_);
    ASSERT_EQ(7, result<WideClass>().a7_);
    ASSERT_EQ(8, result<WideClass>().a8_);
    ASSERT_EQ(9, result<WideClass>().a9_);
    ASSERT_EQ(10, result<WideClass>().a10_);
}

TEST_F(ClassTests, Factory)
{
    struct FactoryConstructibleClass
    {
        FactoryConstructibleClass() = default;
        FactoryConstructibleClass(int x) : x_(x) {}

        int x_ = 33;
    };

    {
        luabridge::getGlobalNamespace(L)
            .beginClass<FactoryConstructibleClass>("FactoryConstructibleClass")
            .addConstructor([](void* ptr) { return new(ptr) FactoryConstructibleClass(); })
            .addProperty("x", &FactoryConstructibleClass::x_)
            .endClass();

        runLua("obj = FactoryConstructibleClass (); result = obj.x");

        ASSERT_TRUE(result().isNumber());
        ASSERT_EQ(33, result<int>());
    }

    {
        luabridge::getGlobalNamespace(L)
            .beginClass<FactoryConstructibleClass>("FactoryConstructibleClass2")
            .addConstructor([](void* ptr, int x) { return new(ptr) FactoryConstructibleClass(x); })
            .addProperty("x", &FactoryConstructibleClass::x_)
            .endClass();

        runLua("obj = FactoryConstructibleClass2 (42); result = obj.x");

        ASSERT_TRUE(result().isNumber());
        ASSERT_EQ(42, result<int>());
    }

    {
        luabridge::getGlobalNamespace(L)
            .beginClass<FactoryConstructibleClass>("FactoryConstructibleClass3")
            .addConstructor([](void* ptr, lua_State* L) { return new(ptr) FactoryConstructibleClass(luaL_checkinteger(L, 2)); })
            .addProperty("x", &FactoryConstructibleClass::x_)
            .endClass();

        runLua("obj = FactoryConstructibleClass3 (42); result = obj.x");

        ASSERT_TRUE(result().isNumber());
        ASSERT_EQ(42, result<int>());
    }
}

class BaseExampleClass
{
public:
    BaseExampleClass() = default;
    
    virtual void virtualFunction(int arg) = 0;
    virtual int virtualCFunction(lua_State*) = 0;
    virtual void virtualFunctionConst(int arg) const = 0;
    virtual int virtualCFunctionConst(lua_State*) const = 0;

    void baseFunction(int arg) { baseFunction_ = arg; }
    int baseCFunction(lua_State*) { return baseCFunction_ = 1; }
    void baseFunctionConst(int arg) const { baseFunctionConst_ = arg; }
    int baseCFunctionConst(lua_State*) const { return baseCFunctionConst_ = 1; }

    int virtualFunction_ = 0;
    int virtualCFunction_ = 0;
    mutable int virtualFunctionConst_ = 0;
    mutable int virtualCFunctionConst_ = 0;
    int baseFunction_ = 0;
    int baseCFunction_ = 0;
    mutable int baseFunctionConst_ = 0;
    mutable int baseCFunctionConst_ = 0;
};

class DerivedExampleClass : public BaseExampleClass
{
public:
    DerivedExampleClass() = default;
    
    void virtualFunction(int arg) override
    {
        virtualFunction_ = arg;
    }

    int virtualCFunction(lua_State*) override
    {
        return virtualCFunction_ = 1;
    }

    void virtualFunctionConst(int arg) const override
    {
        virtualFunctionConst_ = arg;
    }

    int virtualCFunctionConst(lua_State*) const override
    {
        return virtualCFunctionConst_ = 1;
    }
};

TEST_F(ClassTests, NonVirtualMethodInBaseClassCannotBeExposed)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<DerivedExampleClass>("DerivedExampleClass")
        .addConstructor<void (*)()>()
        .addFunction("baseFunction", &BaseExampleClass::baseFunction)
        .addFunction("baseCFunction", &BaseExampleClass::baseCFunction)
        .addFunction("baseFunctionConst", &BaseExampleClass::baseFunctionConst)
        .addFunction("baseCFunctionConst", &BaseExampleClass::baseCFunctionConst)
        .addFunction("virtualFunction", &DerivedExampleClass::virtualFunction)
        .addFunction("virtualCFunction", &DerivedExampleClass::virtualCFunction)
        .addFunction("virtualFunctionConst", &DerivedExampleClass::virtualFunctionConst)
        .addFunction("virtualCFunctionConst", &DerivedExampleClass::virtualCFunctionConst)
        .endClass();

    runLua(R"(
        result = DerivedExampleClass ()
        result:baseFunction(1)
        result:baseCFunction()
        result:baseFunctionConst(1)
        result:baseCFunctionConst()
        result:virtualFunction(1)
        result:virtualCFunction()
        result:virtualFunctionConst(1)
        result:virtualCFunctionConst()
    )");

    ASSERT_TRUE(result().isUserdata());
    ASSERT_EQ(1, result<DerivedExampleClass>().baseFunction_);
    ASSERT_EQ(1, result<DerivedExampleClass>().baseCFunction_);
    ASSERT_EQ(1, result<DerivedExampleClass>().baseFunctionConst_);
    ASSERT_EQ(1, result<DerivedExampleClass>().baseCFunctionConst_);
    ASSERT_EQ(1, result<DerivedExampleClass>().virtualFunction_);
    ASSERT_EQ(1, result<DerivedExampleClass>().virtualCFunction_);
    ASSERT_EQ(1, result<DerivedExampleClass>().virtualFunctionConst_);
    ASSERT_EQ(1, result<DerivedExampleClass>().virtualCFunctionConst_);
}

TEST_F(ClassTests, NilCanBeConvertedToNullptrButNotToReference)
{
    struct X {};

    bool result = false, resultConst = false, called = false;
    
    luabridge::getGlobalNamespace(L)
        .addFunction("TakeNullptr", [&result](X* iAmNullptr) { result = (iAmNullptr == nullptr); })
        .addFunction("TakeConstNullptr", [&resultConst](const X* iAmNullptr) { resultConst = (iAmNullptr == nullptr); })
        .addFunction("TakeReference", [&called](const X& iAmNullptr) { called = true; })
        .beginClass<X>("X")
        .endClass();

    runLua("TakeNullptr(nil)");
    EXPECT_TRUE(result);
    
    runLua("TakeConstNullptr(nil)");
    EXPECT_TRUE(resultConst);

#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_THROW(runLua("TakeReference(nil)"), std::exception);
#else
    EXPECT_FALSE(runLua("TakeReference(nil)"));
#endif
    EXPECT_FALSE(called);
}
