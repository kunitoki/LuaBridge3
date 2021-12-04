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
  find_program (FIND_EXECUTABLE find)
  if (NOT FIND_EXECUTABLE)
    message (WARNING "Luabridge3 -- Command find not found! Code coverage target will not be available...")
    return ()
  endif ()

  find_program (LCOV_EXECUTABLE lcov)
  if (NOT LCOV_EXECUTABLE)
    message (WARNING "Luabridge3 -- Command lcov not found! Code coverage target will not be available...")
    return ()
  endif ()

  find_program (GENHTML_EXECUTABLE genhtml)
  if (NOT GENHTML_EXECUTABLE)
    message (WARNING "Luabridge3 -- Command genhtml not found! Code coverage target will not be available...")
    return ()
  endif ()

  set_target_properties (${TARGET_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS "YES")
  set_target_properties (${TARGET_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_INSTRUMENT_PROGRAM_FLOW_ARCS "YES")
  set_target_properties (${TARGET_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_GENERATE_TEST_COVERAGE_FILES "YES")

  # Remove previous gcda files on pre build
  add_custom_command (TARGET ${TARGET_NAME} PRE_BUILD
    COMMAND ${FIND_EXECUTABLE} . -iname "*.gcda" -delete)

  # Create coverage targets
  add_custom_target (${TARGET_NAME}_coverage
    ${FIND_EXECUTABLE} . -iname "*.gcda" -delete
    COMMAND ${TARGET_NAME} ${ARGV3}
    COMMAND ${CMAKE_COMMAND} -E make_directory coverage
    COMMAND ${CMAKE_COMMAND} -E make_directory coverage_html
    COMMAND ${CMAKE_COMMAND} -E remove coverage/*
    COMMAND ${CMAKE_COMMAND} -E remove coverage_html/*
    COMMAND "${LCOV_EXECUTABLE}"
      "-c" "-d" "${CMAKE_BINARY_DIR}"
      "--include" "*/${SOURCE_PACKAGE}/*"
      "--exclude" "*/Tests/*"
      "--exclude" "*/Distribution/*"
      "-o" "coverage/${TARGET_NAME}.info"
    COMMAND "${GENHTML_EXECUTABLE}"
      "coverage/${TARGET_NAME}.info" "-o" "coverage_html"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

  # Message status
  message (STATUS "Luabridge3 -- Enabled code coverage reporting for ${TARGET_NAME}")
endfunction ()

# ====================================================== Macro

macro (add_test_app LUABRIDGE_TEST_NAME LUA_VERSION LUABRIDGE_TEST_LUA_LIBRARY_FILES LUABRIDGE_EXCEPTIONS)

  add_executable (${LUABRIDGE_TEST_NAME}
    ${LUABRIDGE_TEST_SOURCE_FILES}
    ${LUABRIDGE_TEST_LUA_LIBRARY_FILES}
  )

  if (LUABRIDGE_COVERAGE)
    #set_target_properties (${LUABRIDGE_TEST_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS "YES")
    #set_target_properties (${LUABRIDGE_TEST_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_INSTRUMENT_PROGRAM_FLOW_ARCS "YES")
    #set_target_properties (${LUABRIDGE_TEST_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_GENERATE_TEST_COVERAGE_FILES "YES")

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
