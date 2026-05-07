// https://github.com/kunitoki/LuaBridge3
// Copyright 2020, kunitoki
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include <memory>
#include <string>

namespace {

struct Widget
{
    explicit Widget(int value) : value(value) {}
    int value;
    std::string name = "widget";

    int getValue() const { return value; }
    void setValue(int v) { value = v; }
    std::string getName() const { return name; }
};

} // namespace

struct UniquePtrTests : TestBase
{
    void SetUp() override
    {
        TestBase::SetUp();

        luabridge::getGlobalNamespace(L)
            .beginClass<Widget>("Widget")
            .addConstructor<void(*)(int)>()
            .addFunction("getValue", &Widget::getValue)
            .addFunction("setValue", &Widget::setValue)
            .addFunction("getName", &Widget::getName)
            .endClass();
    }
};

TEST_F(UniquePtrTests, ContainerTraitsTypeAlias)
{
    static_assert(std::is_same_v<luabridge::ContainerTraits<std::unique_ptr<Widget>>::Type, Widget>,
        "ContainerTraits<unique_ptr<T>>::Type must be T");
}

TEST_F(UniquePtrTests, ContainerTraitsGetReturnsRawPtr)
{
    auto widget = std::make_unique<Widget>(99);
    Widget* rawPtr = luabridge::ContainerTraits<std::unique_ptr<Widget>>::get(widget);
    ASSERT_NE(nullptr, rawPtr);
    EXPECT_EQ(99, rawPtr->getValue());
}

TEST_F(UniquePtrTests, ContainerTraitsGetNullReturnsNullptr)
{
    std::unique_ptr<Widget> nullPtr;
    Widget* rawPtr = luabridge::ContainerTraits<std::unique_ptr<Widget>>::get(nullPtr);
    EXPECT_EQ(nullptr, rawPtr);
}

TEST_F(UniquePtrTests, IsContainerDetected)
{
    constexpr bool isContainer = luabridge::detail::IsContainer<std::unique_ptr<Widget>>::value;
    EXPECT_TRUE(isContainer);
}


TEST_F(UniquePtrTests, RegisterAndCallFunctionWithRawPtr)
{
    auto processWidget = [](const Widget* w) -> int {
        return w ? w->getValue() * 2 : -1;
    };

    luabridge::getGlobalNamespace(L)
        .addFunction("processWidget", std::function<int(const Widget*)>(processWidget));

    auto widget = std::make_unique<Widget>(21);

    // Push the raw pointer so Lua can use the registered Widget methods.
    // The unique_ptr must outlive this Lua call.
    ASSERT_TRUE(luabridge::push(L, widget.get()));
    lua_setglobal(L, "w");

    runLua("result = processWidget(w)");
    EXPECT_EQ(42, result<int>());
}

TEST_F(UniquePtrTests, LuaCanCallMethodsOnRawPtrFromUniquePtr)
{
    auto widget = std::make_unique<Widget>(7);

    // Push the raw pointer extracted from the unique_ptr.
    // Lua gets a non-owning view; C++ retains ownership via unique_ptr.
    ASSERT_TRUE(luabridge::push(L, widget.get()));
    lua_setglobal(L, "w");

    runLua("result = w:getValue()");
    EXPECT_EQ(7, result<int>());

    runLua("w:setValue(14)");
    EXPECT_EQ(14, widget->getValue());

    runLua("result = w:getName()");
    EXPECT_EQ("widget", result<std::string>());
}

TEST_F(UniquePtrTests, CppOwnerMustOutliveLuaReference)
{
    int capturedValue = -1;

    {
        auto widget = std::make_unique<Widget>(55);

        ASSERT_TRUE(luabridge::push(L, widget.get()));
        lua_setglobal(L, "w");

        runLua("result = w:getValue()");
        capturedValue = result<int>();

        // widget is destroyed here; after this point, using 'w' from Lua would be UB.
        // This test verifies that, while the owner is alive, Lua can read the value correctly.
    }

    EXPECT_EQ(55, capturedValue);
}
