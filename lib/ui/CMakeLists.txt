add_library(ui ui.c)

target_include_directories(ui PUBLIC ..)
target_link_libraries(ui array raylib)
