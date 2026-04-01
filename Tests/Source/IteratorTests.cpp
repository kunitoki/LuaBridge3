// https://github.com/kunitoki/LuaBridge3
// Copyright 2018, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include "LuaBridge/detail/Iterator.h"

struct IteratorTests : TestBase
{
};

TEST_F(IteratorTests, DictionaryIteration)
{
    runLua("result = {"
           "  bool = true,"
           "  int = 5,"
           "  c = 3.14,"
           "  [true] = 'D',"
           "  [8] = 'abc',"
           "  fn = function (i)"
           "    result = i + 1"
           "  end"
           "}");

    std::map<luabridge::LuaRef, luabridge::LuaRef> expected{
        {{L, "bool"}, {L, true}},
        {{L, "int"}, {L, 5}},
        {{L, 'c'}, {L, 3.14}},
        {{L, true}, {L, 'D'}},
        {{L, 8}, {L, "abc"}},
        {{L, "fn"}, {L, result()["fn"]}},
    };

    std::map<luabridge::LuaRef, luabridge::LuaRef> actual;

    for (luabridge::Iterator iterator(result()); !iterator.isNil(); ++iterator)
    {
        EXPECT_EQ(L, iterator.state());

        actual.emplace(iterator.key(), iterator.value());
    }

    ASSERT_EQ(expected, actual);

    actual.clear();

    for (auto&& pair : pairs(result()))
    {
        actual.emplace(pair.first, pair.second);
    }

    ASSERT_EQ(expected, actual);
}

TEST_F(IteratorTests, IncrementPastEnd)
{
    runLua("result = {1, 2, 3}");

    luabridge::Iterator it(result());
    while (!it.isNil())
        ++it;

    // Now at end (isNil() == true) - incrementing should be a no-op
    EXPECT_TRUE(it.isNil());
    ++it;
    EXPECT_TRUE(it.isNil());
}

TEST_F(IteratorTests, SequenceIteration)
{
    runLua("result = {"
           "  true,"
           "  5,"
           "  3.14,"
           "  'D',"
           "  'abc',"
           "  function (i)"
           "    result = i + 1"
           "  end"
           "}");

    std::map<luabridge::LuaRef, luabridge::LuaRef> expected{
        {{L, 1}, {L, true}},
        {{L, 2}, {L, 5}},
        {{L, 3}, {L, 3.14}},
        {{L, 4}, {L, 'D'}},
        {{L, 5}, {L, "abc"}},
        {{L, 6}, {L, result()[6]}},
    };

    std::map<luabridge::LuaRef, luabridge::LuaRef> actual;

    for (luabridge::Iterator iterator(result()); !iterator.isNil(); ++iterator)
    {
        EXPECT_EQ(L, iterator.state());

        actual.emplace(iterator.key(), iterator.value());
    }

    ASSERT_EQ(expected, actual);

    actual.clear();

    for (auto&& pair : pairs(result()))
    {
        actual.emplace(pair.first, pair.second);
    }

    ASSERT_EQ(expected, actual);
}

TEST_F(IteratorTests, StackOverflowDuringIteration)
{
    // Covers Iterator.h:138-140 - stack overflow path in Iterator::next()
    runLua("result = {1, 2, 3}");

    luabridge::Iterator it(result());
    ASSERT_FALSE(it.isNil()); // iterator starts valid

    exhaustStackSpace();

    ++it; // next() will fail lua_checkstack and set key/value to nil
    EXPECT_TRUE(it.isNil());

    lua_settop(L, 0);
}
