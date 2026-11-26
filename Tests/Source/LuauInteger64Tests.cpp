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

constexpr long long kAboveDoublePrecision = 9007199254740993ll; // 2^53 + 1

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

    EXPECT_TRUE(luabridge::isInstance<long long>(L, -1));
    EXPECT_TRUE(luabridge::isInstance<unsigned long long>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<int32_t>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<double>(L, -1));
    EXPECT_FALSE(luabridge::isInstance<std::string>(L, -1));

    EXPECT_EQ(kAboveDoublePrecision, *luabridge::get<long long>(L, -1));
    EXPECT_EQ(static_cast<unsigned long long>(kAboveDoublePrecision), *luabridge::get<unsigned long long>(L, -1));
}

TEST_F(LuauInteger64Tests, GetNativeIntegerLimits)
{
    runLua("result = integer.minsigned");
    EXPECT_EQ(std::numeric_limits<long long>::min(), result<long long>());

    runLua("result = integer.maxsigned");
    EXPECT_EQ(std::numeric_limits<long long>::max(), result<long long>());

    runLua("result = -1i");
    EXPECT_EQ(-1, result<long long>());
    EXPECT_EQ(std::numeric_limits<unsigned long long>::max(), result<unsigned long long>());
}

#if LUABRIDGE_STRICT_STACK_CONVERSIONS

TEST_F(LuauInteger64Tests, StrictRejectsNumbers)
{
    runLua("result = 42");
    EXPECT_FALSE(result().isInstance<long long>());
    EXPECT_FALSE(result().isInstance<unsigned long long>());

    auto value = result().cast<long long>();
    ASSERT_FALSE(value);
    EXPECT_EQ(luabridge::ErrorCode::InvalidTypeCast, value.error());

    runLua("result = 42i");
    EXPECT_EQ(42, result<long long>());
}

#else

TEST_F(LuauInteger64Tests, GetNumbers)
{
    runLua("result = 42");
    EXPECT_TRUE(result().isInstance<long long>());
    EXPECT_EQ(42, result<long long>());
    EXPECT_EQ(42u, result<unsigned long long>());

    runLua("result = -42");
    EXPECT_EQ(-42, result<long long>());
    EXPECT_FALSE(result().isInstance<unsigned long long>());
    EXPECT_FALSE(result().cast<unsigned long long>());

    runLua("result = 1.5");
    EXPECT_FALSE(result().isInstance<long long>());
    EXPECT_FALSE(result().cast<long long>());

    runLua("result = 2^63");
    EXPECT_FALSE(result().isInstance<long long>());
    EXPECT_TRUE(result().isInstance<unsigned long long>());
    EXPECT_EQ(9223372036854775808ull, result<unsigned long long>());

    runLua("result = 2^64");
    EXPECT_FALSE(result().isInstance<unsigned long long>());

    runLua("result = math.huge");
    EXPECT_FALSE(result().isInstance<long long>());

    runLua("result = 0/0");
    EXPECT_FALSE(result().isInstance<long long>());

    runLua("result = '42'");
    EXPECT_FALSE(result().isInstance<long long>());
    EXPECT_FALSE(result().cast<long long>());
}

#endif // LUABRIDGE_STRICT_STACK_CONVERSIONS

TEST_F(LuauInteger64Tests, BoundFunctionsAcceptNativeIntegers)
{
    luabridge::getGlobalNamespace(L)
        .addFunction("echoSigned", [](long long value) { return value; })
        .addFunction("echoUnsigned", [](unsigned long long value) { return value; })
        .addFunction("isMax", [](unsigned long long value) { return value == std::numeric_limits<unsigned long long>::max(); });

    runLua("result = isMax(-1i)");
    EXPECT_TRUE(result<bool>());

    runLua("result = echoUnsigned(42i)");
    EXPECT_EQ(42u, result<unsigned long long>());

    runLua("result = echoSigned(9007199254740993i)");
    EXPECT_EQ(kAboveDoublePrecision, result<long long>());
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

    ASSERT_TRUE(luabridge::push(L, std::numeric_limits<unsigned long long>::max()));
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
        .addFunction("signedId", [] { return std::numeric_limits<long long>::max(); })
        .addFunction("unsignedId", [] { return std::numeric_limits<unsigned long long>::max(); })
        .addFunction("sameSigned", [](long long value) { return value == std::numeric_limits<long long>::max(); })
        .addFunction("sameUnsigned", [](unsigned long long value) { return value == std::numeric_limits<unsigned long long>::max(); });

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
        .addFunction("id", [] { return 5ll; });

    runLua("result = id() == 5");
    EXPECT_FALSE(result<bool>());

    runLua("result = id() == 5i");
    EXPECT_TRUE(result<bool>());
}

TEST_F(LuauInteger64Tests, PushNumbersWhenIntegerLibraryFlagIsDisabled)
{
    ScopedLuauFlags disabled({ &FFlag::LuauIntegerLibrary }, false);

    ASSERT_TRUE(luabridge::push(L, 42ll));
    EXPECT_EQ(LUA_TNUMBER, lua_type(L, -1));
    EXPECT_TRUE(luabridge::isInstance<long long>(L, -1));
    EXPECT_EQ(42, *luabridge::get<long long>(L, -1));
    lua_pop(L, 1);

    ASSERT_TRUE(luabridge::push(L, 42ull));
    EXPECT_EQ(LUA_TNUMBER, lua_type(L, -1));
    EXPECT_EQ(42u, *luabridge::get<unsigned long long>(L, -1));
    lua_pop(L, 1);
}

TEST_F(LuauInteger64Tests, PlatformIntegersAcceptNumbersAndNativeIntegers)
{
    luabridge::getGlobalNamespace(L)
        .addFunction("takeSizeT", [](std::size_t value) { return value == 7; })
        .addFunction("takeLong", [](long value) { return value == -7; })
        .addFunction("takeULong", [](unsigned long value) { return value == 7; });

    for (const char* code : { "result = takeSizeT(7) and takeSizeT(7i)",
                              "result = takeLong(-7) and takeLong(-7i)",
                              "result = takeULong(7) and takeULong(7i)" })
    {
        runLua(code);
        EXPECT_TRUE(result<bool>()) << code;
    }

    runLua("result = 1.5");
    EXPECT_FALSE(result().isInstance<long>());

    runLua("result = -1");
    EXPECT_FALSE(result().isInstance<unsigned long>());

    runLua("result = '7'");
    EXPECT_FALSE(result().isInstance<long>());
}

TEST_F(LuauInteger64Tests, PlatformIntegersArePushedAsNumbersWhenTheyFit)
{
    ASSERT_TRUE(luabridge::push(L, 42l));
    EXPECT_EQ(LUA_TNUMBER, lua_type(L, -1));
    lua_pop(L, 1);

    ASSERT_TRUE(luabridge::push(L, std::size_t(42)));
    EXPECT_EQ(LUA_TNUMBER, lua_type(L, -1));
    lua_pop(L, 1);

    if constexpr (sizeof(long) == sizeof(long long))
    {
        ASSERT_TRUE(luabridge::push(L, static_cast<long>(kAboveDoublePrecision)));
        EXPECT_EQ(LUA_TINTEGER, lua_type(L, -1));
        EXPECT_EQ(kAboveDoublePrecision, *luabridge::get<long>(L, -1));
        lua_pop(L, 1);

        ASSERT_TRUE(luabridge::push(L, (std::numeric_limits<unsigned long>::max)()));
        EXPECT_EQ(LUA_TINTEGER, lua_type(L, -1));
        EXPECT_EQ((std::numeric_limits<unsigned long>::max)(), *luabridge::get<unsigned long>(L, -1));
        lua_pop(L, 1);
    }
    else
    {
        runLua("result = 9007199254740993i");
        EXPECT_FALSE(result().isInstance<long>());
        EXPECT_FALSE(result().isInstance<unsigned long>());
    }
}

#endif // LUABRIDGE_LUAU_NATIVE_INTEGER64
