// https://github.com/kunitoki/LuaBridge
// Copyright 2020, Lucio Asnaghi
// Copyright 2019, George Tokmaji
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT

//=================================================================================================
/*
  This file incorporates work covered by the following copyright and
  permission notice:

    The Loki Library
    Copyright (c) 2001 by Andrei Alexandrescu
    This code accompanies the book:
    Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design
        Patterns Applied". Copyright (c) 2001. Addison-Wesley.
    Permission to use, copy, modify, distribute and sell this software for any
        purpose is hereby granted without fee, provided that the above copyright
        notice appear in all copies and that both that copyright notice and this
        permission notice appear in supporting documentation.
    The author or Addison-Welsey Longman make no representations about the
        suitability of this software for any purpose. It is provided "as is"
        without express or implied warranty.
*/
//=================================================================================================

#pragma once

#include "Config.h"
#include "Stack.h"

#include <string>
#include <typeinfo>
#include <tuple>

namespace luabridge {
namespace detail {

//=================================================================================================
/**
 * @brief None type means void parameters or return value.
 */
using None = void;

//=================================================================================================
/**
 * @brief Type list type pack..
 */
template <class Head, class Tail = None>
struct TypeList
{
    typedef Tail TailType;
};

template <class List>
struct TypeListSize
{
    static constexpr size_t value = TypeListSize<typename List::TailType>::value + 1;
};

template <>
struct TypeListSize<None>
{
    static constexpr size_t value = 0u;
};

template <class... Params>
struct MakeTypeList;

template <class Param, class... Params>
struct MakeTypeList<Param, Params...>
{
    using Result = TypeList<Param, typename MakeTypeList<Params...>::Result>;
};

template <>
struct MakeTypeList<>
{
    using Result = None;
};

//=================================================================================================
/**
 * @brief A TypeList with actual values.
 */
template <class List>
struct TypeListValues
{
    static std::string const tostring(bool) { return ""; }
};

//=================================================================================================
/**
 * @brief TypeListValues recursive template definition.
 */
template <class Head, class Tail>
struct TypeListValues<TypeList<Head, Tail>>
{
    Head hd;
    TypeListValues<Tail> tl;

    TypeListValues(Head hd_, const TypeListValues<Tail>& tl_)
        : hd(hd_)
        , tl(tl_)
    {
    }

    static std::string tostring(bool comma = false)
    {
        std::string s;

        if (comma)
            s = ", ";

        s = s + typeid(Head).name();

        return s + TypeListValues<Tail>::tostring(true);
    }
};

// Specializations of type/value list for head types that are references and
// const-references.  We need to handle these specially since we can't count
// on the referenced object hanging around for the lifetime of the list.

template <class Head, class Tail>
struct TypeListValues<TypeList<Head&, Tail>>
{
    Head hd;
    TypeListValues<Tail> tl;

    TypeListValues(Head& hd_, const TypeListValues<Tail>& tl_)
        : hd(hd_)
        , tl(tl_)
    {
    }

    static std::string tostring(bool comma = false)
    {
        std::string s;

        if (comma)
            s = ", ";

        s = s + typeid(Head).name() + "&";

        return s + TypeListValues<Tail>::tostring(true);
    }
};

template <class Head, class Tail>
struct TypeListValues<TypeList<const Head&, Tail>>
{
    Head hd;
    TypeListValues<Tail> tl;

    TypeListValues(Head const& hd_, const TypeListValues<Tail>& tl_)
        : hd(hd_)
        , tl(tl_)
    {
    }

    static std::string tostring(bool comma = false)
    {
        std::string s;

        if (comma)
            s = ", ";

        s = s + typeid(Head).name() + " const&";

        return s + TypeListValues<Tail>::tostring(true);
    }
};

//==============================================================================
/**
 * @brief Type list to tuple forwarder.
 */
template <class Head, class Tail>
auto typeListValuesTuple(TypeListValues<TypeList<Head, Tail>>& tvl)
{
    if constexpr (std::is_same_v<Tail, void>)
    {
        return std::forward_as_tuple(tvl.hd);
    }
    else
    {
        return std::tuple_cat(std::forward_as_tuple(tvl.hd), typeListValuesTuple(tvl.tl));
    }
}

template <class Head, class Tail>
auto typeListValuesTuple(const TypeListValues<TypeList<Head, Tail>>& tvl)
{
    if constexpr (std::is_same_v<Tail, void>)
    {
        return std::forward_as_tuple(tvl.hd);
    }
    else
    {
        return std::tuple_cat(std::forward_as_tuple(tvl.hd), typeListValuesTuple(tvl.tl));
    }
}

//==============================================================================
/**
 * @brief Subclass of a TypeListValues constructable from the Lua stack.
 */
template <class List, size_t Start = 1>
struct ArgList
{
};

template <size_t Start>
struct ArgList<None, Start> : public TypeListValues<None>
{
    ArgList(lua_State*)
    {
    }
};

template <class Head, class Tail, size_t Start>
struct ArgList<TypeList<Head, Tail>, Start> : public TypeListValues<TypeList<Head, Tail>>
{
    ArgList(lua_State* L)
        : TypeListValues<TypeList<Head, Tail>>(Stack<Head>::get(L, Start), ArgList<Tail, Start + 1>(L))
    {
    }
};

} // namespace detail
} // namespace luabridge
