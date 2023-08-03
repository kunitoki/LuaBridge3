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

// Compiler
#include "../../ThirdParty/luau/Compiler/src/BuiltinFolding.cpp"
#include "../../ThirdParty/luau/Compiler/src/Builtins.cpp"
#include "../../ThirdParty/luau/Compiler/src/BytecodeBuilder.cpp"
#include "../../ThirdParty/luau/Compiler/src/Compiler.cpp"
#include "../../ThirdParty/luau/Compiler/src/ConstantFolding.cpp"
#include "../../ThirdParty/luau/Compiler/src/CostModel.cpp"
#include "../../ThirdParty/luau/Compiler/src/lcode.cpp"
#include "../../ThirdParty/luau/Compiler/src/TableShape.cpp"
#include "../../ThirdParty/luau/Compiler/src/Types.cpp"
#include "../../ThirdParty/luau/Compiler/src/ValueTracking.cpp"

// Ast
#include "../../ThirdParty/luau/Ast/src/Ast.cpp"
#include "../../ThirdParty/luau/Ast/src/Confusables.cpp"
#include "../../ThirdParty/luau/Ast/src/StringUtils.cpp"
#include "../../ThirdParty/luau/Ast/src/Location.cpp"
#include "../../ThirdParty/luau/Ast/src/Lexer.cpp"
#include "../../ThirdParty/luau/Ast/src/TimeTrace.cpp"

#if _MSC_VER
#pragma pop_macro("_CRT_SECURE_NO_WARNINGS")
#endif

#else
void dummy_symbol_luau2() {}

#endif // LUABRIDGEDEMO_LUAU
