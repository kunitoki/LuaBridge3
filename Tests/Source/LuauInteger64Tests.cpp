// https://github.com/kunitoki/LuaBridge3
// Copyright 2026, kunitoki
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#if LUABRIDGE_LUAU_NATIVE_INTEGER64

#include <cstdint>
#include <limits>
#include <sstream>
#include <vector>

LUAU_FASTFLAG(LuauIntegerLibrary)
LUAU_FASTFLAG(LuauIntegerType2)

namespace {

// Flags are referenced directly instead of walking Luau::FValue<bool>::list: when a shared library embedding
// its own Luau is loaded and unloaded (DynamicLibraryTests), the coalesced list head can dangle into unmapped memory.
class ScopedLuauFlags
{
public:
    ScopedLuauFlags(std::initializer_list<Luau::FValue<bool>*> flags, bool value = true)
    {
        for (Luau::FValue<bool>* flag : flags)
        {
            saved.emplace_back(flag, flag->value);
            flag->value = value;
        }
    }

    ~ScopedLuauFlags()
    {
        for (auto& [flag, value] : saved)
            flag->value = value;
    }

private:
    std::vector<std::pair<Luau::FValue<bool>*, bool>> saved;
};

constexpr int64_t kAboveDoublePrecision = 9007199254740993ll; // 2^53 + 1

} // namespace

struct LuauInteger64Tests : TestBase
{
    // Must be enabled before opening the libraries (integer library) and compiling (1i literals)
    ScopedLuauFlags flags{ &FFlag::LuauIntegerLibrary, &FFlag::LuauIntegerType2 };
};

TEST_F(LuauInteger64Tests, GetNativeIntegers)
{
    runLua("result = 9007199254740993i");
    lua_getglobal(L, "result");
    ASSERT_EQ(LUA_TINTEGER, lua_type(L, -1));

    EXPECT_TRUE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<uint64_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));

    EXPECT_EQ(kAboveDoublePrecision, *luabridge::get<int64_t>(L, -1));
    EXPECT_EQ(static_cast<uint64_t>(kAboveDoublePrecision), *luabridge::get<uint64_t>(L, -1));
    EXPECT_EQ(kAboveDoublePrecision, *luabridge::get<long long>(L, -1));
    if constexpr (sizeof(long) == sizeof(int64_t))
        EXPECT_EQ(kAboveDoublePrecision, *luabridge::get<long>(L, -1));
}

TEST_F(LuauInteger64Tests, GetNativeIntegerLimits)
{
    runLua("result = integer.minsigned");
    EXPECT_EQ(std::numeric_limits<int64_t>::min(), result<int64_t>());

    runLua("result = integer.maxsigned");
    EXPECT_EQ(std::numeric_limits<int64_t>::max(), result<int64_t>());

    runLua("result = -1i");
    EXPECT_EQ(-1, result<int64_t>());
    EXPECT_EQ(std::numeric_limits<uint64_t>::max(), result<uint64_t>());
}

#if LUABRIDGE_STRICT_STACK_CONVERSIONS

TEST_F(LuauInteger64Tests, StrictRejectsNumbers)
{
    runLua("result = 42");
    EXPECT_FALSE(result().isInstance<int64_t>());
    EXPECT_FALSE(result().isInstance<uint64_t>());

    auto value = result().cast<int64_t>();
    ASSERT_FALSE(value);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, value.error());

    runLua("result = 42i");
    EXPECT_EQ(42, result<int64_t>());
}

#else

TEST_F(LuauInteger64Tests, GetNumbers)
{
    runLua("result = 42");
    EXPECT_TRUE(result().isInstance<int64_t>());
    EXPECT_EQ(42, result<int64_t>());
    EXPECT_EQ(42u, result<uint64_t>());

    runLua("result = -42");
    EXPECT_EQ(-42, result<int64_t>());
    EXPECT_FALSE(result().isInstance<uint64_t>());
    EXPECT_FALSE(result().cast<uint64_t>());

    runLua("result = 1.5");
    EXPECT_FALSE(result().isInstance<int64_t>());
    EXPECT_FALSE(result().cast<int64_t>());

    runLua("result = 2^63");
    EXPECT_FALSE(result().isInstance<int64_t>());
    EXPECT_TRUE(result().isInstance<uint64_t>());
    EXPECT_EQ(9223372036854775808ull, result<uint64_t>());

    runLua("result = 2^64");
    EXPECT_FALSE(result().isInstance<uint64_t>());

    runLua("result = math.huge");
    EXPECT_FALSE(result().isInstance<int64_t>());

    runLua("result = 0/0");
    EXPECT_FALSE(result().isInstance<int64_t>());

    runLua("result = '42'");
    EXPECT_FALSE(result().isInstance<int64_t>());
    EXPECT_FALSE(result().cast<int64_t>());
}

#endif // LUABRIDGE_STRICT_STACK_CONVERSIONS

TEST_F(LuauInteger64Tests, BoundFunctionsAcceptNativeIntegers)
{
    luabridge::getGlobalNamespace(L)
        .addFunction("echoSigned", [](int64_t value) { return value; })
        .addFunction("echoUnsigned", [](uint64_t value) { return value; })
        .addFunction("isMax", [](uint64_t value) { return value == std::numeric_limits<uint64_t>::max(); });

    runLua("result = isMax(-1i)");
    EXPECT_TRUE(result<bool>());

    runLua("result = echoUnsigned(42i)");
    EXPECT_EQ(42u, result<uint64_t>());

    runLua("result = echoSigned(9007199254740993i)");
    EXPECT_EQ(kAboveDoublePrecision, result<int64_t>());
}

TEST_F(LuauInteger64Tests, LuaRefPrintAndHash)
{
    runLua("result = 9007199254740993i");

    std::ostringstream stream;
    stream << result();
    EXPECT_EQ("9007199254740993i", stream.str());

    runLua("other = 9007199254740993i");
    auto other = luabridge::getGlobal(L, "other");
    EXPECT_EQ(result(), other);
    EXPECT_EQ(std::hash<luabridge::LuaRef>{}(result()), std::hash<luabridge::LuaRef>{}(other));
}

TEST_F(LuauInteger64Tests, DumpValue)
{
    runLua("result = -5i");
    lua_getglobal(L, "result");

    std::ostringstream stream;
    luabridge::dumpValue(L, -1, 1, 0, false, stream);
    EXPECT_EQ("-5i", stream.str());
}

TEST_F(LuauInteger64Tests, PushNativeIntegers)
{
    ASSERT_TRUE(luabridge::push(L, kAboveDoublePrecision));
    EXPECT_EQ(LUA_TINTEGER, lua_type(L, -1));
    EXPECT_EQ(kAboveDoublePrecision, lua_tointeger64(L, -1, nullptr));
    lua_pop(L, 1);

    ASSERT_TRUE(luabridge::push(L, std::numeric_limits<uint64_t>::max()));
    EXPECT_EQ(LUA_TINTEGER, lua_type(L, -1));
    EXPECT_EQ(-1, lua_tointeger64(L, -1, nullptr));
    lua_pop(L, 1);

    ASSERT_TRUE(luabridge::push(L, 42ll));
    EXPECT_EQ(LUA_TINTEGER, lua_type(L, -1));
    lua_pop(L, 1);
}

TEST_F(LuauInteger64Tests, RoundTripThroughScripts)
{
    luabridge::getGlobalNamespace(L)
        .addFunction("signedId", [] { return std::numeric_limits<int64_t>::max(); })
        .addFunction("unsignedId", [] { return std::numeric_limits<uint64_t>::max(); })
        .addFunction("sameSigned", [](int64_t value) { return value == std::numeric_limits<int64_t>::max(); })
        .addFunction("sameUnsigned", [](uint64_t value) { return value == std::numeric_limits<uint64_t>::max(); });

    runLua(R"(
        local s, u = signedId(), unsignedId()
        local keys = { [s] = "signed" }
        result = typeof(s) == "integer"
            and tostring(s) == "9223372036854775807"
            and s == integer.maxsigned
            and u == -1i
            and keys[signedId()] == "signed"
            and sameSigned(s)
            and sameUnsigned(u)
    )");
    EXPECT_TRUE(result<bool>());
}

TEST_F(LuauInteger64Tests, NativeIntegersAreNotNumbers)
{
    luabridge::getGlobalNamespace(L)
        .addFunction("id", [] { return int64_t(5); });

    runLua("result = id() == 5");
    EXPECT_FALSE(result<bool>());

    runLua("result = id() == 5i");
    EXPECT_TRUE(result<bool>());
}

TEST_F(LuauInteger64Tests, PushNumbersWhenIntegerLibraryFlagIsDisabled)
{
    ScopedLuauFlags disabled{ { &FFlag::LuauIntegerLibrary }, false };

    ASSERT_TRUE(luabridge::push(L, int64_t(42)));
    EXPECT_EQ(LUA_TNUMBER, lua_type(L, -1));
    EXPECT_TRUE(luabridge::isInstance<int64_t>(L, -1));
    EXPECT_EQ(42, *luabridge::get<int64_t>(L, -1));
    lua_pop(L, 1);

    ASSERT_TRUE(luabridge::push(L, uint64_t(42)));
    EXPECT_EQ(LUA_TNUMBER, lua_type(L, -1));
    EXPECT_EQ(42u, *luabridge::get<uint64_t>(L, -1));
    lua_pop(L, 1);
}

#endif // LUABRIDGE_LUAU_NATIVE_INTEGER64
