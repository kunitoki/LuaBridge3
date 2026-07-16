# Introduction

[LuaBridge](https://github.com/kunitoki/LuaBridge3) is a lightweight and dependency-free library for mapping data, functions, and classes back and forth between C++ and [Lua](http://wwww.lua.org), a powerful, fast, lightweight, embeddable scripting language. LuaBridge has been tested and works with Lua 5.1.5, 5.2.4, 5.3.6, 5.4.8 and 5.5.0. It also works transparently with [LuaJIT](http://luajit.org/) 2.1 onwards and for the first time also with [Luau](https://luau-lang.org/) 0.713 onwards and [Ravi](https://github.com/dibyendumajumdar/ravi) 1.0-beta11.

LuaBridge is usable from a compliant C++17 and offers the following features:

* [MIT Licensed](http://www.opensource.org/licenses/mit-license.html), no usage restrictions!
* Headers-only: No Makefile, no .cpp files, just one `#include` and one header file (optional) !
* Works with ANY lua version out there (PUC-Lua, LuaJIT, Luau, Ravi, you name it).
* Simple, light, and nothing else needed.
* No macros, settings, or configuration scripts needed.
* Supports different object lifetime management models.
* Convenient, type-safe access to the Lua stack.
* Automatic function parameter type binding.
* Easy access to Lua objects like tables and functions.
* Expose C++ classes allowing them to use the flexibility of lua property lookup.
* Interoperable with most common c++ standard library container types.
* Written in a clear and easy to debug style.

It also offers a set of improvements compared to vanilla LuaBridge:

* The only binder library that works with PUC-Lua as well as LuaJIT, Luau and Ravi, wonderful for game development !
* Can work with both c++ exceptions and without (Works with `-fno-exceptions` and `/EHsc-`).
* Can safely register and use classes exposed across shared library boundaries.
* Full support for capturing lambdas in all namespace and class methods.
* Overloaded function support in Namespace functions, Class constructors, functions and static functions.
* Supports placement allocation or custom allocations/deallocations of C++ classes exposed to lua.
* Lightweight object creation: allow adding lua tables on the stack and register methods and metamethods in them.
* Allows for fallback `__index` and `__newindex` metamethods in exposed C++ classes, to support flexible and dynamic C++ classes !
* Added `std::shared_ptr` to support shared C++/Lua lifetime for types deriving from `std::enable_shared_from_this`.
* Supports conversion to and from `std::nullptr_t`, `std::byte`, `std::pair`, `std::tuple` and `std::reference_wrapper`.
* Supports conversion to and from C style arrays of any supported type.
* Supports `std::unique_ptr<T>` as an ownership container, giving Lua a non-owning view of the C++ object.
* Supports `std::move_only_function` (C++23) as a registered callable type alongside `std::function`.
* Transparent support of all signed and unsigned integer types up to `int64_t`.
* Consistent numeric handling and conversions (signed, unsigned and floats) across all lua versions.
* Simplified registration of enum types via the `luabridge::Enum` stack wrapper.
* Opt-in custom converters between registered C++ userdata types.
* Opt-out handling of safe stack space checks (automatically avoids exhausting lua stack space when pushing values!).

LuaBridge is distributed as a a collection of header files. You simply add one line, `#include <LuaBridge/LuaBridge.h>` where you want to pass functions, classes, and variables back and forth between C++ and Lua. There are no additional source files, no compilation settings, and no Makefiles or IDE-specific project files. LuaBridge is easy to integrate.

C++ concepts like variables and classes are made available to Lua through a process called _registration_. Because Lua is weakly typed, the resulting structure is not rigid. The API is based on C++ template metaprogramming. It contains template code to automatically generate at compile-time the various Lua C API calls necessary to export your program's classes and functions to the Lua environment.

To expose Lua objects to C++, a class called `luabridge::LuaRef` is provided. The implementation allows C++ code to access Lua objects such as numbers or strings, but more importantly to access things like tables and their values. Using this class makes idioms like calling Lua functions simple and clean.

## Design

LuaBridge tries to be efficient as possible when creating the "glue" that exposes C++ data and functions to Lua. At the same time, the code was written with the intention that it is all as simple and clear as possible, without resorting to obscure C++ idioms, ugly preprocessor macros, or configuration settings. Furthermore, it is designed to be "header-only", making it very easy to integrate into your projects.

Because LuaBridge was written with simplicity in mind there are some features that are not available. LuaBridge3 has been extensively optimized and now competes directly with [sol2](https://github.com/ThePhD/sol2) - one of the fastest C++/Lua binding libraries - across most workloads. In fact, LuaBridge3 outperforms sol2 in certain scenarios such as member function calls from Lua and global table writes, while remaining more compact and faster to compile. While sol2 has an edge in chained table access, the overall performance gap is negligible for most use cases. LuaBridge3 also does not try to implement every possible feature: [LuaBind](http://www.rasterbar.com/products/luabind.html) (requires Boost) and [sol2](https://github.com/ThePhD/sol2) explore every corner of the C++ language.

LuaBridge does not support:

*   Global types (types must be registered in a named scope).
*   Automatic conversion between STL container types and Lua tables (but conversion can be opted in for many standard containers by including the corresponding optional header — see [Standard Library Type Conversions](../cpp-from-lua/lua-stack.md#standard-library-type-conversions))
*   Inheriting Lua classes from C++ classes.
*   Passing nil to a C++ function that expects a pointer or reference.

## Repository

The official repository is located at [https://github.com/kunitoki/LuaBridge3](https://github.com/kunitoki/LuaBridge3).

The **master** branch contains published library versions. Release versions are marked with tags.

## License and Credits

LuaBridge3 is published under the terms of the [MIT License](http://www.opensource.org/licenses/mit-license.html):

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

The original version of LuaBridge was written by Nathan Reed. The project has been taken over by Vinnie Falco, who added new functionality and wrote the new documentation. Vinnie also incorporated `LuaRef` and other Lua to C++ binding contributions from Nigel Atkinson.

For questions, comments, or bug reports feel free to open a Github issue or contact kunitoki directly at the email address indicated below.

*   Copyright 2020, kunitoki [<kunitoki@gmail.com>](mailto:kunitoki@gmail.com)
*   Copyright 2019, Dmitry Tarakanov
*   Copyright 2012, Vinnie Falco [<vinnie.falco@gmail.com>](mailto:vinnie.falco@gmail.com)
*   Copyright 2008, Nigel Atkinson [<suprapilot+LuaCode@gmail.com>](mailto:suprapilot+LuaCode@gmail.com)
*   Copyright 2007, Nathan Reed

Older versions of LuaBridge up to and including 0.2 (available separately) are distributed under the BSD 3-Clause License. See the corresponding license file in those versions (distributed separately) for more details.
