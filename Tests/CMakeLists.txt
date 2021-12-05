cmake_minimum_required (VERSION 3.5)

# ====================================================== Locations

set (LUABRIDGE_LUAU_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../ThirdParty/luau")

# ====================================================== Unit Tests Files

set (LUABRIDGE_TEST_SOURCE_FILES
  Source/AmalgamateTests.cpp
  Source/ArrayTests.cpp
  Source/ClassTests.cpp
  Source/IssueTests.cpp
  Source/IteratorTests.cpp
  Source/LegacyTests.cpp
  Source/LegacyTests.h
  Source/ListTests.cpp
  Source/LuaRefTests.cpp
  Source/MapTests.cpp
  Source/NamespaceTests.cpp
  Source/OptionalTests.cpp
  Source/PerformanceTests.cpp
  Source/RefCountedPtrTests.cpp
  Source/StackTests.cpp
  Source/Tests.cpp
  Source/TestBase.h
  Source/TestTypes.h
  Source/TestsMain.cpp
  Source/UnorderedMapTests.cpp
  Source/VectorTests.cpp
)

if (APPLE)
  list(APPEND LUABRIDGE_TEST_SOURCE_FILES Source/ObjCTests.mm)
endif (APPLE)

source_group ("Source" FILES ${LUABRIDGE_TEST_SOURCE_FILES})

# ====================================================== Lua 5.1

file (GLOB_RECURSE LUABRIDGE_TEST_LUA51_FILES
  LuaLibrary.h
  Lua/LuaLibrary5.1.5.cpp
)

# ====================================================== Lua 5.2

file (GLOB_RECURSE LUABRIDGE_TEST_LUA52_FILES
  Lua/LuaLibrary.h
  Lua/LuaLibrary5.2.4.cpp
)

# ====================================================== Lua 5.3

file (GLOB_RECURSE LUABRIDGE_TEST_LUA53_FILES
  Lua/LuaLibrary.h
  Lua/LuaLibrary5.3.6.cpp
)

# ====================================================== Lua 5.4

file (GLOB_RECURSE LUABRIDGE_TEST_LUA54_FILES
  Lua/LuaLibrary.h
  Lua/LuaLibrary5.4.3.cpp
)

# ====================================================== Luau

file (GLOB_RECURSE LUABRIDGE_TEST_LUAU_FILES
  Lua/LuaLibrary.h
  Lua/Luau.cpp
  Lua/LuauSplit.cpp
)

# ====================================================== Coverage

function (setup_target_for_coverage TARGET_NAME SOURCE_LOCATION SOURCE_PACKAGE)
  if ("${CMAKE_GENERATOR}" STREQUAL "Xcode")
    set_target_properties (${TARGET_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS "YES")
    set_target_properties (${TARGET_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_INSTRUMENT_PROGRAM_FLOW_ARCS "YES")
    set_target_properties (${TARGET_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_GENERATE_TEST_COVERAGE_FILES "YES")
  else ()
    target_compile_options (${TARGET_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
    target_link_options (${TARGET_NAME} PRIVATE -fprofile-arcs)
  endif()

  add_custom_command(TARGET ${TARGET_NAME} PRE_BUILD
  COMMAND ${FIND_EXECUTABLE} . -path "*/Tests/*/${TARGET_NAME}.*/*/*.gcda" -delete
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM)

  add_custom_command (
    OUTPUT "coverage/${TARGET_NAME}.info"
    COMMAND ${FIND_EXECUTABLE} . -path "*/Tests/*/${TARGET_NAME}.*/*/*.gcda" -delete
    COMMAND ${TARGET_NAME} ${ARGV3}
    COMMAND ${CMAKE_COMMAND} -E make_directory coverage
    COMMAND ${CMAKE_COMMAND} -E rm -f coverage/${TARGET_NAME}.info
    COMMAND ${LCOV_EXECUTABLE}
      -c -d "${CMAKE_BINARY_DIR}"
      --include "*/${SOURCE_PACKAGE}/*"
      --exclude "*/Tests/*"
      --exclude "*/Distribution/*"
      --exclude "*/coverage_html/*"
      --rc lcov_branch_coverage=1
      -o "coverage/${TARGET_NAME}.info"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM)

  # Message status
  message (STATUS "Luabridge3 -- Enabled code coverage reporting for ${TARGET_NAME}")
endfunction ()

function (setup_coverage_single_target)
  add_custom_target (LuaBridgeTestsCoverage
    COMMAND ${CMAKE_COMMAND} -E make_directory coverage_html
    COMMAND ${CMAKE_COMMAND} -E rm -Rf coverage_html/*
    COMMAND ${CMAKE_COMMAND} -E rm -f coverage/Merged.info
    COMMAND ${LCOV_EXECUTABLE}
      -a "coverage/LuaBridgeTests51.info"
      -a "coverage/LuaBridgeTests51Noexcept.info"
      -a "coverage/LuaBridgeTests52.info"
      -a "coverage/LuaBridgeTests52Noexcept.info"
      -a "coverage/LuaBridgeTests53.info"
      -a "coverage/LuaBridgeTests53Noexcept.info"
      -a "coverage/LuaBridgeTests54.info"
      -a "coverage/LuaBridgeTests54Noexcept.info"
      -a "coverage/LuaBridgeTestsLuau.info"
      -o "coverage/Merged.info"
    COMMAND ${GENHTML_EXECUTABLE}
      --rc lcov_branch_coverage=1
      "coverage/Merged.info" -o "coverage_html"
    DEPENDS
      "coverage/LuaBridgeTests51.info"
      "coverage/LuaBridgeTests51Noexcept.info"
      "coverage/LuaBridgeTests52.info"
      "coverage/LuaBridgeTests52Noexcept.info"
      "coverage/LuaBridgeTests53.info"
      "coverage/LuaBridgeTests53Noexcept.info"
      "coverage/LuaBridgeTests54.info"
      "coverage/LuaBridgeTests54Noexcept.info"
      "coverage/LuaBridgeTestsLuau.info"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM)

endfunction ()

# ====================================================== Macro

macro (add_test_app LUABRIDGE_TEST_NAME LUA_VERSION LUABRIDGE_TEST_LUA_LIBRARY_FILES LUABRIDGE_EXCEPTIONS)

  add_executable (${LUABRIDGE_TEST_NAME}
    ${LUABRIDGE_TEST_SOURCE_FILES}
    ${LUABRIDGE_TEST_LUA_LIBRARY_FILES}
  )

  if (LUABRIDGE_COVERAGE)
    get_filename_component (SOURCE_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../Source" ABSOLUTE)
    setup_target_for_coverage (${LUABRIDGE_TEST_NAME} ${SOURCE_LOCATION} LuaBridge)
  endif ()

  target_include_directories (${LUABRIDGE_TEST_NAME} PRIVATE . Source)
  if (${LUA_VERSION} STREQUAL "LUAU")
    target_include_directories (${LUABRIDGE_TEST_NAME} PRIVATE "${LUABRIDGE_LUAU_LOCATION}/VM/include")
    target_include_directories (${LUABRIDGE_TEST_NAME} PRIVATE "${LUABRIDGE_LUAU_LOCATION}/Ast/include")
    target_include_directories (${LUABRIDGE_TEST_NAME} PRIVATE "${LUABRIDGE_LUAU_LOCATION}/Compiler/include")
    target_compile_options (${LUABRIDGE_TEST_NAME} PUBLIC -DLUABRIDGEDEMO_LUAU=1)
  else () # if(${LUA_VERSION} MATCHES "^[0-9]*")
    target_compile_options (${LUABRIDGE_TEST_NAME} PUBLIC -DLUABRIDGEDEMO_LUA_VERSION=${LUA_VERSION})
  endif ()

  if (MSVC)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 /W3")
  else ()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
  endif (MSVC)

  if (NOT ${LUABRIDGE_EXCEPTIONS})
    target_compile_definitions (${LUABRIDGE_TEST_NAME} PUBLIC LUA_USE_LONGJMP=1)
    if (APPLE)
      target_compile_options (${LUABRIDGE_TEST_NAME} PUBLIC -fno-exceptions)
      set_target_properties (${LUABRIDGE_TEST_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_ENABLE_CPP_EXCEPTIONS "NO")
    elseif (WIN32)
      target_compile_options (${LUABRIDGE_TEST_NAME} PUBLIC /EHs-c-)
      target_compile_definitions (${LUABRIDGE_TEST_NAME} PUBLIC _HAS_EXCEPTIONS=0)
    else ()
      target_compile_options (${LUABRIDGE_TEST_NAME} PUBLIC -fno-exceptions)
    endif ()
  endif ()

  target_link_libraries (${LUABRIDGE_TEST_NAME}
    LuaBridge
    gtest
  )

endmacro (add_test_app)

# ====================================================== Real Unit Tests

add_test_app (LuaBridgeTests51 501 "${LUABRIDGE_TEST_LUA51_FILES}" 1)
add_test_app (LuaBridgeTests51Noexcept 501 "${LUABRIDGE_TEST_LUA51_FILES}" 0)

add_test_app (LuaBridgeTests52 502 "${LUABRIDGE_TEST_LUA52_FILES}" 1)
add_test_app (LuaBridgeTests52Noexcept 502 "${LUABRIDGE_TEST_LUA52_FILES}" 0)

add_test_app (LuaBridgeTests53 503 "${LUABRIDGE_TEST_LUA53_FILES}" 1)
add_test_app (LuaBridgeTests53Noexcept 503 "${LUABRIDGE_TEST_LUA53_FILES}" 0)

add_test_app (LuaBridgeTests54 504 "${LUABRIDGE_TEST_LUA54_FILES}" 1)
add_test_app (LuaBridgeTests54Noexcept 504 "${LUABRIDGE_TEST_LUA54_FILES}" 0)

add_test_app (LuaBridgeTestsLuau "LUAU" "${LUABRIDGE_TEST_LUAU_FILES}" 1)
#add_test_app (LuaBridgeTestsLuauNoexcept "LUAU" "${LUABRIDGE_TEST_LUAU_FILES}" 0)

if (LUABRIDGE_COVERAGE)
  setup_coverage_single_target ()
endif ()
