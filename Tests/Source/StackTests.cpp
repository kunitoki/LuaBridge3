// https://github.com/kunitoki/LuaBridge3
// Copyright 2021, Lucio Asnaghi
// Copyright 2020, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"

struct StackTests : TestBase
{
};

TEST_F(StackTests, NullptrType)
{
    {
        std::error_code ec;
        bool result = luabridge::push(L, nullptr, ec);
        EXPECT_TRUE(result);
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
        EXPECT_TRUE(result);
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

TEST_F(StackTests, BoolType)
{
    bool value = true;

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

TEST_F(StackTests, CharType)
{
    char value = 'a';

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

TEST_F(StackTests, Int8Type)
{
    int8_t value = 127;

    {
        std::error_code ec;
        bool result = luabridge::push(L, value, ec);
        EXPECT_TRUE(result);
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


TEST_F(StackTests, Int16Type)
{
    int16_t value = 32767;

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

TEST_F(StackTests, Int32Type)
{
    int32_t value = 2147483647;

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

TEST_F(StackTests, DISABLED_Int64Type)
{
    int64_t value = 4294967296ll;

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
    EXPECT_TRUE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint8_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<uint16_t>(L, -1));
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
    EXPECT_FALSE(luabridge::isInstance<std::tuple<int32_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::vector<int32_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int16_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<int64_t>>(L, -1));

    {
        auto result = luabridge::get<long>(L, -1);
        EXPECT_EQ(value, result);
    }
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

TEST_F(StackTests, DISABLED_Uint32Type)
{
    uint32_t value = 2147483648;

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
    EXPECT_FALSE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint32_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<int64_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint64_t>>(L, -1));

    {
        auto result = luabridge::get<uint32_t>(L, -1);
        EXPECT_EQ(value, result);
    }
}

TEST_F(StackTests, DISABLED_Uint64Type) // FAIL: because lua_Integer is long long this is stored as -1
{
    uint64_t value = 9223372036854775808ul;

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
    EXPECT_FALSE(luabridge::isInstance<std::optional<int32_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<uint32_t>>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::optional<int64_t>>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<std::optional<uint64_t>>(L, -1));

    {
        auto result = luabridge::get<uint64_t>(L, -1);
        EXPECT_EQ(value, result);
    }
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

    {
        auto result = luabridge::get<std::string>(L, -1);
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
        auto result = luabridge::get<std::string_view>(L, -1);
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
