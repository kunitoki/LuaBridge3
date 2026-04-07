cmake_minimum_required (VERSION 3.10)

project (LuaBridge)

include (CMakeDependentOption)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

find_program (FIND_EXECUTABLE find)
find_program (LCOV_EXECUTABLE lcov)
find_program (GENHTML_EXECUTABLE genhtml)

cmake_dependent_option (LUABRIDGE_TESTING "Build tests" ON "CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)
cmake_dependent_option (LUABRIDGE_SANITIZE "Sanitizer to enable (address, undefined, thread)" OFF "LUABRIDGE_TESTING" OFF)
cmake_dependent_option (LUABRIDGE_COVERAGE "Enable coverage" OFF "LUABRIDGE_TESTING;FIND_EXECUTABLE;LCOV_EXECUTABLE;GENHTML_EXECUTABLE" OFF)
cmake_dependent_option (LUABRIDGE_BENCHMARKS "Build benchmark executable" OFF "CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)

add_subdirectory (Source)

if (PROJECT_IS_TOP_LEVEL AND LUABRIDGE_TESTING)
    include (CTest)
    enable_testing()
    set (gtest_force_shared_crt ON CACHE BOOL "Use /MD and /MDd" FORCE)
    add_subdirectory (ThirdParty/googletest)
    add_subdirectory (Tests)
endif ()

if (LUABRIDGE_BENCHMARKS)
    add_subdirectory (Benchmarks)
endif ()

add_custom_target (Documentation SOURCES
    CHANGES.md
    README.md
    Manual.md
    Doxyfile
)
