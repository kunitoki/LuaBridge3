// https://github.com/kunitoki/LuaBridge3
// Copyright 2019, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"

struct IssueTests : TestBase
{
};

namespace {
struct AbstractClass
{
    virtual int sum(int a, int b) = 0;
};

struct ConcreteClass : AbstractClass
{
    int sum(int a, int b) override { return a + b; }

    static AbstractClass& get()
    {
        static ConcreteClass instance;
        return instance;
    }
};
} // namespace

TEST_F(IssueTests, Issue87)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<AbstractClass>("Class")
        .addFunction("sum", &AbstractClass::sum)
        .endClass()
        .addFunction("getAbstractClass", &ConcreteClass::get);

    runLua("result = getAbstractClass ():sum (1, 2)");
    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(3, result<int>());
}

TEST_F(IssueTests, Issue121)
{
    runLua(R"(
    first = {
      second = {
        actual = "data"
      }
    }
  )");
    auto first = luabridge::getGlobal(L, "first");
    ASSERT_TRUE(first.isTable());
    ASSERT_EQ(0, first.length());
    ASSERT_TRUE(first["second"].isTable());
    ASSERT_EQ(0, first["second"].length());
}

namespace {
void pushArgs(lua_State*)
{
}

template<class Arg, class... Args>
void pushArgs(lua_State* L, Arg arg, Args... args)
{
    [[maybe_unused]] auto result = luabridge::Stack<Arg>::push(L, arg);

    pushArgs(L, args...);
}

template<class... Args>
std::vector<luabridge::LuaRef> callFunction(const luabridge::LuaRef& function, Args... args)
{
    assert(function.isFunction());

    lua_State* L = function.state();
    int originalTop = lua_gettop(L);
    function.push(L);
    pushArgs(L, args...);

    luabridge::pcall(L, sizeof...(args), LUA_MULTRET);

    std::vector<luabridge::LuaRef> results;
    int top = lua_gettop(L);
    results.reserve(top - originalTop);
    for (int i = originalTop + 1; i <= top; ++i)
    {
        results.push_back(luabridge::LuaRef::fromStack(L, i));
    }
    return results;
}
} // namespace

TEST_F(IssueTests, Issue160)
{
    runLua("function isConnected (arg1, arg2) "
           " return 1, 'srvname', 'ip:10.0.0.1', arg1, arg2 "
           "end");

    luabridge::LuaRef f_isConnected = luabridge::getGlobal(L, "isConnected");

    auto v = callFunction(f_isConnected, 2, "abc");
    ASSERT_EQ(5u, v.size());
    ASSERT_EQ(1, v[0].unsafe_cast<int>());
    ASSERT_EQ("srvname", v[1].unsafe_cast<std::string>());
    ASSERT_EQ("ip:10.0.0.1", v[2].unsafe_cast<std::string>());
    ASSERT_EQ(2, v[3].unsafe_cast<int>());
    ASSERT_EQ("abc", v[4].unsafe_cast<std::string>());
}

namespace {
struct Vector
{
    float getX() const { return x; }

    float x = 0;
};

struct WideVector : Vector
{
    WideVector(float, float, float, float w) { x = w; }
};
} // namespace

TEST_F(IssueTests, Issue178)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<WideVector>("WideVector")
        .addConstructor<void (*)(float, float, float, float)>()
        .addProperty("x", &Vector::x, true)
        .endClass();

    runLua("result = WideVector (0, 1, 2, 3).x");

    ASSERT_TRUE(result().isNumber());
    ASSERT_EQ(3.f, result<float>());
}

namespace {
enum class MyEnum
{
    VALUE0,
    VALUE1,
};

template <class T>
struct EnumWrapper
{
    static auto push(lua_State* L, T value) -> std::enable_if_t<std::is_enum_v<T>, luabridge::Result>
    {
        lua_pushnumber(L, static_cast<std::size_t>(value));
        return {};
    }

    static auto get(lua_State* L, int index) -> std::enable_if_t<std::is_enum_v<T>, luabridge::TypeResult<T>>
    {
        return static_cast<T>(lua_tointeger(L, index));
    }
};
} // namespace

namespace luabridge {
template<>
struct Stack<MyEnum> : EnumWrapper<MyEnum>
{
};
} // namespace luabridge

TEST_F(IssueTests, Issue127)
{
    runLua("result = 1");
    ASSERT_EQ(MyEnum::VALUE1, result<MyEnum>());
}

TEST_F(IssueTests, Issue8)
{
    runLua(R"(function HelloWorld(args) return args end)");
            
    luabridge::LuaRef func = luabridge::getGlobal(L, "HelloWorld");

    {
        auto result = func("helloworld");
        ASSERT_EQ(1, result.size());
        ASSERT_STREQ("helloworld", result[0].unsafe_cast<const char*>());
    }

    {
        const char* str = "helloworld";
        auto result = func(str);
        ASSERT_EQ(1, result.size());
        ASSERT_STREQ("helloworld", result[0].unsafe_cast<const char*>());
    }

    {
        std::string str = "helloworld";
        auto result = func(std::move(str));
        ASSERT_EQ(1, result.size());
        ASSERT_STREQ("helloworld", result[0].unsafe_cast<const char*>());
    }
}

namespace {
struct SomeClass
{
    luabridge::LuaRef override_;

    SomeClass(lua_State* L)
        : override_(luabridge::main_thread(L))
    {
    }

    void SomeMember()
    {
        if (override_.isFunction())
            override_();
    }
};
} // namespace

TEST_F(IssueTests, IssueMainThread)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<SomeClass>("SomeClass")
        .addConstructor<void (*)(lua_State*)>()
        .addFunction("SomeMember", &SomeClass::SomeMember)
        .addProperty("SomeMemberOverride", &SomeClass::override_)
        .endClass();

    const char* source = R"(
        function test()
            c:SomeMember()
            c.SomeMemberOverride = MyHandler
            c:SomeMember()
            --This is pretty cool too!
            c:SomeMemberOverride()
            --Revert to C++ version
            c.SomeMemberOverride = nil
            c:SomeMember()
            return
        end
        
        function MyHandler()
            print 'SomeMember Overidden by Lua'
        end
    )";

    const char* threadSource = "c = SomeClass()";

    lua_State* thread = lua_newthread(L);

    if (!runLua(threadSource, thread))
    {
        EXPECT_TRUE(false);
        return;
    }

    lua_pop(L, 1);
    lua_gc(L, LUA_GCCOLLECT, 0);

    if (!runLua(source, L))
    {
        EXPECT_TRUE(false);
        return;
    }

    luabridge::LuaRef test = luabridge::getGlobal(L, "test");
    test();
}
