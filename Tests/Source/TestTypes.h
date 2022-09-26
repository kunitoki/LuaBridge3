// https://github.com/kunitoki/LuaBridge3
// Copyright 2022, Lucio Asnaghi
// Copyright 2019, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#pragma once

#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

using TestTypes = ::testing::Types<bool,
                                   int8_t,
                                   char,
                                   unsigned char,
                                   short,
                                   unsigned short,
                                   int,
                                   unsigned int,
                                   long,
                                   unsigned long,
                                   long long,
                                   unsigned long long,
                                   float,
                                   double,
                                   long double,
                                   const char*,
                                   std::string,
                                   std::string_view>;

template<class T>
struct TypeTraits;

template<>
struct TypeTraits<bool>
{
    static std::vector<bool> values() { return {true, false, true}; }
    static std::string list() { return "true, false, true"; }
};

template<>
struct TypeTraits<char>
{
    static std::vector<char> values() { return {'a', 'b', 'c'}; }
    static std::string list() { return "'a', 'b', 'c'"; }
};

template<>
struct TypeTraits<int8_t>
{
    static std::vector<int8_t> values() { return {1, -2, 3}; }
    static std::string list() { return "1, -2, 3"; }
};

template<>
struct TypeTraits<unsigned char>
{
    static std::vector<unsigned char> values() { return {1, 2, 3}; }
    static std::string list() { return "1, 2, 3"; }
};

template<>
struct TypeTraits<short>
{
    static std::vector<short> values() { return {1, -2, 3}; }
    static std::string list() { return "1, -2, 3"; }
};

template<>
struct TypeTraits<unsigned short>
{
    static std::vector<unsigned short> values() { return {1, 2, 3}; }
    static std::string list() { return "1, 2, 3"; }
};

template<>
struct TypeTraits<int>
{
    static std::vector<int> values() { return {1, -2, 3}; }
    static std::string list() { return "1, -2, 3"; }
};

template<>
struct TypeTraits<unsigned int>
{
    static std::vector<unsigned int> values() { return {1, 2, 3}; }
    static std::string list() { return "1, 2, 3"; }
};

template<>
struct TypeTraits<long>
{
    static std::vector<long> values() { return {1, -2, 3}; }
    static std::string list() { return "1, -2, 3"; }
};

template<>
struct TypeTraits<unsigned long>
{
    static std::vector<unsigned long> values() { return {1, 2, 3}; }
    static std::string list() { return "1, 2, 3"; }
};

template<>
struct TypeTraits<long long>
{
    static std::vector<long long> values() { return {1, -2, 3}; }
    static std::string list() { return "1, -2, 3"; }
};

template<>
struct TypeTraits<unsigned long long>
{
    static std::vector<unsigned long long> values() { return {1, 2, 3}; }
    static std::string list() { return "1, 2, 3"; }
};

template<>
struct TypeTraits<float>
{
    static std::vector<float> values() { return {1.2f, -2.5f, 3.14f}; }
    static std::string list() { return "1.2, -2.5, 3.14"; }
};

template<>
struct TypeTraits<double>
{
    static std::vector<double> values() { return {1.2, -2.5, 3.14}; }
    static std::string list() { return "1.2, -2.5, 3.14"; }
};

template<>
struct TypeTraits<long double>
{
    static std::vector<long double> values() { return {1.2l, -2.5l, 3.14l}; }
    static std::string list() { return "1.2, -2.5, 3.14"; }
};

template<>
struct TypeTraits<const char*>
{
    static std::vector<const char*> values() { return {"", "a", "xyz"}; }
    static std::string list() { return "'', 'a', 'xyz'"; }
};

template<>
struct TypeTraits<std::string>
{
    static std::vector<std::string> values() { return {"", "a", "xyz"}; }
    static std::string list() { return "'', 'a', 'xyz'"; }
};

template<>
struct TypeTraits<std::string_view>
{
    static std::vector<std::string_view> values() { return {"", "a", "xyz"}; }
    static std::string list() { return "'', 'a', 'xyz'"; }
};
