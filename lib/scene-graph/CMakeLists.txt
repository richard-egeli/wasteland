

add_library(scene-graph scene-graph.c graph-sort.c parallel-graph-sort.c)

target_link_libraries(scene-graph thread-pool thpool)

target_include_directories(scene-graph PUBLIC ..)
