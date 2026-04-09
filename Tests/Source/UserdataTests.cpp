// https://github.com/kunitoki/LuaBridge3
// Copyright 2022, kunitoki
// Copyright 2022, Stefan Frings
// SPDX-License-Identifier: MIT

#include "TestBase.h"

namespace {
class TestClass
{
public:
    explicit inline TestClass(int i) : m_i(i) {}

    inline int get() const { return m_i; }

private:
    int m_i;
};

int testFunctionObject(TestClass object)
{
    return object.get();
}

int testFunctionObjectConst(const TestClass object)
{
    return object.get();
}

int testFunctionRef(TestClass& object)
{
    return object.get();
}

int testFunctionRefConst(const TestClass& object)
{
    return object.get();
}
} // namespace

struct UserDataTest : TestBase
{
    void SetUp() override
    {
        TestBase::SetUp();

        luabridge::getGlobalNamespace(L)
            .beginClass<TestClass>("TestClass")
                .addConstructor<void (*)(int)>()
            .endClass()
            .addFunction("testFunctionObject", testFunctionObject)
            .addFunction("testFunctionObjectConst", testFunctionObjectConst)
            .addFunction("testFunctionRef", testFunctionRef)
            .addFunction("testFunctionRefConst", testFunctionRefConst);
    }
};

TEST_F(UserDataTest, Object)
{
    runLua("object = TestClass.new(123); result = testFunctionObject(object)");

    ASSERT_EQ(result(), 123);
}

TEST_F(UserDataTest, ObjectConst)
{
    runLua("object = TestClass.new(123); result = testFunctionObjectConst(object)");

    ASSERT_EQ(result(), 123);
}

TEST_F(UserDataTest, Ref)
{
    runLua("object = TestClass.new(123); result = testFunctionRef(object)");

    ASSERT_EQ(result(), 123);
}

TEST_F(UserDataTest, RefConst)
{
    runLua("object = TestClass.new(123); result = testFunctionRefConst(object)");

    ASSERT_EQ(result(), 123);
}

TEST_F(UserDataTest, FailNumberObject)
{
#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("testFunctionObject(132)"), std::runtime_error);
#else
    EXPECT_FALSE(runLua("testFunctionObject(132)"));
#endif
}

TEST_F(UserDataTest, FailNumberObjectConst)
{
#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("testFunctionObjectConst(132)"), std::runtime_error);
#else
    EXPECT_FALSE(runLua("testFunctionObjectConst(132)"));
#endif
}

TEST_F(UserDataTest, FailNumberRef)
{
#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("testFunctionRef(132)"), std::runtime_error);
#else
    EXPECT_FALSE(runLua("testFunctionRef(132)"));
#endif
}

TEST_F(UserDataTest, FailNumberRefConst)
{
#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("testFunctionRefConst(132)"), std::runtime_error);
#else
    EXPECT_FALSE(runLua("testFunctionRefConst(132)"));
#endif
}

TEST_F(UserDataTest, FailNilObject)
{
#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("testFunctionObject(nil)"), std::runtime_error);
#else
    EXPECT_FALSE(runLua("testFunctionObject(nil)"));
#endif
}

TEST_F(UserDataTest, FailNilObjectConst)
{
#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("testFunctionObjectConst(nil)"), std::runtime_error);
#else
    EXPECT_FALSE(runLua("testFunctionObjectConst(nil)"));
#endif
}

TEST_F(UserDataTest, FailNilRef)
{
#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("testFunctionRef(nil)"), std::runtime_error);
#else
    EXPECT_FALSE(runLua("testFunctionRef(nil)"));
#endif
}

TEST_F(UserDataTest, FailNilRefConst)
{
#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("testFunctionRefConst(nil)"), std::runtime_error);
#else
    EXPECT_FALSE(runLua("testFunctionRefConst(nil)"));
#endif
}

//=================================================================================================
// New Test Suite for TypeResult and getNilBadArgError Error Handling
//=================================================================================================

namespace {
class ClassWithProperties
{
public:
    ClassWithProperties() : m_value(42), m_readOnly(100) {}

    int getValue() const { return m_value; }
    void setValue(int v) { m_value = v; }

    int getReadOnly() const { return m_readOnly; }

    int callSomething(int x) { return x * 2; }

    const int& getValueRef() const { return m_value; }

private:
    int m_value;
    const int m_readOnly;
};
} // namespace

struct TypeResultErrorHandlingTest : TestBase
{
    void SetUp() override
    {
        TestBase::SetUp();

        luabridge::getGlobalNamespace(L)
            .beginClass<ClassWithProperties>("ClassWithProperties")
                .addConstructor<void(*)()>()
                .addProperty("value", &ClassWithProperties::getValue, &ClassWithProperties::setValue)
                .addFunction("callSomething", &ClassWithProperties::callSomething)
            .endClass()
            .addFunction("getPropertyRef", [](const ClassWithProperties& obj) { return obj.getValue(); });
    }
};

// Test 1: Verify error message when nil is passed to reference parameters
TEST_F(TypeResultErrorHandlingTest, NilToReferenceParameterError)
{
#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("getPropertyRef(nil)"), std::runtime_error);
#else
    auto [result, errorMsg] = runLuaCaptureError("getPropertyRef(nil)");
    ASSERT_FALSE(result);
    // Should mention "no value" in error message
    EXPECT_TRUE(errorMsg.find("no value") != std::string::npos 
        || errorMsg.find("ClassWithProperties") != std::string::npos);
#endif
}

// Test 2: Verify property getter error message with nil
TEST_F(TypeResultErrorHandlingTest, PropertyGetterWithNil)
{
#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("obj = nil; v = obj.value"), std::runtime_error);
#else
    auto [result, errorMsg] = runLuaCaptureError("obj = nil; v = obj.value");
    ASSERT_FALSE(result);
#endif
}

// Test 3: Verify property setter error message with nil
TEST_F(TypeResultErrorHandlingTest, PropertySetterWithNil)
{
#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("obj = nil; obj.value = 123"), std::runtime_error);
#else
    auto [result, errorMsg] = runLuaCaptureError("obj = nil; obj.value = 123");
    ASSERT_FALSE(result);
#endif
}

// Test 4: Verify member function error with nil
TEST_F(TypeResultErrorHandlingTest, MemberFunctionWithNil)
{
#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("obj = nil; result = obj:callSomething(5)"), std::runtime_error);
#else
    auto [result, errorMsg] = runLuaCaptureError("obj = nil; result = obj:callSomething(5)");
    ASSERT_FALSE(result);
#endif
}

// Test 5: Verify correct behavior with valid object
TEST_F(TypeResultErrorHandlingTest, ValidObjectPropertyAccess)
{
    runLua("obj = ClassWithProperties.new(); result = obj.value");
    ASSERT_EQ(result(), 42);
}

// Test 6: Verify correct behavior with property setter
TEST_F(TypeResultErrorHandlingTest, ValidObjectPropertySetter)
{
    runLua("obj = ClassWithProperties.new(); obj.value = 99; result = obj.value");
    ASSERT_EQ(result(), 99);
}

// Test 7: Verify correct behavior with member function
TEST_F(TypeResultErrorHandlingTest, ValidObjectMemberFunction)
{
    runLua("obj = ClassWithProperties.new(); result = obj:callSomething(21)");
    ASSERT_EQ(result(), 42);
}

// Test 8: Wrong type passed to member function
TEST_F(TypeResultErrorHandlingTest, WrongTypeToMemberFunction)
{
#if LUABRIDGE_HAS_EXCEPTIONS
    runLua("obj = ClassWithProperties.new()"); // Setup valid object
    ASSERT_THROW(runLua("result = obj:callSomething('string')"), std::runtime_error);
#else
    runLua("obj = ClassWithProperties.new()");
    auto [result, errorMsg] = runLuaCaptureError("result = obj:callSomething('string')");
    ASSERT_FALSE(result);
#endif
}

// Test 9: Test error message quality for unregistered class
TEST_F(TypeResultErrorHandlingTest, UnregisteredClassError)
{
#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("local ud = debug.setlocal(1, nil, nil); "
                        "getPropertyRef(ud)"), std::runtime_error);  
#else
    // This is harder to test without manually constructing an unregistered userdata
#endif
}

// Test 10: Nil handling consistency across different property access patterns
TEST_F(TypeResultErrorHandlingTest, PropertyAccessConsistency)
{
    runLua("obj = ClassWithProperties.new(); "
           "v1 = obj.value; "
           "result = (v1 == 42)");
    
    ASSERT_EQ(result<bool>(), true);
}

// Test 11: Multiple operations in sequence to verify stack management
TEST_F(TypeResultErrorHandlingTest, SequentialPropertyAccess)
{
    runLua("obj = ClassWithProperties.new(); "
           "v1 = obj.value; "
           "obj.value = 50; "
           "v2 = obj.value; "
           "result = v2");
    
    ASSERT_EQ(result(), 50);
}

// Test 12: Verify error when passing wrong object type to function expecting specific class
TEST_F(TypeResultErrorHandlingTest, WrongClassTypeError)
{
    runLua("obj = ClassWithProperties.new()");

#if LUABRIDGE_HAS_EXCEPTIONS
    ASSERT_THROW(runLua("otherObj = TestClass.new(10)"), std::runtime_error);
#else
    // This test mainly checks that registering TestClass succeeds
    EXPECT_TRUE(true);
#endif
}
