// https://github.com/kunitoki/LuaBridge3
// Copyright 2021, Lucio Asnaghi
// Copyright 2020, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"

struct StackTests : TestBase
{
};

TEST_F(StackTests, Void)
{
    std::error_code ec;
    bool result = luabridge::Stack<void>::push(L, ec);
    ASSERT_TRUE(result);
}

TEST_F(StackTests, NullptrType)
{
    {
        std::error_code ec;
        bool result = luabridge::push(L, nullptr, ec);
        ASSERT_TRUE(result);
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
        EXPECT_FALSE(luabridge::isInstance<const char*>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<std::string_view>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<std::tuple<std::nullptr_t>>(L, -1));
        EXPECT_FALSE(luabridge::isInstance<std::vector<std::nullptr_t>>(L, -1));
        EXPECT_TRUE(luabridge::isInstance<std::optional<std::nullptr_t>>(L, -1));
    }
    
    {
        auto result = luabridge::get<std::nullptr_t>(L, -1);
        EXPECT_EQ(nullptr, result);
    }
}

TEST_F(StackTests, NullptrStackOverflow)
{
    exhaustStackSpace();
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::Stack<std::nullptr_t>::push(L, nullptr, ec));
    EXPECT_FALSE(ec.message().empty());
}

TEST_F(StackTests, LuaStateType)
{
    {
        auto result = luabridge::get<lua_State*>(L, -1);
        EXPECT_TRUE(result);
        EXPECT_EQ(L, result);
    }
}

TEST_F(StackTests, LuaCFunctionType)
{
    lua_CFunction value = +[](lua_State*) { return 0; };

    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        ASSERT_TRUE(result);
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
        auto result = luabridge::get<lua_CFunction>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, LuaCFunctionStackOverflow)
{
    exhaustStackSpace();
    
    lua_CFunction value = +[](lua_State*) { return 0; };
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}

TEST_F(StackTests, BoolType)
{
    bool value = true;

    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        ASSERT_TRUE(result);
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
        auto result = luabridge::get<bool>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, BoolStackOverflow)
{
    exhaustStackSpace();
    
    bool value = true;
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}

TEST_F(StackTests, CharType)
{
    char value = 'a';

    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        ASSERT_TRUE(result);
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
        auto result = luabridge::get<char>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, CharStackOverflow)
{
    exhaustStackSpace();
    
    char value = 'a';
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}

TEST_F(StackTests, StdByteType)
{
    std::byte value{ 128 };

    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        EXPECT_TRUE(result);
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
        auto result = luabridge::get<std::byte>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, StdByteStackOVerflow)
{
    exhaustStackSpace();
    
    std::byte value { 128 };
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}

TEST_F(StackTests, Int8Type)
{
    int8_t value = 127;

    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        ASSERT_TRUE(result);
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
        auto result = luabridge::get<int8_t>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, Int8StackOverflow)
{
    exhaustStackSpace();

    int8_t value = 127;
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}

TEST_F(StackTests, Int16Type)
{
    int16_t value = 32767;

    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        ASSERT_TRUE(result);
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
        auto result = luabridge::get<int16_t>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, Int16StackOverflow)
{
    exhaustStackSpace();

    int16_t value = 32767;
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}

TEST_F(StackTests, Int32Type)
{
    int32_t value = 2147483647;

    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        ASSERT_TRUE(result);
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
        auto result = luabridge::get<int32_t>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, Int32StackOverflow)
{
    exhaustStackSpace();

    int32_t value = 2147483647;
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}

TEST_F(StackTests, Int64Type)
{
    constexpr int64_t max_integral = static_cast<int64_t>(std::numeric_limits<lua_Integer>::max());
    
    int64_t value = max_integral < 4294967296ll ? max_integral : 4294967296ll;

    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        ASSERT_TRUE(result);
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    if constexpr (sizeof(lua_Integer) == sizeof(int32_t))
        EXPECT_TRUE(luabridge::isInstance<int32_t>(L, -1));
    else
        EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    if constexpr (sizeof(lua_Integer) == sizeof(uint32_t))
        EXPECT_TRUE(luabridge::isInstance<uint32_t>(L, -1));
    else
        EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, -1));
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
        EXPECT_TRUE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
    else
        EXPECT_FALSE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<int64_t>>(L, -1));

    {
        auto result = luabridge::get<int64_t>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, Int64StackOverflow)
{
    exhaustStackSpace();
    
    int64_t value = 42;
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}

TEST_F(StackTests, Uint8Type)
{
    uint8_t value = 128;

    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        EXPECT_TRUE(result);
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
        auto result = luabridge::get<uint8_t>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, Uint8StackOverflow)
{
    exhaustStackSpace();
    
    uint8_t value = 42;
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}

TEST_F(StackTests, Uint16Type)
{
    uint16_t value = 32768;

    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        EXPECT_TRUE(result);
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
        auto result = luabridge::get<uint16_t>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, Uint16StackOverflow)
{
    exhaustStackSpace();
    
    uint16_t value = 42;
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}

TEST_F(StackTests, Uint32Type)
{
    constexpr uint32_t max_integral = static_cast<uint32_t>(std::numeric_limits<lua_Integer>::max());
    
    uint32_t value = max_integral < 2147483648u ? max_integral : 2147483648u;

    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        EXPECT_TRUE(result);
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    if constexpr (sizeof(lua_Integer) == sizeof(int32_t))
        EXPECT_TRUE(luabridge::isInstance<int32_t>(L, -1));
    else
        EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
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
        EXPECT_TRUE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
    else
        EXPECT_FALSE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint32_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<int64_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint64_t>>(L, -1));

    {
        auto result = luabridge::get<uint32_t>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, Uint32StackOverflow)
{
    exhaustStackSpace();
    
    uint32_t value = 42;
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}

TEST_F(StackTests, Uint64Type)
{
    uint64_t value = static_cast<uint64_t>(std::numeric_limits<int32_t>::max());

    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        EXPECT_TRUE(result);
    }

    EXPECT_FALSE(luabridge::isInstance<std::nullptr_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<lua_CFunction>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int16_t>(L, -1));
    if constexpr (sizeof(lua_Integer) >= sizeof(int32_t))
        EXPECT_TRUE(luabridge::isInstance<int32_t>(L, -1));
    else
        EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
    if constexpr (sizeof(lua_Integer) >= sizeof(uint32_t))
        EXPECT_TRUE(luabridge::isInstance<uint32_t>(L, -1));
    else
        EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, -1));
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
        auto result = luabridge::get<uint64_t>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, Uint64StackOverflow)
{
    exhaustStackSpace();
    
    uint64_t value = 42;
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}

TEST_F(StackTests, IntTypeNotFittingPush)
{
    std::error_code ec;

    if constexpr (sizeof(uint32_t) == sizeof(lua_Integer) && ! std::is_unsigned_v<lua_Integer>)
    {
        uint32_t value = 4294967295u;

        EXPECT_FALSE(luabridge::push(L, value, ec));
        EXPECT_STREQ("luabridge", ec.category().name());
    }

    if constexpr (sizeof(uint64_t) == sizeof(lua_Integer) && ! std::is_unsigned_v<lua_Integer>)
    {
        uint64_t value = 9223372036854775808ull;

        EXPECT_FALSE(luabridge::push(L, value, ec));
        EXPECT_STREQ("luabridge", ec.category().name());
    }
}

TEST_F(StackTests, IntTypeNotFittingIsInstance)
{
    std::error_code ec;
    
    if constexpr (sizeof(uint32_t) == sizeof(lua_Integer))
    {
        const luabridge::StackRestore sr(L);

        EXPECT_TRUE(luabridge::push(L, 2147483647, ec));
        EXPECT_FALSE(luabridge::isInstance<std::byte>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<int8_t>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<int16_t>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, 1));
    }
    
    if constexpr (sizeof(uint64_t) == sizeof(lua_Integer))
    {
        const luabridge::StackRestore sr(L);

        EXPECT_TRUE(luabridge::push(L, 9223372036854775807ll, ec));
        EXPECT_FALSE(luabridge::isInstance<std::byte>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<int8_t>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<int16_t>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<int32_t>(L, 1));
        EXPECT_FALSE(luabridge::isInstance<uint32_t>(L, 1));
    }
}

TEST_F(StackTests, FloatType)
{
    float value = 123.5678f;

    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        EXPECT_TRUE(result);
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
        auto result = luabridge::get<float>(L, -1);
        EXPECT_FLOAT_EQ(value, result);
    }
}

TEST_F(StackTests, FloatStackOverflow)
{
    exhaustStackSpace();
    
    float value = 42.0f;
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}

TEST_F(StackTests, DoubleType)
{
    double value = 123.5678;

    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        EXPECT_TRUE(result);
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
        auto result = luabridge::get<double>(L, -1);
        EXPECT_DOUBLE_EQ(value, result);
    }
}

TEST_F(StackTests, DoubleStackOverflow)
{
    exhaustStackSpace();
    
    double value = 42.0;
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}

TEST_F(StackTests, LongDoubleType)
{
    long double value = 123.5678l;

    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        EXPECT_TRUE(result);
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
        auto result = luabridge::get<long double>(L, -1);
        EXPECT_DOUBLE_EQ(value, result);
    }
}

TEST_F(StackTests, LongDoubleStackOverflow)
{
    exhaustStackSpace();
    
    long double value = 42.0l;
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}

TEST_F(StackTests, FloatTypeNotFittingPush)
{
    std::error_code ec;

    if constexpr (sizeof(double) > sizeof(lua_Number))
    {
        double value = std::numeric_limits<double>::max();

        EXPECT_FALSE(luabridge::push(L, value, ec));
        EXPECT_STREQ("luabridge", ec.category().name());
    }

    if constexpr (sizeof(long double) > sizeof(double) && sizeof(long double) > sizeof(lua_Number))
    {
        long double value = std::numeric_limits<long double>::max();

        EXPECT_FALSE(luabridge::push(L, value, ec));
        EXPECT_STREQ("luabridge", ec.category().name());
    }
}

TEST_F(StackTests, FloatTypeNotFittingIsInstance)
{
    std::error_code ec;

    const luabridge::StackRestore sr(L);

    EXPECT_TRUE(luabridge::push(L, std::numeric_limits<lua_Number>::max(), ec));

    EXPECT_FALSE(luabridge::isInstance<float>(L, 1));

    if constexpr (sizeof(double) == sizeof(lua_Number))
        EXPECT_TRUE(luabridge::isInstance<double>(L, 1));
    else if constexpr (sizeof(long double) > sizeof(double) && sizeof(long double) == sizeof(lua_Number))
        EXPECT_FALSE(luabridge::isInstance<double>(L, 1));

    EXPECT_TRUE(luabridge::isInstance<long double>(L, 1));
}

TEST_F(StackTests, CharArrayType)
{
    char value[] = "xyz";

    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        EXPECT_TRUE(result);
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
        auto result = luabridge::get<const char*>(L, -1);
        EXPECT_STREQ(value, result);
    }
}

TEST_F(StackTests, CharArrayStackOverflow)
{
    exhaustStackSpace();
    
    char value[] = "xyz";
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}

TEST_F(StackTests, ConstCharArrayType)
{
    const char value[] = "xyz";

    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        EXPECT_TRUE(result);
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
        auto result = luabridge::get<const char*>(L, -1);
        EXPECT_STREQ(value, result);
    }
}

TEST_F(StackTests, ConstCharArrayStackOverflow)
{
    exhaustStackSpace();
    
    const char value[] = "xyz";
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}

TEST_F(StackTests, ConstCharLiteralType)
{
    std::error_code ec;
    bool result = luabridge::push(L, "xyz", ec);
    EXPECT_TRUE(result);

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
        auto result = luabridge::get<const char*>(L, -1);
        EXPECT_STREQ("xyz", result);
    }
}

TEST_F(StackTests, ConstCharLiteralStackOverflow)
{
    exhaustStackSpace();
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, "xyz", ec));
}

TEST_F(StackTests, ConstCharPointerType)
{
    // Normal value
    const char* value = "xyz";
    
    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        EXPECT_TRUE(result);
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
        auto result = luabridge::get<const char*>(L, -1);
        EXPECT_STREQ(value, result);
    }

    // Nullptr value
    const char* nullptr_value = nullptr;

    {
        std::error_code ec;
        bool result = luabridge::push(L, nullptr_value, ec);
        EXPECT_TRUE(result);
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
    EXPECT_FALSE(luabridge::isInstance<std::tuple<const char*>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<const char*>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<char>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<const char*>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<std::string_view>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<std::string>>(L, -1));

    {
        auto result = luabridge::get<char>(L, -1);
        EXPECT_EQ('\0', result);
    }

    {
        auto result = luabridge::get<const char*>(L, -1);
        EXPECT_STREQ("", result);
    }

    {
        auto result = luabridge::get<std::string_view>(L, -1);
        EXPECT_EQ("", result);
    }

    {
        auto result = luabridge::get<std::string>(L, -1);
        EXPECT_EQ("", result);
    }

    {
        auto result = luabridge::get<std::optional<const char*>>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_STREQ("", *result);
    }

    {
        auto result = luabridge::get<std::optional<std::string_view>>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_EQ("", *result);
    }

    {
        auto result = luabridge::get<std::optional<std::string>>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_EQ("", *result);
    }
}

TEST_F(StackTests, ConstCharPointerStackOverflow)
{
    exhaustStackSpace();
    
    const char* value = "xyz";
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}

TEST_F(StackTests, StringViewType)
{
    // Normal value
    std::string_view value = "xyz";
    
    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        EXPECT_TRUE(result);
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
        auto result = luabridge::get<std::string_view>(L, -1);
        EXPECT_EQ(value, result);
    }

    {
        auto result = luabridge::get<char>(L, -1);
        EXPECT_EQ(value[0], result);
    }

    {
        auto result = luabridge::get<const char*>(L, -1);
        EXPECT_EQ(value, result);
    }

    {
        auto result = luabridge::get<std::string>(L, -1);
        EXPECT_EQ(value, result);
    }

    {
        auto result = luabridge::get<std::optional<const char*>>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_EQ(value, *result);
    }

    {
        auto result = luabridge::get<std::optional<std::string_view>>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_EQ(value, *result);
    }

    {
        auto result = luabridge::get<std::optional<std::string>>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_EQ(value, *result);
    }
}

TEST_F(StackTests, StringViewStackOverflow)
{
    exhaustStackSpace();
    
    std::string_view value = "xyz";
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}

TEST_F(StackTests, StringType)
{
    // Normal value
    std::string value = "xyz";
    
    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        EXPECT_TRUE(result);
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

    EXPECT_EQ(value, (luabridge::get<std::string>(L, -1)));
    EXPECT_EQ(value[0], (luabridge::get<char>(L, -1)));
    EXPECT_EQ(value, (luabridge::get<const char*>(L, -1)));

    {
        auto result = luabridge::get<std::optional<const char*>>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_EQ(value, *result);
    }

    {
        auto result = luabridge::get<std::optional<std::string_view>>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_EQ(value, *result);
    }

    {
        auto result = luabridge::get<std::optional<std::string>>(L, -1);
        ASSERT_TRUE(result);
        EXPECT_EQ(value, *result);
    }
}

TEST_F(StackTests, StringStackOverflow)
{
    exhaustStackSpace();
    
    std::string value = "xyz";
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}

TEST_F(StackTests, IntArrayType)
{
    int value[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 , 10 };

    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        EXPECT_TRUE(result);
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
    EXPECT_FALSE(luabridge::isInstance<std::vector<int>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<char>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<const char*>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<std::string_view>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<std::string>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<int[10]>(L, -1));
}

TEST_F(StackTests, IntArrayStackOverflow)
{
    exhaustStackSpace();
    
    int value[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 , 10 };
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}

TEST_F(StackTests, ConstIntArrayType)
{
    const int value[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        EXPECT_TRUE(result);
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
    EXPECT_FALSE(luabridge::isInstance<std::vector<int>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<const char*>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<std::string_view>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<std::string>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<const int[10]>(L, -1));
}

TEST_F(StackTests, ConstIntArrayStackOverflow)
{
    exhaustStackSpace();
    
    const int value[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    
    std::error_code ec;
    ASSERT_FALSE(luabridge::push(L, value, ec));
}
