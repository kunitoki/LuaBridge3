# Namespaces

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
