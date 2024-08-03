add_library(collision collision.c spatial_grid.c)

target_include_directories(collision PUBLIC ..)
target_link_libraries(collision PUBLIC hashmap)
