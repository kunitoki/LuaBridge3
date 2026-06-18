// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, kunitoki
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include "LuaBridge/FlatMap.h"

#if LUABRIDGE_HAS_CXX23_FLAT_MAP

#include <flat_map>
#include <string>

namespace {
struct Unregistered
{
    bool operator<(const Unregistered& other) const
    {
        return true;
    }
};

struct Data
{
    /* explicit */ Data(int i) : i(i) {}

    int i;
};

bool operator==(const Data& lhs, const Data& rhs)
{
    return lhs.i == rhs.i;
}

bool operator<(const Data& lhs, const Data& rhs)
{
    return lhs.i < rhs.i;
}

std::ostream& operator<<(std::ostream& lhs, const Data& rhs)
{
    lhs << "{" << rhs.i << "}";
    return lhs;
}

std::flat_map<Data, Data> processValues(const std::flat_map<Data, Data>& data)
{
    return data;
}

std::flat_map<Data, Data> processPointers(const std::flat_map<Data, const Data*>& data)
{
    std::flat_map<Data, Data> result;

    for (const auto& item : data)
        result.emplace(item.first, *item.second);

    return result;
}
} // namespace

namespace std {
template <>
struct hash<Unregistered>
{
    std::size_t operator()(const Unregistered& value) const
    {
        return 0;
    }
};
} // namespace std

struct FlatMapTests : TestBase
{
};

TEST_F(FlatMapTests, GetNonTable)
{
    lua_pushnumber(L, 42.0);

    auto result = luabridge::Stack<std::flat_map<int, int>>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(FlatMapTests, GetWithInvalidValue)
{
    lua_createtable(L, 0, 1);
    lua_pushinteger(L, 1);
    lua_pushstring(L, "not_an_int");
    lua_settable(L, -3);

    auto result = luabridge::Stack<std::flat_map<int, int>>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(FlatMapTests, LuaRef)
{
    {
        using FlatMap = std::flat_map<int, char>;

        const FlatMap expected { {1, 'a'}, {2, 'b'}, {3, 'c'} };

        runLua("result = {'a', 'b', 'c'}");

        FlatMap actual = result();
        EXPECT_EQ(expected, actual);
        EXPECT_EQ(expected, result<FlatMap>());
    }

    {
        using FlatMap = std::flat_map<int, std::string>;

        const FlatMap expected { {1, "abcdef"}, {2, "bcdef"}, {3, "cdef"} };

        runLua("result = {'abcdef', 'bcdef', 'cdef'}");

        FlatMap actual = result();
        EXPECT_EQ(expected, actual);
        EXPECT_EQ(expected, result<FlatMap>());
    }

#if !defined(LUABRIDGE_TEST_LUA_VERSION) || LUABRIDGE_TEST_LUA_VERSION > 502
    {
        using FlatMap = std::flat_map<luabridge::LuaRef, luabridge::LuaRef>;

        const FlatMap expected {
            { luabridge::LuaRef(L, false), luabridge::LuaRef(L, true) },
            { luabridge::LuaRef(L, 'a'), luabridge::LuaRef(L, "abc") },
            { luabridge::LuaRef(L, 1), luabridge::LuaRef(L, 5) },
            { luabridge::LuaRef(L, 3.14), luabridge::LuaRef(L, -1.1) },
        };

        runLua("result = {[false] = true, a = 'abc', [1] = 5, [3.14] = -1.1}");

        auto resultRef = result();
        EXPECT_TRUE(resultRef.isInstance<FlatMap>());

        FlatMap actual = resultRef;
        EXPECT_EQ(expected, actual);

        EXPECT_EQ(expected, result<FlatMap>());
    }
#endif
}

TEST_F(FlatMapTests, CastToFlatMap)
{
    using StrToInt = std::flat_map<std::string, int>;
    runLua("result = {[1] = 2, a = 3}");
    ASSERT_EQ((StrToInt{{"1", 2}, {"a", 3}}), result<StrToInt>());

    using IntToInt = std::flat_map<int, int>;
    runLua("result = {[1] = 2, a = 3}");

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_ANY_THROW((result<IntToInt>()));
#else
    ASSERT_DEATH_IF_SUPPORTED((result<IntToInt>()), "");
#endif
}

TEST_F(FlatMapTests, PassToFunction)
{
    runLua("function foo (map) "
           "  result = map "
           "end");

    auto foo = luabridge::getGlobal(L, "foo");
    using Int2String = std::flat_map<int, std::string>;

    resetResult();

    Int2String lvalue{{10, "1"}, {20, "2"}, {30, "3"}};
    ASSERT_TRUE(foo.call(lvalue));
    ASSERT_TRUE(result().isTable());
    ASSERT_EQ(lvalue, result<Int2String>());

    resetResult();

    const Int2String constLvalue = lvalue;
    ASSERT_TRUE(foo.call(constLvalue));
    ASSERT_TRUE(result().isTable());
    ASSERT_EQ(constLvalue, result<Int2String>());
}

TEST_F(FlatMapTests, PassFromLua)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Data>("Data")
        .addConstructor<void (*)(int)>()
        .endClass()
        .addFunction("processValues", &processValues)
        .addFunction("processPointers", &processPointers);

    {
        resetResult();
        runLua("result = processValues ({[Data (-1)] = Data (2)})");
        std::flat_map<Data, Data> expected{{Data(-1), Data(2)}};
        const auto actual = result<std::flat_map<Data, Data>>();
        ASSERT_EQ(expected, actual);
    }

    {
        resetResult();
        runLua("result = processPointers ({[Data (3)] = Data (-4)})");
        std::flat_map<Data, Data> expected{{Data(3), Data(-4)}};
        const auto actual = result<std::flat_map<Data, Data>>();
        ASSERT_EQ(expected, actual);
    }
}

TEST_F(FlatMapTests, UnregisteredClass)
{
    {
#if LUABRIDGE_HAS_EXCEPTIONS
        [[maybe_unused]] luabridge::Result r;
        ASSERT_THROW((r = luabridge::push(L, std::flat_map<Unregistered, int>{ { Unregistered(), 1 } })), std::exception);
#else
        ASSERT_FALSE((luabridge::push(L, std::flat_map<Unregistered, int>{ { Unregistered(), 1 } })));
#endif
    }

    {
#if LUABRIDGE_HAS_EXCEPTIONS
        [[maybe_unused]] luabridge::Result r;
        ASSERT_THROW((r = luabridge::push(L, std::flat_map<int, Unregistered>{ { 1, Unregistered() } })), std::exception);
#else
        ASSERT_FALSE((luabridge::push(L, std::flat_map<int, Unregistered>{ { 1, Unregistered() } })));
#endif
    }
}

TEST_F(FlatMapTests, IsInstance)
{
    ASSERT_TRUE((luabridge::push(L, std::flat_map<std::string, int>{ { "x", 1 }, { "y", 2 }, { "z", 3 } })));
    EXPECT_TRUE((luabridge::isInstance<std::flat_map<std::string, int>>(L, -1)));

    lua_pop(L, 1);

    ASSERT_TRUE((luabridge::push(L, 1)));
    EXPECT_FALSE((luabridge::isInstance<std::flat_map<std::string, int>>(L, -1)));
}

TEST_F(FlatMapTests, StackOverflow)
{
    exhaustStackSpace();

    std::flat_map<std::string, int> value{ { "x", 1 }, { "y", 2 }, { "z", 3 } };

    ASSERT_FALSE(luabridge::push(L, value));
}

#if !LUABRIDGE_HAS_EXCEPTIONS
TEST_F(FlatMapTests, PushUnregisteredWithNoExceptionsShouldFailButRestoreStack)
{
    {
        const int initialStackSize = lua_gettop(L);

        lua_pushnumber(L, 1);
        EXPECT_EQ(1, lua_gettop(L) - initialStackSize);

        std::flat_map<int, Unregistered> v;
        v.emplace(std::make_pair(1, Unregistered{}));
        v.emplace(std::make_pair(2, Unregistered{}));

        auto result = luabridge::Stack<decltype(v)>::push(L, v);
        EXPECT_FALSE(result);

        EXPECT_EQ(1, lua_gettop(L) - initialStackSize);

        lua_pop(L, 1);
        EXPECT_EQ(0, lua_gettop(L) - initialStackSize);
    }

    {
        const int initialStackSize = lua_gettop(L);

        lua_pushnumber(L, 1);
        EXPECT_EQ(1, lua_gettop(L) - initialStackSize);

        std::flat_map<Unregistered, int> v;
        v.emplace(std::make_pair(Unregistered{}, 1));
        v.emplace(std::make_pair(Unregistered{}, 2));

        auto result = luabridge::Stack<decltype(v)>::push(L, v);
        EXPECT_FALSE(result);

        EXPECT_EQ(1, lua_gettop(L) - initialStackSize);

        lua_pop(L, 1);
        EXPECT_EQ(0, lua_gettop(L) - initialStackSize);
    }
}
#endif

#endif // LUABRIDGE_HAS_CXX23_FLAT_MAP
