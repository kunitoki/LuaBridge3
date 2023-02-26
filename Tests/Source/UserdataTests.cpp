// https://github.com/kunitoki/LuaBridge3
// Copyright 2022, Lucio Asnaghi
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
    runLua("object = TestClass(123); result = testFunctionObject(object)");

    ASSERT_EQ(result(), 123);
}

TEST_F(UserDataTest, ObjectConst)
{
    runLua("object = TestClass(123); result = testFunctionObjectConst(object)");

    ASSERT_EQ(result(), 123);
}

TEST_F(UserDataTest, Ref)
{
    runLua("object = TestClass(123); result = testFunctionRef(object)");

    ASSERT_EQ(result(), 123);
}

TEST_F(UserDataTest, RefConst)
{
    runLua("object = TestClass(123); result = testFunctionRefConst(object)");

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
