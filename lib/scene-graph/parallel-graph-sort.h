#ifndef LIB_SCENE_GRAPH_PARALLEL_GRAPH_SORT_H_
#define LIB_SCENE_GRAPH_PARALLEL_GRAPH_SORT_H_

typedef struct SceneGraph SceneGraph;

typedef struct ThreadPool ThreadPool;

void scene_graph_ysort_parallel(SceneGraph* graph, ThreadPool* pool);

#endif  //  LIB_SCENE_GRAPH_PARALLEL_GRAPH_SORT_H_