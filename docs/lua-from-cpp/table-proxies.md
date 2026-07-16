# Table Proxies

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
