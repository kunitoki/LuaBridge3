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
} // namespace

struct ExceptionTests : TestBase
{
};

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
