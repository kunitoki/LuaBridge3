//==============================================================================
/*
  https://github.com/kunitoki/LuaBridge3

  Copyright (C) 2020, Lucio Asnaghi <kunitoki@gmail.com>
  Copyright (C) 2012, Vinnie Falco <vinnie.falco@gmail.com>

  License: The MIT License (http://www.opensource.org/licenses/mit-license.php)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
//==============================================================================

#pragma once

// This determines which version of Lua to use.
// The value is the same as LUA_VERSION_NUM in lua.h

#ifndef LUABRIDGEDEMO_LUA_VERSION
#define LUABRIDGEDEMO_LUA_VERSION 504 // By default use 5.4
#endif

#ifndef LUALIBRARY_SOURCE

#if LUABRIDGEDEMO_LUA_VERSION >= 504
#include "Lua.5.4.3/src/lua.hpp"

#elif LUABRIDGEDEMO_LUA_VERSION >= 503
#include "Lua.5.3.6/src/lua.hpp"

#elif LUABRIDGEDEMO_LUA_VERSION >= 502
#include "Lua.5.2.4/src/lua.hpp"

#elif LUABRIDGEDEMO_LUA_VERSION >= 501
extern "C" {
#include "Lua.5.1.5/src/lua.h"
#include "Lua.5.1.5/src/lualib.h"
#include "Lua.5.1.5/src/lauxlib.h"
} // extern "C"

#else
#error "Unknown LUA_VERSION_NUM"

#endif // LUABRIDGEDEMO_LUA_VERSION

#endif // LUALIBRARY_SOURCE
