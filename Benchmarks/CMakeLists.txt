cmake_minimum_required(VERSION 3.10)

include(FetchContent)

set(LUABRIDGE_BENCHMARK_SOURCES
    Benchmark.cpp
    ../Tests/Lua/LuaLibrary5.4.8.cpp)

add_executable(LuaBridgeBenchmarks ${LUABRIDGE_BENCHMARK_SOURCES})

target_include_directories(LuaBridgeBenchmarks PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/..
    ${CMAKE_CURRENT_LIST_DIR}/../Source
    ${CMAKE_CURRENT_LIST_DIR}/../Tests
    ${CMAKE_CURRENT_LIST_DIR}/../Tests/Lua/Lua.5.4.8/src)

target_compile_definitions(LuaBridgeBenchmarks PRIVATE
    LUABRIDGE_BENCHMARK_LUA54=1
    LUABRIDGE_TEST_LUA_VERSION=504)

set(LUABRIDGE_SOL2_GIT_REPOSITORY "https://github.com/ThePhD/sol2.git" CACHE STRING "sol2 repository URL")
set(LUABRIDGE_SOL2_GIT_TAG "v3.3.0" CACHE STRING "sol2 git tag or commit")

FetchContent_Declare(
    sol2
    GIT_REPOSITORY ${LUABRIDGE_SOL2_GIT_REPOSITORY}
    GIT_TAG ${LUABRIDGE_SOL2_GIT_TAG})
FetchContent_MakeAvailable(sol2)

target_include_directories(LuaBridgeBenchmarks PRIVATE
    ${sol2_SOURCE_DIR}/include)

target_compile_definitions(LuaBridgeBenchmarks PRIVATE
    LUABRIDGE_BENCHMARK_WITH_SOL2=1
    SOL_LUA_VERSION=504)
