

add_library(scene-graph scene-graph.c graph-sort.c parallel-graph-sort.c)

target_include_directories(scene-graph PUBLIC ..)
