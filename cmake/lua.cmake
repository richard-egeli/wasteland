include(ExternalProject)

# Define where LuaJIT will be downloaded and built
set(LUAJIT_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/luajit)
set(LUAJIT_INSTALL_DIR ${LUAJIT_PREFIX}/install)

# Set up platform-specific variables
if(WIN32)
  set(LUAJIT_LIBRARY_NAME lua51)
  if(MSVC)
    # Different commands for Visual Studio
    set(LUAJIT_BUILD_COMMAND msvcbuild.bat)
    set(LUAJIT_INSTALL_COMMAND 
      ${CMAKE_COMMAND} -E make_directory ${LUAJIT_INSTALL_DIR}/include/luajit-2.1 &&
      ${CMAKE_COMMAND} -E make_directory ${LUAJIT_INSTALL_DIR}/lib &&
      ${CMAKE_COMMAND} -E copy src/lua51.lib ${LUAJIT_INSTALL_DIR}/lib/ &&
      ${CMAKE_COMMAND} -E copy src/lua51.dll ${LUAJIT_INSTALL_DIR}/lib/ &&
      ${CMAKE_COMMAND} -E copy src/lua.h ${LUAJIT_INSTALL_DIR}/include/luajit-2.1/ &&
      ${CMAKE_COMMAND} -E copy src/luaconf.h ${LUAJIT_INSTALL_DIR}/include/luajit-2.1/ &&
      ${CMAKE_COMMAND} -E copy src/lualib.h ${LUAJIT_INSTALL_DIR}/include/luajit-2.1/ &&
      ${CMAKE_COMMAND} -E copy src/lauxlib.h ${LUAJIT_INSTALL_DIR}/include/luajit-2.1/ &&
      ${CMAKE_COMMAND} -E copy src/luajit.h ${LUAJIT_INSTALL_DIR}/include/luajit-2.1/
    )
  else()
    # MinGW or other Windows compiler
    set(LUAJIT_BUILD_COMMAND make)
    set(LUAJIT_INSTALL_COMMAND make install PREFIX=${LUAJIT_INSTALL_DIR})
  endif()
  set(LUAJIT_LIBRARY ${LUAJIT_INSTALL_DIR}/lib/${LUAJIT_LIBRARY_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX})
else()
  # macOS or Linux
  set(LUAJIT_LIBRARY_NAME luajit-5.1)
  set(LUAJIT_BUILD_COMMAND make)
  set(LUAJIT_INSTALL_COMMAND make install PREFIX=${LUAJIT_INSTALL_DIR})
  
  # Extra flags for macOS
  if(APPLE)
    set(LUAJIT_BUILD_COMMAND make MACOSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
    set(LUAJIT_INSTALL_COMMAND make install PREFIX=${LUAJIT_INSTALL_DIR} MACOSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
  endif()
  
  set(LUAJIT_LIBRARY ${LUAJIT_INSTALL_DIR}/lib/lib${LUAJIT_LIBRARY_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX})
endif()

# Set common include directory
set(LUAJIT_INCLUDE_DIR ${LUAJIT_INSTALL_DIR}/include/luajit-2.1)

# Add LuaJIT as an external project
ExternalProject_Add(
  luajit_external
  GIT_REPOSITORY https://github.com/LuaJIT/LuaJIT.git
  GIT_TAG v2.1.0-beta3
  PREFIX ${LUAJIT_PREFIX}
  CONFIGURE_COMMAND ""  # LuaJIT uses make, not configure
  BUILD_IN_SOURCE 1
  BUILD_COMMAND ${LUAJIT_BUILD_COMMAND}
  INSTALL_COMMAND ${LUAJIT_INSTALL_COMMAND}
)

# Create an imported library target
add_library(luajit STATIC IMPORTED GLOBAL)
set_target_properties(luajit PROPERTIES IMPORTED_LOCATION ${LUAJIT_LIBRARY})
add_dependencies(luajit luajit_external)

# Add platform-specific dependencies
if(UNIX AND NOT APPLE)
  # On Linux, LuaJIT requires linking with dl and math libraries
  find_library(LIBDL_LIBRARY NAMES dl)
  find_library(LIBM_LIBRARY NAMES m)
  set(LUAJIT_DEPENDENCIES ${LIBDL_LIBRARY} ${LIBM_LIBRARY})
elseif(APPLE)
  # On macOS, we might need additional framework dependencies
  find_library(FOUNDATION_FRAMEWORK Foundation)
  set(LUAJIT_DEPENDENCIES ${FOUNDATION_FRAMEWORK})
endif()

# Link your project with LuaJIT
target_include_directories(${PROJECT_NAME} PUBLIC ${LUAJIT_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} luajit ${LUAJIT_DEPENDENCIES})

# Add a custom command to ensure the library is built before your project
add_dependencies(${PROJECT_NAME} luajit_external)
