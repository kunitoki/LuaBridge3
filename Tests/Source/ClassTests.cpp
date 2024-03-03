// https://github.com/kunitoki/LuaBridge3
// Copyright 2023, Lucio Asnaghi
// Copyright 2020, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <unordered_map>

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
    T getDataNoexcept() const noexcept { return data; }

    void setData(T data) { this->data = data; }
    void setDataNoexcept(T data) noexcept { this->data = data; }

    T getDataState(lua_State*) const { return data; }
    T getDataStateNoexcept(lua_State*) const noexcept { return data; }

    void setDataState(T data, lua_State*) { this->data = data; }
    void setDataStateNoexcept(T data, lua_State*) noexcept { this->data = data; }

    static T getStaticData() { return staticData; }
    static void setStaticData(T data) { staticData = data; }

    static T getStaticDataNoexcept() noexcept { return staticData; }
    static void setStaticDataNoexcept(T data) noexcept { staticData = data; }

    mutable T data;
    static T staticData;
    static const T staticConstData;
};

template <class T, class Base>
T Class<T, Base>::staticData = {};

template <class T, class Base>
const T Class<T, Base>::staticConstData = {};

template <class T>
class Foo
{
public:
    Foo() = default;

    Foo(const std::string& name)
        : name(name)
    {
    }

    const std::string& getName() const
    {
        return name;
    }

    static Foo& createRef(const std::string& name)
    {
        static Foo instance(name);
        return instance;
    }

    static const Foo& createConstRef(const std::string& name)
    {
        static Foo instance(name);
        return instance;
    }

private:
    std::string name;
};

template <class T>
class Bar
{
public:
    Bar() = default;

    void setFoo(const luabridge::LuaRef& ref)
    {
        if (ref.isInstance<Foo<T>>())
            foo = ref.cast<Foo<T>>().value();
        else
            foo = Foo<T>("undefined");
    }

    const std::string& getFooName() const
    {
        return foo.getName();
    }

private:
    Foo<T> foo;
};

} // namespace

TEST_F(ClassTests, Assignment)
{
    using BaseClass = Class<int, EmptyBase>;
    
    auto registeredClass = luabridge::getGlobalNamespace(L)
        .beginClass<BaseClass>("BaseClass");

    registeredClass.addConstructor<void(*)()>();
    
    registeredClass.endClass();
}

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
    auto result1 = luabridge::push(L, base);
    ASSERT_TRUE(result1);

    DerivedClass derived;
    auto result2 = luabridge::push(L, derived);
    ASSERT_TRUE(result2);

    OtherClass other;
    auto result3 = luabridge::push(L, other);
    ASSERT_TRUE(result3);

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

TEST_F(ClassTests, IsInstanceRef)
{
    struct A {};
    using Fool = Foo<A>;
    using Barn = Bar<A>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Fool>("Foo")
            .addConstructor<void(*)(const std::string&)>()
            .addStaticFunction("createRef", &Fool::createRef)
            .addFunction("__tostring", +[](const Fool& self) { return self.getName(); })
        .endClass()
        .beginClass<Barn>("Bar")
            .addConstructor<void(*)()>()
            .addFunction("setFoo", &Barn::setFoo)
            .addFunction("getFooName", &Barn::getFooName)
        .endClass();

    runLua(R"(
        local fooFirst = Foo('first')

        local bar = Bar()
        bar:setFoo(fooFirst)
        result = bar:getFooName() .. " " .. tostring(fooFirst)
    )");
    EXPECT_EQ("first first", result<std::string>());

    runLua(R"(
        local fooSecond = Foo.createRef('second')

        local bar = Bar()
        bar:setFoo(fooSecond)
        result = bar:getFooName() .. " " .. tostring(fooSecond)
    )");
    EXPECT_EQ("second second", result<std::string>());
}

TEST_F(ClassTests, IsInstanceConstRef)
{
    struct B {};
    using Fool = Foo<B>;
    using Barn = Bar<B>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Fool>("Foo")
            .addConstructor<void(*)(const std::string&)>()
            .addStaticFunction("createConstRef", &Fool::createConstRef)
            .addFunction("__tostring", +[](const Fool& self) { return self.getName(); })
        .endClass()
        .beginClass<Barn>("Bar")
            .addConstructor<void(*)()>()
            .addFunction("setFoo", &Barn::setFoo)
            .addFunction("getFooName", &Barn::getFooName)
        .endClass();

    runLua(R"(
        local fooFirst = Foo('first')

        local bar = Bar()
        bar:setFoo(fooFirst)
        result = bar:getFooName() .. " " .. tostring(fooFirst)
    )");
    EXPECT_EQ("first first", result<std::string>());

    runLua(R"(
        local fooSecond = Foo.createConstRef('second')

        local bar = Bar()
        bar:setFoo(fooSecond)
        result = bar:getFooName() .. " " .. tostring(fooSecond)
    )");
    EXPECT_EQ("second second", result<std::string>());
}

TEST_F(ClassTests, RefExtensible)
{
    struct C {};
    using Fool = Foo<C>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Fool>("Foo", luabridge::extensibleClass)
            .addConstructor<void(*)(const std::string&)>()
            .addStaticFunction("createRef", &Fool::createRef)
            .addProperty("getName", &Fool::getName)
        .endClass();

    runLua(R"(
        local foo = Foo.createRef('xyz')
        result = foo.getName
    )");
    EXPECT_EQ("xyz", result<std::string>());

    runLua(R"(
        local foo = Foo.createRef('xyz')
        result = foo.nonExistent
    )");
    EXPECT_TRUE(result().isNil());
}

TEST_F(ClassTests, ConstRefExtensible)
{
    struct D {};
    using Fool = Foo<D>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Fool>("Foo", luabridge::extensibleClass)
            .addConstructor<void(*)(const std::string&)>()
            .addStaticFunction("createConstRef", &Fool::createConstRef)
            .addProperty("getName", &Fool::getName)
        .endClass();

    runLua(R"(
        local foo = Foo.createConstRef('xyz')
        result = foo.getName
    )");
    EXPECT_EQ("xyz", result<std::string>());

    runLua(R"(
        local foo = Foo.createConstRef('xyz')
        result = foo.nonExistent
    )");
    EXPECT_TRUE(result().isNil());
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
        .addConstructor<void (*)(int)>() // Show that it doesn't matter
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
struct NodeX
{
    virtual ~NodeX() = default;
};

struct TriggerX : NodeX
{
    int trigger() const
    {
        return 2;
    }
};

struct ExplosionX : NodeX
{
    int explode() const
    {
        return 3;
    }
};

template <class From, class To>
auto classCast(luabridge::LuaRef from, lua_State* L)
    -> std::enable_if_t<std::is_base_of_v<From, To>, To*>
{
    auto fromInstance = from.cast<From*>();
    if (! fromInstance)
        luabridge::raise_lua_error(L, "Impossible to perform the desired cast: %s", fromInstance.message().c_str());

    auto toInstance = dynamic_cast<To*>(fromInstance.value());
    if (toInstance == nullptr)
        luabridge::raise_lua_error(L, "Impossible to perform the desired cast: instance is not of the desired type");

    return toInstance;
}
} // namespace

TEST_F(ClassTests, DerivedClassCast)
{
    TriggerX triggerInstance;
    ExplosionX explosionInstance;

    luabridge::getGlobalNamespace(L)
        .beginClass<NodeX>("NodeX")
        .endClass()
        .deriveClass<TriggerX, NodeX>("TriggerX")
            .addFunction("trigger", &TriggerX::trigger)
        .endClass()
        .deriveClass<ExplosionX, NodeX>("ExplosionX")
            .addFunction("explode", &ExplosionX::explode)
        .endClass()
        .addFunction("getNode", [&](int value) -> NodeX*
        {
            if (value == 0)
                return std::addressof(triggerInstance);
            else
                return std::addressof(explosionInstance);
        });

    luabridge::LuaRef castTable = luabridge::newTable(L);
    castTable[luabridge::getGlobal(L, "TriggerX")] = luabridge::newFunction(L, &classCast<NodeX, TriggerX>);
    castTable[luabridge::getGlobal(L, "ExplosionX")] = luabridge::newFunction(L, &classCast<NodeX, ExplosionX>);
    luabridge::setGlobal(L, castTable, "cast");

    ASSERT_TRUE(runLua("node = getNode (0); result = cast[TriggerX](node):trigger()"));
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(result().cast<int>(), 2);

    ASSERT_TRUE(runLua("node = getNode (1); result = cast[ExplosionX](node):explode()"));
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(result().cast<int>(), 3);

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("node = getNode (1); result = cast[TriggerX](node):trigger()"), std::exception);
    ASSERT_THROW(runLua("node = 1; result = cast[TriggerX](node):trigger()"), std::exception);
#else
    ASSERT_FALSE(runLua("node = getNode (1); result = cast[TriggerX](node):trigger()"));
    ASSERT_FALSE(runLua("node = 1; result = cast[TriggerX](node):trigger()"));
#endif
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

#if LUABRIDGE_HAS_EXCEPTIONS
TEST_F(ClassTests, DeriveFromUnregisteredClassThrows)
{
    using Base = Class<int, EmptyBase>;
    using Derived = Class<float, Base>;

    EXPECT_EQ(0, lua_gettop(L));

    EXPECT_THROW((luabridge::getGlobalNamespace(L).deriveClass<Derived, Base>("Derived")), std::exception);

    EXPECT_EQ(0, lua_gettop(L));
}
#endif

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
struct ClassWithTemplateMembers
{
    template <class T, class... Args>
    int templateMethod(Args&&...)
    {
        return static_cast<int>(sizeof...(Args));
    }

    template <class T, class... Args>
    int templateMethodNoexcept(Args&&...) noexcept
    {
        return static_cast<int>(sizeof...(Args));
    }
};

template<class T, class Base>
T proxyFunction(Class<T, Base>* object, T value)
{
    object->data = value;
    return value;
}

template<class T, class Base>
T proxyFunctionNoexcept(Class<T, Base>* object, T value) noexcept
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
T proxyFunctionStateNoexcept(Class<T, Base>* object, T value, lua_State*) noexcept
{
    object->data = value;
    return value;
}

template<class T, class Base>
T proxyConstFunction(const Class<T, Base>* object, T value)
{
    return value;
}

template<class T, class Base>
T proxyConstFunctionNoexcept(const Class<T, Base>* object, T value) noexcept
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

    [[maybe_unused]] auto result = luabridge::push(L, arg.unsafe_cast<int>() + 1000);

    return 1;
}
} // namespace

TEST_F(ClassFunctions, ClassWithTemplateMembers)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<ClassWithTemplateMembers>("ClassWithTemplateMembers")
            .addConstructor<void()>()
            .addFunction("method", &ClassWithTemplateMembers::templateMethod<int>)
            .addFunction("methodNoexcept", &ClassWithTemplateMembers::templateMethodNoexcept<int>)
        .endClass();

    runLua("local a = ClassWithTemplateMembers(); result = a:method()");
    ASSERT_EQ(0, result<int>());

    runLua("local a = ClassWithTemplateMembers(); result = a:methodNoexcept()");
    ASSERT_EQ(0, result<int>());
}

TEST_F(ClassFunctions, ProxyFunctions)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addFunction("method", &proxyFunction<int, EmptyBase>)
        .addFunction("methodNoexcept", &proxyFunctionNoexcept<int, EmptyBase>)
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

    runLua("result = returnRef ():methodNoexcept (1)");
    ASSERT_EQ(1, result<int>());

    runLua("result = returnConstRef ().methodNoexcept"); // Don't call, just get
    ASSERT_TRUE(result().isNil());

    runLua("result = returnPtr ():methodNoexcept (2)");
    ASSERT_EQ(2, result<int>());

    runLua("result = returnConstPtr ().methodNoexcept"); // Don't call, just get
    ASSERT_TRUE(result().isNil());

    runLua("result = returnValue ():methodNoexcept (3)");
    ASSERT_EQ(3, result<int>());
}

TEST_F(ClassFunctions, ProxyFunctions_PassState)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addFunction("method", &proxyFunctionState<int, EmptyBase>)
        .addFunction("methodNoexcept", &proxyFunctionStateNoexcept<int, EmptyBase>)
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

    runLua("result = returnRef ():methodNoexcept (1)");
    ASSERT_EQ(1, result<int>());

    runLua("result = returnConstRef ().methodNoexcept"); // Don't call, just get
    ASSERT_TRUE(result().isNil());

    runLua("result = returnPtr ():methodNoexcept (2)");
    ASSERT_EQ(2, result<int>());

    runLua("result = returnConstPtr ().methodNoexcept"); // Don't call, just get
    ASSERT_TRUE(result().isNil());

    runLua("result = returnValue ():methodNoexcept (3)");
    ASSERT_EQ(3, result<int>());
}

TEST_F(ClassFunctions, ConstProxyFunctions)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addFunction("constMethod", &proxyConstFunction<int, EmptyBase>)
        .addFunction("constMethodNoexcept", &proxyConstFunctionNoexcept<int, EmptyBase>)
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

    runLua("result = returnRef ():constMethodNoexcept (1)");
    ASSERT_EQ(1, result<int>());

    runLua("result = returnConstRef ():constMethodNoexcept (2)");
    ASSERT_EQ(2, result<int>());

    runLua("result = returnPtr ():constMethodNoexcept (3)");
    ASSERT_EQ(3, result<int>());

    runLua("result = returnConstPtr ():constMethodNoexcept (4)");
    ASSERT_EQ(4, result<int>());

    runLua("result = returnValue ():constMethodNoexcept (5)");
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
    closeLuaState(); // Force garbage collection

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
    closeLuaState(); // Force garbage collection

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
    closeLuaState(); // Force garbage collection

    ASSERT_TRUE(data.expired());
}

struct ClassProperties : ClassTests
{
};

TEST_F(ClassProperties, FieldPointersNonRegistered)
{
    using Int = Class<int, EmptyBase>;
    using UnregisteredInt = Class<long, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addProperty("data", [](const Int&) { return UnregisteredInt(1); })
        .endClass();

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("result = Int().data"), std::exception);
#else
    ASSERT_FALSE(runLua("result = Int().data"));
#endif
}

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
        .addProperty("dataNoexcept", &Int::getDataNoexcept, &Int::setDataNoexcept)
        .addFunction("getDataNoexcept", &Int::getDataNoexcept)
        .addFunction("setDataNoexcept", &Int::setDataNoexcept)
        .endClass();

    runLua("result = Int (501)");
    ASSERT_TRUE(result()["data"].isNumber());
    ASSERT_EQ(501, result()["data"].cast<int>());
    ASSERT_TRUE(result()["dataNoexcept"].isNumber());
    ASSERT_EQ(501, result()["dataNoexcept"].cast<int>());

    runLua("result.data = -2");
    ASSERT_TRUE(result()["data"].isNumber());
    ASSERT_EQ(-2, result()["data"].cast<int>());

    runLua("result.dataNoexcept = -2");
    ASSERT_TRUE(result()["dataNoexcept"].isNumber());
    ASSERT_EQ(-2, result()["dataNoexcept"].cast<int>());
}

TEST_F(ClassProperties, MemberFunctions_PassState)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addProperty("data", &Int::getDataState, &Int::setDataState)
        .addProperty("dataNoexcept", &Int::getDataStateNoexcept, &Int::setDataStateNoexcept)
        .endClass();

    runLua("result = Int (501)");
    ASSERT_TRUE(result()["data"].isNumber());
    ASSERT_EQ(501, result()["data"].cast<int>());
    ASSERT_TRUE(result()["dataNoexcept"].isNumber());
    ASSERT_EQ(501, result()["dataNoexcept"].cast<int>());

    runLua("result.data = -2");
    ASSERT_TRUE(result()["data"].isNumber());
    ASSERT_EQ(-2, result()["data"].cast<int>());

    runLua("result.dataNoexcept = -2");
    ASSERT_TRUE(result()["dataNoexcept"].isNumber());
    ASSERT_EQ(-2, result()["dataNoexcept"].cast<int>());
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

template<class T, class BaseClass>
T getDataNoexcept(const Class<T, BaseClass>* object) noexcept
{
    return object->data;
}

template<class T, class BaseClass>
void setDataNoexcept(Class<T, BaseClass>* object, T data) noexcept
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

TEST_F(ClassProperties, ProxyFunctionsNoexcept)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addProperty("data", &getDataNoexcept<int, EmptyBase>, &setDataNoexcept<int, EmptyBase>)
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
    auto* object = objectRef.unsafe_cast<const Class<T, BaseClass>*>();

    [[maybe_unused]] auto result = luabridge::Stack<T>::push(L, object->data);

    return 1;
}

template<class T, class BaseClass>
int setDataC(lua_State* L)
{
    auto objectRef = luabridge::LuaRef::fromStack(L, 1);
    auto* object = objectRef.unsafe_cast<const Class<T, BaseClass>*>();
    auto valueRef = luabridge::LuaRef::fromStack(L, 2);
    T value = valueRef.unsafe_cast<T>();
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

    int data2 = 1;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addConstructor<void (*)(int)>()
        .addProperty("data", std::move(getter), std::move(setter))
        .addProperty("data2", [&data2](const Int*) { return data2; }, [data2 = std::addressof(data2)](Int*, int v) { *data2 = v; })
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

    runLua("result.data2 = -2");
    ASSERT_TRUE(result()["data2"].isNumber());
    ASSERT_EQ(-2, result()["data2"].cast<int>());

    runLua("result = nil");
    closeLuaState(); // Force garbage collection

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
    closeLuaState(); // Force garbage collection

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

TEST_F(ClassStaticProperties, FieldPointersNonRegistered)
{
    using Int = Class<int, EmptyBase>;
    using UnregisteredInt = Class<long, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addStaticProperty("staticData", [] { return UnregisteredInt(1); })
        .endClass();

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("result = Int.staticData"), std::exception);
#else
    ASSERT_FALSE(runLua("result = Int.staticData"));
#endif
}

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

TEST_F(ClassStaticProperties, FieldPointers_Const)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addStaticProperty("staticConstData", &Int::staticConstData)
        .endClass();

    runLua("result = Int.staticConstData");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(0, result<int>());

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("Int.staticConstData = 20"), std::exception);
#else
    ASSERT_FALSE(runLua("Int.staticConstData = 20"));
#endif

    ASSERT_EQ(0, Int::staticConstData);
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

TEST_F(ClassStaticProperties, FieldPointers_GetterOnly)
{
    using Int = Class<int, EmptyBase>;

    int value = 10;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addStaticProperty("staticData", [&value] { return value; })
        .endClass();

    runLua("result = Int.staticData");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(10, result<int>());
    ASSERT_EQ(10, value);

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("Int.staticData = 20"), std::exception);
#else
    ASSERT_FALSE(runLua("Int.staticData = 20"));
#endif

    runLua("result = Int.staticData");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(10, result<int>());
    ASSERT_EQ(10, value);
}

TEST_F(ClassStaticProperties, FieldPointers_GetterSetter)
{
    using Int = Class<int, EmptyBase>;

    int value = 10;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addStaticProperty("staticData", [&value] { return value; }, [&value](int x) { value = x; })
        .endClass();

    runLua("result = Int.staticData");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(10, result<int>());
    ASSERT_EQ(10, value);

    runLua("Int.staticData = 20; result = Int.staticData");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(20, result<int>());
    ASSERT_EQ(20, value);
}

TEST_F(ClassStaticProperties, FieldPointers_StaticMembers)
{
    using Int = Class<int, EmptyBase>;

    Int::staticData = 10;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addStaticProperty("staticData", &Int::getStaticData, &Int::setStaticData)
        .endClass();

    runLua("result = Int.staticData");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(10, result<int>());

    runLua("Int.staticData = 20; result = Int.staticData");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(20, result<int>());
}

TEST_F(ClassStaticProperties, FieldPointers_StaticMembersReadOnly)
{
    using Int = Class<int, EmptyBase>;

    Int::staticData = 10;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addStaticProperty("staticData", &Int::getStaticData)
        .endClass();

    runLua("result = Int.staticData");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(10, result<int>());

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("Int.staticData = 20"), std::exception);
#else
    ASSERT_FALSE(runLua("Int.staticData = 20"));
#endif
}

TEST_F(ClassStaticProperties, FieldPointers_StaticMembersNoexcept)
{
    using Int = Class<int, EmptyBase>;

    Int::staticData = 10;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addStaticProperty("staticData", &Int::getStaticDataNoexcept, &Int::setStaticDataNoexcept)
        .endClass();

    runLua("result = Int.staticData");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(10, result<int>());

    runLua("Int.staticData = 20; result = Int.staticData");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(20, result<int>());
}

TEST_F(ClassStaticProperties, FieldPointers_StaticMembersReadonlyNoexcept)
{
    using Int = Class<int, EmptyBase>;

    Int::staticData = 10;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .addStaticProperty("staticData", &Int::getStaticDataNoexcept)
        .endClass();

    runLua("result = Int.staticData");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(10, result<int>());

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("Int.staticData = 20"), std::exception);
#else
    ASSERT_FALSE(runLua("Int.staticData = 20"));
#endif
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

TEST_F(ClassStaticProperties, SubsequentRegistration)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
        .endClass()
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
    luabridge::getGlobalNamespace(L)
        .beginClass<Table>("Table")
        .addFunction("__newindex", &Table::newIndex)
        .endClass();

    Table t;

    luabridge::setGlobal(L, &t, "t");

    runLua("t.a = 1; t['b'] = 2");

    ASSERT_EQ((std::map<std::string, int>{{"a", 1}, {"b", 2}}), t.map);
}

TEST_F(ClassTests, __tostring)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Table>("Table")
        .endClass();

    Table t;
    luabridge::setGlobal(L, &t, "t");

    runLua("result = tostring(t)");
    ASSERT_EQ(0, result<std::string>().find("Table"));
}

TEST_F(ClassTests, __tostringOverride)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Table>("Table")
            .addFunction("__tostring", [] (const Table* t) { return "StrangeTable"; })
        .endClass();

    Table t;
    luabridge::setGlobal(L, &t, "t");

    runLua("result = tostring(t)");
    ASSERT_EQ(0, result<std::string>().find("StrangeTable"));
}

#if LUABRIDGE_HAS_EXCEPTIONS
TEST_F(ClassMetaMethods, __gcForbidden)
{
    using Int = Class<int, EmptyBase>;

    ASSERT_THROW(luabridge::getGlobalNamespace(L)
                     .beginClass<Int>("Int")
                     .addFunction("__gc", &Int::method)
                     .endClass(),
                 std::exception);
}
#endif

TEST_F(ClassMetaMethods, MetamethodsShouldNotBePartOfClassInstances)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
            .addConstructor<void (*)(int)>()
            .addFunction("__xyz", [](const Int*) {})
        .endClass();

#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_ANY_THROW(runLua("local x = Int(1); x.__gc()"));
    EXPECT_ANY_THROW(runLua("local x = Int(1); x:__gc()"));
    EXPECT_ANY_THROW(runLua("local x = Int(1); x.__index()"));
    EXPECT_ANY_THROW(runLua("local x = Int(1); x:__index()"));
    EXPECT_ANY_THROW(runLua("local x = Int(1); x.__newindex()"));
    EXPECT_ANY_THROW(runLua("local x = Int(1); x:__newindex()"));
    EXPECT_TRUE(runLua("local x = Int(1); result = x.__gc"));
    EXPECT_TRUE(result().isNil());
    EXPECT_TRUE(runLua("local x = Int(1); result = x.__index"));
    EXPECT_TRUE(result().isNil());
    EXPECT_TRUE(runLua("local x = Int(1); result = x.__newindex"));
    EXPECT_TRUE(result().isNil());
#else
    EXPECT_FALSE(runLua("local x = Int(1); x.__gc()"));
    EXPECT_FALSE(runLua("local x = Int(1); x:__gc()"));
    EXPECT_FALSE(runLua("local x = Int(1); x.__index()"));
    EXPECT_FALSE(runLua("local x = Int(1); x:__index()"));
    EXPECT_FALSE(runLua("local x = Int(1); x.__newindex()"));
    EXPECT_FALSE(runLua("local x = Int(1); x:__newindex()"));
    EXPECT_TRUE(runLua("local x = Int(1); result = x.__gc"));
    EXPECT_TRUE(result().isNil());
    EXPECT_TRUE(runLua("local x = Int(1); result = x.__index"));
    EXPECT_TRUE(result().isNil());
    EXPECT_TRUE(runLua("local x = Int(1); result = x.__newindex"));
    EXPECT_TRUE(result().isNil());
#endif

    EXPECT_TRUE(runLua("local x = Int(1); x:__xyz()"));
    EXPECT_TRUE(runLua("local x = Int(1); result = x.__xyz"));
    EXPECT_TRUE(result().isFunction());
}

TEST_F(ClassMetaMethods, MetamethodsShouldNotBeWritable)
{
    using Int = Class<int, EmptyBase>;

    luabridge::getGlobalNamespace(L)
        .beginClass<Int>("Int")
            .addConstructor<void (*)(int)>()
        .endClass();

#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_ANY_THROW(runLua("local x = Int(1); x.__gc = function() end"));
    EXPECT_ANY_THROW(runLua("local x = Int(1); x.__index = function() end"));
    EXPECT_ANY_THROW(runLua("local x = Int(1); x.__newindex = function() end"));
#else
    EXPECT_FALSE(runLua("local x = Int(1); x.__gc = function() end"));
    EXPECT_FALSE(runLua("local x = Int(1); x.__index = function() end"));
    EXPECT_FALSE(runLua("local x = Int(1); x.__newindex = function() end"));
#endif
}

namespace {
struct StringGetter
{
   std::string str;

   const char* getString() const { return str.c_str(); }
   void setString(const char* val) { str = val; }
};
} // namespace

TEST_F(ClassMetaMethods, ErrorLineWithProperties)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<StringGetter>("StringGetter")
            .addConstructor<void(*)()>()
            .addFunction("setString", &StringGetter::setString)
            .addProperty("str", &StringGetter::getString, &StringGetter::setString)
        .endClass();

#if LUABRIDGE_HAS_EXCEPTIONS
    try
    {
        runLua(R"(
            local myStringGetter = StringGetter()
            myStringGetter.str = 12
        )");

        EXPECT_TRUE(false);
    }
    catch (const std::exception& ex)
    {
        EXPECT_EQ(0, std::string_view(ex.what()).find("[string \"...\"]:3"));

        // This is not comparing with std::string_view::npos because we have debug.traceback in the error handler
        EXPECT_NE(18, std::string_view(ex.what()).find("[string \"...\"]:3: ", 18));
    }

    try
    {
        runLua(R"(
            local myStringGetter = StringGetter()
            myStringGetter:setString(12)
        )");

        EXPECT_TRUE(false);
    }
    catch (const std::exception& ex)
    {
        EXPECT_EQ(0, std::string_view(ex.what()).find("[string \"...\"]:3: "));

        // This is not comparing with std::string_view::npos because we have debug.traceback in the error handler
        EXPECT_NE(18, std::string_view(ex.what()).find("[string \"...\"]:3: ", 18));
    }
#endif

    {
        auto [result, errorString] = runLuaCaptureError(R"(
            local myStringGetter = StringGetter()
            myStringGetter.str = 12
        )");

        EXPECT_EQ(0, std::string_view(errorString).find("[string \"...\"]:3: "));
        EXPECT_EQ(std::string_view::npos, std::string_view(errorString).find("[string \"...\"]:3: ", 18));
    }

    {
        auto [result, errorString] = runLuaCaptureError(R"(
            local myStringGetter = StringGetter()
            myStringGetter:setString(12)
        )");

        EXPECT_EQ(0, std::string_view(errorString).find("[string \"...\"]:3: "));
        EXPECT_EQ(std::string_view::npos, std::string_view(errorString).find("[string \"...\"]:3: ", 18));
    }
}

TEST_F(ClassMetaMethods, SimulateArray)
{
    using ContainerType = std::vector<std::string>;

    ContainerType data(1);
    data[0] = "abcdefg";

    luabridge::getGlobalNamespace(L)
        .beginTable("xyz")
            .addFunction("a", +[] { return "abcdefg"; })
            .addMetaFunction("__index", [&data](luabridge::LuaRef, int index, lua_State* L)
            {
                if (index < 0 || index >= static_cast<int>(data.size()))
                    luaL_error(L, "Invalid index access in table %d", index);

                return data[index];
            })
            .addMetaFunction("__newindex", [&data](luabridge::LuaRef, int index, luabridge::LuaRef ref, lua_State* L)
            {
                if (index < 0)
                    luaL_error(L, "Invalid index access in table %d", index);

                if (! ref.isString())
                    luaL_error(L, "Invalid value provided to set table at index %d", index);

                if (index >= static_cast<int>(data.size()))
                    data.resize(index + 1);

                data[index] = ref.unsafe_cast<std::string>();
            })
        .endTable();

    runLua("xyz[0] = '123'; result = xyz[0]");
    ASSERT_EQ("123", result<std::string>());
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

namespace {
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
} // namespace

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
        [[maybe_unused]] InnerClass y;
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

    closeLuaState(); // Force garbage collection

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

    closeLuaState(); // Force garbage collection

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

TEST_F(ClassTests, ConstructorFactory)
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
            .addConstructor([](void* ptr, lua_State* L) { return new(ptr) FactoryConstructibleClass(static_cast<int>(luaL_checkinteger(L, 2))); })
            .addProperty("x", &FactoryConstructibleClass::x_)
            .endClass();

        runLua("obj = FactoryConstructibleClass3 (42); result = obj.x");

        ASSERT_TRUE(result().isNumber());
        ASSERT_EQ(42, result<int>());
    }
}

namespace {
class BaseExampleClass
{
public:
    BaseExampleClass() = default;
    virtual ~BaseExampleClass() = default;

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
} // namespace

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

namespace {
template <std::size_t Alignment>
struct alignas(Alignment) Vec
{
public:
    Vec(double InA, double InB, double InC, double InD)
    {
        X = InA;
        Y = InB;
        Z = InC;
        W = InD;
    }

public:

    double X;
    double Y;
    double Z;
    double W;

    bool isAligned() const
    {
        return luabridge::is_aligned<Alignment>(reinterpret_cast<const double*>(this));
    }
};
} // namespace

TEST_F(ClassTests, OveralignedClasses)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Vec<8>>("Vec8")
            .addConstructor<void(*) (double, double, double, double)>()
            .addFunction("checkIsAligned", &Vec<8>::isAligned)
        .endClass()
        .beginClass<Vec<16>>("Vec16")
            .addConstructor<void(*) (double, double, double, double)>()
            .addFunction("checkIsAligned", &Vec<16>::isAligned)
        .endClass()
        .beginClass<Vec<32>>("Vec32")
            .addConstructor<void(*) (double, double, double, double)>()
            .addFunction("checkIsAligned", &Vec<32>::isAligned)
        .endClass();

    runLua("result = Vec8(3.0, 2.0, 1.0, 0.5):checkIsAligned()");
    EXPECT_TRUE(result<bool>());

    runLua("result = Vec16(3.0, 2.0, 1.0, 0.5):checkIsAligned()");
    EXPECT_TRUE(result<bool>());

    runLua("result = Vec32(3.0, 2.0, 1.0, 0.5):checkIsAligned()");
    EXPECT_TRUE(result<bool>());
}

namespace {
struct XYZ { int x = 0; };
struct ABC { float y = 0.0f; };
} // namespace

TEST_F(ClassTests, WrongThrowBadArgObjectDescription)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<XYZ>("XYZ")
        .endClass()
        .beginClass<ABC>("ABC")
            .addConstructor<void(*)()>()
        .endClass()
        .addFunction("textXYZ", [](int, float, const XYZ&) {})
        .addFunction("textSingleXYZ", [](const XYZ&) {});

#if LUABRIDGE_HAS_EXCEPTIONS
#if 0
    // These seems to fail coverage collection of .gcda
    try
    {
        runLua("textSingleXYZ()");
        EXPECT_TRUE(false);
    }
    catch (const std::exception& ex)
    {
        EXPECT_NE(std::string::npos, std::string(ex.what()).find("got no value"));
    }

    try
    {
        runLua("textXYZ(1, 1.0)");
        EXPECT_TRUE(false);
    }
    catch (const std::exception& ex)
    {
        EXPECT_NE(std::string::npos, std::string(ex.what()).find("got no value"));
    }

    try
    {
        runLua("textXYZ(1, 1.0, 1)");
        EXPECT_TRUE(false);
    }
    catch (const std::exception& ex)
    {
        EXPECT_NE(std::string::npos, std::string(ex.what()).find("got number"));
    }

    try
    {
        runLua("textXYZ(1, 1.0, '1')");
        EXPECT_TRUE(false);
    }
    catch (const std::exception& ex)
    {
        EXPECT_NE(std::string::npos, std::string(ex.what()).find("got string"));
    }

    try
    {
        runLua("textXYZ(1, 1.0, ABC())");
        EXPECT_TRUE(false);
    }
    catch (const std::exception& ex)
    {
        EXPECT_NE(std::string::npos, std::string(ex.what()).find("got ABC"));
    }
#endif

#else
    {
        auto [result, errorMessage] = runLuaCaptureError("textSingleXYZ()");
        ASSERT_FALSE(result);
        EXPECT_NE(std::string::npos, errorMessage.find("got no value"));
    }

    {
        auto [result, errorMessage] = runLuaCaptureError("textXYZ(1, 1.0)");
        ASSERT_FALSE(result);
        EXPECT_NE(std::string::npos, errorMessage.find("got no value"));
    }

    {
        auto [result, errorMessage] = runLuaCaptureError("textXYZ(1, 1.0, 1)");
        ASSERT_FALSE(result);
        EXPECT_NE(std::string::npos, errorMessage.find("got number"));
    }

    {
        auto [result, errorMessage] = runLuaCaptureError("textXYZ(1, 1.0, '1')");
        ASSERT_FALSE(result);
        EXPECT_NE(std::string::npos, errorMessage.find("got string"));
    }

    {
        auto [result, errorMessage] = runLuaCaptureError("textXYZ(1, 1.0, ABC())");
        ASSERT_FALSE(result);
        EXPECT_NE(std::string::npos, errorMessage.find("got ABC"));
    }
#endif
}

namespace {
struct Node
{
    int data = 0;
    Node* next = nullptr;
    Node* prev = nullptr;

    explicit Node(int value)
        : data(value)
    {
    }

    int val() const { return data; }
    Node* next_node() const { return next; }
};

class DoublyLinkedList
{
public:
    DoublyLinkedList(int numToAdd = 0)
    {
        for (int value = 1; value <= numToAdd; value++)
            addBack(value);
    }

    ~DoublyLinkedList()
    {
        while (head)
            removeBack();
    }

    void addBack(int value)
    {
        Node* newNode = new Node(value);
        if (tail == nullptr)
        {
            head = newNode;
            tail = newNode;
        }
        else
        {
            newNode->prev = tail;
            tail->next = newNode;
            tail = newNode;
        }
    }

    void removeBack()
    {
        if (tail == nullptr)
            return;

        Node* temp = tail;
        tail = tail->prev;

        if (tail != nullptr)
            tail->next = nullptr;
        else
            head = nullptr;

        delete temp;
    }

    Node* first() const { return head; }

private:
    Node* head = nullptr;
    Node* tail = nullptr;
};

class foo
{
public:
    foo(int size = 5000) : list(size) {}

    Node* first() const { return list.first(); }
    Node* next(Node* curr) const { return curr->next_node(); }

private:
    DoublyLinkedList list;
};

class bar
{
public:
   bar(foo* f) : foo_(f) {}

   Node* first() const { return foo_->first(); }
   Node* next(Node* curr) const { return foo_->next(curr); }

private:
   foo* foo_;
};
} // namespace

TEST_F(ClassTests, BugWithPlacementConstructor)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("foobar")
            .beginClass<Node>("Node")
                .addFunction("val", &Node::val)
            .endClass()
            .beginClass<foo>("foo")
                .addConstructor([](void* p, int size) { return new(p) foo(size); })
                .addFunction("first", &foo::first)
                .addFunction("next", &foo::next)
            .endClass()
            .beginClass<bar>("bar")
                .addConstructor<void(*)(foo *)>()
                .addFunction("first", &bar::first)
                .addFunction("next", &bar::next)
            .endClass()
        .endNamespace();

    runLua(R"(
        local foo = foobar.foo(5000)
        local bar = foobar.bar(foo)
        local next = bar:first()
        while next do
            next = bar:next(next)
        end
    )");

    SUCCEED();
}

TEST_F(ClassTests, BugWithFactoryConstructor)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("foobar")
            .beginClass<Node>("Node")
                .addFunction("val", &Node::val)
            .endClass()
            .beginClass<foo>("foo")
                .addFactory(
                    +[]() -> foo* { return new foo(5000); },
                    +[](foo* x) { delete x; })
                .addFunction("first", &foo::first)
                .addFunction("next", &foo::next)
            .endClass()
            .beginClass<bar>("bar")
                .addConstructor<void(*)(foo *)>()
                .addFunction("first", &bar::first)
                .addFunction("next", &bar::next)
            .endClass()
        .endNamespace();

    runLua(R"(
        local foo = foobar.foo()
        local bar = foobar.bar(foo)
        local next = bar:first()
        while next do
            next = bar:next(next)
        end
    )");

    SUCCEED();
}

namespace {

struct ClassWithMethod
{
    ClassWithMethod(int)
    {
    }

    ClassWithMethod(lua_State* L)
    {
        luaL_argerror(L, 1, "test message");
    }

    int methodWithError(lua_State* L)
    {
        luaL_argerror(L, 2, "test message");
        return 0;
    }
};

} // namespace

TEST_F(ClassTests, BugWithLuauNotPrintingClassConstructorNameInErrors)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("foo")
            .beginClass<ClassWithMethod>("ClassWithMethod")
                .addConstructor<void (*)(lua_State*)>()
                .addFunction("methodWithError", &ClassWithMethod::methodWithError)
            .endClass()
        .endNamespace();

    auto [result, error] = runLuaCaptureError(R"(
        local bar = foo.ClassWithMethod()
        result = bar
    )");

    EXPECT_FALSE(result);
    EXPECT_TRUE(error.find("ClassWithMethod") != std::string::npos);
}

TEST_F(ClassTests, BugWithLuauNotPrintingClassMethodNameInErrors)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("foo")
            .beginClass<ClassWithMethod>("ClassWithMethod")
                .addConstructor<void (*)(lua_State*)>()
                .addFunction("methodWithError", &ClassWithMethod::methodWithError)
            .endClass()
        .endNamespace();

    ClassWithMethod x(1);
    luabridge::setGlobal(L, x, "xyz");

    auto [result, error] = runLuaCaptureError(R"(
        result = xyz:methodWithError()
    )");

    EXPECT_FALSE(result);
    EXPECT_TRUE(error.find("methodWithError") != std::string::npos);
}

namespace {
class FooClass {};

class BarClass
{
public:
    BarClass() = default;

    std::shared_ptr<FooClass> getter() const
    {
        return std::make_shared<FooClass>();
    }
};

static std::shared_ptr<FooClass> fooClassGetter()
{
    return std::make_shared<FooClass>();
}
} // namespace

TEST_F(ClassTests, BugWithSharedPtrPropertyGetterFromClassNotDerivingFromSharedFromThis)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<FooClass>("FooClass")
        .endClass()
        .beginClass<BarClass>("BarClass")
            .addConstructor<void (*)()>()
            .addFunction("getAsFunction", &BarClass::getter)
            .addProperty("getAsProperty", &BarClass::getter)
        .endClass()
        .beginNamespace("bar")
            .addFunction("getAsFunction", &fooClassGetter)
            .addProperty("getAsProperty", &fooClassGetter)
        .endNamespace();

    EXPECT_TRUE(runLua("result = BarClass():getAsFunction()"));
    EXPECT_TRUE(runLua("result = BarClass().getAsProperty"));
    EXPECT_TRUE(runLua("result = bar.getAsFunction()"));
    EXPECT_TRUE(runLua("result = bar.getAsProperty"));
}
