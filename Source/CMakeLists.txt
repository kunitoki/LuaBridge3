cmake_minimum_required (VERSION 3.10)

set (LUABRIDGE_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/Array.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/Dump.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/List.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/LuaBridge.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/Map.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/Set.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/UnorderedMap.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/Vector.h)
source_group ("LuaBridge" FILES ${LUABRIDGE_HEADERS})

set (LUABRIDGE_DETAIL_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/detail/CFunctions.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/detail/ClassInfo.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/detail/Config.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/detail/Enum.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/detail/Errors.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/detail/Expected.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/detail/FlagSet.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/detail/FuncTraits.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/detail/Globals.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/detail/Invoke.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/detail/Iterator.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/detail/LuaException.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/detail/LuaHelpers.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/detail/LuaRef.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/detail/Namespace.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/detail/Options.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/detail/Overload.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/detail/Result.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/detail/ScopeGuard.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/detail/Stack.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/detail/TypeTraits.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/detail/Userdata.h)
source_group ("LuaBridgeDetail" FILES ${LUABRIDGE_DETAIL_HEADERS})

add_library (LuaBridge INTERFACE)
target_sources (LuaBridge INTERFACE
  ${LUABRIDGE_HEADERS}
  ${LUABRIDGE_DETAIL_HEADERS})
target_include_directories (LuaBridge INTERFACE .)

if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  install(DIRECTORY LuaBridge DESTINATION "LuaBridge3")
endif ()

if (MSVC AND CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  add_custom_target (LuaBridgeSources SOURCES
    ${LUABRIDGE_HEADERS}
    ${LUABRIDGE_DETAIL_HEADERS})
endif ()
