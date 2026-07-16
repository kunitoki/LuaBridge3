# Class Objects

A class registration is opened using either `beginClass` or `deriveClass` and ended using `endClass`. Once registered, a class can later be re-opened for more registrations using `beginClass`. However, `deriveClass` should only be used once. To add more registrations to an already registered derived class, use `beginClass` on it.

These declarations:

```cpp
struct A {
  static int staticData;
  static float staticProperty;

  static float getStaticProperty () { return staticProperty; }
  static void setStaticProperty (float f) { staticProperty = f; }
  static void staticFunc () { }

  static int staticCFunc (lua_State *L) { return 0; }

  std::string dataMember;

  char dataProperty;
  char getProperty () const { return dataProperty; }
  void setProperty (char v) { dataProperty = v; }
  std::string toString () const { return dataMember; }

  void func1 () { }
  virtual void virtualFunc () { }

  int cfunc (lua_State* L) { return 0; }
};

struct B : public A {
  double dataMember2;

  void func1 () { }
  void func2 () { }
  void virtualFunc () { }
};

int A::staticData;
float A::staticProperty;
```

are registered using:

```cpp
luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<A> ("A")
      .addStaticProperty ("staticData", &A::staticData, &A::staticData)
      .addStaticProperty ("staticProperty", &A::getStaticProperty, &A::setStaticProperty)
      .addStaticFunction ("staticFunc", &A::staticFunc)
      .addStaticFunction ("staticCFunc", &A::staticCFunc)
      .addProperty ("data", &A::dataMember, &A::dataMember)
      .addProperty ("prop", &A::getProperty, &A::setProperty)
      .addFunction ("func1", &A::func1)
      .addFunction ("virtualFunc", &A::virtualFunc)
      .addFunction ("__tostring", &A::toString)     // Metamethod
      .addFunction ("cfunc", &A::cfunc)
    .endClass ()
    .deriveClass<B, A> ("B")
      .addProperty ("data", &B::dataMember2, &B::dataMember2)
      .addFunction ("func1", &B::func1)
      .addFunction ("func2", &B::func2)
    .endClass ()
  .endNameSpace ();
```

Method registration works just like function registration. Virtual methods work normally; no special syntax is needed. const methods are detected and const-correctness is enforced, so if a function returns a const object (or a container holding to a const object) to Lua, that reference to the object will be considered const and only const methods can be called on it. It is possible to register Lua metamethods (except `__gc`). Destructors are registered automatically for each class.

As with regular variables and properties, class properties can be marked read-only by passing false in the second parameter, or omitting the set function. The `deriveClass` takes a derived class and one or more registered base classes as template arguments. Inherited methods do not have to be re-declared and will function normally in Lua. If a class has a base class that is **not** registered with Lua, there is no need to declare it as a subclass.

When registering a data member pointer as both readable and writable, the common pattern of passing the same pointer twice to `addProperty` or `addStaticProperty` can be replaced with the convenience methods `addPropertyReadWrite` and `addStaticPropertyReadWrite`:

```cpp
luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<A> ("A")
      .addStaticPropertyReadWrite ("staticData", &A::staticData)  // equivalent to addStaticProperty ("staticData", &A::staticData, &A::staticData)
      .addPropertyReadWrite ("data", &A::dataMember)              // equivalent to addProperty ("data", &A::dataMember, &A::dataMember)
    .endClass ()
  .endNamespace ();
```

These accept only a pointer-to-data-member (for `addPropertyReadWrite`) or a pointer to a static data member (for `addStaticPropertyReadWrite`). They do not accept getter/setter function pairs; use `addProperty` or `addStaticProperty` directly for those cases.

Remember that in Lua, the colon operator '`:`' is used for method call syntax:

```lua
local a = A ()

a.func1 ()  -- error: func1 expects an object of a registered class
a.func1 (a) -- okay, verbose, this how OOP works in Lua
a:func1 ()  -- okay, less verbose, equivalent to the previous
```

## Multiple Inheritance

`deriveClass` supports multiple registered base classes by specifying them as additional template parameters. LuaBridge will traverse all base class hierarchies when resolving member lookups from Lua:

```cpp
struct A
{
  void funcA () { }
};

struct B
{
  void funcB () { }
};

struct C : public A, public B
{
  void funcC () { }
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<A> ("A")
      .addFunction ("funcA", &A::funcA)
    .endClass ()
    .beginClass<B> ("B")
      .addFunction ("funcB", &B::funcB)
    .endClass ()
    .deriveClass<C, A, B> ("C")
      .addFunction ("funcC", &C::funcC)
    .endClass ()
  .endNamespace ();
```

From Lua, all inherited methods are accessible on instances of `C`:

```lua
local c = test.C ()
c:funcA ()  -- calls A::funcA via multiple-inheritance lookup
c:funcB ()  -- calls B::funcB via multiple-inheritance lookup
c:funcC ()  -- calls C::funcC
```

Only base classes that are themselves registered with LuaBridge need to be listed as template parameters. If a base class is not registered, it can be omitted from `deriveClass`.
