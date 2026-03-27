# Install script for directory: /home/runner/work/LuaBridge3/LuaBridge3/Tests/Lua/LuaJIT.2.1

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/home/runner/work/LuaBridge3/LuaBridge3/Build/Tests/Lua/LuaJIT.2.1/luaconf.h"
    "/home/runner/work/LuaBridge3/LuaBridge3/Tests/Lua/LuaJIT.2.1/src/lua.h"
    "/home/runner/work/LuaBridge3/LuaBridge3/Tests/Lua/LuaJIT.2.1/src/lauxlib.h"
    "/home/runner/work/LuaBridge3/LuaBridge3/Tests/Lua/LuaJIT.2.1/src/lualib.h"
    "/home/runner/work/LuaBridge3/LuaBridge3/Tests/Lua/LuaJIT.2.1/src/lua.hpp"
    "/home/runner/work/LuaBridge3/LuaBridge3/Tests/Lua/LuaJIT.2.1/src/luajit.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/lua/5.1/jit" TYPE FILE FILES
    "/home/runner/work/LuaBridge3/LuaBridge3/Tests/Lua/LuaJIT.2.1/src/jit/bc.lua"
    "/home/runner/work/LuaBridge3/LuaBridge3/Tests/Lua/LuaJIT.2.1/src/jit/v.lua"
    "/home/runner/work/LuaBridge3/LuaBridge3/Tests/Lua/LuaJIT.2.1/src/jit/dump.lua"
    "/home/runner/work/LuaBridge3/LuaBridge3/Tests/Lua/LuaJIT.2.1/src/jit/dis_x86.lua"
    "/home/runner/work/LuaBridge3/LuaBridge3/Tests/Lua/LuaJIT.2.1/src/jit/dis_x64.lua"
    "/home/runner/work/LuaBridge3/LuaBridge3/Tests/Lua/LuaJIT.2.1/src/jit/dis_arm.lua"
    "/home/runner/work/LuaBridge3/LuaBridge3/Tests/Lua/LuaJIT.2.1/src/jit/dis_ppc.lua"
    "/home/runner/work/LuaBridge3/LuaBridge3/Tests/Lua/LuaJIT.2.1/src/jit/dis_mips.lua"
    "/home/runner/work/LuaBridge3/LuaBridge3/Tests/Lua/LuaJIT.2.1/src/jit/dis_mipsel.lua"
    "/home/runner/work/LuaBridge3/LuaBridge3/Tests/Lua/LuaJIT.2.1/src/jit/bcsave.lua"
    "/home/runner/work/LuaBridge3/LuaBridge3/Build/Tests/Lua/LuaJIT.2.1/vmdef.lua"
    "/home/runner/work/LuaBridge3/LuaBridge3/Tests/Lua/LuaJIT.2.1/src/jit/p.lua"
    "/home/runner/work/LuaBridge3/LuaBridge3/Tests/Lua/LuaJIT.2.1/src/jit/zone.lua"
    "/home/runner/work/LuaBridge3/LuaBridge3/Tests/Lua/LuaJIT.2.1/src/jit/dis_arm64.lua"
    "/home/runner/work/LuaBridge3/LuaBridge3/Tests/Lua/LuaJIT.2.1/src/jit/dis_arm64be.lua"
    "/home/runner/work/LuaBridge3/LuaBridge3/Tests/Lua/LuaJIT.2.1/src/jit/dis_mips64.lua"
    "/home/runner/work/LuaBridge3/LuaBridge3/Tests/Lua/LuaJIT.2.1/src/jit/dis_mips64el.lua"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/runner/work/LuaBridge3/LuaBridge3/Build/Tests/Lua/LuaJIT.2.1/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
