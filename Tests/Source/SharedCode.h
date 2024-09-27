// https://github.com/kunitoki/LuaBridge3
// Copyright 2022, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#pragma once

#include "Lua/LuaLibrary.h"

#if _WIN32
#if LUABRIDGE_TEST_SHARED_EXPORT
#define LUABRIDGE_TEST_SHARED_API __declspec(dllexport)
#else
#define LUABRIDGE_TEST_SHARED_API __declspec(dllimport)
#endif
#else
#define LUABRIDGE_TEST_SHARED_API
#endif

#include <string>

namespace xyz {

class LUABRIDGE_TEST_SHARED_API ISharedClass
{
public:
    ISharedClass();
    virtual ~ISharedClass();

    virtual int publicMethod(const std::string& s) const = 0;
};

} // namespace xyz
