// https://github.com/kunitoki/LuaBridge3
// Copyright 2022, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#include "SharedCode.h"

namespace xyz {

int SharedClass::publicMethod(const std::string& s) const
{
    return std::stoi(s) + value;
}

} // namespace xyz
