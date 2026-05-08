// https://github.com/kunitoki/LuaBridge3
// Copyright 2023, kunitoki
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include "LuaBridge/UnorderedSet.h"

#include <unordered_set>

struct Unregistered
{
    bool operator<(const Unregistered& other) const { return true; }
    bool operator==(const Unregistered& other) const { return true; }
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

namespace std {
template <>
struct hash<Unregistered>
{
    std::size_t operator()(const Unregistered& value) const
    {
        return 0;
    }
};

template <>
struct hash<Data>
{
    std::size_t operator()(const Data& value) const
    {
        return std::hash<int>{}(value.i);
    }
};
} // namespace std

namespace {
std::unordered_set<Data> processValues(const std::unordered_set<Data>& data)
{
    return data;
}

std::unordered_set<Data> processPointers(const std::unordered_set<const Data*>& data)
{
    std::unordered_set<Data> result;

    for (const auto& item : data)
        result.emplace(*item);

    return result;
}
} // namespace

struct UnorderedSetTests : TestBase
{
};

TEST_F(UnorderedSetTests, GetNonTable)
{
    lua_pushnumber(L, 42.0);

    auto result = luabridge::Stack<std::unordered_set<int>>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(UnorderedSetTests, LuaRef)
{
    {
        using UnorderedSet = std::unordered_set<int>;

        const UnorderedSet expected { 1, 2, 3 };

        runLua("result = {1, 2, 3}");

        UnorderedSet actual = result();
        EXPECT_EQ(expected, actual);
        EXPECT_EQ(expected, result<UnorderedSet>());
    }

    {
        using UnorderedSet = std::unordered_set<std::string>;

        const UnorderedSet expected { {"abcdef"}, {"bcdef"}, {"cdef"} };

        runLua("result = {'abcdef', 'bcdef', 'cdef'}");

        UnorderedSet actual = result();
        EXPECT_EQ(expected, actual);
        EXPECT_EQ(expected, result<UnorderedSet>());
    }
}

TEST_F(UnorderedSetTests, CastToSet)
{
    using StringUnorderedSet = std::unordered_set<std::string>;
    runLua("result = {'1', 'a'}");
    ASSERT_EQ((StringUnorderedSet{"1", "a"}), result<StringUnorderedSet>());

    using IntUnorderedSet = std::unordered_set<int>;
    runLua("result = {2, 'a'}");

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_ANY_THROW((result<IntUnorderedSet>()));
#else
    ASSERT_DEATH_IF_SUPPORTED((result<IntUnorderedSet>()), "");
#endif
}

TEST_F(UnorderedSetTests, PassToFunction)
{
    runLua("function foo (unorderedSet) "
           "  result = unorderedSet "
           "end");

    auto foo = luabridge::getGlobal(L, "foo");
    using IntUnorderedSet = std::unordered_set<int>;

    resetResult();

    IntUnorderedSet lvalue{ 10, 20, 30 };
    ASSERT_TRUE(foo.call(lvalue));
    ASSERT_TRUE(result().isTable());
    ASSERT_EQ(lvalue, result<IntUnorderedSet>());

    resetResult();

    const IntUnorderedSet constLvalue = lvalue;
    ASSERT_TRUE(foo.call(constLvalue));
    ASSERT_TRUE(result().isTable());
    ASSERT_EQ(constLvalue, result<IntUnorderedSet>());
}

TEST_F(UnorderedSetTests, PassFromLua)
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
        std::unordered_set<Data> expected{ Data(-1), Data(2) };
        const auto actual = result<std::unordered_set<Data>>();
        ASSERT_EQ(expected, actual);
    }

    {
        resetResult();
        runLua("result = processPointers ({ Data(3), Data(-4) })");
        std::unordered_set<Data> expected{ Data(3), Data(-4) };
        const auto actual = result<std::unordered_set<Data>>();
        ASSERT_EQ(expected, actual);
    }
}

TEST_F(UnorderedSetTests, UnregisteredClass)
{
    {
#if LUABRIDGE_HAS_EXCEPTIONS
        [[maybe_unused]] luabridge::Result r;
        ASSERT_THROW((r = luabridge::push(L, std::unordered_set<Unregistered>{ Unregistered() })), std::exception);
#else
        ASSERT_FALSE((luabridge::push(L, std::unordered_set<Unregistered>{ Unregistered() })));
#endif
    }
}

TEST_F(UnorderedSetTests, IsInstance)
{
    ASSERT_TRUE((luabridge::push(L, std::unordered_set<std::string>{ "x", "y", "z" })));
    EXPECT_TRUE((luabridge::isInstance<std::unordered_set<std::string>>(L, -1)));

    lua_pop(L, 1);

    ASSERT_TRUE((luabridge::push(L, 1)));
    EXPECT_FALSE((luabridge::isInstance<std::unordered_set<int>>(L, -1)));
}

TEST_F(UnorderedSetTests, StackOverflow)
{
    exhaustStackSpace();

    std::unordered_set<std::string> value{ "x", "y", "z" };

    ASSERT_FALSE(luabridge::push(L, value));
}

#if !LUABRIDGE_HAS_EXCEPTIONS
TEST_F(UnorderedSetTests, PushUnregisteredWithNoExceptionsShouldFailButRestoreStack)
{
    {
        const int initialStackSize = lua_gettop(L);

        lua_pushnumber(L, 1);
        EXPECT_EQ(1, lua_gettop(L) - initialStackSize);

        std::unordered_set<Unregistered> v;
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
