# Configuration

LuaBridge3 exposes several compile-time configuration macros. Each macro can be overridden by defining it **before** including any LuaBridge header, or by passing it as a compiler flag (e.g. `-DLUABRIDGE_SAFE_STACK_CHECKS=0`).

## LUABRIDGE_SAFE_STACK_CHECKS

**Default: `1` (enabled)**

When enabled, every `Stack<T>::push` operation calls `lua_checkstack` before pushing a value. This prevents silent stack overflows when the Lua stack is exhausted.

Disable this flag only when you are certain that the Lua stack will never overflow and you need to squeeze out the last bit of performance:

```cpp
#define LUABRIDGE_SAFE_STACK_CHECKS 0
#include <LuaBridge/LuaBridge.h>
```

## LUABRIDGE_STRICT_STACK_CONVERSIONS

**Default: `0` (disabled)**

Controls how permissive the `Stack<T>::get` operations are when reading values off the Lua stack.

| Type | Non-strict (default) | Strict |
|------|---------------------|--------|
| `bool` | Any Lua value is accepted via `lua_toboolean` (legacy behavior) | Only `LUA_TBOOLEAN` is accepted |
| Integers | Any `LUA_TNUMBER` that fits the target integer type is accepted | Any `LUA_TNUMBER` that fits the target integer type is accepted (same behavior) |
| `std::string` | `LUA_TSTRING` and `LUA_TNUMBER` accepted (numbers coerced to strings) | Only `LUA_TSTRING` is accepted |

In non-strict mode (the default), `lua_toboolean` semantics apply to `bool`: every Lua value except `false` and `nil` is truthy. This preserves backward-compatible behavior for existing code bases.

Enable strict mode when you want explicit, type-safe conversions:

```cpp
#define LUABRIDGE_STRICT_STACK_CONVERSIONS 1
#include <LuaBridge/LuaBridge.h>
```

With strict mode enabled:

```cpp
lua_pushinteger (L, 42);
auto r = luabridge::Stack<bool>::get (L, -1); // error: not a boolean

lua_pushnil (L);
auto r = luabridge::Stack<bool>::get (L, -1); // error: not a boolean

lua_pushstring (L, "hello");
auto r = luabridge::Stack<bool>::get (L, -1); // error: not a boolean

lua_pushboolean (L, 1);
auto r = luabridge::Stack<bool>::get (L, -1); // ok: true
```

## LUABRIDGE_SAFE_LUA_C_EXCEPTION_HANDLING

**Default: `0` (disabled). Only meaningful when `LUABRIDGE_HAS_EXCEPTIONS` is `1`.**

When Lua is compiled as C and a C++ exception escapes a registered `lua_CFunction`, the Lua runtime will call `longjmp` instead of propagating the exception, which leads to undefined behavior. Enabling this flag adds a safe indirection that catches C++ exceptions at the CFunction boundary and re-raises them as Lua errors.

Enable this flag only if you are compiling Lua as C (not as C++), have exceptions enabled in your application, and you observe crashes when registered CFunctions throw:

```cpp
#define LUABRIDGE_SAFE_LUA_C_EXCEPTION_HANDLING 1
#include <LuaBridge/LuaBridge.h>
```

> **Warning:** Enabling this flag introduces a small performance overhead on every registered CFunction call through the library.

## LUABRIDGE_RAISE_UNREGISTERED_CLASS_USAGE

**Default: `1` when exceptions are enabled, `0` otherwise.**

When enabled, using an unregistered class with LuaBridge (for example, passing an instance of a type that has not been registered via `beginClass`) will raise an error rather than silently failing. With exceptions enabled this translates to a `luabridge::LuaException`; with exceptions disabled it translates to a Lua error via `lua_error`.

Override the default when you need fine-grained control:

```cpp
#define LUABRIDGE_RAISE_UNREGISTERED_CLASS_USAGE 0
#include <LuaBridge/LuaBridge.h>
```

## LUABRIDGE_HAS_CXX20_COROUTINES / LUABRIDGE_DISABLE_CXX20_COROUTINES

**`LUABRIDGE_HAS_CXX20_COROUTINES` - auto-detected, override allowed**

LuaBridge3 automatically enables C++20 coroutine support when it detects a C++20 compiler (`__cplusplus >= 202002L` or MSVC `_HAS_CXX20`). The macro is set to `1` when the feature is active and `0` otherwise.

You can force the feature **off** by defining `LUABRIDGE_DISABLE_CXX20_COROUTINES` before including any LuaBridge header:

```cpp
#define LUABRIDGE_DISABLE_CXX20_COROUTINES
#include <LuaBridge/LuaBridge.h>
```

You can also override the detection result explicitly:

```cpp
#define LUABRIDGE_HAS_CXX20_COROUTINES 0   // force off
#define LUABRIDGE_HAS_CXX20_COROUTINES 1   // force on (must actually have C++20)
#include <LuaBridge/LuaBridge.h>
```

Attempting to use coroutine integration on Lua 5.1, LuaJIT, or Luau will emit a compile-time `#error` unless `LUABRIDGE_DISABLE_COROUTINE_INTEGRATION` is also defined.

## LUABRIDGE_HAS_CXX17_FILESYSTEM / LUABRIDGE_DISABLE_CXX17_FILESYSTEM

**`LUABRIDGE_HAS_CXX17_FILESYSTEM` - auto-detected, override allowed**

When a C++17 compiler and `<filesystem>` are available, LuaBridge automatically enables `std::filesystem::path` ↔ Lua string conversion. No additional header is needed; the specialization lives inside `LuaBridge/LuaBridge.h`.

To force the feature off:

```cpp
#define LUABRIDGE_DISABLE_CXX17_FILESYSTEM
#include <LuaBridge/LuaBridge.h>
```

## LUABRIDGE_HAS_CXX17_ANY / LUABRIDGE_DISABLE_CXX17_ANY

**`LUABRIDGE_HAS_CXX17_ANY` - auto-detected, override allowed**

When a C++17 compiler and `<any>` are available, LuaBridge enables push support for `std::any` via `LuaBridge/Any.h`. Because `std::any` erases the type at runtime, push is performed through a runtime registry. Types must be pre-registered with `luabridge::registerAnyPush<T>(L)` before a value of that type can be pushed.

To force the feature off:

```cpp
#define LUABRIDGE_DISABLE_CXX17_ANY
#include <LuaBridge/LuaBridge.h>
```

## LUABRIDGE_HAS_CXX20_SPAN / LUABRIDGE_DISABLE_CXX20_SPAN

**`LUABRIDGE_HAS_CXX20_SPAN` - auto-detected when C++20 is enabled, override allowed**

When a C++20 compiler and `<span>` are available, LuaBridge enables push support for `std::span<T, Extent>` via `LuaBridge/Span.h`. `std::span` is push-only — it cannot be retrieved from Lua (use `std::vector<T>` to read sequences back).

To force the feature off:

```cpp
#define LUABRIDGE_DISABLE_CXX20_SPAN
#include <LuaBridge/LuaBridge.h>
```

## LUABRIDGE_HAS_CXX20_RANGES / LUABRIDGE_DISABLE_CXX20_RANGES

**`LUABRIDGE_HAS_CXX20_RANGES` - auto-detected when C++20 is enabled, override allowed**

When a C++20 compiler with ranges support is detected, LuaBridge's `luabridge::Iterator` gains the additional members required to satisfy the `std::input_iterator` concept (`value_type`, `difference_type`, `iterator_concept`) and a matching `operator==`. This allows `luabridge::Range` — the object returned by `luabridge::pairs()` — to be used directly in C++20 range-based algorithms and `std::views` pipelines.

```cpp
// Requires LUABRIDGE_HAS_CXX20_RANGES
for (auto [key, val] : luabridge::pairs(tableRef))
    std::cout << key.tostring() << " = " << val.tostring() << "\n";
```

To force the feature off:

```cpp
#define LUABRIDGE_DISABLE_CXX20_RANGES
#include <LuaBridge/LuaBridge.h>
```

## LUABRIDGE_HAS_CXX23_EXPECTED / LUABRIDGE_DISABLE_CXX23_EXPECTED

**`LUABRIDGE_HAS_CXX23_EXPECTED` - auto-detected when C++23 is enabled, override allowed**

When a C++23 compiler and `<expected>` are available, LuaBridge enables `std::expected<T,E>` ↔ Lua conversion via `LuaBridge/StdExpected.h`. A successful value is pushed as the contained `T`; a failure pushes `nil`.

To force the feature off:

```cpp
#define LUABRIDGE_DISABLE_CXX23_EXPECTED
#include <LuaBridge/LuaBridge.h>
```

## LUABRIDGE_HAS_CXX23_FLAT_MAP / LUABRIDGE_DISABLE_CXX23_FLAT_MAP

**`LUABRIDGE_HAS_CXX23_FLAT_MAP` - auto-detected when C++23 is enabled, override allowed**

When a C++23 compiler and `<flat_map>` are available, LuaBridge enables conversion support for `std::flat_map` via `LuaBridge/FlatMap.h`. This is a contiguous-storage analogue of `std::map` with identical Lua table semantics.

To force the feature off:

```cpp
#define LUABRIDGE_DISABLE_CXX23_FLAT_MAP
#include <LuaBridge/LuaBridge.h>
```

## LUABRIDGE_HAS_CXX23_FLAT_SET / LUABRIDGE_DISABLE_CXX23_FLAT_SET

**`LUABRIDGE_HAS_CXX23_FLAT_SET` - auto-detected when C++23 is enabled, override allowed**

When a C++23 compiler and `<flat_set>` are available, LuaBridge enables conversion support for `std::flat_set` via `LuaBridge/FlatSet.h`. This is a contiguous-storage analogue of `std::set` with identical Lua table semantics.

To force the feature off:

```cpp
#define LUABRIDGE_DISABLE_CXX23_FLAT_SET
#include <LuaBridge/LuaBridge.h>
```

## LUABRIDGE_HAS_CXX23_MOVE_ONLY_FUNCTION / LUABRIDGE_DISABLE_CXX23_MOVE_ONLY_FUNCTION

**`LUABRIDGE_HAS_CXX23_MOVE_ONLY_FUNCTION` - auto-detected when C++23 is enabled, override allowed**

When a C++23 compiler with `std::move_only_function` support is detected, LuaBridge's function-traits machinery recognises `std::move_only_function<R(Args...)>` and its `noexcept` / `const` variants as valid callable types. This allows registering move-only callables (e.g. lambdas that capture `std::unique_ptr`) directly with `addFunction` / `addStaticFunction` / `addCoroutine`.

To force the feature off:

```cpp
#define LUABRIDGE_DISABLE_CXX23_MOVE_ONLY_FUNCTION
#include <LuaBridge/LuaBridge.h>
```
