# Calling Lua

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

## Exceptions

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

## Class LuaException

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

## Calling with Error Handlers

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
