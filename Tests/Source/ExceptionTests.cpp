// https://github.com/kunitoki/LuaBridge3
// Copyright 2022, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include <string>

#if LUABRIDGE_HAS_EXCEPTIONS
namespace {
static void throwError()
{
    throw std::runtime_error("runtime error");
}

static int standaloneError()
{
    throwError();
    return 0;
}

class ErrorClass
{
public:
    int classError() const
    {
        throwError();
        return 0;
    }
};

struct ThrowingClass : public std::enable_shared_from_this<ThrowingClass>
{
    ThrowingClass()
    {
        throw std::runtime_error("Throwing during construction");
        
        x = 100;
    }

    explicit ThrowingClass(int y)
    {
        x = y;
    }
    
    void throwingMethod()
    {
        throw std::runtime_error("Throwing during method invocation");
    }

    void throwingMethodConst() const
    {
        throw std::runtime_error("Throwing during const method invocation");
    }

    int throwingCMethod(lua_State* L)
    {
        throw std::runtime_error("Throwing during cfunction method invocation");
        return 0;
    }

    int throwingCMethodConst(lua_State* L) const
    {
        throw std::runtime_error("Throwing during cfunction const method invocation");
        return 0;
    }
    
    int x = 0;
};
} // namespace

struct ExceptionTests : TestBase
{
};

TEST_F(ExceptionTests, ThrowingPrimaryConstructors)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<ThrowingClass>("ThrowingClass")
        .addConstructor<void(*)()>()
        .endClass();

    EXPECT_ANY_THROW(runLua("result = ThrowingClass()"));
}

TEST_F(ExceptionTests, ThrowingPlacementConstructors)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<ThrowingClass>("ThrowingClass")
        .addConstructor([](void* ptr)
        {
            throw std::runtime_error("Throwing during construction");
            return new(ptr) ThrowingClass();
        })
        .endClass();
    
    EXPECT_ANY_THROW(runLua("result = ThrowingClass()"));
    
    luabridge::getGlobalNamespace(L)
        .beginClass<ThrowingClass>("ThrowingClass2")
        .addConstructor([](void* ptr)
        {
            return new(ptr) ThrowingClass();
        })
        .endClass();
    
    EXPECT_ANY_THROW(runLua("result = ThrowingClass2()"));
}

TEST_F(ExceptionTests, ThrowingContainerPrimaryConstructors)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<ThrowingClass>("ThrowingClass")
        .addConstructorFrom<std::shared_ptr<ThrowingClass>, void(*)()>()
        .endClass();

    EXPECT_ANY_THROW(runLua("result = ThrowingClass()"));
}

TEST_F(ExceptionTests, ThrowingContainerPlacementConstructors)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<ThrowingClass>("ThrowingClass")
        .addConstructorFrom<std::shared_ptr<ThrowingClass>>([]
        {
            throw std::runtime_error("Throwing during construction");
            return std::make_shared<ThrowingClass>();
        })
        .endClass();
    
    EXPECT_ANY_THROW(runLua("result = ThrowingClass()"));
    
    luabridge::getGlobalNamespace(L)
        .beginClass<ThrowingClass>("ThrowingClass2")
        .addConstructorFrom<std::shared_ptr<ThrowingClass>>([]
        {
            return std::make_shared<ThrowingClass>();
        })
        .endClass();

    EXPECT_ANY_THROW(runLua("result = ThrowingClass2()"));
}

TEST_F(ExceptionTests, ThrowingFactoryConstructors)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<ThrowingClass>("ThrowingClass")
        .addFactory(
            +[]() -> ThrowingClass* {
                throw std::runtime_error("Throwing during construction");
                return new ThrowingClass(); },
            +[](ThrowingClass* x) { delete x; })                    
        .endClass();
    
    EXPECT_ANY_THROW(runLua("result = ThrowingClass()"));
    
    luabridge::getGlobalNamespace(L)
        .beginClass<ThrowingClass>("ThrowingClass2")
        .addFactory(
            +[]() -> ThrowingClass* { return new ThrowingClass(); },
            +[](ThrowingClass* x) { delete x; })                    
        .endClass();

    EXPECT_ANY_THROW(runLua("result = ThrowingClass2()"));
}

TEST_F(ExceptionTests, ThrowingMethodInvocation)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<ThrowingClass>("ThrowingClass")
        .addConstructor<void(*)(int)>()
        .addFunction("throwingMethod", &ThrowingClass::throwingMethod)
        .addFunction("throwingMethodConst", &ThrowingClass::throwingMethodConst)
        .addFunction("throwingCMethod", &ThrowingClass::throwingCMethod)
        .addFunction("throwingCMethodConst", &ThrowingClass::throwingCMethodConst)
        .addFunction("throwingMethodLambda", [](ThrowingClass& self) { self.throwingMethod(); })
        .addFunction("throwingMethodLambdaConst", [](const ThrowingClass& self) { self.throwingMethodConst(); })
        .addFunction("throwingMethodLambdaRaise", [](ThrowingClass& self) {
            throw std::runtime_error("Throwing during lambda"); self.throwingMethod(); })
        .addFunction("throwingMethodLambdaConstRaise", [](const ThrowingClass& self) {
            throw std::runtime_error("Throwing during const lambda"); self.throwingMethodConst(); })
        .endClass();

    EXPECT_ANY_THROW(runLua("local t = ThrowingClass(1); result = t:throwingMethod()"));
    EXPECT_ANY_THROW(runLua("local t = ThrowingClass(2); result = t:throwingMethodConst()"));
    EXPECT_ANY_THROW(runLua("local t = ThrowingClass(3); result = t:throwingCMethod()"));
    EXPECT_ANY_THROW(runLua("local t = ThrowingClass(4); result = t:throwingCMethodConst()"));
    EXPECT_ANY_THROW(runLua("local t = ThrowingClass(5); result = t:throwingMethodLambda()"));
    EXPECT_ANY_THROW(runLua("local t = ThrowingClass(6); result = t:throwingMethodLambdaConst()"));
    EXPECT_ANY_THROW(runLua("local t = ThrowingClass(7); result = t:throwingMethodLambdaRaise()"));
    EXPECT_ANY_THROW(runLua("local t = ThrowingClass(8); result = t:throwingMethodLambdaConstRaise()"));
}

namespace {
static int ThrowingFunction(lua_State* L)
{
    const int numArgs = lua_gettop(L);
    
    const char* value = numArgs < 1 ? nullptr : luabridge::Stack<const char*>::get(L, 1).value();
    luabridge::Stack<const char*>::push(L, value).throw_on_error();

    return 1;
}

static std::unique_ptr<luabridge::LuaRef> luaCallback;

static void RegisterCallback(luabridge::LuaRef callback)
{
   luaCallback.reset(new luabridge::LuaRef(callback));
}
} // namespace

TEST_F(ExceptionTests, Bug153)
{
    luabridge::getGlobalNamespace(L)
        .addFunction("ThrowingFunction", ThrowingFunction)
        .addFunction("RegisterCallback", RegisterCallback);
    
    runLua(R"(
        function Callback()
            ThrowingFunction({})
        end

        RegisterCallback(Callback)
    )");
    
    ASSERT_TRUE(luaCallback);
    ASSERT_TRUE(luaCallback->isFunction());
    
    struct ScopeExit{ ~ScopeExit() { luaCallback.reset(); } } finalize;

    try
    {
        (*luaCallback)();
    }
    catch (const std::exception& e)
    {
        std::string msg = e.what();
        EXPECT_FALSE(msg.empty());

        lua_Debug debug;
        bool isLuaRunning = luabridge::lua_getstack_x(L, 0, &debug);
        EXPECT_FALSE(isLuaRunning);
    }
}

TEST_F(ExceptionTests, PassFromLua)
{
    luabridge::getGlobalNamespace(L)
       .beginNamespace("test")
          .addFunction("standaloneError", standaloneError)
          .beginClass<ErrorClass>("ErrorClass")
             .addConstructor<void (*) (void)>()
             .addStaticFunction("staticFunctionError", &standaloneError)
             .addStaticProperty("staticPropertyError", &standaloneError)
             .addFunction("memberFunctionError", &ErrorClass::classError)
             .addProperty("memberPropertyError", &ErrorClass::classError)
             .addProperty("memberPropertyError2", [&](ErrorClass*) { throwError(); return 1; })
          .endClass()
       .endNamespace();

    // Standalone function error will report line
    {
        auto result = runLuaCaptureError(R"(


            test.standaloneError()
        )");

        EXPECT_FALSE(std::get<0>(result));
        EXPECT_TRUE(std::get<1>(result).find("...") != std::string::npos || std::get<1>(result).find("code") != std::string::npos);
        EXPECT_TRUE(std::get<1>(result).find("4") != std::string::npos);
    }

    // Class static function error will report line
    {
        auto result = runLuaCaptureError(R"(


            local x = test.ErrorClass.staticFunctionError()
        )");

        EXPECT_FALSE(std::get<0>(result));
        EXPECT_TRUE(std::get<1>(result).find("...") != std::string::npos || std::get<1>(result).find("code") != std::string::npos);
        EXPECT_TRUE(std::get<1>(result).find("4") != std::string::npos);
    }

    // Class static property error will report line
    {
        auto result = runLuaCaptureError(R"(


            local x = test.ErrorClass.staticPropertyError
        )");

        EXPECT_FALSE(std::get<0>(result));
        EXPECT_TRUE(std::get<1>(result).find("...") != std::string::npos || std::get<1>(result).find("code") != std::string::npos);
        EXPECT_TRUE(std::get<1>(result).find("4") != std::string::npos);
    }

    // Class member function error will report line
    {
        auto result = runLuaCaptureError(R"(

            local errorClass = test.ErrorClass()
            local x = errorClass:memberFunctionError()
        )");

        EXPECT_FALSE(std::get<0>(result));
        EXPECT_TRUE(std::get<1>(result).find("...") != std::string::npos || std::get<1>(result).find("code") != std::string::npos);
        EXPECT_TRUE(std::get<1>(result).find("4") != std::string::npos);
    }

    // Class property error will report line
    {
        auto result = runLuaCaptureError(R"(

            local errorClass = test.ErrorClass()
            local y = errorClass.memberPropertyError
        )");

        EXPECT_FALSE(std::get<0>(result));
        EXPECT_TRUE(std::get<1>(result).find("...") != std::string::npos || std::get<1>(result).find("code") != std::string::npos);
        EXPECT_TRUE(std::get<1>(result).find("4") != std::string::npos);
    }

    // Class property error by lambda will report line
    {
        auto result = runLuaCaptureError(R"(

            local errorClass = test.ErrorClass()
            local y = errorClass.memberPropertyError2
        )");

        EXPECT_FALSE(std::get<0>(result));
        EXPECT_TRUE(std::get<1>(result).find("...") != std::string::npos || std::get<1>(result).find("code") != std::string::npos);
        EXPECT_TRUE(std::get<1>(result).find("4") != std::string::npos);
    }
}

#endif
