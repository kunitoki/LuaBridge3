// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// Copyright 2019, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include <sstream>

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

    runLua("result = 3.14");
    EXPECT_TRUE(result().isNumber());
    ASSERT_FLOAT_EQ(3.14f, result<float>());
    ASSERT_DOUBLE_EQ(3.14, result<double>());

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
    auto fnResult = result()(41); // Replaces result variable
    EXPECT_TRUE(fnResult);
    EXPECT_TRUE(fnResult.size());
    EXPECT_TRUE(fnResult[0].isNumber());
    ASSERT_EQ(41, fnResult[0].unsafe_cast<int>());
    ASSERT_EQ(41, *fnResult[0].cast<int>());
    ASSERT_EQ(41, luabridge::unsafe_cast<int>(fnResult[0]));
    ASSERT_EQ(41, *luabridge::cast<int>(fnResult[0]));
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
    auto fnResult = result()["fn"](41); // Replaces result variable
    EXPECT_TRUE(fnResult);
    EXPECT_TRUE(fnResult.size());
    EXPECT_TRUE(fnResult[0].isNumber());
    ASSERT_EQ(41, fnResult[0].unsafe_cast<int>());
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

#if LUABRIDGEDEMO_LUA_VERSION >= 503 && !LUABRIDGE_ON_LUAU && !LUABRIDGE_ON_LUAJIT
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
    obj();
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

    auto result = f.callWithHandler(handler, "badly");
    EXPECT_FALSE(result);
    EXPECT_TRUE(calledHandler);
    EXPECT_TRUE(errorMessage.find("we failed badly") != std::string::npos);
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

    runLua("result = 3.14");
    EXPECT_FALSE(result().isInstance<Base>());
    EXPECT_FALSE(result().isInstance<Derived>());
    EXPECT_FALSE(result().isInstance<Other>());
    EXPECT_FALSE(result().isInstance<Unknown>());
    EXPECT_FALSE(result().isUserdata());
    EXPECT_TRUE(result().isNumber());
    EXPECT_FALSE(result().isNil());
    EXPECT_FALSE(result().getClassName());
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
        func.second(0, "x");
    }
}
