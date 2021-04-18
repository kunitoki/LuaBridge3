## Master

## Version 3.0

* Moved to C++17 as minimum supported standard C++ version.
* Reworked the whole library to be able to use it without c++ exceptions enabled.
* Breaking Change: The method `Stack<T>::push` now takes a `std::error_code&` as last parameter and returns a `bool`.
* Breaking Change: The class `LuaException` has been reworked and it now take a `std::error_code` instead of a int.
* Breaking Change: The class `LuaException` is now thrown if a unregistered class is pushed via the Stack class, also when calling `LuaRef::operator()`, but only if exceptions are enabled.
* Breaking Change: `LuaRef::operator()` now returns the class `LuaResult`, where it is possible to obtain the call results or error message.
* Breaking Change: LuaBridge does not silently enable exceptions when calling `getGlobalNamespace`. Call `enableExceptions(lua_State*)` if you want to enable them explicitly.
* Breaking Change: Removed `RefCounterPtr`, maintaining the reference counts in a unsynchronized global table is not production quality.
* Breaking Change: Removed `Class<T>::addStaticData`, it was just an alias for `Class<T>::addStaticProperty`.
* Breaking Change: Removed `Class<T>::addCFunction`, it was just an alias for `Class<T>::addFunction`.
* Breaking Change: Removed `Class<T>::addStaticCFunction`, it was just an alias for `Class<T>::addStaticFunction`.
* Allow specifying a non virtual base class method when declaring class members (functions or variables) not exposed in the inherited class.
* Allow using capturing lambdas in `Namespace::addFunction` and `Class<T>::addFunction`.
* Added support for specifying factory functor in `Class<T>::addConstructor` to do placement new of the object instance.
* Allow using capturing lambdas in `Namespace::addProperty`.
* Added `getNamespaceFromStack` function to construct a namespace object from a table on the stack.
* Added `std::shared_ptr` support for types intrusively deriving from `std::enable_shared_from_this`.
* Added `Class<T>::addFunction` overload taking a `lua_CFunction` as if it were a member.
* Added `LuaRef::isValid` to check when the reference is a LUA_NOREF.
* Added support for `std::byte` as stack value type.
* Added support for `std::string_view` as stack value type.
* Added support for `std::tuple` as stack value type.
* Added support for `std::optional` as stack value type by using `LuaBridge/Optional.h`.
* Added support for `std::set` as stack value type by using `LuaBridge/Set.h`.
* Added single header amalgamated distribution file, to simplify including in projects.
* Added more asserts for functions and property names.
* Renamed `luabridge::Nil` to `luabridge::LuaNil` to allow including LuaBridge in Obj-C sources.
* Removed the limitation of maximum 8 parameters in functions.
* Removed the limitation of maximum 8 parameters in constructors.
* Removed `Class<T>::addData`, it was just an alias for `Class<T>::addProperty`.
* Removed `TypeList` from loki, using parameter packs and `std::tuple` with `std::apply`.
* Removed juce traces from unit tests, simplified unit tests runs.
* Fixed issue when `LuaRef::cast<>` fails with exceptions enabled, popping from the now empty stack could trigger the panic handler twice.
* Fixed unaligned access in user allocated member pointers in 64bit machines reported by ASAN.
* Bumped lua 5.2.x in unit tests from lua 5.2.0 to 5.2.4.
* Bumped lua 5.4.x in unit tests from lua 5.4.1 to 5.4.3.
* Run against lua 5.3.6 and 5.4.3 in unit tests.
* Converted the manual from html to markdown.
* Small improvements to code and doxygen comments readability.

## Version 2.6

* Added namespace `addFunction()` accepting `std::function` (C++11 only).
* Added class `addStaticFunction()` accepting `std::function` (C++11 only).
* Update the Doxygen documentation.
* Add brief API reference into the manual.
* Hide non-public `luabridge` members into the `detail` namespace.
* Fix stack cleanup by `LuaRef::isInstance()` method.

## Version 2.5

* Introduce stack `isInstance()` method.
* Introduce LuaRef `isInstance()` method.
* Added a convenience `isInstance()` function template.

## Version 2.4.1

* Do not call the object destructor then its constructor throws.

## Version 2.4

* String stack get specialization doesn't change the stack value anymore.
* Added namespace `addProperty()` accepting C-functions.
* Introduce enableExceptions function.

## Version 2.3.2

* Fixed registration continuation for an already registered class.

## Version 2.3.1

* Fixed registration continuation issues.

## Version 2.3

* Added class `addFunction()` accepting proxy functions (C++11 only).
* Added class `addFunction()` accepting `std::function` (C++11 only).
* Added class `addProperty()` accepting functions with lua_State* parameter.
* Added class `addProperty()` accepting `std::function` (C++11 only).
* Added stack traits for `std::unordered_map` (`UnorderedMap.h`).
* Now using lightuserdata for function pointers.

## Version 2.2.2

* Performance optimization.

## Version 2.2.1

* Refactored namespace and class handling.

## Version 2.2

* Refactored stack operations.
* Handle exceptions in stack operations.

## Version 2.1.2

* Added `operator==` and `operator!=` for `RefCountedPtr` template.

## Version 2.1.1

* Support for `__stdcall` function pointers.

## Version 2.1

* Added stack traits for `std::vector` (`Vector.h`).
* Added stack traits for `std::list` (`List.h`).
* Added stack traits for `std::map` (`Map.h`).
* Added ability to use `LuaRef` objects as an `std::map` keys.
* Fixed some manual errata.

## Version 2.0

* Numerous bug fixes.
* Feature Requests from Github issues.
* Added `LuaRef` object.
* Rewritten documentation.

## Version 1.1.0

* Split code up into several files.
* Add Lua table and type representations (based on Nigel's code).
* Reformat documentation as external HTML file.

## Version 1.0.3

* Pass `nil` to Lua when a null pointer is passed for objects with shared lifetime.

## Version 1.0.2

* Option to hide metatables selectable at runtime, default to true.
* `addStaticMethod()` renamed to `addStaticFunction()` for consistency.
* `addMethod()` renamed to `addFunction()` for consistency.
* `addCFunction()` registrations.
* Convert null pointers to and from `nil`.
* Small performance increase in class pointer extraction.

## Version 1.0.1

* Backward compatibility with Lua 5.1.x.

## Version 1.0

* Explicit lifetime management models.
* Generalized containers.
* Single header distribution.
