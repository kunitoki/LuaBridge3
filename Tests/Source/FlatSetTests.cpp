// https://github.com/kunitoki/LuaBridge3
// Copyright 2023, kunitoki
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include "LuaBridge/FlatSet.h"

#if LUABRIDGE_HAS_CXX23_FLAT_SET

#include <flat_set>
#include <string>

namespace {
struct Unregistered
{
    bool operator<(const Unregistered& other) const
    {
        return false;
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

std::flat_set<Data> processValues(const std::flat_set<Data>& data)
{
    return data;
}

std::flat_set<Data> processPointers(const std::flat_set<const Data*>& data)
{
    std::flat_set<Data> result;

    for (const auto& item : data)
        result.emplace(*item);

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

struct FlatSetTests : TestBase
{
};

TEST_F(FlatSetTests, GetNonTable)
{
    lua_pushnumber(L, 42.0);

    auto result = luabridge::Stack<std::flat_set<int>>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(FlatSetTests, LuaRef)
{
    {
        using FlatSet = std::flat_set<int>;

        const FlatSet expected { 1, 2, 3 };

        runLua("result = {1, 2, 3}");

        FlatSet actual = result();
        EXPECT_EQ(expected, actual);
        EXPECT_EQ(expected, result<FlatSet>());
    }

    {
        using FlatSet = std::flat_set<std::string>;

        const FlatSet expected { {"abcdef"}, {"bcdef"}, {"cdef"} };

        runLua("result = {'abcdef', 'bcdef', 'cdef'}");

        FlatSet actual = result();
        EXPECT_EQ(expected, actual);
        EXPECT_EQ(expected, result<FlatSet>());
    }

#if !defined(LUABRIDGE_TEST_LUA_VERSION) || LUABRIDGE_TEST_LUA_VERSION > 502
    {
        using FlatSet = std::flat_set<luabridge::LuaRef>;

        const FlatSet expected {
            luabridge::LuaRef(L, 'a'),
            luabridge::LuaRef(L, 1),
            luabridge::LuaRef(L, 3.14),
            luabridge::LuaRef(L, "abc"),
            luabridge::LuaRef(L, 5),
            luabridge::LuaRef(L, -1.1),
        };

        runLua("result = {'a', 1, 3.14, 'abc', 5, -1.1}");

        auto resultRef = result();
        EXPECT_TRUE(resultRef.isInstance<FlatSet>());

        FlatSet actual = resultRef;
        EXPECT_EQ(expected, actual);

        EXPECT_EQ(expected, result<FlatSet>());
    }
#endif
}

TEST_F(FlatSetTests, CastToFlatSet)
{
    using StringFlatSet = std::flat_set<std::string>;
    runLua("result = {'1', 'a'}");
    ASSERT_EQ((StringFlatSet{"1", "a"}), result<StringFlatSet>());

    using IntFlatSet = std::flat_set<int>;
    runLua("result = {2, 'a'}");

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_ANY_THROW((result<IntFlatSet>()));
#else
    ASSERT_DEATH_IF_SUPPORTED((result<IntFlatSet>()), "");
#endif
}

TEST_F(FlatSetTests, PassToFunction)
{
    runLua("function foo (set) "
           "  result = set "
           "end");

    auto foo = luabridge::getGlobal(L, "foo");
    using IntFlatSet = std::flat_set<int>;

    resetResult();

    IntFlatSet lvalue{ 10, 20, 30 };
    ASSERT_TRUE(foo.call(lvalue));
    ASSERT_TRUE(result().isTable());
    ASSERT_EQ(lvalue, result<IntFlatSet>());

    resetResult();

    const IntFlatSet constLvalue = lvalue;
    ASSERT_TRUE(foo.call(constLvalue));
    ASSERT_TRUE(result().isTable());
    ASSERT_EQ(constLvalue, result<IntFlatSet>());
}

TEST_F(FlatSetTests, PassFromLua)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Data>("Data")
        .addConstructor<void (*)(int)>()
        .endClass()
        .addFunction("processValues", &processValues)
        .addFunction("processPointers", &processPointers);

    {
        resetResult();
        runLua("result = processValues ({ Data(-1), Data(2) })");
        std::flat_set<Data> expected{ Data(-1), Data(2) };
        const auto actual = result<std::flat_set<Data>>();
        ASSERT_EQ(expected, actual);
    }

    {
        resetResult();
        runLua("result = processPointers ({ Data(3), Data(-4) })");
        std::flat_set<Data> expected{ Data(3), Data(-4) };
        const auto actual = result<std::flat_set<Data>>();
        ASSERT_EQ(expected, actual);
    }
}

TEST_F(FlatSetTests, UnregisteredClass)
{
    {
#if LUABRIDGE_HAS_EXCEPTIONS
        [[maybe_unused]] luabridge::Result r;
        ASSERT_THROW((r = luabridge::push(L, std::flat_set<Unregistered>{ Unregistered() })), std::exception);
#else
        ASSERT_FALSE((luabridge::push(L, std::flat_set<Unregistered>{ Unregistered() })));
#endif
    }
}

TEST_F(FlatSetTests, IsInstance)
{
    ASSERT_TRUE((luabridge::push(L, std::flat_set<std::string>{ "x", "y", "z" })));
    EXPECT_TRUE((luabridge::isInstance<std::flat_set<std::string>>(L, -1)));

    lua_pop(L, 1);

    ASSERT_TRUE((luabridge::push(L, 1)));
    EXPECT_FALSE((luabridge::isInstance<std::flat_set<int>>(L, -1)));
}

TEST_F(FlatSetTests, StackOverflow)
{
    exhaustStackSpace();

    std::flat_set<std::string> value{ "x", "y", "z" };

    ASSERT_FALSE(luabridge::push(L, value));
}

#if !LUABRIDGE_HAS_EXCEPTIONS
TEST_F(FlatSetTests, PushUnregisteredWithNoExceptionsShouldFailButRestoreStack)
{
    {
        const int initialStackSize = lua_gettop(L);

        lua_pushnumber(L, 1);
        EXPECT_EQ(1, lua_gettop(L) - initialStackSize);

        std::flat_set<Unregistered> v;
        v.emplace(Unregistered{});
        v.emplace(Unregistered{});

        auto result = luabridge::Stack<decltype(v)>::push(L, v);
        EXPECT_FALSE(result);

        EXPECT_EQ(1, lua_gettop(L) - initialStackSize);

        lua_pop(L, 1);
        EXPECT_EQ(0, lua_gettop(L) - initialStackSize);
    }
}
#endif

#endif // LUABRIDGE_HAS_CXX23_FLAT_SET
