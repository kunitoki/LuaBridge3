# C++20 Coroutine Integration

LuaBridge3 provides first-class interoperability between C++20 coroutines and Lua coroutines, available when compiling with C++20 or later and Lua 5.2+ (requires `lua_yieldk`). The feature is guarded by `LUABRIDGE_HAS_CXX20_COROUTINES`, which is detected automatically and can be suppressed with `LUABRIDGE_DISABLE_CXX20_COROUTINES`.

> **Note:** C++20 coroutine integration is not supported on Lua 5.1, LuaJIT, or Luau (those targets lack a public `lua_yieldk` equivalent).

## CppCoroutine\<R\> - Generators callable from Lua

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

## Accepting Arguments

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

## Class Coroutines - Static and Member

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

## LuaCoroutine - Awaiting a Lua Thread from C++

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

## Limitations

* **Lua version:** Requires Lua 5.2+ (`lua_yieldk`). Not supported on Lua 5.1, LuaJIT, or Luau.
* **C++ version:** Requires C++20 (`<coroutine>`). Non-coroutine features continue to work under C++17.
* **Multi-value yield:** `co_yield` sends exactly one value per suspension. Use `std::tuple` or a struct if multiple values are needed.
* **Thread safety:** Coroutine frames must be driven from a single OS thread.
