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

#define LUALIBRARY_SOURCE
#include "Lua/LuaLibrary.h"

#if LUABRIDGEDEMO_LUA_VERSION == 502

#if _MSC_VER
#pragma push_macro("_CRT_SECURE_NO_WARNINGS")
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/* luaconf.h only declares some things if it is being included
   from certain source files. We define all the relevant macros
   and include luaconf.h once so we get all the declarations.
*/
#define lobject_c
#define lvm_c
#define LUA_CORE
#define LUA_LIB
#include "Lua.5.2.4/src/luaconf.h"
#undef lobject_c
#undef lvm_c
#undef LUA_CORE
#undef LUA_LIB

#if _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4244) /* Possible loss of data */
#pragma warning (disable: 4297) /* Function assumed not to throw an exception but does */
#pragma warning (disable: 4334) /* Result of 32-bit shift implicitly converted to 64 bits */
#pragma warning (disable: 4702) /* Unreachable code */
#elif __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wempty-body"
#pragma GCC diagnostic ignored "-Wstring-plus-int"
#endif

/* Include this early to prevent the conflict with luai_hashnum
   and supress the warning caused by #define lua_assert
*/
#include "Lua.5.2.4/src/ltable.c"

#include "Lua.5.2.4/src/lauxlib.c"
#include "Lua.5.2.4/src/lbaselib.c"

#include "Lua.5.2.4/src/lbitlib.c"
#include "Lua.5.2.4/src/lcorolib.c"
#include "Lua.5.2.4/src/ldblib.c"
#include "Lua.5.2.4/src/linit.c"
#include "Lua.5.2.4/src/liolib.c"
#include "Lua.5.2.4/src/lmathlib.c"
#include "Lua.5.2.4/src/loslib.c"
#include "Lua.5.2.4/src/lstrlib.c"
#include "Lua.5.2.4/src/ltablib.c"

#include "Lua.5.2.4/src/lapi.c"
#include "Lua.5.2.4/src/lcode.c"
#include "Lua.5.2.4/src/lctype.c"
#include "Lua.5.2.4/src/ldebug.c"
#include "Lua.5.2.4/src/ldo.c"
#include "Lua.5.2.4/src/ldump.c"
#include "Lua.5.2.4/src/lfunc.c"
#include "Lua.5.2.4/src/lgc.c"
#include "Lua.5.2.4/src/llex.c"
#include "Lua.5.2.4/src/lmem.c"
#include "Lua.5.2.4/src/lobject.c"
#include "Lua.5.2.4/src/lopcodes.c"
#include "Lua.5.2.4/src/lparser.c"
#include "Lua.5.2.4/src/lstate.c"
#include "Lua.5.2.4/src/lstring.c"
#include "Lua.5.2.4/src/ltm.c"
#include "Lua.5.2.4/src/lundump.c"
#include "Lua.5.2.4/src/lvm.c"
#include "Lua.5.2.4/src/lzio.c"

/* loadlib.c includes Windows.h, which defines the LoadString macro,
   so include it last to prevent errors.
*/
#include "Lua.5.2.4/src/loadlib.c"

#if _MSC_VER
#pragma warning (pop)
#elif __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef __cplusplus
}
#endif

#if _MSC_VER
#pragma pop_macro("_CRT_SECURE_NO_WARNINGS")
#endif

#else
void dummy_symbol_lua52() {}

#endif // LUABRIDGEDEMO_LUA_VERSION == 502
