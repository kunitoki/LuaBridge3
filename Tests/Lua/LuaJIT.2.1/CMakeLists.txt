# This CMakeLists.txt has been first taken from LuaDist
# Copyright (C) 2007-2011 LuaDist.
# Created by Peter Drahoš
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# Debugged and (now seriously) modified by Ronan Collobert, for Torch7
# Upgrade to luajit-2.1 by 9chu
# Modified 2022 by kunitoki

CMAKE_MINIMUM_REQUIRED(VERSION 3.6 FATAL_ERROR)

project(Lua C ASM)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# If you want to include Lua in a main project,
# you might want to define those variables yourself
IF(NOT Lua_IS_SUBPROJECT)
  INCLUDE(LuaPaths)
ENDIF()

INCLUDE(CheckLibraryExists)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckCSourceCompiles)
INCLUDE(CheckTypeSize)
INCLUDE(CheckCCompilerFlag)

# LuaJIT specific
option ( LUAJIT_DISABLE_FFI "Disable FFI." OFF )
option ( LUAJIT_ENABLE_LUA52COMPAT "Enable Lua 5.2 compatibility." OFF )
option ( LUAJIT_DISABLE_JIT "Disable JIT." OFF )
option ( LUAJIT_CPU_SSE2 "Use SSE2 instead of x87 instructions." ON )
option ( LUAJIT_CPU_NOCMOV "Disable NOCMOV." OFF )
option ( LUAJIT_DISABLE_GC64 "Disable LJ_GC64 mode for x64." OFF )

# Debug purpose
option ( LUAJIT_USE_SYSMALLOC "Use the system provided memory allocator (realloc)." OFF )
option ( LUAJIT_USE_VALGRIND "This define is required to run LuaJIT under Valgrind." OFF )
option ( LUAJIT_USE_GDBJIT "This is the client for the GDB JIT API." OFF )
option ( LUA_USE_APICHECK "Turn on assertions for the Lua/C API to debug problems with lua_* calls." OFF )
option ( LUA_USE_ASSERT "Turn on assertions for the whole LuaJIT VM." OFF )
MARK_AS_ADVANCED(LUAJIT_DISABLE_FFI LUAJIT_ENABLE_LUA52COMPAT LUAJIT_DISABLE_JIT LUAJIT_CPU_SSE2 LUAJIT_CPU_NOCMOV LUAJIT_RUN_DYNASM LUAJIT_DISABLE_GC64
  LUAJIT_USE_SYSMALLOC LUAJIT_USE_VALGRIND LUAJIT_USE_GDBJIT LUA_USE_APICHECK LUA_USE_ASSERT )

CHECK_TYPE_SIZE("void*" SIZEOF_VOID_P)
IF(SIZEOF_VOID_P EQUAL 8)
  ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE)
ENDIF()

IF(NOT WIN32)
  FIND_LIBRARY(DL_LIBRARY "dl")
  IF(DL_LIBRARY)
    SET(CMAKE_REQUIRED_LIBRARIES ${DL_LIBRARY})
    LIST(APPEND LIBS ${DL_LIBRARY})
  ENDIF(DL_LIBRARY)
  CHECK_FUNCTION_EXISTS(dlopen LUA_USE_DLOPEN)
  IF(NOT LUA_USE_DLOPEN)
    MESSAGE(FATAL_ERROR "Cannot compile a useful lua.
Function dlopen() seems not to be supported on your platform.
Apparently you are not on a Windows platform as well.
So lua has no way to deal with shared libraries!")
  ENDIF(NOT LUA_USE_DLOPEN)
ENDIF(NOT WIN32)

CHECK_LIBRARY_EXISTS(m sin "" LUA_USE_LIBM)
if ( LUA_USE_LIBM )
  list ( APPEND LIBS m )
endif ()

#FIND_PACKAGE(Readline)
#IF(READLINE_FOUND)
#  INCLUDE_DIRECTORIES(${READLINE_INCLUDE_DIR})
#  list(APPEND LIBS ${READLINE_LIBRARIES})
#  SET(LUAJIT_USE_READLINE 1)
#ENDIF(READLINE_FOUND)
SET(LUAJIT_USE_READLINE 0)

IF (CMAKE_SHARED_LIBRARY_SUFFIX STREQUAL CMAKE_SHARED_MODULE_SUFFIX)
  SET(LUA_USE_MODULE_AND_LIBRARY 0)
ELSE (CMAKE_SHARED_LIBRARY_SUFFIX STREQUAL CMAKE_SHARED_MODULE_SUFFIX)
  SET(LUA_USE_MODULE_AND_LIBRARY 1)
ENDIF (CMAKE_SHARED_LIBRARY_SUFFIX STREQUAL CMAKE_SHARED_MODULE_SUFFIX)

## SOURCES
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/luaconf.h src/lua.h src/lauxlib.h src/lualib.h src/lua.hpp src/luajit.h
  DESTINATION "${Lua_INSTALL_INCLUDE_SUBDIR}")

MACRO(LJ_TEST_ARCH stuff)
  CHECK_C_SOURCE_COMPILES("
#undef ${stuff}
#include \"lj_arch.h\"
#if ${stuff}
int main() { return 0; }
#else
#error \"not defined\"
#endif
" ${stuff})
ENDMACRO()

MACRO(LJ_TEST_ARCH_VALUE stuff value)
  CHECK_C_SOURCE_COMPILES("
#undef ${stuff}
#include \"lj_arch.h\"
#if ${stuff} == ${value}
int main() { return 0; }
#else
#error \"not defined\"
#endif
" ${stuff}_${value})
ENDMACRO()

SET(CMAKE_REQUIRED_INCLUDES ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
FOREACH(arch X64 X86 ARM ARM64 PPC PPCSPE MIPS MIPS64 S390X)
  LJ_TEST_ARCH(LJ_TARGET_${arch})
  if(LJ_TARGET_${arch})
    STRING(TOLOWER ${arch} TARGET_LJARCH)
    MESSAGE(STATUS "LuaJIT Target ${TARGET_LJARCH}")
    BREAK()
  ENDIF()
ENDFOREACH()

IF(NOT TARGET_LJARCH)
  MESSAGE(FATAL_ERROR "architecture not supported")
ENDIF()

SET(DASM_ARCH ${TARGET_LJARCH})
SET(DASM_FLAGS)
SET(TARGET_ARCH)
LIST(APPEND TARGET_ARCH "LUAJIT_TARGET=LUAJIT_ARCH_${TARGET_LJARCH}")

LJ_TEST_ARCH(LJ_TARGET_ARM64)
IF(LJ_TARGET_ARM64)
  LJ_TEST_ARCH(__AARCH64EB__)
  IF(__AARCH64EB__)
    LIST(APPEND TARGET_ARCH "__AARCH64EB__=1")
  ENDIF()
ENDIF()

LJ_TEST_ARCH(LJ_TARGET_PPC)
IF(LJ_TARGET_PPC)
  LJ_TEST_ARCH_VALUE(LJ_LE 1)
  IF(LJ_LE_1)
    LIST(APPEND TARGET_ARCH "LJ_ARCH_ENDIAN=LUAJIT_LE")
  ELSE()
    LIST(APPEND TARGET_ARCH "LJ_ARCH_ENDIAN=LUAJIT_BE")
  ENDIF()
ENDIF()

LJ_TEST_ARCH(LJ_TARGET_MIPS)
IF(LJ_TARGET_MIPS)
  LJ_TEST_ARCH(MIPSEL)
  IF(NOT MIPSEL)
    LIST(APPEND TARGET_ARCH "__MIPSEL__=1")
  ENDIF()
ENDIF()

LJ_TEST_ARCH(LJ_TARGET_PS3)
IF(LJ_TARGET_PS3)
  LIST(APPEND TARGET_ARCH "__CELLOS_LV2__=1")
  set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLUAJIT_USE_SYSMALLOC" )
ENDIF()

LJ_TEST_ARCH_VALUE(LJ_LE 1)
IF(LJ_LE_1)
  SET(DASM_FLAGS ${DASM_FLAGS} -D ENDIAN_LE)
ELSE()
  SET(DASM_FLAGS ${DASM_FLAGS} -D ENDIAN_BE)
ENDIF()
LJ_TEST_ARCH_VALUE(LJ_ARCH_BITS 64)
IF(LJ_ARCH_BITS_64)
  SET(DASM_FLAGS ${DASM_FLAGS} -D P64)
ENDIF()
LJ_TEST_ARCH_VALUE(LJ_HASJIT 1)
IF(LJ_HASJIT_1)
  SET(DASM_FLAGS ${DASM_FLAGS} -D JIT)
ENDIF()
LJ_TEST_ARCH_VALUE(LJ_HASFFI 1)
IF(LJ_HASFFI_1)
  SET(DASM_FLAGS ${DASM_FLAGS} -D FFI)
ENDIF()
LJ_TEST_ARCH_VALUE(LJ_DUALNUM 1)
IF(LJ_DUALNUM_1)
  SET(DASM_FLAGS ${DASM_FLAGS} -D DUALNUM)
ENDIF()
LJ_TEST_ARCH_VALUE(LJ_ARCH_HASFPU 1)
IF(LJ_ARCH_HASFPU_1)
  SET(DASM_FLAGS ${DASM_FLAGS} -D HASFPU)
  LIST(APPEND TARGET_ARCH "LJ_ARCH_HASFPU=1")
ELSE()
  LIST(APPEND TARGET_ARCH "LJ_ARCH_HASFPU=0")
ENDIF()
LJ_TEST_ARCH_VALUE(LJ_ABI_SOFTFP 1)
IF(NOT LJ_ABI_SOFTFP_1)
  SET(DASM_FLAGS ${DASM_FLAGS} -D HFABI)
  LIST(APPEND TARGET_ARCH "LJ_ABI_SOFTFP=0")
ELSE()
  LIST(APPEND TARGET_ARCH "LJ_ABI_SOFTFP=1")
ENDIF()
LJ_TEST_ARCH_VALUE(LJ_NO_UNWIND 1)
IF(LJ_NO_UNWIND_1)
  SET(DASM_FLAGS ${DASM_FLAGS} -D NO_UNWIND)
  LIST(APPEND TARGET_ARCH "LUAJIT_NO_UNWIND=1")
ENDIF()
IF(WIN32)
  SET(DASM_FLAGS ${DASM_FLAGS} -D WIN)
ENDIF()

#IF(TARGET_LJARCH STREQUAL "x86")
#  LJ_TEST_ARCH_VALUE(__SSE2__ 1)
#  IF(__SSE2__1)
#    SET(DASM_FLAGS ${DASM_FLAGS} -D SSE)
#  ENDIF()
#ENDIF()
IF(TARGET_LJARCH STREQUAL "x64")
  LJ_TEST_ARCH_VALUE(LJ_FR2 1)
  IF (NOT LJ_FR2_1)
    SET(DASM_ARCH "x86")
  ENDIF ()
ENDIF()
IF(TARGET_LJARCH STREQUAL "arm")
  if(${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    SET(DASM_AFLAGS ${DASM_AFLAGS} -D IOS)
  ENDIF()
ENDIF()
LJ_TEST_ARCH_VALUE(LJ_TARGET_MIPSR6 1)
IF(LJ_TARGET_MIPSR6_1)
  SET(DASM_AFLAGS ${DASM_AFLAGS} -D MIPSR6)
ENDIF()
IF(TARGET_LJARCH STREQUAL "ppc")
  LJ_TEST_ARCH_VALUE(LJ_ARCH_SQRT 1)
  IF(LJ_ARCH_SQRT_1)
    SET(DASM_FLAGS ${DASM_FLAGS} -D SQRT)
  ENDIF()
  LJ_TEST_ARCH_VALUE(LJ_ARCH_ROUND 1)
  IF(LJ_ARCH_ROUND_1)
    SET(DASM_FLAGS ${DASM_FLAGS} -D ROUND)
  ENDIF()
  LJ_TEST_ARCH_VALUE(LJ_ARCH_PPC32ON64 1)
  IF(LJ_ARCH_PPC32ON64_1)
    SET(DASM_FLAGS ${DASM_FLAGS} -D GPR64)
  ENDIF()
  if(LJ_TARGET_PS3)
    SET(DASM_FLAGS ${DASM_FLAGS} -D PPE)
  ENDIF()
  LJ_TEST_ARCH_VALUE(LJ_ARCH_PPC_OPD 1)
  IF(LJ_ARCH_PPC_OPD_1)
    SET(DASM_FLAGS ${DASM_FLAGS} -D OPD)
  ENDIF()
  LJ_TEST_ARCH_VALUE(LJ_ARCH_PPC_OPDENV 1)
  IF(LJ_ARCH_PPC_OPDENV_1)
    SET(DASM_FLAGS ${DASM_FLAGS} -D OPDENV)
  ENDIF()
  LJ_TEST_ARCH_VALUE(LJ_ARCH_PPC_ELFV2 1)
  IF(LJ_ARCH_PPC_ELFV2_1)
    SET(DASM_FLAGS ${DASM_FLAGS} -D ELFV2)
  ENDIF()
ENDIF()
IF(TARGET_LJARCH STREQUAL "s390x")  # untested
  SET(DASM_ARCH "s390x")
ENDIF()

check_c_compiler_flag("-fno-stack-protector" HAS_NO_STACK_PROTECTOR)
IF(HAS_NO_STACK_PROTECTOR)
  set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-stack-protector")
ENDIF()

check_c_compiler_flag("-fomit-frame-pointer" HAS_OMIT_FRAME_POINTER)
IF(HAS_NO_STACK_PROTECTOR)
  set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fomit-frame-pointer")
ENDIF()

check_c_compiler_flag("-fPIC" HAS_POSITION_INDEPENDENT_CODE)
IF(HAS_POSITION_INDEPENDENT_CODE)
  set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
ENDIF()

if ( WIN32 )
  add_definitions ( -DLUAJIT_OS=LUAJIT_OS_WINDOWS )
  IF(NOT MSVC)
    set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -malign-double" )
  ENDIF()
  set ( LJVM_MODE peobj )
elseif ( APPLE )
  set ( LJVM_MODE machasm )
  add_definitions ( -DLUAJIT_OS=LUAJIT_OS_OSX )
  if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    add_definitions ( -DLUAJIT_UNWIND_EXTERNAL )
  endif ()
  if ( ${CMAKE_SYSTEM_NAME} MATCHES "iOS" )  # untested
    add_definitions ( -DTARGET_OS_IPHONE=1 )
    IF (TARGET_LJARCH STREQUAL "arm64")
      set ( CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-fno-omit-frame-pointer" )
    endif ()
  else ()
    add_definitions ( -DTARGET_OS_IPHONE=0 )
  endif ()
else ()
  if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_definitions ( -DLUAJIT_OS=LUAJIT_OS_LINUX )
  else()
    add_definitions ( -DLUAJIT_OS=LUAJIT_OS_OTHER )
  endif ()

  if (${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
    set ( LJVM_MODE static )
  elseif (LJ_TARGET_PS3)
    set ( LJVM_MODE static )
  else()
    set ( LJVM_MODE elfasm )
  endif()

  if (NOT LJ_NO_UNWIND_1)
    # Find out whether the target toolchain always generates unwind tables.
    execute_process(COMMAND bash "-c"
      "exec 2>/dev/null; echo 'extern void b(void);int a(void){b();return 0;}' | ${CMAKE_C_COMPILER} -c -x c - -o tmpunwind.o && { grep -qa -e eh_frame -e __unwind_info tmpunwind.o || grep -qU -e eh_frame -e __unwind_info tmpunwind.o; } && echo E"
      OUTPUT_VARIABLE TARGET_TESTUNWIND)
    IF (${TARGET_TESTUNWIND} MATCHES "E")
      message(STATUS "Use -DLUAJIT_UNWIND_EXTERNAL")
      add_definitions ( -DLUAJIT_UNWIND_EXTERNAL )
    endif ()
  endif ()
endif ()

# lj_str_hash requirement
LJ_TEST_ARCH_VALUE(LJ_HAS_OPTIMISED_HASH 1)
IF (TARGET_LJARCH STREQUAL "x64" AND LJ_HAS_OPTIMISED_HASH_1)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4.2")
endif ()

add_executable(minilua src/host/minilua.c)
SET_TARGET_PROPERTIES(minilua PROPERTIES COMPILE_DEFINITIONS "${TARGET_ARCH}")
CHECK_LIBRARY_EXISTS(m pow "" MINILUA_USE_LIBM)
if(MINILUA_USE_LIBM)
  message(STATUS "Use libm to build minilua")
  TARGET_LINK_LIBRARIES(minilua m)
endif()

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/buildvm_arch.h
  COMMAND minilua ${CMAKE_CURRENT_SOURCE_DIR}/dynasm/dynasm.lua ${DASM_FLAGS} -o ${CMAKE_CURRENT_BINARY_DIR}/buildvm_arch.h ${CMAKE_CURRENT_SOURCE_DIR}/src/vm_${DASM_ARCH}.dasc
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/dynasm/dynasm.lua minilua
)

SET(SRC_LJLIB src/lib_base.c src/lib_math.c src/lib_bit.c src/lib_string.c src/lib_table.c
  src/lib_io.c src/lib_os.c src/lib_package.c src/lib_debug.c src/lib_jit.c src/lib_ffi.c src/lib_buffer.c)

SET(SRC_LJCORE src/lj_gc.c src/lj_err.c src/lj_char.c src/lj_bc.c src/lj_obj.c src/lj_buf.c
  src/lj_str.c src/lj_tab.c src/lj_func.c src/lj_udata.c src/lj_meta.c src/lj_debug.c
  src/lj_state.c src/lj_dispatch.c src/lj_vmevent.c src/lj_vmmath.c src/lj_strscan.c src/lj_strfmt.c src/lj_strfmt_num.c
  src/lj_api.c src/lj_profile.c src/lj_lex.c src/lj_parse.c src/lj_bcread.c src/lj_bcwrite.c src/lj_load.c
  src/lj_ir.c src/lj_opt_mem.c src/lj_opt_fold.c src/lj_opt_narrow.c
  src/lj_opt_dce.c src/lj_opt_loop.c src/lj_opt_split.c src/lj_opt_sink.c
  src/lj_mcode.c src/lj_snap.c src/lj_record.c src/lj_crecord.c src/lj_ffrecord.c
  src/lj_asm.c src/lj_trace.c src/lj_gdbjit.c
  src/lj_ctype.c src/lj_cdata.c src/lj_cconv.c src/lj_ccall.c src/lj_ccallback.c
  src/lj_carith.c src/lj_clib.c src/lj_cparse.c
  src/lj_lib.c src/lj_alloc.c src/lib_aux.c
  src/lj_prng.c src/lj_serialize.c
  ${SRC_LJLIB} src/lib_init.c)

SET(SRC_BUILDVM src/host/buildvm.c src/host/buildvm_asm.c
src/host/buildvm_peobj.c src/host/buildvm_lib.c src/host/buildvm_fold.c
${CMAKE_CURRENT_BINARY_DIR}/buildvm_arch.h)

## GENERATE
ADD_EXECUTABLE(buildvm ${SRC_BUILDVM})
SET_TARGET_PROPERTIES(buildvm PROPERTIES COMPILE_DEFINITIONS "${TARGET_ARCH}")

macro(add_buildvm_target _target _mode)
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_target}
    COMMAND buildvm ARGS -m ${_mode} -o ${CMAKE_CURRENT_BINARY_DIR}/${_target} ${ARGN}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS buildvm ${ARGN}
  )
endmacro(add_buildvm_target)

if (MSVC)
  add_buildvm_target ( lj_vm.obj peobj )
  set (LJ_VM_SRC ${CMAKE_CURRENT_BINARY_DIR}/lj_vm.obj)
else ()
  add_buildvm_target ( lj_vm.s ${LJVM_MODE} )
  set (LJ_VM_SRC ${CMAKE_CURRENT_BINARY_DIR}/lj_vm.s)
endif ()
add_buildvm_target ( lj_ffdef.h   ffdef   ${SRC_LJLIB} )
add_buildvm_target ( lj_bcdef.h  bcdef  ${SRC_LJLIB} )
add_buildvm_target ( lj_folddef.h folddef src/lj_opt_fold.c )
add_buildvm_target ( lj_recdef.h  recdef  ${SRC_LJLIB} )
add_buildvm_target ( lj_libdef.h  libdef  ${SRC_LJLIB} )
add_buildvm_target ( vmdef.lua  vmdef  ${SRC_LJLIB} )

SET(DEPS
  ${LJ_VM_SRC}
  ${CMAKE_CURRENT_BINARY_DIR}/lj_ffdef.h
  ${CMAKE_CURRENT_BINARY_DIR}/lj_bcdef.h
  ${CMAKE_CURRENT_BINARY_DIR}/lj_libdef.h
  ${CMAKE_CURRENT_BINARY_DIR}/lj_recdef.h
  ${CMAKE_CURRENT_BINARY_DIR}/lj_folddef.h
  ${CMAKE_CURRENT_BINARY_DIR}/vmdef.lua
  )

## COMPILE
include_directories (BEFORE ${CMAKE_CURRENT_BINARY_DIR} dynasm src )

#add_library ( liblua-shared SHARED ${SRC_LJCORE} ${DEPS} )
#target_link_libraries ( liblua-shared ${LIBS} )
#target_include_directories( liblua-shared INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src )
#SET_TARGET_PROPERTIES(liblua-shared PROPERTIES
#  PREFIX "lib" IMPORT_PREFIX "lib" OUTPUT_NAME "lua")
#if ( WIN32 AND NOT CYGWIN )
#  target_compile_definitions ( liblua-shared PRIVATE -DLUA_BUILD_AS_DLL )
#endif ()

add_library ( liblua-static STATIC ${SRC_LJCORE} ${DEPS} )
target_link_libraries ( liblua-static ${LIBS} )
target_include_directories( liblua-static INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src )
SET_TARGET_PROPERTIES(liblua-static PROPERTIES
  COMPILE_DEFINITIONS "liblua_STATIC"
  OUTPUT_NAME "lua-static")

#add_executable ( lua src/luajit.c)
#target_link_libraries ( lua liblua-shared )

# add_executable ( lua-static src/luajit.c)
# target_link_libraries ( lua-static liblua-static )

#INSTALL(TARGETS liblua-shared lua
#  EXPORT torch-exports
#  RUNTIME DESTINATION "${Lua_INSTALL_BIN_SUBDIR}"
#  LIBRARY DESTINATION "${Lua_INSTALL_LIB_SUBDIR}"
#  ARCHIVE DESTINATION "${Lua_INSTALL_LIB_SUBDIR}")

# Create internal FindLua.cmake
#SET(LUA_LIBRARIES liblua-shared)
SET(LUA_EXECUTABLE lua-static)
SET(LUAC_EXECUTABLE torch-luac)
SET(LUA_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_BINARY_DIR}
  ${READLINE_INCLUDE_DIR})
#CONFIGURE_FILE(cmake/LuaConfig.cmake.in "${Lua_INSTALL_FINDLUA_DIR}/FindLua.cmake")

INSTALL(FILES
  src/jit/bc.lua src/jit/v.lua src/jit/dump.lua src/jit/dis_x86.lua src/jit/dis_x64.lua src/jit/dis_arm.lua
  src/jit/dis_ppc.lua src/jit/dis_mips.lua src/jit/dis_mipsel.lua src/jit/bcsave.lua ${CMAKE_CURRENT_BINARY_DIR}/vmdef.lua
  src/jit/p.lua src/jit/zone.lua src/jit/dis_arm64.lua src/jit/dis_arm64be.lua src/jit/dis_mips64.lua src/jit/dis_mips64el.lua
  DESTINATION "${Lua_INSTALL_LUA_PATH_SUBDIR}/jit")
