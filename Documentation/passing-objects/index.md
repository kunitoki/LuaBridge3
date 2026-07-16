# Passing Objects

An object of a registered class `T` may be passed to Lua as:

**`T`**

Passed by value (a copy), with _Lua lifetime_.

**`const T`**

Passed by value (a copy), with _Lua lifetime_.

**`T*`**

Passed by reference, with _C++ lifetime_.

**`T&`**

Passed by reference, with _C++ lifetime_.

**`const T*`**

Passed by const reference, with _C++ lifetime_.

**`const T&`**

Passed by const reference, with _C++ lifetime_.

## C++ Lifetime

The creation and deletion of objects with _C++ lifetime_ is controlled by the C++ code. Lua does nothing when it garbage collects a reference to such an object. Specifically, the object's destructor is not called (since C++ owns it). Care must be taken to ensure that objects with C++ lifetime are not deleted while still being referenced by a `lua_State*`, or else undefined behavior results. In the previous examples, an instance of `A` can be passed to Lua with C++ lifetime, like this:

```cpp
A a;

auto result = luabridge::push (L, &a);              // pointer to 'a', C++ lifetime
lua_setglobal (L, "a");

auto result = luabridge::push (L, (const A*) &a);   // pointer to 'a const', C++ lifetime
lua_setglobal (L, "ac");

auto result = luabridge::push <const A*> (L, &a);   // equivalent to push (L, (A const*) &a)
lua_setglobal (L, "ac2");

auto result = luabridge::push (L, new A);           // compiles, but will leak memory
lua_setglobal (L, "ap");
```

## Lua Lifetime

When an object of a registered class is passed by value to Lua, it will have _Lua lifetime_. A copy of the passed object is constructed inside the userdata. When Lua has no more references to the object, it becomes eligible for garbage collection. When the userdata is collected, the destructor for the class will be called on the object. Care must be taken to ensure that objects with Lua lifetime are not accessed by C++ after they are garbage collected, or else undefined behavior results. An instance of `B` can be passed to Lua with Lua lifetime this way:

```cpp
B b;

auto result = luabridge::push (L, b);  // Copy of b passed, Lua lifetime.
lua_setglobal (L, "b");
```

Given the previous code segments, these Lua statements are applicable:

```lua
print (test.A.staticData)       -- Prints the static data member.
print (test.A.staticProperty)   -- Prints the static property member.
test.A.staticFunc ()            -- Calls the static method.

print (a.data)                  -- Prints the data member.
print (a.prop)                  -- Prints the property member.
a:func1 ()                      -- Calls A::func1 ().
test.A.func1 (a)                -- Equivalent to a:func1 ().
test.A.func1 ("hello")          -- Error: "hello" is not a class A.
a:virtualFunc ()                -- Calls A::virtualFunc ().

print (b.data)                  -- Prints B::dataMember.
print (b.prop)                  -- Prints inherited property member.
b:func1 ()                      -- Calls B::func1 ().
b:func2 ()                      -- Calls B::func2 ().
test.B.func2 (a)                -- Error: a is not a class B.
test.A.func1 (b)                -- Calls A::func1 ().
b:virtualFunc ()                -- Calls B::virtualFunc ().
test.B.virtualFunc (b)          -- Calls B::virtualFunc ().
test.A.virtualFunc (b)          -- Calls B::virtualFunc ().
test.B.virtualFunc (a)          -- Error: a is not a class B.

a = nil; collectgarbage ()      -- 'a' still exists in C++.
b = nil; collectgarbage ()      -- Lua calls ~B() on the copy of b.
```

When Lua script creates an object of class type using a registered constructor, the resulting value will have Lua lifetime. After Lua no longer references the object, it becomes eligible for garbage collection. You can still pass these to C++, either by reference or by value. If passed by reference, the usual warnings apply about accessing the reference later, after it has been garbage collected.

## Pointers, References, and Pass by Value

When C++ objects are passed from Lua back to C++ as arguments to functions, or set as data members, LuaBridge does its best to automate the conversion. Using the previous definitions, the following functions may be registered to Lua:

```cpp
void func0 (A a);
void func1 (A* a);
void func2 (A const* a);
void func3 (A& a);
void func4 (A const& a);
```

Executing this Lua code will have the prescribed effect:

```lua
func0 (a)   -- Passes a copy of a, using A's copy constructor.
func1 (a)   -- Passes a pointer to a.
func2 (a)   -- Passes a pointer to a const a.
func3 (a)   -- Passes a reference to a.
func4 (a)   -- Passes a reference to a const a.
```

In the example above, all functions can read the data members and property members of `a`, or call const member functions of `a`. Only `func0`, `func1`, and `func3` can modify the data members and data properties, or call non-const member functions of `a`.

The usual C++ inheritance and pointer assignment rules apply. Given:

```cpp
void func5 (B b);
void func6 (B* b);
```

These Lua statements hold:

```lua
func5 (b)   -- Passes a copy of b, using B's copy constructor.
func6 (b)   -- Passes a pointer to b.
func6 (a)   -- Error: Pointer to B expected.
func1 (b)   -- Okay, b is a subclass of a.
```

When a pointer or pointer to const is passed to Lua and the pointer is null (zero), LuaBridge will pass Lua a `nil` instead. When Lua passes a `nil` to C++ where a pointer is expected, a null (zero) is passed instead. Attempting to pass a null pointer to a C++ function expecting a reference results in `lua_error` being called.

## Shared Lifetime

LuaBridge supports a _shared lifetime_ model: dynamically allocated and reference counted objects whose ownership is shared by both Lua and C++. The object remains in existence until there are no remaining C++ or Lua references, and Lua performs its usual garbage collection cycle. A container is recognized by a specialization of the `ContainerTraits` template class. LuaBridge will automatically recognize when a data type is a container when the corresponding specialization is present. Two styles of containers come with LuaBridge, including the necessary specializations.

### User-defined Containers

If you have your own container, you must provide a specialization of `luabridge::ContainerTraits` in the `luabridge` namespace for your type before it will be recognized by LuaBridge (or else the code will not compile):

```cpp
namespace luabridge {

template <class T>
struct ContainerTraits<CustomContainer<T>>
{
  using Type = T;

  static CustomContainer<T> construct (T* c)
  {
    return c;
  }

  static T* get (const CustomContainer<T>& c)
  {
    return c.getPointerToObject ();
  }
};

} // namespace luabridge
```

Containers must be safely constructible from raw pointers to objects that are already referenced by other instances of the container (such as is the case for the provided containers or for example `boost::intrusive_ptr` but not `std::shared_ptr` or `boost::shared_ptr`).

### shared_ptr As Container

Standard containers like `std::shared_ptr` or `boost::shared_ptr` will work in LuaBridge3, but they require special care. This is because of type erasure; when the object goes from C++ to Lua and back to C++, constructing a new shared_ptr from the raw pointer will create another reference count and result in undefined behavior, unless it could intrusively reconstruct the container from a raw pointer.

To overcome this issue classes that should be managed by `shared_ptr` have to provide a way to correctly reconstruct a `shared_ptr` which can be done only if type hold it is deriving publicly from `std::enable_shared_from_this` or `boost::enable_shared_from_this`. No additional specialization of traits is needed in this case.

```cpp
struct A : public std::enable_shared_from_this<A>
{
  A () { }
  A (int) { }

  void foo () { }
};

luabridge::getGlobalNamespace (L)
  .beginClass<A> ("A")
    .addConstructorFrom<std::shared_ptr<A>, void(), void(int)> ()
    .addFunction ("foo", &A::foo)
  .endClass ();

std::shared_ptr<A> a = std::make_shared<A> (1);
luabridge::setGlobal (L, a, "a");

std::shared_ptr<A> retrieveA = luabridge::getGlobal<std::shared_ptr<A>> (L, "a");
retrieveA->foo ();
```

```lua
a.foo ()

anotherA = A ()
anotherA.foo ()

anotherA2 = A (1)
anotherA2.foo ()
```

### Container Constructors

When a constructor is registered for a class, there is a method called `addConstructorFrom` which accepts the type of container to use. This parameter allows the constructor to create the object dynamically, via operator new, and place it a container of that type. The container must have been previously specialized in `ContainerTraits`, or else it will produce a compile error:

```cpp
class C : public std::enable_shared_from_this<C>
{
  C () { }
  C (int) { }
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass <C> ("C")
      .addConstructorFrom<std::shared_ptr<C>, void(), void(int)> ()
    .endClass ()
  .endNamespace ()
```

Alternatively is possible to pass custom lambdas to construct the container, where the return value of those lambdas must be exactly the container specified:

```cpp
class C : public std::enable_shared_from_this<C>
{
  C () { }
  C (int) { }
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass <C> ("C")
      .addConstructorFrom<std::shared_ptr<C>> (
        []() { return std::make_shared<C> (); },
        [](int value) { return std::make_shared<C> (value); })
    .endClass ()
  .endNamespace ()
```

## Mixing Lifetimes

Mixing object lifetime models is entirely possible, subject to the usual caveats of holding references to objects which could get deleted. For example, C++ can be called from Lua with a pointer to an object of class type; the function can modify the object or call non-const data members. These modifications are visible to Lua (since they both refer to the same object). An object store in a container can be passed to a function expecting a pointer. These conversion work seamlessly.

## Convenience Functions

The `luabridge::setGlobal` function can be used to assign any convertible value into a global variable.
