
add_library(ldtk ldtk.c ldtk_entity.c tilemap.c)

find_package(yyjson QUIET)
if(NOT yyjson_FOUND)
  include(FetchContent)
  FetchContent_Declare(
      yyjson 
      GIT_REPOSITORY https://github.com/ibireme/yyjson.git 
      GIT_TAG master
    )
  FetchContent_MakeAvailable(yyjson)
endif()

target_include_directories(ldtk PUBLIC ..)
target_link_libraries(ldtk yyjson hashmap array raylib)
