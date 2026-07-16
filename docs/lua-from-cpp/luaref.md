# Class LuaRef

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

## Lifetime, States and Lua Threads

Lifetime of `luabridge::LuaRef` is bound to the lua state or thread passed in when constructing the reference. It is responsibility of the developer to keep the passed lua state/thread alive for the duration of the usage of the `luabridge::LuaRef`. In case of storing objects in those references that might be created in lua threads that could be destroyed during the application lifetime, it is advised to pass `luabridge::main_thread (L)` in place of `L` when constructing a `luabridge::LuaRef`, to make sure the reference is kept in the main lua state instead of the volatile lua thread where it has been created.

In order to have `luabridge::main_thread` method working in all lua versions, one have to call `luabridge::registerMainThread` function at the beginning of the usage of luabridge (lua 5.1 doesn't store the main thread in the registry, and this needs to be manually setup by the developer).

## Type Conversions

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
