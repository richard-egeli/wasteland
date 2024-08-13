cmake_minimum_required(VERSION 3.20)

# Add the ASan flags
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
endif()

message(STATUS "BUILD: ${CMAKE_BUILD_TYPE}")
if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    message(STATUS "Compiling Release Build")
    add_definitions(-DNDEBUG)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -march=native -flto -ffast-math")
endif()

project(wasteland)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED YES)
set(CMAKE_C_EXTENSIONS NO)

find_package(raylib QUIET)
if (NOT raylib_FOUND)
  include(FetchContent)
  FetchContent_Declare(
      raylib
      GIT_REPOSITORY https://github.com/raysan5/raylib.git
      GIT_TAG master
  )

  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
      set(FETCHCONTENT_QUIET NO)
      FetchContent_Populate(raylib)
      set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
      add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
  endif()

  FetchContent_MakeAvailable(raylib)
endif()

find_package(Lua QUIET)
if (NOT Lua_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    Lua
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://lua.org/ftp/lua-5.4.7.tar.gz
  )

  FetchContent_MakeAvailable(lua)
endif()

find_package(enet QUIET)
if(NOT enet_FOUND)
  include(FetchContent)
  FetchContent_Declare(
      enet
      GIT_REPOSITORY https://github.com/lsalzman/enet.git
      GIT_TAG master
    )

  FetchContent_MakeAvailable(enet)
endif()

set(LUAJIT_LIBRARIES /usr/local/lib/libluajit-5.1.a)
set(LUAJIT_INCLUDE_DIR /usr/local/include/luajit-2.1)

add_subdirectory(lib/collision)
add_subdirectory(lib/hashmap)
add_subdirectory(lib/array)
add_subdirectory(lib/ldtk)
add_subdirectory(lib/ui)

message(STATUS ${LUA_INCLUDE_DIR})

add_executable(${PROJECT_NAME} 
  core/main.c 
  core/src/level.c 
  core/src/global.c 
  core/src/action.c 
  core/src/entity.c
  core/src/texture.c
  core/src/tilemap.c
  core/src/lua_funcs.c 
  core/src/lua_entity.c
  core/src/lua_level.c
  core/src/lua_helper.c
)

message(STATUS ${enet_SOURCE_DIR})

target_include_directories(${PROJECT_NAME} PUBLIC 
  core/include 
  ${LUAJIT_INCLUDE_DIR}
  ${enet_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} 
  ${LUAJIT_LIBRARIES}
  raylib 
  collision 
  hashmap 
  array 
  ldtk 
  enet
  ui
)



if (${BUILD_UNIT_TESTS})
    enable_testing()
    add_subdirectory(tests)
endif()

# Checks if OSX and links appropriate frameworks (Only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()
