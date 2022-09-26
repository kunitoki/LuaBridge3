// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, Lucio Asnaghi
// Copyright 2020, Dmitry Tarakanov
// Copyright 2019, George Tokmaji
// SPDX-License-Identifier: MIT

#pragma once

#if !(__cplusplus >= 201703L || (defined(_MSC_VER) && _HAS_CXX17))
#error LuaBridge 3 requires a compliant C++17 compiler, or C++17 has not been enabled !
#endif

#if defined(_MSC_VER)
#if _CPPUNWIND || _HAS_EXCEPTIONS
#define LUABRIDGE_HAS_EXCEPTIONS 1
#else
#define LUABRIDGE_HAS_EXCEPTIONS 0
#endif
#elif defined(__clang__)
#if __EXCEPTIONS && __has_feature(cxx_exceptions)
#define LUABRIDGE_HAS_EXCEPTIONS 1
#else
#define LUABRIDGE_HAS_EXCEPTIONS 0
#endif
#elif defined(__GNUC__)
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS)
#define LUABRIDGE_HAS_EXCEPTIONS 1
#else
#define LUABRIDGE_HAS_EXCEPTIONS 0
#endif
#endif

#if defined(LUAU_FASTMATH_BEGIN)
#define LUABRIDGE_ON_LUAU 1
#elif defined(LUA_JROOT)
#define LUABRIDGE_ON_LUAJIT 1
#elif defined(LUA_VERSION_NUM)
#define LUABRIDGE_ON_LUA 1
#else
#error "Lua headers must be included prior to LuaBridge ones"
#endif

#if !defined(LUABRIDGE_SAFE_STACK_CHECKS)
#define LUABRIDGE_SAFE_STACK_CHECKS 1
#endif
