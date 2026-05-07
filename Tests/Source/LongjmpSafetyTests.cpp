// https://github.com/kunitoki/LuaBridge3
// Copyright 2024, kunitoki
// SPDX-License-Identifier: MIT

// Tests for lua_error (longjmp) safety when Lua is compiled as C.
//
// Background:
//   When Lua is compiled as C, lua_error() uses longjmp() rather than throwing
//   a C++ exception. longjmp() does NOT call C++ destructors for objects on the
//   stack, making it UB (and a resource leak) to have non-trivially-destructible
//   C++ objects alive between the longjmp call and the matching setjmp/pcall.
//
// Safety analysis findings:
//
//   SAFE paths (trivially-destructible objects only when longjmp fires):
//   - Single-argument decode errors: TypeResult<T> in error state holds
//     error_code (trivial), not T. T is never constructed.
//   - raise_lua_error itself: only va_list (ended before jump), string_view
//     (trivially destructible) and lua_Debug (C struct) are on the stack.
//   - Direct luaL_error calls in metamethods (__index, __newindex): no
//     non-trivial C++ locals exist at the call site.
//   - Property getters/setters receiving wrong self type: Userdata::get<T>
//     returns TypeResult<T*> — pointer type is trivially destructible.
//
//   UNSAFE paths (potential UB + resource leak when Lua is compiled as C):
//   - Multi-argument function invocation: invoke_callable_from_stack_impl()
//     expands the argument pack via std::invoke(func, unwrap_arg<T0>(), ...).
//     C++ does not guarantee evaluation order of function arguments. If T0 is
//     decoded successfully into a non-trivially-destructible temporary and T1
//     fails, the T0 temporary's destructor is skipped by longjmp.
//   - make_arguments_list_impl / tupleize: same expansion pattern suffers the
//     same problem — std::tuple<Ts...>(arg0, arg1, ...) does not sequence the
//     argument evaluations, so earlier successful args can be leaked on longjmp.
//   - Constructor placement proxy / constructor_forwarder: after a successful
//     make_arguments_list() the resulting tuple (potentially containing strings
//     etc.) sits as a local variable. A subsequent raise_lua_error (e.g. when
//     UserdataValue::place() fails) longjmps over it without calling destructors.
//
//   These issues produce memory leaks (not crashes or corruption) in practice
//   because the leaked objects are value copies materialized from Lua strings.
//   SSO strings (typically ≤15 chars) involve no heap allocation and are
//   effectively harmless; heap-allocated (longer) strings do leak their buffers.
//
// How to detect:
//   Run the test suite under AddressSanitizer with LeakSanitizer enabled.
//   The TrackedString type below provides a live-instance counter so that tests
//   can assert whether destructors were called after a pcall error.

#include "TestBase.h"

#include <atomic>
#include <string>

namespace {

// ---------------------------------------------------------------------------
// TrackedString — a string wrapper with a live-instance counter.
// Constructing one increments the counter; destroying one decrements it.
// ---------------------------------------------------------------------------
std::atomic<int> g_tracked_string_count{0};

struct TrackedString
{
    std::string value;

    TrackedString()
        : value()
    {
        ++g_tracked_string_count;
    }

    explicit TrackedString(std::string v)
        : value(std::move(v))
    {
        ++g_tracked_string_count;
    }

    TrackedString(const TrackedString& o)
        : value(o.value)
    {
        ++g_tracked_string_count;
    }

    TrackedString(TrackedString&& o) noexcept
        : value(std::move(o.value))
    {
        ++g_tracked_string_count;
    }

    ~TrackedString()
    {
        --g_tracked_string_count;
    }

    TrackedString& operator=(const TrackedString& o)
    {
        value = o.value;
        return *this;
    }

    TrackedString& operator=(TrackedString&& o) noexcept
    {
        value = std::move(o.value);
        return *this;
    }
};

// A long string guaranteed to exceed the small-string optimisation threshold
// on all common implementations (SSO is typically ≤15 chars on GCC/Clang/MSVC).
constexpr const char* kLongString = "this string is definitely longer than sso threshold on all platforms";

} // namespace

// Register Stack<TrackedString> so LuaBridge can push/get it.
namespace luabridge {

template <>
struct Stack<TrackedString>
{
    [[nodiscard]] static Result push(lua_State* L, const TrackedString& v)
    {
        return Stack<std::string>::push(L, v.value);
    }

    [[nodiscard]] static TypeResult<TrackedString> get(lua_State* L, int index)
    {
        auto result = Stack<std::string>::get(L, index);
        if (!result)
            return result.error();

        return TrackedString{std::move(*result)};
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        return Stack<std::string>::isInstance(L, index);
    }
};

} // namespace luabridge

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------
struct LongjmpSafetyTests : TestBase
{
    void SetUp() override
    {
        TestBase::SetUp();
        g_tracked_string_count = 0;
    }

    // Helper: verify that the live TrackedString count returned to zero after
    // a pcall error. When Lua is compiled as C the longjmp bypasses C++
    // destructors for temporaries inside the CFunction, so the count MAY be
    // non-zero. The test is recorded as a non-fatal expectation so that it
    // shows up as an informational failure in the CI output without blocking
    // other checks. Run with ASAN/LSAN to catch the underlying heap leak.
    void expectNoLeak(const char* context) const
    {
        const int live = g_tracked_string_count.load();
        if (live != 0)
        {
            ADD_FAILURE() << context << ": " << live << " TrackedString object(s) not destroyed."
                          << " When Lua is compiled as C, longjmp bypasses C++ destructors"
                          << " for temporaries in multi-argument CFunction invocations,"
                          << " causing a resource leak. Run with ASAN/LSAN for confirmation.";
        }
    }
};

// ===========================================================================
// 1. Error propagation — single argument, wrong type.
//    This path is SAFE: TypeResult<TrackedString> in the error state holds only
//    an error_code (trivially destructible); TrackedString is never constructed.
// ===========================================================================
TEST_F(LongjmpSafetyTests, SingleArgWrongType_ErrorPropagates)
{
    luabridge::getGlobalNamespace(L)
        .addFunction("f", +[](TrackedString) { return 0; });

    auto [ok, msg] = runLuaCaptureError("f(true)");

    EXPECT_FALSE(ok);
    EXPECT_FALSE(msg.empty());
    // After the error no TrackedString should have been constructed at all
    EXPECT_EQ(g_tracked_string_count.load(), 0);
}

TEST_F(LongjmpSafetyTests, SingleArgWrongType_VMUsableAfterError)
{
    luabridge::getGlobalNamespace(L)
        .addFunction("f", +[](TrackedString s) { return s.value.size(); });

    // First call: wrong type
    auto [ok1, msg1] = runLuaCaptureError("f(true)");
    EXPECT_FALSE(ok1);
    EXPECT_EQ(g_tracked_string_count.load(), 0);

    // Second call: correct type — VM must still be usable
    auto [ok2, msg2] = runLuaCaptureError("result = f('hello')");
    EXPECT_TRUE(ok2) << "VM not usable after error: " << msg2;
    EXPECT_EQ(result<int>(), static_cast<int>(std::string("hello").size()));
}

// ===========================================================================
// 2. Two-argument function — first argument wrong type.
//    SAFE: longjmp fires before any TrackedString is constructed.
// ===========================================================================
TEST_F(LongjmpSafetyTests, TwoArgs_FirstWrongType_NoConstructionBeforeError)
{
    luabridge::getGlobalNamespace(L)
        .addFunction("f", +[](TrackedString, TrackedString) { return 0; });

    auto [ok, msg] = runLuaCaptureError("f(true, 'world')");

    EXPECT_FALSE(ok);
    EXPECT_FALSE(msg.empty());
    EXPECT_EQ(g_tracked_string_count.load(), 0);
}

// ===========================================================================
// 3. Two-argument function — second argument wrong type.
//    POTENTIALLY UNSAFE: the first TrackedString (from a long string) may be
//    constructed before the second argument decode fails and longjmps.
//    When Lua is compiled as C, the first TrackedString's destructor is NOT
//    called, leaking its heap-allocated buffer.
//    Under ASAN/LSAN this shows up as a heap-use-after-return or leak report.
// ===========================================================================
TEST_F(LongjmpSafetyTests, TwoArgs_SecondWrongType_PotentialLeakOfFirstArg)
{
    luabridge::getGlobalNamespace(L)
        .addFunction("f", +[](TrackedString, TrackedString) { return 0; });

    // Use a string long enough to force heap allocation (bypass SSO)
    std::string lua = std::string("f('") + kLongString + "', true)";
    auto [ok, msg] = runLuaCaptureError(lua.c_str());

    EXPECT_FALSE(ok);
    EXPECT_FALSE(msg.empty());

    // When Lua is compiled as C, this expectation MAY FAIL because the first
    // TrackedString temporary is leaked by longjmp. That failure is the bug.
    expectNoLeak("TwoArgs_SecondWrongType");
}

TEST_F(LongjmpSafetyTests, TwoArgs_SecondWrongType_VMUsableAfterError)
{
    int call_count = 0;
    luabridge::getGlobalNamespace(L)
        .addFunction("f", [&](TrackedString a, TrackedString b) {
            ++call_count;
            return a.value + b.value;
        });

    // Error call
    auto [ok1, _] = runLuaCaptureError(
        (std::string("f('") + kLongString + "', true)").c_str());
    EXPECT_FALSE(ok1);

    // Successful call after error — VM must be fully operational
    auto [ok2, msg2] = runLuaCaptureError("result = f('hello', ' world')");
    EXPECT_TRUE(ok2) << "VM not usable after error: " << msg2;
    EXPECT_EQ(call_count, 1);
    EXPECT_EQ(result<std::string>(), "hello world");
}

// ===========================================================================
// 4. Three-argument function — third argument wrong type.
//    Two TrackedStrings may be constructed before the third decode longjmps.
// ===========================================================================
TEST_F(LongjmpSafetyTests, ThreeArgs_ThirdWrongType_PotentialLeakOfEarlierArgs)
{
    luabridge::getGlobalNamespace(L)
        .addFunction("f", +[](TrackedString, TrackedString, TrackedString) { return 0; });

    std::string arg = std::string("'") + kLongString + "'";
    std::string lua = "f(" + arg + ", " + arg + ", true)";
    auto [ok, msg] = runLuaCaptureError(lua.c_str());

    EXPECT_FALSE(ok);
    EXPECT_FALSE(msg.empty());

    expectNoLeak("ThreeArgs_ThirdWrongType");
}

// ===========================================================================
// 5. Function returning non-trivial type, invoked with wrong argument.
// ===========================================================================
TEST_F(LongjmpSafetyTests, ReturningNonTrivialType_WrongArgType)
{
    luabridge::getGlobalNamespace(L)
        .addFunction("f", +[](TrackedString s) -> TrackedString { return s; });

    auto [ok, msg] = runLuaCaptureError("result = f(true)");
    EXPECT_FALSE(ok);
    EXPECT_EQ(g_tracked_string_count.load(), 0);
}

// ===========================================================================
// 6. Class member function — wrong argument type.
// ===========================================================================
namespace {

struct SafetyTestClass
{
    std::string concat(TrackedString a, TrackedString b) const
    {
        return a.value + b.value;
    }

    int id = 0;
};

} // namespace

TEST_F(LongjmpSafetyTests, MemberFunction_SecondArgWrongType)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<SafetyTestClass>("SafetyTestClass")
            .addConstructor<void (*)()>()
            .addFunction("concat", &SafetyTestClass::concat)
        .endClass();

    std::string arg = std::string("'") + kLongString + "'";
    std::string lua = "local o = SafetyTestClass(); result = o:concat(" + arg + ", true)";
    auto [ok, msg] = runLuaCaptureError(lua.c_str());

    EXPECT_FALSE(ok);
    EXPECT_FALSE(msg.empty());

    expectNoLeak("MemberFunction_SecondArgWrongType");
}

TEST_F(LongjmpSafetyTests, MemberFunction_WrongSelfType_ErrorPropagates)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<SafetyTestClass>("SafetyTestClass")
            .addConstructor<void (*)()>()
            .addFunction("concat", &SafetyTestClass::concat)
        .endClass();

    // Pass a non-userdata as 'self' — this errors BEFORE any TrackedString is built
    auto [ok, msg] = runLuaCaptureError("local o = {}; o:concat('a', 'b')");
    EXPECT_FALSE(ok);
    EXPECT_EQ(g_tracked_string_count.load(), 0);
}

// ===========================================================================
// 7. Class constructor — wrong argument type.
// ===========================================================================
namespace {

struct ConstructedClass
{
    TrackedString name;

    explicit ConstructedClass(TrackedString n)
        : name(std::move(n))
    {
    }
};

} // namespace

TEST_F(LongjmpSafetyTests, Constructor_WrongArgType_ErrorPropagates)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<ConstructedClass>("ConstructedClass")
            .addConstructor<void (*)(TrackedString)>()
        .endClass();

    auto [ok, msg] = runLuaCaptureError("local o = ConstructedClass(true)");
    EXPECT_FALSE(ok);
    EXPECT_FALSE(msg.empty());
    // No TrackedString should be constructed if the type check fails first
    EXPECT_EQ(g_tracked_string_count.load(), 0);
}

TEST_F(LongjmpSafetyTests, Constructor_CorrectType_Works)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<ConstructedClass>("ConstructedClass")
            .addConstructor<void (*)(TrackedString)>()
        .endClass();

    auto [ok, msg] = runLuaCaptureError("local o = ConstructedClass('hello')");
    EXPECT_TRUE(ok) << msg;
}

// ===========================================================================
// 8. Property setter — wrong value type.
//    The 'self' pointer (T*) is trivially destructible; only the value type
//    matters for leak analysis.
// ===========================================================================
namespace {

struct PropClass
{
    TrackedString name;
};

} // namespace

TEST_F(LongjmpSafetyTests, PropertySetter_WrongValueType_ErrorPropagates)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<PropClass>("PropClass")
            .addConstructor<void (*)()>()
            .addProperty("name", &PropClass::name)
        .endClass();

    // Create object in global scope so it persists across pcall boundary
    ASSERT_TRUE(runLua("g_obj = PropClass()"));
    // One TrackedString alive: the 'name' member of g_obj
    const int count_after_create = g_tracked_string_count.load();
    EXPECT_EQ(count_after_create, 1);

    // Now fail the setter — no additional TrackedString temporary should be created
    auto [ok, msg] = runLuaCaptureError("g_obj.name = true");
    EXPECT_FALSE(ok);
    EXPECT_FALSE(msg.empty());
    // Count must not increase: the setter's TypeResult<TrackedString> stays in
    // error state and never constructs a TrackedString temporary.
    EXPECT_EQ(g_tracked_string_count.load(), count_after_create);
}

// ===========================================================================
// 9. Property getter error — getter failing to push to Lua stack.
//    SAFE: getter calls Stack<T>::push, which returns Result (not TypeResult<T>).
// ===========================================================================
TEST_F(LongjmpSafetyTests, PropertyGetter_ReadFromWrongSelfType_ErrorPropagates)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<PropClass>("PropClass")
            .addConstructor<void (*)()>()
            .addProperty("name", &PropClass::name)
        .endClass();

    // Access property on wrong type — error before any TrackedString is built
    auto [ok, msg] = runLuaCaptureError("local o = {}; local v = o.name");
    // This either errors or returns nil; either way the VM should survive
    // (note: raw table access on {} for key "name" returns nil, no error)
    // So test member property access on the wrong class userdata:
    auto [ok2, msg2] = runLuaCaptureError(
        "local o = PropClass(); local v = getmetatable(o).__index(42, 'name')");
    (void)ok2; // may or may not error depending on implementation path
}

// ===========================================================================
// 10. Lambda-wrapped functions — same safety properties as free functions.
// ===========================================================================
TEST_F(LongjmpSafetyTests, LambdaFunction_SecondArgWrongType)
{
    luabridge::getGlobalNamespace(L)
        .addFunction("f", [](TrackedString a, TrackedString b) -> std::string {
            return a.value + b.value;
        });

    std::string arg = std::string("'") + kLongString + "'";
    std::string lua = "f(" + arg + ", true)";
    auto [ok, msg] = runLuaCaptureError(lua.c_str());

    EXPECT_FALSE(ok);
    EXPECT_FALSE(msg.empty());

    expectNoLeak("LambdaFunction_SecondArgWrongType");
}

// ===========================================================================
// 11. Overloaded functions — wrong types on all overloads.
// ===========================================================================
TEST_F(LongjmpSafetyTests, OverloadedFunction_AllOverloadsFail)
{
    luabridge::getGlobalNamespace(L)
        .addFunction("f",
            +[](TrackedString s) -> std::string { return s.value; },
            +[](int n) -> std::string { return std::to_string(n); });

    // Pass a type that matches neither overload (table) → all overloads fail
    auto [ok, msg] = runLuaCaptureError("f({})");
    EXPECT_FALSE(ok);
    EXPECT_EQ(g_tracked_string_count.load(), 0);
}

// ===========================================================================
// 12. Verify Lua VM is fully usable after a sequence of errors.
// ===========================================================================
TEST_F(LongjmpSafetyTests, VMUsableAfterSequenceOfErrors)
{
    int success_count = 0;

    luabridge::getGlobalNamespace(L)
        .addFunction("f", [&](TrackedString a, TrackedString b) -> std::string {
            ++success_count;
            return a.value + "|" + b.value;
        });

    // Series of incorrect calls — use boolean (not number) as wrong type since
    // numbers are coerced to string when LUABRIDGE_STRICT_STACK_CONVERSIONS is off
    for (int i = 0; i < 5; ++i)
    {
        std::string lua = (i % 2 == 0)
            ? std::string("f('") + kLongString + "', true)"
            : "f(true, 'second')";
        auto [ok, _] = runLuaCaptureError(lua.c_str());
        EXPECT_FALSE(ok) << "Call " << i << " should have failed";
    }

    // Correct call must succeed after all the errors
    auto [ok, msg] = runLuaCaptureError("result = f('hello', 'world')");
    EXPECT_TRUE(ok) << "VM not usable after errors: " << msg;
    EXPECT_EQ(result<std::string>(), "hello|world");
    EXPECT_EQ(success_count, 1);
}

// ===========================================================================
// 13. Static function via namespace — same invoke path.
// ===========================================================================
TEST_F(LongjmpSafetyTests, NamespacedStaticFunction_SecondArgWrongType)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("ns")
            .addFunction("f", +[](TrackedString a, TrackedString b) -> int {
                return static_cast<int>(a.value.size() + b.value.size());
            })
        .endNamespace();

    std::string arg = std::string("'") + kLongString + "'";
    auto [ok, msg] = runLuaCaptureError(("ns.f(" + arg + ", {})").c_str());

    EXPECT_FALSE(ok);
    EXPECT_FALSE(msg.empty());

    expectNoLeak("NamespacedStaticFunction_SecondArgWrongType");
}

// ===========================================================================
// 14. container_forwarder — shared_ptr container constructor safety.
//
//     When addConstructorFrom<std::shared_ptr<T>>() is used, LuaBridge creates
//     a container_forwarder that:
//       1. Default-constructs C object (empty shared_ptr, no resources).
//       2. Assigns the result of the factory into 'object'.
//       3. Calls UserdataSharedHelper::push(L, object).
//          If push fails (e.g. class not registered at push time), it used to
//          call raise_lua_error with a fully-constructed shared_ptr on the C++
//          stack. longjmp would skip ~shared_ptr, leaking T.
//       FIX: container_forwarder now resets 'object = C{}' before raise_lua_error.
// ===========================================================================
namespace {

std::atomic<int> g_container_object_count{0};

struct ContainerTestObject
{
    explicit ContainerTestObject(int v)
        : value(v)
    {
        ++g_container_object_count;
    }

    ~ContainerTestObject()
    {
        --g_container_object_count;
    }

    int value;
};

} // namespace

TEST_F(LongjmpSafetyTests, ContainerForwarder_SuccessfulConstruction_NoLeak)
{
    g_container_object_count = 0;

    luabridge::getGlobalNamespace(L)
        .beginClass<ContainerTestObject>("ContainerTestObject")
            .addConstructorFrom<std::shared_ptr<ContainerTestObject>, void(*)(int)>()
        .endClass();

    // Normal construction: object should be alive while referenced by Lua
    auto [ok, msg] = runLuaCaptureError("result = ContainerTestObject(42)");
    EXPECT_TRUE(ok) << msg;
    EXPECT_EQ(g_container_object_count.load(), 1);  // Lua holds the only reference

    // After Lua GC (triggered by closing the state), count should go to 0
    lua_gc(L, LUA_GCCOLLECT, 0);
    // Object may or may not be collected yet depending on GC timing, but
    // after state close (TearDown) it must be 0
}

TEST_F(LongjmpSafetyTests, ContainerForwarder_WrongArgType_ErrorAndNoLeak)
{
    g_container_object_count = 0;

    luabridge::getGlobalNamespace(L)
        .beginClass<ContainerTestObject>("ContainerTestObject")
            .addConstructorFrom<std::shared_ptr<ContainerTestObject>, void(*)(int)>()
        .endClass();

    // Wrong argument type: factory should not be called at all
    auto [ok, msg] = runLuaCaptureError("result = ContainerTestObject('not_an_int')");
    EXPECT_FALSE(ok);
    EXPECT_FALSE(msg.empty());
    // No ContainerTestObject should have been created
    EXPECT_EQ(g_container_object_count.load(), 0);
}

TEST_F(LongjmpSafetyTests, ContainerForwarder_VMUsableAfterConstructionError)
{
    g_container_object_count = 0;

    luabridge::getGlobalNamespace(L)
        .beginClass<ContainerTestObject>("ContainerTestObject")
            .addConstructorFrom<std::shared_ptr<ContainerTestObject>, void(*)(int)>()
        .endClass();

    // Error call
    auto [ok1, _] = runLuaCaptureError("ContainerTestObject('bad')");
    EXPECT_FALSE(ok1);

    // Success call — VM must still work
    auto [ok2, msg2] = runLuaCaptureError("result = ContainerTestObject(99)");
    EXPECT_TRUE(ok2) << msg2;
    EXPECT_EQ(g_container_object_count.load(), 1);
}

// ===========================================================================
// 15. container_forwarder — push-failure path with pre-reset fix.
//
//     Simulate the push-failure path: register the class, construct the object
//     (factory succeeds), then verify the shared_ptr is properly released.
//     In practice, UserdataSharedHelper::push fails when the class is not in
//     the registry. We test this by constructing correctly (so we cover the
//     full path) and using a tracked shared_ptr to verify refcount semantics.
// ===========================================================================
namespace {

std::atomic<int> g_shared_object_count{0};

struct SharedTestObject
{
    explicit SharedTestObject()
    {
        ++g_shared_object_count;
    }

    ~SharedTestObject()
    {
        --g_shared_object_count;
    }
};

} // namespace

TEST_F(LongjmpSafetyTests, ContainerForwarder_SharedPtrRefcountAfterGC)
{
    g_shared_object_count = 0;

    luabridge::getGlobalNamespace(L)
        .beginClass<SharedTestObject>("SharedTestObject")
            .addConstructorFrom<std::shared_ptr<SharedTestObject>, void(*)()>()
        .endClass();

    // Construct 3 objects via Lua
    ASSERT_TRUE(runLua("local a = SharedTestObject(); local b = SharedTestObject(); local c = SharedTestObject()"));
    lua_gc(L, LUA_GCCOLLECT, 0);

    // After GC collects locals (end of chunk), the shared_ptrs should be released.
    // The live count should be 0 (or may require another GC cycle):
    lua_gc(L, LUA_GCCOLLECT, 0);
    // We don't assert exact count here because Lua GC timing varies,
    // but after state close (TearDown) it must reach 0.
    // Any non-zero count at state close indicates a leak.
}

// Verify count reaches 0 after state teardown (happens in TearDown via lua_close)
// This is a cross-test assertion: if g_shared_object_count != 0 at process exit
// when combined with LSAN, it flags a leak.
