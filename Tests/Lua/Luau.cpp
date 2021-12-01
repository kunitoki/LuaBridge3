//==============================================================================
/*
  https://github.com/kunitoki/LuaBridge3

  Copyright (C) 2021, Lucio Asnaghi <kunitoki@gmail.com>

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

#if LUABRIDGEDEMO_LUAU

#if _MSC_VER
#pragma push_macro("_CRT_SECURE_NO_WARNINGS")
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

// Ast
#include "../../ThirdParty/luau/Ast/src/Parser.cpp"

// Vm
#include "../../ThirdParty/luau/VM/src/ltm.cpp"
#include "../../ThirdParty/luau/VM/src/lvmexecute.cpp"
#include "../../ThirdParty/luau/VM/src/lfunc.cpp"
#include "../../ThirdParty/luau/VM/src/lbitlib.cpp"
#include "../../ThirdParty/luau/VM/src/lvmload.cpp"
#include "../../ThirdParty/luau/VM/src/lapi.cpp"
#include "../../ThirdParty/luau/VM/src/ldebug.cpp"
#include "../../ThirdParty/luau/VM/src/lbaselib.cpp"
#include "../../ThirdParty/luau/VM/src/loslib.cpp"
#include "../../ThirdParty/luau/VM/src/lutf8lib.cpp"
#include "../../ThirdParty/luau/VM/src/lstring.cpp"
#include "../../ThirdParty/luau/VM/src/lmem.cpp"
#include "../../ThirdParty/luau/VM/src/lcorolib.cpp"
#include "../../ThirdParty/luau/VM/src/lstate.cpp"
#include "../../ThirdParty/luau/VM/src/lgc.cpp"
#include "../../ThirdParty/luau/VM/src/ldo.cpp"
#include "../../ThirdParty/luau/VM/src/ltablib.cpp"
#include "../../ThirdParty/luau/VM/src/lstrlib.cpp"
#include "../../ThirdParty/luau/VM/src/lobject.cpp"
#include "../../ThirdParty/luau/VM/src/laux.cpp"
#include "../../ThirdParty/luau/VM/src/ltable.cpp"
#include "../../ThirdParty/luau/VM/src/lvmutils.cpp"
#include "../../ThirdParty/luau/VM/src/linit.cpp"
#include "../../ThirdParty/luau/VM/src/lmathlib.cpp"
#include "../../ThirdParty/luau/VM/src/lbuiltins.cpp"
#include "../../ThirdParty/luau/VM/src/ldblib.cpp"

/* lperf.cpp includes Windows.h, which defines the min and max macro,
   so include it last to prevent errors.
*/
#include "../../ThirdParty/luau/VM/src/lperf.cpp"

#if _MSC_VER
#pragma pop_macro("_CRT_SECURE_NO_WARNINGS")
#endif

#else
void dummy_symbol_luau() {}

#endif // LUABRIDGEDEMO_LUAU
