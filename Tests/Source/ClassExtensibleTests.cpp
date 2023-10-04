// https://github.com/kunitoki/LuaBridge3
// Copyright 2023, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <unordered_map>

struct ClassExtensibleTests : TestBase
{
    template<class T>
    T variable(const std::string& name)
    {
        runLua("result = " + name);
        return result<T>();
    }
};

namespace {
struct OverridableX
{
    OverridableX() = default;

    luabridge::LuaRef indexMetaMethod(const luabridge::LuaRef& key, lua_State* L);
    luabridge::LuaRef newIndexMetaMethod(const luabridge::LuaRef& key, const luabridge::LuaRef& value, lua_State* L);

    std::unordered_map<luabridge::LuaRef, luabridge::LuaRef> data;
};

luabridge::LuaRef indexMetaMethodFunction(OverridableX& x, const luabridge::LuaRef& key, lua_State* L)
{
    if (key.tostring() == "xyz")
    {
        if (!luabridge::push(L, "123"))
            lua_pushnil(L);
    }
    else
    {
        auto it = x.data.find(key);
        if (it != x.data.end())
            return it->second;

        lua_pushnil(L);
    }

    return luabridge::LuaRef::fromStack(L);
}

luabridge::LuaRef OverridableX::indexMetaMethod(const luabridge::LuaRef& key, lua_State* L)
{
    return indexMetaMethodFunction(*this, key, L);
}

luabridge::LuaRef newIndexMetaMethodFunction(OverridableX& x, const luabridge::LuaRef& key, const luabridge::LuaRef& value, lua_State* L)
{
    x.data.emplace(std::make_pair(key, value));
    return value;
}

luabridge::LuaRef OverridableX::newIndexMetaMethod(const luabridge::LuaRef& key, const luabridge::LuaRef& value, lua_State* L)
{
    return newIndexMetaMethodFunction(*this, key, value, L);
}
} // namespace

TEST_F(ClassExtensibleTests, IndexFallbackMetaMethodMemberFptr)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<OverridableX>("X")
            .addIndexMetaMethod(&OverridableX::indexMetaMethod)
        .endClass();

    OverridableX x;
    luabridge::setGlobal(L, &x, "x");

    runLua("result = x.xyz");
    ASSERT_EQ("123", result<std::string_view>());
}

TEST_F(ClassExtensibleTests, IndexFallbackMetaMethodFunctionPtr)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<OverridableX>("X")
            .addIndexMetaMethod(&indexMetaMethodFunction)
        .endClass();

    OverridableX x;
    luabridge::setGlobal(L, &x, "x");

    runLua("result = x.xyz");
    ASSERT_EQ("123", result<std::string_view>());
}

TEST_F(ClassExtensibleTests, IndexFallbackMetaMethodFreeFunctor)
{
    std::string capture = "123";

    auto indexMetaMethod = [&capture](OverridableX&, luabridge::LuaRef key, lua_State* L) -> luabridge::LuaRef
    {
        if (key.tostring() == "xyz")
        {
            if (!luabridge::push(L, capture + "123"))
                lua_pushnil(L);
        }
        else
        {
            if (!luabridge::push(L, 456))
                lua_pushnil(L);
        }

        return luabridge::LuaRef::fromStack(L);
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<OverridableX>("X")
            .addIndexMetaMethod(indexMetaMethod)
        .endClass();

    OverridableX x;
    luabridge::setGlobal(L, &x, "x");

    runLua("result = x.xyz");
    ASSERT_EQ("123123", result<std::string_view>());
}

TEST_F(ClassExtensibleTests, IndexFallbackMetaMethodFreeFunctorOnClass)
{
    auto indexMetaMethod = [&](OverridableX& x, const luabridge::LuaRef& key, lua_State* L) -> luabridge::LuaRef
    {
        auto it = x.data.find(key);
        if (it != x.data.end())
            return it->second;

        return luabridge::LuaRef(L);
    };

    auto newIndexMetaMethod = [&](OverridableX& x, const luabridge::LuaRef& key, const luabridge::LuaRef& value, lua_State* L) -> luabridge::LuaRef
    {
        x.data.emplace(std::make_pair(key, value));
        return luabridge::LuaRef(L);
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<OverridableX>("X", luabridge::extensibleClass)
            .addIndexMetaMethod(indexMetaMethod)
            .addNewIndexMetaMethod(newIndexMetaMethod)
        .endClass();

    OverridableX x, y;
    luabridge::setGlobal(L, &x, "x");
    luabridge::setGlobal(L, &y, "y");

    runLua(R"(
        X.xyz = 1

        function X:setProperty(v)
            self.xyz = v
        end

        function X:getProperty()
            return self.xyz
        end

        y.xyz = 100
        x:setProperty(2)

        result = x.xyz + y.xyz
    )");
    ASSERT_EQ(102, result<int>());
}

TEST_F(ClassExtensibleTests, IndexFallbackMetaMethodFreeFunctorOnClassOverwriteProperty)
{
    auto indexMetaMethod = [&](OverridableX& x, const luabridge::LuaRef& key, lua_State* L) -> luabridge::LuaRef
    {
        auto it = x.data.find(key);
        if (it != x.data.end())
            return it->second;

        return luabridge::LuaRef(L);
    };

    auto newIndexMetaMethod = [&](OverridableX& x, const luabridge::LuaRef& key, const luabridge::LuaRef& value, lua_State* L) -> luabridge::LuaRef
    {
        x.data.emplace(std::make_pair(key, value));
        return luabridge::LuaRef(L);
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<OverridableX>("X", luabridge::extensibleClass | luabridge::allowOverridingMethods)
            .addIndexMetaMethod(indexMetaMethod)
            .addNewIndexMetaMethod(newIndexMetaMethod)
        .endClass();

    OverridableX x, y;
    luabridge::setGlobal(L, &x, "x");
    luabridge::setGlobal(L, &y, "y");

    runLua(R"(
        function X:property(v)
            self.property = v
        end

        x:property(2)

        result = x.property
    )");
    ASSERT_TRUE(result().isNumber());
    EXPECT_EQ(2, result<int>());
}

TEST_F(ClassExtensibleTests, NewIndexFallbackMetaMethodMemberFptr)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<OverridableX>("X")
            .addIndexMetaMethod(&OverridableX::indexMetaMethod)
            .addNewIndexMetaMethod(&OverridableX::newIndexMetaMethod)
        .endClass();

    OverridableX x;
    luabridge::setGlobal(L, &x, "x");

    runLua("x.qwertyuiop = 123; result = x.qwertyuiop");
    ASSERT_EQ(123, result<int>());
}

TEST_F(ClassExtensibleTests, NewIndexFallbackMetaMethodFunctionPtr)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<OverridableX>("X")
            .addIndexMetaMethod(&indexMetaMethodFunction)
            .addNewIndexMetaMethod(&newIndexMetaMethodFunction)
        .endClass();

    OverridableX x;
    luabridge::setGlobal(L, &x, "x");

    runLua("x.qwertyuiop = 123; result = x.qwertyuiop");
    ASSERT_EQ(123, result<int>());
}

TEST_F(ClassExtensibleTests, NewIndexFallbackMetaMethodFreeFunctor)
{
    int capture = 123;

    auto newIndexMetaMethod = [&capture](OverridableX& x, const luabridge::LuaRef& key, const luabridge::LuaRef& value, lua_State* L) -> luabridge::LuaRef
    {
        if (!luabridge::push(L, capture + value.unsafe_cast<int>()))
            lua_pushnil(L);

        auto v = luabridge::LuaRef::fromStack(L);
        x.data.emplace(std::make_pair(key, v));
        return v;
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<OverridableX>("X")
            .addIndexMetaMethod(&indexMetaMethodFunction)
            .addNewIndexMetaMethod(newIndexMetaMethod)
        .endClass();

    OverridableX x;
    luabridge::setGlobal(L, &x, "x");

    runLua("x.qwertyuiop = 123; result = x.qwertyuiop");
    ASSERT_EQ(246, result<int>());
}

namespace {
struct ExtensibleBase
{
    ExtensibleBase() = default;

    int baseClass() { return 1; }
    int baseClassConst() const { return 2; }

    std::unordered_map<luabridge::LuaRef, luabridge::LuaRef> properties;
};

struct ExtensibleDerived : ExtensibleBase
{
    ExtensibleDerived() = default;

    int derivedClass() { return 11; }
    int derivedClassConst() const { return 22; }
};
} // namespace

TEST_F(ClassExtensibleTests, ExtensibleClass)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<ExtensibleBase>("ExtensibleBase", luabridge::extensibleClass)
            .addConstructor<void(*)()>()
            .addFunction("baseClass", &ExtensibleBase::baseClass)
        .endClass()
    ;

    runLua(R"(
        function ExtensibleBase:test() return 41 + self:baseClass() end

        local base = ExtensibleBase(); result = base:test()
    )");

    EXPECT_EQ(42, result<int>());
}

TEST_F(ClassExtensibleTests, ExtensibleBaseClassNotDerived)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<ExtensibleBase>("ExtensibleBase", luabridge::extensibleClass)
            .addConstructor<void(*)()>()
            .addFunction("baseClass", &ExtensibleBase::baseClass)
        .endClass()
        .deriveClass<ExtensibleDerived, ExtensibleBase>("ExtensibleDerived")
            .addConstructor<void(*)()>()
        .endClass()
    ;

    runLua(R"(
        function ExtensibleBase:test() return 41 + self:baseClass() end

        local derived = ExtensibleDerived(); result = derived:test()
    )");

    EXPECT_EQ(42, result<int>());
}

TEST_F(ClassExtensibleTests, ExtensibleDerivedClassNotBase)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<ExtensibleBase>("ExtensibleBase")
            .addConstructor<void(*)()>()
            .addFunction("baseClass", &ExtensibleBase::baseClass)
        .endClass()
        .deriveClass<ExtensibleDerived, ExtensibleBase>("ExtensibleDerived", luabridge::extensibleClass)
            .addConstructor<void(*)()>()
        .endClass()
    ;

    runLua(R"(
        function ExtensibleDerived:test() return 41 + self:baseClass() end

        local derived = ExtensibleDerived(); result = derived:test()
    )");

    EXPECT_EQ(42, result<int>());
}

TEST_F(ClassExtensibleTests, ExtensibleDerivedClassAndBase)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<ExtensibleBase>("ExtensibleBase", luabridge::extensibleClass)
            .addConstructor<void(*)()>()
            .addFunction("baseClass", &ExtensibleBase::baseClass)
        .endClass()
        .deriveClass<ExtensibleDerived, ExtensibleBase>("ExtensibleDerived", luabridge::extensibleClass)
            .addConstructor<void(*)()>()
            .addFunction("derivedClass", &ExtensibleDerived::derivedClass)
        .endClass()
    ;

    runLua(R"(
        function ExtensibleBase:test1() return self:baseClass() end
        function ExtensibleDerived:test2() return self:derivedClass() end

        local derived = ExtensibleDerived(); result = derived:test1() - derived:test2()
    )");

    EXPECT_EQ(-10, result<int>());
}

TEST_F(ClassExtensibleTests, ExtensibleDerivedClassAndBaseCascading)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<ExtensibleBase>("ExtensibleBase", luabridge::extensibleClass)
            .addConstructor<void(*)()>()
            .addFunction("baseClass", &ExtensibleBase::baseClass)
        .endClass()
        .deriveClass<ExtensibleDerived, ExtensibleBase>("ExtensibleDerived", luabridge::extensibleClass)
            .addConstructor<void(*)()>()
            .addFunction("derivedClass", &ExtensibleDerived::derivedClass)
        .endClass()
    ;

    runLua(R"(
        function ExtensibleBase:testBase() return self:baseClass() end
        function ExtensibleDerived:testDerived() return self:testBase() end

        local derived = ExtensibleDerived(); result = derived:testDerived()
    )");

    EXPECT_EQ(1, result<int>());
}

TEST_F(ClassExtensibleTests, ExtensibleDerivedClassAndBaseSameMethod)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<ExtensibleBase>("ExtensibleBase", luabridge::extensibleClass)
            .addConstructor<void(*)()>()
        .endClass()
        .deriveClass<ExtensibleDerived, ExtensibleBase>("ExtensibleDerived", luabridge::extensibleClass)
            .addConstructor<void(*)()>()
        .endClass()
    ;

    runLua(R"(
        function ExtensibleBase:test() return 1337 end
        function ExtensibleBase:test() return 1338 end -- This is on purpose
        function ExtensibleDerived:test() return 42 end

        local derived = ExtensibleDerived()
        result = derived:test()
    )");

    EXPECT_EQ(42, result<int>());
}

TEST_F(ClassExtensibleTests, ExtensibleClassExtendExistingMethod)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<ExtensibleBase>("ExtensibleBase", luabridge::extensibleClass)
            .addConstructor<void(*)()>()
            .addFunction("baseClass", &ExtensibleBase::baseClass)
            .addFunction("baseClassConst", &ExtensibleBase::baseClassConst)
        .endClass()
    ;

#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_ANY_THROW(runLua(R"(
        function ExtensibleBase:baseClass() return 42 end

        local base = ExtensibleBase(); result = base:baseClass()
    )"));
#else
    EXPECT_FALSE(runLua(R"(
        function ExtensibleBase:baseClass() return 42 end

        local base = ExtensibleBase(); result = base:baseClass()
    )"));
#endif

#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_ANY_THROW(runLua(R"(
        function ExtensibleBase:baseClassConst() return 42 end

        local base = ExtensibleBase(); result = base:baseClassConst()
    )"));
#else
    EXPECT_FALSE(runLua(R"(
        function ExtensibleBase:baseClassConst() return 42 end

        local base = ExtensibleBase(); result = base:baseClassConst()
    )"));
#endif
}

TEST_F(ClassExtensibleTests, ExtensibleClassExtendExistingMethodAllowingOverride)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<ExtensibleBase>("ExtensibleBase", luabridge::extensibleClass | luabridge::allowOverridingMethods)
            .addConstructor<void(*)()>()
            .addFunction("baseClass", &ExtensibleBase::baseClass)
            .addFunction("baseClassConst", &ExtensibleBase::baseClassConst)
        .endClass()
    ;

    runLua(R"(
        function ExtensibleBase:baseClass()
            return 42 + self:super_baseClass()
        end

        local base = ExtensibleBase()
        result = base:baseClass()
    )");

    EXPECT_EQ(43, result<int>());

    runLua(R"(
        function ExtensibleBase:baseClassConst()
            return 42 + self:super_baseClassConst()
        end

        local base = ExtensibleBase()
        result = base:baseClassConst()
    )");

    EXPECT_EQ(44, result<int>());
}

TEST_F(ClassExtensibleTests, ExtensibleDerivedOverrideOneFunctionCallBaseForTheOther)
{
    constexpr auto options = luabridge::extensibleClass | luabridge::allowOverridingMethods;

    luabridge::getGlobalNamespace(L)
        .beginClass<ExtensibleBase>("ExtensibleBase", options)
            .addConstructor<void(*)()>()
            .addFunction("baseClass", &ExtensibleBase::baseClass)
            .addFunction("baseClassConst", &ExtensibleBase::baseClassConst)
        .endClass()
        .deriveClass<ExtensibleDerived, ExtensibleBase>("ExtensibleDerived", options)
            .addConstructor<void(*)()>()
            .addFunction("derivedClass", &ExtensibleDerived::derivedClass)
            .addFunction("derivedClassConst", &ExtensibleDerived::derivedClassConst)
        .endClass()
    ;

    runLua(R"(
        function ExtensibleDerived:baseClass() return 100 + self:super_baseClass() end

        local derived = ExtensibleDerived()
        result = derived:baseClass() + derived:baseClassConst()
    )");

    EXPECT_EQ(103, result<int>());
}

TEST_F(ClassExtensibleTests, ExtensibleClassCustomMetamethods)
{
    auto options = luabridge::allowOverridingMethods | luabridge::extensibleClass;

    luabridge::getGlobalNamespace(L)
        .beginClass<ExtensibleBase>("ExtensibleBase", options)
            .addConstructor<void(*)()>()
            .addFunction("baseClass", &ExtensibleBase::baseClass)
        .endClass()
    ;

    runLua(R"(
        function ExtensibleBase:__tostring()
            return ('ExtensibleBase(%d)'):format(self:baseClass())
        end

        local base = ExtensibleBase(); result = tostring(base)
    )");

    EXPECT_EQ("ExtensibleBase(1)", result<std::string>());
}

TEST_F(ClassExtensibleTests, ExtensibleClassCustomMetamethodsSuper)
{
    auto options = luabridge::allowOverridingMethods | luabridge::extensibleClass;

    luabridge::getGlobalNamespace(L)
        .beginClass<ExtensibleBase>("ExtensibleBase", options)
            .addConstructor<void(*)()>()
        .endClass()
    ;

    runLua(R"(
        function ExtensibleBase:__tostring()
            return '123456 - ' .. self:super__tostring()
        end

        local base = ExtensibleBase(); result = tostring(base)
    )");

    EXPECT_EQ(0u, result<std::string>().find("123456"));
    EXPECT_NE(std::string::npos, result<std::string>().find("ExtensibleBase"));
}

TEST_F(ClassExtensibleTests, ExtensibleClassCustomMetamethodEq)
{
    auto options = luabridge::allowOverridingMethods | luabridge::extensibleClass;

    luabridge::getGlobalNamespace(L)
        .beginClass<ExtensibleBase>("ExtensibleBase", options)
            .addConstructor<void(*)()>()
            .addFunction("baseClass", &ExtensibleBase::baseClass)
        .endClass()
    ;

    runLua(R"(
        function ExtensibleBase:__eq(other)
            return self:baseClass() == other:baseClass()
        end

        local base1 = ExtensibleBase()
        local base2 = ExtensibleBase()
        result = base1 == base2
    )");

    EXPECT_TRUE(result<bool>());
}

TEST_F(ClassExtensibleTests, ExtensibleClassWithCustomIndexMethod)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<ExtensibleBase>("ExtensibleBase", luabridge::extensibleClass)
            .addConstructor<void(*)()>()
            .addFunction("baseClass", &ExtensibleBase::baseClass)
            .addIndexMetaMethod([](ExtensibleBase& self, const luabridge::LuaRef& key, lua_State* L)
            {

                auto it = self.properties.find(key);
                if (it != self.properties.end())
                    return it->second;

                return luabridge::LuaRef(L);
            })
            .addNewIndexMetaMethod([](ExtensibleBase& self, const luabridge::LuaRef& key, const luabridge::LuaRef& value, lua_State* L)
            {
                self.properties.emplace(std::make_pair (key, value));
                return luabridge::LuaRef(L);
            })
        .endClass()
    ;

    runLua(R"(
        function ExtensibleBase:test()
            return 41 + self.xyz + self:baseClass()
        end

        local base = ExtensibleBase()
        base.xyz = 1000
        result = base:test()
    )");

    EXPECT_EQ(1042, result<int>());

    runLua(R"(
        ExtensibleBase.staticProperty = 101;
        result = ExtensibleBase.staticProperty
    )");

    EXPECT_EQ(101, result<int>());

    runLua(R"(
        ExtensibleBase.staticProperty = nil;
        result = ExtensibleBase.staticProperty
    )");

    EXPECT_TRUE(result().isNil());
}

namespace {
class NonExtensible
{
public:
    luabridge::LuaRef getProperty(const luabridge::LuaRef& key, lua_State* L)
    {
        auto it = properties.find(key);
        if (it != properties.end())
            return it->second;

        return luabridge::LuaRef(L);
    }

    luabridge::LuaRef setProperty(const luabridge::LuaRef& key, const luabridge::LuaRef& value, lua_State* L)
    {
        properties.emplace(key, value);
        return luabridge::LuaRef(L);
    }

private:
    std::unordered_map<luabridge::LuaRef, luabridge::LuaRef> properties;
};

class DerivedExtensible : public NonExtensible
{
public:
    inline DerivedExtensible() {};
};
} // namespace

TEST_F(ClassExtensibleTests, IndexAndNewMetaMethodCalledInBaseClass)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<NonExtensible>("NonExtensible")
            .addIndexMetaMethod(&NonExtensible::getProperty)
            .addNewIndexMetaMethod(&NonExtensible::setProperty)
        .endClass()
        .deriveClass<DerivedExtensible, NonExtensible>("DerivedExtensible", luabridge::extensibleClass)
            .addConstructor<void()>()
        .endClass();

    runLua(R"(
        DerivedExtensible.property = 100

        function DerivedExtensible:getProperty()
          return self.property
        end

        function DerivedExtensible:setProperty(value)
          self.property = value
        end

        local test = DerivedExtensible()
        test:setProperty(2)
        result = test:getProperty() + DerivedExtensible.property
    )");

    EXPECT_EQ(102, result<int>());

    runLua(R"(
        local test = DerivedExtensible()
        test.property = 3
        result = test.property
    )");

    EXPECT_EQ(3, result<int>());
}

namespace {
class ExampleStringifiableClass
{
public:
    ExampleStringifiableClass()
        : a(0), b(0), c(0)
    {
    }

    std::string tostring() const
    {
        return "whatever";
    }

    int a, b, c;
};
} // namespace

TEST_F(ClassExtensibleTests, MetatableSecurityNotHidden)
{
    {
        luabridge::getGlobalNamespace(L)
            .beginNamespace("test", luabridge::visibleMetatables)
            .endNamespace();

        runLua("local t = test; result = getmetatable(t)");

        const auto res = result();
        ASSERT_TRUE(res.isTable());
    }

    {
        luabridge::getGlobalNamespace(L)
            .beginClass<ExampleStringifiableClass>("ExampleStringifiableClass", luabridge::visibleMetatables)
                .addConstructor<void(*) ()>()
                .addFunction("__tostring", &ExampleStringifiableClass::tostring)
            .endClass();

        runLua("local t = ExampleStringifiableClass(); result = getmetatable(t)");

        const auto res = result();
        ASSERT_TRUE(res.isTable());
    }
}

TEST_F(ClassExtensibleTests, MetatableSecurity)
{
    {
        luabridge::getGlobalNamespace(L)
            .beginNamespace("test")
            .endNamespace();

        runLua("local t = test; result = getmetatable(t)");

        const auto res = result();
        ASSERT_TRUE(res.isBool());
        EXPECT_FALSE(res.unsafe_cast<bool>());
    }

    {
        luabridge::getGlobalNamespace(L)
            .beginClass<ExampleStringifiableClass>("ExampleStringifiableClass")
                .addConstructor<void(*) ()>()
                .addFunction("__tostring", &ExampleStringifiableClass::tostring)
            .endClass();

        runLua("local t = ExampleStringifiableClass(); result = getmetatable(t)");

        const auto res = result();
        ASSERT_TRUE(res.isBool());
        EXPECT_FALSE(res.unsafe_cast<bool>());
    }
}

#if ! LUABRIDGEDEMO_LUAU && LUABRIDGEDEMO_LUA_VERSION >= 502
TEST_F(ClassExtensibleTests, MetatablePrinting)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<ExampleStringifiableClass>("ExampleStringifiableClass", luabridge::visibleMetatables)
            .addConstructor<void(*) ()>()
        .endClass();

    runLua(R"(
        local text = ''
        local mt = getmetatable(ExampleStringifiableClass)
        for k, v in pairs(mt) do
          text = text .. ('%s - %s'):format(k, v)
        end
        result = text
    )");

    const auto res = result();
    ASSERT_TRUE(res.isString());
    EXPECT_FALSE(res.unsafe_cast<std::string>().empty());
}
#endif
