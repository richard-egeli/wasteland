add_library(collision collision.c)

target_include_directories(collision PUBLIC ..)
target_link_libraries(collision PUBLIC hashmap)
