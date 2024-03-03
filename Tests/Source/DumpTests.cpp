// https://github.com/kunitoki/LuaBridge3
// Copyright 2023, Lucio Asnaghi
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include "LuaBridge/Dump.h"

#include <sstream>

struct DumpTests : TestBase
{
};

TEST_F(DumpTests, DumpNil)
{
    std::stringstream ss;

    lua_pushnil(L);

    luabridge::dumpValue(L, -1, 0, 0, false, ss);
    EXPECT_EQ("nil", ss.str());
    EXPECT_TRUE(ss.str().find("\n") == std::string::npos);

    ss.clear();
    luabridge::dumpValue(L, -1, 0, 0, true, ss);
    EXPECT_TRUE(ss.str().find("\n") != std::string::npos);
}

TEST_F(DumpTests, DumpBoolean)
{
    {
        std::stringstream ss;

        lua_pushboolean(L, 1);

        luabridge::dumpValue(L, -1, 0, 0, false, ss);
        EXPECT_EQ("true", ss.str());
        EXPECT_TRUE(ss.str().find("\n") == std::string::npos);

        ss.clear();
        luabridge::dumpValue(L, -1, 0, 0, true, ss);
        EXPECT_TRUE(ss.str().find("\n") != std::string::npos);
    }

    {
        std::stringstream ss;

        lua_pushboolean(L, 0);

        luabridge::dumpValue(L, -1, 0, 0, false, ss);
        EXPECT_EQ("false", ss.str());
        EXPECT_TRUE(ss.str().find("\n") == std::string::npos);

        ss.clear();
        luabridge::dumpValue(L, -1, 0, 0, true, ss);
        EXPECT_TRUE(ss.str().find("\n") != std::string::npos);
    }
}

TEST_F(DumpTests, DumpNumber)
{
    {
        std::stringstream ss;

        lua_pushnumber(L, 1);

        luabridge::dumpValue(L, -1, 0, 0, false, ss);
        EXPECT_EQ("1", ss.str());
        EXPECT_TRUE(ss.str().find("\n") == std::string::npos);

        ss.clear();
        luabridge::dumpValue(L, -1, 0, 0, true, ss);
        EXPECT_TRUE(ss.str().find("\n") != std::string::npos);
    }

    {
        std::stringstream ss;

        lua_pushnumber(L, 1.25);

        luabridge::dumpValue(L, -1, 0, 0, false, ss);
        EXPECT_EQ("1.25", ss.str());
        EXPECT_TRUE(ss.str().find("\n") == std::string::npos);

        ss.clear();
        luabridge::dumpValue(L, -1, 0, 0, true, ss);
        EXPECT_TRUE(ss.str().find("\n") != std::string::npos);
    }
}

TEST_F(DumpTests, DumpString)
{
    {
        std::stringstream ss;

        lua_pushliteral(L, "");

        luabridge::dumpValue(L, -1, 0, 0, false, ss);
        EXPECT_EQ("\"\"", ss.str());
        EXPECT_TRUE(ss.str().find("\n") == std::string::npos);

        ss.clear();
        luabridge::dumpValue(L, -1, 0, 0, true, ss);
        EXPECT_TRUE(ss.str().find("\n") != std::string::npos);
    }

    {
        std::stringstream ss;

        lua_pushliteral(L, "this_is_a_literal_string");

        luabridge::dumpValue(L, -1, 0, 0, false, ss);
        EXPECT_EQ("\"this_is_a_literal_string\"", ss.str());
        EXPECT_TRUE(ss.str().find("\n") == std::string::npos);

        ss.clear();
        luabridge::dumpValue(L, -1, 0, 0, true, ss);
        EXPECT_TRUE(ss.str().find("\n") != std::string::npos);
    }
}

TEST_F(DumpTests, DumpFunction)
{
    {
        std::stringstream ss;

        luabridge::lua_pushcfunction_x(L, nullptr, "");

        luabridge::dumpValue(L, -1, 0, 0, false, ss);
        EXPECT_TRUE(ss.str().find("cfunction@") == 0);
        EXPECT_TRUE(ss.str().find("\n") == std::string::npos);

        ss.clear();
        luabridge::dumpValue(L, -1, 0, 0, true, ss);
        EXPECT_TRUE(ss.str().find("\n") != std::string::npos);
    }

    {
        std::stringstream ss;

        luabridge::lua_pushcfunction_x(L, +[](lua_State*) { return 0; }, "");

        luabridge::dumpValue(L, -1, 0, 0, false, ss);
        EXPECT_TRUE(ss.str().find("cfunction@") == 0);
        EXPECT_TRUE(ss.str().find("\n") == std::string::npos);

        ss.clear();
        luabridge::dumpValue(L, -1, 0, 0, true, ss);
        EXPECT_TRUE(ss.str().find("\n") != std::string::npos);
    }

    {
        std::stringstream ss;

        runLua("result = function() end");

        result().push(L);

        luabridge::dumpValue(L, -1, 0, 0, false, ss);
        EXPECT_TRUE(ss.str().find("function@") == 0);
        EXPECT_TRUE(ss.str().find("\n") == std::string::npos);

        ss.clear();
        luabridge::dumpValue(L, -1, 0, 0, true, ss);
        EXPECT_TRUE(ss.str().find("\n") != std::string::npos);
    }
}

TEST_F(DumpTests, DumpThread)
{
    std::stringstream ss;

    lua_newthread(L);

    luabridge::dumpValue(L, -1, 0, 0, false, ss);
    EXPECT_TRUE(ss.str().find("thread@") == 0);
    EXPECT_TRUE(ss.str().find("\n") == std::string::npos);

    ss.clear();
    luabridge::dumpValue(L, -1, 0, 0, true, ss);
    EXPECT_TRUE(ss.str().find("\n") != std::string::npos);
}

TEST_F(DumpTests, DumpLightUserData)
{
    std::stringstream ss;

    lua_pushlightuserdata(L, nullptr);

    luabridge::dumpValue(L, -1, 0, 0, false, ss);
    EXPECT_TRUE(ss.str().find("lightuserdata@") == 0);
    EXPECT_TRUE(ss.str().find("\n") == std::string::npos);

    ss.clear();
    luabridge::dumpValue(L, -1, 0, 0, true, ss);
    EXPECT_TRUE(ss.str().find("\n") != std::string::npos);
}

TEST_F(DumpTests, DumpUserData)
{
    std::stringstream ss;

    lua_newuserdata(L, sizeof(int));

    luabridge::dumpValue(L, -1, 0, 0, false, ss);
    EXPECT_TRUE(ss.str().find("userdata@") == 0);
    EXPECT_TRUE(ss.str().find("\n") == std::string::npos);

    ss.clear();
    luabridge::dumpValue(L, -1, 0, 0, true, ss);
    EXPECT_TRUE(ss.str().find("\n") != std::string::npos);
}

TEST_F(DumpTests, DumpTable)
{
    {
        std::stringstream ss;

        lua_newtable(L);

        luabridge::dumpValue(L, -1, 0, 0, false, ss);
        EXPECT_TRUE(ss.str().find("table@") == 0);
        EXPECT_TRUE(ss.str().find("\n") == std::string::npos);

        ss.clear();
        luabridge::dumpValue(L, -1, 0, 0, true, ss);
        EXPECT_TRUE(ss.str().find("\n") != std::string::npos);
    }

    {
        std::stringstream ss;

        lua_newtable(L);
        lua_pushliteral(L, "key1");
        lua_pushliteral(L, "value1");
        lua_settable(L, -3);
        lua_pushliteral(L, "key2");
        lua_pushliteral(L, "value2");
        lua_settable(L, -3);

        luabridge::dumpValue(L, -1, 1, 0, false, ss);
        EXPECT_TRUE(ss.str().find("table@") == 0);
        EXPECT_TRUE(ss.str().find("\n") != std::string::npos);
        EXPECT_TRUE(ss.str().find("{") != std::string::npos);
        EXPECT_TRUE(ss.str().find("\"key1\": \"value1\",") != std::string::npos);
        EXPECT_TRUE(ss.str().find("\"key2\": \"value2\",") != std::string::npos);
        EXPECT_TRUE(ss.str().find("}") != std::string::npos);
    }

    {
        std::stringstream ss;

        lua_newtable(L);
        lua_pushliteral(L, "first");
        lua_newtable(L);
        lua_pushliteral(L, "second");
        lua_pushliteral(L, "value");
        lua_settable(L, -3);
        lua_settable(L, -3);

        luabridge::dumpValue(L, -1, 0, 0, false, ss);
        EXPECT_TRUE(ss.str().find("table@") == 0);
        EXPECT_TRUE(ss.str().find("\n") != std::string::npos);
        EXPECT_TRUE(ss.str().find("{") != std::string::npos);
        EXPECT_TRUE(ss.str().find("\"first\": table@") != std::string::npos);
        EXPECT_TRUE(ss.str().find("\"second\": \"value\"") == std::string::npos);
        EXPECT_TRUE(ss.str().find("}") != std::string::npos);

        ss.clear();
        luabridge::dumpValue(L, -1, 1, 0, false, ss);
        EXPECT_TRUE(ss.str().find("\"second\": \"value\"") != std::string::npos);
    }
}

TEST_F(DumpTests, DumpState)
{
    std::stringstream ss;

    lua_pushnil(L);
    lua_pushboolean(L, 1);
    lua_pushnumber(L, 1337);
    lua_pushliteral(L, "this_is_a_literal_string");

    luabridge::dumpState(L, 0, ss);
    EXPECT_TRUE(ss.str().find("stack #1 (-4): nil") != std::string::npos);
    EXPECT_TRUE(ss.str().find("stack #2 (-3): true") != std::string::npos);
    EXPECT_TRUE(ss.str().find("stack #3 (-2): 1337") != std::string::npos);
    EXPECT_TRUE(ss.str().find("stack #4 (-1): \"this_is_a_literal_string\"") != std::string::npos);
}
