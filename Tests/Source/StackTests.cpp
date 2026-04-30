// https://github.com/kunitoki/LuaBridge3
// Copyright 2021, kunitoki
// Copyright 2020, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include "LuaBridge/Array.h"
#include "LuaBridge/List.h"
#include "LuaBridge/Map.h"
#include "LuaBridge/Set.h"
#include "LuaBridge/UnorderedMap.h"
#include "LuaBridge/Vector.h"

namespace {
struct Unregistered {};
} // namespace

struct StackTests : TestBase
{
};

TEST_F(StackTests, Void)
{
    ASSERT_TRUE(luabridge::Stack<void>::push(L));
}

TEST_F(StackTests, VoidStackOverflow)
{
    exhaustStackSpace();

    ASSERT_TRUE(luabridge::Stack<void>::push(L));
}

TEST_F(StackTests, NullptrType)
{
    {
        ASSERT_TRUE(luabridge::push(L, nullptr));
    }

    {
        EXPECT_TRUE(luabridge::isInstance<std::nullptr_t>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<int64_t>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<uint64_t>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<float>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<double>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<long double>(L, -1));
        EXPECT_TRUE(luabridge::isInstance<const char*>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<std::string_view>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<std::tuple<std::nullptr_t>>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<std::vector<std::nullptr_t>>(L, -1));
        EXPECT_TRUE(luabridge::isInstance<std::optional<std::nullptr_t>>(L, -1));
    }
    
    {
        auto result = *luabridge::get<std::nullptr_t>(L, -1);
        EXPECT_EQ(nullptr, result);
    }
}

TEST_F(StackTests, NullptrStackOverflow)
{
    exhaustStackSpace();

    auto result = luabridge::Stack<std::nullptr_t>::push(L, nullptr);
    ASSERT_FALSE(result);
    EXPECT_FALSE(result.message().empty());
}

TEST_F(StackTests, NullptrInvalidType)
{
    (void)luabridge::Stack<int>::push(L, 42);

    auto result = luabridge::Stack<std::nullptr_t>::get(L, -1);
    ASSERT_FALSE(result);
}

TEST_F(StackTests, LuaStateType)
{
    {
        auto result = luabridge::get<lua_State*>(L, -1);
        EXPECT_TRUE(result);
        EXPECT_EQ(L, *result);
    }
}

TEST_F(StackTests, LuaCFunctionType)
{
    lua_CFunction value = +[](lua_State*) { return 0; };

    {
        ASSERT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<float>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<lua_CFunction>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<lua_CFunction>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<lua_CFunction>>(L, -1));

    {
        auto result = *luabridge::get<lua_CFunction>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, LuaCFunctionStackOverflow)
{
    exhaustStackSpace();
    
    lua_CFunction value = +[](lua_State*) { return 0; };
    
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, LuaCFunctionInvalidType)
{
    (void)luabridge::Stack<int>::push(L, 42);

    auto result = luabridge::Stack<lua_CFunction>::get(L, -1);
    ASSERT_FALSE(result);
}

TEST_F(StackTests, BoolType)
{
    bool value = true;

    {
        ASSERT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<float>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<bool>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<bool>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<bool>>(L, -1));

    {
        auto result = *luabridge::get<bool>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, BoolStackOverflow)
{
    exhaustStackSpace();
    
    bool value = true;
    
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, BoolInvalidType)
{
#if !LUABRIDGE_STRICT_STACK_CONVERSIONS
    // Non-strict mode: any Lua value is accepted via lua_toboolean
    {
        (void)luabridge::Stack<int>::push(L, 0);
        auto result = luabridge::Stack<bool>::get(L, -1);
        ASSERT_TRUE(result);
        EXPECT_TRUE(*result); // integer 0 is truthy in Lua
    }

    {
        (void)luabridge::Stack<int>::push(L, 42);
        auto result = luabridge::Stack<bool>::get(L, -1);
        ASSERT_TRUE(result);
        EXPECT_TRUE(*result);
    }

    {
        (void)luabridge::Stack<std::nullptr_t>::push(L, nullptr);
        auto result = luabridge::Stack<bool>::get(L, -1);
        ASSERT_TRUE(result);
        EXPECT_FALSE(*result); // nil is falsy
    }
#else
    // Strict mode: only LUA_TBOOLEAN is accepted
    {
        (void)luabridge::Stack<int>::push(L, 0);
        auto result = luabridge::Stack<bool>::get(L, -1);
        ASSERT_FALSE(result);
    }

    {
        (void)luabridge::Stack<int>::push(L, 42);
        auto result = luabridge::Stack<bool>::get(L, -1);
        ASSERT_FALSE(result);
    }

    {
        (void)luabridge::Stack<std::nullptr_t>::push(L, nullptr);
        auto result = luabridge::Stack<bool>::get(L, -1);
        ASSERT_FALSE(result);
    }
#endif
}

TEST_F(StackTests, CharType)
{
    char value = 'a';

    {
        ASSERT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<char>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<float>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long double>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<char>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<char>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<char>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<const char*>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<std::string_view>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<std::string>>(L, -1));

    {
        auto result = *luabridge::get<char>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, CharStackOverflow)
{
    exhaustStackSpace();
    
    char value = 'a';
    
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, CharInvalidType)
{
    {
        (void)luabridge::Stack<int>::push(L, 1024);
        auto result = luabridge::Stack<char>::get(L, -1);
        ASSERT_FALSE(result);
    }

    {
        (void)luabridge::Stack<std::string_view>::push(L, "");
        auto result = luabridge::Stack<char>::get(L, -1);
        ASSERT_FALSE(result);
    }

    {
        (void)luabridge::Stack<std::string_view>::push(L, "123456");
        auto result = luabridge::Stack<char>::get(L, -1);
        ASSERT_FALSE(result);
    }
}

TEST_F(StackTests, StdByteType)
{
    std::byte value{ 128 };

    {
        EXPECT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<int16_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint16_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint32_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<float>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<double>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<long double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<uint8_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<uint8_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int8_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint8_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<std::byte>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<char>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<int16_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint16_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint32_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<int64_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint64_t>>(L, -1));

    {
        auto result = *luabridge::get<std::byte>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, StdByteStackOVerflow)
{
    exhaustStackSpace();
    
    std::byte value { 128 };
    
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, StdByteInvalidType)
{
    {
        (void)luabridge::Stack<std::string_view>::push(L, "1024");
        auto result = luabridge::Stack<std::byte>::get(L, -1);
        ASSERT_FALSE(result);
    }

    {
        (void)luabridge::Stack<int>::push(L, 1024);
        auto result = luabridge::Stack<std::byte>::get(L, -1);
        ASSERT_FALSE(result);
    }
}

TEST_F(StackTests, Int8Type)
{
    int8_t value = 127;

    {
        ASSERT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<int16_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint16_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint32_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<float>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<double>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<long double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<int16_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<int16_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<int8_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint8_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<char>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint8_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<int16_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint16_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint32_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<int64_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint64_t>>(L, -1));

    {
        auto result = *luabridge::get<int8_t>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, Int8StackOverflow)
{
    exhaustStackSpace();

    int8_t value = 127;
    
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, Int8InvalidType)
{
    {
        (void)luabridge::Stack<std::string_view>::push(L, "1024");
        auto result = luabridge::Stack<int8_t>::get(L, -1);
        ASSERT_FALSE(result);
    }

    {
        (void)luabridge::Stack<int>::push(L, 1024);
        auto result = luabridge::Stack<int8_t>::get(L, -1);
        ASSERT_FALSE(result);
    }
}

TEST_F(StackTests, Int16Type)
{
    int16_t value = 32767;

    {
        ASSERT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<int16_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint16_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint32_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<float>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<double>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<long double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<int16_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<int16_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<uint8_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<int16_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<int64_t>>(L, -1));

    {
        auto result = *luabridge::get<int16_t>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, Int16StackOverflow)
{
    exhaustStackSpace();

    int16_t value = 32767;
    
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, Int16InvalidType)
{
    {
        (void)luabridge::Stack<std::string_view>::push(L, "1000000");
        auto result = luabridge::Stack<int16_t>::get(L, -1);
        ASSERT_FALSE(result);
    }

    {
        (void)luabridge::Stack<int>::push(L, 1000000);
        auto result = luabridge::Stack<int16_t>::get(L, -1);
        ASSERT_FALSE(result);
    }
}

TEST_F(StackTests, Int32Type)
{
    int32_t value = 2147483647;

    {
        ASSERT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint32_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<float>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<double>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<long double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<int32_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<int32_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int16_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<int64_t>>(L, -1));

    {
        auto result = *luabridge::get<int32_t>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, Int32StackOverflow)
{
    exhaustStackSpace();

    int32_t value = 2147483647;
    
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, Int32InvalidType)
{
    {
        (void)luabridge::Stack<std::string_view>::push(L, "100000000");
        auto result = luabridge::Stack<int32_t>::get(L, -1);
        ASSERT_FALSE(result);
    }

    {
        (void)luabridge::Stack<int64_t>::push(L, 2147483648ll);
        auto result = luabridge::Stack<int32_t>::get(L, -1);
        ASSERT_FALSE(result);
    }
}

TEST_F(StackTests, Int64Type)
{
    constexpr int64_t max_integral = static_cast<int64_t>(std::numeric_limits<lua_Integer>::max());
    
    int64_t value = max_integral < 4294967296ll ? max_integral : 4294967296ll;

    {
        ASSERT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    if constexpr (sizeof(lua_Integer) == sizeof(int32_t))
    {
        EXPECT_TRUE(luabridge::isInstance<int32_t>(L, -1));
    }
    else
    {
        EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    }
    EXPECT_TRUE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    if constexpr (sizeof(lua_Integer) == sizeof(uint32_t))
    {
        EXPECT_TRUE(luabridge::isInstance<uint32_t>(L, -1));
    }
    else
    {
        EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, -1));
    }
    EXPECT_TRUE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<float>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<double>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<long double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<int64_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<int64_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int16_t>>(L, -1));
    if constexpr (sizeof(lua_Integer) == sizeof(int32_t))
    {
        EXPECT_TRUE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
    }
    else
    {
        EXPECT_FALSE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
    }
    EXPECT_TRUE(luabridge::isInstance<std::optional<int64_t>>(L, -1));

    {
        auto result = *luabridge::get<int64_t>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, Int64StackOverflow)
{
    exhaustStackSpace();
    
    int64_t value = 42;
    
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, Int64InvalidType)
{
    (void)luabridge::Stack<std::string_view>::push(L, "100000000");
    auto result = luabridge::Stack<int64_t>::get(L, -1);
    ASSERT_FALSE(result);
}

#if 0 // defined(__SIZEOF_INT128__)
TEST_F(StackTests, Int128Type)
{
    constexpr __int128_t max_integral = static_cast<__int128_t>(std::numeric_limits<lua_Integer>::max());

    __int128_t value = max_integral < 4294967296ll ? max_integral : 4294967296ll;

    {
        ASSERT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    if constexpr (sizeof(lua_Integer) == sizeof(int32_t))
    {
        EXPECT_TRUE(luabridge::isInstance<int32_t>(L, -1));
    }
    else
    {
        EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    }
    EXPECT_TRUE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<__int128_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    if constexpr (sizeof(lua_Integer) == sizeof(uint32_t))
    {
        EXPECT_TRUE(luabridge::isInstance<uint32_t>(L, -1));
    }
    else
    {
        EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, -1));
    }
    EXPECT_TRUE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<__uint128_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<float>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<double>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<long double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<__int128_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<__int128_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int16_t>>(L, -1));
    if constexpr (sizeof(lua_Integer) == sizeof(int32_t))
    {
        EXPECT_TRUE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
    }
    else
    {
        EXPECT_FALSE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
    }
    EXPECT_TRUE(luabridge::isInstance<std::optional<int64_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<__int128_t>>(L, -1));

    {
        auto result = luabridge::get<__int128_t>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, Int128StackOverflow)
{
    exhaustStackSpace();

    __int128_t value = 42;

    ASSERT_FALSE(luabridge::push(L, value));
}
#endif

TEST_F(StackTests, Uint8Type)
{
    uint8_t value = 128;

    {
        EXPECT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<int16_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint16_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint32_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<float>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<double>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<long double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<uint8_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<uint8_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int8_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint8_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<char>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<int16_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint16_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint32_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<int64_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint64_t>>(L, -1));

    {
        auto result = *luabridge::get<uint8_t>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, Uint8StackOverflow)
{
    exhaustStackSpace();
    
    uint8_t value = 42;
    
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, Uint8InvalidType)
{
    {
        (void)luabridge::Stack<std::string_view>::push(L, "100000000");
        auto result = luabridge::Stack<uint8_t>::get(L, -1);
        ASSERT_FALSE(result);
    }

    {
        (void)luabridge::Stack<int64_t>::push(L, 2147483648ll);
        auto result = luabridge::Stack<uint8_t>::get(L, -1);
        ASSERT_FALSE(result);
    }
}

TEST_F(StackTests, Uint16Type)
{
    uint16_t value = 32768;

    {
        EXPECT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint16_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint32_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<float>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<double>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<long double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<uint16_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<uint16_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<uint8_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int16_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint16_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint32_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<int64_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint64_t>>(L, -1));

    {
        auto result = *luabridge::get<uint16_t>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, Uint16StackOverflow)
{
    exhaustStackSpace();
    
    uint16_t value = 42;
    
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, Uint16InvalidType)
{
    {
        (void)luabridge::Stack<std::string_view>::push(L, "100000000");
        auto result = luabridge::Stack<uint16_t>::get(L, -1);
        ASSERT_FALSE(result);
    }

    {
        (void)luabridge::Stack<int64_t>::push(L, 2147483648ll);
        auto result = luabridge::Stack<uint16_t>::get(L, -1);
        ASSERT_FALSE(result);
    }
}

TEST_F(StackTests, Uint32Type)
{
    constexpr uint32_t max_integral = static_cast<uint32_t>(std::numeric_limits<lua_Integer>::max());
    
    uint32_t value = max_integral < 2147483648u ? max_integral : 2147483648u;

    {
        EXPECT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    if constexpr (sizeof(lua_Integer) == sizeof(int32_t))
    {
        EXPECT_TRUE(luabridge::isInstance<int32_t>(L, -1));
    }
    else
    {
        EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    }
    EXPECT_TRUE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint32_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<float>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<double>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<long double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<uint32_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<uint32_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int8_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<uint8_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int16_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<uint16_t>>(L, -1));
    if constexpr (sizeof(lua_Integer) == sizeof(int32_t))
    {
        EXPECT_TRUE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
    }
    else
    {
        EXPECT_FALSE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
    }
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint32_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<int64_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint64_t>>(L, -1));

    {
        auto result = *luabridge::get<uint32_t>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, Uint32StackOverflow)
{
    exhaustStackSpace();
    
    uint32_t value = 42;
    
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, Uint32InvalidType)
{
    {
        (void)luabridge::Stack<std::string_view>::push(L, "100000000");
        auto result = luabridge::Stack<uint32_t>::get(L, -1);
        ASSERT_FALSE(result);
    }

    {
        (void)luabridge::Stack<uint64_t>::push(L, 9223372036854775808ull);
        auto result = luabridge::Stack<uint32_t>::get(L, -1);
        ASSERT_FALSE(result);
    }
}

TEST_F(StackTests, Uint64Type)
{
    uint64_t value = static_cast<uint64_t>(std::numeric_limits<int32_t>::max());

    {
        EXPECT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    if constexpr (sizeof(lua_Integer) >= sizeof(int32_t))
    {
        EXPECT_TRUE(luabridge::isInstance<int32_t>(L, -1));
    }
    else
    {
        EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    }
    EXPECT_TRUE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    if constexpr (sizeof(lua_Integer) >= sizeof(uint32_t))
    {
        EXPECT_TRUE(luabridge::isInstance<uint32_t>(L, -1));
    }
    else
    {
        EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, -1));
    }
    EXPECT_TRUE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<float>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<double>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<long double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<uint64_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<uint64_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int8_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<uint8_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int16_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<uint16_t>>(L, -1));
    if constexpr (sizeof(lua_Integer) >= sizeof(int32_t))
    {
        EXPECT_TRUE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
        EXPECT_TRUE(luabridge::isInstance<std::optional<uint32_t>>(L, -1));
    }
    else
    {
        EXPECT_FALSE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<std::optional<uint32_t>>(L, -1));
    }
    EXPECT_TRUE(luabridge::isInstance<std::optional<int64_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint64_t>>(L, -1));

    {
        auto result = *luabridge::get<uint64_t>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, Uint64StackOverflow)
{
    exhaustStackSpace();
    
    uint64_t value = 42;
    
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, Uint64InvalidType)
{
    {
        (void)luabridge::Stack<std::string_view>::push(L, "100000000");
        auto result = luabridge::Stack<uint32_t>::get(L, -1);
        ASSERT_FALSE(result);
    }
}

#if 0 // defined(__SIZEOF_INT128__)
TEST_F(StackTests, Uint128Type)
{
    __uint128_t value = static_cast<__uint128_t>(std::numeric_limits<int32_t>::max());

    {
        EXPECT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    if constexpr (sizeof(lua_Integer) >= sizeof(int32_t))
    {
        EXPECT_TRUE(luabridge::isInstance<int32_t>(L, -1));
    }
    else
    {
        EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    }
    EXPECT_TRUE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<__int128_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    if constexpr (sizeof(lua_Integer) >= sizeof(uint32_t))
    {
        EXPECT_TRUE(luabridge::isInstance<uint32_t>(L, -1));
    }
    else
    {
        EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, -1));
    }
    EXPECT_TRUE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<__uint128_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<float>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<double>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<long double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<__uint128_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<__uint128_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int8_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<uint8_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int16_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<uint16_t>>(L, -1));
    if constexpr (sizeof(lua_Integer) >= sizeof(int32_t))
    {
        EXPECT_TRUE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
        EXPECT_TRUE(luabridge::isInstance<std::optional<uint32_t>>(L, -1));
    }
    else
    {
        EXPECT_FALSE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<std::optional<uint32_t>>(L, -1));
    }
    EXPECT_TRUE(luabridge::isInstance<std::optional<int64_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint64_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<__int128_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<__uint128_t>>(L, -1));

    {
        auto result = *luabridge::get<__uint128_t>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, Uint128StackOverflow)
{
    exhaustStackSpace();

    __uint128_t value = 42;

    ASSERT_FALSE(luabridge::push(L, value));
}
#endif

TEST_F(StackTests, IntTypeNotFittingPush)
{
    std::error_code ec;

    if constexpr (sizeof(uint32_t) == sizeof(lua_Integer) && ! std::is_unsigned_v<lua_Integer>)
    {
        uint32_t value = 4294967295u;

        auto result = luabridge::push(L, value);
        EXPECT_FALSE(result);
        EXPECT_STREQ("luabridge", static_cast<std::error_code>(result).category().name());
    }

    if constexpr (sizeof(uint64_t) == sizeof(lua_Integer) && ! std::is_unsigned_v<lua_Integer>)
    {
        uint64_t value = 9223372036854775808ull;

        auto result = luabridge::push(L, value);
        EXPECT_FALSE(result);
        EXPECT_STREQ("luabridge", static_cast<std::error_code>(result).category().name());
    }

#if 0 // defined(__SIZEOF_INT128__)
    if constexpr (sizeof(uint64_t) == sizeof(lua_Integer) && ! std::is_unsigned_v<lua_Integer>)
    {
        __uint128_t value = __uint128_t(9223372036854775808ull) + __uint128_t(9223372036854775808ull);

        auto result = luabridge::push(L, value);
        EXPECT_FALSE(result);
        EXPECT_STREQ("luabridge", static_cast<std::error_code>(result).category().name());
    }
#endif
}

TEST_F(StackTests, IntTypeNotFittingIsInstance)
{
    if constexpr (sizeof(int32_t) == sizeof(lua_Integer))
    {
        const luabridge::StackRestore sr(L);

        EXPECT_TRUE(luabridge::push(L, 2147483647));
        EXPECT_FALSE(luabridge::isInstance<std::byte>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<int8_t>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<int16_t>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, 1));
    }
    
    if constexpr (sizeof(int64_t) == sizeof(lua_Integer))
    {
        const luabridge::StackRestore sr(L);

        EXPECT_TRUE(luabridge::push(L, 9223372036854775807ll));
        EXPECT_FALSE(luabridge::isInstance<std::byte>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<int8_t>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<int16_t>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<int32_t>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, 1));
    }

#if 0 // defined(__SIZEOF_INT128__)
    if constexpr (sizeof(int64_t) == sizeof(lua_Integer))
    {
        const luabridge::StackRestore sr(L);

        EXPECT_TRUE(luabridge::push(L, __int128_t(9223372036854775807ll)));
        EXPECT_FALSE(luabridge::isInstance<std::byte>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<int8_t>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<int16_t>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<int32_t>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, 1));
    }
#endif
}

TEST_F(StackTests, FloatType)
{
    float value = 123.5678f;

    {
        EXPECT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<float>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<double>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<long double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<uint16_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<uint16_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int8_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<uint8_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int16_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<uint16_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<uint32_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int64_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<uint64_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<float>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<double>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<long double>>(L, -1));

    {
        auto result = *luabridge::get<float>(L, -1);
        EXPECT_FLOAT_EQ(value, result);
    }
}

TEST_F(StackTests, FloatStackOverflow)
{
    exhaustStackSpace();
    
    float value = 42.0f;
    
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, FloatInvalidType)
{
    {
        (void)luabridge::Stack<std::string_view>::push(L, "100000000");
        auto result = luabridge::Stack<float>::get(L, -1);
        ASSERT_FALSE(result);
    }
}

TEST_F(StackTests, DoubleType)
{
    double value = 123.5678;

    {
        EXPECT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<float>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<double>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<long double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<uint16_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<uint16_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int8_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<uint8_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int16_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<uint16_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<uint32_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int64_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<uint64_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<float>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<double>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<long double>>(L, -1));

    {
        auto result = *luabridge::get<double>(L, -1);
        EXPECT_DOUBLE_EQ(value, result);
    }
}

TEST_F(StackTests, DoubleStackOverflow)
{
    exhaustStackSpace();
    
    double value = 42.0;
    
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, DoubleInvalidType)
{
    {
        (void)luabridge::Stack<std::string_view>::push(L, "100000000");
        auto result = luabridge::Stack<double>::get(L, -1);
        ASSERT_FALSE(result);
    }
}

TEST_F(StackTests, LongDoubleType)
{
    long double value = 123.5678l;

    {
        EXPECT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<float>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<double>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<long double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<uint16_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<uint16_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int8_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<uint8_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int16_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<uint16_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<uint32_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int64_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<uint64_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<float>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<double>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<long double>>(L, -1));

    {
        auto result = *luabridge::get<long double>(L, -1);
        EXPECT_DOUBLE_EQ(value, result);
    }
}

TEST_F(StackTests, LongDoubleStackOverflow)
{
    exhaustStackSpace();
    
    long double value = 42.0l;
    
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, LongDoubleInvalidType)
{
    {
        (void)luabridge::Stack<std::string_view>::push(L, "100000000");
        auto result = luabridge::Stack<long double>::get(L, -1);
        ASSERT_FALSE(result);
    }
}

TEST_F(StackTests, FloatTypeNotFittingPush)
{
    if constexpr (sizeof(double) > sizeof(lua_Number))
    {
        double value = std::numeric_limits<double>::max();

        auto result = luabridge::push(L, value);
        EXPECT_FALSE(result);
        EXPECT_STREQ("luabridge", static_cast<std::error_code>(result).category().name());
    }

    if constexpr (sizeof(long double) > sizeof(double) && sizeof(long double) > sizeof(lua_Number))
    {
        long double value = std::numeric_limits<long double>::max();

        auto result = luabridge::push(L, value);
        EXPECT_FALSE(result);
        EXPECT_STREQ("luabridge", static_cast<std::error_code>(result).category().name());
    }
}

TEST_F(StackTests, FloatTypeNotFittingIsInstance)
{
    const luabridge::StackRestore sr(L);

    EXPECT_TRUE(luabridge::push(L, std::numeric_limits<lua_Number>::max()));

    EXPECT_FALSE(luabridge::isInstance<float>(L, 1));

    if constexpr (sizeof(double) == sizeof(lua_Number))
    {
        EXPECT_TRUE(luabridge::isInstance<double>(L, 1));
    }

    if constexpr (sizeof(long double) > sizeof(double) && sizeof(long double) == sizeof(lua_Number))
    {
        EXPECT_FALSE(luabridge::isInstance<double>(L, 1));
    }

    EXPECT_TRUE(luabridge::isInstance<long double>(L, 1));
}

TEST_F(StackTests, FloatNaNAndInfPush)
{
    {
        const luabridge::StackRestore sr(L);
        EXPECT_TRUE(luabridge::push(L, std::numeric_limits<float>::quiet_NaN()));
        EXPECT_TRUE(luabridge::isInstance<float>(L, -1));
        EXPECT_TRUE(luabridge::isInstance<double>(L, -1));
        EXPECT_TRUE(luabridge::isInstance<long double>(L, -1));
        auto result = luabridge::get<float>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_TRUE(std::isnan(*result));
    }

    {
        const luabridge::StackRestore sr(L);
        EXPECT_TRUE(luabridge::push(L, std::numeric_limits<float>::infinity()));
        EXPECT_TRUE(luabridge::isInstance<float>(L, -1));
        EXPECT_TRUE(luabridge::isInstance<double>(L, -1));
        EXPECT_TRUE(luabridge::isInstance<long double>(L, -1));
        auto result = luabridge::get<float>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_TRUE(std::isinf(*result));
    }

    {
        const luabridge::StackRestore sr(L);
        EXPECT_TRUE(luabridge::push(L, -std::numeric_limits<float>::infinity()));
        auto result = luabridge::get<float>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_TRUE(std::isinf(*result));
        EXPECT_LT(*result, 0.0f);
    }
}

TEST_F(StackTests, DoubleNaNAndInfPush)
{
    {
        const luabridge::StackRestore sr(L);
        EXPECT_TRUE(luabridge::push(L, std::numeric_limits<double>::quiet_NaN()));
        EXPECT_TRUE(luabridge::isInstance<float>(L, -1));
        EXPECT_TRUE(luabridge::isInstance<double>(L, -1));
        EXPECT_TRUE(luabridge::isInstance<long double>(L, -1));
        auto result = luabridge::get<double>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_TRUE(std::isnan(*result));
    }

    {
        const luabridge::StackRestore sr(L);
        EXPECT_TRUE(luabridge::push(L, std::numeric_limits<double>::infinity()));
        EXPECT_TRUE(luabridge::isInstance<float>(L, -1));
        EXPECT_TRUE(luabridge::isInstance<double>(L, -1));
        EXPECT_TRUE(luabridge::isInstance<long double>(L, -1));
        auto result = luabridge::get<double>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_TRUE(std::isinf(*result));
    }

    {
        const luabridge::StackRestore sr(L);
        EXPECT_TRUE(luabridge::push(L, -std::numeric_limits<double>::infinity()));
        auto result = luabridge::get<double>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_TRUE(std::isinf(*result));
        EXPECT_LT(*result, 0.0);
    }
}

TEST_F(StackTests, LongDoubleNaNAndInfPush)
{
    {
        const luabridge::StackRestore sr(L);
        EXPECT_TRUE(luabridge::push(L, std::numeric_limits<long double>::quiet_NaN()));
        EXPECT_TRUE(luabridge::isInstance<float>(L, -1));
        EXPECT_TRUE(luabridge::isInstance<double>(L, -1));
        EXPECT_TRUE(luabridge::isInstance<long double>(L, -1));
        auto result = luabridge::get<long double>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_TRUE(std::isnan(*result));
    }

    {
        const luabridge::StackRestore sr(L);
        EXPECT_TRUE(luabridge::push(L, std::numeric_limits<long double>::infinity()));
        EXPECT_TRUE(luabridge::isInstance<float>(L, -1));
        EXPECT_TRUE(luabridge::isInstance<double>(L, -1));
        EXPECT_TRUE(luabridge::isInstance<long double>(L, -1));
        auto result = luabridge::get<long double>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_TRUE(std::isinf(*result));
    }

    {
        const luabridge::StackRestore sr(L);
        EXPECT_TRUE(luabridge::push(L, -std::numeric_limits<long double>::infinity()));
        auto result = luabridge::get<long double>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_TRUE(std::isinf(*result));
        EXPECT_LT(*result, 0.0l);
    }
}

TEST_F(StackTests, CharArrayType)
{
    char value[] = "xyz";

    {
        EXPECT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<short>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned char>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned short>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned int>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned long long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<float>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long double>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<int>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<int>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<char>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<const char*>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<std::string_view>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<std::string>>(L, -1));

    {
        auto result = *luabridge::get<const char*>(L, -1);
        EXPECT_STREQ(value, result);
    }
}

TEST_F(StackTests, CharArrayStackOverflow)
{
    exhaustStackSpace();
    
    char value[] = "xyz";
    
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, ConstCharArrayType)
{
    const char value[] = "xyz";

    {
        EXPECT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<short>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned char>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned short>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned int>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned long long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<float>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long double>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<int>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<int>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<char>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<const char*>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<std::string_view>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<std::string>>(L, -1));

    {
        auto result = *luabridge::get<const char*>(L, -1);
        EXPECT_STREQ(value, result);
    }
}

TEST_F(StackTests, ConstCharArrayStackOverflow)
{
    exhaustStackSpace();
    
    const char value[] = "xyz";
    
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, ConstCharLiteralType)
{
    EXPECT_TRUE(luabridge::push(L, "xyz"));

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<short>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned char>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned short>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned int>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned long long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<float>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long double>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<int>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<int>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<char>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<const char*>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<std::string_view>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<std::string>>(L, -1));

    {
        auto result = *luabridge::get<const char*>(L, -1);
        EXPECT_STREQ("xyz", result);
    }
}

TEST_F(StackTests, ConstCharLiteralStackOverflow)
{
    exhaustStackSpace();
    
    ASSERT_FALSE(luabridge::push(L, "xyz"));
}

TEST_F(StackTests, ConstCharPointerType)
{
    // Normal value
    const char* value = "xyz";
    
    {
        EXPECT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<short>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned char>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned short>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned int>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned long long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<float>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long double>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<int>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<int>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<char>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<const char*>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<std::string_view>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<std::string>>(L, -1));

    {
        auto result = *luabridge::get<const char*>(L, -1);
        EXPECT_STREQ(value, result);
    }

    // Nullptr value
    const char* nullptr_value = nullptr;

    {
        EXPECT_TRUE(luabridge::push(L, nullptr_value));
    }

    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<short>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned char>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned short>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned int>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned long long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<float>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long double>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<const char*>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<const char*>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<char>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<const char*>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<std::string_view>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<std::string>>(L, -1));

    {
        auto result = luabridge::get<char>(L, -1);
        EXPECT_FALSE(result);
    }

    {
        auto result = luabridge::get<const char*>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_EQ(nullptr, *result);
    }

    {
        auto result = luabridge::get<std::string_view>(L, -1);
        EXPECT_FALSE(result);
    }

    {
        auto result = luabridge::get<std::string>(L, -1);
        EXPECT_FALSE(result);
    }

    {
        auto result = luabridge::get<std::optional<const char*>>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_FALSE(*result);
    }

    {
        auto result = luabridge::get<std::optional<std::string_view>>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_FALSE(*result);
    }

    {
        auto result = luabridge::get<std::optional<std::string>>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_FALSE(*result);
    }
}

TEST_F(StackTests, ConstCharPointerStackOverflow)
{
    exhaustStackSpace();
    
    const char* value = "xyz";
    
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, ConstCharPointerAsNullptr)
{
    ASSERT_TRUE(luabridge::push(L, luabridge::LuaNil()));

    {
        auto result = luabridge::get<const char*>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_EQ(nullptr, *result);
    }
}

TEST_F(StackTests, StringViewType)
{
    // Normal value
    std::string_view value = "xyz";
    
    {
        EXPECT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<short>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned char>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned short>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned int>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned long long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<float>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long double>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<std::string_view>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<std::string_view>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<char>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<const char*>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<std::string_view>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<std::string>>(L, -1));

    {
        auto result = luabridge::get<char>(L, -1);
        EXPECT_FALSE(result);
    }

    {
        auto result = *luabridge::get<std::string_view>(L, -1);
        EXPECT_EQ(value, result);
    }

    {
        auto result = *luabridge::get<const char*>(L, -1);
        EXPECT_EQ(value, result);
    }

    {
        auto result = *luabridge::get<std::string>(L, -1);
        EXPECT_EQ(value, result);
    }

    {
        auto result = *luabridge::get<std::optional<const char*>>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_EQ(value, *result);
    }

    {
        auto result = *luabridge::get<std::optional<std::string_view>>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_EQ(value, *result);
    }

    {
        auto result = *luabridge::get<std::optional<std::string>>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_EQ(value, *result);
    }
}

TEST_F(StackTests, StringViewStackOverflow)
{
    exhaustStackSpace();
    
    std::string_view value = "xyz";
    
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, StringViewInvalidType)
{
    ASSERT_TRUE(luabridge::push(L, luabridge::LuaNil()));

    {
        auto result = luabridge::get<std::string_view>(L, -1);
        EXPECT_FALSE(result);
    }
}

TEST_F(StackTests, StringType)
{
    // Normal value
    std::string value = "xyz";
    
    {
        EXPECT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<short>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned char>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned short>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned int>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned long long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<float>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long double>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<std::string>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<std::string>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<char>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<const char*>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<std::string_view>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<std::string>>(L, -1));

    EXPECT_EQ(value, (*luabridge::get<std::string>(L, -1)));
    EXPECT_EQ(value, (*luabridge::get<const char*>(L, -1)));

    {
        auto result = luabridge::get<char>(L, -1);
        EXPECT_FALSE(result);
    }

    {
        auto result = *luabridge::get<std::optional<const char*>>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_EQ(value, *result);
    }

    {
        auto result = *luabridge::get<std::optional<std::string_view>>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_EQ(value, *result);
    }

    {
        auto result = *luabridge::get<std::optional<std::string>>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_EQ(value, *result);
    }
}

TEST_F(StackTests, StringStackOverflow)
{
    exhaustStackSpace();
    
    std::string value = "xyz";
    
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, StringInvalidType)
{
    ASSERT_TRUE(luabridge::push(L, luabridge::LuaNil()));

    {
        auto result = luabridge::get<std::string>(L, -1);
        EXPECT_FALSE(result);
    }
}

TEST_F(StackTests, IntArrayType)
{
    int value[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 , 10 };

    {
        EXPECT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<short>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned char>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned short>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned int>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned long long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<float>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<int>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<char>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<const char*>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<std::string_view>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<std::string>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::vector<int>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<int[10]>(L, -1));
}

TEST_F(StackTests, IntArrayStackOverflow)
{
    exhaustStackSpace();
    
    int value[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 , 10 };
    
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, ConstIntArrayType)
{
    const int value[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    {
        EXPECT_TRUE(luabridge::push(L, value));
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<bool>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::byte>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<char>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<short>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned char>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned short>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned int>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<unsigned long long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<float>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<long double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<const char*>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string_view>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::tuple<int>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<const char*>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<std::string_view>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<std::string>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::vector<int>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<const int[10]>(L, -1));
}

TEST_F(StackTests, ConstIntArrayStackOverflow)
{
    exhaustStackSpace();
    
    const int value[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, ConstLongArrayStackOverflow)
{
    exhaustStackSpace();

    const long value[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, ConstLongLongArrayStackOverflow)
{
    exhaustStackSpace();

    const long long value[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, ConstUnsignedLongLongArrayStackOverflow)
{
    exhaustStackSpace();

    const unsigned long long value[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, OptionalStackOverflow)
{
    exhaustStackSpace();

    std::optional<int> value = 1;

    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, Pair)
{
    {
        auto value = std::make_pair(1, "one");
        ASSERT_TRUE(luabridge::push(L, value));

        auto result = luabridge::get<decltype(value)>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_EQ(1, std::get<0>(*result));
        EXPECT_STREQ("one", std::get<1>(*result));
    }

    {
        auto value = std::make_pair(1, std::string("one"));
        ASSERT_TRUE(luabridge::push(L, value));

        auto result = luabridge::get<std::pair<std::string, int>>(L, -1);
        ASSERT_FALSE(result);
        EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
    }

    {
        auto value = std::make_tuple(1, 2, 3);
        ASSERT_TRUE(luabridge::push(L, value));

        auto result = luabridge::get<std::pair<int, int>>(L, -1);
        ASSERT_FALSE(result);
        EXPECT_EQ(luabridge::ErrorCode::InvalidTableSizeInCast, result.error());
    }
}

TEST_F(StackTests, PairNesting)
{
    auto value = std::make_pair(1, std::make_pair(1, std::make_pair(1, std::string("one"))));
    ASSERT_TRUE(luabridge::push(L, value));

    auto result = luabridge::get<decltype(value)>(L, -1);
    ASSERT_TRUE(result);
    EXPECT_EQ(1, std::get<0>(*result));
    EXPECT_EQ(1, std::get<0>(std::get<1>(*result)));
    EXPECT_EQ(1, std::get<0>(std::get<1>(std::get<1>(*result))));
    EXPECT_EQ("one", std::get<1>(std::get<1>(std::get<1>(*result))));
}

TEST_F(StackTests, PairStackOverflow)
{
    exhaustStackSpace();

    auto value = std::make_pair(1, "one");

    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, PairFailUnregistered)
{
    {
        auto value = std::make_pair(Unregistered(), 1);

#if LUABRIDGE_HAS_EXCEPTIONS
        EXPECT_ANY_THROW(luabridge::push(L, value).error());
#else
        ASSERT_FALSE(luabridge::push(L, value));
        EXPECT_EQ(luabridge::ErrorCode::ClassNotRegistered, luabridge::push(L, value).error());
#endif
    }

    {
        auto value = std::make_pair(1, Unregistered());

#if LUABRIDGE_HAS_EXCEPTIONS
        EXPECT_ANY_THROW(luabridge::push(L, value).error());
#else
        ASSERT_FALSE(luabridge::push(L, value));
        EXPECT_EQ(luabridge::ErrorCode::ClassNotRegistered, luabridge::push(L, value).error());
#endif
    }
}

TEST_F(StackTests, Tuple)
{
    {
        auto value = std::make_tuple(1, 1.0f, "one");
        ASSERT_TRUE(luabridge::push(L, value));

        auto result = luabridge::get<decltype(value)>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_EQ(1, std::get<0>(*result));
        EXPECT_FLOAT_EQ(1.0f, std::get<1>(*result));
        EXPECT_STREQ("one", std::get<2>(*result));
    }

    {
        auto value = std::make_tuple(1, std::string("one"));
        ASSERT_TRUE(luabridge::push(L, value));

        auto result = luabridge::get<std::tuple<std::string, int>>(L, -1);
        ASSERT_FALSE(result);
        EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
    }

    {
        auto value = std::make_tuple(1, 2, 3);
        ASSERT_TRUE(luabridge::push(L, value));

        auto result = luabridge::get<std::tuple<int, int>>(L, -1);
        ASSERT_FALSE(result);
        EXPECT_EQ(luabridge::ErrorCode::InvalidTableSizeInCast, result.error());
    }
}

TEST_F(StackTests, TupleNesting)
{
    auto value = std::make_tuple(1, std::make_tuple(1, std::make_tuple(1, std::string("one"))));
    ASSERT_TRUE(luabridge::push(L, value));

    auto result = luabridge::get<decltype(value)>(L, -1);
    ASSERT_TRUE(result);
    EXPECT_EQ(1, std::get<0>(*result));
    EXPECT_EQ(1, std::get<0>(std::get<1>(*result)));
    EXPECT_EQ(1, std::get<0>(std::get<1>(std::get<1>(*result))));
    EXPECT_EQ("one", std::get<1>(std::get<1>(std::get<1>(*result))));
}

TEST_F(StackTests, TupleStackOverflow)
{
    exhaustStackSpace();

    auto value = std::make_tuple(1, 1.0f, "one");

    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, TupleFailUnregistered)
{
    {
        auto value = std::make_tuple(Unregistered(), 1);

#if LUABRIDGE_HAS_EXCEPTIONS
        EXPECT_ANY_THROW(luabridge::push(L, value).error());
#else
        ASSERT_FALSE(luabridge::push(L, value));
        EXPECT_EQ(luabridge::ErrorCode::ClassNotRegistered, luabridge::push(L, value).error());
#endif
    }

    {
        auto value = std::make_tuple(1, Unregistered());

#if LUABRIDGE_HAS_EXCEPTIONS
        EXPECT_ANY_THROW(luabridge::push(L, value).error());
#else
        ASSERT_FALSE(luabridge::push(L, value));
        EXPECT_EQ(luabridge::ErrorCode::ClassNotRegistered, luabridge::push(L, value).error());
#endif
    }
}

TEST_F(StackTests, NilStackOverflow)
{
    exhaustStackSpace();

    ASSERT_FALSE(luabridge::push(L, luabridge::LuaNil()));
}

TEST_F(StackTests, Nil)
{
    (void)luabridge::push(L, luabridge::LuaNil());

    ASSERT_TRUE(luabridge::isInstance<luabridge::LuaNil>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<bool>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<int>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<unsigned>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<float>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<double>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<std::string>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<std::string_view>(L, -1));
}

TEST_F(StackTests, Bool)
{
    (void)luabridge::push(L, true);

    ASSERT_FALSE(luabridge::isInstance<luabridge::LuaNil>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<luabridge::LuaNil>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<bool>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<int>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<unsigned>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<float>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<double>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<const char*>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<std::string>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<std::string_view>(L, -1));

    ASSERT_EQ(true, *luabridge::get<bool>(L, -1));
}

TEST_F(StackTests, Int)
{
    (void)luabridge::push(L, 5);

    ASSERT_FALSE(luabridge::isInstance<luabridge::LuaNil>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<bool>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<int>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<unsigned>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<float>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<double>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<const char*>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<std::string>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<std::string_view>(L, -1));

    ASSERT_EQ(5, *luabridge::get<int>(L, -1));
    ASSERT_NEAR(5.f, *luabridge::get<float>(L, -1), 1e-5);
    ASSERT_NEAR(5.0, *luabridge::get<double>(L, -1), 1e-6);
}

TEST_F(StackTests, Float)
{
    (void)luabridge::push(L, 3.14f);

    ASSERT_FALSE(luabridge::isInstance<luabridge::LuaNil>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<bool>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<int>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<unsigned>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<float>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<double>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<const char*>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<std::string>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<std::string_view>(L, -1));

    ASSERT_NEAR(3.14f, *luabridge::get<float>(L, -1), 1e-5);
    ASSERT_NEAR(3.14, *luabridge::get<double>(L, -1), 1e-6);
}

TEST_F(StackTests, CString)
{
    (void)luabridge::push(L, "abc");

    ASSERT_FALSE(luabridge::isInstance<luabridge::LuaNil>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<bool>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<int>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<unsigned>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<float>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<double>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<const char*>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<std::string>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<std::string_view>(L, -1));

    ASSERT_STREQ("abc", *luabridge::get<const char*>(L, -1));
    ASSERT_EQ("abc", *luabridge::get<std::string>(L, -1));
    ASSERT_EQ("abc", *luabridge::get<std::string_view>(L, -1));
}

TEST_F(StackTests, StdString)
{
    (void)luabridge::push(L, std::string("abc"));

    ASSERT_FALSE(luabridge::isInstance<luabridge::LuaNil>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<bool>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<int>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<unsigned>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<float>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<double>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<const char*>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<std::string>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<std::string_view>(L, -1));

    ASSERT_STREQ("abc", *luabridge::get<const char*>(L, -1));
    ASSERT_EQ("abc", *luabridge::get<std::string>(L, -1));
    ASSERT_EQ("abc", *luabridge::get<std::string_view>(L, -1));
}

TEST_F(StackTests, StdStringView)
{
    (void)luabridge::push(L, std::string_view("abc"));

    ASSERT_FALSE(luabridge::isInstance<luabridge::LuaNil>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<bool>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<int>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<unsigned>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<float>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<double>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<const char*>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<std::string>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<std::string_view>(L, -1));

    ASSERT_STREQ("abc", *luabridge::get<const char*>(L, -1));
    ASSERT_EQ("abc", *luabridge::get<std::string>(L, -1));
    ASSERT_EQ("abc", *luabridge::get<std::string_view>(L, -1));
}

TEST_F(StackTests, VoidPointer)
{
    {
        void* ptr = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0xdead1984ll));
        (void)luabridge::push(L, ptr);

        EXPECT_TRUE(luabridge::isInstance<void*>(L, -1));
        EXPECT_TRUE(luabridge::isInstance<const void*>(L, -1));

        {
            auto result = luabridge::get<void*>(L, -1);
            ASSERT_TRUE(result);
            EXPECT_EQ(ptr, *result);
        }

        {
            auto result = luabridge::get<const void*>(L, -1);
            ASSERT_TRUE(result);
            EXPECT_EQ(ptr, *result);
        }
    }

    {
        void* ptr = nullptr;
        (void)luabridge::push(L, ptr);

        EXPECT_TRUE(luabridge::isInstance<void*>(L, -1));
        EXPECT_TRUE(luabridge::isInstance<const void*>(L, -1));

        {
            auto result = luabridge::get<void*>(L, -1);
            ASSERT_TRUE(result);
            EXPECT_EQ(ptr, *result);
        }

        {
            auto result = luabridge::get<const void*>(L, -1);
            ASSERT_TRUE(result);
            EXPECT_EQ(ptr, *result);
        }
    }

    {
        lua_pushnumber(L, 0.0);

        EXPECT_FALSE(luabridge::isInstance<void*>(L, -1));

        {
            auto result = luabridge::get<void*>(L, -1);
            EXPECT_FALSE(result);
        }
    }
}

TEST_F(StackTests, VoidPointerStackOverflow)
{
    exhaustStackSpace();

    void* ptr = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0xdead1984ll));

    ASSERT_FALSE(luabridge::push(L, ptr));
}

TEST_F(StackTests, ConstVoidPointer)
{
    {
        const void* ptr = reinterpret_cast<const void*>(static_cast<std::uintptr_t>(0xdead1984ll));
        (void)luabridge::push(L, ptr);

        EXPECT_TRUE(luabridge::isInstance<void*>(L, -1));
        EXPECT_TRUE(luabridge::isInstance<const void*>(L, -1));

        {
            auto result = luabridge::get<void*>(L, -1);
            ASSERT_TRUE(result);
            EXPECT_EQ(ptr, *result);
        }

        {
            auto result = luabridge::get<const void*>(L, -1);
            ASSERT_TRUE(result);
            EXPECT_EQ(ptr, *result);
        }
    }

    {
        const void* ptr = nullptr;
        (void)luabridge::push(L, ptr);

        EXPECT_TRUE(luabridge::isInstance<void*>(L, -1));
        EXPECT_TRUE(luabridge::isInstance<const void*>(L, -1));

        {
            auto result = luabridge::get<void*>(L, -1);
            ASSERT_TRUE(result);
            EXPECT_EQ(ptr, *result);
        }

        {
            auto result = luabridge::get<const void*>(L, -1);
            ASSERT_TRUE(result);
            EXPECT_EQ(ptr, *result);
        }
    }

    {
        lua_pushnumber(L, 0.0);

        EXPECT_FALSE(luabridge::isInstance<const void*>(L, -1));

        {
            auto result = luabridge::get<const void*>(L, -1);
            EXPECT_FALSE(result);
        }
    }
}

TEST_F(StackTests, ConstVoidPointerStackOverflow)
{
    exhaustStackSpace();

    const void* ptr = reinterpret_cast<const void*>(static_cast<std::uintptr_t>(0xdead1984ll));

    ASSERT_FALSE(luabridge::push(L, ptr));
}

TEST_F(StackTests, VoidPointerGetNil)
{
    lua_pushnil(L);

    {
        auto result = luabridge::get<void*>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_EQ(nullptr, *result);
    }

    {
        auto result = luabridge::get<const void*>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_EQ(nullptr, *result);
    }
}

TEST_F(StackTests, LongLongType)
{
    long long value = 42ll;

    ASSERT_TRUE(luabridge::push(L, value));

    auto result = luabridge::get<long long>(L, -1);
    ASSERT_TRUE(result);
    EXPECT_EQ(value, *result);

    EXPECT_TRUE(luabridge::isInstance<long long>(L, -1));
}

TEST_F(StackTests, LongLongStackOverflow)
{
    exhaustStackSpace();

    ASSERT_FALSE(luabridge::push(L, 42ll));
}

TEST_F(StackTests, LongLongInvalidType)
{
    (void)luabridge::Stack<std::string_view>::push(L, "not_a_number");

    auto result = luabridge::Stack<long long>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(StackTests, LongLongNotFittingGet)
{
    // Push a non-integer float - tointeger returns isValid=0, can't represent as long long
    lua_pushnumber(L, 1.5);

    auto result = luabridge::Stack<long long>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::IntegerDoesntFitIntoLuaInteger, result.error());
}

TEST_F(StackTests, LongLongNotFittingPush)
{
    if constexpr (sizeof(long long) > sizeof(lua_Integer))
    {
        long long value = static_cast<long long>(std::numeric_limits<lua_Integer>::max()) + 1ull;
        auto result = luabridge::push(L, value);
        ASSERT_FALSE(result);
        EXPECT_EQ(luabridge::ErrorCode::IntegerDoesntFitIntoLuaInteger, result.error());
    }
}

TEST_F(StackTests, UlongLongNotFittingPush)
{
    if constexpr (sizeof(unsigned long long) == sizeof(lua_Integer) && !std::is_unsigned_v<lua_Integer>)
    {
        // Value > LLONG_MAX can't fit in signed lua_Integer
        auto result = luabridge::push(L, 9223372036854775808ull);
        ASSERT_FALSE(result);
        EXPECT_EQ(luabridge::ErrorCode::IntegerDoesntFitIntoLuaInteger, result.error());
    }
}

TEST_F(StackTests, UlongLongType)
{
    unsigned long long value = 42ull;

    ASSERT_TRUE(luabridge::push(L, value));

    auto result = luabridge::get<unsigned long long>(L, -1);
    ASSERT_TRUE(result);
    EXPECT_EQ(value, *result);

    EXPECT_TRUE(luabridge::isInstance<unsigned long long>(L, -1));
}

TEST_F(StackTests, UlongLongStackOverflow)
{
    exhaustStackSpace();

    ASSERT_FALSE(luabridge::push(L, 42ull));
}

TEST_F(StackTests, UlongLongInvalidType)
{
    (void)luabridge::Stack<std::string_view>::push(L, "not_a_number");

    auto result = luabridge::Stack<unsigned long long>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(StackTests, UnsignedIntNotFittingGet)
{
    // Push a negative number - can't fit in unsigned int
    lua_pushinteger(L, -1);

    auto result = luabridge::Stack<unsigned int>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::IntegerDoesntFitIntoLuaInteger, result.error());
}

TEST_F(StackTests, LongNotFittingGet)
{
    if constexpr (sizeof(long) < sizeof(lua_Integer))
    {
        // Push a value larger than LONG_MAX
        lua_pushinteger(L, static_cast<lua_Integer>(static_cast<unsigned long>(std::numeric_limits<long>::max()) + 1u));

        auto result = luabridge::Stack<long>::get(L, -1);
        ASSERT_FALSE(result);
        EXPECT_EQ(luabridge::ErrorCode::IntegerDoesntFitIntoLuaInteger, result.error());
    }
}

TEST_F(StackTests, UnsignedLongInvalidType)
{
    (void)luabridge::Stack<std::string_view>::push(L, "not_a_number");

    auto result = luabridge::Stack<unsigned long>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(StackTests, UnsignedLongNotFittingGet)
{
    lua_pushnumber(L, 18446744073709551616.0);
    auto r = luabridge::Stack<unsigned long>::get(L, -1);
    ASSERT_FALSE(r);
    EXPECT_EQ(luabridge::ErrorCode::IntegerDoesntFitIntoLuaInteger, r.error());
}

TEST_F(StackTests, UlongLongNotFittingGet)
{
    lua_pushnumber(L, 18446744073709551616.0);
    auto r = luabridge::Stack<unsigned long long>::get(L, -1);
    ASSERT_FALSE(r);
    EXPECT_EQ(luabridge::ErrorCode::IntegerDoesntFitIntoLuaInteger, r.error());
}

#if !LUABRIDGE_STRICT_STACK_CONVERSIONS
TEST_F(StackTests, StringGetFromNumberStackOverflow)
{
    // Push a number (will trigger string coercion path in non-strict mode)
    lua_pushnumber(L, 42.0);

    exhaustStackSpace();

    // In non-strict mode, get<string> tries lua_pushvalue which fails on overflow
    auto result = luabridge::Stack<std::string>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::LuaStackOverflow, result.error());
}
#endif

TEST_F(StackTests, FloatNotFittingGet)
{
    // Push lua_Number::max which doesn't fit in float
    (void)luabridge::push(L, std::numeric_limits<lua_Number>::max());

    auto result = luabridge::Stack<float>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::FloatingPointDoesntFitIntoLuaNumber, result.error());
}

TEST_F(StackTests, ConstCharPtrGetNil)
{
    lua_pushnil(L);
    auto r = luabridge::Stack<const char*>::get(L, -1);
    ASSERT_TRUE(r);
    EXPECT_EQ(nullptr, *r);
}

TEST_F(StackTests, OptionalNulloptStackOverflow)
{
    exhaustStackSpace();

    std::optional<int> value = std::nullopt;
    ASSERT_FALSE(luabridge::push(L, value));
}

TEST_F(StackTests, OptionalGetInnerError)
{
    // Push a string (not nil, but can't be converted to int)
    (void)luabridge::Stack<std::string_view>::push(L, "not_a_number");

    auto result = luabridge::Stack<std::optional<int>>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(StackTests, PairGetNonTable)
{
    lua_pushnumber(L, 42.0);

    auto result = luabridge::Stack<std::pair<int, int>>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(StackTests, PairGetFirstElementError)
{
    // Push pair<string, string> but try to get as pair<int, string>
    auto value = std::make_pair(std::string("not_int"), std::string("second"));
    ASSERT_TRUE(luabridge::push(L, value));

    auto result = luabridge::Stack<std::pair<int, std::string>>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(StackTests, TupleGetNonTable)
{
    lua_pushnumber(L, 42.0);

    auto result = luabridge::Stack<std::tuple<int, int>>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(StackTests, TupleGetFirstElementError)
{
    // Push tuple<string, string> but try to get as tuple<int, string>
    auto value = std::make_tuple(std::string("not_int"), std::string("second"));
    ASSERT_TRUE(luabridge::push(L, value));

    auto result = luabridge::Stack<std::tuple<int, std::string>>::get(L, -1);
    ASSERT_FALSE(result);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, result.error());
}

TEST_F(StackTests, ErrorCodeMessages)
{
    EXPECT_STREQ("The class is not registered in LuaBridge",
        luabridge::makeErrorCode(luabridge::ErrorCode::ClassNotRegistered).message().c_str());

    EXPECT_STREQ("The lua stack has overflow",
        luabridge::makeErrorCode(luabridge::ErrorCode::LuaStackOverflow).message().c_str());

    EXPECT_STREQ("The lua function invocation raised an error",
        luabridge::makeErrorCode(luabridge::ErrorCode::LuaFunctionCallFailed).message().c_str());

    EXPECT_STREQ("The native integer can't fit inside a lua integer",
        luabridge::makeErrorCode(luabridge::ErrorCode::IntegerDoesntFitIntoLuaInteger).message().c_str());

    EXPECT_STREQ("The native floating point can't fit inside a lua number",
        luabridge::makeErrorCode(luabridge::ErrorCode::FloatingPointDoesntFitIntoLuaNumber).message().c_str());

    EXPECT_STREQ("The lua object can't be cast to desired type",
        luabridge::makeErrorCode(luabridge::ErrorCode::InvalidTypeCast).message().c_str());

    EXPECT_STREQ("The lua table has different size than expected",
        luabridge::makeErrorCode(luabridge::ErrorCode::InvalidTableSizeInCast).message().c_str());

    // Default case: an unknown error code value
    EXPECT_STREQ("Unknown error",
        luabridge::detail::ErrorCategory::getInstance().message(9999).c_str());
}

TEST_F(StackTests, GetGlobalTyped)
{
    // Test the template getGlobal<T> from Globals.h (not the LuaRef version from LuaRef.h)
    lua_pushinteger(L, 42);
    lua_setglobal(L, "test_int_global");

    auto result = luabridge::getGlobal<int>(L, "test_int_global");
    ASSERT_TRUE(result);
    EXPECT_EQ(42, *result);
}

TEST_F(StackTests, SetGlobalFailure)
{
    struct Unregistered2 {};

    // setGlobal should return false when push fails (non-exception mode only)
#if !LUABRIDGE_HAS_EXCEPTIONS
    bool ok = luabridge::setGlobal(L, Unregistered2{}, "test_var");
    EXPECT_FALSE(ok);
#endif
}

TEST_F(StackTests, ResultCheck)
{
#if LUABRIDGE_HAS_EXCEPTIONS
    EXPECT_ANY_THROW(luabridge::push(L, std::optional<Unregistered>(Unregistered{})).error());
#else
    EXPECT_EQ(luabridge::ErrorCode::ClassNotRegistered, luabridge::push(L, std::optional<Unregistered>(Unregistered{})).error());
#endif
}

TEST_F(StackTests, TypeResultCheck)
{
    (void)luabridge::push(L, std::string_view("abc"));

    EXPECT_EQ(std::string{ "abc" }, luabridge::get<std::string>(L, -1));
    EXPECT_EQ(luabridge::get<std::string>(L, -1), std::string{ "abc" });
    EXPECT_NE(std::string{ "123" }, luabridge::get<std::string>(L, -1));
    EXPECT_NE(luabridge::get<std::string>(L, -1), std::string{ "123" });

    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, luabridge::get<int>(L, -1).error());
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, static_cast<std::error_code>(luabridge::get<int>(L, -1)));
}

TEST_F(StackTests, TypeResultValueOr)
{
    (void)luabridge::push(L, std::string_view("abc"));
    EXPECT_EQ(1337, luabridge::get<int>(L, -1).valueOr(1337));
    auto result1 = luabridge::get<int>(L, -1);
    EXPECT_EQ(1337, result1.valueOr(1337));

    (void)luabridge::push(L, 42);
    EXPECT_EQ(42, luabridge::get<int>(L, -1).valueOr(1337));
    auto result2 = luabridge::get<int>(L, -1);
    EXPECT_EQ(42, result2.valueOr(1337));
}

TEST_F(StackTests, ArrayPushElementFailure)
{
    // When pushing a fixed-size array, if an element push fails the error is
    // propagated immediately (Stack.h:1330).
    if constexpr (sizeof(long double) > sizeof(lua_Number))
    {
        long double arr[2] = { std::numeric_limits<long double>::max(), 1.0L };
        auto result = luabridge::push(L, arr);
        ASSERT_FALSE(result);
        EXPECT_EQ(luabridge::ErrorCode::FloatingPointDoesntFitIntoLuaNumber, result.error());
    }
}

namespace {
template <class T, std::size_t S>
struct Array
{
    Array() = default;

    T& operator[](std::size_t index) { return value; }
    const T& operator[](std::size_t index) const { return value; }

    T value;
};

using Color = Array<float, 4>;

struct Sprite
{
    void SetColor(const Color& col) {}
    void SetColor(const Color& col, size_t index) {}
};
} // namespace

namespace luabridge {
template <class T, std::size_t Size>
struct Stack<Array<T, Size>>
{
    using Type = Array<T, Size>;

    [[nodiscard]] static Result push(lua_State* L, const Type& array)
    {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (!lua_checkstack(L, 3))
            return makeErrorCode(ErrorCode::LuaStackOverflow);
#endif

        StackRestore stackRestore(L);

        lua_createtable(L, static_cast<int>(Size), 0);

        for (std::size_t i = 0; i < Size; ++i)
        {
            lua_pushinteger(L, static_cast<lua_Integer>(i + 1));

            auto result = Stack<T>::push(L, array[i]);
            if (!result)
                return result;

            lua_settable(L, -3);
        }

        stackRestore.reset();
        return {};
    }

    [[nodiscard]] static TypeResult<Type> get(lua_State* L, int index)
    {
        if (!lua_istable(L, index))
            return makeErrorCode(ErrorCode::InvalidTypeCast);

        if (get_length(L, index) != Size)
            return makeErrorCode(ErrorCode::InvalidTableSizeInCast);

        const StackRestore stackRestore(L);

        Type array;

        int absIndex = lua_absindex(L, index);
        lua_pushnil(L);

        int arrayIndex = 0;
        while (lua_next(L, absIndex) != 0)
        {
            auto item = Stack<T>::get(L, -1);
            if (!item)
                return makeErrorCode(ErrorCode::InvalidTypeCast);

            array[arrayIndex++] = *item;
            lua_pop(L, 1);
        }

        return array;
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index)
    {
        return lua_istable(L, index) && get_length(L, index) == Size;
    }
};
} // namespace luabridge

TEST_F(StackTests, OverloadTriggersStackTemplateCompilationError)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Sprite>("Sprite")
        .addFunction("SetColor",
            luabridge::overload<const Color&>(&Sprite::SetColor),
            luabridge::overload<const Color&, size_t>(&Sprite::SetColor))
        .endClass();
        
    SUCCEED();
}

#if LUABRIDGE_HAS_EXCEPTIONS
TEST_F(StackTests, PcallFailureThrowsException)
{
    luabridge::lua_pushcfunction_x(L, [](lua_State* L) -> int {
        luaL_error(L, "deliberate error");
        return 0;
    }, "");

    EXPECT_THROW(luabridge::pcall(L, 0, 0), luabridge::LuaException);
}

TEST_F(StackTests, ResultThrowOnError)
{
    exhaustStackSpace();

    EXPECT_THROW(luabridge::push(L, 1337).throw_on_error(), std::system_error);
}

TEST_F(StackTests, ResultNoThrowOnError)
{
    EXPECT_NO_THROW(luabridge::push(L, 1337).throw_on_error());
}

TEST_F(StackTests, TypeResultThrowOnError)
{
    (void)luabridge::push(L, std::string_view("abc"));

    EXPECT_THROW(luabridge::get<int>(L, -1).throw_on_error(), std::system_error);
}

TEST_F(StackTests, TypeResultNoThrowOnError)
{
    (void)luabridge::push(L, std::string_view("abc"));

    EXPECT_NO_THROW(luabridge::get<std::string>(L, -1).throw_on_error());
}
#endif
