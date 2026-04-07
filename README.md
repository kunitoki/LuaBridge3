<p float="left">
  <a href="https://kunitoki.github.io/LuaBridge3">
    <picture>
      <source media="(prefers-color-scheme: dark)" srcset="./Images/logo-dark.png">
      <img height="118" src="./Images/logo-bright.png">
    </picture>
  </a>
  &nbsp;&nbsp;&nbsp;&nbsp;
  <a href="https://lua.org">
    <picture>
      <source media="(prefers-color-scheme: dark)" srcset="./Images/lua-dark.png">
      <img height="118" src="./Images/lua-bright.png">
    </picture>
  </a>
</p>

[![Coverage Status](https://coveralls.io/repos/github/kunitoki/LuaBridge3/badge.svg?branch=master&kill_cache=1)](https://coveralls.io/github/kunitoki/LuaBridge3?branch=master)
[![MacOS](https://github.com/kunitoki/LuaBridge3/actions/workflows/build_macos.yml/badge.svg?branch=master)](https://github.com/kunitoki/LuaBridge3/actions/workflows/build_macos.yml)
[![Windows](https://github.com/kunitoki/LuaBridge3/actions/workflows/build_windows.yml/badge.svg?branch=master)](https://github.com/kunitoki/LuaBridge3/actions/workflows/build_windows.yml)
[![Linux](https://github.com/kunitoki/LuaBridge3/actions/workflows/build_linux.yml/badge.svg?branch=master)](https://github.com/kunitoki/LuaBridge3/actions/workflows/build_linux.yml)
[![UBSAN](https://github.com/kunitoki/LuaBridge3/actions/workflows/build_ubsan.yml/badge.svg?branch=master)](https://github.com/kunitoki/LuaBridge3/actions/workflows/build_ubsan.yml)
[![ASAN](https://github.com/kunitoki/LuaBridge3/actions/workflows/build_asan.yml/badge.svg?branch=master)](https://github.com/kunitoki/LuaBridge3/actions/workflows/build_asan.yml)
<br/>
[![Lua 5.1](https://img.shields.io/badge/lua-5.1-red)](https://www.lua.org/manual/5.1/readme.html)
[![Lua 5.2](https://img.shields.io/badge/lua-5.2-red)](https://www.lua.org/manual/5.2/readme.html)
[![Lua 5.3](https://img.shields.io/badge/lua-5.3-red)](https://www.lua.org/manual/5.3/readme.html)
[![Lua 5.4](https://img.shields.io/badge/lua-5.4-red)](https://www.lua.org/manual/5.4/readme.html)
[![Lua 5.5](https://img.shields.io/badge/lua-5.5-red)](https://www.lua.org/manual/5.5/readme.html)
[![LuaJIT](https://img.shields.io/badge/lua-LuaJIT-red)](https://luajit.org/luajit.html)
[![Luau](https://img.shields.io/badge/lua-Luau-red)](https://luau.org/)
[![Ravi](https://img.shields.io/badge/lua-Ravi-red)](https://ravilang.github.io/)

# LuaBridge 3.0

[LuaBridge3](https://github.com/kunitoki/LuaBridge3) is a lightweight and dependency-free library for mapping data,
functions, and classes back and forth between C++ and [Lua](http://wwww.lua.org) (a powerful,
fast, lightweight, embeddable scripting language). LuaBridge has been tested
and works with:
* [PUC-Lua](https://lua.org) 5.1.5, 5.2.4, 5.3.6, 5.4.8 and 5.5.0
* [LuaJit](https://luajit.org/) 2.1
* [Luau](https://luau-lang.org/) 0.713
* [Ravi](https://github.com/dibyendumajumdar/ravi) 1.0-beta11

## Features

LuaBridge3 is usable from a compliant C++17 compiler and offers the following features:

* [MIT Licensed](https://www.opensource.org/licenses/mit-license.html), no usage restrictions!
* Headers-only: No Makefile, no .cpp files, just one `#include` and one header file (optional) !
* Works with ANY lua version out there (PUC-Lua, LuaJIT, Luau, Ravi, you name it).
* Simple, light, and nothing else needed.
* Competitive performance with the fastest C++/Lua binding libraries available.
* Fast to compile (even in release mode), scaling linearly with the size of your binded code.
* No macros, settings, or configuration scripts needed.
* Supports different object lifetime management models.
* Convenient, type-safe access to the Lua stack.
* Automatic function parameter type binding.
* Functions and constructors overloading support.
* Easy access to Lua objects like tables and functions.
* Expose C++ classes allowing them to use the flexibility of lua property lookup.
* Interoperable with most common c++ standard library container types.
* Written in a clear and easy to debug style.

## Performance

LuaBridge3 has been heavily optimized and now competes directly with [sol2](https://github.com/ThePhD/sol2) — one of the fastest C++/Lua binding libraries — across most workloads.

![Benchmarks](./Images/benchmarks.png)

## Improvements Over Vanilla LuaBridge

LuaBridge3 offers a set of improvements compared to vanilla LuaBridge:

* The only binder library that works with PUC-Lua as well as LuaJIT, Luau and Ravi, wonderful for game development !
* Faster runtime execution for most common use cases, playing in the same league as the fastest binders in town.
* Can work with both c++ exceptions and without (Works with `-fno-exceptions` and `/EHsc-`).
* Full support for capturing lambdas in all namespace and class methods.
* Overloaded functions support in Namespace functions, Class constructors, functions and static functions.
* Multiple inheritance: `deriveClass<D, A, B, ...>` supports any number of registered base classes.
* Supports placement allocation or custom allocations/deallocations of C++ classes exposed to lua.
* Lightweight object creation: allow adding lua tables on the stack and register methods and metamethods in them.
* Instance metamethods fallbacks via `__index` and `__newindex` in exposed C++ classes.
* Static metamethod fallbacks via `__index` and `__newindex` in exposed C++ classes on the class static table.
* Custom destructor hook via `addDestructor` (`__destruct` metamethod) called just before the C++ destructor.
* Added `std::shared_ptr` to support shared C++/Lua lifetime for types deriving from `std::enable_shared_from_this`.
* Supports conversion to and from `std::nullptr_t`, `std::byte`, `std::pair`, `std::tuple` and `std::reference_wrapper`.
* Supports conversion to and from C style arrays of any supported type.
* `void*` and `const void*` are transparently mapped to Lua lightuserdata.
* Transparent support of all signed and unsigned integer types up to `int64_t`.
* Consistent numeric handling and conversions (signed, unsigned and floats) across all lua versions.
* NaN and Inf values pass through floating-point stack conversions without error.
* Simplified registration of enum types via the `luabridge::Enum` stack wrapper.
* C++20 coroutine integration via `addCoroutine()` and `CppCoroutine<R>`; await Lua threads from C++ with `LuaCoroutine`.
* Opt-out handling of safe stack space checks (automatically avoids exhausting lua stack space when pushing values!).
* Optional strict stack conversions via `LUABRIDGE_STRICT_STACK_CONVERSIONS` (e.g. `bool` requires an actual boolean, not any truthy value).
* Error handler support in Lua calls via `LuaRef::callWithHandler` and `luabridge::callWithHandler`.
* `newFunction` free function wraps any C++ callable into a Lua function exposed as a `LuaRef`.
* `LuaFunction<Signature>` provides a strongly-typed callable wrapper around a Lua function.
* `TypeResult<T>::valueOr(default)` allows safe value extraction with an explicit fallback.
* Can safely register and use classes exposed across shared library boundaries.

## Documentation

Please read the [LuaBridge3 Reference Manual](https://kunitoki.github.io/LuaBridge3/Manual) for more details on the API.

## Release Notes

Plase read the [LuaBridge3 Release Notes](https://kunitoki.github.io/LuaBridge3/CHANGES) for more details

## Installing LuaBridge3 (vcpkg)

You can download and install LuaBridge3 using the [vcpkg](https://github.com/Microsoft/vcpkg) dependency manager:
```Powershell or bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh # The name of the script should be "./bootstrap-vcpkg.bat" for Powershell
./vcpkg integrate install
./vcpkg install luabridge3
```

The LuaBridge3 port in vcpkg is kept up to date by Microsoft team members and community contributors. If the version is out of date, please [create an issue or pull request](https://github.com/Microsoft/vcpkg) on the vcpkg repository.

### Update vcpkg

To update the vcpkg port, we need to know the hash of the commit and the sha512 of its downloaded artifact.
Starting from the commit hash that needs to be published, download the archived artifact and get the sha512 of it:

```bash
COMMIT_HASH=$(git rev-parse HEAD)

wget https://github.com/kunitoki/LuaBridge3/archive/${COMMIT_HASH}.tar.gz

shasum -a 512 ${COMMIT_HASH}.tar.gz
# fbdf09e3bd0d4e55c27afa314ff231537b57653b7c3d96b51eac2a41de0c302ed093500298f341cb168695bae5d3094fb67e019e93620c11c7d6f8c86d3802e2 0e17140276d215e98764813078f48731125e4784.tar.gz
```
Now update the version in https://github.com/microsoft/vcpkg/blob/master/ports/luabridge3/vcpkg.json and the commit hash and sha512 in https://github.com/microsoft/vcpkg/blob/master/ports/luabridge3/portfile.cmake then commit the changes.
Enter into vcpkg folder and issue:

```bash
./vcpkg x-add-version --all
```

Commit the changed files and create a Pull Request for vcpkg.

## Unit Tests

Unit test build requires a CMake and C++17 compliant compiler.

There are 14 unit test flavors:
* `LuaBridgeTests51` - uses Lua 5.1
* `LuaBridgeTests51Noexcept` - uses Lua 5.1 without exceptions enabled
* `LuaBridgeTests52` - uses Lua 5.2
* `LuaBridgeTests52Noexcept` - uses Lua 5.2 without exceptions enabled
* `LuaBridgeTests53` - uses Lua 5.3
* `LuaBridgeTests53Noexcept` - uses Lua 5.3 without exceptions enabled
* `LuaBridgeTests54` - uses Lua 5.4
* `LuaBridgeTests54Noexcept` - uses Lua 5.4 without exceptions enabled
* `LuaBridgeTests55` - uses Lua 5.5
* `LuaBridgeTests55Noexcept` - uses Lua 5.5 without exceptions enabled
* `LuaBridgeTestsLuaJIT` - uses LuaJIT 2.1
* `LuaBridgeTestsLuaJITNoexcept` - uses LuaJIT 2.1 without exceptions enabled
* `LuaBridgeTestsLuau` - uses Luau
* `LuaBridgeTestsRavi` - uses Ravi

(Luau compiler needs exceptions, so there are no test targets on Luau without exceptions)
(Ravi doesn't fully work without exceptions, so there are no test targets on Ravi without exceptions)

Generate Unix Makefiles and build on Linux:
```bash
git clone --recursive git@github.com:kunitoki/LuaBridge3.git

mkdir -p LuaBridge3/build
pushd LuaBridge3/build
cmake -G "Unix Makefiles" ../
cmake --build . -DCMAKE_BUILD_TYPE=Debug
# or cmake --build . -DCMAKE_BUILD_TYPE=Release
# or cmake --build . -DCMAKE_BUILD_TYPE=RelWithDebInfo
popd
```

Generate XCode project and build on MacOS:
```bash
git clone --recursive git@github.com:kunitoki/LuaBridge3.git

mkdir -p LuaBridge3/build
pushd LuaBridge3/build
cmake -G Xcode ../ # Generates XCode project build/LuaBridge.xcodeproj
cmake --build . -DCMAKE_BUILD_TYPE=Debug
# or cmake --build . -DCMAKE_BUILD_TYPE=Release
# or cmake --build . -DCMAKE_BUILD_TYPE=RelWithDebInfo
popd
```

Generate VS2019 solution on Windows:
```cmd
git clone --recursive git@github.com:kunitoki/LuaBridge3.git

mkdir LuaBridge3/build
pushd LuaBridge3/build
cmake -G "Visual Studio 16" ../ # Generates MSVS solution build/LuaBridge.sln
popd
```

## Official Repository

LuaBridge3 is published under the terms of the [MIT License](https://www.opensource.org/licenses/mit-license.html).

The original version of LuaBridge3 was written by Nathan Reed. The project has
been taken over by Vinnie Falco, who added new functionality, wrote the new
documentation, and incorporated contributions from Nigel Atkinson. Then it has
been forked from the original https://github.com/vinniefalco/LuaBridge into its
own LuaBridge3 repository by kunitoki, and development continued there.

For questions, comments, or bug reports feel free to open a Github issue
or contact kunitoki directly at the email address indicated below.

Copyright 2020, kunitoki (<kunitoki@gmail.com>)<br>
Copyright 2019, Dmitry Tarakanov<br>
Copyright 2012, Vinnie Falco (<vinnie.falco@gmail.com>)<br>
Copyright 2008, Nigel Atkinson<br>
Copyright 2007, Nathan Reed<br>
