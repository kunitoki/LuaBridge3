// https://github.com/kunitoki/LuaBridge3
// Copyright 2026, kunitoki
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include "LuaBridge/LuaBridge.h"

#if LUABRIDGE_ENABLE_REFLECT
#include "LuaBridge/Inspect.h"

#include <algorithm>
#include <sstream>

struct InspectTests : TestBase
{
};

//=================================================================================================
// Helpers

namespace {

const luabridge::MemberInfo* findMember(const luabridge::ClassInspectInfo& cls, const std::string& name)
{
    for (const auto& m : cls.members)
        if (m.name == name) return &m;
    return nullptr;
}

const luabridge::MemberInfo* findFreeMember(const luabridge::NamespaceInspectInfo& ns, const std::string& name)
{
    for (const auto& m : ns.freeMembers)
        if (m.name == name) return &m;
    return nullptr;
}

const luabridge::ClassInspectInfo* findClass(const luabridge::NamespaceInspectInfo& ns, const std::string& name)
{
    for (const auto& c : ns.classes)
        if (c.name == name) return &c;
    return nullptr;
}

const luabridge::NamespaceInspectInfo* findSubNamespace(const luabridge::NamespaceInspectInfo& ns, const std::string& name)
{
    for (const auto& s : ns.subNamespaces)
        if (s.name == name) return &s;
    return nullptr;
}

bool hasBase(const luabridge::ClassInspectInfo& cls, const std::string& name)
{
    return std::find(cls.baseClasses.begin(), cls.baseClasses.end(), name) != cls.baseClasses.end();
}

} // anonymous namespace

//=================================================================================================
// inspect<T>() — single class tests

TEST_F(InspectTests, InspectUnregisteredClass)
{
    struct Unregistered {};
    auto info = luabridge::inspect<Unregistered>(L);
    EXPECT_TRUE(info.name.empty());
    EXPECT_TRUE(info.members.empty());
    EXPECT_TRUE(info.baseClasses.empty());
}

TEST_F(InspectTests, InspectSimpleClass)
{
    struct Foo
    {
        int x = 0;
        int getX() const { return x; }
        void setX(int v) { x = v; }
        void doSomething() {}
        static int staticFunc() { return 42; }
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<Foo>("Foo")
            .addConstructor<void()>()
            .addProperty("x", &Foo::getX, &Foo::setX)
            .addFunction("doSomething", &Foo::doSomething)
            .addStaticFunction("staticFunc", &Foo::staticFunc)
        .endClass();

    auto info = luabridge::inspect<Foo>(L);
    EXPECT_EQ("Foo", info.name);

    auto* prop = findMember(info, "x");
    ASSERT_NE(nullptr, prop);
    EXPECT_EQ(luabridge::MemberKind::Property, prop->kind);

    auto* method = findMember(info, "doSomething");
    ASSERT_NE(nullptr, method);
    EXPECT_EQ(luabridge::MemberKind::Method, method->kind);
    EXPECT_EQ(1u, method->overloads.size());

    auto* staticFn = findMember(info, "staticFunc");
    ASSERT_NE(nullptr, staticFn);
    EXPECT_EQ(luabridge::MemberKind::StaticMethod, staticFn->kind);

    auto* ctor = findMember(info, "__call");
    ASSERT_NE(nullptr, ctor);
    EXPECT_EQ(luabridge::MemberKind::Constructor, ctor->kind);
}

TEST_F(InspectTests, InspectReadOnlyProperty)
{
    struct Bar { int getValue() const { return 1; } };
    luabridge::getGlobalNamespace(L)
        .beginClass<Bar>("Bar")
            .addProperty("value", &Bar::getValue)
        .endClass();

    auto info = luabridge::inspect<Bar>(L);
    auto* prop = findMember(info, "value");
    ASSERT_NE(nullptr, prop);
    EXPECT_EQ(luabridge::MemberKind::ReadOnlyProperty, prop->kind);
}

TEST_F(InspectTests, InspectOverloadedMethod)
{
    struct Baz {};
    luabridge::getGlobalNamespace(L)
        .beginClass<Baz>("Baz")
            .addFunction("method",
                [](Baz&, int) {},
                [](Baz&, float) {})
        .endClass();

    auto info = luabridge::inspect<Baz>(L);
    auto* method = findMember(info, "method");
    ASSERT_NE(nullptr, method);
    EXPECT_EQ(luabridge::MemberKind::Method, method->kind);
    EXPECT_EQ(2u, method->overloads.size());
}

TEST_F(InspectTests, InspectDerivedClass)
{
    struct Base { int baseMethod() const { return 1; } };
    struct Derived : Base { int derivedMethod() const { return 2; } };

    luabridge::getGlobalNamespace(L)
        .beginClass<Base>("Base")
            .addFunction("baseMethod", &Base::baseMethod)
        .endClass()
        .deriveClass<Derived, Base>("Derived")
            .addFunction("derivedMethod", &Derived::derivedMethod)
        .endClass();

    auto info = luabridge::inspect<Derived>(L);
    EXPECT_EQ("Derived", info.name);
    EXPECT_FALSE(info.baseClasses.empty());
    EXPECT_TRUE(hasBase(info, "Base"));
}

TEST_F(InspectTests, InspectMetamethods)
{
    struct Vec
    {
        Vec operator+(const Vec& /*rhs*/) const { return {}; }
    };
    luabridge::getGlobalNamespace(L)
        .beginClass<Vec>("Vec")
            .addFunction("__add", &Vec::operator+)
        .endClass();

    auto info = luabridge::inspect<Vec>(L);
    auto* add = findMember(info, "__add");
    ASSERT_NE(nullptr, add);
    EXPECT_EQ(luabridge::MemberKind::Metamethod, add->kind);
}

TEST_F(InspectTests, InspectStaticReadOnlyProperty)
{
    struct WithStatic {};
    static int sVal = 42;
    luabridge::getGlobalNamespace(L)
        .beginClass<WithStatic>("WithStatic")
            .addStaticProperty("ROProp", +[]() { return sVal; })
            .addStaticProperty("RWProp", +[]() { return sVal; }, +[](int v) { sVal = v; })
        .endClass();

    auto info = luabridge::inspect<WithStatic>(L);

    auto* ro = findMember(info, "ROProp");
    ASSERT_NE(nullptr, ro);
    EXPECT_EQ(luabridge::MemberKind::StaticReadOnlyProperty, ro->kind);

    auto* rw = findMember(info, "RWProp");
    ASSERT_NE(nullptr, rw);
    EXPECT_EQ(luabridge::MemberKind::StaticProperty, rw->kind);
}

//=================================================================================================
// inspectNamespace() tests

TEST_F(InspectTests, InspectNamespace)
{
    struct Widget { void draw() {} };
    luabridge::getGlobalNamespace(L)
        .beginNamespace("UI")
            .beginClass<Widget>("Widget")
                .addFunction("draw", &Widget::draw)
            .endClass()
            .addFunction("ping", +[]() -> int { return 1; })
        .endNamespace();

    auto ns = luabridge::inspectNamespace(L, "UI");
    EXPECT_EQ("UI", ns.name);

    auto* w = findClass(ns, "Widget");
    ASSERT_NE(nullptr, w);
    EXPECT_EQ("Widget", w->name);

    auto* ping = findFreeMember(ns, "ping");
    ASSERT_NE(nullptr, ping);
    EXPECT_EQ(luabridge::MemberKind::Method, ping->kind);
}

TEST_F(InspectTests, InspectSubNamespace)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("Outer")
            .beginNamespace("Inner")
                .addFunction("innerFunc", +[]() {})
            .endNamespace()
        .endNamespace();

    auto ns = luabridge::inspectNamespace(L, "Outer");
    EXPECT_EQ("Outer", ns.name);

    auto* inner = findSubNamespace(ns, "Inner");
    ASSERT_NE(nullptr, inner);
    EXPECT_EQ("Inner", inner->name);

    auto* fn = findFreeMember(*inner, "innerFunc");
    ASSERT_NE(nullptr, fn);
}

TEST_F(InspectTests, InspectEmptyNamespace)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("Empty")
        .endNamespace();

    auto ns = luabridge::inspectNamespace(L, "Empty");
    EXPECT_EQ("Empty", ns.name);
    EXPECT_TRUE(ns.classes.empty());
    EXPECT_TRUE(ns.freeMembers.empty());
    EXPECT_TRUE(ns.subNamespaces.empty());
}

TEST_F(InspectTests, InspectNonexistentNamespace)
{
    auto ns = luabridge::inspectNamespace(L, "DoesNotExist");
    EXPECT_TRUE(ns.name.empty());
}

TEST_F(InspectTests, InspectDottedNamespacePath)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("A")
            .beginNamespace("B")
                .addFunction("deep", +[]() {})
            .endNamespace()
        .endNamespace();

    auto ns = luabridge::inspectNamespace(L, "A.B");
    EXPECT_EQ("B", ns.name);
    EXPECT_NE(nullptr, findFreeMember(ns, "deep"));
}

TEST_F(InspectTests, InspectNamespaceProperty)
{
    static int nsVal = 0;
    luabridge::getGlobalNamespace(L)
        .beginNamespace("NsProp")
            .addProperty("roProp", +[]() { return nsVal; })
            .addProperty("rwProp", +[]() { return nsVal; }, +[](int v) { nsVal = v; })
        .endNamespace();

    auto ns = luabridge::inspectNamespace(L, "NsProp");
    auto* ro = findFreeMember(ns, "roProp");
    ASSERT_NE(nullptr, ro);
    EXPECT_EQ(luabridge::MemberKind::ReadOnlyProperty, ro->kind);

    auto* rw = findFreeMember(ns, "rwProp");
    ASSERT_NE(nullptr, rw);
    EXPECT_EQ(luabridge::MemberKind::Property, rw->kind);
}

//=================================================================================================
// Visitor tests

TEST_F(InspectTests, ConsoleVisitorProducesOutput)
{
    struct Cls {};
    luabridge::getGlobalNamespace(L)
        .beginNamespace("ConsTest")
            .beginClass<Cls>("Cls")
                .addConstructor<void()>()
            .endClass()
        .endNamespace();

    std::stringstream ss;
    luabridge::inspectPrint(L, "ConsTest", ss);
    EXPECT_FALSE(ss.str().empty());
    EXPECT_NE(std::string::npos, ss.str().find("ConsTest"));
    EXPECT_NE(std::string::npos, ss.str().find("Cls"));
}

TEST_F(InspectTests, LuaTableVisitorProducesTable)
{
    struct Cls { void m() {} };
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TblTest")
            .beginClass<Cls>("Cls")
                .addFunction("m", &Cls::m)
            .endClass()
        .endNamespace();

    int topBefore = lua_gettop(L);
    luabridge::inspectToLua(L, "TblTest");

    ASSERT_EQ(topBefore + 1, lua_gettop(L));
    ASSERT_TRUE(lua_istable(L, -1));

    lua_getfield(L, -1, "name");
    EXPECT_STREQ("TblTest", lua_tostring(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, -1, "classes");
    EXPECT_TRUE(lua_istable(L, -1));
    EXPECT_EQ(1, luabridge::get_length(L, -1));
    lua_pop(L, 1);

    lua_pop(L, 1); // pop result table
    EXPECT_EQ(topBefore, lua_gettop(L));
}

TEST_F(InspectTests, LuaLSVisitorProducesAnnotations)
{
    struct Cls { int getValue() const { return 0; } };
    luabridge::getGlobalNamespace(L)
        .beginNamespace("LuaLS")
            .beginClass<Cls>("Cls")
                .addConstructor<void()>()
                .addProperty("value", &Cls::getValue)
                .addFunction("xyz", [](const Cls& self, int a) { return a + 1; })
                .addStaticFunction("abc", [](bool, float, int) { return "42"; })
            .endClass()
        .endNamespace();

    auto ns = luabridge::inspectNamespace(L, "LuaLS");
    std::stringstream ss;
    luabridge::LuaLSVisitor v(ss);
    luabridge::accept(ns, v);

    const auto result = ss.str();
    std::cout << result << std::endl;
    EXPECT_FALSE(result.empty());
    EXPECT_NE(std::string::npos, result.find("---@class LuaLS.Cls"));
    EXPECT_NE(std::string::npos, result.find("---@field value integer # readonly"));
    EXPECT_NE(std::string::npos, result.find("---@overload fun(): LuaLS.Cls"));
    EXPECT_NE(std::string::npos, result.find("local Cls = {}"));
    EXPECT_NE(std::string::npos, result.find("---@param self LuaLS.Cls"));
    EXPECT_NE(std::string::npos, result.find("---@param p1 integer"));
    EXPECT_NE(std::string::npos, result.find("---@return integer"));
    EXPECT_NE(std::string::npos, result.find("function LuaLS.Cls:xyz(p1) end"));
    EXPECT_NE(std::string::npos, result.find("---@param p1 boolean"));
    EXPECT_NE(std::string::npos, result.find("---@param p2 number"));
    EXPECT_NE(std::string::npos, result.find("---@param p3 integer"));
    EXPECT_NE(std::string::npos, result.find("---@return string"));
    EXPECT_NE(std::string::npos, result.find("function LuaLS.Cls.abc(p1, p2, p3) end"));
}

TEST_F(InspectTests, LuaProxyVisitorProducesStubs)
{
    struct Cls { void method() {} };
    luabridge::getGlobalNamespace(L)
        .beginNamespace("Proxy")
            .beginClass<Cls>("Cls")
                .addConstructor<void()>()
                .addFunction("method", &Cls::method)
            .endClass()
        .endNamespace();

    auto ns = luabridge::inspectNamespace(L, "Proxy");
    std::stringstream ss;
    luabridge::LuaProxyVisitor v(ss);
    luabridge::accept(ns, v);

    const auto result = ss.str();
    EXPECT_FALSE(result.empty());
    EXPECT_NE(std::string::npos, result.find("Cls"));
    EXPECT_NE(std::string::npos, result.find("method"));
}

//=================================================================================================
// inspect<T>() stack balance test

TEST_F(InspectTests, InspectDoesNotLeakStack)
{
    struct Foo { void m() {} };
    luabridge::getGlobalNamespace(L)
        .beginClass<Foo>("Foo")
            .addFunction("m", &Foo::m)
        .endClass();

    int top = lua_gettop(L);
    auto info = luabridge::inspect<Foo>(L);
    EXPECT_EQ(top, lua_gettop(L));
    EXPECT_EQ("Foo", info.name);
}

TEST_F(InspectTests, InspectNamespaceDoesNotLeakStack)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("StackTest")
            .addFunction("f", +[]() {})
        .endNamespace();

    int top = lua_gettop(L);
    auto ns = luabridge::inspectNamespace(L, "StackTest");
    EXPECT_EQ(top, lua_gettop(L));
    EXPECT_EQ("StackTest", ns.name);
}

//=================================================================================================
// inspectAccept() helper

TEST_F(InspectTests, InspectAcceptDrivesVisitor)
{
    struct Cls { void m() {} };
    luabridge::getGlobalNamespace(L)
        .beginNamespace("AccTest")
            .beginClass<Cls>("Cls")
                .addFunction("m", &Cls::m)
            .endClass()
        .endNamespace();

    struct CountingVisitor : luabridge::InspectVisitor
    {
        int classCount = 0;
        int memberCount = 0;
        void beginClass(const luabridge::ClassInspectInfo&) override { ++classCount; }
        void visitMember(const luabridge::ClassInspectInfo&, const luabridge::MemberInfo&) override { ++memberCount; }
    };

    CountingVisitor cv;
    luabridge::inspectAccept(L, "AccTest", cv);
    EXPECT_EQ(1, cv.classCount);
    EXPECT_GE(cv.memberCount, 1);
}

//=================================================================================================
// withHints() API test (tests FunctionWithHints detection)

TEST_F(InspectTests, WithHintsIsAcceptedByAddFunction)
{
    struct Actor { void attack(float /*damage*/) {} };

    // This test verifies that withHints compiles and the function is registered correctly.
    luabridge::getGlobalNamespace(L)
        .beginClass<Actor>("Actor")
            .addFunction("attack", luabridge::withHints(&Actor::attack, "damage"))
        .endClass();

    // Verify the function is callable from Lua
    auto info = luabridge::inspect<Actor>(L);
    auto* m = findMember(info, "attack");
    ASSERT_NE(nullptr, m);
    EXPECT_EQ(luabridge::MemberKind::Method, m->kind);
}

TEST_F(InspectTests, WithHintsOverloadsCompile)
{
    struct Shooter {};

    luabridge::getGlobalNamespace(L)
        .beginClass<Shooter>("Shooter")
            .addFunction("fire",
                luabridge::withHints([](Shooter&, int /*count*/) {}, "count"),
                luabridge::withHints([](Shooter&, float /*angle*/, int /*count*/) {}, "angle", "count"))
        .endClass();

    auto info = luabridge::inspect<Shooter>(L);
    auto* m = findMember(info, "fire");
    ASSERT_NE(nullptr, m);
    EXPECT_EQ(2u, m->overloads.size());
}

TEST_F(InspectTests, ReflectTypeInfoPopulated)
{
    struct Enemy { void takeDamage(float /*amount*/, int /*count*/) {} };

    luabridge::getGlobalNamespace(L)
        .beginClass<Enemy>("Enemy")
            .addFunction("takeDamage", luabridge::withHints(&Enemy::takeDamage, "amount", "count"))
        .endClass();

    auto info = luabridge::inspect<Enemy>(L);
    auto* m = findMember(info, "takeDamage");
    ASSERT_NE(nullptr, m);
    ASSERT_EQ(1u, m->overloads.size());

    const auto& ov = m->overloads[0];
    ASSERT_EQ(2u, ov.params.size());

    // Type names should be non-empty when LUABRIDGE_ENABLE_REFLECT is active
    EXPECT_FALSE(ov.params[0].typeName.empty());
    EXPECT_FALSE(ov.params[1].typeName.empty());

    // Hints should be populated from withHints
    EXPECT_EQ("amount", ov.params[0].hint);
    EXPECT_EQ("count", ov.params[1].hint);
}

TEST_F(InspectTests, ReflectConstructorTypeInfo)
{
    struct Widget { int x; float y; };

    luabridge::getGlobalNamespace(L)
        .beginClass<Widget>("Widget")
            .addConstructor<void(int, float)>()
        .endClass();

    auto info = luabridge::inspect<Widget>(L);
    auto* ctor = findMember(info, "__call");
    ASSERT_NE(nullptr, ctor);
    ASSERT_EQ(1u, ctor->overloads.size());

    EXPECT_EQ(2u, ctor->overloads[0].params.size());
}

TEST_F(InspectTests, ReflectFreeFunction)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("ReflectNS")
            .addFunction("add",
                luabridge::withHints(+[](int a, int b) -> int { return a + b; }, "a", "b"))
        .endNamespace();

    auto ns = luabridge::inspectNamespace(L, "ReflectNS");
    auto* fn = findFreeMember(ns, "add");
    ASSERT_NE(nullptr, fn);
    ASSERT_EQ(1u, fn->overloads.size());
    ASSERT_EQ(2u, fn->overloads[0].params.size());
    EXPECT_EQ("a", fn->overloads[0].params[0].hint);
    EXPECT_EQ("b", fn->overloads[0].params[1].hint);
}

#endif // LUABRIDGE_ENABLE_REFLECT
