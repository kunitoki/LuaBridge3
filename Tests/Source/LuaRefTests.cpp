// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, kunitoki
// Copyright 2019, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include <sstream>

namespace {
int addInts(int a, int b) { return a + b; }
} // namespace

struct LuaRefTests : TestBase
{
};

TEST_F(LuaRefTests, TypeCheck)
{
    {
        luabridge::LuaRef none(L);
        EXPECT_FALSE(none.isValid());
        EXPECT_TRUE(none.isNil());
        std::stringstream ss;
        none.print(ss);
        EXPECT_EQ("nil", ss.str());
        EXPECT_NE(0u, none.hash());
    }

    {
        runLua("result = nil");
        EXPECT_EQ(LUA_TNIL, result().type());
        EXPECT_TRUE(result().isValid());
        EXPECT_TRUE(result().isNil());
        std::stringstream ss;
        result().print(ss);
        EXPECT_EQ("nil", ss.str());
        EXPECT_NE(0u, result().hash());
    }

    {
        runLua("result = true");
        EXPECT_EQ(LUA_TBOOLEAN, result().type());
        EXPECT_TRUE(result().isValid());
        EXPECT_TRUE(result().isBool());
        std::stringstream ss;
        result().print(ss);
        EXPECT_EQ("true", ss.str());
        EXPECT_NE(0u, result().hash());
    }

    {
        runLua("result = 11");
        EXPECT_EQ(LUA_TNUMBER, result().type());
        EXPECT_TRUE(result().isValid());
        EXPECT_TRUE(result().isNumber());
        std::stringstream ss;
        result().print(ss);
        EXPECT_EQ("11", ss.str());
        EXPECT_NE(0u, result().hash());
    }

#if LUA_VERSION_NUM != 502 // Lua 5.2 hashnum has signed integer overflow UB with float literals
    {
        runLua("result = 3.1");
        EXPECT_EQ(LUA_TNUMBER, result().type());
        EXPECT_TRUE(result().isValid());
        EXPECT_TRUE(result().isNumber());
        std::stringstream ss;
        result().print(ss);
        EXPECT_EQ("3.1", ss.str());
        EXPECT_NE(0u, result().hash());
    }
#endif

    {
        runLua("result = 'abcd'");
        EXPECT_EQ(LUA_TSTRING, result().type());
        EXPECT_TRUE(result().isValid());
        EXPECT_TRUE(result().isString());
        std::stringstream ss;
        result().print(ss);
        EXPECT_EQ("\"abcd\"", ss.str());
        EXPECT_NE(0u, result().hash());
    }

    {
        runLua("result = {}");
        EXPECT_EQ(LUA_TTABLE, result().type());
        EXPECT_TRUE(result().isValid());
        EXPECT_TRUE(result().isTable());
        std::stringstream ss;
        result().print(ss);
        EXPECT_NE(std::string::npos, ss.str().find("table:"));
        EXPECT_NE(0u, result().hash());
    }

    {
        runLua("result = function () end");
        EXPECT_EQ(LUA_TFUNCTION, result().type());
        EXPECT_TRUE(result().isValid());
        EXPECT_TRUE(result().isFunction());
        EXPECT_TRUE(result().isCallable());
        std::stringstream ss;
        result().print(ss);
        EXPECT_NE(std::string::npos, ss.str().find("function:"));
        EXPECT_NE(0u, result().hash());
    }

    {
        void* x = nullptr;
        lua_pushlightuserdata(L, x);
        auto result = luabridge::LuaRef::fromStack(L);
        EXPECT_EQ(LUA_TLIGHTUSERDATA, result.type());
        EXPECT_TRUE(result.isValid());
        EXPECT_TRUE(result.isLightUserdata());
        std::stringstream ss;
        result.print(ss);
        EXPECT_NE(std::string::npos, ss.str().find("userdata:"));
        EXPECT_NE(0u, result.hash());
        lua_pop(L, 1);
    }

    {
        lua_newuserdata(L, 100);
        auto result = luabridge::LuaRef::fromStack(L);
        EXPECT_EQ(LUA_TUSERDATA, result.type());
        EXPECT_TRUE(result.isValid());
        EXPECT_TRUE(result.isUserdata());
        std::stringstream ss;
        result.print(ss);
        EXPECT_NE(std::string::npos, ss.str().find("userdata:"));
        EXPECT_NE(0u, result.hash());
    }

    {
        auto threadL = lua_newthread(L);
        lua_pushthread(threadL);
        auto result = luabridge::LuaRef::fromStack(L);
        EXPECT_EQ(LUA_TTHREAD, result.type());
        EXPECT_TRUE(result.isValid());
        EXPECT_TRUE(result.isThread());
        std::stringstream ss;
        result.print(ss);
        EXPECT_NE(std::string::npos, ss.str().find("thread:"));
        EXPECT_NE(0u, result.hash());
    }
}

TEST_F(LuaRefTests, ValueAccess)
{
    runLua("result = true");
    EXPECT_TRUE(result().isBool());
    ASSERT_TRUE(result<bool>());

    runLua("result = 7");
    EXPECT_TRUE(result().isNumber());
    ASSERT_EQ(7u, result<unsigned char>());
    ASSERT_EQ(7, result<short>());
    ASSERT_EQ(7u, result<unsigned short>());
    ASSERT_EQ(7, result<int>());
    ASSERT_EQ(7u, result<unsigned int>());
    ASSERT_EQ(7, result<long>());
    ASSERT_EQ(7u, result<unsigned long>());
    ASSERT_EQ(7, result<long long>());
    ASSERT_EQ(7u, result<unsigned long long>());

#if LUA_VERSION_NUM != 502 // Lua 5.2 hashnum has signed integer overflow UB with float literals
    runLua("result = 3.14");
    EXPECT_TRUE(result().isNumber());
    ASSERT_FLOAT_EQ(3.14f, result<float>());
    ASSERT_DOUBLE_EQ(3.14, result<double>());
#endif

    runLua("result = 'D'");
    EXPECT_TRUE(result().isString());
    ASSERT_EQ("D", result());
    ASSERT_EQ('D', result<char>());
    ASSERT_EQ("D", result<std::string>());
    ASSERT_STREQ("D", result<const char*>());

    runLua("result = 'abc'");
    EXPECT_TRUE(result().isString());
    ASSERT_EQ("abc", result<std::string>());
    ASSERT_STREQ("abc", result<char const*>());

    runLua("result = function (i) "
           "  result = i + 1 "
           "  return i "
           "end");
    EXPECT_TRUE(result().isFunction());
    auto fnResult = result().call<int>(41); // Replaces result variable
    ASSERT_TRUE(fnResult);
    ASSERT_EQ(41, *fnResult);
    EXPECT_TRUE(result().isNumber());
    ASSERT_EQ(42, result<int>());
}

TEST_F(LuaRefTests, DictionaryRead)
{
    runLua("result = {"
           "  bool = true,"
           "  int = 5,"
           "  c = 3.14,"
           "  [true] = 'D',"
           "  [8] = 'abc',"
           "  fn = function (i) "
           "    result = i + 1 "
           "    return i "
           "  end"
           "}");

    EXPECT_TRUE(result()["bool"].isBool());
    EXPECT_TRUE(result()["bool"].cast<bool>());

    EXPECT_TRUE(result()["int"].isNumber());
    ASSERT_EQ(5u, result()["int"].unsafe_cast<unsigned char>());
    ASSERT_EQ(5, result()["int"].unsafe_cast<short>());
    ASSERT_EQ(5u, result()["int"].unsafe_cast<unsigned short>());
    ASSERT_EQ(5, result()["int"].unsafe_cast<int>());
    ASSERT_EQ(5u, result()["int"].unsafe_cast<unsigned int>());
    ASSERT_EQ(5, result()["int"].unsafe_cast<long>());
    ASSERT_EQ(5u, result()["int"].unsafe_cast<unsigned long>());
    ASSERT_EQ(5, result()["int"].unsafe_cast<long long>());
    ASSERT_EQ(5u, result()["int"].unsafe_cast<unsigned long long>());

    EXPECT_TRUE(result()['c'].isNumber());
    ASSERT_FLOAT_EQ(3.14f, result()['c'].unsafe_cast<float>());
    ASSERT_DOUBLE_EQ(3.14, result()['c'].unsafe_cast<double>());

    EXPECT_TRUE(result()[true].isString());
    ASSERT_EQ('D', result()[true].unsafe_cast<char>());
    ASSERT_EQ("D", result()[true].unsafe_cast<std::string>());
    ASSERT_STREQ("D", result()[true].unsafe_cast<const char*>());

    EXPECT_TRUE(result()[8].isString());
    ASSERT_EQ("abc", result()[8].unsafe_cast<std::string>());
    ASSERT_STREQ("abc", result()[8].unsafe_cast<char const*>());

    EXPECT_TRUE(result()["fn"].isFunction());
    auto fnResult = result()["fn"].call<int>(41); // Replaces result variable
    ASSERT_TRUE(fnResult);
    ASSERT_EQ(41, *fnResult);
    EXPECT_TRUE(result().isNumber());
    ASSERT_EQ(42, result<int>());
}

TEST_F(LuaRefTests, DictionaryWrite)
{
    runLua("result = {a = 5}");
    EXPECT_TRUE(result()["a"].isNumber());
    ASSERT_EQ(5, result()["a"].unsafe_cast<int>());

    result()["a"] = 7;
    ASSERT_EQ(7, result()["a"].unsafe_cast<int>());

    runLua("result = result.a");
    ASSERT_EQ(7, result<int>());

    runLua("result = {a = {b = 1}}");
    ASSERT_EQ(1, result()["a"]["b"].unsafe_cast<int>());

    result()["a"]["b"] = 2;
    ASSERT_EQ(2, result()["a"]["b"].unsafe_cast<int>());
}

struct Class
{
};

TEST_F(LuaRefTests, Comparison)
{
    runLua("function foo () end "
           "local m = {} "
           "m.__eq = function (l, r) return l.a == r.a end "
           "m.__lt = function (l, r) return l.a < r.a end "
           "m.__le = function (l, r) return l.a <= r.a end "
           "t1 = {a = 1} setmetatable (t1, m) "
           "t2 = {a = 2} setmetatable (t2, m) "
           "t3 = {a = 1} setmetatable (t3, m) "
           "t4 = {a = 2} ");

    luabridge::getGlobalNamespace(L).beginClass<Class>("Class").endClass();

    luabridge::LuaRef invalid(L);
    luabridge::LuaRef nil(L, luabridge::LuaNil());
    luabridge::LuaRef boolFalse(L, false);
    luabridge::LuaRef boolTrue(L, true);
    luabridge::LuaRef minus5(L, -5);
    luabridge::LuaRef numPi(L, 3.14);
    luabridge::LuaRef stringA(L, 'a');
    luabridge::LuaRef stringAB(L, "ab");
    luabridge::LuaRef t1 = luabridge::getGlobal(L, "t1");
    luabridge::LuaRef t2 = luabridge::getGlobal(L, "t2");
    luabridge::LuaRef t3 = luabridge::getGlobal(L, "t3");
    luabridge::LuaRef t4 = luabridge::getGlobal(L, "t4");

    EXPECT_FALSE(invalid.isValid());
    EXPECT_TRUE(nil.isValid());

    EXPECT_TRUE(nil == invalid);
    EXPECT_TRUE(invalid == nil);

    EXPECT_TRUE(nil == nil);

    EXPECT_TRUE(nil < boolFalse);

    EXPECT_TRUE(boolFalse == boolFalse);
    EXPECT_TRUE(boolTrue == boolTrue);

    EXPECT_TRUE(boolTrue < minus5);

    EXPECT_TRUE(minus5 == minus5);
    EXPECT_FALSE(minus5 == numPi);
    EXPECT_TRUE(minus5 < numPi);
    EXPECT_TRUE(minus5 <= numPi);
    EXPECT_FALSE(minus5 > numPi);
    EXPECT_FALSE(minus5 >= numPi);

    EXPECT_TRUE(numPi < stringA);

    EXPECT_TRUE(stringA == stringA);
    EXPECT_FALSE(stringA == stringAB);
    EXPECT_TRUE(stringA < stringAB);
    EXPECT_TRUE(stringA <= stringAB);
    EXPECT_FALSE(stringA > stringAB);
    EXPECT_FALSE(stringA >= stringAB);

    EXPECT_TRUE(stringA < t1);

    EXPECT_TRUE(t1 == t1);
    EXPECT_FALSE(t1 == t2);
    EXPECT_TRUE(t1 == t3);
    EXPECT_FALSE(t1.rawequal(t3));
    EXPECT_FALSE(t1 == t4);
    EXPECT_TRUE(t2 == t2);
    EXPECT_FALSE(t2 == t3);

#if LUA_VERSION_NUM >= 503 && !LUABRIDGE_ON_LUAU && !LUABRIDGE_ON_LUAJIT
    // This has changed in lua 5.3 and is quite a behaviour change
    EXPECT_TRUE(t2 == t4);
#else
    EXPECT_FALSE(t2 == t4);
#endif

    EXPECT_TRUE(t3 == t3);
    EXPECT_FALSE(t3 == t4);

    EXPECT_FALSE(t1 < t1);
    EXPECT_TRUE(t1 < t2);
    EXPECT_FALSE(t1 < t3);
    EXPECT_FALSE(t2 < t3);

    EXPECT_TRUE(t1 <= t1);
    EXPECT_TRUE(t1 <= t2);
    EXPECT_TRUE(t1 <= t3);
    EXPECT_FALSE(t2 <= t3);

    EXPECT_FALSE(t1 > t1);
    EXPECT_FALSE(t1 > t2);
    EXPECT_FALSE(t1 > t3);
    EXPECT_TRUE(t2 > t3);

    EXPECT_TRUE(t1 >= t1);
    EXPECT_FALSE(t1 >= t2);
    EXPECT_TRUE(t1 >= t3);
    EXPECT_TRUE(t2 >= t3);
}

TEST_F(LuaRefTests, ComparisonNumbers)
{
    runLua("result = 7");
    EXPECT_TRUE(result().isNumber());

    EXPECT_EQ(7, result());
    EXPECT_EQ(result(), 7);
    EXPECT_NE(8, result());
    EXPECT_NE(result(), 8);

    EXPECT_GT(8, result());
    EXPECT_LT(result(), 8);

    EXPECT_GE(8, result());
    EXPECT_GE(7, result());
    EXPECT_LE(result(), 8);
    EXPECT_LE(result(), 7);
}

TEST_F(LuaRefTests, ComparisonLuaRef)
{
    runLua("result = 7");
    auto seven = result();

    runLua("result = 8");
    auto eight = result();

    EXPECT_EQ(seven, seven);
    EXPECT_NE(seven, eight);
    EXPECT_NE(eight, seven);

    EXPECT_GT(eight, seven);
    EXPECT_LT(seven, eight);

    EXPECT_GE(eight, seven);
    EXPECT_LE(seven, eight);
}

TEST_F(LuaRefTests, ComparisonLuaRefInvalidTypes)
{
    runLua("result = 7");
    auto seven = result();

    runLua("result = '8'");
    auto eight = result();

    EXPECT_NE(seven, eight);
    EXPECT_NE(eight, seven);

    EXPECT_GT(eight, seven);
    EXPECT_LT(seven, eight);

    EXPECT_GE(eight, seven);
    EXPECT_LE(seven, eight);
}

TEST_F(LuaRefTests, Assignment)
{
    runLua("value = {a = 5}");
    auto value = luabridge::getGlobal(L, "value");
    EXPECT_TRUE(value.isTable());
    EXPECT_TRUE(value["a"].isNumber());
    ASSERT_EQ(5, value["a"].unsafe_cast<int>());

    value = value["a"];
    EXPECT_TRUE(value.isNumber());
    ASSERT_EQ(5, value.unsafe_cast<int>());

#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif

    value = value;

#if __clang__
#pragma clang diagnostic pop
#endif

    ASSERT_EQ(LUA_TNUMBER, value.type());
    EXPECT_TRUE(value.isNumber());
    ASSERT_EQ(5, value.unsafe_cast<int>());

    runLua("t = {a = {b = 5}}");
    auto table = luabridge::getGlobal(L, "t");
    luabridge::LuaRef entry = table["a"];
    luabridge::LuaRef b1 = entry["b"];
    luabridge::LuaRef b2 = table["a"]["b"];
    EXPECT_TRUE(b1 == b2);

    runLua("c1 = 1");
    auto c1 = luabridge::getGlobal(L, "c1");
    ASSERT_EQ(1, c1.unsafe_cast<int>());
    c1 = 11;
    ASSERT_EQ(11, c1.unsafe_cast<int>());
}

TEST_F(LuaRefTests, MoveConstructAndAssign)
{
    runLua("value = {a = 5}");
    auto value = luabridge::getGlobal(L, "value");
    EXPECT_TRUE(value.isTable());
    EXPECT_TRUE(value["a"].isNumber());
    EXPECT_TRUE(value.rawget("a").isNumber());
    ASSERT_EQ(5, value["a"].unsafe_cast<int>());

    luabridge::LuaRef moveConstructed = std::move(value);
    EXPECT_TRUE(moveConstructed.isTable());
    EXPECT_TRUE(moveConstructed["a"].isNumber());
    ASSERT_EQ(5, moveConstructed["a"].unsafe_cast<int>());
    EXPECT_FALSE(value.isValid());

    luabridge::LuaRef moveAssigned(L);
    moveAssigned = std::move(moveConstructed);
    EXPECT_TRUE(moveAssigned.isTable());
    EXPECT_TRUE(moveAssigned["a"].isNumber());
    ASSERT_EQ(5, moveAssigned["a"].unsafe_cast<int>());
    EXPECT_FALSE(moveConstructed.isValid());
}

TEST_F(LuaRefTests, Callable)
{
    runLua("function f () end");
    auto f = luabridge::getGlobal(L, "f");
    EXPECT_TRUE(f.isCallable());

    runLua("x = 1");
    auto x = luabridge::getGlobal(L, "x");
    EXPECT_FALSE(x.isCallable());

    runLua("meta1 = { __call = function(self) return 5 end }");
    auto meta1 = luabridge::getGlobal(L, "meta1");
    EXPECT_FALSE(meta1.isCallable());

    runLua("meta2 = { __call = function(self) self.i = self.i + 100 end }; obj = { i = 100 }; setmetatable(obj, meta2)");
    auto obj = luabridge::getGlobal(L, "obj");
    EXPECT_TRUE(obj.isCallable());
    EXPECT_EQ(100, obj["i"].unsafe_cast<int>());
    EXPECT_TRUE(obj.call());
    EXPECT_EQ(200, obj["i"].unsafe_cast<int>());
}

TEST_F(LuaRefTests, CallableWithHandler)
{
    runLua("function f(x) error('we failed ' .. x) end");
    auto f = luabridge::getGlobal(L, "f");
    EXPECT_TRUE(f.isCallable());

#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_ANY_THROW(f.call("badly"));
#else
    EXPECT_FALSE(f.call("badly"));
#endif

    bool calledHandler = false;
    std::string errorMessage;
    auto handler = [&](lua_State*) -> int
    {
        calledHandler = true;

        if (auto msg = lua_tostring(L, 1))
            errorMessage = msg;

        return 0;
    };

    auto result = f.callWithHandler<std::tuple<>>(handler, "badly");
    EXPECT_FALSE(result);
    EXPECT_TRUE(calledHandler);
    EXPECT_TRUE(errorMessage.find("we failed badly") != std::string::npos);
}

TEST_F(LuaRefTests, CallableWithHandlerAsIntToBoolValuedFunction)
{
    runLua("function f(x) return x <= 1 end");
    auto f = luabridge::getGlobal(L, "f");
    EXPECT_TRUE(f.isCallable());
    
    bool calledHandler = false;
    std::string errorMessage;
    auto handler = [&](lua_State*) -> int
    {
        calledHandler = true;

        if (auto msg = lua_tostring(L, 1))
            errorMessage = msg;

        return 0;
    };

    auto result = f.callWithHandler<bool>(handler, 2);
    ASSERT_TRUE(result);
    EXPECT_FALSE(calledHandler);
    EXPECT_FALSE(result.value());
}

TEST_F(LuaRefTests, CallableWithStdFunction)
{
    runLua("function f(x) error('we failed ' .. x) end");
    auto f = luabridge::getGlobal(L, "f");
    EXPECT_TRUE(f.isCallable());

    bool calledHandler = false;
    std::string errorMessage;
    auto handler = [&](lua_State*) -> int
    {
        calledHandler = true;

        if (auto msg = lua_tostring(L, 1))
            errorMessage = msg;

        return 0;
    };

    std::function<int(lua_State*)> pHandler = handler;

    EXPECT_FALSE(f.callWithHandler(pHandler, "badly"));
    EXPECT_TRUE(calledHandler);
    EXPECT_TRUE(errorMessage.find("we failed badly") != std::string::npos);
}

TEST_F(LuaRefTests, CallableWithNullifiedStdFunction)
{
    runLua("function f(x) error('we failed ' .. x) end");
    auto f = luabridge::getGlobal(L, "f");
    EXPECT_TRUE(f.isCallable());

    std::function<int(lua_State*)> pHandler = nullptr;
    EXPECT_FALSE(f.callWithHandler(pHandler, "badly"));

#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_ANY_THROW(f.callWithHandler(pHandler, "badly").throw_on_error());
#endif
}

TEST_F(LuaRefTests, CallableWithCFunction)
{
    runLua("function f(x) error('we failed ' .. x) end");
    auto f = luabridge::getGlobal(L, "f");
    EXPECT_TRUE(f.isCallable());

    lua_CFunction pHandler = +[](lua_State* L) { return 0; };
    EXPECT_FALSE(f.callWithHandler(pHandler, "badly"));
}

TEST_F(LuaRefTests, CallableWithNullCFunction)
{
    runLua("function f(x) error('we failed ' .. x) end");
    auto f = luabridge::getGlobal(L, "f");
    EXPECT_TRUE(f.isCallable());

    lua_CFunction pHandler = nullptr;
    EXPECT_FALSE(f.callWithHandler(pHandler, "badly"));

#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_ANY_THROW(f.callWithHandler(pHandler, "badly").throw_on_error());
#endif
}

#if LUABRIDGE_HAS_EXCEPTIONS
TEST_F(LuaRefTests, CallableWithThrowingHandler)
{
    runLua("function f(x) error('we failed ' .. x) end");
    auto f = luabridge::getGlobal(L, "f");
    EXPECT_TRUE(f.isCallable());

    bool calledHandler = false;
    auto handler = [&](lua_State*) -> int
    {
        calledHandler = true;
        return 0;
    };

    EXPECT_ANY_THROW(f.callWithHandler(handler, "badly").throw_on_error());
    EXPECT_TRUE(calledHandler);
}
#endif

TEST_F(LuaRefTests, CallableWrapper)
{
    runLua("function sum(a, b) return a + b end");
    auto sumRef = luabridge::getGlobal(L, "sum");

    auto sumFn = sumRef.callable<int(int, int)>();
    EXPECT_TRUE(sumFn.isValid());

    {
        auto result = sumFn(20, 22);
        ASSERT_TRUE(result);
        EXPECT_EQ(42, *result);
    }

    {
        auto result = sumFn.call(1, 2);
        ASSERT_TRUE(result);
        EXPECT_EQ(3, *result);
    }

    runLua("function sumFail(a, b) error('sum failed') end");
    auto sumFailRef = luabridge::getGlobal(L, "sumFail");
    auto sumFailFn = sumFailRef.callable<int(int, int)>();

    bool calledHandler = false;
    auto handler = [&calledHandler](lua_State*) -> int
    {
        calledHandler = true;
        return 0;
    };

    auto failed = sumFailFn.callWithHandler(handler, 1, 2);
    EXPECT_FALSE(failed);
    EXPECT_TRUE(calledHandler);
    EXPECT_EQ(luabridge::makeErrorCode(luabridge::ErrorCode::LuaFunctionCallFailed), failed.error());

    runLua("notCallable = 10");
    auto notCallableRef = luabridge::getGlobal(L, "notCallable");
    auto invalidFn = notCallableRef.callable<int(int)>();
    EXPECT_FALSE(invalidFn.isValid());
}

TEST_F(LuaRefTests, CallableWrapperVoidAndTypedMismatch)
{
    runLua("pingCalls = 0 "
           "function ping() pingCalls = pingCalls + 1 end "
           "function returnText() return 'abc' end");

    auto pingRef = luabridge::getGlobal(L, "ping");
    auto pingFn = pingRef.callable<void()>();
    ASSERT_TRUE(pingFn.isValid());

    auto pingResult = pingFn();
    ASSERT_TRUE(pingResult);
    EXPECT_EQ(1, luabridge::getGlobal(L, "pingCalls").unsafe_cast<int>());

    auto returnTextRef = luabridge::getGlobal(L, "returnText");
    auto typedFn = returnTextRef.callable<int()>();
    ASSERT_TRUE(typedFn.isValid());

    auto mismatch = typedFn.call();
    EXPECT_FALSE(mismatch);
    EXPECT_EQ(luabridge::makeErrorCode(luabridge::ErrorCode::InvalidTypeCast), mismatch.error());
}

TEST_F(LuaRefTests, Pop)
{
    lua_pushstring(L, "hello");
    luabridge::LuaRef ref1 = luabridge::LuaRef::fromStack(L);

    lua_pushstring(L, "world!");
    luabridge::LuaRef ref2 = luabridge::LuaRef::fromStack(L);

    ref1.push();
    ref2.pop();

    EXPECT_EQ(ref1.unsafe_cast<std::string>(), ref2.unsafe_cast<std::string>());
    EXPECT_EQ("hello", ref1.tostring());
    EXPECT_EQ("hello", ref2.tostring());
}

TEST_F(LuaRefTests, IsInstance)
{
    struct Base {};
    struct Derived : Base {};
    struct Other {};
    struct Unknown : Base {};

    luabridge::getGlobalNamespace(L)
        .beginClass<Base>("Base")
        .addConstructor<void (*)()>()
        .endClass()
        .deriveClass<Derived, Base>("Derived")
        .addConstructor<void (*)()>()
        .endClass()
        .beginClass<Other>("Other")
        .addConstructor<void (*)()>()
        .endClass();

    runLua("result = Base ()");
    EXPECT_TRUE(result().isInstance<Base>());
    EXPECT_FALSE(result().isInstance<Derived>());
    EXPECT_FALSE(result().isInstance<Other>());
    EXPECT_FALSE(result().isInstance<Unknown>());
    EXPECT_TRUE(result().isUserdata());
    EXPECT_FALSE(result().isNil());
    EXPECT_EQ("Base", result().getClassName().value_or(""));

    runLua("result = Derived ()");
    EXPECT_TRUE(result().isInstance<Base>());
    EXPECT_TRUE(result().isInstance<Derived>());
    EXPECT_FALSE(result().isInstance<Other>());
    EXPECT_FALSE(result().isInstance<Unknown>());
    EXPECT_TRUE(result().isUserdata());
    EXPECT_FALSE(result().isNil());
    EXPECT_EQ("Derived", result().getClassName().value_or(""));

    runLua("result = Other ()");
    EXPECT_FALSE(result().isInstance<Base>());
    EXPECT_FALSE(result().isInstance<Derived>());
    EXPECT_TRUE(result().isInstance<Other>());
    EXPECT_FALSE(result().isInstance<Unknown>());
    EXPECT_TRUE(result().isUserdata());
    EXPECT_FALSE(result().isNil());
    EXPECT_EQ("Other", result().getClassName().value_or(""));

    lua_newuserdata(L, sizeof(Unknown));
    EXPECT_TRUE(lua_isuserdata(L, -1));
    luaL_newmetatable(L, "Unknown");
    EXPECT_TRUE(lua_istable(L, -1));
    lua_setmetatable(L, -2);
    EXPECT_TRUE(lua_isuserdata(L, -1));
    lua_setglobal(L, "unkown");
    runLua("result = unknown");
    EXPECT_FALSE(result().isInstance<Base>());
    EXPECT_FALSE(result().isInstance<Derived>());
    EXPECT_FALSE(result().isInstance<Other>());
    EXPECT_FALSE(result().isInstance<Unknown>());
    //EXPECT_TRUE(result().isUserdata()); // TODO
    EXPECT_FALSE(result().isTable());
    //EXPECT_FALSE(result().isNil()); // TODO
    EXPECT_FALSE(result().getClassName());

    runLua("result = {}");
    EXPECT_FALSE(result().isInstance<Base>());
    EXPECT_FALSE(result().isInstance<Derived>());
    EXPECT_FALSE(result().isInstance<Other>());
    EXPECT_FALSE(result().isInstance<Unknown>());
    EXPECT_FALSE(result().isUserdata());
    EXPECT_TRUE(result().isTable());
    EXPECT_FALSE(result().isNil());
    EXPECT_FALSE(result().getClassName());

#if LUA_VERSION_NUM != 502 // Lua 5.2 hashnum has signed integer overflow UB with float literals
    runLua("result = 3.14");
    EXPECT_FALSE(result().isInstance<Base>());
    EXPECT_FALSE(result().isInstance<Derived>());
    EXPECT_FALSE(result().isInstance<Other>());
    EXPECT_FALSE(result().isInstance<Unknown>());
    EXPECT_FALSE(result().isUserdata());
    EXPECT_TRUE(result().isNumber());
    EXPECT_FALSE(result().isNil());
    EXPECT_FALSE(result().getClassName());
#endif
}

TEST_F(LuaRefTests, Print)
{
    {
        runLua("result = true");
        std::ostringstream stream;
        stream << result();
        ASSERT_EQ("true", stream.str());
    }
    {
        runLua("result = false");
        std::ostringstream stream;
        stream << result();
        ASSERT_EQ("false", stream.str());
    }
    {
        runLua("result = 5");
        std::ostringstream stream;
        stream << result();
        ASSERT_EQ("5", stream.str());
    }
    {
        runLua("result = 'abc'");
        std::ostringstream stream;
        stream << result();
        ASSERT_EQ("\"abc\"", stream.str());
    }

    runLua("result = {"
           "  true_ = true,"
           "  false_ = false,"
           "  five = 5,"
           "  abc = 'abc'"
           "}");
    {
        std::ostringstream stream;
        stream << result()["true_"];
        ASSERT_EQ("true", stream.str());
    }
    {
        std::ostringstream stream;
        stream << result()["false_"];
        ASSERT_EQ("false", stream.str());
    }
    {
        std::ostringstream stream;
        stream << result()["five"];
        ASSERT_EQ("5", stream.str());
    }
    {
        std::ostringstream stream;
        stream << result()["abc"];
        ASSERT_EQ("\"abc\"", stream.str());
    }
}

TEST_F(LuaRefTests, RegisterLambdaInTable)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("Entities")
            .addFunction("GetLocalHero", [&]() {
                auto table = luabridge::newTable(L);
                table.push(L);

                luabridge::getNamespaceFromStack(L)
                    .addProperty("index", [] { return 150; })
                    .addFunction("Health", [&] { return 500; });

                table.pop();
                return table;
            })
        .endNamespace();

    runLua("result = Entities.GetLocalHero().Health()");
    ASSERT_EQ(500, result<int>());
}

TEST_F(LuaRefTests, RegisterLambdaInFunction)
{
    struct Class
    {
        Class() = default;

        int test() const
        {
            return 1;
        }
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<Class>("Class")
            .addConstructor<void (*)()>()
            .addFunction("test", &Class::test)
        .endClass();

    luabridge::setGlobal(L, luabridge::newFunction(L, [](const Class* obj, int x) { return obj->test() + x; }), "takeClass");
    luabridge::setGlobal(L, luabridge::newFunction(L, [](Class* obj, int x, int y, lua_State* L) { return obj->test() + x + y + lua_gettop(L); }), "takeClassState");

    runLua("obj = Class(); result = takeClass (obj, 10)");
    ASSERT_EQ(1 + 10, result<int>());

    runLua("obj = Class(); result = takeClassState (obj, 10, 100)");
    ASSERT_EQ(1 + 10 + 100 + 3, result<int>());
}

TEST_F(LuaRefTests, RegisterBindFrontInNewFunction)
{
    luabridge::setGlobal(L, luabridge::newFunction(L, luabridge::bind_front(&addInts, 10)), "addTen");

    runLua("result = addTen (32)");
    ASSERT_EQ(42, result<int>());
}

TEST_F(LuaRefTests, RegisterBindFrontLambdaInNewFunction)
{
    luabridge::setGlobal(L, luabridge::newFunction(L, luabridge::bind_front([](int a, int b) { return a + b; }, 10)), "addTen");

    runLua("result = addTen (32)");
    ASSERT_EQ(42, result<int>());
}

TEST_F(LuaRefTests, RegisterBindBackInNewFunction)
{
    luabridge::setGlobal(L, luabridge::newFunction(L, luabridge::bind_back(&addInts, 10)), "addTen");

    runLua("result = addTen (32)");
    ASSERT_EQ(42, result<int>());
}

TEST_F(LuaRefTests, RegisterBindBackLambdaInNewFunction)
{
    luabridge::setGlobal(L, luabridge::newFunction(L, luabridge::bind_back([](int a, int b) { return a + b; }, 10)), "addTen");

    runLua("result = addTen (32)");
    ASSERT_EQ(42, result<int>());
}

TEST_F(LuaRefTests, HookTesting)
{
    std::map<std::string, luabridge::LuaRef> hooklist;

    auto hook = [&](const std::string& name, luabridge::LuaRef cb) {
        hooklist.emplace(name, std::move(cb));
    };

    luabridge::getGlobalNamespace(L)
        .addFunction("Hook", hook);

    runLua(R"(
        function hook1(type, packet)
            print("lol")
        end

        Hook("hook1", hook1)
    )");

    for (auto& func : hooklist) {
        EXPECT_TRUE(func.second.call(0, "x"));
    }
}

TEST_F(LuaRefTests, AppendSingleValue)
{
    runLua("result = {}");

    EXPECT_TRUE(result().append(1));
    EXPECT_TRUE(result().append(2));
    EXPECT_TRUE(result().append(3));

    ASSERT_EQ(3, result().length());
    ASSERT_EQ(1, result()[1].unsafe_cast<int>());
    ASSERT_EQ(2, result()[2].unsafe_cast<int>());
    ASSERT_EQ(3, result()[3].unsafe_cast<int>());
}

TEST_F(LuaRefTests, AppendMultipleValues)
{
    runLua("result = {}");

    EXPECT_TRUE(result().append(10, 20, 30));

    ASSERT_EQ(3, result().length());
    ASSERT_EQ(10, result()[1].unsafe_cast<int>());
    ASSERT_EQ(20, result()[2].unsafe_cast<int>());
    ASSERT_EQ(30, result()[3].unsafe_cast<int>());
}

TEST_F(LuaRefTests, AppendMixedTypes)
{
    runLua("result = {}");

    EXPECT_TRUE(result().append(42, std::string("hello"), true, 3.14));

    ASSERT_EQ(4, result().length());
    ASSERT_EQ(42, result()[1].unsafe_cast<int>());
    ASSERT_EQ("hello", result()[2].unsafe_cast<std::string>());
    ASSERT_TRUE(result()[3].unsafe_cast<bool>());
    ASSERT_DOUBLE_EQ(3.14, result()[4].unsafe_cast<double>());
}

TEST_F(LuaRefTests, AppendToExistingSequence)
{
    runLua("result = {10, 20}");

    ASSERT_EQ(2, result().length());

    EXPECT_TRUE(result().append(30, 40));

    ASSERT_EQ(4, result().length());
    ASSERT_EQ(10, result()[1].unsafe_cast<int>());
    ASSERT_EQ(20, result()[2].unsafe_cast<int>());
    ASSERT_EQ(30, result()[3].unsafe_cast<int>());
    ASSERT_EQ(40, result()[4].unsafe_cast<int>());
}

TEST_F(LuaRefTests, FieldHelpersRespectMetamethodsAndRawAccess)
{
    runLua("indexCalls = 0 "
           "newindexCalls = 0 "
           "shadow = {} "
           "result = setmetatable({}, {"
           "  __index = function(_, key) indexCalls = indexCalls + 1; return shadow[key] end,"
           "  __newindex = function(_, key, value) newindexCalls = newindexCalls + 1; shadow[key] = value end"
           "})");

    auto table = result();

    EXPECT_TRUE(table.setField("metaValue", 42));
    EXPECT_EQ(1, luabridge::getGlobal(L, "newindexCalls").unsafe_cast<int>());

    auto viaMeta = table.getField<int>("metaValue");
    ASSERT_TRUE(viaMeta);
    EXPECT_EQ(42, *viaMeta);
    EXPECT_EQ(1, luabridge::getGlobal(L, "indexCalls").unsafe_cast<int>());

    auto rawMissing = table.rawgetField<int>("metaValue");
    EXPECT_FALSE(rawMissing);
    EXPECT_EQ(luabridge::makeErrorCode(luabridge::ErrorCode::InvalidTypeCast), rawMissing.error());

    EXPECT_TRUE(table.rawsetField("rawValue", 99));
    EXPECT_EQ(1, luabridge::getGlobal(L, "newindexCalls").unsafe_cast<int>());

    auto rawValue = table.rawgetField<int>("rawValue");
    ASSERT_TRUE(rawValue);
    EXPECT_EQ(99, *rawValue);
}

TEST_F(LuaRefTests, UnsafeRawFieldHelpersKeepStackBalanced)
{
    runLua("indexCalls = 0 "
           "newindexCalls = 0 "
           "result = setmetatable({}, {"
           "  __index = function() indexCalls = indexCalls + 1; return 777 end,"
           "  __newindex = function(_, _, _) newindexCalls = newindexCalls + 1 end"
           "})");

    auto table = result();
    const int topBeforeSet = lua_gettop(L);

    table.unsafeRawsetField("rawNumber", 55);
    EXPECT_EQ(topBeforeSet, lua_gettop(L));
    EXPECT_EQ(0, luabridge::getGlobal(L, "newindexCalls").unsafe_cast<int>());

    const int topBeforeGet = lua_gettop(L);
    auto rawNumber = table.unsafeRawgetField<int>("rawNumber");
    EXPECT_EQ(55, rawNumber);
    EXPECT_EQ(topBeforeGet, lua_gettop(L));
    EXPECT_EQ(0, luabridge::getGlobal(L, "indexCalls").unsafe_cast<int>());
}

TEST_F(LuaRefTests, RawgetFieldBypassesIndexMetamethod)
{
    runLua("indexCalls = 0 "
           "result = setmetatable({}, {"
           "  __index = function(_, _) indexCalls = indexCalls + 1; return 123 end"
           "})");

    auto table = result();

    auto viaMeta = table.getField<int>("metaOnly");
    ASSERT_TRUE(viaMeta);
    EXPECT_EQ(123, *viaMeta);
    EXPECT_EQ(1, luabridge::getGlobal(L, "indexCalls").unsafe_cast<int>());

    auto rawViaMissing = table.rawgetField<int>("metaOnly");
    EXPECT_FALSE(rawViaMissing);
    EXPECT_EQ(luabridge::makeErrorCode(luabridge::ErrorCode::InvalidTypeCast), rawViaMissing.error());
    EXPECT_EQ(1, luabridge::getGlobal(L, "indexCalls").unsafe_cast<int>());
}

TEST_F(LuaRefTests, UnsafeRawgetFieldBypassesIndexMetamethod)
{
    runLua("indexCalls = 0 "
           "result = setmetatable({ rawValue = 999 }, {"
           "  __index = function(_, _) indexCalls = indexCalls + 1; return 123 end"
           "})");

    auto table = result();

    const int topBefore = lua_gettop(L);
    auto rawValue = table.unsafeRawgetField<int>("rawValue");
    EXPECT_EQ(999, rawValue);
    EXPECT_EQ(topBefore, lua_gettop(L));
    EXPECT_EQ(0, luabridge::getGlobal(L, "indexCalls").unsafe_cast<int>());
}

TEST_F(LuaRefTests, UserdataIndexMetamethodPropgetFastPath)
{
    struct PropsClass
    {
        int value = 7;
    };

    int getterCalls = 0;

    luabridge::getGlobalNamespace(L)
        .beginClass<PropsClass>("PropsClass")
        .addConstructor<void (*)()>()
        .addProperty("tracked", [&getterCalls](const PropsClass* self) {
            ++getterCalls;
            return self->value;
        })
        .endClass();

    runLua("obj = PropsClass(); result = obj");
    auto obj = result();
    ASSERT_TRUE(obj.isUserdata());

    auto tracked = obj["tracked"];
    auto trackedValue = luabridge::LuaRef(tracked);
    ASSERT_TRUE(trackedValue.isNumber());
    EXPECT_EQ(7, luabridge::unsafe_cast<int>(trackedValue));
    EXPECT_EQ(1, getterCalls);

    // Missing key exercises the propget fast-path miss branch (line 556 pop) and falls through to nil.
    auto missing = obj["definitely_missing"];
    auto missingValue = luabridge::LuaRef(missing);
    EXPECT_TRUE(missingValue.isNil());
    EXPECT_EQ(1, getterCalls);
}

TEST_F(LuaRefTests, TableItemNestedRawHelpersAndCopy)
{
    runLua("result = { outer = { value = 10 } }");

    auto outer = result()["outer"];
    auto outerCopy = outer; // Exercise TableItem copy constructor

    outerCopy.rawset(luabridge::newTable(L));
    ASSERT_TRUE(result()["outer"].isTable());

    result()["outer"].rawset(luabridge::newTable(L));
    auto replacedOuter = result()["outer"];

    replacedOuter.unsafeRawsetField("value", 77);
    const int value = replacedOuter.unsafeRawgetField<int>("value");
    EXPECT_EQ(77, value);

    auto rawField = replacedOuter.rawget("value");
    ASSERT_TRUE(rawField.isNumber());
    EXPECT_EQ(77, luabridge::cast<int>(rawField).valueOr(0));
}

TEST_F(LuaRefTests, TableItemOperatorIndexAdoptPathAndRawRoundTrip)
{
    runLua("result = { outer = {} }");

    std::string outerKey = "outer";
    std::string childKey = "child";

    auto outer = result()[outerKey];
    auto child = outer[childKey]; // Exercise TableItem::operator[](const T&) and AdoptTableRef ctor path

    child.rawset(19);

    auto fromRaw = outer.rawget(childKey);
    ASSERT_TRUE(fromRaw.isNumber());
    EXPECT_EQ(19, luabridge::unsafe_cast<int>(fromRaw));

    child.rawset(luabridge::newTable(L));

    const int stackTopBefore = lua_gettop(L);
    auto childAgain = result()[outerKey][childKey];
    childAgain.unsafeRawsetField("nested", 1234);
    EXPECT_EQ(stackTopBefore, lua_gettop(L));

    auto nested = childAgain.unsafeRawgetField<int>("nested");
    EXPECT_EQ(1234, nested);
    EXPECT_EQ(stackTopBefore, lua_gettop(L));
}

TEST_F(LuaRefTests, GetClassNameNoMetatable)
{
    // A raw userdata with no metatable causes lua_getmetatable to return 0,
    // hitting the early-return nullopt at LuaRef.h:311.
    lua_newuserdata(L, 100);
    auto ref = luabridge::LuaRef::fromStack(L); // fromStack pops the userdata

    EXPECT_FALSE(ref.getClassName());
}

TEST_F(LuaRefTests, CallReturningTupleSuccess)
{
    // Exercises the success path of decodeTupleResult (Invoke.h:63).
    runLua("result = function() return 42, 'hello' end");
    auto r = result().call<std::tuple<int, std::string>>();
    ASSERT_TRUE(r);
    EXPECT_EQ(42, std::get<0>(*r));
    EXPECT_EQ("hello", std::get<1>(*r));
}

TEST_F(LuaRefTests, CallReturningTupleWithLuaRef)
{
    runLua("result = function() return 1, {2, 'three'} end");
    auto r = result().call<std::tuple<int, luabridge::LuaRef>>();
    ASSERT_TRUE(r);
    EXPECT_EQ(1, std::get<0>(*r));
    ASSERT_TRUE(std::get<1>(*r).isTable());
    EXPECT_EQ(2, *std::get<1>(*r)[1].cast<int>());
    EXPECT_EQ("three", *std::get<1>(*r)[2].cast<std::string>());
}

TEST_F(LuaRefTests, CallReturningTupleWrongType)
{
    // Exercises the error path of decodeTupleResult (Invoke.h:58-59):
    // the first returned value cannot be converted to int.
    runLua("result = function() return 'not_an_int', 'hello' end");
    auto r = result().call<std::tuple<int, std::string>>();
    EXPECT_FALSE(r);
}

TEST_F(LuaRefTests, ToStringStackOverflow)
{
    // Exercises the early return in LuaRefBase::tostring (LuaRef.h:117)
    // when the Lua stack is exhausted.
    runLua("result = 42");
    auto ref = result(); // capture ref before exhausting the stack
    exhaustStackSpace();
    std::string s = ref.tostring();
    EXPECT_TRUE(s.empty());
    lua_settop(L, 0); // restore stack so ref's destructor can call luaL_unref safely
}

TEST_F(LuaRefTests, GetMetatableOnNil)
{
    // Covers LuaRef.h:391 - early return when the ref is nil
    // Use LuaRef(L) which creates an invalid (nil-like) ref without touching the Lua stack
    luabridge::LuaRef nilRef(L);

    EXPECT_TRUE(nilRef.isNil());
    auto mt = nilRef.getMetatable();
    EXPECT_TRUE(mt.isNil());
}

TEST_F(LuaRefTests, NewTableStackOverflow)
{
    // Covers LuaRef.h:1184-1185 - early return when stack is exhausted
    exhaustStackSpace();
    auto t = luabridge::LuaRef::newTable(L);
    EXPECT_FALSE(t.isValid());
}

TEST_F(LuaRefTests, NewFunctionStackOverflow)
{
    // Covers LuaRef.h:1208-1209 - early return when stack is exhausted
    exhaustStackSpace();
    auto f = luabridge::LuaRef::newFunction(L, [](lua_State*) -> int { return 0; });
    EXPECT_FALSE(f.isValid());
}

TEST_F(LuaRefTests, GetGlobalStackOverflow)
{
    // Covers LuaRef.h:1230-1231 - early return when stack is exhausted
    exhaustStackSpace();
    auto g = luabridge::LuaRef::getGlobal(L, "print");
    EXPECT_FALSE(g.isValid());
}

TEST_F(LuaRefTests, PushStackOverflow)
{
    // Covers LuaRef.h:1338-1339 - early return in push() when stack is exhausted
    runLua("result = 42");
    auto ref = result();
    exhaustStackSpace();
    // push() should silently return without pushing
    const int topBefore = lua_gettop(L);
    ref.push(L);
    EXPECT_EQ(topBefore, lua_gettop(L));
    lua_settop(L, 0); // restore stack so ref's destructor can call luaL_unref safely
}

TEST_F(LuaRefTests, SetFieldPushFailure)
{
    // Covers LuaRef.h:1466-1467 - setField returns false when value push fails
    if constexpr (sizeof(long double) > sizeof(lua_Number))
    {
        runLua("result = {}");
        auto t = result();
        long double huge = std::numeric_limits<long double>::max();
        EXPECT_FALSE(t.setField("key", huge));
    }
}

TEST_F(LuaRefTests, RawSetFieldPushFailure)
{
    // Covers LuaRef.h:1510-1511 - rawsetField returns false when value push fails
    if constexpr (sizeof(long double) > sizeof(lua_Number))
    {
        runLua("result = {}");
        auto t = result();
        long double huge = std::numeric_limits<long double>::max();
        EXPECT_FALSE(t.rawsetField("key", huge));
    }
}

TEST_F(LuaRefTests, RawGetPushFailure)
{
    // Covers LuaRef.h:1419-1420 - rawget returns nil LuaRef when key push fails
    if constexpr (sizeof(long double) > sizeof(lua_Number))
    {
        runLua("result = {}");
        auto t = result();
        long double huge = std::numeric_limits<long double>::max();
        auto r = t.rawget(huge);
        EXPECT_TRUE(r.isNil());
    }
}

TEST_F(LuaRefTests, OperatorIndexPushFailure)
{
    // Covers LuaRef.h:1390-1391 - operator[] returns default TableItem when key push fails
    if constexpr (sizeof(long double) > sizeof(lua_Number))
    {
        runLua("result = {}");
        auto t = result();
        long double huge = std::numeric_limits<long double>::max();
        // The subscript with a failing push returns a default TableItem
        auto item = t[huge];
        (void)item; // just ensure we don't crash
    }
}

TEST_F(LuaRefTests, AppendPushFailure)
{
    // Covers LuaRef.h:611-612 - append returns false when element push fails
    if constexpr (sizeof(long double) > sizeof(lua_Number))
    {
        runLua("result = {}");
        auto t = result();
        long double huge = std::numeric_limits<long double>::max();
        EXPECT_FALSE(t.append(huge));
    }
}

TEST_F(LuaRefTests, ComparisonOperatorPushFailure)
{
    // Covers LuaRef.h:421 (operator==), 458 (operator<), 485 (operator<=),
    // 511 (operator>), 539 (operator>=), 566 (rawequal) - early return false when rhs push fails
#if LUA_VERSION_NUM != 502 // Lua 5.2 hashnum has signed integer overflow UB with float literals
    if constexpr (sizeof(long double) > sizeof(lua_Number))
    {
        runLua("result = 1.0");
        auto ref = result();
        long double huge = std::numeric_limits<long double>::max();

        EXPECT_FALSE(ref == huge);
        EXPECT_FALSE(ref < huge);
        EXPECT_FALSE(ref <= huge);
        EXPECT_FALSE(ref > huge);
        EXPECT_FALSE(ref >= huge);
        EXPECT_FALSE(ref.rawequal(huge));
    }
#endif
}

TEST_F(LuaRefTests, TableItemCopyWithKeyRef)
{
    // Covers LuaRef.h:757-758 - copy constructor path when other.m_keyRef != LUA_NOREF
    runLua("result = { [\"outer\"] = { value = 99 } }");

    std::string key = "outer";
    auto item = result()[key]; // creates TableItem with m_keyRef (not m_keyLiteral)
    auto itemCopy = item;      // exercises lines 757-758

    auto val = itemCopy["value"];
    EXPECT_EQ(99, luabridge::cast<int>(val).valueOr(0));
}

TEST_F(LuaRefTests, TableItemCopyStackOverflow)
{
    // Covers LuaRef.h:748 - early return in TableItem copy constructor when stack is exhausted
    runLua("result = { [\"k\"] = 1 }");
    std::string key = "k";
    auto item = result()[key];
    exhaustStackSpace();
    auto itemCopy = item; // should not crash; m_tableRef stays LUA_NOREF
    lua_settop(L, 0);
}

TEST_F(LuaRefTests, TableItemAssignStackOverflow)
{
    // Covers LuaRef.h:794 - early return in TableItem::operator= when stack is exhausted
    runLua("result = { key = 0 }");
    auto item = result()["key"];
    exhaustStackSpace();
    item = 42; // should silently return *this without modifying the table
    lua_settop(L, 0);
}

TEST_F(LuaRefTests, TableItemRawsetStackOverflow)
{
    // Covers LuaRef.h:837 - early return in TableItem::rawset when stack is exhausted
    runLua("result = { key = 0 }");
    auto item = result()["key"];
    exhaustStackSpace();
    item.rawset(42); // should silently return *this without modifying the table
    lua_settop(L, 0);
}

TEST_F(LuaRefTests, TableItemPushStackOverflow)
{
    // Covers LuaRef.h:880 - early return in TableItem::push when stack is exhausted
    runLua("result = { key = 1 }");
    auto item = result()["key"];
    exhaustStackSpace();
    const int topBefore = lua_gettop(L);
    item.push(L); // should silently return without pushing
    EXPECT_EQ(topBefore, lua_gettop(L));
    lua_settop(L, 0);
}

TEST_F(LuaRefTests, LuaRefFromStackOverflow)
{
    // Covers LuaRef.h:1048 - early return in LuaRef(L, index, FromStack) when stack is exhausted
    lua_pushinteger(L, 7);
    exhaustStackSpace();
    auto ref = luabridge::LuaRef::fromStack(L, -1);
    EXPECT_FALSE(ref.isValid());
    lua_settop(L, 0);
}

TEST_F(LuaRefTests, TableItemAssignKeyRefPushFailure)
{
    // Covers LuaRef.h:812 - return *this in operator= key-ref path when Stack push fails
    if constexpr (sizeof(long double) > sizeof(lua_Number))
    {
        runLua("result = { [\"k\"] = 0 }");
        std::string key = "k";
        auto item = result()[key]; // TableItem with m_keyRef
        long double huge = std::numeric_limits<long double>::max();
        item = huge; // push fails - early return at line 812
    }
}

TEST_F(LuaRefTests, TableItemRawsetLiteralPushFailure)
{
    // Covers LuaRef.h:848 - return *this in rawset literal path when Stack push fails
    if constexpr (sizeof(long double) > sizeof(lua_Number))
    {
        runLua("result = { key = 0 }");
        auto item = result()["key"]; // TableItem with m_keyLiteral
        long double huge = std::numeric_limits<long double>::max();
        item.rawset(huge); // push fails - early return at line 848
    }
}

TEST_F(LuaRefTests, TableItemRawsetKeyRefPushFailure)
{
    // Covers LuaRef.h:857 - return *this in rawset key-ref path when Stack push fails
    if constexpr (sizeof(long double) > sizeof(lua_Number))
    {
        runLua("result = { [\"k\"] = 0 }");
        std::string key = "k";
        auto item = result()[key]; // TableItem with m_keyRef
        long double huge = std::numeric_limits<long double>::max();
        item.rawset(huge); // push fails - early return at line 857
    }
}

TEST_F(LuaRefTests, TableItemOperatorIndexKeyPushFailure)
{
    // Covers LuaRef.h:918 - lua_pushnil fallback in TableItem::operator[] when key push fails
    if constexpr (sizeof(long double) > sizeof(lua_Number))
    {
        runLua("result = { [\"k\"] = {} }");
        std::string key = "k";
        auto item = result()[key]; // TableItem
        long double huge = std::numeric_limits<long double>::max();
        auto child = item[huge]; // key push fails -> lua_pushnil used as key
        (void)child;
    }
}

TEST_F(LuaRefTests, LuaRefConstructorPushFailure)
{
    // Covers LuaRef.h:1085 - early return in LuaRef(L, v) template constructor when push fails
    if constexpr (sizeof(long double) > sizeof(lua_Number))
    {
        long double huge = std::numeric_limits<long double>::max();
        luabridge::LuaRef ref(L, huge); // push fails - early return at line 1085
        EXPECT_FALSE(ref.isValid());
    }
}
