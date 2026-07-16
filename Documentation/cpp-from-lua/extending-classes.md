# Extending Classes

The LuaBridge library provides a set of features dedicated to extending classes from lua.

## Extensible Classes

The `luabridge::extensibleClass` option tells LuaBridge to create a metatable for the class that can be modified at runtime by Lua code. By doing so, it's then possible to extend the class from lua by adding custom instance methods and static methods to the type. Because those methods are stored in the metatable of the type, no additional storage is needed.

```cpp
struct ExtensibleClass
{
  int propertyOne = 42;
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<ExtensibleClass> ("ExtensibleClass", luabridge::extensibleClass)
      .addProperty ("propertyOne", &ExtensibleClass::propertyOne, &ExtensibleClass::propertyOne)
    .endClass ()
  .endNamespace ();

ExtensibleClass clazz;
luabridge::pushGlobal (L, &clazz, "clazz");
```

From lua is then possible to extend the class by registering methods on the type:

```lua
function ExtensibleClass:newInstanceMethod()
  return self.propertyOne * 2
end

function ExtensibleClass.newStaticMethod()
  return 1337
end

print (clazz:newMethod(), ExtensibleClass.newStaticMethod())
```

This way extending an already existing method will raise a lua error when trying to do so. To be able to override existing methods, pass `luabridge::allowOverridingMethods` together with `luabridge::extensibleClass`.

```cpp
struct ExtensibleClass
{
  int existingMethod() { return 42; }
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<ExtensibleClass> ("ExtensibleClass", luabridge::extensibleClass | luabridge::allowOverridingMethods)
      .addFunction ("existingMethod", &ExtensibleClass::existingMethod)
    .endClass ()
  .endNamespace ();

ExtensibleClass clazz;
luabridge::pushGlobal (L, &clazz, "clazz");
```

```lua
function ExtensibleClass:existingMethod()
  return "this has been replaced"
end

print (clazz:existingMethod())
```


In case storing instance properties is needed, storage needs to be provided per instance. See the next chapter for an explanation on how to add custom properties per instance.

## Index and New Index Metamethods Fallback

In general LuaBridge for each class will add a `__index` and `__newindex` metamethods in order to be able to handle member function, properties and inheritance resolution. This will make it impossible for a user to override them because in doing so, we'll make the exposed classes non functioning. Although possible to override those metamethods directly, they will preclude any possibility to locate exposed members and properties in such classes.

If a LuaBridge exposed class still need to handle the case of handling `__index` and `__newindex` metamethods calls, it's possible to use the `addIndexMetaMethod` and `addNewIndexMetaMethod` registration functions that will be executed as fallback in case an already existing function/property is not exposed in the class itself or any of its parent.

```cpp
struct FlexibleClass
{
  int propertyOne = 42;
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<FlexibleClass> ("FlexibleClass")
      .addProperty ("propertyOne", &FlexibleClass::propertyOne, &FlexibleClass::propertyOne)
      .addIndexMetaMethod ([] (FlexibleClass& self, const luabridge::LuaRef& key, lua_State* L)
      {
        if (key.tostring () == "existingProperty")
          return luabridge::LuaRef (L, 1337);

        return luabridge::LuaRef (L, luabridge::LuaNil ()); // or luaL_error("Failed lookup of key !")
      })
    .endClass ()
  .endNamespace ();

FlexibleClass flexi;
luabridge::pushGlobal (L, &flexi, "flexi");
```

Then in lua:

```lua
propertyOne = flexi.propertyOne
assert (propertyOne == 42, "Getting value from LuaBridge exposed property")

propertyTwo = flexi.existingProperty
assert (propertyTwo == 1337, "Getting value from non exposed LuaBridge property via __index fallback")

propertyThree = flexi.nonExistingProperty
assert (propertyThree == nil, "Getting value from non exposed LuaBridge property via __index fallback")
```

The same can be done for the `__newindex` metamethod fallback:

```cpp
struct FlexibleClass
{
  std::unordered_map<luabridge::LuaRef, luabridge::LuaRef> properties;
};

luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .beginClass<FlexibleClass> ("FlexibleClass")
      .addIndexMetaMethod ([] (FlexibleClass& self, const luabridge::LuaRef& key, lua_State* L)
      {
        auto it = self.properties.find(key);
        if (it != self.properties.end())
          return it->second;

        return luabridge::LuaRef (L, luabridge::LuaNil ()); // or luaL_error("Failed lookup of key !")
      })
      .addNewIndexMetaMethod ([] (FlexibleClass& self, const luabridge::LuaRef& key, const luabridge::LuaRef& value, lua_State* L)
      {
        self.properties.emplace (std::make_pair (key, value))
        return luabridge::LuaRef (L, luabridge::LuaNil ());
      })
    .endClass ()
  .endNamespace ();

FlexibleClass flexi;
luabridge::pushGlobal (L, &flexi, "flexi");
```

Then in lua:

```lua
propertyOne = flexi.propertyOne
assert (propertyOne == nil, "Value is not existing")

flexi.propertyOne = 1337

propertyOne = flexi.propertyOne
assert (propertyOne == 1337, "Value is now present !")
```

## Static Index and New Index Metamethods Fallback

The same fallback mechanism is available for the *static class table* - i.e. for key lookups performed on the class name itself (e.g. `MyClass.someKey`) rather than on an instance.  Use `addStaticIndexMetaMethod` and `addStaticNewIndexMetaMethod` to register the callbacks.  Unlike their instance counterparts, the static callbacks receive only the key (and optionally `lua_State*`) - there is no `self` parameter.

```cpp
struct MyClass {};

std::unordered_map<std::string, int> store;

luabridge::getGlobalNamespace (L)
  .beginClass<MyClass> ("MyClass")
    .addStaticIndexMetaMethod ([] (const luabridge::LuaRef& key, lua_State* L) -> luabridge::LuaRef
    {
      auto it = store.find (key.tostring ());
      if (it != store.end ())
        return luabridge::LuaRef (L, it->second);

      return luabridge::LuaRef (L, luabridge::LuaNil ());
    })
    .addStaticNewIndexMetaMethod ([] (const luabridge::LuaRef& key, const luabridge::LuaRef& value, lua_State* L) -> luabridge::LuaRef
    {
      if (value.isNumber ())
        store[key.tostring ()] = value.unsafe_cast<int> ();
      return value;
    })
  .endClass ();
```

Then in lua:

```lua
MyClass.dynamicProp = 42

value = MyClass.dynamicProp
assert (value == 42, "Value stored via static __newindex fallback and retrieved via static __index fallback")

missing = MyClass.nonExistingKey
assert (missing == nil, "Unknown key returns nil through the static __index fallback")
```

Existing static properties and functions registered with `addStaticProperty` / `addStaticFunction` are found *after* the fallback is consulted.  If the fallback callback returns `nil` (or a nil `LuaRef`) for a given key, normal lookup continues and the real property or function is returned.  Conversely, if the callback returns a non-nil value the fallback result takes priority over any registered static property with the same name.

```cpp
struct MyClass
{
  static int answer () { return 42; }
};

luabridge::getGlobalNamespace (L)
  .beginClass<MyClass> ("MyClass")
    .addStaticFunction ("answer", &MyClass::answer)
    .addStaticIndexMetaMethod ([] (const luabridge::LuaRef& /*key*/, lua_State* L) -> luabridge::LuaRef
    {
      // Returning nil lets the registered static function be found normally
      return luabridge::LuaRef (L, luabridge::LuaNil ());
    })
  .endClass ();
```

Then in lua:

```lua
-- The registered static function is still callable because the fallback returned nil
result = MyClass.answer ()
assert (result == 42)
```
