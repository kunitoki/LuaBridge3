# Lua Stack

In the Lua C API, all operations on the `lua_State` are performed through the Lua stack. In order to pass values back and forth between C++ and Lua, LuaBridge uses specializations of this template class concept:

```cpp
namespace luabridge {

template <class T>
struct Stack
{
  static Result push (lua_State* L, const T& value);

  static TypeResult<T> get (lua_State* L, int index);

  static bool isInstance (lua_State* L, int index);
};

} // namespace luabridge
```

When a specialization of `luabridge::Stack <>` exists for a given type `T` we say that the `T` is _convertible_. Throughout this document and the LuaBridge API, these types can be used anywhere a convertible type is expected.

The Stack template class specializations are used automatically for variables, properties, data members, property members, function arguments and return values. These basic types are supported:

* `bool`
* `char`, converted to a string of length one.
* `const char*`, `std::string_view` and `std::string` all converted to strings.
* `std::byte`, integers, `float`, `double`, `long double` all converted to `lua_Number`. Floating-point values including NaN and Inf pass through without error.
* `void*` and `const void*` mapped to Lua lightuserdata.

User-defined types which are convertible to one of the basic types are possible, simply provide a `luabridge::Stack <>` specialization in the `luabridge` namespace for your user-defined type, modeled after the existing types. For example, here is a specialization for a `juce::String`:

```cpp
namespace luabridge {

template <>
struct Stack<juce::String>
{
  static Result push (lua_State* L, const juce::String& s)
  {
    lua_pushstring (L, s.toUTF8 ());
    return {};
  }

  static TypeResult<juce::String> get (lua_State* L, int index)
  {
    if (lua_type (L, index) != LUA_TSTRING)
        return makeErrorCode (ErrorCode::InvalidTypeCast);

    std::size_t length = 0;
    const char* str = lua_tolstring (L, index, &length);
    if (str == nullptr)
        return makeErrorCode (ErrorCode::InvalidTypeCast);

    return juce::String::fromUTF8 (str);
  }

  static bool isInstance (lua_State* L, int index)
  {
    return lua_type (L, index) == LUA_TSTRING;
  }
};

} // namespace luabridge
```

To make sure the library can work without exceptions enabled, if for some reason the push and get of the value on/from the lua stack cannot be performed, it is mandatory to return a `luabridge::Result` object that can be constructed from a `std::error_code`. It also good practice to resotre the stack to it's original state in case of failures:

```cpp
namespace luabridge {

template <class T>
struct Stack<Array<T>>
{
  static Result push (lua_State* L, const Array<T>& array)
  {
    const int initialStackSize = lua_gettop (L);

    lua_createtable (L, static_cast<int> (array.size ()), 0);

    for (std::size_t i = 0; i < array.size (); ++i)
    {
      lua_pushinteger (L, static_cast<lua_Integer> (i + 1));

      auto result = Stack<T>::push (L, array[i]);
      if (! result)
      {
        lua_pop (L, lua_gettop (L) - initialStackSize);
        return result;
      }

      lua_settable (L, -3);
    }

    return {};
  }

  static TypeResult<Array<T>> get (lua_State* L, int index)
  {
    if (!lua_istable (L, index))
      return makeErrorCode (ErrorCode::InvalidTypeCast);

    const int initialStackSize = lua_gettop (L);

    Array<T> a;
    a.reserve (static_cast<std::size_t> (get_length(L, index)));

    const int absIndex = lua_absindex (L, index);

    lua_pushnil (L);
    while (lua_next (L, absIndex) != 0)
    {
      auto item = Stack<T>::get (L, -1);
      if (! item)
      {
        lua_pop (L, lua_gettop (L) - initialStackSize);
        return item.error ();
      }

      a.append (*item);

      lua_pop (L, 1);
    }

    return a;
  }
};

} // namespace luabridge
```

## Custom Type Converters

For registered class types, LuaBridge can also convert one C++ userdata type into another C++ value type while reading function arguments from Lua. This is useful when Lua should pass a lightweight or external representation to a function that expects your domain type.

Custom converters are opt-in per target type. To enable them:

* Specialize `luabridge::StackConversion<To>` with `enabled = true`.
* Specialize `luabridge::StackConverter<To, From>` and provide `static To convert (const From&)`.
* Register the source class with `.addConverter<To>()`.

```cpp
struct Vec3Source
{
  float x = 0.f;
  float y = 0.f;
  float z = 0.f;

  Vec3Source () = default;
  Vec3Source (float x, float y, float z) : x (x), y (y), z (z) {}
};

struct Vec3
{
  float x = 0.f;
  float y = 0.f;
  float z = 0.f;

  Vec3 () = default;
  Vec3 (float x, float y, float z) : x (x), y (y), z (z) {}
};

float length (Vec3 v);
float lengthRef (const Vec3& v);
```

Define the conversion in the `luabridge` namespace:

```cpp
namespace luabridge {

template <>
struct StackConversion<Vec3>
{
  static constexpr bool enabled = true;
};

template <>
struct StackConverter<Vec3, Vec3Source>
{
  static Vec3 convert (const Vec3Source& v)
  {
    return { v.x, v.y, v.z };
  }
};

} // namespace luabridge
```

Then register the source type and the converter:

```cpp
luabridge::getGlobalNamespace (L)
  .beginClass<Vec3Source> ("Vec3Source")
    .addConstructor<void (float, float, float)> ()
    .addConverter<Vec3> ()
  .endClass ()
  .addFunction ("length", &length)
  .addFunction ("lengthRef", &lengthRef);
```

Lua can now pass a `Vec3Source` where C++ expects `Vec3` by value or by `const Vec3&`:

```lua
v = Vec3Source (1, 2, 3)

length (v)
lengthRef (v)
```

The converter is consulted only after normal userdata extraction fails. If the Lua value already holds the requested target type, LuaBridge uses that object directly. If the value is derived from the requested target type, normal inheritance lookup is used first. The registered converter is the fallback path.

Multiple source classes can register converters to the same target type by specializing `StackConverter<To, From>` for each source type and calling `.addConverter<To>()` on each source class. Registering the same converter more than once is harmless; the later registration replaces the earlier entry.

Converters return an owned target value. That means converted temporaries can be passed to functions taking `To` or `const To&`. They are not a substitute for mutable references or pointers: a function that expects `To&`, `To*`, or `const To*` still requires a Lua value that actually stores the target object.

`Stack<To>::isInstance` remains an exact/inheritance userdata test. It does not report source userdata as instances of the target type just because a converter is registered. This matters mainly for overload selection: converters are most predictable when the overload set does not rely on `isInstance` to distinguish converted arguments.

The target class only needs to be registered with `beginClass<To>` if Lua must construct, store, or receive target objects directly. A converter used only to feed a C++ function argument does not require the target type to be exposed to Lua.

## Enums

In order to expose C++ enums to lua and be able to work bidirectionally with them, it's necesary to create a Stack specialization for each exposed enum. As the process might become tedious, a library wrapper class is provided to simplify the steps.

```cpp
enum class MyEnum : int16_t
{
  A,
  B,
  C
};

template <>
struct luabridge::Stack<MyEnum> : luabridge::Enum<MyEnum>
{
};
```

This will map the enum to an integer as `int16_t` (the `underlying_type_t` of the enum) that will be converted to a `lua_Integer` in lua space. This has the drawback that any `lua_Integer` could be casted to a C++ enum. In order to provide a runtime check over the possible alternatives a `lua_Integer` could casted to, it's possible to specify the list of values the C++ enum has: the values registered into the `luabridge::Enum` will be checked against the passed integer and LuaBridge will raise an error in case the cast couldn't be made when using a `luabridge::Stack<>::get` method:

```cpp
enum class MyEnum
{
  A = 1,
  B,
  C
};

template <>
struct luabridge::Stack<MyEnum> : luabridge::Enum<MyEnum,
                                                  MyEnum::A,
                                                  MyEnum::B,
                                                  MyEnum::C>
{
};

luabridge::push (L, 0); // Zero is NOT a valid enum value

auto result = luabridge::get<MyEnum> (L, 1);
assert (! result);
```

The preferred and easier way to expose enum values to lua, is by using a namespace and registering variables or properties for each value:

```cpp

// Variables are just values in lua, so for example doing `MyEnum1.A = 42` from lua will modified the value
luabridge::getGlobalNamespace (L)
  .beginNamespace ("MyEnum1")
    .addVariable ("A", MyEnum::A)
    .addVariable ("B", MyEnum::B)
    .addVariable ("C", MyEnum::C)
  .endNamespace();

// This ¡nstead will prevent the modification of the value from lua
luabridge::getGlobalNamespace (L)
  .beginNamespace ("MyEnum2")
    .addProperty ("A", +[] { return MyEnum::A; })
    .addProperty ("B", +[] { return MyEnum::B; })
    .addProperty ("C", +[] { return MyEnum::C; })
  .endNamespace();
```

## lua_State

Sometimes it is convenient from within a bound function or member function to gain access to the `lua_State*` normally available to a lua_CFunction. With LuaBridge, all you need to do is add a `lua_State*` as the last parameter of your bound function:

```cpp
void useState (lua_State* L);

luabridge::getGlobalNamespace (L).addFunction ("useState", &useState);
```

You can still include regular arguments while receiving the state:

```cpp
void useStateAndArgs (int i, std::string s, lua_State* L);

luabridge::getGlobalNamespace (L).addFunction ("useStateAndArgs", &useStateAndArgs);
```

When the script calls `useStateAndArgs`, it passes only the integer and string parameters. LuaBridge takes care of inserting the `lua_State*` into the argument list for the corresponding C++ function. This will work correctly even for the state created by coroutines. Undefined behavior results if the `lua_State*` is not the last parameter.

The same is applicable for properties.

## Standard Library Type Conversions

LuaBridge does not enable STL container-to-Lua-table conversions by default. Each supported container type has its own optional header that must be included explicitly. All conversions map the container to a Lua table and back.

The table below lists every optional header and its requirements:

| Header | Type | C++ Standard | Notes |
|--------|------|-------------|-------|
| `LuaBridge/Array.h` | `std::array<T,N>` | C++17 | Fixed-size sequence |
| `LuaBridge/Vector.h` | `std::vector<T>` | C++17 | Sequence |
| `LuaBridge/Deque.h` | `std::deque<T>` | C++17 | Double-ended sequence |
| `LuaBridge/ForwardList.h` | `std::forward_list<T>` | C++17 | Singly-linked sequence |
| `LuaBridge/List.h` | `std::list<T>` | C++17 | Doubly-linked sequence |
| `LuaBridge/Map.h` | `std::map<K,V>` | C++17 | Ordered key-value table |
| `LuaBridge/MultiMap.h` | `std::multimap<K,V>` | C++17 | Ordered multi-value table; each key maps to an array of values in Lua |
| `LuaBridge/Set.h` | `std::set<K>` | C++17 | Ordered set |
| `LuaBridge/UnorderedMap.h` | `std::unordered_map<K,V>` | C++17 | Hash key-value table |
| `LuaBridge/UnorderedMultiMap.h` | `std::unordered_multimap<K,V>` | C++17 | Hash multi-value table; each key maps to an array of values in Lua |
| `LuaBridge/UnorderedSet.h` | `std::unordered_set<K>` | C++17 | Hash set |
| `LuaBridge/Optional.h` | `std::optional<T>` | C++17 | Nullable value |
| `LuaBridge/Variant.h` | `std::variant<Ts...>` | C++17 | Tagged union |
| `LuaBridge/Any.h` | `std::any` | C++17 (`LUABRIDGE_HAS_CXX17_ANY`) | Push-only; types must be pre-registered with `luabridge::registerAnyPush<T>(L)` |
| `LuaBridge/Span.h` | `std::span<T>` | C++20 (`LUABRIDGE_HAS_CXX20_SPAN`) | Push-only; use `std::vector<T>` to retrieve sequences from Lua |
| `LuaBridge/StdExpected.h` | `std::expected<T,E>` | C++23 (`LUABRIDGE_HAS_CXX23_EXPECTED`) | Pushes the value on success, nil on error |
| `LuaBridge/FlatMap.h` | `std::flat_map<K,V>` | C++23 (`LUABRIDGE_HAS_CXX23_FLAT_MAP`) | Ordered key-value table backed by contiguous storage |
| `LuaBridge/FlatSet.h` | `std::flat_set<K>` | C++23 (`LUABRIDGE_HAS_CXX23_FLAT_SET`) | Ordered set backed by contiguous storage |

`std::filesystem::path` is also supported as a built-in type when C++17 filesystem is available (`LUABRIDGE_HAS_CXX17_FILESYSTEM`). It is converted to and from a Lua string automatically; no additional header is required beyond `LuaBridge/LuaBridge.h`.

**Example — using `std::deque` and `std::multimap`:**

```cpp
#include <LuaBridge/LuaBridge.h>
#include <LuaBridge/Deque.h>
#include <LuaBridge/MultiMap.h>

// std::deque<int> pushes as a 1-based Lua table: {10, 20, 30}
std::deque<int> dq = {10, 20, 30};
luabridge::push(L, dq);

// std::multimap pushes as a table where each key maps to an array of values:
// { a = {1, 2}, b = {3} }
std::multimap<std::string, int> mm = {{"a", 1}, {"a", 2}, {"b", 3}};
luabridge::push(L, mm);
```

**Example — push-only `std::any`:**

Types stored inside a `std::any` must be registered before they can be pushed:

```cpp
#include <LuaBridge/LuaBridge.h>
#include <LuaBridge/Any.h>

luabridge::registerAnyPush<int>(L);
luabridge::registerAnyPush<std::string>(L);

std::any value = 42;
luabridge::push(L, value); // pushes integer 42
```

**Example — `std::expected` (C++23):**

```cpp
#include <LuaBridge/LuaBridge.h>
#include <LuaBridge/StdExpected.h>

std::expected<int, std::string> ok = 42;
luabridge::push(L, ok); // pushes integer 42

std::expected<int, std::string> err = std::unexpected("oops");
luabridge::push(L, err); // pushes nil
```

**`std::unique_ptr` as an ownership container:**

`std::unique_ptr<T>` is supported as a container type without any additional header. Lua receives a non-owning view of the object. The C++ side retains ownership and must outlive any Lua reference to the object:

```cpp
auto obj = std::make_unique<MyClass>();
luabridge::push(L, obj); // Lua gets a non-owning reference; obj must outlive the Lua reference
```

**`std::move_only_function` (C++23):**

When `LUABRIDGE_HAS_CXX23_MOVE_ONLY_FUNCTION` is active, `std::move_only_function<R(Args...)>` can be registered in a namespace or class just like `std::function`. This allows registering callables that are move-only (e.g. lambdas capturing unique ownership):

```cpp
luabridge::getGlobalNamespace(L)
    .addFunction("compute", std::move_only_function<int(int)>([resource = std::make_unique<MyResource>()](int x) {
        return resource->process(x);
    }));
```
