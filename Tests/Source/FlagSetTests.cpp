// https://github.com/kunitoki/LuaBridge3
// Copyright 2023, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#include "TestBase.h"

namespace {
struct FlagSetOption1;
struct FlagSetOption2;

using Options = luabridge::FlagSet<uint32_t, FlagSetOption1, FlagSetOption2>;
static inline constexpr Options defaultOptions = Options();
static inline constexpr Options option1 = Options::Value<FlagSetOption1>();
static inline constexpr Options option2 = Options::Value<FlagSetOption2>();
} // namespace

TEST(FlagSetTests, DefaultConstructor)
{
    {
        Options opts;
        EXPECT_FALSE(opts.test(option1));
        EXPECT_FALSE(opts.test(option2));
    }

    {
        Options opts = defaultOptions;
        EXPECT_FALSE(opts.test(option1));
        EXPECT_FALSE(opts.test(option2));
    }

    {
        EXPECT_FALSE(defaultOptions.test(option1));
        EXPECT_FALSE(defaultOptions.test(option2));
    }
}

TEST(FlagSetTests, OperatorOr)
{
    Options opts = option1 | option2;
    EXPECT_TRUE(opts.test(option1));
    EXPECT_TRUE(opts.test(option2));
}

TEST(FlagSetTests, OperatorAnd)
{
    Options opts1 = option1 | option2;
    Options opts2 = option1;

    Options opts = opts1 & opts2;
    EXPECT_TRUE(opts.test(option1));
    EXPECT_FALSE(opts.test(option2));
}

TEST(FlagSetTests, OperatorNeg)
{
    Options opts;
    EXPECT_EQ(std::numeric_limits<uint32_t>::min(), opts.toUnderlying());

    opts = ~opts;
    EXPECT_EQ(std::numeric_limits<uint32_t>::max(), opts.toUnderlying());
}

TEST(FlagSetTests, Set)
{
    Options opts;
    EXPECT_FALSE(opts.test(option1));
    EXPECT_FALSE(opts.test(option2));

    opts.set(option1);
    EXPECT_TRUE(opts.test(option1));
    EXPECT_FALSE(opts.test(option2));
}

TEST(FlagSetTests, WithSet)
{
    Options opts;
    EXPECT_FALSE(opts.test(option1));
    EXPECT_FALSE(opts.test(option2));

    Options opts2 = opts.withSet(option1);
    EXPECT_FALSE(opts.test(option1));
    EXPECT_FALSE(opts.test(option2));

    EXPECT_TRUE(opts2.test(option1));
    EXPECT_FALSE(opts2.test(option2));
}

TEST(FlagSetTests, Unset)
{
    Options opts = option1 | option2;
    EXPECT_TRUE(opts.test(option1));
    EXPECT_TRUE(opts.test(option2));

    opts.unset(option1);
    EXPECT_FALSE(opts.test(option1));
    EXPECT_TRUE(opts.test(option2));
}

TEST(FlagSetTests, WithUnset)
{
    Options opts = option1 | option2;
    EXPECT_TRUE(opts.test(option1));
    EXPECT_TRUE(opts.test(option2));

    Options opts2 = opts.withUnset(option1);
    EXPECT_TRUE(opts.test(option1));
    EXPECT_TRUE(opts.test(option2));

    EXPECT_FALSE(opts2.test(option1));
    EXPECT_TRUE(opts2.test(option2));
}

TEST(FlagSetTests, FromUnderlying)
{
    {
        Options opts = Options::fromUnderlying(1);
        EXPECT_TRUE(opts.test(option1));
        EXPECT_EQ(1, opts.toUnderlying());
    }

    {
        Options opts = Options::fromUnderlying(3);
        EXPECT_TRUE(opts.test(option1 | option2));
        EXPECT_EQ(3, opts.toUnderlying());
    }
}

TEST(FlagSetTests, ToString)
{
    Options opts0 = defaultOptions;
    EXPECT_EQ("00000000000000000000000000000000", opts0.toString());

    Options opts1 = option1;
    EXPECT_EQ("00000000000000000000000000000001", opts1.toString());

    Options opts2 = option2;
    EXPECT_EQ("00000000000000000000000000000010", opts2.toString());

    Options opts12 = option1 | option2;
    EXPECT_EQ("00000000000000000000000000000011", opts12.toString());
}
