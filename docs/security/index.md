# Security

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
