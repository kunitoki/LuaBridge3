// https://github.com/kunitoki/LuaBridge3
// Copyright 2023, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include "LuaBridge/Set.h"

#include <set>

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

std::set<Data> processValues(const std::set<Data>& data)
{
    return data;
}

std::set<Data> processPointers(const std::set<const Data*>& data)
{
    std::set<Data> result;

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

struct SetTests : TestBase
{
};

TEST_F(SetTests, LuaRef)
{
    {
        using Set = std::set<int>;

        const Set expected { 1, 2, 3 };

        runLua("result = {1, 2, 3}");

        Set actual = result();
        EXPECT_EQ(expected, actual);
        EXPECT_EQ(expected, result<Set>());
    }

    {
        using Set = std::set<std::string>;

        const Set expected { {"abcdef"}, {"bcdef"}, {"cdef"} };

        runLua("result = {'abcdef', 'bcdef', 'cdef'}");

        Set actual = result();
        EXPECT_EQ(expected, actual);
        EXPECT_EQ(expected, result<Set>());
    }

    {
        using Set = std::set<luabridge::LuaRef>;

        const Set expected {
            luabridge::LuaRef(L, 'a'),
            luabridge::LuaRef(L, 1),
            luabridge::LuaRef(L, 3.14),
            luabridge::LuaRef(L, "abc"),
            luabridge::LuaRef(L, 5),
            luabridge::LuaRef(L, -1.1),
        };

        runLua("result = {'a', 1, 3.14, 'abc', 5, -1.1}");

        auto resultRef = result();
        EXPECT_TRUE(resultRef.isInstance<Set>());

        Set actual = resultRef;
        EXPECT_EQ(expected, actual);

        EXPECT_EQ(expected, result<Set>());
    }
}

TEST_F(SetTests, CastToSet)
{
    using StringSet = std::set<std::string>;
    runLua("result = {'1', 'a'}");
    ASSERT_EQ((StringSet{"1", "a"}), result<StringSet>());

    using IntSet = std::set<int>;
    runLua("result = {2, 'a'}");

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_ANY_THROW((result<IntSet>()));
#else
    ASSERT_DEATH((result<IntSet>()), "");
#endif
}

TEST_F(SetTests, PassToFunction)
{
    runLua("function foo (set) "
           "  result = set "
           "end");

    auto foo = luabridge::getGlobal(L, "foo");
    using IntSet = std::set<int>;

    resetResult();

    IntSet lvalue{ 10, 20, 30 };
    foo(lvalue);
    ASSERT_TRUE(result().isTable());
    ASSERT_EQ(lvalue, result<IntSet>());

    resetResult();

    const IntSet constLvalue = lvalue;
    foo(constLvalue);
    ASSERT_TRUE(result().isTable());
    ASSERT_EQ(constLvalue, result<IntSet>());
}

TEST_F(SetTests, PassFromLua)
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
        std::set<Data> expected{ Data(-1), Data(2) };
        const auto actual = result<std::set<Data>>();
        ASSERT_EQ(expected, actual);
    }

    {
        resetResult();
        runLua("result = processPointers ({ Data(3), Data(-4) })");
        std::set<Data> expected{ Data(3), Data(-4) };
        const auto actual = result<std::set<Data>>();
        ASSERT_EQ(expected, actual);
    }
}

TEST_F(SetTests, UnregisteredClass)
{
    {
#if LUABRIDGE_HAS_EXCEPTIONS
        [[maybe_unused]] luabridge::Result r;
        ASSERT_THROW((r = luabridge::push(L, std::set<Unregistered>{ Unregistered() })), std::exception);
#else
        ASSERT_FALSE((luabridge::push(L, std::set<Unregistered>{ Unregistered() })));
#endif
    }
}

TEST_F(SetTests, IsInstance)
{
    ASSERT_TRUE((luabridge::push(L, std::set<std::string>{ "x", "y", "z" })));
    EXPECT_TRUE((luabridge::isInstance<std::set<std::string>>(L, -1)));

    lua_pop(L, 1);

    ASSERT_TRUE((luabridge::push(L, 1)));
    EXPECT_FALSE((luabridge::isInstance<std::set<int>>(L, -1)));
}

TEST_F(SetTests, StackOverflow)
{
    exhaustStackSpace();

    std::set<std::string> value{ "x", "y", "z" };

    ASSERT_FALSE(luabridge::push(L, value));
}

#if !LUABRIDGE_HAS_EXCEPTIONS
TEST_F(SetTests, PushUnregisteredWithNoExceptionsShouldFailButRestoreStack)
{
    {
        const int initialStackSize = lua_gettop(L);

        lua_pushnumber(L, 1);
        EXPECT_EQ(1, lua_gettop(L) - initialStackSize);

        std::set<Unregistered> v;
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
