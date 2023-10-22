cmake_minimum_required (VERSION 3.5)

# ====================================================== Locations

set (LUABRIDGE_LUAJIT_LOCATION "${CMAKE_CURRENT_LIST_DIR}/Lua/LuaJIT.2.1")
set (LUABRIDGE_LUAU_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../ThirdParty/luau")
set (LUABRIDGE_RAVI_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../ThirdParty/ravi")

add_subdirectory(${LUABRIDGE_LUAJIT_LOCATION})
add_subdirectory(${LUABRIDGE_RAVI_LOCATION} ravi)

# ====================================================== Unit Tests Files

set (LUABRIDGE_TEST_SOURCE_FILES
  Source/AmalgamateTests.cpp
  Source/ArrayTests.cpp
  Source/ClassExtensibleTests.cpp
  Source/ClassTests.cpp
  Source/CoroutineTests.cpp
  Source/DumpTests.cpp
  Source/EnumTests.cpp
  Source/ExceptionTests.cpp
  Source/FlagSetTests.cpp
  Source/IssueTests.cpp
  Source/IteratorTests.cpp
  Source/LegacyTests.cpp
  Source/LegacyTests.h
  Source/ListTests.cpp
  Source/LuaRefTests.cpp
  Source/MapTests.cpp
  Source/NamespaceTests.cpp
  Source/OptionalTests.cpp
  Source/OverloadTests.cpp
  Source/PairTests.cpp
  Source/PerformanceTests.cpp
  Source/RefCountedPtrTests.cpp
  Source/ScopeGuardTests.cpp
  Source/SetTests.cpp
  Source/StackTests.cpp
  Source/Tests.cpp
  Source/TestBase.h
  Source/TestTypes.h
  Source/TestsMain.cpp
  Source/UnorderedMapTests.cpp
  Source/UserdataTests.cpp
  Source/VectorTests.cpp
)

if (APPLE)
  list(APPEND LUABRIDGE_TEST_SOURCE_FILES Source/ObjCTests.mm)
endif ()

source_group ("Source Files" FILES ${LUABRIDGE_TEST_SOURCE_FILES})

# ====================================================== Lua C

set (LUABRIDGE_LUA_C_DEFINES "LUABRIDGE_SAFE_LUA_C_EXCEPTION_HANDLING=1")

# ====================================================== Lua 5.1

file (GLOB_RECURSE LUABRIDGE_TEST_LUA51_FILES
  Lua/LuaLibrary.h
  Lua/LuaLibrary5.1.5.cpp
)

file (GLOB_RECURSE LUABRIDGE_TEST_LUA51_C_FILES
  Lua/LuaLibrary.h
  Lua/LuaLibrary5.1.5.c
)

# ====================================================== Lua 5.2

file (GLOB_RECURSE LUABRIDGE_TEST_LUA52_FILES
  Lua/LuaLibrary.h
  Lua/LuaLibrary5.2.4.cpp
)

file (GLOB_RECURSE LUABRIDGE_TEST_LUA52_C_FILES
  Lua/LuaLibrary.h
  Lua/LuaLibrary5.2.4.c
)

# ====================================================== Lua 5.3

file (GLOB_RECURSE LUABRIDGE_TEST_LUA53_FILES
  Lua/LuaLibrary.h
  Lua/LuaLibrary5.3.6.cpp
)

file (GLOB_RECURSE LUABRIDGE_TEST_LUA53_C_FILES
  Lua/LuaLibrary.h
  Lua/LuaLibrary5.3.6.c
)

# ====================================================== Lua 5.4

file (GLOB_RECURSE LUABRIDGE_TEST_LUA54_FILES
  Lua/LuaLibrary.h
  Lua/LuaLibrary5.4.6.cpp
)

file (GLOB_RECURSE LUABRIDGE_TEST_LUA54_C_FILES
  Lua/LuaLibrary.h
  Lua/LuaLibrary5.4.6.c
)

# ====================================================== Luau

file (GLOB_RECURSE LUABRIDGE_TEST_LUAU_FILES
  Lua/LuaLibrary.h
  Lua/Luau.cpp
  Lua/LuauSplit.cpp
)

# ====================================================== LuaJIT

file (GLOB_RECURSE LUABRIDGE_TEST_LUAJIT_FILES
  Lua/LuaLibrary.h
)

# ====================================================== Ravi

file (GLOB_RECURSE LUABRIDGE_TEST_RAVI_FILES
  Lua/LuaLibrary.h
)

# ====================================================== Shared Library

file (GLOB_RECURSE LUABRIDGE_TEST_SHARED_LIBRARY_FILES
  Source/SharedCode.h
  Source/SharedCode.cpp
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
  endif ()

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
      -a "coverage/LuaBridgeTests51LuaC.info"
      -a "coverage/LuaBridgeTests51Noexcept.info"
      -a "coverage/LuaBridgeTests51LuaCNoexcept.info"
      -a "coverage/LuaBridgeTests52.info"
      -a "coverage/LuaBridgeTests52LuaC.info"
      -a "coverage/LuaBridgeTests52Noexcept.info"
      -a "coverage/LuaBridgeTests52LuaCNoexcept.info"
      -a "coverage/LuaBridgeTests53.info"
      -a "coverage/LuaBridgeTests53LuaC.info"
      -a "coverage/LuaBridgeTests53Noexcept.info"
      -a "coverage/LuaBridgeTests53LuaCNoexcept.info"
      -a "coverage/LuaBridgeTests54.info"
      -a "coverage/LuaBridgeTests54LuaC.info"
      -a "coverage/LuaBridgeTests54Noexcept.info"
      -a "coverage/LuaBridgeTests54LuaCNoexcept.info"
      -a "coverage/LuaBridgeTestsLuaJIT.info"
      -a "coverage/LuaBridgeTestsLuaJITNoexcept.info"
      -a "coverage/LuaBridgeTestsLuau.info"
      -a "coverage/LuaBridgeTestsRavi.info"
      -o "coverage/Merged.info"
    COMMAND ${GENHTML_EXECUTABLE}
      --rc lcov_branch_coverage=1
      "coverage/Merged.info" -o "coverage_html"
    DEPENDS
      "coverage/LuaBridgeTests51.info"
      "coverage/LuaBridgeTests51LuaC.info"
      "coverage/LuaBridgeTests51Noexcept.info"
      "coverage/LuaBridgeTests51LuaCNoexcept.info"
      "coverage/LuaBridgeTests52.info"
      "coverage/LuaBridgeTests52LuaC.info"
      "coverage/LuaBridgeTests52Noexcept.info"
      "coverage/LuaBridgeTests52LuaCNoexcept.info"
      "coverage/LuaBridgeTests53.info"
      "coverage/LuaBridgeTests53LuaC.info"
      "coverage/LuaBridgeTests53Noexcept.info"
      "coverage/LuaBridgeTests53LuaCNoexcept.info"
      "coverage/LuaBridgeTests54.info"
      "coverage/LuaBridgeTests54LuaC.info"
      "coverage/LuaBridgeTests54Noexcept.info"
      "coverage/LuaBridgeTests54LuaCNoexcept.info"
      "coverage/LuaBridgeTestsLuaJIT.info"
      "coverage/LuaBridgeTestsLuaJITNoexcept.info"
      "coverage/LuaBridgeTestsLuau.info"
      "coverage/LuaBridgeTestsRavi.info"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM)

endfunction ()

# ====================================================== Macro

macro (add_test_app LUABRIDGE_TEST_NAME LUA_VERSION LUABRIDGE_TEST_LUA_LIBRARY_FILES LUABRIDGE_EXCEPTIONS LUABRIDGE_DEFINES LUABRIDGE_LIBS)
  get_filename_component (SOURCE_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../Source" ABSOLUTE)

  if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 /W3 /MP /D_CRT_SECURE_NO_WARNINGS")
  else ()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
  endif ()

  # Dynamic library test
  set(LUABRIDGE_TESTLIB_NAME ${LUABRIDGE_TEST_NAME}_DynamicLibrary)
  add_library (${LUABRIDGE_TESTLIB_NAME} SHARED
    ${LUABRIDGE_TEST_SHARED_LIBRARY_FILES}
    ${LUABRIDGE_TEST_LUA_LIBRARY_FILES})

  target_compile_definitions (${LUABRIDGE_TESTLIB_NAME} PRIVATE
    LUABRIDGEDEMO_SHARED_EXPORT=1
    ${LUABRIDGE_DEFINES})

  target_include_directories (${LUABRIDGE_TESTLIB_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${SOURCE_LOCATION}
    Source)

  list(APPEND LUABRIDGE_TEST_SOURCE_FILES Source/DynamicLibraryTests.cpp)

  # Main binary test
  add_executable (${LUABRIDGE_TEST_NAME}
    ${LUABRIDGE_TEST_SOURCE_FILES}
    ${LUABRIDGE_TEST_LUA_LIBRARY_FILES}
  )

  target_compile_definitions (${LUABRIDGE_TEST_NAME} PRIVATE
    ${LUABRIDGE_DEFINES})

  if (LUABRIDGE_COVERAGE)
    setup_target_for_coverage (${LUABRIDGE_TEST_NAME} ${SOURCE_LOCATION} LuaBridge)
  endif ()

  target_include_directories (${LUABRIDGE_TEST_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${SOURCE_LOCATION}
    Source)

  if (${LUA_VERSION} STREQUAL "LUAU")
    target_include_directories (${LUABRIDGE_TEST_NAME} PRIVATE "${LUABRIDGE_LUAU_LOCATION}/VM/include")
    target_include_directories (${LUABRIDGE_TEST_NAME} PRIVATE "${LUABRIDGE_LUAU_LOCATION}/Ast/include")
    target_include_directories (${LUABRIDGE_TEST_NAME} PRIVATE "${LUABRIDGE_LUAU_LOCATION}/Compiler/include")
    target_include_directories (${LUABRIDGE_TEST_NAME} PRIVATE "${LUABRIDGE_LUAU_LOCATION}/Common/include")
    target_compile_definitions (${LUABRIDGE_TEST_NAME} PRIVATE LUABRIDGEDEMO_LUAU=1)

    target_include_directories (${LUABRIDGE_TESTLIB_NAME} PRIVATE "${LUABRIDGE_LUAU_LOCATION}/VM/include")
    target_include_directories (${LUABRIDGE_TESTLIB_NAME} PRIVATE "${LUABRIDGE_LUAU_LOCATION}/Ast/include")
    target_include_directories (${LUABRIDGE_TESTLIB_NAME} PRIVATE "${LUABRIDGE_LUAU_LOCATION}/Compiler/include")
    target_include_directories (${LUABRIDGE_TESTLIB_NAME} PRIVATE "${LUABRIDGE_LUAU_LOCATION}/Common/include")
    target_compile_definitions (${LUABRIDGE_TESTLIB_NAME} PRIVATE LUABRIDGEDEMO_LUAU=1)
  elseif (${LUA_VERSION} STREQUAL "LUAJIT")
    target_compile_definitions (${LUABRIDGE_TEST_NAME} PRIVATE LUABRIDGEDEMO_LUAJIT=1)
    target_compile_definitions (${LUABRIDGE_TESTLIB_NAME} PRIVATE LUABRIDGEDEMO_LUAJIT=1)
  elseif (${LUA_VERSION} STREQUAL "RAVI")
    target_compile_definitions (${LUABRIDGE_TEST_NAME} PRIVATE LUABRIDGEDEMO_RAVI=1)
    target_compile_definitions (${LUABRIDGE_TESTLIB_NAME} PRIVATE LUABRIDGEDEMO_RAVI=1)
  else () # if(${LUA_VERSION} MATCHES "^[0-9]*")
    target_compile_definitions (${LUABRIDGE_TEST_NAME} PRIVATE LUABRIDGEDEMO_LUA_VERSION=${LUA_VERSION})
    target_compile_definitions (${LUABRIDGE_TESTLIB_NAME} PRIVATE LUABRIDGEDEMO_LUA_VERSION=${LUA_VERSION})
  endif ()

  if (WIN32)
    set(LUABRIDGEDEMO_SHARED_LIBRARY "${LUABRIDGE_TESTLIB_NAME}.dll")
  elseif (APPLE)
    set(LUABRIDGEDEMO_SHARED_LIBRARY "lib${LUABRIDGE_TESTLIB_NAME}.dylib")
  else ()
    set(LUABRIDGEDEMO_SHARED_LIBRARY "lib${LUABRIDGE_TESTLIB_NAME}.so")
  endif ()
  target_compile_definitions (${LUABRIDGE_TEST_NAME} PRIVATE
    LUABRIDGEDEMO_SHARED_LIBRARY="${LUABRIDGEDEMO_SHARED_LIBRARY}"
    LUABRIDGEDEMO_SHARED_EXPORT=0)

  if (NOT ${LUABRIDGE_EXCEPTIONS})
    target_compile_definitions (${LUABRIDGE_TEST_NAME} PRIVATE LUA_USE_LONGJMP=1)
    if (APPLE)
      target_compile_options (${LUABRIDGE_TEST_NAME} PRIVATE -fno-exceptions)
      set_target_properties (${LUABRIDGE_TEST_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_ENABLE_CPP_EXCEPTIONS "NO")
    elseif (WIN32)
      target_compile_options (${LUABRIDGE_TEST_NAME} PRIVATE /EHs-c-)
      target_compile_definitions (${LUABRIDGE_TEST_NAME} PRIVATE _HAS_EXCEPTIONS=0)
    else ()
      target_compile_options (${LUABRIDGE_TEST_NAME} PRIVATE -fno-exceptions)
    endif ()
  endif ()

  target_link_libraries (${LUABRIDGE_TEST_NAME} PRIVATE LuaBridge gtest ${CMAKE_DL_LIBS})
  add_dependencies (${LUABRIDGE_TEST_NAME} ${LUABRIDGE_TESTLIB_NAME})

  if ("${LUABRIDGE_LIBS}" STREQUAL "")
  else ()
    target_link_libraries (${LUABRIDGE_TEST_NAME} PRIVATE ${LUABRIDGE_LIBS})
    target_link_libraries (${LUABRIDGE_TESTLIB_NAME} PRIVATE ${LUABRIDGE_LIBS})
  endif ()

  add_test(NAME ${LUABRIDGE_TEST_NAME} COMMAND ${LUABRIDGE_TEST_NAME})

endmacro (add_test_app)

# ====================================================== Real Unit Tests

add_test_app (LuaBridgeTests51 501 "${LUABRIDGE_TEST_LUA51_FILES}" 1 "" "")
add_test_app (LuaBridgeTests51LuaC 501 "${LUABRIDGE_TEST_LUA51_C_FILES}" 1 "${LUABRIDGE_LUA_C_DEFINES}" "")
add_test_app (LuaBridgeTests51Noexcept 501 "${LUABRIDGE_TEST_LUA51_FILES}" 0 "" "")
add_test_app (LuaBridgeTests51LuaCNoexcept 501 "${LUABRIDGE_TEST_LUA51_C_FILES}" 0 "${LUABRIDGE_LUA_C_DEFINES}" "")

add_test_app (LuaBridgeTests52 502 "${LUABRIDGE_TEST_LUA52_FILES}" 1 "" "")
add_test_app (LuaBridgeTests52LuaC 502 "${LUABRIDGE_TEST_LUA52_C_FILES}" 1 "${LUABRIDGE_LUA_C_DEFINES}" "")
add_test_app (LuaBridgeTests52Noexcept 502 "${LUABRIDGE_TEST_LUA52_FILES}" 0 "" "")
add_test_app (LuaBridgeTests52LuaCNoexcept 502 "${LUABRIDGE_TEST_LUA52_C_FILES}" 0 "${LUABRIDGE_LUA_C_DEFINES}" "")

add_test_app (LuaBridgeTests53 503 "${LUABRIDGE_TEST_LUA53_FILES}" 1 "" "")
add_test_app (LuaBridgeTests53LuaC 503 "${LUABRIDGE_TEST_LUA53_C_FILES}" 1 "${LUABRIDGE_LUA_C_DEFINES}" "")
add_test_app (LuaBridgeTests53Noexcept 503 "${LUABRIDGE_TEST_LUA53_FILES}" 0 "" "")
add_test_app (LuaBridgeTests53LuaCNoexcept 503 "${LUABRIDGE_TEST_LUA53_C_FILES}" 0 "${LUABRIDGE_LUA_C_DEFINES}" "")

add_test_app (LuaBridgeTests54 504 "${LUABRIDGE_TEST_LUA54_FILES}" 1 "" "")
add_test_app (LuaBridgeTests54LuaC 504 "${LUABRIDGE_TEST_LUA54_C_FILES}" 1 "${LUABRIDGE_LUA_C_DEFINES}" "")
add_test_app (LuaBridgeTests54Noexcept 504 "${LUABRIDGE_TEST_LUA54_FILES}" 0 "" "")
add_test_app (LuaBridgeTests54LuaCNoexcept 504 "${LUABRIDGE_TEST_LUA54_C_FILES}" 0 "${LUABRIDGE_LUA_C_DEFINES}" "")

add_test_app (LuaBridgeTestsLuaJIT "LUAJIT" "${LUABRIDGE_TEST_LUAJIT_FILES}" 1 "" "liblua-static")
add_test_app (LuaBridgeTestsLuaJITNoexcept "LUAJIT" "${LUABRIDGE_TEST_LUAJIT_FILES}" 0 "" "liblua-static")

add_test_app (LuaBridgeTestsLuau "LUAU" "${LUABRIDGE_TEST_LUAU_FILES}" 1 "" "")
#add_test_app (LuaBridgeTestsLuauNoexcept "LUAU" "${LUABRIDGE_TEST_LUAU_FILES}" 0 "" "")

add_test_app (LuaBridgeTestsRavi "RAVI" "${LUABRIDGE_TEST_RAVI_FILES}" 1 "${LUABRIDGE_LUA_C_DEFINES}" "libravi")
#add_test_app (LuaBridgeTestsRaviNoexcept "RAVI" "${LUABRIDGE_TEST_RAVI_FILES}" 0 "" "libravi")

if (LUABRIDGE_COVERAGE)
  setup_coverage_single_target ()
endif ()
