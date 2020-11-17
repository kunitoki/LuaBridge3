// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2020, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT

#pragma once

// All #include dependencies are listed here
// instead of in the individual header files.

#define LUABRIDGE_MAJOR_VERSION 3
#define LUABRIDGE_MINOR_VERSION 0
#define LUABRIDGE_VERSION 300

#ifndef LUA_VERSION_NUM
#error "Lua headers must be included prior to LuaBridge ones"
#endif

#include "detail/Config.h"
#include "detail/CFunctions.h"
#include "detail/ClassInfo.h"
#include "detail/Constructor.h"
#include "detail/FuncTraits.h"
#include "detail/Iterator.h"
#include "detail/LuaException.h"
#include "detail/LuaHelpers.h"
#include "detail/LuaRef.h"
#include "detail/Namespace.h"
#include "detail/Security.h"
#include "detail/Stack.h"
#include "detail/TypeList.h"
#include "detail/TypeTraits.h"
#include "detail/Userdata.h"
