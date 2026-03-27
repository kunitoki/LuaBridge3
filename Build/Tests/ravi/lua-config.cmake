find_path(LUA_INCLUDE_DIR lua.h
    PATHS
    /usr/local/include/ravi
  )

find_library(LUA_LIBRARIES
    NAMES libravi libravi.so libravi.dylib
    PATHS
    /usr/local/lib
  )

find_program(LUA_EXE
    NAMES ravi_s
    PATHS
    /usr/local/bin
  )

# LUA_INCDIR - place where lua headers exist
set(LUA_INCDIR ${LUA_INCLUDE_DIR})

# LIBDIR - LUA_CPATH
if (WIN32)

  get_filename_component(LIBDIR
    ${LUA_EXE}
    DIRECTORY)

else()

  get_filename_component(LIBDIR
    ${LUA_LIBRARIES}
    DIRECTORY)

endif()

get_filename_component(LUA_BINDIR
  ${LUA_EXE}
  DIRECTORY)

# LUA_LIBDIR - place where lua native libraries exist
get_filename_component(LUA_LIBDIR
  ${LUA_LIBRARIES}
  DIRECTORY
)

if (NOT WIN32) 
  set(LUA_LIBRARIES "${LUA_LIBRARIES};m")
endif()

# LUALIB - the lua library to link against
set(LUALIB ${LUA_LIBRARIES})

# LUADIR - LUA_PATH
if (USE_LUA53)
  set(LUADIR "${LUA_LIBDIR}/../share/lua/5.3")
else()
  set(LUADIR "${LUA_LIBDIR}/../share/lua/5.3")
endif()

set(LUA "${LUA_EXE}")

