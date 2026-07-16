# API Reference

## Global Options

```cpp
/// Flag set of options
class Options : FlagSet<uint32_t>;

/// Default options for classes / namespaces registration.
Option defaultOptions;

/// Specify that class methods should allow to the extended by lua scripts.
Option extensibleClass;

/// Allow an extensible class to overriding C++ exposed methods.
Option allowOverridingMethods;

/// Allow access to class / namespace metatables.
Option visibleMetatables;
```

## Free Functions

```cpp
/// Enable exceptions globally. Will translate lua_errors into C++ LuaExceptions. Usable only if compiled with C++ exceptions enabled.
void enableExceptions (lua_State* L);

/// Gets a global Lua variable reference as LuaRef.
LuaRef getGlobal (lua_State* L, const char* name);

/// Gets a global Lua variable reference as type T.
template <class T>
TypeResult<T> getGlobal (lua_State* L, const char* name);

/// Sets a global Lua variable. Throws or return false if the class is not registered.
template <class T>
bool setGlobal (lua_State* L, T* varPtr, const char* name);

/// Gets the global namespace registration object.
Namespace getGlobalNamespace (lua_State* L);

/// Gets a namespace registration object using a table on top of the stack.
Namespace getNamespaceFromStack (lua_State* L);

/// Invokes a LuaRef if it references a lua callable.
template <class R = void, class... Args>
TypeResult<R> call (const LuaRef& object, Args&&... args);

/// Invokes a LuaRef with a custom Lua error message handler.
template <class R = void, class F, class... Args>
TypeResult<R> callWithHandler (const LuaRef& object, F&& errorHandler, Args&&... args);

/// Wraps a C++ callable into a LuaRef representing a Lua function.
template <class F>
LuaRef newFunction (lua_State* L, F&& func);

/// Wrapper for lua_pcall, converting lua errors into C++ exceptions if they are enabled.
int pcall (lua_State* L, int nargs = 0, int nresults = 0, int msgh = 0);

/// Return a range iterable view over a lua table.
Range pairs (const LuaRef& table);
```

## Namespace Registration - Namespace

```cpp
/// Begin or continues namespace registration, returns this namespace object.
template <class T>
Namespace beginNamespace (const char* name);

/// Ends namespace registration, returns the parent namespace object.
template <class T>
Namespace endNamespace ();

/// Registers one or multiple overloaded functions.
template <class... Functions>
Namespace addFunction (const char* name, Functions... functions);

/// Registers a readonly property with only a getter.
template <class Getter>
Namespace addProperty (const char* name, Getter getter);

/// Registers a readwrite property with a getter and a setter.
template <class Getter, class Setter>
Namespace addProperty (const char* name, Getter getter, Setter setter);
```

## Class Registration - Class\<T\>

```cpp
/// Begins or continues class registration, returns this class object.
template <class T>
Class<T> beginClass (const char* name);

/// Begins derived class registration with one or more base classes, returns this class object.
template <class T, class Base1, class... Bases>
Class<T> deriveClass (const char* name);

/// Ends class registration, returns the parent namespace object.
template <class T>
Namespace endClass ();
```

### Constructor Registration

```cpp
/// Registers one or multiple overloaded constructors for type T.
template <class... Functions>
Class<T> addConstructor ();

/// Registers one or multiple overloaded constructors for type T using callable arguments.
template <class... Functions>
Class<T> addConstructor (Functions... functions);

/// Registers one or multiple overloaded constructors for type T when usable from intrusive container C.
template <class C, class... Functions>
Class<T> addConstructorFrom ();

/// Registers one or multiple overloaded constructors for type T when usable from intrusive container C using callable arguments.
template <class C, class... Functions>
Class<T> addConstructorFrom (Functions... functions);

/// Registers allocator and deallocators for type T.
template <class Alloc, class Dealloc>
Class<T> addFactory (Alloc alloc, Dealloc dealloc);

/// Registers a destructor hook called just before the C++ destructor (__destruct metamethod).
template <class Function>
Class<T> addDestructor (Function function);
```

### Member Function Registration

```cpp
/// Registers one or multiple overloaded functions as member functions.
template <class... Functions>
Class<T> addFunction (const char* name, Functions... functions);
```

### Member Property Registration

```cpp
/// Registers a readonly property with a getter.
template <class Getter>
Class<T> addProperty (const char* name, Getter getter);

/// Registers a readwrite property with a getter and a setter.
template <class Getter>
Class<T> addProperty (const char* name, Getter getter, Setter setter);
```

### Static Function Registration

```cpp
/// Registers one function or multiple overloads.
template <class... Functions>
Class<T> addStaticFunction (const char* name, Functions... functions);
```

### Static Property Registration

```cpp
/// Registers a static readonly property with a getter.
template <class Getter>
Class<T> addStaticProperty (const char* name, Getter getter);

/// Registers a static readwrite property with a getter and a setter.
template <class Getter>
Class<T> addStaticProperty (const char* name, Getter getter, Setter setter);
```

### Metamethod Registration

```cpp
/// Registers a fallback __index handler for instances (called when a key is not found).
template <class Function>
Class<T> addIndexMetaMethod (Function function);

/// Registers a fallback __newindex handler for instances (called when a key is not found).
template <class Function>
Class<T> addNewIndexMetaMethod (Function function);

/// Registers a fallback __index handler for the static class table.
template <class Function>
Class<T> addStaticIndexMetaMethod (Function function);

/// Registers a fallback __newindex handler for the static class table.
template <class Function>
Class<T> addStaticNewIndexMetaMethod (Function function);
```

### Converter Registration

```cpp
/// Registers a converter from this class type T to target type To.
/// Requires StackConversion<To>::enabled and StackConverter<To, T>.
template <class To>
Class<T> addConverter ();
```

## Lua Variable Reference - LuaRef

```cpp
/// Creates a nil reference.
LuaRef (lua_State* L);

/// Returns native Lua string representation.
std::string tostring () const;

/// Dumps reference to a stream.
void print (std::ostream& stream) const;

/// Returns the Lua state.
lua_State* state () const;

/// Place the object onto the Lua stack.
void push (lua_State* L);

/// Indicate whether it is a valid reference (not a LUA_NOREF).
bool isValid () const;

/// Return the lua_type.
int type () const;

/// Indicate whether it is a nil reference.
bool isNil () const;

/// Indicate whether it is a reference to a boolean.
bool isBool () const;

/// Indicate whether it is a reference to a number.
bool isNumber () const;

/// Indicate whether it is a reference to a string.
bool isString () const;

/// Indicate whether it is a reference to a table.
bool isTable () const;

/// Indicate whether it is a reference to a function.
bool isFunction () const;

/// Indicate whether it is a reference to a full userdata.
bool isUserdata () const;

/// Indicate whether it is a reference to a light userdata.
bool isLightUserdata () const;

/// Indicate whether it is a reference to a Lua thread.
bool isThread () const;

/// Indicate whether it is a callable, can be either a lua function or an object with the __call metamethod.
bool isCallable () const;

/// Perform implicit type conversion.
template <class T>
operator T () const;

/// Perform the explicit type conversion, safe.
template <class T>
TypeResult<T> cast () const;

/// Perform the explicit type conversion, unsafe (throws or abort on failure).
template <class T>
T unsafe_cast () const;

/// Check if the Lua value is convertible to the type T.
template <class T>
bool isInstance () const;

/// Get the metatable for the LuaRef.
LuaRef getMetatable () const;

/// Compare this reference with a specified value using lua_compare(). This invokes metamethods.
template <class T>
bool operator== (T rhs) const;

/// Compare this reference with a specified value using lua_compare(). This invokes metamethods.
template <class T>
bool operator!= (T rhs) const;

/// Compare this reference with a specified value using lua_compare(). This invokes metamethods.
template <class T>
bool operator< (T rhs) const;

/// Compare this reference with a specified value using lua_compare(). This invokes metamethods.
template <class T>
bool operator<= (T rhs) const;

/// Compare this reference with a specified value using lua_compare(). This invokes metamethods.
template <class T>
bool operator> (T rhs) const;

/// Compare this reference with a specified value using lua_compare(). This invokes metamethods.
template <class T>
bool operator>= (T rhs) const;

/// Compare this reference with a specified value using lua_compare(). This does not invoke metamethods.
template <class T>
bool rawequal (T v) const;

/// Append one or more values to a referred table. If the table is a sequence this will add more elements to it.
/// Uses lua_rawseti internally. Returns true if all values were successfully appended.
template <class... Ts>
bool append (const Ts&... vs) const;

/// Return the length of a referred array. This is identical to applying the Lua # operator.
int length () const;

/// Invoke the lua ref with no expected return value.
template <class... Args>
TypeResult<void> operator() (Args&&... args) const;

/// Invoke the lua ref and decode the return value to R.
template <class R = void, class... Args>
TypeResult<R> call (Args&&... args) const;

/// Invoke the lua ref with an error handler and decode the return value to R.
template <class R = void, class F, class... Args>
TypeResult<R> callWithHandler (F&& errorHandler, Args&&... args) const;

/// Build a strongly-typed callable wrapper from this Lua object.
template <class Signature>
LuaFunction<Signature> callable () const;

/// Wrap a C++ callable into a new Lua function returned as a LuaRef.
template <class F>
static LuaRef newFunction (lua_State* L, F&& func);
```

## Lua Nil Special Value - LuaNil

```cpp
/// LuaNil can be used to construct LuaRef.
```

## TypeResult\<T\> - Result of a Call or Cast

```cpp
explicit operator bool() const;

/// Return the contained value (undefined behavior if result holds an error).
const T& value() const;

/// Dereference operator - equivalent to value().
T& operator*();

/// Return the contained value or a default when the result holds an error.
template <class U>
T valueOr(U&& defaultValue) const;

/// Return the error code, if any.
std::error_code error() const;

/// Return the error message string, if any.
std::string message() const;
```

## Typed Lua Function Wrapper - LuaFunction\<R(Args...)\>

```cpp
/// Construct from a LuaRef.
explicit LuaFunction (const LuaRef& function);

/// Call the function with the given arguments.
TypeResult<R> operator() (Args... args) const;

/// Call the function - equivalent to operator().
TypeResult<R> call (Args... args) const;

/// Call the function with a custom error handler.
template <class F>
TypeResult<R> callWithHandler (F&& errorHandler, Args... args) const;

/// Return true if the underlying LuaRef is callable.
bool isValid () const;

/// Return the underlying LuaRef.
const LuaRef& ref () const;
```

## Stack Traits - Stack\<T\>

```cpp
/// Opt-in trait for enabling registered converter lookup for target type T.
template <class T>
struct StackConversion
{
  static constexpr bool enabled = false;
};

/// User-specialized conversion hook from source type From to target type To.
template <class To, class From>
struct StackConverter
{
  static To convert (const From& from);
};

/// Converts the C++ value into the Lua value at the top of the Lua stack. Returns true if the push could be performed.
/// When false is returned, `ec` contains the error code corresponding to the failure.
Result push (lua_State* L, const T& value);

/// Converts the Lua value at the index into the C++ value of the type T.
TypeResult<T> get (lua_State* L, int index);

/// Checks if the Lua value at the index is convertible into the C++ value of the type T.
bool isInstance (lua_State* L, int index);
```

## C++20 Coroutine Types (requires `LUABRIDGE_HAS_CXX20_COROUTINES`)

```cpp
/// Coroutine return type for C++ generators callable from Lua.
/// R may be any LuaBridge-registered type, or void.
template <class R>
struct CppCoroutine;

/// Awaitable wrapper for a Lua thread. Use inside a CppCoroutine body with co_await.
/// Resumes the child thread synchronously and returns {status, nresults}.
class LuaCoroutine
{
public:
    /// thread  - the Lua thread to resume.
    /// from    - the calling Lua state (typically L inside the coroutine body).
    LuaCoroutine(lua_State* thread, lua_State* from = nullptr) noexcept;

    std::pair<int, int> await_resume() noexcept; // returns {status, nresults}
};

/// Register a CppCoroutine factory in a namespace (available on Namespace).
/// F must be a callable whose return type is CppCoroutine<R>.
template <class F>
Namespace& addCoroutine(const char* name, F factory);

/// Register a CppCoroutine factory as a static class function (available on Class<T>).
/// F must be a callable whose return type is CppCoroutine<R>.
template <class F>
Class<T>& addStaticCoroutine(const char* name, F factory);

/// Register a CppCoroutine factory as a member method (available on Class<T>).
/// F must be a callable whose first argument is T* or const T*, and whose return type is
/// CppCoroutine<R>. A const T* first argument registers the method as const-accessible.
template <class F>
Class<T>& addCoroutine(const char* name, F factory);

/// Portable lua_resume wrapper - fills *nresults on all Lua versions (5.1–5.5).
int lua_resume_x(lua_State* L, lua_State* from, int nargs, int* nresults = nullptr);

/// Returns true if the current C function can yield via lua_yieldk.
bool lua_isyieldable_x(lua_State* L);
```
