// https://github.com/kunitoki/LuaBridge3
// Copyright 2023, kunitoki
// SPDX-License-Identifier: MIT

#include "TestBase.h"

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
        auto result = luabridge::lua_resume_x(thread1, nullptr, 0);
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
        auto result = luabridge::lua_resume_x(thread2, nullptr, 0);
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
        auto result = luaL_loadstring(thread1, "x = SomeClass.new(42)");
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

#if LUABRIDGE_ON_LUAU || LUABRIDGE_ON_LUAJIT || LUA_VERSION_NUM == 501
    lua_setfenv(thread1, 1);
#else
    auto upvalue = lua_setupvalue(thread1, -2, 1);
    ASSERT_STREQ("_ENV", upvalue);
#endif

    ASSERT_EQ(LUA_TFUNCTION, lua_type(thread1, -1));
    auto scriptRef = luaL_ref(thread1, LUA_REGISTRYINDEX);
    lua_rawgeti(thread1, LUA_REGISTRYINDEX, scriptRef);

    {
        auto result = luabridge::lua_resume_x(thread1, nullptr, 0);
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
    EXPECT_THROW(runLua("x = SomeClass.new(42)", thread2), std::exception);
#else
    EXPECT_FALSE(runLua("x = SomeClass.new(42)", thread2));
#endif
}

//=============================================================================
// C++20 coroutine tests
//=============================================================================
#if LUABRIDGE_HAS_CXX20_COROUTINES

struct CppCoroutineTests : TestBase
{
};

TEST_F(CppCoroutineTests, SingleYield)
{
    luabridge::getGlobalNamespace(L)
        .addCoroutine("gen", []() -> luabridge::CppCoroutine<int>
        {
            co_yield 42;
            co_return 0;
        });

    // coroutine.wrap returns a function; each call resumes
    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(gen)\n"
        "first  = f()\n"   // first resume: yields 42
        "second = f()\n"   // second resume: returns 0
    ));

    EXPECT_EQ(42, luabridge::getGlobal(L, "first").unsafe_cast<int>());
    EXPECT_EQ(0,  luabridge::getGlobal(L, "second").unsafe_cast<int>());
}

TEST_F(CppCoroutineTests, MultipleYields)
{
    luabridge::getGlobalNamespace(L)
        .addCoroutine("counter", []() -> luabridge::CppCoroutine<int>
        {
            co_yield 1;
            co_yield 2;
            co_yield 3;
            co_return -1;
        });

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(counter)\n"
        "a = f()\n"
        "b = f()\n"
        "c = f()\n"
        "d = f()\n"
    ));

    EXPECT_EQ(1,  luabridge::getGlobal(L, "a").unsafe_cast<int>());
    EXPECT_EQ(2,  luabridge::getGlobal(L, "b").unsafe_cast<int>());
    EXPECT_EQ(3,  luabridge::getGlobal(L, "c").unsafe_cast<int>());
    EXPECT_EQ(-1, luabridge::getGlobal(L, "d").unsafe_cast<int>());
}

TEST_F(CppCoroutineTests, ReturnWithNoYield)
{
    luabridge::getGlobalNamespace(L)
        .addCoroutine("immediate", []() -> luabridge::CppCoroutine<int>
        {
            co_return 99;
        });

    // No yield — coroutine.wrap returns 99 on the first (and only) call
    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(immediate)\n"
        "result = f()\n"
    ));

    EXPECT_EQ(99, luabridge::getGlobal(L, "result").unsafe_cast<int>());
}

TEST_F(CppCoroutineTests, CoroutineWithArguments)
{
    luabridge::getGlobalNamespace(L)
        .addCoroutine("adder", [](int a, int b) -> luabridge::CppCoroutine<int>
        {
            co_yield a + b;
            co_return a * b;
        });

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(adder)\n"
        "s = f(3, 4)\n"   // yield: 3+4=7
        "p = f()\n"        // return: 3*4=12
    ));

    EXPECT_EQ(7,  luabridge::getGlobal(L, "s").unsafe_cast<int>());
    EXPECT_EQ(12, luabridge::getGlobal(L, "p").unsafe_cast<int>());
}

TEST_F(CppCoroutineTests, VoidCoroutine)
{
    int sideEffect = 0;

    luabridge::getGlobalNamespace(L)
        .addCoroutine("doWork", [&sideEffect]() -> luabridge::CppCoroutine<void>
        {
            sideEffect = 1;
            co_return;
        });

    ASSERT_TRUE(runLua("doWork = coroutine.wrap(doWork)\ndoWork()"));
    EXPECT_EQ(1, sideEffect);
}

TEST_F(CppCoroutineTests, StringYield)
{
    luabridge::getGlobalNamespace(L)
        .addCoroutine("greet", []() -> luabridge::CppCoroutine<std::string>
        {
            co_yield std::string("hello");
            co_return std::string("world");
        });

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(greet)\n"
        "g1 = f()\n"
        "g2 = f()\n"
    ));

    EXPECT_EQ("hello", luabridge::getGlobal(L, "g1").unsafe_cast<std::string>());
    EXPECT_EQ("world", luabridge::getGlobal(L, "g2").unsafe_cast<std::string>());
}

#if LUABRIDGE_HAS_EXCEPTIONS
TEST_F(CppCoroutineTests, ExceptionInCoroutine)
{
    luabridge::getGlobalNamespace(L)
        .addCoroutine("boom", []() -> luabridge::CppCoroutine<int>
        {
            throw std::runtime_error("coroutine error");
            co_return 0;
        });

    ASSERT_TRUE(runLua(
        "local ok, err = pcall(coroutine.wrap(boom))\n"
        "success = ok\n"
        "errmsg  = err\n"
    ));

    EXPECT_FALSE(luabridge::getGlobal(L, "success").unsafe_cast<bool>());
    auto msg = luabridge::getGlobal(L, "errmsg").unsafe_cast<std::string>();
    EXPECT_NE(std::string::npos, msg.find("coroutine error"));
}

TEST_F(CppCoroutineTests, ExceptionOnSecondYield)
{
    luabridge::getGlobalNamespace(L)
        .addCoroutine("boom2", []() -> luabridge::CppCoroutine<int>
        {
            co_yield 1;
            throw std::runtime_error("second yield error");
            co_return 0;
        });

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(boom2)\n"
        "first = f()\n"  // yields 1 successfully
        "local ok, err = pcall(f)\n"
        "success = ok\n"
        "errmsg  = err\n"
    ));

    EXPECT_EQ(1, luabridge::getGlobal(L, "first").unsafe_cast<int>());
    EXPECT_FALSE(luabridge::getGlobal(L, "success").unsafe_cast<bool>());
    auto msg = luabridge::getGlobal(L, "errmsg").unsafe_cast<std::string>();
    EXPECT_NE(std::string::npos, msg.find("second yield error"));
}

TEST_F(CppCoroutineTests, ExceptionType_LogicError)
{
    // std::logic_error (not runtime_error) — verifies any std::exception subclass is caught
    // and its what() message is forwarded as the Lua error string.
    luabridge::getGlobalNamespace(L)
        .addCoroutine("badlogic", []() -> luabridge::CppCoroutine<int>
        {
            throw std::logic_error("logic went wrong");
            co_return 0;
        });

    ASSERT_TRUE(runLua(
        "local ok, err = pcall(coroutine.wrap(badlogic))\n"
        "success = ok\n"
        "errmsg  = err\n"
    ));

    EXPECT_FALSE(luabridge::getGlobal(L, "success").unsafe_cast<bool>());
    auto msg = luabridge::getGlobal(L, "errmsg").unsafe_cast<std::string>();
    EXPECT_NE(std::string::npos, msg.find("logic went wrong"));
}

TEST_F(CppCoroutineTests, ExceptionType_NonStdThrow)
{
    // Throwing a non-std type (int) hits the catch(...) path in raise_from_exception,
    // which produces the fixed message "unknown exception in C++ coroutine".
    luabridge::getGlobalNamespace(L)
        .addCoroutine("intthrow", []() -> luabridge::CppCoroutine<int>
        {
            throw 42; // NOLINT: intentional non-std throw to exercise catch(...)
            co_return 0;
        });

    ASSERT_TRUE(runLua(
        "local ok, err = pcall(coroutine.wrap(intthrow))\n"
        "success = ok\n"
        "errmsg  = err\n"
    ));

    EXPECT_FALSE(luabridge::getGlobal(L, "success").unsafe_cast<bool>());
    auto msg = luabridge::getGlobal(L, "errmsg").unsafe_cast<std::string>();
    EXPECT_NE(std::string::npos, msg.find("unknown exception in C++ coroutine"));
}

TEST_F(CppCoroutineTests, ExceptionOnNthResume)
{
    // Exception thrown on the 4th resume (after 3 successful yields), verifying that
    // the continuation path handles late exceptions correctly.
    luabridge::getGlobalNamespace(L)
        .addCoroutine("latecrash", []() -> luabridge::CppCoroutine<int>
        {
            co_yield 10;
            co_yield 20;
            co_yield 30;
            throw std::runtime_error("crashed on fourth");
            co_return 0;
        });

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(latecrash)\n"
        "r1 = f()\n"
        "r2 = f()\n"
        "r3 = f()\n"
        "local ok, err = pcall(f)\n"
        "success = ok\n"
        "errmsg  = err\n"
    ));

    EXPECT_EQ(10, luabridge::getGlobal(L, "r1").unsafe_cast<int>());
    EXPECT_EQ(20, luabridge::getGlobal(L, "r2").unsafe_cast<int>());
    EXPECT_EQ(30, luabridge::getGlobal(L, "r3").unsafe_cast<int>());
    EXPECT_FALSE(luabridge::getGlobal(L, "success").unsafe_cast<bool>());
    auto msg = luabridge::getGlobal(L, "errmsg").unsafe_cast<std::string>();
    EXPECT_NE(std::string::npos, msg.find("crashed on fourth"));
}

TEST_F(CppCoroutineTests, ExceptionFromNestedCall)
{
    // Exception is not thrown directly but propagates up from a nested helper function,
    // exercising that the coroutine frame capture works for indirect throw sites.
    auto helper = []() { throw std::runtime_error("nested throw"); };

    luabridge::getGlobalNamespace(L)
        .addCoroutine("nested", [helper]() -> luabridge::CppCoroutine<int>
        {
            co_yield 1;
            helper(); // throws from a non-coroutine call frame
            co_return 0;
        });

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(nested)\n"
        "first = f()\n"
        "local ok, err = pcall(f)\n"
        "success = ok\n"
        "errmsg  = err\n"
    ));

    EXPECT_EQ(1, luabridge::getGlobal(L, "first").unsafe_cast<int>());
    EXPECT_FALSE(luabridge::getGlobal(L, "success").unsafe_cast<bool>());
    auto msg = luabridge::getGlobal(L, "errmsg").unsafe_cast<std::string>();
    EXPECT_NE(std::string::npos, msg.find("nested throw"));
}

TEST_F(CppCoroutineTests, ExceptionCleansUpRAII)
{
    // When an exception unwinds the coroutine body, local objects with destructors must
    // still be destroyed. Guard tracks whether its destructor ran.
    int destructed = 0;
    struct Guard { int* p; ~Guard() { ++(*p); } };

    luabridge::getGlobalNamespace(L)
        .addCoroutine("guardedcrash", [&destructed]() -> luabridge::CppCoroutine<int>
        {
            Guard g{ &destructed };
            co_yield 1;
            throw std::runtime_error("unwind me");
            co_return 0;
        });

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(guardedcrash)\n"
        "first = f()\n"
        "local ok, err = pcall(f)\n"
        "success = ok\n"
        "errmsg  = err\n"
    ));

    EXPECT_EQ(1,     luabridge::getGlobal(L, "first").unsafe_cast<int>());
    EXPECT_FALSE(luabridge::getGlobal(L, "success").unsafe_cast<bool>());
    auto msg = luabridge::getGlobal(L, "errmsg").unsafe_cast<std::string>();
    EXPECT_NE(std::string::npos, msg.find("unwind me"));

    // Force GC so the CppCoroutineFrame __gc runs, triggering the handle destructor
    // which in turn unwinds any remaining suspension points.
    lua_gc(L, LUA_GCCOLLECT, 0);
    lua_gc(L, LUA_GCCOLLECT, 0);

    EXPECT_EQ(1, destructed);
}

TEST_F(CppCoroutineTests, ExceptionViaCoroutineResumeApi)
{
    // Use coroutine.create + coroutine.resume instead of coroutine.wrap.
    // On exception coroutine.resume returns false plus the error message.
    luabridge::getGlobalNamespace(L)
        .addCoroutine("boomboom", []() -> luabridge::CppCoroutine<int>
        {
            co_yield 7;
            throw std::runtime_error("resume api error");
            co_return 0;
        });

    ASSERT_TRUE(runLua(
        "local co = coroutine.create(boomboom)\n"
        "local ok1, v1  = coroutine.resume(co)\n"   // first resume: yields 7
        "local ok2, err = coroutine.resume(co)\n"   // second resume: throws
        "resumeOk1 = ok1\n"
        "yieldVal  = v1\n"
        "resumeOk2 = ok2\n"
        "errmsg    = err\n"
    ));

    EXPECT_TRUE(luabridge::getGlobal(L, "resumeOk1").unsafe_cast<bool>());
    EXPECT_EQ(7, luabridge::getGlobal(L, "yieldVal").unsafe_cast<int>());
    EXPECT_FALSE(luabridge::getGlobal(L, "resumeOk2").unsafe_cast<bool>());
    auto msg = luabridge::getGlobal(L, "errmsg").unsafe_cast<std::string>();
    EXPECT_NE(std::string::npos, msg.find("resume api error"));
}

TEST_F(CppCoroutineTests, ExceptionLeavesCoroutineDead)
{
    // After an exception kills a coroutine, further resume attempts should fail with
    // a "cannot resume dead coroutine" error, not a C++ exception or crash.
    luabridge::getGlobalNamespace(L)
        .addCoroutine("dieonce", []() -> luabridge::CppCoroutine<int>
        {
            throw std::runtime_error("die");
            co_return 0;
        });

    ASSERT_TRUE(runLua(
        "local co = coroutine.create(dieonce)\n"
        "local ok1, _  = coroutine.resume(co)\n"   // exception: coroutine dies
        "local ok2, err = coroutine.resume(co)\n"  // resuming a dead coroutine
        "firstOk  = ok1\n"
        "secondOk = ok2\n"
        "deadErr  = err\n"
    ));

    EXPECT_FALSE(luabridge::getGlobal(L, "firstOk").unsafe_cast<bool>());
    EXPECT_FALSE(luabridge::getGlobal(L, "secondOk").unsafe_cast<bool>());
    auto msg = luabridge::getGlobal(L, "deadErr").unsafe_cast<std::string>();
    EXPECT_NE(std::string::npos, msg.find("dead"));
}

TEST_F(CppCoroutineTests, ExceptionMessagePassedVerbatim)
{
    // The exact what() text must survive the C++ → Lua error boundary unchanged.
    const std::string uniqueMsg = "unique-error-XYZ-12345";

    luabridge::getGlobalNamespace(L)
        .addCoroutine("verbatim", [&uniqueMsg]() -> luabridge::CppCoroutine<int>
        {
            throw std::runtime_error(uniqueMsg);
            co_return 0;
        });

    ASSERT_TRUE(runLua(
        "local ok, err = pcall(coroutine.wrap(verbatim))\n"
        "success = ok\n"
        "errmsg  = err\n"
    ));

    EXPECT_FALSE(luabridge::getGlobal(L, "success").unsafe_cast<bool>());
    auto msg = luabridge::getGlobal(L, "errmsg").unsafe_cast<std::string>();
    EXPECT_NE(std::string::npos, msg.find(uniqueMsg));
}
#endif // LUABRIDGE_HAS_EXCEPTIONS

TEST_F(CppCoroutineTests, FloatYield)
{
    luabridge::getGlobalNamespace(L)
        .addCoroutine("floats", []() -> luabridge::CppCoroutine<float>
        {
            co_yield 1.5f;
            co_yield 2.5f;
            co_return 3.5f;
        });

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(floats)\n"
        "a = f()\n"
        "b = f()\n"
        "c = f()\n"
    ));

    EXPECT_FLOAT_EQ(1.5f, luabridge::getGlobal(L, "a").unsafe_cast<float>());
    EXPECT_FLOAT_EQ(2.5f, luabridge::getGlobal(L, "b").unsafe_cast<float>());
    EXPECT_FLOAT_EQ(3.5f, luabridge::getGlobal(L, "c").unsafe_cast<float>());
}

TEST_F(CppCoroutineTests, BoolYield)
{
    luabridge::getGlobalNamespace(L)
        .addCoroutine("bools", []() -> luabridge::CppCoroutine<bool>
        {
            co_yield true;
            co_yield false;
            co_return true;
        });

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(bools)\n"
        "a = f()\n"
        "b = f()\n"
        "c = f()\n"
    ));

    EXPECT_TRUE(luabridge::getGlobal(L, "a").unsafe_cast<bool>());
    EXPECT_FALSE(luabridge::getGlobal(L, "b").unsafe_cast<bool>());
    EXPECT_TRUE(luabridge::getGlobal(L, "c").unsafe_cast<bool>());
}

TEST_F(CppCoroutineTests, ManyArguments)
{
    luabridge::getGlobalNamespace(L)
        .addCoroutine("sum4", [](int a, int b, int c, int d) -> luabridge::CppCoroutine<int>
        {
            co_yield a + b;
            co_return c + d;
        });

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(sum4)\n"
        "s1 = f(1, 2, 3, 4)\n"  // yield: 1+2=3
        "s2 = f()\n"            // return: 3+4=7
    ));

    EXPECT_EQ(3, luabridge::getGlobal(L, "s1").unsafe_cast<int>());
    EXPECT_EQ(7, luabridge::getGlobal(L, "s2").unsafe_cast<int>());
}

TEST_F(CppCoroutineTests, LuaStateArgument)
{
    luabridge::getGlobalNamespace(L)
        .addCoroutine("reader", [](lua_State* LS) -> luabridge::CppCoroutine<int>
        {
            lua_getglobal(LS, "magic");
            int val = static_cast<int>(lua_tointeger(LS, -1));
            lua_pop(LS, 1);
            co_return val;
        });

    ASSERT_TRUE(runLua(
        "magic = 77\n"
        "local f = coroutine.wrap(reader)\n"
        "result = f()\n"
    ));

    EXPECT_EQ(77, luabridge::getGlobal(L, "result").unsafe_cast<int>());
}

TEST_F(CppCoroutineTests, FibonacciGenerator)
{
    luabridge::getGlobalNamespace(L)
        .addCoroutine("fib", [](int n) -> luabridge::CppCoroutine<int>
        {
            int a = 0, b = 1;
            for (int i = 0; i < n; ++i)
            {
                co_yield a;
                int tmp = a + b;
                a = b;
                b = tmp;
            }
            co_return -1;
        });

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(fib)\n"
        "r0 = f(6)\n"  // 0
        "r1 = f()\n"   // 1
        "r2 = f()\n"   // 1
        "r3 = f()\n"   // 2
        "r4 = f()\n"   // 3
        "r5 = f()\n"   // 5
        "r6 = f()\n"   // -1 (done)
    ));

    EXPECT_EQ(0,  luabridge::getGlobal(L, "r0").unsafe_cast<int>());
    EXPECT_EQ(1,  luabridge::getGlobal(L, "r1").unsafe_cast<int>());
    EXPECT_EQ(1,  luabridge::getGlobal(L, "r2").unsafe_cast<int>());
    EXPECT_EQ(2,  luabridge::getGlobal(L, "r3").unsafe_cast<int>());
    EXPECT_EQ(3,  luabridge::getGlobal(L, "r4").unsafe_cast<int>());
    EXPECT_EQ(5,  luabridge::getGlobal(L, "r5").unsafe_cast<int>());
    EXPECT_EQ(-1, luabridge::getGlobal(L, "r6").unsafe_cast<int>());
}

TEST_F(CppCoroutineTests, LargeYieldCount)
{
    luabridge::getGlobalNamespace(L)
        .addCoroutine("counter100", []() -> luabridge::CppCoroutine<int>
        {
            for (int i = 0; i < 100; ++i)
                co_yield i;
            co_return -1;
        });

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(counter100)\n"
        "local sum = 0\n"
        "for i = 0, 99 do\n"
        "    sum = sum + f()\n"
        "end\n"
        "last = f()\n"
        "total = sum\n"
    ));

    // sum of 0..99 = 4950
    EXPECT_EQ(4950, luabridge::getGlobal(L, "total").unsafe_cast<int>());
    EXPECT_EQ(-1,   luabridge::getGlobal(L, "last").unsafe_cast<int>());
}

TEST_F(CppCoroutineTests, ConcurrentInstances)
{
    // Two independent instances of the same coroutine factory run concurrently
    luabridge::getGlobalNamespace(L)
        .addCoroutine("counter", []() -> luabridge::CppCoroutine<int>
        {
            co_yield 1;
            co_yield 2;
            co_return 3;
        });

    ASSERT_TRUE(runLua(
        "local a = coroutine.wrap(counter)\n"
        "local b = coroutine.wrap(counter)\n"
        "a1 = a()\n"  // instance A: 1
        "b1 = b()\n"  // instance B: 1 (independent)
        "a2 = a()\n"  // instance A: 2
        "b2 = b()\n"  // instance B: 2
        "a3 = a()\n"  // instance A: 3
        "b3 = b()\n"  // instance B: 3
    ));

    EXPECT_EQ(1, luabridge::getGlobal(L, "a1").unsafe_cast<int>());
    EXPECT_EQ(1, luabridge::getGlobal(L, "b1").unsafe_cast<int>());
    EXPECT_EQ(2, luabridge::getGlobal(L, "a2").unsafe_cast<int>());
    EXPECT_EQ(2, luabridge::getGlobal(L, "b2").unsafe_cast<int>());
    EXPECT_EQ(3, luabridge::getGlobal(L, "a3").unsafe_cast<int>());
    EXPECT_EQ(3, luabridge::getGlobal(L, "b3").unsafe_cast<int>());
}

TEST_F(CppCoroutineTests, AbandonedCoroutineNoLeak)
{
    // Coroutine is created and partially iterated, then abandoned.
    // The __gc metamethod on the frame userdata must clean up without crashing.
    int destructed = 0;
    struct Guard { int* p; ~Guard() { ++(*p); } };

    luabridge::getGlobalNamespace(L)
        .addCoroutine("guarded", [&destructed]() -> luabridge::CppCoroutine<int>
        {
            Guard g{ &destructed };
            co_yield 1;
            co_yield 2;
            co_return 3;
        });

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(guarded)\n"
        "first = f()\n"  // resume once; guard is alive, suspended after first yield
        // f goes out of scope here; GC will collect the coroutine frame
    ));

    EXPECT_EQ(1, luabridge::getGlobal(L, "first").unsafe_cast<int>());

    // Force a full GC cycle to trigger __gc on the frame userdata
    lua_gc(L, LUA_GCCOLLECT, 0);
    lua_gc(L, LUA_GCCOLLECT, 0);

    // The Guard destructor should have run, proving the C++ frame was cleaned up
    EXPECT_EQ(1, destructed);
}

TEST_F(CppCoroutineTests, VoidCoroutineWithSideEffects)
{
    // Verify multiple void co_return paths all execute correctly
    std::vector<int> log;

    luabridge::getGlobalNamespace(L)
        .addCoroutine("worker", [&log](int x) -> luabridge::CppCoroutine<void>
        {
            log.push_back(x);
            log.push_back(x * 2);
            co_return;
        });

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(worker)\n"
        "f(5)\n"
    ));

    ASSERT_EQ(2u, log.size());
    EXPECT_EQ(5,  log[0]);
    EXPECT_EQ(10, log[1]);
}

TEST_F(CppCoroutineTests, LuaCoroutineAwaited)
{
    // Load a Lua function that yields a value
    ASSERT_TRUE(runLua(
        "function luaGen()\n"
        "  coroutine.yield(123)\n"
        "end\n"
    ));

    int capturedNresults = 0;

    luabridge::getGlobalNamespace(L)
        .addCoroutine("driver", [&capturedNresults](lua_State* L) -> luabridge::CppCoroutine<int>
        {
            // Create a child Lua thread. lua_newthread pushes the thread onto L's stack;
            // anchor it in the registry so GC won't collect it, and pop it from L's stack.
            lua_State* child = lua_newthread(L);
            int childRef = luaL_ref(L, LUA_REGISTRYINDEX); // pops thread from L's stack

            lua_getglobal(child, "luaGen");

            // Resume the child Lua coroutine synchronously
            auto [status, nresults] = co_await luabridge::LuaCoroutine{ child, L };
            capturedNresults = nresults;

            // The child's stack now has the yielded value
            int yielded = 0;
            if (nresults > 0)
                yielded = static_cast<int>(lua_tointeger(child, -nresults));

            luaL_unref(L, LUA_REGISTRYINDEX, childRef);
            co_return yielded;
        });

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(driver)\n"
        "result = f()\n"
    ));

    EXPECT_EQ(1, capturedNresults);
    EXPECT_EQ(123, luabridge::getGlobal(L, "result").unsafe_cast<int>());
}

TEST_F(CppCoroutineTests, MultipleLuaCoroutineAwaits)
{
    // C++ coroutine awaits two different Lua child threads in sequence
    ASSERT_TRUE(runLua(
        "function luaA() coroutine.yield(10) end\n"
        "function luaB() coroutine.yield(20) end\n"
    ));

    int sumResults = 0;

    luabridge::getGlobalNamespace(L)
        .addCoroutine("driver2", [&sumResults](lua_State* LS) -> luabridge::CppCoroutine<int>
        {
            lua_State* childA = lua_newthread(LS);
            int refA = luaL_ref(LS, LUA_REGISTRYINDEX);
            lua_getglobal(childA, "luaA");
            auto [stA, nA] = co_await luabridge::LuaCoroutine{ childA, LS };
            int a = (nA > 0) ? static_cast<int>(lua_tointeger(childA, -nA)) : 0;
            luaL_unref(LS, LUA_REGISTRYINDEX, refA);

            lua_State* childB = lua_newthread(LS);
            int refB = luaL_ref(LS, LUA_REGISTRYINDEX);
            lua_getglobal(childB, "luaB");
            auto [stB, nB] = co_await luabridge::LuaCoroutine{ childB, LS };
            int b = (nB > 0) ? static_cast<int>(lua_tointeger(childB, -nB)) : 0;
            luaL_unref(LS, LUA_REGISTRYINDEX, refB);

            sumResults = a + b;
            co_return sumResults;
        });

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(driver2)\n"
        "result2 = f()\n"
    ));

    EXPECT_EQ(30, sumResults);
    EXPECT_EQ(30, luabridge::getGlobal(L, "result2").unsafe_cast<int>());
}

TEST_F(CppCoroutineTests, LuaCoroutineChildReturnsImmediately)
{
    // Child Lua thread returns a value without yielding (LUA_OK instead of LUA_YIELD)
    ASSERT_TRUE(runLua(
        "function luaImmediate() return 42 end\n"
    ));

    int capturedStatus = -1;
    int capturedN = -1;

    luabridge::getGlobalNamespace(L)
        .addCoroutine("driverImm", [&capturedStatus, &capturedN](lua_State* LS) -> luabridge::CppCoroutine<int>
        {
            lua_State* child = lua_newthread(LS);
            int ref = luaL_ref(LS, LUA_REGISTRYINDEX);
            lua_getglobal(child, "luaImmediate");

            auto [st, n] = co_await luabridge::LuaCoroutine{ child, LS };
            capturedStatus = st;
            capturedN = n;

            int val = (n > 0) ? static_cast<int>(lua_tointeger(child, -n)) : 0;
            luaL_unref(LS, LUA_REGISTRYINDEX, ref);
            co_return val;
        });

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(driverImm)\n"
        "immResult = f()\n"
    ));

    EXPECT_EQ(LUABRIDGE_LUA_OK, capturedStatus);
    EXPECT_EQ(1, capturedN);
    EXPECT_EQ(42, luabridge::getGlobal(L, "immResult").unsafe_cast<int>());
}

TEST_F(CppCoroutineTests, CoroutineStateCapture)
{
    // Lambda captures mutable state; verify it persists correctly across suspensions
    luabridge::getGlobalNamespace(L)
        .addCoroutine("stateful", [acc = 0](int step) mutable -> luabridge::CppCoroutine<int>
        {
            acc += step;
            co_yield acc;  // acc = step
            acc += step;
            co_yield acc;  // acc = 2*step
            acc += step;
            co_return acc; // acc = 3*step
        });

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(stateful)\n"
        "v1 = f(7)\n"
        "v2 = f()\n"
        "v3 = f()\n"
    ));

    EXPECT_EQ(7,  luabridge::getGlobal(L, "v1").unsafe_cast<int>());
    EXPECT_EQ(14, luabridge::getGlobal(L, "v2").unsafe_cast<int>());
    EXPECT_EQ(21, luabridge::getGlobal(L, "v3").unsafe_cast<int>());
}

TEST_F(CppCoroutineTests, LuaIteratorPattern)
{
    // Classic Lua iterator: use C++ coroutine as the iterator function in a for-in loop
    luabridge::getGlobalNamespace(L)
        .addCoroutine("range", [](int from, int to) -> luabridge::CppCoroutine<int>
        {
            for (int i = from; i <= to; ++i)
                co_yield i;
            co_return -1; // sentinel for the loop termination via nil-check isn't used here
        });

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(range)\n"
        "local sum = 0\n"
        "local v = f(1, 5)\n"
        "while v ~= -1 do\n"
        "    sum = sum + v\n"
        "    v = f()\n"
        "end\n"
        "rangeSum = sum\n"
    ));

    EXPECT_EQ(15, luabridge::getGlobal(L, "rangeSum").unsafe_cast<int>());
}

//=============================================================================
// Class<T>::addStaticCoroutine and Class<T>::addCoroutine tests
//=============================================================================

struct CppCoroutineClassTests : TestBase
{
    struct Counter
    {
        int value = 0;

        void increment() { ++value; }
    };
};

TEST_F(CppCoroutineClassTests, StaticCoroutine_Basic)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Counter>("Counter")
            .addConstructor<void()>()
            .addStaticCoroutine("generate", []() -> luabridge::CppCoroutine<int>
            {
                co_yield 10;
                co_return 20;
            })
        .endClass();

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(Counter.generate)\n"
        "first  = f()\n"
        "second = f()\n"
    ));

    EXPECT_EQ(10, luabridge::getGlobal(L, "first").unsafe_cast<int>());
    EXPECT_EQ(20, luabridge::getGlobal(L, "second").unsafe_cast<int>());
}

TEST_F(CppCoroutineClassTests, StaticCoroutine_WithArguments)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Counter>("Counter")
            .addConstructor<void()>()
            .addStaticCoroutine("range", [](int from, int count) -> luabridge::CppCoroutine<int>
            {
                for (int i = 0; i < count; ++i)
                    co_yield from + i;
                co_return -1;
            })
        .endClass();

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(Counter.range)\n"
        "a = f(5, 3)\n"
        "b = f()\n"
        "c = f()\n"
        "d = f()\n"
    ));

    EXPECT_EQ(5,  luabridge::getGlobal(L, "a").unsafe_cast<int>());
    EXPECT_EQ(6,  luabridge::getGlobal(L, "b").unsafe_cast<int>());
    EXPECT_EQ(7,  luabridge::getGlobal(L, "c").unsafe_cast<int>());
    EXPECT_EQ(-1, luabridge::getGlobal(L, "d").unsafe_cast<int>());
}

TEST_F(CppCoroutineClassTests, StaticCoroutine_VoidReturn)
{
    int sideEffect = 0;

    luabridge::getGlobalNamespace(L)
        .beginClass<Counter>("Counter")
            .addConstructor<void()>()
            .addStaticCoroutine("doWork", [&sideEffect](int x) -> luabridge::CppCoroutine<void>
            {
                sideEffect = x;
                co_return;
            })
        .endClass();

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(Counter.doWork)\n"
        "f(42)\n"
    ));

    EXPECT_EQ(42, sideEffect);
}

TEST_F(CppCoroutineClassTests, MemberCoroutine_NonConst)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Counter>("Counter")
            .addConstructor<void()>()
            .addProperty("value", &Counter::value)
            .addCoroutine("generate", [](Counter* obj, int n) -> luabridge::CppCoroutine<int>
            {
                for (int i = 0; i < n; ++i)
                {
                    co_yield obj->value;
                    obj->increment();
                }
                co_return -1;
            })
        .endClass();

    ASSERT_TRUE(runLua(
        "local obj = Counter()\n"
        "local f = coroutine.wrap(Counter.generate)\n"
        "a = f(obj, 3)\n"
        "b = f()\n"
        "c = f()\n"
        "d = f()\n"
        "finalVal = obj.value\n"
    ));

    EXPECT_EQ(0,  luabridge::getGlobal(L, "a").unsafe_cast<int>());
    EXPECT_EQ(1,  luabridge::getGlobal(L, "b").unsafe_cast<int>());
    EXPECT_EQ(2,  luabridge::getGlobal(L, "c").unsafe_cast<int>());
    EXPECT_EQ(-1, luabridge::getGlobal(L, "d").unsafe_cast<int>());
    EXPECT_EQ(3,  luabridge::getGlobal(L, "finalVal").unsafe_cast<int>());
}

TEST_F(CppCoroutineClassTests, MemberCoroutine_ConstAccessible)
{
    // A const coroutine (const T* first arg) is accessible on non-const objects.
    // Counter default-constructs with value=0; coroutine yields/returns computed offsets.
    luabridge::getGlobalNamespace(L)
        .beginClass<Counter>("Counter")
            .addConstructor<void()>()
            .addCoroutine("peek", [](const Counter* obj) -> luabridge::CppCoroutine<int>
            {
                co_yield obj->value + 10;
                co_return obj->value + 20;
            })
        .endClass();

    ASSERT_TRUE(runLua(
        "local obj = Counter()\n"
        "local f = coroutine.wrap(Counter.peek)\n"
        "first  = f(obj)\n"
        "second = f()\n"
    ));

    EXPECT_EQ(10, luabridge::getGlobal(L, "first").unsafe_cast<int>());
    EXPECT_EQ(20, luabridge::getGlobal(L, "second").unsafe_cast<int>());
}

TEST_F(CppCoroutineClassTests, MemberCoroutine_MultipleYields)
{
    // Counter default-constructs with value=0; each yield/return adds an increasing offset.
    luabridge::getGlobalNamespace(L)
        .beginClass<Counter>("Counter")
            .addConstructor<void()>()
            .addCoroutine("count", [](Counter* obj) -> luabridge::CppCoroutine<int>
            {
                co_yield obj->value + 1;
                co_yield obj->value + 2;
                co_yield obj->value + 3;
                co_return obj->value + 4;
            })
        .endClass();

    ASSERT_TRUE(runLua(
        "local obj = Counter()\n"
        "local f = coroutine.wrap(Counter.count)\n"
        "a = f(obj)\n"
        "b = f()\n"
        "c = f()\n"
        "d = f()\n"
    ));

    EXPECT_EQ(1, luabridge::getGlobal(L, "a").unsafe_cast<int>());
    EXPECT_EQ(2, luabridge::getGlobal(L, "b").unsafe_cast<int>());
    EXPECT_EQ(3, luabridge::getGlobal(L, "c").unsafe_cast<int>());
    EXPECT_EQ(4, luabridge::getGlobal(L, "d").unsafe_cast<int>());
}

TEST_F(CppCoroutineClassTests, MemberCoroutine_VoidReturn)
{
    // Counter starts with value=0; the coroutine sets it to a known sentinel so we can verify
    // the void coroutine body ran and mutated the object correctly.
    luabridge::getGlobalNamespace(L)
        .beginClass<Counter>("Counter")
            .addConstructor<void()>()
            .addProperty("value", &Counter::value)
            .addCoroutine("stamp", [](Counter* obj) -> luabridge::CppCoroutine<void>
            {
                obj->value = 77;
                co_return;
            })
        .endClass();

    ASSERT_TRUE(runLua(
        "local obj = Counter()\n"
        "local f = coroutine.wrap(Counter.stamp)\n"
        "f(obj)\n"
        "result = obj.value\n"
    ));

    EXPECT_EQ(77, luabridge::getGlobal(L, "result").unsafe_cast<int>());
}

TEST_F(CppCoroutineClassTests, MemberCoroutine_AbandonedNoLeak)
{
    int destructed = 0;
    struct Guard { int* p; ~Guard() { ++(*p); } };

    luabridge::getGlobalNamespace(L)
        .beginClass<Counter>("Counter")
            .addConstructor<void()>()
            .addCoroutine("guarded", [&destructed](Counter* /*obj*/) -> luabridge::CppCoroutine<int>
            {
                Guard g{ &destructed };
                co_yield 1;
                co_yield 2;
                co_return 3;
            })
        .endClass();

    ASSERT_TRUE(runLua(
        "local obj = Counter()\n"
        "local f = coroutine.wrap(Counter.guarded)\n"
        "first = f(obj)\n"
    ));

    EXPECT_EQ(1, luabridge::getGlobal(L, "first").unsafe_cast<int>());

    lua_gc(L, LUA_GCCOLLECT, 0);
    lua_gc(L, LUA_GCCOLLECT, 0);

    EXPECT_EQ(1, destructed);
}

TEST_F(CppCoroutineClassTests, StaticCoroutine_ConcurrentInstances)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Counter>("Counter")
            .addConstructor<void()>()
            .addStaticCoroutine("ticker", []() -> luabridge::CppCoroutine<int>
            {
                co_yield 1;
                co_yield 2;
                co_return 3;
            })
        .endClass();

    ASSERT_TRUE(runLua(
        "local a = coroutine.wrap(Counter.ticker)\n"
        "local b = coroutine.wrap(Counter.ticker)\n"
        "a1 = a()\n"
        "b1 = b()\n"
        "a2 = a()\n"
        "b2 = b()\n"
        "a3 = a()\n"
        "b3 = b()\n"
    ));

    EXPECT_EQ(1, luabridge::getGlobal(L, "a1").unsafe_cast<int>());
    EXPECT_EQ(1, luabridge::getGlobal(L, "b1").unsafe_cast<int>());
    EXPECT_EQ(2, luabridge::getGlobal(L, "a2").unsafe_cast<int>());
    EXPECT_EQ(2, luabridge::getGlobal(L, "b2").unsafe_cast<int>());
    EXPECT_EQ(3, luabridge::getGlobal(L, "a3").unsafe_cast<int>());
    EXPECT_EQ(3, luabridge::getGlobal(L, "b3").unsafe_cast<int>());
}

// Error / non-happy-path tests

#if LUABRIDGE_HAS_EXCEPTIONS
TEST_F(CppCoroutineClassTests, StaticCoroutine_ExceptionOnFirstResume)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Counter>("Counter")
            .addConstructor<void()>()
            .addStaticCoroutine("boom", []() -> luabridge::CppCoroutine<int>
            {
                throw std::runtime_error("static coroutine error");
                co_return 0;
            })
        .endClass();

    ASSERT_TRUE(runLua(
        "local ok, err = pcall(coroutine.wrap(Counter.boom))\n"
        "success = ok\n"
        "errmsg  = err\n"
    ));

    EXPECT_FALSE(luabridge::getGlobal(L, "success").unsafe_cast<bool>());
    auto msg = luabridge::getGlobal(L, "errmsg").unsafe_cast<std::string>();
    EXPECT_NE(std::string::npos, msg.find("static coroutine error"));
}

TEST_F(CppCoroutineClassTests, StaticCoroutine_ExceptionOnContinuation)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Counter>("Counter")
            .addConstructor<void()>()
            .addStaticCoroutine("boom2", []() -> luabridge::CppCoroutine<int>
            {
                co_yield 1;
                throw std::runtime_error("static continuation error");
                co_return 0;
            })
        .endClass();

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(Counter.boom2)\n"
        "first = f()\n"
        "local ok, err = pcall(f)\n"
        "success = ok\n"
        "errmsg  = err\n"
    ));

    EXPECT_EQ(1, luabridge::getGlobal(L, "first").unsafe_cast<int>());
    EXPECT_FALSE(luabridge::getGlobal(L, "success").unsafe_cast<bool>());
    auto msg = luabridge::getGlobal(L, "errmsg").unsafe_cast<std::string>();
    EXPECT_NE(std::string::npos, msg.find("static continuation error"));
}

TEST_F(CppCoroutineClassTests, MemberCoroutine_ExceptionOnFirstResume)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Counter>("Counter")
            .addConstructor<void()>()
            .addCoroutine("boom", [](Counter* /*obj*/) -> luabridge::CppCoroutine<int>
            {
                throw std::runtime_error("member coroutine error");
                co_return 0;
            })
        .endClass();

    ASSERT_TRUE(runLua(
        "local obj = Counter()\n"
        "local ok, err = pcall(coroutine.wrap(Counter.boom), obj)\n"
        "success = ok\n"
        "errmsg  = err\n"
    ));

    EXPECT_FALSE(luabridge::getGlobal(L, "success").unsafe_cast<bool>());
    auto msg = luabridge::getGlobal(L, "errmsg").unsafe_cast<std::string>();
    EXPECT_NE(std::string::npos, msg.find("member coroutine error"));
}

TEST_F(CppCoroutineClassTests, MemberCoroutine_ExceptionOnContinuation)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Counter>("Counter")
            .addConstructor<void()>()
            .addCoroutine("boom2", [](Counter* /*obj*/) -> luabridge::CppCoroutine<int>
            {
                co_yield 1;
                throw std::runtime_error("member continuation error");
                co_return 0;
            })
        .endClass();

    ASSERT_TRUE(runLua(
        "local obj = Counter()\n"
        "local f = coroutine.wrap(Counter.boom2)\n"
        "first = f(obj)\n"
        "local ok, err = pcall(f)\n"
        "success = ok\n"
        "errmsg  = err\n"
    ));

    EXPECT_EQ(1, luabridge::getGlobal(L, "first").unsafe_cast<int>());
    EXPECT_FALSE(luabridge::getGlobal(L, "success").unsafe_cast<bool>());
    auto msg = luabridge::getGlobal(L, "errmsg").unsafe_cast<std::string>();
    EXPECT_NE(std::string::npos, msg.find("member continuation error"));
}
#endif // LUABRIDGE_HAS_EXCEPTIONS

TEST_F(CppCoroutineClassTests, StaticCoroutine_ResumeAfterDone)
{
    // Resuming a finished coroutine should produce an error
    luabridge::getGlobalNamespace(L)
        .beginClass<Counter>("Counter")
            .addConstructor<void()>()
            .addStaticCoroutine("once", []() -> luabridge::CppCoroutine<int>
            {
                co_return 42;
            })
        .endClass();

    ASSERT_TRUE(runLua(
        "local f = coroutine.wrap(Counter.once)\n"
        "result = f()\n"    // finishes immediately
        "local ok, err = pcall(f)\n"  // second resume on dead coroutine
        "resumeOk = ok\n"
    ));

    EXPECT_EQ(42, luabridge::getGlobal(L, "result").unsafe_cast<int>());
    EXPECT_FALSE(luabridge::getGlobal(L, "resumeOk").unsafe_cast<bool>());
}

TEST_F(CppCoroutineClassTests, MemberCoroutine_ResumeAfterDone)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Counter>("Counter")
            .addConstructor<void()>()
            .addCoroutine("once", [](Counter* /*obj*/) -> luabridge::CppCoroutine<int>
            {
                co_return 99;
            })
        .endClass();

    ASSERT_TRUE(runLua(
        "local obj = Counter()\n"
        "local f = coroutine.wrap(Counter.once)\n"
        "result = f(obj)\n"   // finishes immediately
        "local ok, err = pcall(f)\n"  // second resume on dead coroutine
        "resumeOk = ok\n"
    ));

    EXPECT_EQ(99, luabridge::getGlobal(L, "result").unsafe_cast<int>());
    EXPECT_FALSE(luabridge::getGlobal(L, "resumeOk").unsafe_cast<bool>());
}

TEST_F(CppCoroutineClassTests, MemberCoroutine_WrongArgumentType)
{
    // Passing a non-object (integer) where Counter* is expected.
    // With exceptions: the type-mismatch C++ exception escapes the Lua coroutine boundary
    // before lua_error can intercept it, so it surfaces as a C++ throw through runLua.
    // Without exceptions: luaL_error is called inside the coroutine, which is caught by pcall.
    luabridge::getGlobalNamespace(L)
        .beginClass<Counter>("Counter")
            .addConstructor<void()>()
            .addCoroutine("generate", [](Counter* /*obj*/) -> luabridge::CppCoroutine<int>
            {
                co_return 1;
            })
        .endClass();

#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_THROW(
        runLua("coroutine.wrap(Counter.generate)(42)\n"),
        std::exception
    );
#else
    ASSERT_TRUE(runLua(
        "local ok, err = pcall(coroutine.wrap(Counter.generate), 42)\n"
        "success = ok\n"
    ));
    EXPECT_FALSE(luabridge::getGlobal(L, "success").unsafe_cast<bool>());
#endif
}

#endif // LUABRIDGE_HAS_CXX20_COROUTINES
