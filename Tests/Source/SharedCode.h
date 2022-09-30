// https://github.com/kunitoki/LuaBridge3
// Copyright 2022, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#pragma once

#include "Lua/LuaLibrary.h"

#if _WIN32
#if LUABRIDGEDEMO_SHARED_EXPORT
#define LUABRIDGEDEMO_SHARED_API __declspec(dllexport)
#else
#define LUABRIDGEDEMO_SHARED_API __declspec(dllimport)
#endif
#else
#define LUABRIDGEDEMO_SHARED_API
#endif

#include <string>

namespace xyz {

class LUABRIDGEDEMO_SHARED_API ISharedClass
{
public:
    ISharedClass();
    virtual ~ISharedClass();

    virtual int publicMethod(const std::string& s) const = 0;
};

} // namespace xyz
