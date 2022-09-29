// https://github.com/kunitoki/LuaBridge3
// Copyright 2022, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace xyz {

class SharedClass
{
public:
    SharedClass() = default;

    int publicMethod(const std::string& s) const;

private:
    int value = 42;
};

} // namespace xyz
