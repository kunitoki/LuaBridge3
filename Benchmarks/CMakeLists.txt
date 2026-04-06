cmake_minimum_required(VERSION 3.10)

include(FetchContent)

set(LUABRIDGE_BENCHMARK_WITH_SOL3 ON CACHE BOOL "Build Sol3 benchmark target")
set(LUABRIDGE_SOL2_GIT_REPOSITORY "https://github.com/ThePhD/sol2.git" CACHE STRING "sol2 repository URL")
set(LUABRIDGE_SOL2_GIT_TAG "v3.3.0" CACHE STRING "sol2 git tag or commit")

set(LUABRIDGE_BENCHMARK_WITH_LUABRIDGE ON CACHE BOOL "Build LuaBridge benchmark target")
set(LUABRIDGE_VANILLA_GIT_REPOSITORY "https://github.com/vinniefalco/LuaBridge.git" CACHE STRING "LuaBridge vanilla repository URL")
set(LUABRIDGE_VANILLA_GIT_TAG "master" CACHE STRING "LuaBridge vanilla git tag or commit")

set(LUABRIDGE_GOOGLE_BENCHMARK_GIT_REPOSITORY "https://github.com/google/benchmark.git" CACHE STRING "Google Benchmark repository URL")
set(LUABRIDGE_GOOGLE_BENCHMARK_GIT_TAG "v1.8.4" CACHE STRING "Google Benchmark git tag or commit")

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY ${LUABRIDGE_GOOGLE_BENCHMARK_GIT_REPOSITORY}
    GIT_TAG ${LUABRIDGE_GOOGLE_BENCHMARK_GIT_TAG})
FetchContent_MakeAvailable(googlebenchmark)

if (LUABRIDGE_BENCHMARK_WITH_SOL3)
    FetchContent_Declare(
        sol2
        GIT_REPOSITORY ${LUABRIDGE_SOL2_GIT_REPOSITORY}
        GIT_TAG ${LUABRIDGE_SOL2_GIT_TAG})
    FetchContent_MakeAvailable(sol2)

    # Work around a sol2 optional<T&>::emplace bug on recent Apple Clang toolchains.
    set(SOL2_OPTIONAL_IMPL "${sol2_SOURCE_DIR}/include/sol/optional_implementation.hpp")
    if (EXISTS "${SOL2_OPTIONAL_IMPL}")
        file(READ "${SOL2_OPTIONAL_IMPL}" SOL2_OPTIONAL_IMPL_CONTENT)
        set(SOL2_OPTIONAL_REFBLOCK_OLD "\t\ttemplate <class... Args>\n\t\tT& emplace(Args&&... args) noexcept {\n\t\t\tstatic_assert(std::is_constructible<T, Args&&...>::value, \"T must be constructible with Args\");\n\n\t\t\t*this = nullopt;\n\t\t\tthis->construct(std::forward<Args>(args)...);\n\t\t\treturn *m_value;\n\t\t}\n")
        string(FIND "${SOL2_OPTIONAL_IMPL_CONTENT}" "${SOL2_OPTIONAL_REFBLOCK_OLD}" SOL2_PATCH_NEEDLE_POS)
        if (NOT SOL2_PATCH_NEEDLE_POS EQUAL -1)
            set(SOL2_OPTIONAL_REFBLOCK_NEW "\t\ttemplate <class... Args>\n\t\tT& emplace(Args&&... args) noexcept {\n\t\t\tstatic_assert(std::is_constructible<T, Args&&...>::value, \"T must be constructible with Args\");\n\n\t\t\t*this = nullopt;\n\t\t\tint emplace_workaround[] = { 0, ((*this = std::forward<Args>(args)), 0)... };\n\t\t\t(void) emplace_workaround;\n\t\t\treturn *m_value;\n\t\t}\n")
            string(REPLACE
                "${SOL2_OPTIONAL_REFBLOCK_OLD}"
                "${SOL2_OPTIONAL_REFBLOCK_NEW}"
                SOL2_OPTIONAL_IMPL_CONTENT
                "${SOL2_OPTIONAL_IMPL_CONTENT}")
            file(WRITE "${SOL2_OPTIONAL_IMPL}" "${SOL2_OPTIONAL_IMPL_CONTENT}")
        endif()
    endif()
endif()

if (LUABRIDGE_BENCHMARK_WITH_LUABRIDGE)
    FetchContent_Declare(
        luabridge_vanilla
        GIT_REPOSITORY ${LUABRIDGE_VANILLA_GIT_REPOSITORY}
        GIT_TAG ${LUABRIDGE_VANILLA_GIT_TAG})
    FetchContent_GetProperties(luabridge_vanilla)
    if (NOT luabridge_vanilla_POPULATED)
        FetchContent_Populate(luabridge_vanilla)
    endif()
endif()

function(add_luabridge_benchmark_target target_name source_file)
    add_executable(${target_name}
        ${source_file}
        benchmark_common.cpp
        ../Tests/Lua/LuaLibrary5.4.8.cpp)

    target_include_directories(${target_name} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/..
        ${CMAKE_CURRENT_LIST_DIR}/../Tests
        ${CMAKE_CURRENT_LIST_DIR}/../Tests/Lua/Lua.5.4.8/src)

    target_compile_definitions(${target_name} PRIVATE
        LUABRIDGE_BENCHMARK_LUA54=1
        LUABRIDGE_TEST_LUA_VERSION=504)

    target_link_libraries(${target_name} PRIVATE
        benchmark::benchmark
        benchmark::benchmark_main)
endfunction()

add_luabridge_benchmark_target(LuaBridge3Benchmark benchmark_luabridge3.cpp)
target_include_directories(LuaBridge3Benchmark PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../Source)

if (LUABRIDGE_BENCHMARK_WITH_LUABRIDGE)
    add_luabridge_benchmark_target(LuaBridgeVanillaBenchmark benchmark_luabridge.cpp)
    target_include_directories(LuaBridgeVanillaBenchmark PRIVATE
        ${luabridge_vanilla_SOURCE_DIR}/Source)
endif()

if (LUABRIDGE_BENCHMARK_WITH_SOL3)
    add_luabridge_benchmark_target(Sol3Benchmark benchmark_sol3.cpp)
    target_include_directories(Sol3Benchmark PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/../Source
        ${sol2_SOURCE_DIR}/include)

    target_compile_definitions(Sol3Benchmark PRIVATE
        SOL_ALL_SAFETIES_ON=1
        SOL_NO_EXCEPTIONS=1
        SOL_LUA_VERSION=504)
endif()
