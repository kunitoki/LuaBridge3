* * *

LuaBridge 3.0 Reference Manual
==============================

* * *

LuaBridge3 repository is located at [https://github.com/kunitoki/LuaBridge3](https://github.com/kunitoki/LuaBridge3).
Official LuaBridge (up to version 2) repository is located at [https://github.com/vinniefalco/LuaBridge](https://github.com/vinniefalco/LuaBridge).

*   Copyright © 2020 kunitoki.
*   Copyright © 2019 Dmitry Tarakanov.
*   Copyright © 2012 Vinnie Falco.
*   Copyright © 2007 Nathan Reed.

Freely available under the terms of the [MIT License](http://www.opensource.org/licenses/mit-license.html).

Contents
--------

*   [1 - Introduction](#1---introduction)

    *   [1.1 - Design](#11---design)
    *   [1.2 - Repository](#12---repository)
    *   [1.3 - License and Credits](#13---license-and-credits)

*   [2 - Accessing C++ from Lua](#2---accessing-c-from-lua)

    *   [2.1 - Namespaces](#21---namespaces)
    *   [2.2 - Properties and Functions](#22---properties-and-functions)
    *   [2.3 - Class Objects](#23---class-objects)
        *   [2.3.1 - Multiple Inheritance](#231---multiple-inheritance)
    *   [2.4 - Property Member Proxies](#24---property-member-proxies)
    *   [2.5 - Function Member Proxies](#25---function-member-proxies)
    *   [2.5.1 - Function Overloading](#251---function-overloading)
    *   [2.6 - Constructors](#26---constructors)
    *   [2.6.1 - Constructor Proxies](#261---constructor-proxies)
    *   [2.6.2 - Constructor Factories](#262---constructor-factories)
    *   [2.6.3 - Destructors](#263---destructors)
    *   [2.7 - Extending Classes](#27---extending-classes)
        *   [2.7.1 - Extensible Classes](#271---extensible-classes)
        *   [2.7.2 - Index and New Index Metamethods Fallback](#272---index-and-new-index-metamethods-fallback)
        *   [2.7.3 - Static Index and New Index Metamethods Fallback](#273---static-index-and-new-index-metamethods-fallback)
    *   [2.8 - Lua Stack](#28---lua-stack)
        *   [2.8.1 - Enums](#281---enums)
        *   [2.8.2 - lua_State](#282---lua_state)
    *   [2.9 - C++20 Coroutine Integration](#29---c20-coroutine-integration)
        *   [2.9.1 - CppCoroutine\<R\> - Generators callable from Lua](#291---cppcoroutiner----generators-callable-from-lua)
        *   [2.9.2 - Accepting Arguments](#292---accepting-arguments)
        *   [2.9.3 - Class Coroutines - Static and Member](#293---class-coroutines----static-and-member)
        *   [2.9.4 - LuaCoroutine - Awaiting a Lua Thread from C++](#294---luacoroutine----awaiting-a-lua-thread-from-c)
        *   [2.9.5 - Limitations](#295---limitations)

*   [3 - Passing Objects](#3---passing-objects)

    *   [3.1 - C++ Lifetime](#31---c-lifetime)
    *   [3.2 - Lua Lifetime](#32---lua-lifetime)
    *   [3.3 - Pointers, References, and Pass by Value](#33---pointers-references-and-pass-by-value)
    *   [3.4 - Shared Lifetime](#34---shared-lifetime)
        *   [3.4.1 - User-defined Containers](#341---user-defined-containers)
        *   [3.4.2 - shared_ptr As Container](#342---shared_ptr-as-container)
        *   [3.4.3 - Container Constructors](#343---container-constructors)
    *   [3.5 - Mixing Lifetimes](#35---mixing-lifetimes)
    *   [3.6 - Convenience Functions](#36---convenience-functions)

*   [4 - Accessing Lua from C++](#4---accessing-lua-from-c)

    *   [4.1 - Class LuaRef](#41---class-luaref)
        *   [4.1.1 - Lifetime, States and Lua Threads](#411---lifetime-states-and-lua-threads)
        *   [4.1.2 - Type Conversions](#412---type-conversions)
    *   [4.2 - Table Proxies](#42---table-proxies)
    *   [4.3 - Calling Lua](#43---calling-lua)
        *   [4.3.1 - Exceptions](#431---exceptions)
        *   [4.3.2 - Class LuaException](#432---class-luaexception)
        *   [4.3.3 - Calling with Error Handlers](#433---calling-with-error-handlers)
    *   [4.4 - Wrapping C++ Callables](#44---wrapping-c-callables)

*   [5 - Security](#5---security)

*   [6 - Configuration](#6---configuration)

    * [6.1 - LUABRIDGE_SAFE_STACK_CHECKS](#61---luabridge-safe-stack-checks)
    * [6.2 - LUABRIDGE_STRICT_STACK_CONVERSIONS](#62---luabridge-strict-stack-conversions)
    * [6.3 - LUABRIDGE_SAFE_LUA_C_EXCEPTION_HANDLING](#63---luabridge-safe-c-exception-handling)
    * [6.4 - LUABRIDGE_RAISE_UNREGISTERED_CLASS_USAGE](#64---luabridge-raise-unregistered-class-usage)
    * [6.5 - LUABRIDGE_HAS_CXX20_COROUTINES / LUABRIDGE_DISABLE_CXX20_COROUTINES](#65---luabridge-has-cxx20-coroutines--luabridge-disable-cxx20-coroutines)

*   [Appendix - API Reference](#appendix---api-reference)

1 - Introduction
================

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
* Transparent support of all signed and unsigned integer types up to `int64_t`.
* Consistent numeric handling and conversions (signed, unsigned and floats) across all lua versions.
* Simplified registration of enum types via the `luabridge::Enum` stack wrapper.
* Opt-out handling of safe stack space checks (automatically avoids exhausting lua stack space when pushing values!).

LuaBridge is distributed as a a collection of header files. You simply add one line, `#include <LuaBridge/LuaBridge.h>` where you want to pass functions, classes, and variables back and forth between C++ and Lua. There are no additional source files, no compilation settings, and no Makefiles or IDE-specific project files. LuaBridge is easy to integrate.

C++ concepts like variables and classes are made available to Lua through a process called _registration_. Because Lua is weakly typed, the resulting structure is not rigid. The API is based on C++ template metaprogramming. It contains template code to automatically generate at compile-time the various Lua C API calls necessary to export your program's classes and functions to the Lua environment.

To expose Lua objects to C++, a class called `luabridge::LuaRef` is provided. The implementation allows C++ code to access Lua objects such as numbers or strings, but more importantly to access things like tables and their values. Using this class makes idioms like calling Lua functions simple and clean.

1.1 - Design
------------

LuaBridge tries to be efficient as possible when creating the "glue" that exposes C++ data and functions to Lua. At the same time, the code was written with the intention that it is all as simple and clear as possible, without resorting to obscure C++ idioms, ugly preprocessor macros, or configuration settings. Furthermore, it is designed to be "header-only", making it very easy to integrate into your projects.

Because LuaBridge was written with simplicity in mind there are some features that are not available. LuaBridge3 has been extensively optimized and now competes directly with [sol2](https://github.com/ThePhD/sol2) - one of the fastest C++/Lua binding libraries - across most workloads. In fact, LuaBridge3 outperforms sol2 in certain scenarios such as member function calls from Lua and global table writes, while remaining more compact and faster to compile. While sol2 has an edge in chained table access, the overall performance gap is negligible for most use cases. LuaBridge3 also does not try to implement every possible feature: [LuaBind](http://www.rasterbar.com/products/luabind.html) (requires Boost) and [sol2](https://github.com/ThePhD/sol2) explore every corner of the C++ language.

LuaBridge does not support:

*   Global types (types must be registered in a named scope).
*   Automatic conversion between STL container types and Lua tables (but conversion can be enabled for `std::array`, `std::vector`, `std::map`, `std::unordered_map`, `std::set` `std::list`, `std::optional`, by including `LuaBridge/Array.h`, `LuaBridge/Vector.h`, `LuaBridge/Map`, `LuaBridge/UnorderedMap.h`, `LuaBridge/Set.h`, `LuaBridge/List.h`, `LuaBridge/Optional.h` respectively)
*   Inheriting Lua classes from C++ classes.
*   Passing nil to a C++ function that expects a pointer or reference.

1.2 - Repository
----------------

The official repository is located at [https://github.com/kunitoki/LuaBridge3](https://github.com/kunitoki/LuaBridge3).

The **master** branch contains published library versions. Release versions are marked with tags.

1.3 - License and Credits
-------------------------

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

2 - Accessing C++ from Lua
==========================

In order to expose C++ data and functions to Lua, each piece of exported information must be _registered_. There are five types of objects that LuaBridge can register:

**Namespaces**  

A Lua table that contains other registrations.

**Data**  

Global or static variables, data members, and static data members.

**Functions**

Regular functions, member functions, and static member functions.

**CFunctions**

A regular function, member function, or static member function that uses the `lua_CFunction` calling convention.

**Properties**

Global properties, property members, and static property members. These appear like data to Lua, but are implemented in C++ using functions to get and set the values.

Both data and properties can be marked as _read-only_ at the time of registration. This is different from `const`; the values of these objects can be modified on the C++ side, but Lua scripts cannot change them. Code samples that follow are in C++ or Lua, depending on context. For brevity of exposition code samples in C++ assume the traditional variable `lua_State* L` is defined.

2.1 - Namespaces
----------------

All LuaBridge registrations take place in a _namespace_. When we refer to a _namespace_ we are always talking about a namespace in the Lua sense, which is implemented using tables. The namespace need not correspond to a C++ namespace; in fact no C++ namespaces need to exist at all unless you want them to. LuaBridge namespaces are visible only to Lua scripts; they are used as a logical grouping tool. To obtain access to the global namespace we write:

```cpp
luabridge::getGlobalNamespace (L);
```

This returns an object on which further registrations can be performed. The subsequent registrations will go into the global namespace, a practice which is not recommended. Instead, we can add our own namespace by writing:

```cpp
luabridge::getGlobalNamespace (L)
  .beginNamespace ("test");
```

This creates a table in `_G` called "test". Since we have not performed any registrations, this table will be empty except for some bookkeeping key/value pairs. LuaBridge reserves all identifiers that start with a double underscore. So `__test` would be an invalid name (although LuaBridge will silently accept it). Functions like `beginNamespace` return the corresponding object on which we can make more registrations. Given:

```cpp
luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginNamespace ("detail")
    .endNamespace ()
    .beginNamespace ("utility")
    .endNamespace ()
  .endNamespace ();
```

The results are accessible to Lua as `test`, `test.detail`, and `test.utility`. Here we introduce the `endNamespace` function; it returns an object representing the original enclosing namespace. All LuaBridge functions which create registrations return an object upon which subsequent registrations can be made, allowing for an unlimited number of registrations to be chained together using the dot operator. Adding two objects with the same name, in the same namespace, results in undefined behavior (although LuaBridge will silently accept it).

A namespace can be re-opened later to add more functions. This lets you split up the registration between different source files. These are equivalent:

```cpp
luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .addFunction ("foo", foo)
  .endNamespace ();

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .addFunction ("bar", bar)
  .endNamespace ();
```

and

```cpp
luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .addFunction ("foo", foo)
    .addFunction ("bar", bar)
  .endNamespace ();
```

It's also possible to obtain a namespace from a table on the stack, and perform the registration of sub namespaces, properties, functions and classes into that table.

```cpp
lua_newtable (L);

luabridge::getNamespaceFromStack (L)
  .addFunction ("test", +[] (int x) { return x; })
  .addFunction ("bar", &bar);
```

The table is still on top of the stack here and has not been popped, so it's possible to further manipulate it or eventually use it as environment for closures.

2.2 - Properties and Functions
------------------------------

These are registered into a namespace using `addProperty` and `addFunction`. When registered functions are called by scripts, LuaBridge automatically takes care of the conversion of arguments into the appropriate data type when doing so is possible. This automated system works for the function's return value, and up to 8 parameters although more can be added by extending the templates. Pointers, references, and objects of class type as parameters are treated specially, and explained later.

If we have:

```cpp
int globalVar;
static float staticVar;

std::string stringProperty;
std::string getString () { return stringProperty; }
void setString (std::string s) { stringProperty = s; }

std::tuple <int, std::string> tuple;

int foo () { return 42; }
void bar (char const*) { }
int cFunc (lua_State* L) { return 0; }
```

These are registered with:

```cpp
luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .addProperty ("var1", &globalVar) // read-only
    .addProperty ("var2", &staticVar, &staticVar) // read-write
    .addProperty ("prop1", getString) // read-only
    .addProperty ("prop2", getString, setString) // read-write
    .addProperty ("tup1", &tuple) // read-only
    .addProperty ("tup2", &tuple, &tuple) // read-write
    .addFunction ("foo", foo)
    .addFunction ("bar", bar)
    .addFunction ("cfunc", cFunc)
  .endNamespace ();
```

Variables can be marked _read-only_ by passing `false` in the second optional parameter. If the parameter is omitted, _true_ is used making the variable read/write. Properties are marked read-only by omitting the set function. After the registrations above, the following Lua identifiers are valid:

```lua
test        -- a namespace
test.var1   -- a read-only lua_Number property
test.var2   -- a read-write lua_Number property
test.prop1  -- a read-only lua_String property
test.prop2  -- a read-write lua_String property
test.tup1   -- a read-only lua_Table property mapping to a c++ tuple
test.tup2   -- a read-write lua_Table property mapping to a c++ tuple
test.foo    -- a function returning a lua_Number
test.bar    -- a function taking a lua_String as a parameter
test.cfunc  -- a function with a variable argument list and multi-return
```

Note that `test.prop1` and `test.prop2` both refer to the same value. However, since `test.prop2` is read-only, assignment attempts will generate a run-time error. These Lua statements have the stated effects:

```lua
test.var1 = 5          -- error: var1 is not writable
test.var2 = 6          -- okay
test.prop1 = "bar"     -- error: prop1 is not writable
test.prop2 = "Hello"   -- okay
test.prop2 = 68        -- okay, Lua converts the number to a string
test.tup1 = { 1, "a" } -- error: tup1 is not writable
test.tup2 = { 1, "a" } -- okay, converts a table to tuple with the same size
test.tup2 = { "size" } -- error: table has different size than tuple

test.foo ()            -- calls foo and discards the return value
test.var1 = foo ()     -- calls foo and stores the result in var1
test.bar ("Employee")  -- calls bar with a string
test.bar (test)        -- error: bar expects a string not a table
```

2.3 - Class Objects
-------------------

A class registration is opened using either `beginClass` or `deriveClass` and ended using `endClass`. Once registered, a class can later be re-opened for more registrations using `beginClass`. However, `deriveClass` should only be used once. To add more registrations to an already registered derived class, use `beginClass` on it.

These declarations:

```cpp
struct A {
  static int staticData;
  static float staticProperty;

  static float getStaticProperty () { return staticProperty; }
  static void setStaticProperty (float f) { staticProperty = f; }
  static void staticFunc () { }

  static int staticCFunc (lua_State *L) { return 0; }

  std::string dataMember;

  char dataProperty;
  char getProperty () const { return dataProperty; }
  void setProperty (char v) { dataProperty = v; }
  std::string toString () const { return dataMember; }

  void func1 () { }
  virtual void virtualFunc () { }

  int cfunc (lua_State* L) { return 0; }
};

struct B : public A {
  double dataMember2;

  void func1 () { }
  void func2 () { }
  void virtualFunc () { }
};

int A::staticData;
float A::staticProperty;
```

are registered using:

```cpp
luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<A> ("A")
      .addStaticProperty ("staticData", &A::staticData, &A::staticData)
      .addStaticProperty ("staticProperty", &A::getStaticProperty, &A::setStaticProperty)
      .addStaticFunction ("staticFunc", &A::staticFunc)
      .addStaticFunction ("staticCFunc", &A::staticCFunc)
      .addProperty ("data", &A::dataMember, &A::dataMember)
      .addProperty ("prop", &A::getProperty, &A::setProperty)
      .addFunction ("func1", &A::func1)
      .addFunction ("virtualFunc", &A::virtualFunc)
      .addFunction ("__tostring", &A::toString)     // Metamethod
      .addFunction ("cfunc", &A::cfunc)
    .endClass ()
    .deriveClass<B, A> ("B")
      .addProperty ("data", &B::dataMember2, &B::dataMember2)
      .addFunction ("func1", &B::func1)
      .addFunction ("func2", &B::func2)
    .endClass ()
  .endNameSpace ();
```

Method registration works just like function registration. Virtual methods work normally; no special syntax is needed. const methods are detected and const-correctness is enforced, so if a function returns a const object (or a container holding to a const object) to Lua, that reference to the object will be considered const and only const methods can be called on it. It is possible to register Lua metamethods (except `__gc`). Destructors are registered automatically for each class.

As with regular variables and properties, class properties can be marked read-only by passing false in the second parameter, or omitting the set function. The `deriveClass` takes a derived class and one or more registered base classes as template arguments. Inherited methods do not have to be re-declared and will function normally in Lua. If a class has a base class that is **not** registered with Lua, there is no need to declare it as a subclass.

When registering a data member pointer as both readable and writable, the common pattern of passing the same pointer twice to `addProperty` or `addStaticProperty` can be replaced with the convenience methods `addPropertyReadWrite` and `addStaticPropertyReadWrite`:

```cpp
luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<A> ("A")
      .addStaticPropertyReadWrite ("staticData", &A::staticData)  // equivalent to addStaticProperty ("staticData", &A::staticData, &A::staticData)
      .addPropertyReadWrite ("data", &A::dataMember)              // equivalent to addProperty ("data", &A::dataMember, &A::dataMember)
    .endClass ()
  .endNamespace ();
```

These accept only a pointer-to-data-member (for `addPropertyReadWrite`) or a pointer to a static data member (for `addStaticPropertyReadWrite`). They do not accept getter/setter function pairs; use `addProperty` or `addStaticProperty` directly for those cases.

Remember that in Lua, the colon operator '`:`' is used for method call syntax:

```lua
local a = A ()

a.func1 ()  -- error: func1 expects an object of a registered class
a.func1 (a) -- okay, verbose, this how OOP works in Lua
a:func1 ()  -- okay, less verbose, equivalent to the previous
```

### 2.3.1 - Multiple Inheritance

`deriveClass` supports multiple registered base classes by specifying them as additional template parameters. LuaBridge will traverse all base class hierarchies when resolving member lookups from Lua:

```cpp
struct A
{
  void funcA () { }
};

struct B
{
  void funcB () { }
};

struct C : public A, public B
{
  void funcC () { }
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<A> ("A")
      .addFunction ("funcA", &A::funcA)
    .endClass ()
    .beginClass<B> ("B")
      .addFunction ("funcB", &B::funcB)
    .endClass ()
    .deriveClass<C, A, B> ("C")
      .addFunction ("funcC", &C::funcC)
    .endClass ()
  .endNamespace ();
```

From Lua, all inherited methods are accessible on instances of `C`:

```lua
local c = test.C ()
c:funcA ()  -- calls A::funcA via multiple-inheritance lookup
c:funcB ()  -- calls B::funcB via multiple-inheritance lookup
c:funcC ()  -- calls C::funcC
```

Only base classes that are themselves registered with LuaBridge need to be listed as template parameters. If a base class is not registered, it can be omitted from `deriveClass`.

2.4 - Property Member Proxies
-----------------------------

Sometimes when registering a class which comes from a third party library, the data is not exposed in a way that can be expressed as a pointer to member, there are no get or set functions, or the get and set functions do not have the right function signature. Since the class declaration is closed for changes, LuaBridge allows for a _property member proxy_. This is a pair of get and set flat functions which take as their first parameter a pointer to the object. This is easily understood with the following example:

```cpp
// Third party declaration, can't be changed
struct Vec
{
  float coord [3];
};
```

Taking the address of an array element, e.g. `&Vec::coord [0]` results in an error instead of a pointer-to-member. The class is closed for modifications, but we want to export Vec objects to Lua using the familiar object notation. To do this, first we add a "helper" class:

```cpp
struct VecHelper
{
  template <unsigned index>
  static float get (Vec const* vec)
  {
    return vec->coord [index];
  }

  template <unsigned index>
  static void set (Vec* vec, float value)
  {
    vec->coord [index] = value;
  }
};
```

This helper class is only used to provide property member proxies. `Vec` continues to be used in the C++ code as it was before. Now we can register the `Vec` class with property member proxies for `x`, `y`, and `z`:

```cpp
luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<Vec> ("Vec")
      .addProperty ("x", &VecHelper::get<0>, &VecHelper::set<0>)
      .addProperty ("y", &VecHelper::get<1>, &VecHelper::set<1>)
      .addProperty ("z", &VecHelper::get<2>, &VecHelper::set<2>)
    .endClass ()
  .endNamespace ();
```

It is also possible to use both capturing and non capturing lambdas, as well as `std::function <>` instances as proxies:

```cpp
std::function<float (const Vec*)> get_x = [] (const Vec* vec) { return vec->coord [0]; };
std::function<void (Vec*, float)> set_x = [] (Vec* vec, float v) { vec->coord [0] = v; };

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<Vec> ("Vec")
      .addProperty ("x", get_x, set_x)
      // ... same for "y" and "z"
    .endClass ()
  .endNamespace ();
```

Or the more concise version (notice the `+` before the lambda is useful to convert a non capturing lambda to a function pointer in order to avoid allocating a std::function internally, where storing the lambda as function pointer might avoid lua usertype allocation overhead):

```cpp
luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<Vec> ("Vec")
      .addProperty ("x",
        +[] (const Vec* vec) { return vec->coord [0]; },
        +[] (Vec* vec, float v) { vec->coord [0] = v; })
      // ... same for "y" and "z"
    .endClass ()
  .endNamespace ();
```

2.5 - Function Member Proxies
-----------------------------

Where it is not possible or inconvenient to add a member to be registered, LuaBridge also allows for a _function member proxy_. This is a flat function which take as its first parameter a pointer to the object:

```cpp
// Third party declaration, can't be changed
struct Vec
{
  float coord [3];
};
```

The class is closed for modifications, but we want to extend Vec objects with our member function. To do this, first we add a "helper" function:

```cpp
void scale (Vec* vec, float value)
{
  vec->coord [0] *= value;
  vec->coord [1] *= value;
  vec->coord [2] *= value;
}
```

Now we can register the `Vec` class with a member function `scale`:

```cpp
luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<Vec> ("Vec")
      .addFunction ("scale", &scale)
    .endClass ()
  .endNamespace ();
```

It is also possible to use lambdas (both capturing and non capturing) as functions proxies:

```cpp
float y = atan (1.0f) * 4.0f;

luabridge::getGlobalNamespace (L)
  .beginClass<Vec> ("Vec")
    .addFunction ("scaleX", +[] (Vec* vec, float v) { vec->coord [0] *= v; })
    .addFunction ("scaleY", [y] (Vec* vec, float v) { vec->coord [1] *= v * y; })
  .endClass ()
```

Of course when not capturing, it is better to prefix the lambda with `+` so it is converted and stored internally to a function pointer instead of an `std::function`, so it is actually lighter to store and faster to call.

### 2.5.1 - Function Overloading

When specifying more than one method to the `addFunction` or `addStaticFunction` of both `Namespace` and `Class`, those overloads will be invoked in case of a call. Only overloads that have matched arguments arity will be considered, and they will be tried from first to last until the call succeeds.

```cpp
struct Vec { float coord [3]; };
struct Quat { float values [4]; };

void rotateByDegreees (Vec* vec, float degrees);
void rotateByQuaternion (Vec* vec, const Quat& quaternion);
```

Now we can register the `Vec` class with a member function `rotate` that will be resolving the call into the two provided overloads:

```cpp
luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<Vec> ("Vec")
      .addFunction ("rotate", &rotateByDegreees, &rotateByQuaternion)
    .endClass ()
  .endNamespace ();
```

In case of members (or functions) with the same name, it's necessary to use `luabridge::overload`, `luabridge::constOverload` or `luabridge::nonConstOverload` to disambiguate which of the functions needs to be registered:

```cpp
struct Quat { float values [4]; };

struct Vec
{
  void rotate (float degrees);
  void rotate (const Quat& quaternion);

  void x (float new_value);
  float x (float value_if_zero) const;

  float coord [3];
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<Vec> ("Vec")
      .addFunction ("rotate",
        luabridge::overload<float> (&Vec::rotate),
        luabridge::overload<const Quat&> (&Vec::rotate))
      .addFunction ("x",
        luabridge::nonConstOverload<float> (&Vec::x),
        luabridge::constOverload<float> (&Vec::x))
    .endClass ()
  .endNamespace ();

```

It's possible to mix lambdas, function pointers and member functions in overload creation. Providing a `lua_Cfunction` as last method will ensure it can be reached in case no other overload is successfully executed, kind of like a "catch all" method.

Special attention needs to be given to the order (priority) of the overloads, based on the number and type of the arguments. Better to place first the overloads that can be called more frequently, and putting "stronger" types first: for example when having an overload taking an `int` and an overload taking `float`, as lua is not able to distinguish between them properly (until lua 5.4) the first overload will always be called.

2.6 - Constructors
------------------

A single constructor may be added for a class using `addConstructor`. LuaBridge cannot automatically determine the number and types of constructor parameters like it can for functions and methods, so you must provide them. This is done by specifying the signature of the desired constructor functions as template parameters to `addConstructor`. The parameter types will be extracted from this (the return type is ignored). For example, these statements register constructors for the given classes:

```cpp
struct A
{
  A ();
  A (std::string_view a);
  A (std::string_view a, int b);
};

struct B
{
  explicit B (const char* s, int nChars);
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<A> ("A")
      .addConstructor<void (), void (std::string_view), void (std::string_view, int)> ()
    .endClass ()
    .beginClass<B> ("B")
      .addConstructor<void (const char*, int)> ()
    .endClass ()
  .endNamespace ();
```

Constructors added in this fashion are called from Lua using the fully qualified name of the class. This Lua code will create instances of `A` and `B`.

```lua
a = test.A ()           -- Create a new A.
b = test.B ("hello", 5) -- Create a new B.
b = test.B ()           -- Error: expected string in argument 1
```

### 2.6.1 - Constructor Proxies

Sometimes is not possible to use a constructor for a class, because some of the constructor arguments have types that couldn't be exposed to lua, or more control is needed when constructors need to be invoked (like checking the lau stack). So it is possible to workaround those limitations by using a special `addConstructor` that doesn't need any template specialiation, but takes only one or more functors, which will allow to placement new the c++ class in a c++ lambda, specifying any custom parameter there:

```cpp
struct NotExposed;
NotExposed* shouldNotSeeMe;

struct HardToCreate
{
  explicit HardToCreate (const NotExposed* x, int easy);
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<HardToCreate> ("HardToCreate")
      .addConstructor ([&shouldNotSeeMe] (void* ptr, int easy) { return new (ptr) HardToCreate (shouldNotSeeMe, easy); })
    .endClass ()
  .endNamespace ();
```

Then in lua:

```lua
hard = test.HardToCreate (5) -- Create a new HardToCreate.
```

The `addConstructor` overload taking a generic functor also accepts a `lua_State*` as last parameter in order to be used for constructors that needs to be overloaded by different numbers of arguments (arguments will start at index 2 of the stack):

```cpp
luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<HardToCreate> ("HardToCreate")
      .addConstructor ([] (void* ptr, lua_State* L) { return new (ptr) HardToCreate (shouldNotSeeMe, lua_checkinteger (L, 2)); })
    .endClass ()
  .endNamespace ();
```

As mentioned at the beginning, it's possible to specify multiple functors, that will be tried in order until the object can be constructed:

```cpp
struct NotExposed;
NotExposed* shouldNotSeeMe;

struct HardToCreate
{
  HardToCreate (const NotExposed* x, int easy);
  HardToCreate (const NotExposed* x, int easy, int lessEasy);
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<HardToCreate> ("HardToCreate")
      .addConstructor (
        [&shouldNotSeeMe] (void* ptr, int easy) { return new (ptr) HardToCreate (shouldNotSeeMe, easy); },
        [&shouldNotSeeMe] (void* ptr, int easy, int lessEasy) { return new (ptr) HardToCreate (shouldNotSeeMe, easy, lessEasy); })
    .endClass ()
  .endNamespace ();
```

### 2.6.2 - Constructor Factories

If granular control over allocation and deallocation of a type is needed, the `addFactory` method can be used to register both and allocator and deallocator of C++ type. This is useful in case of having classes being provided by factory methods from shared libraries.

```cpp
struct IObject
{
  virtual void overridableMethod () const = 0;
};

// These might be defined in a shared library, returning concrete types.
extern "C" IObject* objectFactoryAllocator ();
extern "C" void objectFactoryDeallocator (IObject*);

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<IObject> ("Object")
      .addFactory (&objectFactoryAllocator, &objectFactoryDeallocator)
      .addFunction ("overridableMethod", &IObject::overridableMethod)
    .endClass ()
  .endNamespace ();
```

The object is the perfectly instantiable through lua:

```lua
a = test.Object ()           -- Create a new Object using objectFactoryAllocator
a = nil                      -- Remove any reference count
collectgarbage ("collect")   -- The object is garbage collected using objectFactoryDeallocator
```

### 2.6.3 - Destructors

In addition to the automatic `__gc` metamethod that LuaBridge registers for every class (which calls the C++ destructor when Lua garbage-collects the userdata), you can register an extra hook that is called **just before** the destructor runs. This is useful for performing clean-up work in a context where the object is still fully valid.

Use `addDestructor` to register the hook. The callable must accept a `T*` (and optionally a trailing `lua_State*`):

```cpp
struct Resource
{
  ~Resource () { /* cleanup */ }
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<Resource> ("Resource")
      .addDestructor ([] (Resource* r) {
        // called before ~Resource()
        std::cout << "Resource about to be destroyed\n";
      })
    .endClass ()
  .endNamespace ();
```

The `__destruct` metamethod is invoked by Lua's garbage collector before `__gc` calls the C++ destructor. This metamethod is only available on non-Luau builds; Luau already calls `__gc` directly.

2.7 - Extending Classes
-----------------------

The LuaBridge library provides a set of features dedicated to extending classes from lua.

### 2.7.1 - Extensible Classes

The `luabridge::extensibleClass` option tells LuaBridge to create a metatable for the class that can be modified at runtime by Lua code. By doing so, it's then possible to extend the class from lua by adding custom instance methods and static methods to the type. Because those methods are stored in the metatable of the type, no additional storage is needed.

```cpp
struct ExtensibleClass
{
  int propertyOne = 42;
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<ExtensibleClass> ("ExtensibleClass", luabridge::extensibleClass)
      .addProperty ("propertyOne", &ExtensibleClass::propertyOne, &ExtensibleClass::propertyOne)
    .endClass ()
  .endNamespace ();

ExtensibleClass clazz;
luabridge::pushGlobal (L, &clazz, "clazz");
```

From lua is then possible to extend the class by registering methods on the type:

```lua
function ExtensibleClass:newInstanceMethod()
  return self.propertyOne * 2
end

function ExtensibleClass.newStaticMethod()
  return 1337
end

print (clazz:newMethod(), ExtensibleClass.newStaticMethod())
```

This way extending an already existing method will raise a lua error when trying to do so. To be able to override existing methods, pass `luabridge::allowOverridingMethods` together with `luabridge::extensibleClass`.

```cpp
struct ExtensibleClass
{
  int existingMethod() { return 42; }
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<ExtensibleClass> ("ExtensibleClass", luabridge::extensibleClass | luabridge::allowOverridingMethods)
      .addFunction ("existingMethod", &ExtensibleClass::existingMethod)
    .endClass ()
  .endNamespace ();

ExtensibleClass clazz;
luabridge::pushGlobal (L, &clazz, "clazz");
```

```lua
function ExtensibleClass:existingMethod()
  return "this has been replaced"
end

print (clazz:existingMethod())
```


In case storing instance properties is needed, storage needs to be provided per instance. See the next chapter for an explanation on how to add custom properties per instance.

### 2.7.2 - Index and New Index Metamethods Fallback

In general LuaBridge for each class will add a `__index` and `__newindex` metamethods in order to be able to handle member function, properties and inheritance resolution. This will make it impossible for a user to override them because in doing so, we'll make the exposed classes non functioning. Although possible to override those metamethods directly, they will preclude any possibility to locate exposed members and properties in such classes.

If a LuaBridge exposed class still need to handle the case of handling `__index` and `__newindex` metamethods calls, it's possible to use the `addIndexMetaMethod` and `addNewIndexMetaMethod` registration functions that will be executed as fallback in case an already existing function/property is not exposed in the class itself or any of its parent.

```cpp
struct FlexibleClass
{
  int propertyOne = 42;
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<FlexibleClass> ("FlexibleClass")
      .addProperty ("propertyOne", &FlexibleClass::propertyOne, &FlexibleClass::propertyOne)
      .addIndexMetaMethod ([] (FlexibleClass& self, const luabridge::LuaRef& key, lua_State* L)
      {
        if (key.tostring () == "existingProperty")
          return luabridge::LuaRef (L, 1337);

        return luabridge::LuaRef (L, luabridge::LuaNil ()); // or luaL_error("Failed lookup of key !")
      })
    .endClass ()
  .endNamespace ();

FlexibleClass flexi;
luabridge::pushGlobal (L, &flexi, "flexi");
```

Then in lua:

```lua
propertyOne = flexi.propertyOne
assert (propertyOne == 42, "Getting value from LuaBridge exposed property")

propertyTwo = flexi.existingProperty
assert (propertyTwo == 1337, "Getting value from non exposed LuaBridge property via __index fallback")

propertyThree = flexi.nonExistingProperty
assert (propertyThree == nil, "Getting value from non exposed LuaBridge property via __index fallback")
```

The same can be done for the `__newindex` metamethod fallback:

```cpp
struct FlexibleClass
{
  std::unordered_map<luabridge::LuaRef, luabridge::LuaRef> properties;
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<FlexibleClass> ("FlexibleClass")
      .addIndexMetaMethod ([] (FlexibleClass& self, const luabridge::LuaRef& key, lua_State* L)
      {
        auto it = self.properties.find(key);
        if (it != self.properties.end())
          return it->second;

        return luabridge::LuaRef (L, luabridge::LuaNil ()); // or luaL_error("Failed lookup of key !")
      })
      .addNewIndexMetaMethod ([] (FlexibleClass& self, const luabridge::LuaRef& key, const luabridge::LuaRef& value, lua_State* L)
      {
        self.properties.emplace (std::make_pair (key, value))
        return luabridge::LuaRef (L, luabridge::LuaNil ());
      })
    .endClass ()
  .endNamespace ();

FlexibleClass flexi;
luabridge::pushGlobal (L, &flexi, "flexi");
```

Then in lua:

```lua
propertyOne = flexi.propertyOne
assert (propertyOne == nil, "Value is not existing")

flexi.propertyOne = 1337

propertyOne = flexi.propertyOne
assert (propertyOne == 1337, "Value is now present !")
```

### 2.7.3 - Static Index and New Index Metamethods Fallback

The same fallback mechanism is available for the *static class table* - i.e. for key lookups performed on the class name itself (e.g. `MyClass.someKey`) rather than on an instance.  Use `addStaticIndexMetaMethod` and `addStaticNewIndexMetaMethod` to register the callbacks.  Unlike their instance counterparts, the static callbacks receive only the key (and optionally `lua_State*`) - there is no `self` parameter.

```cpp
struct MyClass {};

std::unordered_map<std::string, int> store;

luabridge::getGlobalNamespace (L)
  .beginClass<MyClass> ("MyClass")
    .addStaticIndexMetaMethod ([] (const luabridge::LuaRef& key, lua_State* L) -> luabridge::LuaRef
    {
      auto it = store.find (key.tostring ());
      if (it != store.end ())
        return luabridge::LuaRef (L, it->second);

      return luabridge::LuaRef (L, luabridge::LuaNil ());
    })
    .addStaticNewIndexMetaMethod ([] (const luabridge::LuaRef& key, const luabridge::LuaRef& value, lua_State* L) -> luabridge::LuaRef
    {
      if (value.isNumber ())
        store[key.tostring ()] = value.unsafe_cast<int> ();
      return value;
    })
  .endClass ();
```

Then in lua:

```lua
MyClass.dynamicProp = 42

value = MyClass.dynamicProp
assert (value == 42, "Value stored via static __newindex fallback and retrieved via static __index fallback")

missing = MyClass.nonExistingKey
assert (missing == nil, "Unknown key returns nil through the static __index fallback")
```

Existing static properties and functions registered with `addStaticProperty` / `addStaticFunction` are found *after* the fallback is consulted.  If the fallback callback returns `nil` (or a nil `LuaRef`) for a given key, normal lookup continues and the real property or function is returned.  Conversely, if the callback returns a non-nil value the fallback result takes priority over any registered static property with the same name.

```cpp
struct MyClass
{
  static int answer () { return 42; }
};

luabridge::getGlobalNamespace (L)
  .beginClass<MyClass> ("MyClass")
    .addStaticFunction ("answer", &MyClass::answer)
    .addStaticIndexMetaMethod ([] (const luabridge::LuaRef& /*key*/, lua_State* L) -> luabridge::LuaRef
    {
      // Returning nil lets the registered static function be found normally
      return luabridge::LuaRef (L, luabridge::LuaNil ());
    })
  .endClass ();
```

Then in lua:

```lua
-- The registered static function is still callable because the fallback returned nil
result = MyClass.answer ()
assert (result == 42)
```

2.8 - Lua Stack
---------------

In the Lua C API, all operations on the `lua_State` are performed through the Lua stack. In order to pass values back and forth between C++ and Lua, LuaBridge uses specializations of this template class concept:

```cpp
namespace luabridge {

template <class T>
struct Stack
{
  static Result push (lua_State* L, const T& value);

  static TypeResult<T> get (lua_State* L, int index);

  static bool isInstance (lua_State* L, int index);
};

} // namespace luabridge
```

When a specialization of `luabridge::Stack <>` exists for a given type `T` we say that the `T` is _convertible_. Throughout this document and the LuaBridge API, these types can be used anywhere a convertible type is expected.

The Stack template class specializations are used automatically for variables, properties, data members, property members, function arguments and return values. These basic types are supported:

* `bool`
* `char`, converted to a string of length one.
* `const char*`, `std::string_view` and `std::string` all converted to strings.
* `std::byte`, integers, `float`, `double`, `long double` all converted to `lua_Number`. Floating-point values including NaN and Inf pass through without error.
* `void*` and `const void*` mapped to Lua lightuserdata.

User-defined types which are convertible to one of the basic types are possible, simply provide a `luabridge::Stack <>` specialization in the `luabridge` namespace for your user-defined type, modeled after the existing types. For example, here is a specialization for a `juce::String`:

```cpp
namespace luabridge {

template <>
struct Stack<juce::String>
{
  static Result push (lua_State* L, const juce::String& s)
  {
    lua_pushstring (L, s.toUTF8 ());
    return {};
  }

  static TypeResult<juce::String> get (lua_State* L, int index)
  {
    if (lua_type (L, index) != LUA_TSTRING)
        return makeErrorCode (ErrorCode::InvalidTypeCast);

    std::size_t length = 0;
    const char* str = lua_tolstring (L, index, &length);
    if (str == nullptr)
        return makeErrorCode (ErrorCode::InvalidTypeCast);

    return juce::String::fromUTF8 (str);
  }

  static bool isInstance (lua_State* L, int index)
  {
    return lua_type (L, index) == LUA_TSTRING;
  }
};

} // namespace luabridge
```

To make sure the library can work without exceptions enabled, if for some reason the push and get of the value on/from the lua stack cannot be performed, it is mandatory to return a `luabridge::Result` object that can be constructed from a `std::error_code`. It also good practice to resotre the stack to it's original state in case of failures:

```cpp
namespace luabridge {

template <class T>
struct Stack<Array<T>>
{
  static Result push (lua_State* L, const Array<T>& array)
  {
    const int initialStackSize = lua_gettop (L);

    lua_createtable (L, static_cast<int> (array.size ()), 0);

    for (std::size_t i = 0; i < array.size (); ++i)
    {
      lua_pushinteger (L, static_cast<lua_Integer> (i + 1));

      auto result = Stack<T>::push (L, array[i]);
      if (! result)
      {
        lua_pop (L, lua_gettop (L) - initialStackSize);
        return result;
      }

      lua_settable (L, -3);
    }

    return {};
  }

  static TypeResult<Array<T>> get (lua_State* L, int index)
  {
    if (!lua_istable (L, index))
      return makeErrorCode (ErrorCode::InvalidTypeCast);

    const int initialStackSize = lua_gettop (L);

    Array<T> a;
    a.reserve (static_cast<std::size_t> (get_length(L, index)));

    const int absIndex = lua_absindex (L, index);

    lua_pushnil (L);
    while (lua_next (L, absIndex) != 0)
    {
      auto item = Stack<T>::get (L, -1);
      if (! item)
      {
        lua_pop (L, lua_gettop (L) - initialStackSize);
        return item.error ();
      }

      a.append (*item);

      lua_pop (L, 1);
    }

    return a;
  }
};

} // namespace luabridge
```

### 2.8.1 - Enums

In order to expose C++ enums to lua and be able to work bidirectionally with them, it's necesary to create a Stack specialization for each exposed enum. As the process might become tedious, a library wrapper class is provided to simplify the steps.

```cpp
enum class MyEnum : int16_t
{
  A,
  B,
  C
};

template <>
struct luabridge::Stack<MyEnum> : luabridge::Enum<MyEnum>
{
};
```

This will map the enum to an integer as `int16_t` (the `underlying_type_t` of the enum) that will be converted to a `lua_Integer` in lua space. This has the drawback that any `lua_Integer` could be casted to a C++ enum. In order to provide a runtime check over the possible alternatives a `lua_Integer` could casted to, it's possible to specify the list of values the C++ enum has: the values registered into the `luabridge::Enum` will be checked against the passed integer and LuaBridge will raise an error in case the cast couldn't be made when using a `luabridge::Stack<>::get` method:

```cpp
enum class MyEnum
{
  A = 1,
  B,
  C
};

template <>
struct luabridge::Stack<MyEnum> : luabridge::Enum<MyEnum,
                                                  MyEnum::A,
                                                  MyEnum::B,
                                                  MyEnum::C>
{
};

luabridge::push (L, 0); // Zero is NOT a valid enum value

auto result = luabridge::get<MyEnum> (L, 1);
assert (! result);
```

The preferred and easier way to expose enum values to lua, is by using a namespace and registering variables or properties for each value:

```cpp

// Variables are just values in lua, so for example doing `MyEnum1.A = 42` from lua will modified the value
luabridge::getGlobalNamespace (L)
  .beginNamespace ("MyEnum1")
    .addVariable ("A", MyEnum::A)
    .addVariable ("B", MyEnum::B)
    .addVariable ("C", MyEnum::C)
  .endNamespace();

// This ¡nstead will prevent the modification of the value from lua
luabridge::getGlobalNamespace (L)
  .beginNamespace ("MyEnum2")
    .addProperty ("A", +[] { return MyEnum::A; })
    .addProperty ("B", +[] { return MyEnum::B; })
    .addProperty ("C", +[] { return MyEnum::C; })
  .endNamespace();
```

### 2.8.2 - lua_State

Sometimes it is convenient from within a bound function or member function to gain access to the `lua_State*` normally available to a lua_CFunction. With LuaBridge, all you need to do is add a `lua_State*` as the last parameter of your bound function:

```cpp
void useState (lua_State* L);

luabridge::getGlobalNamespace (L).addFunction ("useState", &useState);
```

You can still include regular arguments while receiving the state:

```cpp
void useStateAndArgs (int i, std::string s, lua_State* L);

luabridge::getGlobalNamespace (L).addFunction ("useStateAndArgs", &useStateAndArgs);
```

When the script calls `useStateAndArgs`, it passes only the integer and string parameters. LuaBridge takes care of inserting the `lua_State*` into the argument list for the corresponding C++ function. This will work correctly even for the state created by coroutines. Undefined behavior results if the `lua_State*` is not the last parameter.

The same is applicable for properties.

2.9 - C++20 Coroutine Integration
----------------------------------

LuaBridge3 provides first-class interoperability between C++20 coroutines and Lua coroutines, available when compiling with C++20 or later and Lua 5.2+ (requires `lua_yieldk`). The feature is guarded by `LUABRIDGE_HAS_CXX20_COROUTINES`, which is detected automatically and can be suppressed with `LUABRIDGE_DISABLE_CXX20_COROUTINES`.

> **Note:** C++20 coroutine integration is not supported on Lua 5.1, LuaJIT, or Luau (those targets lack a public `lua_yieldk` equivalent).

### 2.9.1 - CppCoroutine\<R\> - Generators callable from Lua

`luabridge::CppCoroutine<R>` is a coroutine return type that bridges C++20 coroutines with Lua's `coroutine.wrap` / `coroutine.resume` API. A function returning `CppCoroutine<R>` can use `co_yield` to suspend and pass a value back to Lua, and `co_return` to finish and return a final value.

Register via `Namespace::addCoroutine`:

```cpp
luabridge::getGlobalNamespace(L)
    .addCoroutine("range", [](int from, int to) -> luabridge::CppCoroutine<int>
    {
        for (int i = from; i <= to; ++i)
            co_yield i;
        co_return -1;  // sentinel value when the range is exhausted
    });
```

From Lua, use `coroutine.wrap` to create a callable iterator:

```lua
local gen = coroutine.wrap(range)
local v = gen(1, 5)   -- first call passes arguments; yields 1
while v ~= -1 do
    print(v)          -- 1, 2, 3, 4, 5
    v = gen()         -- subsequent calls resume without arguments
end
```

`CppCoroutine<void>` is also supported for coroutines that produce no values:

```cpp
.addCoroutine("doWork", []() -> luabridge::CppCoroutine<void>
{
    performStep1();
    co_return;
});
```

An abandoned coroutine (one that goes out of scope in Lua without being fully consumed) is automatically cleaned up by the Lua garbage collector - no manual resource management is needed.

### 2.9.2 - Accepting Arguments

The factory lambda receives the Lua call arguments on first invocation. A `lua_State*` parameter, if present, must be the **first** parameter and receives the running Lua thread:

```cpp
.addCoroutine("adder", [](int a, int b) -> luabridge::CppCoroutine<int>
{
    co_yield a + b;   // first resume yields the sum
    co_return a * b;  // second resume returns the product
});
```

```lua
local f = coroutine.wrap(adder)
print(f(3, 4))   -- 7   (yield: 3+4)
print(f())       -- 12  (return: 3*4)
```

Multiple independent instances of the same coroutine factory can run concurrently - each call to `coroutine.wrap(name)` creates a separate C++ coroutine frame:

```lua
local a = coroutine.wrap(adder)
local b = coroutine.wrap(adder)
a(1, 2)   -- independent from b
b(10, 20)
```

### 2.9.3 - Class Coroutines - Static and Member

Coroutines can be attached directly to a registered class using `addStaticCoroutine` and `addCoroutine`.

**Static coroutines** behave identically to namespace-level coroutines but live in the class's static table. The factory requires no object argument:

```cpp
luabridge::getGlobalNamespace(L)
    .beginClass<Counter>("Counter")
        .addStaticCoroutine("range", [](int from, int count) -> luabridge::CppCoroutine<int>
        {
            for (int i = 0; i < count; ++i)
                co_yield from + i;
            co_return -1;
        })
    .endClass();
```

```lua
local f = coroutine.wrap(Counter.range)
print(f(5, 3))   -- 5  (first yield)
print(f())       -- 6
print(f())       -- 7
print(f())       -- -1 (done)
```

**Member coroutines** bind a coroutine factory to individual class instances. The factory's **first argument must be `T*` or `const T*`** - LuaBridge passes the Lua object as that argument automatically:

```cpp
.beginClass<Counter>("Counter")
    .addCoroutine("generate", [](Counter* obj, int n) -> luabridge::CppCoroutine<int>
    {
        for (int i = 0; i < n; ++i)
        {
            co_yield obj->value;
            obj->increment();
        }
        co_return -1;
    })
.endClass();
```

```lua
local obj = Counter()
local f = coroutine.wrap(Counter.generate)
print(f(obj, 3))   -- 0  (obj.value before first increment)
print(f())         -- 1
print(f())         -- 2
print(f())         -- -1 (done)
```

**Const vs non-const:** a factory that takes `const T*` as its first argument is registered as a const method - accessible on both const and non-const objects (it appears in both the const and non-const class tables). A factory taking `T*` is registered as a non-const method and is accessible on non-const objects only.

```cpp
// Accessible on const and non-const objects:
.addCoroutine("peek", [](const Counter* obj) -> luabridge::CppCoroutine<int>
{
    co_yield obj->value;
    co_return obj->value * 2;
})

// Accessible on non-const objects only:
.addCoroutine("pop", [](Counter* obj) -> luabridge::CppCoroutine<int>
{
    co_yield obj->value--;
    co_return obj->value;
})
```

### 2.9.4 - LuaCoroutine - Awaiting a Lua Thread from C++

`luabridge::LuaCoroutine` is an awaitable that can be used inside a `CppCoroutine` body to resume a child Lua thread synchronously. It runs the child thread to its first yield or return and gives back the status and the number of values the child left on its stack:

```cpp
.addCoroutine("driver", [](lua_State* L) -> luabridge::CppCoroutine<int>
{
    // Spawn a child Lua thread and anchor it in the registry so the GC
    // doesn't collect it while we hold a pointer to it.
    lua_State* child = lua_newthread(L);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);  // pops thread from L's stack

    lua_getglobal(child, "luaGenerator");

    // Resume the child synchronously; suspends this C++ coroutine until done.
    auto [status, nresults] = co_await luabridge::LuaCoroutine{ child, L };

    int value = (nresults > 0) ? static_cast<int>(lua_tointeger(child, -nresults)) : 0;

    luaL_unref(L, LUA_REGISTRYINDEX, ref);
    co_return value;
});
```

`LuaCoroutine` always completes synchronously (no external event loop is required). The `status` field contains `LUA_YIELD` if the child yielded or `LUA_OK` if it returned normally.

### 2.9.5 - Limitations

* **Lua version:** Requires Lua 5.2+ (`lua_yieldk`). Not supported on Lua 5.1, LuaJIT, or Luau.
* **C++ version:** Requires C++20 (`<coroutine>`). Non-coroutine features continue to work under C++17.
* **Multi-value yield:** `co_yield` sends exactly one value per suspension. Use `std::tuple` or a struct if multiple values are needed.
* **Thread safety:** Coroutine frames must be driven from a single OS thread.

3 - Passing Objects
===================

An object of a registered class `T` may be passed to Lua as:

**`T`**

Passed by value (a copy), with _Lua lifetime_.

**`const T`**

Passed by value (a copy), with _Lua lifetime_.

**`T*`**

Passed by reference, with _C++ lifetime_.

**`T&`**

Passed by reference, with _C++ lifetime_.

**`const T*`**

Passed by const reference, with _C++ lifetime_.

**`const T&`**

Passed by const reference, with _C++ lifetime_.

3.1 - C++ Lifetime
------------------

The creation and deletion of objects with _C++ lifetime_ is controlled by the C++ code. Lua does nothing when it garbage collects a reference to such an object. Specifically, the object's destructor is not called (since C++ owns it). Care must be taken to ensure that objects with C++ lifetime are not deleted while still being referenced by a `lua_State*`, or else undefined behavior results. In the previous examples, an instance of `A` can be passed to Lua with C++ lifetime, like this:

```cpp
A a;

auto result = luabridge::push (L, &a);              // pointer to 'a', C++ lifetime
lua_setglobal (L, "a");

auto result = luabridge::push (L, (const A*) &a);   // pointer to 'a const', C++ lifetime
lua_setglobal (L, "ac");

auto result = luabridge::push <const A*> (L, &a);   // equivalent to push (L, (A const*) &a)
lua_setglobal (L, "ac2");

auto result = luabridge::push (L, new A);           // compiles, but will leak memory
lua_setglobal (L, "ap");
```

3.2 - Lua Lifetime
------------------

When an object of a registered class is passed by value to Lua, it will have _Lua lifetime_. A copy of the passed object is constructed inside the userdata. When Lua has no more references to the object, it becomes eligible for garbage collection. When the userdata is collected, the destructor for the class will be called on the object. Care must be taken to ensure that objects with Lua lifetime are not accessed by C++ after they are garbage collected, or else undefined behavior results. An instance of `B` can be passed to Lua with Lua lifetime this way:

```cpp
B b;

auto result = luabridge::push (L, b);  // Copy of b passed, Lua lifetime.
lua_setglobal (L, "b");
```

Given the previous code segments, these Lua statements are applicable:

```lua
print (test.A.staticData)       -- Prints the static data member.
print (test.A.staticProperty)   -- Prints the static property member.
test.A.staticFunc ()            -- Calls the static method.

print (a.data)                  -- Prints the data member.
print (a.prop)                  -- Prints the property member.
a:func1 ()                      -- Calls A::func1 ().
test.A.func1 (a)                -- Equivalent to a:func1 ().
test.A.func1 ("hello")          -- Error: "hello" is not a class A.
a:virtualFunc ()                -- Calls A::virtualFunc ().

print (b.data)                  -- Prints B::dataMember.
print (b.prop)                  -- Prints inherited property member.
b:func1 ()                      -- Calls B::func1 ().
b:func2 ()                      -- Calls B::func2 ().
test.B.func2 (a)                -- Error: a is not a class B.
test.A.func1 (b)                -- Calls A::func1 ().
b:virtualFunc ()                -- Calls B::virtualFunc ().
test.B.virtualFunc (b)          -- Calls B::virtualFunc ().
test.A.virtualFunc (b)          -- Calls B::virtualFunc ().
test.B.virtualFunc (a)          -- Error: a is not a class B.

a = nil; collectgarbage ()      -- 'a' still exists in C++.
b = nil; collectgarbage ()      -- Lua calls ~B() on the copy of b.
```

When Lua script creates an object of class type using a registered constructor, the resulting value will have Lua lifetime. After Lua no longer references the object, it becomes eligible for garbage collection. You can still pass these to C++, either by reference or by value. If passed by reference, the usual warnings apply about accessing the reference later, after it has been garbage collected.

3.3 - Pointers, References, and Pass by Value
---------------------------------------------

When C++ objects are passed from Lua back to C++ as arguments to functions, or set as data members, LuaBridge does its best to automate the conversion. Using the previous definitions, the following functions may be registered to Lua:

```cpp
void func0 (A a);
void func1 (A* a);
void func2 (A const* a);
void func3 (A& a);
void func4 (A const& a);
```

Executing this Lua code will have the prescribed effect:

```lua
func0 (a)   -- Passes a copy of a, using A's copy constructor.
func1 (a)   -- Passes a pointer to a.
func2 (a)   -- Passes a pointer to a const a.
func3 (a)   -- Passes a reference to a.
func4 (a)   -- Passes a reference to a const a.
```

In the example above, all functions can read the data members and property members of `a`, or call const member functions of `a`. Only `func0`, `func1`, and `func3` can modify the data members and data properties, or call non-const member functions of `a`.

The usual C++ inheritance and pointer assignment rules apply. Given:

```cpp
void func5 (B b);
void func6 (B* b);
```

These Lua statements hold:

```lua
func5 (b)   -- Passes a copy of b, using B's copy constructor.
func6 (b)   -- Passes a pointer to b.
func6 (a)   -- Error: Pointer to B expected.
func1 (b)   -- Okay, b is a subclass of a.
```

When a pointer or pointer to const is passed to Lua and the pointer is null (zero), LuaBridge will pass Lua a `nil` instead. When Lua passes a `nil` to C++ where a pointer is expected, a null (zero) is passed instead. Attempting to pass a null pointer to a C++ function expecting a reference results in `lua_error` being called.

3.4 - Shared Lifetime
---------------------

LuaBridge supports a _shared lifetime_ model: dynamically allocated and reference counted objects whose ownership is shared by both Lua and C++. The object remains in existence until there are no remaining C++ or Lua references, and Lua performs its usual garbage collection cycle. A container is recognized by a specialization of the `ContainerTraits` template class. LuaBridge will automatically recognize when a data type is a container when the corresponding specialization is present. Two styles of containers come with LuaBridge, including the necessary specializations.

### 3.4.1 - User-defined Containers

If you have your own container, you must provide a specialization of `luabridge::ContainerTraits` in the `luabridge` namespace for your type before it will be recognized by LuaBridge (or else the code will not compile):

```cpp
namespace luabridge {

template <class T>
struct ContainerTraits<CustomContainer<T>>
{
  using Type = T;

  static CustomContainer<T> construct (T* c)
  {
    return c;
  }

  static T* get (const CustomContainer<T>& c)
  {
    return c.getPointerToObject ();
  }
};

} // namespace luabridge
```

Containers must be safely constructible from raw pointers to objects that are already referenced by other instances of the container (such as is the case for the provided containers or for example `boost::intrusive_ptr` but not `std::shared_ptr` or `boost::shared_ptr`).

### 3.4.2 - shared_ptr As Container

Standard containers like `std::shared_ptr` or `boost::shared_ptr` will work in LuaBridge3, but they require special care. This is because of type erasure; when the object goes from C++ to Lua and back to C++, constructing a new shared_ptr from the raw pointer will create another reference count and result in undefined behavior, unless it could intrusively reconstruct the container from a raw pointer.

To overcome this issue classes that should be managed by `shared_ptr` have to provide a way to correctly reconstruct a `shared_ptr` which can be done only if type hold it is deriving publicly from `std::enable_shared_from_this` or `boost::enable_shared_from_this`. No additional specialization of traits is needed in this case.

```cpp
struct A : public std::enable_shared_from_this<A>
{
  A () { }
  A (int) { }

  void foo () { }
};

luabridge::getGlobalNamespace (L)
  .beginClass<A> ("A")
    .addConstructorFrom<std::shared_ptr<A>, void(), void(int)> ()
    .addFunction ("foo", &A::foo)
  .endClass ();

std::shared_ptr<A> a = std::make_shared<A> (1);
luabridge::setGlobal (L, a, "a");

std::shared_ptr<A> retrieveA = luabridge::getGlobal<std::shared_ptr<A>> (L, "a");
retrieveA->foo ();
```

```lua
a.foo ()

anotherA = A ()
anotherA.foo ()

anotherA2 = A (1)
anotherA2.foo ()
```

### 3.4.3 - Container Constructors

When a constructor is registered for a class, there is a method called `addConstructorFrom` which accepts the type of container to use. This parameter allows the constructor to create the object dynamically, via operator new, and place it a container of that type. The container must have been previously specialized in `ContainerTraits`, or else it will produce a compile error:

```cpp
class C : public std::enable_shared_from_this<C>
{
  C () { }
  C (int) { }
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass <C> ("C")
      .addConstructorFrom<std::shared_ptr<C>, void(), void(int)> ()
    .endClass ()
  .endNamespace ()
```

Alternatively is possible to pass custom lambdas to construct the container, where the return value of those lambdas must be exactly the container specified:

```cpp
class C : public std::enable_shared_from_this<C>
{
  C () { }
  C (int) { }
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass <C> ("C")
      .addConstructorFrom<std::shared_ptr<C>> (
        []() { return std::make_shared<C> (); },
        [](int value) { return std::make_shared<C> (value); })
    .endClass ()
  .endNamespace ()
```

3.5 - Mixing Lifetimes
----------------------

Mixing object lifetime models is entirely possible, subject to the usual caveats of holding references to objects which could get deleted. For example, C++ can be called from Lua with a pointer to an object of class type; the function can modify the object or call non-const data members. These modifications are visible to Lua (since they both refer to the same object). An object store in a container can be passed to a function expecting a pointer. These conversion work seamlessly.

3.6 - Convenience Functions
---------------------------

The `luabridge::setGlobal` function can be used to assign any convertible value into a global variable.

4 - Accessing Lua from C++
==========================

Because Lua is a _dynamically typed language_, special consideration is required to map values in Lua to C++. The following sections describe the classes and functions used for representing Lua types. Only the essential operations are explained; To gain understanding of all available functions, please refer to the documentation comments in the corresponding source files.

4.1 - Class LuaRef
------------------

The `luabridge::LuaRef` class is a container which references any Lua type. It can hold anything which a Lua variable can hold: **nil**, number, boolean, string, table, function, thread, userdata, and lightuserdata. Because `luabridge::LuaRef` uses the `luabridge::Stack` template specializations to do its work, classes, functions, and data exported to Lua through namespace registrations can also be stored (these are instances of userdata). In general, a `luabridge::LuaRef` can represent any _convertible_ C++ type as well as all Lua types.

A `luabridge::LuaRef` variable constructed with no parameters produces a reference to **nil**:

```cpp
luabridge::LuaRef v (L); // References nil
```

To construct a `LuaRef` to a specific value, the two parameter constructor is used:

```cpp
luabridge::LuaRef v1 (L, 1);                   // A LUA_TNUMBER
luabridge::LuaRef v2 (L, 1.1);                 // Also a LUA_TNUMBER
luabridge::LuaRef v3 (L, true);                // A LUA_TBOOLEAN
luabridge::LuaRef v4 (L, "string");            // A LUA_TSTRING
```

The functions `newTable` and `getGlobal` create references to new empty table and an existing value in the global table respectively:

```cpp
luabridge::LuaRef v1 = luabridge::newTable (L);           // Create a new table
luabridge::LuaRef v2 = luabridge::getGlobal (L, "print")  // Reference to _G ["print"]
```

A `LuaRef` can hold classes _registered_ using LuaBridge:

```cpp
class A;

//...

luabridge::LuaRef v (L, new A); // A LuaBridge userdata holding a pointer to A
```

Any convertible type may be assigned to an already-existing `LuaRef`:

```cpp
luabridge::LuaRef v (L);        // Nil
v = luabridge::newTable (L);    // An empty table
v = "string";                   // A string. The previous value becomes eligible for garbage collection.
```

A `LuaRef` is itself a convertible type, and the convertible type `LuaNil` can be used to represent a Lua **nil**.

```cpp
luabridge::LuaRef v1 (L, "x");  // assign "x"
luabridge::LuaRef v2 (L, "y");  // assign "y"
v2 = v1;                        // v2 becomes "x"
v1 = "z";                       // v1 becomes "z", v2 is unchanged
v1 = luabridge::newTable (L);   // An empty table
v2 = v1;                        // v2 references the same table as v1
v1 = luabridge::LuaNil ();      // v1 becomes nil, table is still referenced by v2.
```

Values stored in a `luabridge::LuaRef` object obey the same rules as variables in Lua: tables, functions, threads, and full userdata values are _objects_. The `luabridge::LuaRef` does not actually _contain_ these values, only _references_ to them. Assignment, parameter passing, and function returns always manipulate references to such values; these operations do not imply any kind of copy.

### 4.1.1 - Lifetime, States and Lua Threads

Lifetime of `luabridge::LuaRef` is bound to the lua state or thread passed in when constructing the reference. It is responsibility of the developer to keep the passed lua state/thread alive for the duration of the usage of the `luabridge::LuaRef`. In case of storing objects in those references that might be created in lua threads that could be destroyed during the application lifetime, it is advised to pass `luabridge::main_thread (L)` in place of `L` when constructing a `luabridge::LuaRef`, to make sure the reference is kept in the main lua state instead of the volatile lua thread where it has been created.

In order to have `luabridge::main_thread` method working in all lua versions, one have to call `luabridge::registerMainThread` function at the beginning of the usage of luabridge (lua 5.1 doesn't store the main thread in the registry, and this needs to be manually setup by the developer).

### 4.1.2 - Type Conversions

A universal C++ conversion operator is provided for implicit conversions which allow a `LuaRef` to be used where any convertible type is expected. These operations will all compile:

```cpp
void passInt (int);
void passBool (bool);
void passString (std::string);
void passObject (A*);

LuaRef v (L);
//...
passInt (v);        // implicit conversion to int
passBool (v);       // implicit conversion to bool
passString (v);     // implicit conversion to string
passObject (v);     // must hold a registered LuaBridge class or a
                    // lua_error() will be called.
```

Since Lua types are dynamic, the conversion is performed at run time using traditional functions like `lua_toboolean` or `lua_tostring`. In some cases, the type information may be incorrect especially when passing objects of registered class types. When performing these conversions, LuaBridge may raise a Lua error by directly or indirectly calling `lua_error` To be bullet-proof, such code must either be wrapped in a `lua_pcall`, or you must install a Lua _panic function_ that throws an exception which you can catch.

When an explicit conversion is required (such as when writing templates), use the `cast` template function or an explicit C++ style cast.

```cpp
void passString (std::string);

luabridge::LuaRef v (L);

// The following are all equivalent, and they could be potentially unsafe:

passString (std::string (v));
passString ((std::string) v);
passString (static_cast<std::string> (v));
passString (v.unsafe_cast<std::string> ());
```

The only way to ensure safety when type casting is to use the `luabridge::LuaRef::cast<T>` method, which is a safe cast of a lua reference to a type `T`. It will return a `luabridge::TypeResult<T>` which will contain the type if the cast was successful, and an error code otherwise. No exception or abort will be triggered from such call (while it's not the same for `luabridge::LuaRef::cast<T>`).

```cpp
void passString (std::string);

luabridge::LuaRef v (L);

// The following is safe and will never throw exceptions or call the lua panic handler.

passString (v.cast<std::string> ().valueOr ("fallback"));
```

4.2 - Table Proxies
-------------------

As tables are the sole data structuring mechanism in Lua, the `luabridge::LuaRef` class provides robust facilities for accessing and manipulating table elements using a simple, precise syntax. Any convertible type may be used as a key or value. Applying the array indexing operator `[]` to a `luabridge::LuaRef` returns a special temporary object called a _table proxy_ which supports all the operations which can be performed on a `luabridge::LuaRef`. In addition, assignments made to table proxies change the underlying table. Because table proxies are compiler-created temporary objects, you don't work with them directly. A LuaBridge table proxy should not be confused with the Lua proxy table technique described in the book "Programming in Lua"; the LuaBridge table proxy is simply an intermediate C++ class object that works behind the scenes to make table manipulation syntax conform to C++ idioms. These operations all invoke table proxies:

```cpp
luabridge::LuaRef v (L);
v = luabridge::newTable (L);

v ["name"] = "John Doe";             // string key, string value
v [1] = 200;                         // integer key, integer value
v [2] = luabridge::newTable (L);     // integer key, LuaRef value
v [3] = v [1];                       // assign 200 to integer index 3
v [1] = 100;                         // v[1] is 100, v[3] is still 200
v [3] = v [2];                       // v[2] and v[3] reference the same table
v [2] = luabridge::LuaNil ();        // Removes the value with key = 2. The table is still referenced by v[3].
```

To append one or more values to a sequence table, use `append`. It is equivalent to assigning to `#t + 1`, `#t + 2`, etc., and uses `lua_rawseti` internally:

```cpp
luabridge::LuaRef t = luabridge::newTable (L);

t.append (1);              // t = {1}
t.append (2, 3);           // t = {1, 2, 3}
t.append ("hello", true);  // t = {1, 2, 3, "hello", true}
```

`append` returns `true` if all values were successfully pushed and stored, and stops early (returning `false`) if any value fails to push onto the Lua stack.

4.3 - Calling Lua
-----------------

Table proxies and `luabridge::LuaRef` objects provide a convenient syntax for invoking `lua_pcall` on suitable referenced object. This includes C functions, Lua functions, or Lua objects with an appropriate `__call` metamethod set. The provided implementation supports up to eight parameters (although more can be supported by adding new functions). Any convertible C++ type can be passed as a parameter in its native format. The return value of the function call is provided as a `luabridge::LuaRef`, which may be **nil**.

```lua
function same (arg1, arg)
  return arg1 == arg2
end
```

```cpp
luabridge::LuaRef same = luabridge::getGlobal (L, "same");

// These all evaluate to true
same (1,1);
!same (1,2);
same ("text", "text");
!same (1, "text");
same (1, 1, 2); // third param ignored
```

Table proxies support all of the Lua call notation that `luabridge::LuaRef` supports, making these statements possible:

```lua
t[1]();
t[2]("a", "b");
t[2](t[1]); // Call t[3] with the value in t[2]
t[4]=t[3]();   // Call t[3] and store the result in t[4].

t [t[5]()] = "wow"; // Store "wow" at the key returned by
                    //   the call to t[5]

t = {}
t[1] = function () print ("hello") end
t[2] = function (u, v) print (u, v) end
t[3] = "foo"
```

```cpp
luabridge::LuaRef v = luabridge::getGlobal (L, "t");
```

### 4.3.1 - Exceptions

By default `LuaBridge3` is able to work without exceptions, and it's perfectly compatible with the `-fno-exceptions` or `/EHsc-` flags, which is typically used in games. Even if compiling with exceptions enabled, they are not used internally when calling into lua to convert lua errors, but exceptions are only used in registration code to signal potential issues when registering namespaces, classes and methods. You can use the free function `luabridge::enableExceptions` to enable exceptions once before starting to use any luabridge call, and of course that will work only if the application is compiled with exceptions enabled.

When using `luabridge::call` or `LuaRef::operator()`, no exception is raised regardless of whether exceptions are enabled in the application. To detect whether the invoked Lua function raised an error, check the `TypeResult<void>` returned by those calls:

```lua
function fail ()
  error ("A problem occurred")
end
```

```cpp
luabridge::LuaRef f = luabridge::getGlobal (L, "fail");

auto result = f ();
if (! result)
  std::cerr << result.message ();
```

To call a Lua function and decode its first return value to a specific C++ type, use the typed `call<R>()` overload:

```lua
function add (a, b)
  return a + b
end
```

```cpp
luabridge::LuaRef f = luabridge::getGlobal (L, "add");

auto result = f.call<int> (1, 2);
if (result)
  std::cout << *result;  // prints 3
```

It is also possible that pushing an unregistered class instance into those function will generate an error, that can be trapped using the same mechanism:

```lua
function fail (unregistred)
  error ("Should never reach here")
end
```

```cpp
struct UnregisteredClass {};

luabridge::LuaRef f = luabridge::getGlobal (L, "fail");

auto argument = UnregisteredClass();

auto result = f (argument);
if (! result)
  std::cerr << result.message ();
```

Calling `luabridge::pcall` will not return a `TypeResult` but only the raw status code. It will throw an exception if the return code is not `LUA_OK` (when exceptions are enabled), or return the error code otherwise.

When compiling `LuaBridge3` with exceptions disabled, all references to try catch blocks and throws will be removed.

### 4.3.2 - Class LuaException

When the application is compiled with exceptions and `luabridge::enableExceptions` function has been called, using `luabridge::call` or `LuaRef::operator()` will uses the C++ exception handling mechanism, throwing a `luabridge::LuaException` object in case an argument has a type that has not been registered (and cannot be pushed onto the lua stack) or the lua function generated an error:

```lua
function fail ()
  error ("A problem occurred")
end
```

```cpp
luabridge::LuaRef f = luabridge::getGlobal (L, "fail");

try
{
  f ();
}
catch (const luabridge::LuaException& e)
{
  std::cerr << e.what ();
}
```

### 4.3.3 - Calling with Error Handlers

By default, when a Lua error occurs the raw Lua error message is stored in the `TypeResult`. For more detailed diagnostics you can supply a custom message handler (equivalent to the `msgh` parameter of `lua_pcall`). The handler is a C++ callable that receives a `lua_State*`, may inspect the stack, and must return an `int`:

```lua
function riskyOp ()
  error ("something went wrong")
end
```

```cpp
luabridge::LuaRef f = luabridge::getGlobal (L, "riskyOp");

auto handler = [] (lua_State* L) -> int {
  // Augment the error message with a traceback
  luaL_traceback (L, L, lua_tostring (L, 1), 1);
  return 1;
};

auto result = f.callWithHandler (handler);
if (! result)
  std::cerr << result.message ();  // includes traceback
```

The same function is available as a free function:

```cpp
auto result = luabridge::callWithHandler (f, handler, /* args... */);
```

4.4 - Wrapping C++ Callables
-----------------------------

`luabridge::newFunction` (and its equivalent `LuaRef::newFunction`) wraps any C++ callable - a lambda, a function pointer, or a `std::function` - into a Lua function and returns it as a `LuaRef`. This is useful when you need to pass a C++ callback to Lua without going through the namespace/class registration API:

```cpp
// Create a Lua function that squares its argument
luabridge::LuaRef square = luabridge::newFunction (L, [] (int x) { return x * x; });

// Store it in a Lua global
luabridge::setGlobal (L, square, "square");
```

From Lua:
```lua
print (square (5))  -- 25
```

You can also store such a function in a table or pass it as a callback argument.

### 4.4.1 - LuaFunction\<Signature\>

When you have a `LuaRef` that you know will always be called with fixed argument and return types, `LuaFunction<Signature>` provides a strongly-typed wrapper that avoids repeating the template arguments at every call site:

```cpp
// Retrieve a Lua function and wrap it with a known signature
auto add = luabridge::getGlobal (L, "add").callable<int(int, int)>();

auto result = add (3, 4);  // TypeResult<int>
if (result)
  std::cout << *result;    // 7
```

`LuaFunction<Signature>` supports the same `call`, `callWithHandler`, and `isValid` interface as a `LuaRef`. The wrapped `LuaRef` is accessible via `ref()`.

5 - Security
============

The metatables and userdata that LuaBridge creates in the `lua_State*` are protected using a security system, to eliminate the possibility of undefined behavior resulting from scripted manipulation of the environment. The security system has these components:

*   Class and const class tables use the _table proxy_ technique. The corresponding metatables have `__index` and `__newindex` metamethods, so these class tables are immutable from Lua.
*   Metatables have `__metatable` set to a boolean false value. Scripts cannot obtain the metatable from a LuaBridge object.
*   Classes are mapped to metatables through the registry, which Lua scripts cannot access. The global environment does not expose metatables
*   Metatables created by LuaBridge are tagged with a lightuserdata key which is unique in the process. Other libraries cannot forge a LuaBridge metatable.

This security system can be easily bypassed if scripts are given access to the debug library (or functionality similar to it, i.e. a raw `getmetatable`). The security system can also be defeated by C code in the host, either by revealing the unique lightuserdata key to another module or by putting a LuaBridge metatable in a place that can be accessed by scripts.

When a class member function is called, or class property member accessed, the `this` pointer is type-checked. This is because member functions exposed to Lua are just plain functions that usually get called with the Lua colon notation, which passes the object in question as the first parameter. Lua's dynamic typing makes this type-checking mandatory to prevent undefined behavior resulting from improper use.

If a type check error occurs, LuaBridge uses the `lua_error` mechanism to trigger a failure. A host program can always recover from an error through the use of `lua_pcall`; proper usage of LuaBridge will never result in undefined behavior.

However, in certain situations, it may be necessary for the library user to access the metatables of LuaBridge objects. To allow for this, LuaBridge provides the option to expose metatables on a class-by-class basis when scripts are coming from a trusted source. This can be achieved by passing `luabridge::visibleMetatables` option at class registration:

```cpp
luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass <C> ("C", luabridge::visibleMetatables)
      .addConstructor<void ()> ()
    .endClass ()
  .endNamespace ()
```

Metatables are also special in registered LuaBridge's namespaces, as they namespace specific properties and class definitions, and usually their access should be prevented. But it's possible to expose the much like for classes:

```cpp

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test", luabridge::visibleMetatables)
    .beginClass <C> ("C")
      .addConstructor<void ()> ()
    .endClass ()
  .endNamespace ()
```

6 - Configuration
=================

LuaBridge3 exposes several compile-time configuration macros. Each macro can be overridden by defining it **before** including any LuaBridge header, or by passing it as a compiler flag (e.g. `-DLUABRIDGE_SAFE_STACK_CHECKS=0`).

6.1 - LUABRIDGE_SAFE_STACK_CHECKS
----------------------------------

**Default: `1` (enabled)**

When enabled, every `Stack<T>::push` operation calls `lua_checkstack` before pushing a value. This prevents silent stack overflows when the Lua stack is exhausted.

Disable this flag only when you are certain that the Lua stack will never overflow and you need to squeeze out the last bit of performance:

```cpp
#define LUABRIDGE_SAFE_STACK_CHECKS 0
#include <LuaBridge/LuaBridge.h>
```

6.2 - LUABRIDGE_STRICT_STACK_CONVERSIONS
-----------------------------------------

**Default: `0` (disabled)**

Controls how permissive the `Stack<T>::get` operations are when reading values off the Lua stack.

| Type | Non-strict (default) | Strict |
|------|---------------------|--------|
| `bool` | Any Lua value is accepted via `lua_toboolean` (legacy behavior) | Only `LUA_TBOOLEAN` is accepted |
| Integers | Any `LUA_TNUMBER` that fits the target integer type is accepted | Any `LUA_TNUMBER` that fits the target integer type is accepted (same behavior) |
| `std::string` | `LUA_TSTRING` and `LUA_TNUMBER` accepted (numbers coerced to strings) | Only `LUA_TSTRING` is accepted |

In non-strict mode (the default), `lua_toboolean` semantics apply to `bool`: every Lua value except `false` and `nil` is truthy. This preserves backward-compatible behavior for existing code bases.

Enable strict mode when you want explicit, type-safe conversions:

```cpp
#define LUABRIDGE_STRICT_STACK_CONVERSIONS 1
#include <LuaBridge/LuaBridge.h>
```

With strict mode enabled:

```cpp
lua_pushinteger (L, 42);
auto r = luabridge::Stack<bool>::get (L, -1); // error: not a boolean

lua_pushnil (L);
auto r = luabridge::Stack<bool>::get (L, -1); // error: not a boolean

lua_pushstring (L, "hello");
auto r = luabridge::Stack<bool>::get (L, -1); // error: not a boolean

lua_pushboolean (L, 1);
auto r = luabridge::Stack<bool>::get (L, -1); // ok: true
```

6.3 - LUABRIDGE_SAFE_LUA_C_EXCEPTION_HANDLING
-----------------------------------------------

**Default: `0` (disabled). Only meaningful when `LUABRIDGE_HAS_EXCEPTIONS` is `1`.**

When Lua is compiled as C and a C++ exception escapes a registered `lua_CFunction`, the Lua runtime will call `longjmp` instead of propagating the exception, which leads to undefined behavior. Enabling this flag adds a safe indirection that catches C++ exceptions at the CFunction boundary and re-raises them as Lua errors.

Enable this flag only if you are compiling Lua as C (not as C++), have exceptions enabled in your application, and you observe crashes when registered CFunctions throw:

```cpp
#define LUABRIDGE_SAFE_LUA_C_EXCEPTION_HANDLING 1
#include <LuaBridge/LuaBridge.h>
```

> **Warning:** Enabling this flag introduces a small performance overhead on every registered CFunction call through the library.

6.4 - LUABRIDGE_RAISE_UNREGISTERED_CLASS_USAGE
------------------------------------------------

**Default: `1` when exceptions are enabled, `0` otherwise.**

When enabled, using an unregistered class with LuaBridge (for example, passing an instance of a type that has not been registered via `beginClass`) will raise an error rather than silently failing. With exceptions enabled this translates to a `luabridge::LuaException`; with exceptions disabled it translates to a Lua error via `lua_error`.

Override the default when you need fine-grained control:

```cpp
#define LUABRIDGE_RAISE_UNREGISTERED_CLASS_USAGE 0
#include <LuaBridge/LuaBridge.h>
```

6.5 - LUABRIDGE_HAS_CXX20_COROUTINES / LUABRIDGE_DISABLE_CXX20_COROUTINES
--------------------------------------------------------------------------

**`LUABRIDGE_HAS_CXX20_COROUTINES` - auto-detected, override allowed**

LuaBridge3 automatically enables C++20 coroutine support when it detects a C++20 compiler (`__cplusplus >= 202002L` or MSVC `_HAS_CXX20`). The macro is set to `1` when the feature is active and `0` otherwise.

You can force the feature **off** by defining `LUABRIDGE_DISABLE_CXX20_COROUTINES` before including any LuaBridge header:

```cpp
#define LUABRIDGE_DISABLE_CXX20_COROUTINES
#include <LuaBridge/LuaBridge.h>
```

You can also override the detection result explicitly:

```cpp
#define LUABRIDGE_HAS_CXX20_COROUTINES 0   // force off
#define LUABRIDGE_HAS_CXX20_COROUTINES 1   // force on (must actually have C++20)
#include <LuaBridge/LuaBridge.h>
```

Attempting to use coroutine integration on Lua 5.1, LuaJIT, or Luau will emit a compile-time `#error` unless `LUABRIDGE_DISABLE_COROUTINE_INTEGRATION` is also defined.

Appendix - API Reference
========================

Global Options
--------------

```cpp
/// Flag set of options
class Options : FlagSet<uint32_t>;

/// Default options for classes / namespaces registration.
Option defaultOptions;

/// Specify that class methods should allow to the extended by lua scripts.
Option extensibleClass;

/// Allow an extensible class to overriding C++ exposed methods.
Option allowOverridingMethods;

/// Allow access to class / namespace metatables.
Option visibleMetatables;
```

Free Functions
--------------

```cpp
/// Enable exceptions globally. Will translate lua_errors into C++ LuaExceptions. Usable only if compiled with C++ exceptions enabled.
void enableExceptions (lua_State* L);

/// Gets a global Lua variable reference as LuaRef.
LuaRef getGlobal (lua_State* L, const char* name);

/// Gets a global Lua variable reference as type T.
template <class T>
TypeResult<T> getGlobal (lua_State* L, const char* name);

/// Sets a global Lua variable. Throws or return false if the class is not registered.
template <class T>
bool setGlobal (lua_State* L, T* varPtr, const char* name);

/// Gets the global namespace registration object.
Namespace getGlobalNamespace (lua_State* L);

/// Gets a namespace registration object using a table on top of the stack.
Namespace getNamespaceFromStack (lua_State* L);

/// Invokes a LuaRef if it references a lua callable.
template <class R = void, class... Args>
TypeResult<R> call (const LuaRef& object, Args&&... args);

/// Invokes a LuaRef with a custom Lua error message handler.
template <class R = void, class F, class... Args>
TypeResult<R> callWithHandler (const LuaRef& object, F&& errorHandler, Args&&... args);

/// Wraps a C++ callable into a LuaRef representing a Lua function.
template <class F>
LuaRef newFunction (lua_State* L, F&& func);

/// Wrapper for lua_pcall, converting lua errors into C++ exceptions if they are enabled.
int pcall (lua_State* L, int nargs = 0, int nresults = 0, int msgh = 0);

/// Return a range iterable view over a lua table.
Range pairs (const LuaRef& table);
```

Namespace Registration - Namespace
----------------------------------

```cpp
/// Begin or continues namespace registration, returns this namespace object.
template <class T>
Namespace beginNamespace (const char* name);

/// Ends namespace registration, returns the parent namespace object.
template <class T>
Namespace endNamespace ();

/// Registers one or multiple overloaded functions.
template <class... Functions>
Namespace addFunction (const char* name, Functions... functions);

/// Registers a readonly property with only a getter.
template <class Getter>
Namespace addProperty (const char* name, Getter getter);

/// Registers a readwrite property with a getter and a setter.
template <class Getter, class Setter>
Namespace addProperty (const char* name, Getter getter, Setter setter);
```

Class Registration - Class<T>
-----------------------------

```cpp
/// Begins or continues class registration, returns this class object.
template <class T>
Class<T> beginClass (const char* name);

/// Begins derived class registration with one or more base classes, returns this class object.
template <class T, class Base1, class... Bases>
Class<T> deriveClass (const char* name);

/// Ends class registration, returns the parent namespace object.
template <class T>
Namespace endClass ();
```

### Constructor Registration

```cpp
/// Registers one or multiple overloaded constructors for type T.
template <class... Functions>
Class<T> addConstructor ();

/// Registers one or multiple overloaded constructors for type T using callable arguments.
template <class... Functions>
Class<T> addConstructor (Functions... functions);

/// Registers one or multiple overloaded constructors for type T when usable from intrusive container C.
template <class C, class... Functions>
Class<T> addConstructorFrom ();

/// Registers one or multiple overloaded constructors for type T when usable from intrusive container C using callable arguments.
template <class C, class... Functions>
Class<T> addConstructorFrom (Functions... functions);

/// Registers allocator and deallocators for type T.
template <class Alloc, class Dealloc>
Class<T> addFactory (Alloc alloc, Dealloc dealloc);

/// Registers a destructor hook called just before the C++ destructor (__destruct metamethod).
template <class Function>
Class<T> addDestructor (Function function);
```

### Member Function Registration

```cpp
/// Registers one or multiple overloaded functions as member functions.
template <class... Functions>
Class<T> addFunction (const char* name, Functions... functions);
```

### Member Property Registration

```cpp
/// Registers a readonly property with a getter.
template <class Getter>
Class<T> addProperty (const char* name, Getter getter);

/// Registers a readwrite property with a getter and a setter.
template <class Getter>
Class<T> addProperty (const char* name, Getter getter, Setter setter);
```

### Static Function Registration

```cpp
/// Registers one function or multiple overloads.
template <class... Functions>
Class<T> addStaticFunction (const char* name, Functions... functions);
```

### Static Property Registration

```cpp
/// Registers a static readonly property with a getter.
template <class Getter>
Class<T> addStaticProperty (const char* name, Getter getter);

/// Registers a static readwrite property with a getter and a setter.
template <class Getter>
Class<T> addStaticProperty (const char* name, Getter getter, Setter setter);
```

### Metamethod Registration

```cpp
/// Registers a fallback __index handler for instances (called when a key is not found).
template <class Function>
Class<T> addIndexMetaMethod (Function function);

/// Registers a fallback __newindex handler for instances (called when a key is not found).
template <class Function>
Class<T> addNewIndexMetaMethod (Function function);

/// Registers a fallback __index handler for the static class table.
template <class Function>
Class<T> addStaticIndexMetaMethod (Function function);

/// Registers a fallback __newindex handler for the static class table.
template <class Function>
Class<T> addStaticNewIndexMetaMethod (Function function);
```

Lua Variable Reference - LuaRef
-------------------------------

```cpp
/// Creates a nil reference.
LuaRef (lua_State* L);

/// Returns native Lua string representation.
std::string tostring () const;

/// Dumps reference to a stream.
void print (std::ostream& stream) const;

/// Returns the Lua state.
lua_State* state () const;

/// Place the object onto the Lua stack.
void push (lua_State* L);

/// Indicate whether it is a valid reference (not a LUA_NOREF).
bool isValid () const;

/// Return the lua_type.
int type () const;

/// Indicate whether it is a nil reference.
bool isNil () const;

/// Indicate whether it is a reference to a boolean.
bool isBool () const;

/// Indicate whether it is a reference to a number.
bool isNumber () const;

/// Indicate whether it is a reference to a string.
bool isString () const;

/// Indicate whether it is a reference to a table.
bool isTable () const;

/// Indicate whether it is a reference to a function.
bool isFunction () const;

/// Indicate whether it is a reference to a full userdata.
bool isUserdata () const;

/// Indicate whether it is a reference to a light userdata.
bool isLightUserdata () const;

/// Indicate whether it is a reference to a Lua thread.
bool isThread () const;

/// Indicate whether it is a callable, can be either a lua function or an object with the __call metamethod.
bool isCallable () const;

/// Perform implicit type conversion.
template <class T>
operator T () const;

/// Perform the explicit type conversion, safe.
template <class T>
TypeResult<T> cast () const;

/// Perform the explicit type conversion, unsafe (throws or abort on failure).
template <class T>
T unsafe_cast () const;

/// Check if the Lua value is convertible to the type T.
template <class T>
bool isInstance () const;

/// Get the metatable for the LuaRef.
LuaRef getMetatable () const;

/// Compare this reference with a specified value using lua_compare(). This invokes metamethods.
template <class T>
bool operator== (T rhs) const;

/// Compare this reference with a specified value using lua_compare(). This invokes metamethods.
template <class T>
bool operator!= (T rhs) const;

/// Compare this reference with a specified value using lua_compare(). This invokes metamethods.
template <class T>
bool operator< (T rhs) const;

/// Compare this reference with a specified value using lua_compare(). This invokes metamethods.
template <class T>
bool operator<= (T rhs) const;

/// Compare this reference with a specified value using lua_compare(). This invokes metamethods.
template <class T>
bool operator> (T rhs) const;

/// Compare this reference with a specified value using lua_compare(). This invokes metamethods.
template <class T>
bool operator>= (T rhs) const;

/// Compare this reference with a specified value using lua_compare(). This does not invoke metamethods.
template <class T>
bool rawequal (T v) const;

/// Append one or more values to a referred table. If the table is a sequence this will add more elements to it.
/// Uses lua_rawseti internally. Returns true if all values were successfully appended.
template <class... Ts>
bool append (const Ts&... vs) const;

/// Return the length of a referred array. This is identical to applying the Lua # operator.
int length () const;

/// Invoke the lua ref with no expected return value.
template <class... Args>
TypeResult<void> operator() (Args&&... args) const;

/// Invoke the lua ref and decode the return value to R.
template <class R = void, class... Args>
TypeResult<R> call (Args&&... args) const;

/// Invoke the lua ref with an error handler and decode the return value to R.
template <class R = void, class F, class... Args>
TypeResult<R> callWithHandler (F&& errorHandler, Args&&... args) const;

/// Build a strongly-typed callable wrapper from this Lua object.
template <class Signature>
LuaFunction<Signature> callable () const;

/// Wrap a C++ callable into a new Lua function returned as a LuaRef.
template <class F>
static LuaRef newFunction (lua_State* L, F&& func);
```

Lua Nil Special Value - LuaNil
------------------------------

```cpp
/// LuaNil can be used to construct LuaRef.
```

TypeResult<T> - Result of a Call or Cast
-----------------------------------------

```cpp
explicit operator bool() const;

/// Return the contained value (undefined behavior if result holds an error).
const T& value() const;

/// Dereference operator - equivalent to value().
T& operator*();

/// Return the contained value or a default when the result holds an error.
template <class U>
T valueOr(U&& defaultValue) const;

/// Return the error code, if any.
std::error_code error() const;

/// Return the error message string, if any.
std::string message() const;
```

Typed Lua Function Wrapper - LuaFunction\<R(Args...)\>
-------------------------------------------------------

```cpp
/// Construct from a LuaRef.
explicit LuaFunction (const LuaRef& function);

/// Call the function with the given arguments.
TypeResult<R> operator() (Args... args) const;

/// Call the function - equivalent to operator().
TypeResult<R> call (Args... args) const;

/// Call the function with a custom error handler.
template <class F>
TypeResult<R> callWithHandler (F&& errorHandler, Args... args) const;

/// Return true if the underlying LuaRef is callable.
bool isValid () const;

/// Return the underlying LuaRef.
const LuaRef& ref () const;
```

Stack Traits - Stack<T>
-----------------------

```cpp
/// Converts the C++ value into the Lua value at the top of the Lua stack. Returns true if the push could be performed.
/// When false is returned, `ec` contains the error code corresponding to the failure.
Result push (lua_State* L, const T& value);

/// Converts the Lua value at the index into the C++ value of the type T.
TypeResult<T> get (lua_State* L, int index);

/// Checks if the Lua value at the index is convertible into the C++ value of the type T.
bool isInstance (lua_State* L, int index);
```

C++20 Coroutine Types (requires `LUABRIDGE_HAS_CXX20_COROUTINES`)
-----------------------------------------------------------------

```cpp
/// Coroutine return type for C++ generators callable from Lua.
/// R may be any LuaBridge-registered type, or void.
template <class R>
struct CppCoroutine;

/// Awaitable wrapper for a Lua thread. Use inside a CppCoroutine body with co_await.
/// Resumes the child thread synchronously and returns {status, nresults}.
class LuaCoroutine
{
public:
    /// thread  - the Lua thread to resume.
    /// from    - the calling Lua state (typically L inside the coroutine body).
    LuaCoroutine(lua_State* thread, lua_State* from = nullptr) noexcept;

    std::pair<int, int> await_resume() noexcept; // returns {status, nresults}
};

/// Register a CppCoroutine factory in a namespace (available on Namespace).
/// F must be a callable whose return type is CppCoroutine<R>.
template <class F>
Namespace& addCoroutine(const char* name, F factory);

/// Register a CppCoroutine factory as a static class function (available on Class<T>).
/// F must be a callable whose return type is CppCoroutine<R>.
template <class F>
Class<T>& addStaticCoroutine(const char* name, F factory);

/// Register a CppCoroutine factory as a member method (available on Class<T>).
/// F must be a callable whose first argument is T* or const T*, and whose return type is
/// CppCoroutine<R>. A const T* first argument registers the method as const-accessible.
template <class F>
Class<T>& addCoroutine(const char* name, F factory);

/// Portable lua_resume wrapper - fills *nresults on all Lua versions (5.1–5.5).
int lua_resume_x(lua_State* L, lua_State* from, int nargs, int* nresults = nullptr);

/// Returns true if the current C function can yield via lua_yieldk.
bool lua_isyieldable_x(lua_State* L);
```
