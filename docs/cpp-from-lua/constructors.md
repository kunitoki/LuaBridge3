# Constructors

A single constructor may be added for a class using `addConstructor`. LuaBridge cannot automatically determine the number and types of constructor parameters like it can for functions and methods, so you must provide them. This is done by specifying the signature of the desired constructor functions as template parameters to `addConstructor`. The parameter types will be extracted from this (the return type is ignored). For example, these statements register constructors for the given classes:

```cpp
struct A
{
  A ();
  A (std::string_view a);
  A (std::string_view a, int b);
};

struct B
{
  explicit B (const char* s, int nChars);
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<A> ("A")
      .addConstructor<void (), void (std::string_view), void (std::string_view, int)> ()
    .endClass ()
    .beginClass<B> ("B")
      .addConstructor<void (const char*, int)> ()
    .endClass ()
  .endNamespace ();
```

Constructors added in this fashion are called from Lua using the fully qualified name of the class. This Lua code will create instances of `A` and `B`.

```lua
a = test.A ()           -- Create a new A.
b = test.B ("hello", 5) -- Create a new B.
b = test.B ()           -- Error: expected string in argument 1
```

## Constructor Proxies

Sometimes is not possible to use a constructor for a class, because some of the constructor arguments have types that couldn't be exposed to lua, or more control is needed when constructors need to be invoked (like checking the lau stack). So it is possible to workaround those limitations by using a special `addConstructor` that doesn't need any template specialiation, but takes only one or more functors, which will allow to placement new the c++ class in a c++ lambda, specifying any custom parameter there:

```cpp
struct NotExposed;
NotExposed* shouldNotSeeMe;

struct HardToCreate
{
  explicit HardToCreate (const NotExposed* x, int easy);
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<HardToCreate> ("HardToCreate")
      .addConstructor ([&shouldNotSeeMe] (void* ptr, int easy) { return new (ptr) HardToCreate (shouldNotSeeMe, easy); })
    .endClass ()
  .endNamespace ();
```

Then in lua:

```lua
hard = test.HardToCreate (5) -- Create a new HardToCreate.
```

The `addConstructor` overload taking a generic functor also accepts a `lua_State*` as last parameter in order to be used for constructors that needs to be overloaded by different numbers of arguments (arguments will start at index 2 of the stack):

```cpp
luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<HardToCreate> ("HardToCreate")
      .addConstructor ([] (void* ptr, lua_State* L) { return new (ptr) HardToCreate (shouldNotSeeMe, lua_checkinteger (L, 2)); })
    .endClass ()
  .endNamespace ();
```

As mentioned at the beginning, it's possible to specify multiple functors, that will be tried in order until the object can be constructed:

```cpp
struct NotExposed;
NotExposed* shouldNotSeeMe;

struct HardToCreate
{
  HardToCreate (const NotExposed* x, int easy);
  HardToCreate (const NotExposed* x, int easy, int lessEasy);
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<HardToCreate> ("HardToCreate")
      .addConstructor (
        [&shouldNotSeeMe] (void* ptr, int easy) { return new (ptr) HardToCreate (shouldNotSeeMe, easy); },
        [&shouldNotSeeMe] (void* ptr, int easy, int lessEasy) { return new (ptr) HardToCreate (shouldNotSeeMe, easy, lessEasy); })
    .endClass ()
  .endNamespace ();
```

## Constructor Factories

If granular control over allocation and deallocation of a type is needed, the `addFactory` method can be used to register both and allocator and deallocator of C++ type. This is useful in case of having classes being provided by factory methods from shared libraries.

```cpp
struct IObject
{
  virtual void overridableMethod () const = 0;
};

// These might be defined in a shared library, returning concrete types.
extern "C" IObject* objectFactoryAllocator ();
extern "C" void objectFactoryDeallocator (IObject*);

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<IObject> ("Object")
      .addFactory (&objectFactoryAllocator, &objectFactoryDeallocator)
      .addFunction ("overridableMethod", &IObject::overridableMethod)
    .endClass ()
  .endNamespace ();
```

The object is the perfectly instantiable through lua:

```lua
a = test.Object ()           -- Create a new Object using objectFactoryAllocator
a = nil                      -- Remove any reference count
collectgarbage ("collect")   -- The object is garbage collected using objectFactoryDeallocator
```

## Destructors

In addition to the automatic `__gc` metamethod that LuaBridge registers for every class (which calls the C++ destructor when Lua garbage-collects the userdata), you can register an extra hook that is called **just before** the destructor runs. This is useful for performing clean-up work in a context where the object is still fully valid.

Use `addDestructor` to register the hook. The callable must accept a `T*` (and optionally a trailing `lua_State*`):

```cpp
struct Resource
{
  ~Resource () { /* cleanup */ }
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<Resource> ("Resource")
      .addDestructor ([] (Resource* r) {
        // called before ~Resource()
        std::cout << "Resource about to be destroyed\n";
      })
    .endClass ()
  .endNamespace ();
```

The `__destruct` metamethod is invoked by Lua's garbage collector before `__gc` calls the C++ destructor. This metamethod is only available on non-Luau builds; Luau already calls `__gc` directly.
