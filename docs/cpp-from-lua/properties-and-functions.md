# Properties and Functions

These are registered into a namespace using `addProperty` and `addFunction`. When registered functions are called by scripts, LuaBridge automatically takes care of the conversion of arguments into the appropriate data type when doing so is possible. This automated system works for the function's return value, and up to 8 parameters although more can be added by extending the templates. Pointers, references, and objects of class type as parameters are treated specially, and explained later.

If we have:

```cpp
int globalVar;
static float staticVar;

std::string stringProperty;
std::string getString () { return stringProperty; }
void setString (std::string s) { stringProperty = s; }

std::tuple <int, std::string> tuple;

int foo () { return 42; }
void bar (char const*) { }
int cFunc (lua_State* L) { return 0; }
```

These are registered with:

```cpp
luabridge::getGlobalNamespace (L)
  .beginNamespace ("test")
    .addProperty ("var1", &globalVar) // read-only
    .addProperty ("var2", &staticVar, &staticVar) // read-write
    .addProperty ("prop1", getString) // read-only
    .addProperty ("prop2", getString, setString) // read-write
    .addProperty ("tup1", &tuple) // read-only
    .addProperty ("tup2", &tuple, &tuple) // read-write
    .addFunction ("foo", foo)
    .addFunction ("bar", bar)
    .addFunction ("cfunc", cFunc)
  .endNamespace ();
```

Variables can be marked _read-only_ by passing `false` in the second optional parameter. If the parameter is omitted, _true_ is used making the variable read/write. Properties are marked read-only by omitting the set function. After the registrations above, the following Lua identifiers are valid:

```lua
test        -- a namespace
test.var1   -- a read-only lua_Number property
test.var2   -- a read-write lua_Number property
test.prop1  -- a read-only lua_String property
test.prop2  -- a read-write lua_String property
test.tup1   -- a read-only lua_Table property mapping to a c++ tuple
test.tup2   -- a read-write lua_Table property mapping to a c++ tuple
test.foo    -- a function returning a lua_Number
test.bar    -- a function taking a lua_String as a parameter
test.cfunc  -- a function with a variable argument list and multi-return
```

Note that `test.prop1` and `test.prop2` both refer to the same value. However, since `test.prop2` is read-only, assignment attempts will generate a run-time error. These Lua statements have the stated effects:

```lua
test.var1 = 5          -- error: var1 is not writable
test.var2 = 6          -- okay
test.prop1 = "bar"     -- error: prop1 is not writable
test.prop2 = "Hello"   -- okay
test.prop2 = 68        -- okay, Lua converts the number to a string
test.tup1 = { 1, "a" } -- error: tup1 is not writable
test.tup2 = { 1, "a" } -- okay, converts a table to tuple with the same size
test.tup2 = { "size" } -- error: table has different size than tuple

test.foo ()            -- calls foo and discards the return value
test.var1 = foo ()     -- calls foo and stores the result in var1
test.bar ("Employee")  -- calls bar with a string
test.bar (test)        -- error: bar expects a string not a table
```
