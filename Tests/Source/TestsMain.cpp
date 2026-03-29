// https://github.com/kunitoki/LuaBridge3
// Copyright 2018, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#if !defined(_WIN32)
#include <backward.hpp>
#endif

int main(int argc, char** argv)
{
#if !defined(_WIN32)
    backward::SignalHandling sh;
#endif

    // Disable performance tests by default
    if (argc == 1)
    {
        testing::GTEST_FLAG(filter) = "-PerformanceTests.AllTests";
    }

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
