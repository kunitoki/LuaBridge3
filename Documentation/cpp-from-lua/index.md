# Accessing C++ from Lua

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

## In this area

- [Namespaces](namespaces.md) - registration scopes implemented as Lua tables.
- [Properties and Functions](properties-and-functions.md) - exposing variables, properties, and free functions.
- [Class Objects](class-objects.md) - registering classes, static members, and inheritance.
- [Property Member Proxies](property-member-proxies.md) - exposing properties for closed third-party classes.
- [Function Member Proxies](function-member-proxies.md) - flat functions as member functions, partial application, and overloading.
- [Constructors](constructors.md) - constructors, constructor proxies, factories, and destructors.
- [Extending Classes](extending-classes.md) - extensible classes and index metamethod fallbacks.
- [Lua Stack](lua-stack.md) - stack traits, custom type converters, enums, and standard library conversions.
- [C++20 Coroutine Integration](coroutines.md) - bridging C++20 coroutines with Lua coroutines.

```{toctree}
:hidden:
:maxdepth: 1

namespaces
properties-and-functions
class-objects
property-member-proxies
function-member-proxies
constructors
extending-classes
lua-stack
coroutines
```
