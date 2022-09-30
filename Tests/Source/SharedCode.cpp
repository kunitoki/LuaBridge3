// https://github.com/kunitoki/LuaBridge3
// Copyright 2022, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#include "SharedCode.h"

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
} // namespace xyz

extern "C" {
LUABRIDGEDEMO_SHARED_API xyz::ISharedClass* allocator()
{
    return new xyz::SharedClass();
}

LUABRIDGEDEMO_SHARED_API void deleter(xyz::ISharedClass* ptr)
{
    delete ptr;
}
} // extern "C"
