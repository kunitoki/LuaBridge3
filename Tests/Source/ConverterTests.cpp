// https://github.com/kunitoki/LuaBridge3
// Copyright 2024, kunitoki
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include <string>

struct ConverterTests : TestBase
{
};

namespace {

//=================================================================================================
// Source types — registered with LuaBridge; addConverter points FROM these
struct Vec3Source
{
    float x = 0.f, y = 0.f, z = 0.f;
    Vec3Source() = default;
    Vec3Source(float x, float y, float z) : x(x), y(y), z(z) {}
};

struct ColorSource
{
    float r = 0.f, g = 0.f, b = 0.f;
    ColorSource() = default;
    ColorSource(float r, float g, float b) : r(r), g(g), b(b) {}
};

// Source with inheritance — used to test that addConverter on base propagates correctly
struct BaseSource
{
    int base_val = 0;
    BaseSource() = default;
    explicit BaseSource(int v) : base_val(v) {}
};

struct DerivedSource : BaseSource
{
    int derived_val = 0;
    DerivedSource() = default;
    DerivedSource(int b, int d) : BaseSource(b), derived_val(d) {}
};

// Multiple-inheritance source — tests pointer adjustment in converter
struct OffsetBase
{
    int padding = 999; // ensures DerivedMulti has non-zero offset to BaseSource
};

struct DerivedMulti : OffsetBase, BaseSource
{
    DerivedMulti() = default;
    DerivedMulti(int b) : BaseSource(b) {}
};

//=================================================================================================
// Target types — StackConversion<T>::enabled = true; converters produce these
struct Vec3Target
{
    float x = 0.f, y = 0.f, z = 0.f;
    Vec3Target() = default;
    Vec3Target(float x, float y, float z) : x(x), y(y), z(z) {}
    bool operator==(const Vec3Target& o) const { return x == o.x && y == o.y && z == o.z; }
};

struct MultiTarget
{
    int value = 0;
    MultiTarget() = default;
    explicit MultiTarget(int v) : value(v) {}
    bool operator==(const MultiTarget& o) const { return value == o.value; }
};

} // namespace

// Opt-in traits
namespace luabridge {
template <> struct StackConversion<Vec3Target> { static constexpr bool enabled = true; };
template <> struct StackConversion<MultiTarget> { static constexpr bool enabled = true; };
} // namespace luabridge

// Converters
namespace luabridge {
template <>
struct StackConverter<Vec3Target, Vec3Source>
{
    static Vec3Target convert(const Vec3Source& s) { return {s.x, s.y, s.z}; }
};

template <>
struct StackConverter<MultiTarget, Vec3Source>
{
    static MultiTarget convert(const Vec3Source& s) { return MultiTarget{static_cast<int>(s.x)}; }
};

template <>
struct StackConverter<MultiTarget, ColorSource>
{
    static MultiTarget convert(const ColorSource& s) { return MultiTarget{static_cast<int>(s.r * 255)}; }
};

template <>
struct StackConverter<MultiTarget, BaseSource>
{
    static MultiTarget convert(const BaseSource& s) { return MultiTarget{s.base_val}; }
};

template <>
struct StackConverter<MultiTarget, DerivedSource>
{
    static MultiTarget convert(const DerivedSource& s) { return MultiTarget{s.base_val}; }
};

template <>
struct StackConverter<MultiTarget, DerivedMulti>
{
    // DerivedMulti : OffsetBase, BaseSource — base_val is at non-zero offset
    static MultiTarget convert(const DerivedMulti& s) { return MultiTarget{s.base_val}; }
};
} // namespace luabridge

namespace {

//=================================================================================================
// Free functions that accept the target types
static float sumVec3(Vec3Target v) { return v.x + v.y + v.z; }
static float sumVec3Ref(const Vec3Target& v) { return v.x + v.y + v.z; }
static const Vec3Target* capturedVec3Ref = nullptr;
static float captureVec3Ref(const Vec3Target& v)
{
    capturedVec3Ref = &v;
    return sumVec3Ref(v);
}
static int getMultiValue(MultiTarget t) { return t.value; }

void registerAll(lua_State* L)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Vec3Source>("Vec3Source")
            .addConstructor<void(float, float, float)>()
            .addConverter<Vec3Target>()
            .addConverter<MultiTarget>()
        .endClass()
        .beginClass<ColorSource>("ColorSource")
            .addConstructor<void(float, float, float)>()
            .addConverter<MultiTarget>()
        .endClass()
        .beginClass<BaseSource>("BaseSource")
            .addConstructor<void(int)>()
            .addConverter<MultiTarget>()
        .endClass()
        .deriveClass<DerivedSource, BaseSource>("DerivedSource")
            .addConstructor<void(int, int)>()
            .addConverter<MultiTarget>()
        .endClass()
        .deriveClass<DerivedMulti, BaseSource>("DerivedMulti")
            .addConstructor<void(int)>()
            .addConverter<MultiTarget>()
        .endClass()
        .beginClass<Vec3Target>("Vec3Target")
            .addConstructor<void(float, float, float)>()
        .endClass()
        .beginClass<MultiTarget>("MultiTarget")
        .endClass()
        .addFunction("sumVec3", sumVec3)
        .addFunction("sumVec3Ref", sumVec3Ref)
        .addFunction("captureVec3Ref", captureVec3Ref)
        .addFunction("getMultiValue", getMultiValue);
}

} // namespace

//=================================================================================================
// Diagnostic: verify the converters sub-table is actually stored in cl

TEST_F(ConverterTests, Diagnostic_MetatableHasConverters)
{
    registerAll(L);

    // Check that Vec3Source's class table has a ConverterRegistry userdata
    luabridge::lua_rawgetp_x(L, LUA_REGISTRYINDEX, luabridge::detail::getClassRegistryKey<Vec3Source>());
    ASSERT_TRUE(lua_istable(L, -1)) << "Vec3Source cl not found in registry";

    luabridge::lua_rawgetp_x(L, -1, luabridge::detail::getConvertersKey());
    EXPECT_TRUE(lua_isuserdata(L, -1) && !lua_islightuserdata(L, -1))
        << "Expected ConverterRegistry full userdata in Vec3Source cl (type=" << lua_type(L, -1) << ")";

    if (lua_isuserdata(L, -1) && !lua_islightuserdata(L, -1)) {
        auto* reg = luabridge::align<luabridge::detail::ConverterRegistry>(lua_touserdata(L, -1));
        EXPECT_NE(reg->converters.count(luabridge::detail::getClassRegistryKey<Vec3Target>()), 0u)
            << "No Vec3Target converter in ConverterRegistry";
    }
    lua_pop(L, 2); // registry userdata + cl
}

TEST_F(ConverterTests, Diagnostic_DirectStackGet)
{
    registerAll(L);

    // Push a Vec3Source into Lua, then get it as a Vec3Target via Phase 3
    runLua("_src = Vec3Source(1, 2, 3)");
    lua_getglobal(L, "_src");
    ASSERT_TRUE(lua_isuserdata(L, -1)) << "Expected Vec3Source userdata, got type=" << lua_type(L, -1);

    // Verify its metatable has the ConverterRegistry userdata
    lua_getmetatable(L, -1);
    ASSERT_TRUE(lua_istable(L, -1)) << "No metatable on Vec3Source";
    luabridge::lua_rawgetp_x(L, -1, luabridge::detail::getConvertersKey());
    EXPECT_TRUE(lua_isuserdata(L, -1) && !lua_islightuserdata(L, -1))
        << "Userdata metatable has no ConverterRegistry (type=" << lua_type(L, -1) << ")";
    lua_pop(L, 2); // registry userdata + metatable

    // Try Stack<Vec3Target>::get directly on the userdata
    auto res = luabridge::Stack<Vec3Target>::get(L, -1);
    EXPECT_TRUE(static_cast<bool>(res)) << "Stack<Vec3Target>::get failed";
    if (res) {
        EXPECT_FLOAT_EQ((*res).x + (*res).y + (*res).z, 6.0f);
    }

    lua_pop(L, 1); // pop _src
}

//=================================================================================================
// Phase 1+2 still work (exact match is unaffected by converter registration)

TEST_F(ConverterTests, ExactTypePassThrough)
{
    registerAll(L);
    runLua("result = sumVec3(Vec3Target(1, 2, 3))");
    EXPECT_FLOAT_EQ(result<float>(), 6.f);
}

TEST_F(ConverterTests, ExactTypeConstRefUsesUserdataReference)
{
    registerAll(L);
    capturedVec3Ref = nullptr;

    runLua("_target = Vec3Target(1, 2, 3)");
    lua_getglobal(L, "_target");
    auto target = luabridge::detail::Userdata::get<Vec3Target>(L, -1, true);
    ASSERT_TRUE(target);
    ASSERT_NE(*target, nullptr);
    Vec3Target* targetPtr = *target;
    lua_pop(L, 1);

    runLua("result = captureVec3Ref(_target)");
    EXPECT_FLOAT_EQ(result<float>(), 6.f);
    EXPECT_EQ(capturedVec3Ref, targetPtr);
}

//=================================================================================================
// Phase 3: value conversion (function taking T by value)

TEST_F(ConverterTests, ValueConversionBasic)
{
    registerAll(L);
    runLua("result = sumVec3(Vec3Source(1, 2, 3))");
    EXPECT_FLOAT_EQ(result<float>(), 6.f);
}

TEST_F(ConverterTests, ValueConversionCorrectFields)
{
    registerAll(L);
    runLua("result = sumVec3(Vec3Source(10, 20, 30))");
    EXPECT_FLOAT_EQ(result<float>(), 60.f);
}

//=================================================================================================
// Phase 3: const-ref conversion (function taking const T&)

TEST_F(ConverterTests, ConstRefConversion)
{
    registerAll(L);
    runLua("result = sumVec3Ref(Vec3Source(4, 5, 6))");
    EXPECT_FLOAT_EQ(result<float>(), 15.f);
}

//=================================================================================================
// Multiple converters to the same target type (from different sources)

TEST_F(ConverterTests, MultipleSourcesToSameTarget_Vec3Source)
{
    registerAll(L);
    runLua("result = getMultiValue(Vec3Source(7, 0, 0))");
    EXPECT_EQ(result<int>(), 7);
}

TEST_F(ConverterTests, MultipleSourcesToSameTarget_ColorSource)
{
    registerAll(L);
    // r=1.0 → int(1.0 * 255) = 255
    runLua("result = getMultiValue(ColorSource(1, 0, 0))");
    EXPECT_EQ(result<int>(), 255);
}

//=================================================================================================
// Derived class with its own converter; the converter internally uses Userdata::get<BaseSource>
// which triggers Phase 2 (inheritance) for pointer adjustment.

TEST_F(ConverterTests, DerivedSourceWithRegisteredConverter)
{
    registerAll(L);
    // DerivedSource(base=42, derived=99) — converter reads BaseSource::base_val = 42
    runLua("result = getMultiValue(DerivedSource(42, 99))");
    EXPECT_EQ(result<int>(), 42);
}

//=================================================================================================
// Multiple inheritance: DerivedMulti inherits from both OffsetBase and BaseSource.
// The BaseSource sub-object is at a non-zero offset; the cast table must adjust the pointer.

TEST_F(ConverterTests, MultipleInheritancePointerAdjustment)
{
    registerAll(L);
    // DerivedMulti(base=77) — converter reads BaseSource::base_val = 77 after offset adjustment
    runLua("result = getMultiValue(DerivedMulti(77))");
    EXPECT_EQ(result<int>(), 77);
}

//=================================================================================================
// Wrong type should fail (not a userdata that has any registered converter to Vec3Target)

TEST_F(ConverterTests, WrongTypeFails)
{
    registerAll(L);
    // ColorSource has no converter to Vec3Target — should produce an error
    auto [ok, _] = runLuaCaptureError("result = sumVec3(ColorSource(1, 2, 3))");
    EXPECT_FALSE(ok);
}

//=================================================================================================
// Nil should fail for value target

TEST_F(ConverterTests, NilFails)
{
    registerAll(L);
    auto [ok, _] = runLuaCaptureError("result = sumVec3(nil)");
    EXPECT_FALSE(ok);
}

//=================================================================================================
// Registering same converter twice is idempotent (second registration overwrites first)

TEST_F(ConverterTests, DoubleRegistration)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Vec3Source>("Vec3Source")
            .addConstructor<void(float, float, float)>()
            .addConverter<Vec3Target>()
            .addConverter<Vec3Target>() // second call — should overwrite, not duplicate
        .endClass()
        .beginClass<Vec3Target>("Vec3Target")
            .addConstructor<void(float, float, float)>()
        .endClass()
        .addFunction("sumVec3", sumVec3);

    runLua("result = sumVec3(Vec3Source(1, 2, 3))");
    EXPECT_FLOAT_EQ(result<float>(), 6.f);
}

//=================================================================================================
// Const userdata object should also work (converter stored in const table)

TEST_F(ConverterTests, ConstUserdataConverter)
{
    // Expose a function that returns a const reference to a Vec3Source stored in C++
    static Vec3Source constSrc{3.f, 4.f, 5.f};

    luabridge::getGlobalNamespace(L)
        .beginClass<Vec3Source>("Vec3Source")
            .addConverter<Vec3Target>()
        .endClass()
        .beginClass<Vec3Target>("Vec3Target")
            .addConstructor<void(float, float, float)>()
        .endClass()
        .addFunction("getConstSrc", []() -> const Vec3Source& { return constSrc; })
        .addFunction("sumVec3", sumVec3);

    runLua("result = sumVec3(getConstSrc())");
    EXPECT_FLOAT_EQ(result<float>(), 12.f);
}
