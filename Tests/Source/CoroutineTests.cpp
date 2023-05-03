// https://github.com/kunitoki/LuaBridge3
// Copyright 2023, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#include "TestBase.h"

namespace {
int lua_resume_x(lua_State* L, int nargs)
{
#if LUABRIDGEDEMO_LUAJIT || LUA_VERSION_NUM == 501
    return lua_resume(L, nargs);
#elif LUABRIDGEDEMO_LUAU || LUABRIDGEDEMO_RAVI || LUA_VERSION_NUM < 504
    return lua_resume(L, nullptr, nargs);
#else
    [[maybe_unused]] int nresults = 0;
    return lua_resume(L, nullptr, nargs, &nresults);
#endif
}
} // namespace

struct CoroutineTests : TestBase
{
};

TEST_F(CoroutineTests, LuaRefAsThread)
{
    [[maybe_unused]] lua_State* thread = lua_newthread(L);

    auto x = luabridge::LuaRef::fromStack(L, 1);
    EXPECT_TRUE(x.isThread());
    EXPECT_EQ(L, x.unsafe_cast<lua_State*>());
}

TEST_F(CoroutineTests, LuaRefGlobal)
{
    lua_State* thread = lua_newthread(L);

    if (!runLua("x = 42", thread))
    {
        EXPECT_TRUE(false);
        return;
    }

    luabridge::LuaRef x = luabridge::getGlobal(thread, "x");
    EXPECT_EQ(thread, x.state());
    EXPECT_EQ(42, x.unsafe_cast<int>());

    luabridge::LuaRef y = luabridge::getGlobal(L, "x");
    EXPECT_EQ(L, y.state());
    EXPECT_EQ(42, y.unsafe_cast<int>());
}

TEST_F(CoroutineTests, LuaRefLocal)
{
    lua_State* thread = lua_newthread(L);

    if (!runLua("local x = 42", thread))
    {
        EXPECT_TRUE(false);
        return;
    }

    luabridge::LuaRef x = luabridge::getGlobal(thread, "x");
    EXPECT_EQ(thread, x.state());
    EXPECT_FALSE(!!x.cast<int>());

    luabridge::LuaRef y = luabridge::getGlobal(L, "x");
    EXPECT_EQ(L, y.state());
    EXPECT_FALSE(!!y.cast<int>());
}

TEST_F(CoroutineTests, LuaRefMove)
{
    lua_State* thread1 = lua_newthread(L);
    lua_State* thread2 = lua_newthread(L);

    EXPECT_TRUE(luabridge::push(thread1, 42));
    luabridge::LuaRef x = *luabridge::get<luabridge::LuaRef>(thread1, 1);
    EXPECT_EQ(thread1, x.state());
    EXPECT_EQ(42, x.unsafe_cast<int>());

    EXPECT_TRUE(luabridge::push(thread2, 1337));
    luabridge::LuaRef y = *luabridge::get<luabridge::LuaRef>(thread2, 1);
    EXPECT_EQ(thread2, y.state());
    EXPECT_EQ(1337, y.unsafe_cast<int>());

    x.moveTo(thread2);
    EXPECT_EQ(thread2, x.state());
    EXPECT_EQ(42, x.unsafe_cast<int>());

    y = x;
    EXPECT_EQ(thread2, y.state());
    EXPECT_EQ(42, y.unsafe_cast<int>());
}

TEST_F(CoroutineTests, LuaRefPushInDifferentThread)
{
    lua_State* thread1 = lua_newthread(L);
    lua_State* thread2 = lua_newthread(L);

    luabridge::LuaRef y = luabridge::LuaRef(L, 1337);

    luabridge::setGlobal(thread1, y, "y1");
    luabridge::setGlobal(thread2, y, "y2");

    {
        auto result = luaL_loadstring(thread1, "coroutine.yield(y1)");
        ASSERT_EQ(LUABRIDGE_LUA_OK, result);
    }

    {
        auto result = lua_resume_x(thread1, 0);
        ASSERT_EQ(LUA_YIELD, result);
        EXPECT_EQ(1, lua_gettop(thread1));

        auto x1 = luabridge::LuaRef::fromStack(thread1);
        EXPECT_EQ(y, x1);
    }

    {
        auto result = luaL_loadstring(thread2, "coroutine.yield(y2)");
        ASSERT_EQ(LUABRIDGE_LUA_OK, result);
    }

    {
        auto result = lua_resume_x(thread2, 0);
        ASSERT_EQ(LUA_YIELD, result);
        EXPECT_EQ(1, lua_gettop(thread2));

        auto x1 = luabridge::LuaRef::fromStack(thread2);
        EXPECT_EQ(y, x1);
    }
}

TEST_F(CoroutineTests, ThreadedRegistration)
{
    using namespace luabridge;

    struct SomeClass
    {
        SomeClass(int x) : value(x) {}

        int value = 0;
    };

    // Create a thread
    lua_State* thread2 = lua_newthread(L);
    lua_State* thread1 = lua_newthread(L);

    {
        auto result = luaL_loadstring(thread1, "x = SomeClass(42)");
        ASSERT_EQ(LUABRIDGE_LUA_OK, result);
    }

    // Build the thread local table
    lua_newtable(thread1);
    luabridge::getNamespaceFromStack(thread1)
        .beginClass<SomeClass>("SomeClass")
            .addConstructor<void (*)(int)>()
            .addProperty("value", &SomeClass::value)
        .endClass();

    auto env = luaL_ref(thread1, LUA_REGISTRYINDEX);
    lua_rawgeti(thread1, LUA_REGISTRYINDEX, env);

#if LUABRIDGEDEMO_LUAU || LUABRIDGEDEMO_LUAJIT || LUABRIDGEDEMO_LUA_VERSION == 501
    lua_setfenv(thread1, 1);
#else
    auto upvalue = lua_setupvalue(thread1, -2, 1);
    ASSERT_STREQ("_ENV", upvalue);
#endif

    ASSERT_EQ(LUA_TFUNCTION, lua_type(thread1, -1));
    auto scriptRef = luaL_ref(thread1, LUA_REGISTRYINDEX);
    lua_rawgeti(thread1, LUA_REGISTRYINDEX, scriptRef);

    {
        auto result = lua_resume_x(thread1, 0);
        ASSERT_EQ(LUABRIDGE_LUA_OK, result);
    }

    lua_rawgeti(thread1, LUA_REGISTRYINDEX, env);
    luabridge::LuaRef envx = luabridge::LuaRef::fromStack(thread1, 1);
    luabridge::LuaRef x0 = envx["x"];
    EXPECT_EQ(thread1, x0.state());
    EXPECT_TRUE(!!x0.cast<SomeClass>());
    EXPECT_EQ(42, x0.unsafe_cast<SomeClass>().value);

    // SomeClass x should not "leak" into globals of any thread/state
    luabridge::LuaRef x1 = luabridge::getGlobal(L, "x");
    EXPECT_EQ(L, x1.state());
    EXPECT_FALSE(!!x1.cast<SomeClass>());

    luabridge::LuaRef x2 = luabridge::getGlobal(thread1, "x");
    EXPECT_EQ(thread1, x2.state());
    EXPECT_FALSE(!!x2.cast<SomeClass>());

    luabridge::LuaRef x3 = luabridge::getGlobal(thread2, "x");
    EXPECT_EQ(thread2, x3.state());
    EXPECT_FALSE(!!x3.cast<SomeClass>());

    // Another coroutine will see no luabridge registration
#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_THROW(runLua("x = SomeClass(42)", thread2), std::exception);
#else
    EXPECT_FALSE(runLua("x = SomeClass(42)", thread2));
#endif
}
