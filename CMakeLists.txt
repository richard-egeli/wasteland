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

add_subdirectory(lib/collision)
add_subdirectory(lib/hashmap)
add_subdirectory(lib/array)
add_subdirectory(lib/ldtk)
add_subdirectory(lib/ui)
add_subdirectory(lib/thread-pool)
add_subdirectory(lib/scene-graph)
add_subdirectory(lib/thpool)
add_subdirectory(submodules/mpx)

add_executable(${PROJECT_NAME} 
  core/main.c 
  core/src/spritesheet.c
  core/src/assets.c

  # Lua
  core/src/lua/world.c
  core/src/lua/asset_loader.c
  core/src/lua/sprite.c
  core/src/lua/input.c
  core/src/lua/entity.c
  core/src/lua/utils.c
  core/src/lua/dynamic_body.c
  core/src/lua/static_body.c
  core/src/lua/event_handler.c
  core/src/lua/box_collider.c
  core/src/lua/animator.c
)

include(cmake/lua.cmake)
include(cmake/raylib.cmake)
include(cmake/flecs.cmake)
include(cmake/box2d.cmake)
include(cmake/enet.cmake)

target_include_directories(${PROJECT_NAME} PUBLIC 
  core/include 
)

target_link_libraries(${PROJECT_NAME}
  thpool
  scene-graph
  thread-pool
  collision 
  hashmap 
  array 
  mpx
  ldtk 
  ui
)

if (${BUILD_UNIT_TESTS})
    message(STATUS "Building Unit Tests!")
    enable_testing()
    add_subdirectory(tests)
endif()

# Checks if OSX and links appropriate frameworks (Only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()
