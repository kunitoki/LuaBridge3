// https://github.com/kunitoki/LuaBridge3
// Copyright 2018, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#if LUABRIDGE_TEST_LUAU && LUABRIDGE_TEST_LUAU_INTEGER64
#include "../../ThirdParty/luau/Common/include/Luau/Common.h"

LUAU_FASTFLAG(LuauIntegerLibrary)
LUAU_FASTFLAG(LuauIntegerType2)
#endif

#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)
#include <backward.hpp>
#endif

int main(int argc, char** argv)
{
#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)
    backward::SignalHandling sh;
#endif

    // Disable performance tests by default
    if (argc == 1)
    {
        testing::GTEST_FLAG(filter) = "-PerformanceTests.AllTests";
    }

#if LUABRIDGE_TEST_LUAU && LUABRIDGE_TEST_LUAU_INTEGER64
    // Native 64-bit integers are only pushed when the Luau integer library flag is enabled
    FFlag::LuauIntegerLibrary.value = true;
    FFlag::LuauIntegerType2.value = true;
#endif

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
