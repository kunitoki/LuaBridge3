# Property Member Proxies

Sometimes when registering a class which comes from a third party library, the data is not exposed in a way that can be expressed as a pointer to member, there are no get or set functions, or the get and set functions do not have the right function signature. Since the class declaration is closed for changes, LuaBridge allows for a _property member proxy_. This is a pair of get and set flat functions which take as their first parameter a pointer to the object. This is easily understood with the following example:

```cpp
// Third party declaration, can't be changed
struct Vec
{
  float coord [3];
};
```

Taking the address of an array element, e.g. `&Vec::coord [0]` results in an error instead of a pointer-to-member. The class is closed for modifications, but we want to export Vec objects to Lua using the familiar object notation. To do this, first we add a "helper" class:

```cpp
struct VecHelper
{
  template <unsigned index>
  static float get (Vec const* vec)
  {
    return vec->coord [index];
  }

  template <unsigned index>
  static void set (Vec* vec, float value)
  {
    vec->coord [index] = value;
  }
};
```

This helper class is only used to provide property member proxies. `Vec` continues to be used in the C++ code as it was before. Now we can register the `Vec` class with property member proxies for `x`, `y`, and `z`:

```cpp
luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<Vec> ("Vec")
      .addProperty ("x", &VecHelper::get<0>, &VecHelper::set<0>)
      .addProperty ("y", &VecHelper::get<1>, &VecHelper::set<1>)
      .addProperty ("z", &VecHelper::get<2>, &VecHelper::set<2>)
    .endClass ()
  .endNamespace ();
```

It is also possible to use both capturing and non capturing lambdas, as well as `std::function <>` instances as proxies:

```cpp
std::function<float (const Vec*)> get_x = [] (const Vec* vec) { return vec->coord [0]; };
std::function<void (Vec*, float)> set_x = [] (Vec* vec, float v) { vec->coord [0] = v; };

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<Vec> ("Vec")
      .addProperty ("x", get_x, set_x)
      // ... same for "y" and "z"
    .endClass ()
  .endNamespace ();
```

Or the more concise version (notice the `+` before the lambda is useful to convert a non capturing lambda to a function pointer in order to avoid allocating a std::function internally, where storing the lambda as function pointer might avoid lua usertype allocation overhead):

```cpp
luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<Vec> ("Vec")
      .addProperty ("x",
        +[] (const Vec* vec) { return vec->coord [0]; },
        +[] (Vec* vec, float v) { vec->coord [0] = v; })
      // ... same for "y" and "z"
    .endClass ()
  .endNamespace ();
```
