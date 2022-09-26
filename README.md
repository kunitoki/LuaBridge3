<a href="https://kunitoki.github.io/LuaBridge3">
<img src="https://github.com/kunitoki/LuaBridge3/blob/master/logo.png?raw=true">
</a>
<a href="https://lua.org">
<img src="https://github.com/kunitoki/LuaBridge3/blob/master/lua.png?raw=true">
</a>
<br>

# LuaBridge 3.0

[LuaBridge][1] is a lightweight and dependency-free library for mapping data,
functions, and classes back and forth between C++ and [Lua][2] (a powerful,
fast, lightweight, embeddable scripting language). LuaBridge has been tested
and works with Lua revisions starting from 5.1.5, and also compatibility is
provided with lua 5.2.4, 5.3.6 and 5.4.4 as well as [LuaJit][3] and [Luau][4].

## Features

LuaBridge is usable from a compliant C++17 compiler and offers the following features:

* [MIT Licensed][5]
* Headers-only: No Makefile, no .cpp files, just one `#include` and one header file (optional) !
* Simple, light, and nothing else needed.
* No macros, settings, or configuration scripts needed.
* Supports different object lifetime management models.
* Convenient, type-safe access to the Lua stack.
* Automatic function parameter type binding.
* Easy access to Lua objects like tables and functions.
* Interoperable with most common c++ standard library container types.
* Written in a clear and easy to debug style.

## Improvements Over Vanilla LuaBridge

LuaBridge3 offers a set of improvements compared to vanilla LuaBridge:

* Can work with both c++ exceptions and without (Works with `-fno-exceptions` and `/EHsc-`).
* Added `std::shared_ptr` support for types intrusively deriving from `std::enable_shared_from_this`.
* Supports conversion to and from `std::nullptr_t`, `std::byte`, `std::tuple` and `std::reference_wrapper`.
* Transparent support of all signed and unsigned integer types up to `int64_t`.
* Automatic handling of enum types by communicating with lua through `std::underlying_type_t`.
* Support for converting to and from C style arrays of any supported type.
* Full support for capturing lambdas in all namespace and class methods.
* Allows creating class instances using non intrusive class factories instead of requiring public constructors.
* Lightweight object creation: allow adding lua tables on the stack and register methods and metamethods in them.
* Consistent numeric handling and conversions (signed, unsigned and floats) across all lua versions.
* Opt-in handling of safe stack space checks (automatically avoids exhausting lua stack space when pushing values!).
* The only binder library that works with both LuaJIT and Luau, wonderful for game development !

## Status

![Build MacOS](https://github.com/kunitoki/LuaBridge3/workflows/Build%20MacOS/badge.svg?branch=master)
![Build Windows](https://github.com/kunitoki/LuaBridge3/workflows/Build%20Windows/badge.svg?branch=master)
![Build Linux](https://github.com/kunitoki/LuaBridge3/workflows/Build%20Linux/badge.svg?branch=master)

## Code Coverage
[![Coverage Status](https://coveralls.io/repos/github/kunitoki/LuaBridge3/badge.svg?branch=master)](https://coveralls.io/github/kunitoki/LuaBridge3?branch=master)

## Documentation

Please read the [LuaBridge Reference Manual][6] for more details on the API.

## Release Notes

Plase read the [LuaBridge Release Notes][7] for more details

## Unit Tests

Unit test build requires a CMake and C++17 compliant compiler.

There are 9 unit test flavors:
* `LuaBridgeTests51` - uses Lua 5.1
* `LuaBridgeTests51Noexcept` - uses Lua 5.1 without exceptions enabled
* `LuaBridgeTests52` - uses Lua 5.2
* `LuaBridgeTests52Noexcept` - uses Lua 5.2 without exceptions enabled
* `LuaBridgeTests53` - uses Lua 5.3
* `LuaBridgeTests53Noexcept` - uses Lua 5.3 without exceptions enabled
* `LuaBridgeTests54` - uses Lua 5.4
* `LuaBridgeTests54Noexcept` - uses Lua 5.4 without exceptions enabled
* `LuaBridgeTestsLuau` - uses Luau

(Luau compiler needs exceptions, so there is no tests that runs on Luau without exceptions)

Generate Unix Makefiles and build on Linux:
```bash
git clone --recursive git@github.com:kunitoki/LuaBridge3.git

mkdir -p LuaBridge/build
pushd LuaBridge/build
cmake -G "Unix Makefiles" ../
cmake --build . -DCMAKE_BUILD_TYPE=Debug
# or cmake --build . -DCMAKE_BUILD_TYPE=Release
# or cmake --build . -DCMAKE_BUILD_TYPE=RelWithDebInfo
popd
```

Generate XCode project and build on MacOS:
```bash
git clone --recursive git@github.com:kunitoki/LuaBridge3.git

mkdir -p LuaBridge/build
pushd LuaBridge/build
cmake -G Xcode ../ # Generates XCode project build/LuaBridge.xcodeproj
cmake --build . -DCMAKE_BUILD_TYPE=Debug
# or cmake --build . -DCMAKE_BUILD_TYPE=Release
# or cmake --build . -DCMAKE_BUILD_TYPE=RelWithDebInfo
popd
```

Generate VS2019 solution on Windows:
```cmd
git clone --recursive git@github.com:kunitoki/LuaBridge3.git

mkdir LuaBridge/build
pushd LuaBridge/build
cmake -G "Visual Studio 16" ../ # Generates MSVS solution build/LuaBridge.sln
popd
```

## Official Repository

LuaBridge is published under the terms of the [MIT License][5].

The original version of LuaBridge was written by Nathan Reed. The project has
been taken over by Vinnie Falco, who added new functionality, wrote the new
documentation, and incorporated contributions from Nigel Atkinson. Then it has
been forked from the original https://github.com/vinniefalco/LuaBridge into its
own LuaBridge3 repository by Lucio Asnaghi, and development continued there.

For questions, comments, or bug reports feel free to open a Github issue
or contact Lucio Asnaghi directly at the email address indicated below.

Copyright 2020, Lucio Asnaghi (<kunitoki@gmail.com>)<br>
Copyright 2019, Dmitry Tarakanov<br>
Copyright 2012, Vinnie Falco (<vinnie.falco@gmail.com>)<br>
Copyright 2008, Nigel Atkinson<br>
Copyright 2007, Nathan Reed<br>

[1]:  https://github.com/kunitoki/LuaBridge3 "LuaBridge"
[2]:  https://lua.org "The Lua Programming Language"
[3]:  https://luajit.org/ "The LuaJIT Project"
[4]:  https://luau-lang.org/ "The Luau Project"
[5]:  https://www.opensource.org/licenses/mit-license.html "The MIT License"
[6]:  https://kunitoki.github.io/LuaBridge3/Manual "LuaBridge Reference Manual"
[7]:  https://kunitoki.github.io/LuaBridge3/CHANGES "LuaBridge Release Notes"
