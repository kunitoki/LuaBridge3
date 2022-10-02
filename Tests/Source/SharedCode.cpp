// https://github.com/kunitoki/LuaBridge3
// Copyright 2022, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#include "SharedCode.h"
#include "LuaBridge/LuaBridge.h"

namespace xyz {
ISharedClass::ISharedClass() = default;
ISharedClass::~ISharedClass() = default;

class SharedClass : public ISharedClass
{
public:
    virtual int publicMethod(const std::string& s) const override
    {
        return std::stoi(s) + value;
    }

private:
    int value = 42;
};

class LUABRIDGEDEMO_SHARED_API AnotherClass
{
public:
    AnotherClass() = default;
    ~AnotherClass() = default;

    int publicMethod(const std::string& s)
    {
        return value = std::stoi(s);
    }

    int publicConstMethod(const std::string& s) const
    {
        return value + std::stoi(s);
    }

    int value = 30;
};
} // namespace xyz

extern "C" {
LUABRIDGEDEMO_SHARED_API xyz::ISharedClass* allocator()
{
    return new xyz::SharedClass();
}

LUABRIDGEDEMO_SHARED_API void deallocator(xyz::ISharedClass* ptr)
{
    delete ptr;
}

LUABRIDGEDEMO_SHARED_API void registerAnotherClass(lua_State* L)
{
    luabridge::getGlobalNamespace(L)
        .beginNamespace("dll")
            .beginClass<xyz::AnotherClass>("AnotherClass")
                .addConstructor<void (*)()>()
                .addFunction("publicMethod", &xyz::AnotherClass::publicMethod)
                .addFunction("publicConstMethod", &xyz::AnotherClass::publicConstMethod)
                .addProperty("value", &xyz::AnotherClass::value)
            .endClass()
        .endNamespace();
}

} // extern "C"
