cmake_minimum_required (VERSION 3.5)

project (LuaBridge)

include (CMakeDependentOption)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

find_program (FIND_EXECUTABLE find)
find_program (LCOV_EXECUTABLE lcov)
find_program (GENHTML_EXECUTABLE genhtml)

cmake_dependent_option (LUABRIDGE_TESTING "Build tests" ON "CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)
cmake_dependent_option (LUABRIDGE_COVERAGE "Enable coverage" ON "LUABRIDGE_TESTING;FIND_EXECUTABLE;LCOV_EXECUTABLE;GENHTML_EXECUTABLE" OFF)

add_subdirectory (Source)

if (LUABRIDGE_TESTING)
    set (gtest_force_shared_crt ON CACHE BOOL "Use /MD and /MDd" FORCE)
    add_subdirectory (ThirdParty/googletest)
    add_subdirectory (Tests)
endif ()

add_custom_target (Documentation SOURCES
    CHANGES.md
    README.md
    Manual.md
    Doxyfile
)
