# LuaBridge3 Documentation

**LuaBridge3** is a lightweight and dependency-free library for mapping data,
functions, and classes back and forth between C++ and [Lua](https://www.lua.org),
a powerful, fast, lightweight, embeddable scripting language. It is header-only,
works with any Lua version out there (PUC-Lua, LuaJIT, Luau, Ravi), and can be
used with or without C++ exceptions enabled.

```{note}
LuaBridge3 is distributed as a collection of header files. You simply add one
line, `#include <LuaBridge/LuaBridge.h>`, where you want to pass functions,
classes, and variables back and forth between C++ and Lua. There are no
additional source files, no compilation settings, and no Makefiles.
```

## Documentation areas

The documentation is organized by topic. Each area contains its own concept
guides, walkthroughs, and reference material.

- [Introduction](introduction/index.md) - what LuaBridge3 is, its design goals, repository, and license.
- [Accessing C++ from Lua](cpp-from-lua/index.md) - registering namespaces, functions, properties, classes, constructors, the Lua stack, and coroutines.
- [Passing Objects](passing-objects/index.md) - object lifetime models: C++, Lua, and shared lifetime.
- [Accessing Lua from C++](lua-from-cpp/index.md) - `LuaRef`, table proxies, calling Lua functions, and wrapping callables.
- [Security](security/index.md) - the metatable security system and how to relax it for trusted scripts.
- [Configuration](configuration/index.md) - compile-time configuration macros.
- [API Reference](api-reference/index.md) - the complete registration and reference API.

## External resources

- [LuaBridge3 on GitHub](https://github.com/kunitoki/LuaBridge3) - source code, issues, releases, and discussion.
- [Original LuaBridge](https://github.com/vinniefalco/LuaBridge) - the official LuaBridge repository (up to version 2).

```{toctree}
:hidden:
:maxdepth: 2

introduction/index
cpp-from-lua/index
passing-objects/index
lua-from-cpp/index
security/index
configuration/index
api-reference/index
```
