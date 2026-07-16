# Function Member Proxies

Where it is not possible or inconvenient to add a member to be registered, LuaBridge also allows for a _function member proxy_. This is a flat function which take as its first parameter a pointer to the object:

```cpp
// Third party declaration, can't be changed
struct Vec
{
  float coord [3];
};
```

The class is closed for modifications, but we want to extend Vec objects with our member function. To do this, first we add a "helper" function:

```cpp
void scale (Vec* vec, float value)
{
  vec->coord [0] *= value;
  vec->coord [1] *= value;
  vec->coord [2] *= value;
}
```

Now we can register the `Vec` class with a member function `scale`:

```cpp
luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<Vec> ("Vec")
      .addFunction ("scale", &scale)
    .endClass ()
  .endNamespace ();
```

It is also possible to use lambdas (both capturing and non capturing) as functions proxies:

```cpp
float y = atan (1.0f) * 4.0f;

luabridge::getGlobalNamespace (L)
  .beginClass<Vec> ("Vec")
    .addFunction ("scaleX", +[] (Vec* vec, float v) { vec->coord [0] *= v; })
    .addFunction ("scaleY", [y] (Vec* vec, float v) { vec->coord [1] *= v * y; })
  .endClass ()
```

Of course when not capturing, it is better to prefix the lambda with `+` so it is converted and stored internally to a function pointer instead of an `std::function`, so it is actually lighter to store and faster to call.

## Partial Application with luabridge::bind_front

`std::bind_front` (C++20) returns a callable whose `operator()` is a template, so LuaBridge cannot statically resolve its argument types. Passing such a result directly to `addFunction` therefore fails to compile. The workaround without `luabridge::bind_front` is an explicit cast to `std::function`:

```cpp
int add (int a, int b) { return a + b; }

// Verbose: explicit signature required
ns.addFunction ("add10", std::function<int(int)> (std::bind_front (&add, 10)));
```

`luabridge::bind_front` is a drop-in replacement that deduces the remaining argument types from the underlying callable's signature, so no explicit cast is needed:

```cpp
// Concise: types are deduced automatically
ns.addFunction ("add10", luabridge::bind_front (&add, 10));
```

It also works with member function pointers. The bound object is not counted as a Lua-visible parameter, so `bind_front` turns a member function into a plain callable that Lua sees as a regular function:

```cpp
struct Vec { float coord [3]; float getCoord (int i) const { return coord [i]; } };

Vec v;
// Bind v as the object; Lua supplies the index
ns.addFunction ("getCoord", luabridge::bind_front (&Vec::getCoord, &v));
// Lua: getCoord(0) == v.coord[0]
```

It also accepts lambdas:

```cpp
int base = 10;
ns.addFunction ("addBase", luabridge::bind_front ([base](int offset, int v) { return base + offset + v; }, 2));
// Lua: addBase(30) == 42
```

## Partial Application with luabridge::bind_back

`luabridge::bind_back` is the trailing-argument counterpart to `bind_front`: it pre-binds the **last** arguments of a callable, leaving the **leading** arguments for Lua to supply.

```cpp
int add (int a, int b) { return a + b; }

// bind_back fixes the trailing argument; Lua supplies the leading one
ns.addFunction ("add10", luabridge::bind_back (&add, 10));
// Lua: add10(32) == 42
```

For member function pointers, `bind_back` automatically prepends the class pointer to the remaining (Lua-visible) parameter list so that LuaBridge can dispatch it as a proxy function. This lets you pre-bind a method's trailing arguments while the object still comes from Lua:

```cpp
struct Vec { float coord [3]; float getCoord (int i) const { return coord [i]; } };

ns.beginClass<Vec> ("Vec")
  .addFunction ("x", luabridge::bind_back (&Vec::getCoord, 0))
  .addFunction ("y", luabridge::bind_back (&Vec::getCoord, 1))
  .addFunction ("z", luabridge::bind_back (&Vec::getCoord, 2))
.endClass ();
// Lua: v:x() returns v.coord[0], v:y() returns v.coord[1], etc.
```

`bind_back` also accepts lambdas:

```cpp
int base = 10;
ns.addFunction ("addBase", luabridge::bind_back ([base](int v, int offset) { return base + offset + v; }, 2));
// Lua: addBase(30) == 42
```

## Function Overloading

When specifying more than one method to the `addFunction` or `addStaticFunction` of both `Namespace` and `Class`, those overloads will be invoked in case of a call. Only overloads that have matched arguments arity will be considered, and they will be tried from first to last until the call succeeds.

```cpp
struct Vec { float coord [3]; };
struct Quat { float values [4]; };

void rotateByDegreees (Vec* vec, float degrees);
void rotateByQuaternion (Vec* vec, const Quat& quaternion);
```

Now we can register the `Vec` class with a member function `rotate` that will be resolving the call into the two provided overloads:

```cpp
luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<Vec> ("Vec")
      .addFunction ("rotate", &rotateByDegreees, &rotateByQuaternion)
    .endClass ()
  .endNamespace ();
```

In case of members (or functions) with the same name, it's necessary to use `luabridge::overload`, `luabridge::constOverload` or `luabridge::nonConstOverload` to disambiguate which of the functions needs to be registered:

```cpp
struct Quat { float values [4]; };

struct Vec
{
  void rotate (float degrees);
  void rotate (const Quat& quaternion);

  void x (float new_value);
  float x (float value_if_zero) const;

  float coord [3];
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<Vec> ("Vec")
      .addFunction ("rotate",
        luabridge::overload<float> (&Vec::rotate),
        luabridge::overload<const Quat&> (&Vec::rotate))
      .addFunction ("x",
        luabridge::nonConstOverload<float> (&Vec::x),
        luabridge::constOverload<float> (&Vec::x))
    .endClass ()
  .endNamespace ();

```

It's possible to mix lambdas, function pointers and member functions in overload creation. Providing a `lua_Cfunction` as last method will ensure it can be reached in case no other overload is successfully executed, kind of like a "catch all" method.

Special attention needs to be given to the order (priority) of the overloads, based on the number and type of the arguments. Better to place first the overloads that can be called more frequently, and putting "stronger" types first: for example when having an overload taking an `int` and an overload taking `float`, as lua is not able to distinguish between them properly (until lua 5.4) the first overload will always be called.
