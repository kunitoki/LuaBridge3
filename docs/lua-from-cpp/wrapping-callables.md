# Wrapping C++ Callables

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

## LuaFunction\<Signature\>

When you have a `LuaRef` that you know will always be called with fixed argument and return types, `LuaFunction<Signature>` provides a strongly-typed wrapper that avoids repeating the template arguments at every call site:

```cpp
// Retrieve a Lua function and wrap it with a known signature
auto add = luabridge::getGlobal (L, "add").callable<int(int, int)>();

auto result = add (3, 4);  // TypeResult<int>
if (result)
  std::cout << *result;    // 7
```

`LuaFunction<Signature>` supports the same `call`, `callWithHandler`, and `isValid` interface as a `LuaRef`. The wrapped `LuaRef` is accessible via `ref()`.
