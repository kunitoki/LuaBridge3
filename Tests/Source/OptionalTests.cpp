// https://github.com/kunitoki/LuaBridge3
// Copyright 2021, Stefan Frings
// Copyright 2020, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#include "TestBase.h"
#include "TestTypes.h"

#include <optional>

namespace {
template <typename T>
std::string toLuaSrcString(const T& value)
{
    return std::to_string(value);
}

template<>
std::string toLuaSrcString(const bool& value)
{
    return value ? "true" : "false";
}

template<>
std::string toLuaSrcString(const char& value)
{
    return "'" + std::string(&value, 1) + "'";
}

template<>
std::string toLuaSrcString(const char* const& value)
{
    return "'" + std::string(value) + "'";
}

template<>
std::string toLuaSrcString(const std::string& value)
{
    return "'" + value + "'";
}

template<>
std::string toLuaSrcString(const std::string_view& value)
{
    return toLuaSrcString(std::string(value));
}

template<typename T>
std::optional<T> optCast(luabridge::LuaRef const& ref)
{
    // NOTE cast to std::optional: https://stackoverflow.com/a/45865802
    return ref.unsafe_cast<std::optional<T>>();
}

template <class T>
void checkEquals(T expected, T actual)
{
    using U = std::decay_t<T>;

    if constexpr (std::is_same_v<U, float>)
    {
        ASSERT_FLOAT_EQ(expected, actual);
    }
    else if constexpr (std::is_same_v<U, double> || std::is_same_v<U, long double>)
    {
        ASSERT_DOUBLE_EQ(expected, actual);
    }
    else if constexpr (std::is_same_v<U, const char*>)
    {
        ASSERT_STREQ(expected, actual);
    }
    else
    {
        ASSERT_EQ(expected, actual);
    }
}
} // namespace

template<class T>
struct OptionalTest : TestBase
{
};

TYPED_TEST_SUITE_P(OptionalTest);

TYPED_TEST_P(OptionalTest, LuaRefPresent)
{
    using Traits = TypeTraits<TypeParam>;

    for (TypeParam const& value : Traits::values())
    {
        std::string const luaSrc = "result = " + toLuaSrcString(value);

        SCOPED_TRACE(luaSrc);
        this->runLua(luaSrc);

        std::optional<TypeParam> const actual = optCast<TypeParam>(this->result());
        ASSERT_TRUE(actual.has_value());

        checkEquals(value, actual.value());
    }
}

TYPED_TEST_P(OptionalTest, LuaRefNotPresent)
{
    this->runLua("result = nil");

    std::optional<TypeParam> const actual = optCast<TypeParam>(this->result());
    ASSERT_FALSE(actual.has_value());
}

TYPED_TEST_P(OptionalTest, LuaRefIsInstancePresent)
{
    using Traits = TypeTraits<TypeParam>;

    for (TypeParam const& value : Traits::values())
    {
        std::string const luaSrc = "result = " + toLuaSrcString(value);

        SCOPED_TRACE(luaSrc);
        this->runLua(luaSrc);

        luabridge::LuaRef const actualRef = this->result();
        ASSERT_TRUE(actualRef.isInstance<std::optional<TypeParam>>());

        // if isInstance returns true a cast without issues is possible
        [[maybe_unused]] std::optional<TypeParam> const actual = optCast<TypeParam>(actualRef);
    }
}

TYPED_TEST_P(OptionalTest, LuaRefIsInstancePresentWrongType)
{
    this->runLua("function func() end; result = func");

    luabridge::LuaRef const actualRef = this->result();
    ASSERT_FALSE(actualRef.isInstance<std::optional<TypeParam>>());
}

TYPED_TEST_P(OptionalTest, LuaRefIsInstanceNotPresent)
{
    this->runLua("result = nil");

    luabridge::LuaRef const actualRef = this->result();
    ASSERT_TRUE(actualRef.isInstance<std::optional<TypeParam>>());

    // if isInstance returns true a cast without issues is possible
    [[maybe_unused]] std::optional<TypeParam> const actual = optCast<TypeParam>(actualRef);
}

REGISTER_TYPED_TEST_SUITE_P(OptionalTest,
                           LuaRefPresent,
                           LuaRefNotPresent,
                           LuaRefIsInstancePresent,
                           LuaRefIsInstancePresentWrongType,
                           LuaRefIsInstanceNotPresent);

INSTANTIATE_TYPED_TEST_SUITE_P(OptionalTest, OptionalTest, TestTypes);

namespace {
struct Data
{
    explicit Data(int i) : i(i) {}

    int i;
};

bool operator==(Data const& lhs, Data const& rhs)
{
    return lhs.i == rhs.i;
}

template<typename T>
bool operator==(std::optional<T> const& lhs, std::optional<T> const& rhs)
{
    if (lhs.has_value() != rhs.has_value())
    {
        return false;
    }

    if (lhs.has_value())
    {
        assert(rhs.has_value());
        return lhs.value() == rhs.value();
    }

    assert(!lhs.has_value());
    assert(!rhs.has_value());
    return true;
}

std::optional<Data> processValue(std::optional<Data> const& data)
{
    return data;
}

std::optional<Data> processPointer(std::optional<const Data*> const& data)
{
    std::optional<Data> result;
    if (data)
    {
        result = **data;
    }
    return result;
}
} // namespace

struct OptionalTests : TestBase
{
};

template<typename T>
void testPassFromLua(OptionalTests const& fixture,
                     std::string const& functionName,
                     std::string const& valueString,
                     std::optional<T> const expected)
{
    fixture.resetResult();

    std::string const luaSrc = "result = " + functionName + "(" + valueString + ")";

    SCOPED_TRACE(luaSrc);
    fixture.runLua(luaSrc);

    std::optional<T> const actual = optCast<T>(fixture.result());
    ASSERT_EQ(expected, actual);
}

TEST_F(OptionalTests, PassFromLua)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Data>("Data")
        .addConstructor<void (*)(int)>()
        .endClass()
        .addFunction("processValue", &processValue)
        .addFunction("processPointer", &processPointer);

    testPassFromLua<Data>(*this, "processValue", "Data(-1)", Data(-1));
    testPassFromLua<Data>(*this, "processValue", "Data(2)", Data(2));
    testPassFromLua<Data>(*this, "processValue", "nil", std::nullopt);

    testPassFromLua<Data>(*this, "processPointer", "Data(-1)", Data(-1));
    testPassFromLua<Data>(*this, "processPointer", "Data(2)", Data(2));
    testPassFromLua<Data>(*this, "processPointer", "nil", std::nullopt);
}

TEST_F(OptionalTests, PassToFunction)
{
    runLua("function foo (opt) "
           "  result = opt "
           "end");

    auto foo = luabridge::getGlobal(L, "foo");

    {
        resetResult();

        std::optional<int> lvalue{ 10 };
        foo(lvalue);
        EXPECT_FALSE(result().isNil());
        EXPECT_EQ(lvalue, result<std::optional<int>>());

        resetResult();

        const std::optional<int> constLvalue = lvalue;
        foo(constLvalue);
        EXPECT_FALSE(result().isNil());
        EXPECT_EQ(lvalue, result<std::optional<int>>());
    }

    {
        resetResult();

        std::optional<std::string> lvalue;
        foo(lvalue);
        EXPECT_TRUE(result().isNil());
        EXPECT_FALSE(result<std::optional<std::string>>());

        resetResult();

        const std::optional<std::string> constLvalue = lvalue;
        foo(constLvalue);
        EXPECT_TRUE(result().isNil());
        EXPECT_FALSE(result<std::optional<std::string>>());
    }
}

TEST_F(OptionalTests, FromCppApi)
{
    struct OptionalClass
    {
        OptionalClass() = default;

        void testOptionalAsArg(std::optional<int> v)
        {
            x = v;
        }

        std::optional<std::string> testOptionalAsReturn()
        {
            return "abcdef";
        }

        std::optional<int> x;
    };

    luabridge::getGlobalNamespace(L)
        .beginClass<OptionalClass>("OptionalClass")
        .addConstructor<void (*)()>()
        .addFunction("testOptionalAsArg", &OptionalClass::testOptionalAsArg)
        .addFunction("testOptionalAsReturn", &OptionalClass::testOptionalAsReturn)
        .endClass();

    resetResult();
    runLua("result = OptionalClass () result:testOptionalAsArg (1337)");

    EXPECT_TRUE(result().isUserdata());
    EXPECT_EQ(1337, *result<OptionalClass>().x);

    resetResult();
    runLua("x = OptionalClass () result = x:testOptionalAsReturn ()");

    EXPECT_FALSE(result().isNil());
    EXPECT_EQ("abcdef", *result<std::optional<std::string>>());
}

TEST_F(OptionalTests, MissingOptionalArgument)
{
    luabridge::getGlobalNamespace(L)
        .addFunction("testOptionalAsArg", [](const std::optional<int>& v) { return v.has_value(); });

    resetResult();
    runLua("result = testOptionalAsArg()");

    EXPECT_TRUE(result().isBool());
    EXPECT_FALSE(result<bool>());
}

#if !LUABRIDGE_HAS_EXCEPTIONS
TEST_F(OptionalTests, PushUnregisteredWithNoExceptionsShouldFailButRestoreStack)
{
    class Unregistered {};

    const int initialStackSize = lua_gettop(L);

    lua_pushnumber(L, 1);
    EXPECT_EQ(1, lua_gettop(L) - initialStackSize);

    std::optional<Unregistered> v = Unregistered{};

    auto result = luabridge::Stack<decltype(v)>::push(L, v);
    EXPECT_FALSE(result);

    EXPECT_EQ(1, lua_gettop(L) - initialStackSize);

    lua_pop(L, 1);
    EXPECT_EQ(0, lua_gettop(L) - initialStackSize);
}
#endif
