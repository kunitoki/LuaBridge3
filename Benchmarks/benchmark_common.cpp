// https://github.com/kunitoki/LuaBridge3
// Copyright 2026, kunitoki
// Inspired from https://github.com/ThePhD/lua-bindings-shootout by ThePhD
// SPDX-License-Identifier: MIT

#include "benchmark_common.hpp"

#include <string>

namespace lbsbench {

void luaCheckOrThrow(lua_State* L, int status, std::string_view where)
{
    if (status == LUA_OK)
        return;

    const char* message = lua_tostring(L, -1);
    std::string error(where);
    error += ": ";
    error += (message ? message : "unknown lua error");
    lua_pop(L, 1);
    throw std::runtime_error(error);
}

void luaDoStringOrThrow(lua_State* L, std::string_view code, std::string_view where)
{
    const int status = luaL_dostring(L, std::string(code).c_str());
    luaCheckOrThrow(L, status, where);
}

} // namespace lbsbench
