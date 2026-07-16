# Accessing Lua from C++

Because Lua is a _dynamically typed language_, special consideration is required to map values in Lua to C++. The following sections describe the classes and functions used for representing Lua types. Only the essential operations are explained; To gain understanding of all available functions, please refer to the documentation comments in the corresponding source files.

## In this area

- [Class LuaRef](luaref.md) - referencing any Lua value from C++, lifetime, and type conversions.
- [Table Proxies](table-proxies.md) - reading and writing Lua table elements with C++ syntax.
- [Calling Lua](calling-lua.md) - invoking Lua functions, exceptions, and error handlers.
- [Wrapping C++ Callables](wrapping-callables.md) - turning lambdas and functions into Lua functions.

```{toctree}
:hidden:
:maxdepth: 1

luaref
table-proxies
calling-lua
wrapping-callables
```
