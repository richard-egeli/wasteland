add_library(collision box_collider.c polygon_collider.c sparse_grid.c)

# find_package(box2d QUIET)
# if(NOT box2d_FOUND)
#   include(FetchContent)
#
#   FetchContent_Declare(
#       box2d
#       GIT_REPOSITORY https://github.com/erincatto/box2c.git
#       GIT_TAG main
#   )
#
#   FetchContent_MakeAvailable(box2d)
# endif()

target_include_directories(collision PUBLIC ..)
target_link_libraries(collision PUBLIC hashmap)
