add_library(collision box_collider.c sparse_grid.c)

target_include_directories(collision PUBLIC ..)
target_link_libraries(collision PUBLIC hashmap)
