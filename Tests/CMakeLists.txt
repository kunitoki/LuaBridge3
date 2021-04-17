cmake_minimum_required (VERSION 3.5)

# ====================================================== Unit Tests Files

set (LUABRIDGE_TEST_SOURCE_FILES
  Source/AmalgamateTests.cpp
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

# ====================================================== Macro

macro (add_test_app LUABRIDGE_TEST_NAME LUA_VERSION LUABRIDGE_TEST_LUA_LIBRARY_FILES LUABRIDGE_EXCEPTIONS)

  add_executable (${LUABRIDGE_TEST_NAME}
    ${LUABRIDGE_TEST_SOURCE_FILES}
    ${LUABRIDGE_TEST_LUA_LIBRARY_FILES}
  )

  target_include_directories (${LUABRIDGE_TEST_NAME} PRIVATE . Source)
  target_compile_options (${LUABRIDGE_TEST_NAME} PUBLIC -DLUABRIDGEDEMO_LUA_VERSION=${LUA_VERSION})

  if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
  endif (MSVC)

  if (NOT ${LUABRIDGE_EXCEPTIONS})
    target_compile_definitions(${LUABRIDGE_TEST_NAME} PUBLIC LUA_USE_LONGJMP=1)
    if (APPLE)
      target_compile_options (${LUABRIDGE_TEST_NAME} PUBLIC -fno-exceptions)
      set_target_properties (${LUABRIDGE_TEST_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_ENABLE_CPP_EXCEPTIONS "NO")
    elseif (WIN32)
      target_compile_options (${LUABRIDGE_TEST_NAME} PUBLIC /EHs-c-)
      target_compile_definitions(${LUABRIDGE_TEST_NAME} PUBLIC _HAS_EXCEPTIONS=0)
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
add_test_app (LuaBridgeTests51NoExcept 501 "${LUABRIDGE_TEST_LUA51_FILES}" 0)

add_test_app (LuaBridgeTests52 502 "${LUABRIDGE_TEST_LUA52_FILES}" 1)
add_test_app (LuaBridgeTests52Noexcept 502 "${LUABRIDGE_TEST_LUA52_FILES}" 0)

add_test_app (LuaBridgeTests53 503 "${LUABRIDGE_TEST_LUA53_FILES}" 1)
add_test_app (LuaBridgeTests53Noexcept 503 "${LUABRIDGE_TEST_LUA53_FILES}" 0)

add_test_app (LuaBridgeTests54 504 "${LUABRIDGE_TEST_LUA54_FILES}" 1)
add_test_app (LuaBridgeTests54Noexcept 504 "${LUABRIDGE_TEST_LUA54_FILES}" 0)
