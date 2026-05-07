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

#if LUABRIDGE_HAS_CXX20_RANGES

TEST_F(IteratorTests, RangesForLoop)
{
    runLua("result = {10, 20, 30}");

    std::vector<luabridge::LuaRef> values;
    for (auto&& [key, value] : luabridge::pairs(result()))
    {
        values.push_back(value);
    }
    EXPECT_EQ(3u, values.size());
}

TEST_F(IteratorTests, SentinelEquality)
{
    runLua("result = {1, 2, 3}");

    luabridge::Iterator begin(result(), false);
    luabridge::Iterator end(result(), true);
    luabridge::IteratorSentinel sentinel;

    EXPECT_FALSE(begin == sentinel);
    EXPECT_TRUE(end == sentinel);
    EXPECT_TRUE(sentinel == end);
}

TEST_F(IteratorTests, IteratorEquality)
{
    runLua("result = {1, 2}");

    luabridge::Iterator it1(result(), false);
    luabridge::Iterator it2(result(), false);
    luabridge::Iterator endIt(result(), true);

    EXPECT_TRUE(it1 == it2);
    EXPECT_FALSE(it1 == endIt);
    EXPECT_TRUE(endIt == endIt);
}

TEST_F(IteratorTests, EnableBorrowedRangeFalse)
{
    static_assert(!std::ranges::enable_borrowed_range<luabridge::Range>,
        "Range should not be a borrowed range");
}

#endif // LUABRIDGE_HAS_CXX20_RANGES
