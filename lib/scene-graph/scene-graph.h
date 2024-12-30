#ifndef LIB_SCENE_GRAPH_NODE_H_
#define LIB_SCENE_GRAPH_NODE_H_

#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>

#define MAX_NODES 32768
#define NODE_NULL -1
#define NODE_ROOT 0

#define NODE_WORLD 0
#define NODE_LOCAL 1

typedef int Node;

typedef struct SceneGraph SceneGraph;

typedef struct Position {
    float x;
    float y;
} Position;

typedef struct SceneNode {
    int id;
    int parent;
    int first_child;
    int next_sibling;
    int children_count;
    int layer;
    void* userdata;
} SceneNode;

typedef struct GameObject {
    Node node;
    void (*update)(SceneGraph* graph, struct GameObject* object);
    void (*destroy)(SceneGraph* graph, struct GameObject* object);
    void* data;
} GameObject;

typedef struct Drawable {
    Node node;
    void (*draw)(SceneGraph* graph, struct Drawable* renderable);
    void (*destroy)(SceneGraph* graph, struct Drawable* renderable);
    void* data;
} Drawable;

typedef struct __attribute__((aligned(16))) UpdatedSceneNode {
    Node node;
    int type;
    float x;
    float y;
} UpdatedSceneNode;

typedef struct SceneGraph {
    // NOTE: Local & World Transforms
    int nodes_count;
    int node_indices[MAX_NODES];
    int node_next_index;
    SceneNode nodes[MAX_NODES];
    UpdatedSceneNode updated_nodes[MAX_NODES];
    int updated_nodes_count;

    Position local_positions[MAX_NODES];
    Position world_positions[MAX_NODES];

    // NOTE: Game Objects
    GameObject game_objects[MAX_NODES];
    int game_object_indices[MAX_NODES];
    int game_objects_count;

    // NOTE: Drawables
    Drawable drawables[MAX_NODES];
    int drawable_indices[MAX_NODES];
    int drawables_count;

    // NOTE: Destruction Queue
    int nodes_to_destroy[MAX_NODES];
    int nodes_to_destroy_count;
} SceneGraph;

Drawable* scene_graph_drawable_new(SceneGraph* graph, Node node);

GameObject* scene_graph_game_object_new(SceneGraph* graph, Node node);

Node scene_graph_node_new(SceneGraph* graph, Node parent);

void scene_graph_remove_destroyed_nodes(SceneGraph* graph);

void scene_graph_compute_positions(SceneGraph* graph);

void scene_graph_update(SceneGraph* graph);

void scene_graph_render(SceneGraph* graph);

SceneGraph* scene_graph_new(void);

static inline int scene_graph_index_get(const SceneGraph* graph, Node node) {
    assert(node >= 0 && node < MAX_NODES && "Node is out of bounds");
    return graph->node_indices[node];
}

static inline void scene_graph_index_set(SceneGraph* graph, Node node, int index) {
    assert(node >= 0 && node < MAX_NODES && "Node is out of bounds");
    graph->node_indices[node] = index;
}

static inline int scene_graph_node_child_count(const SceneGraph* graph, Node node) {
    int index = scene_graph_index_get(graph, node);
    return graph->nodes[index].children_count;
}

static inline void* scene_graph_userdata_get(SceneGraph* graph, Node node) {
    int index = scene_graph_index_get(graph, node);
    return graph->nodes[index].userdata;
}

static inline void scene_graph_userdata_set(SceneGraph* graph, Node node, void* userdata) {
    int index                    = scene_graph_index_get(graph, node);
    graph->nodes[index].userdata = userdata;
}

static inline int scene_graph_layer_set(SceneGraph* graph, Node node, int layer) {
    int index                        = scene_graph_index_get(graph, node);
    return graph->nodes[index].layer = layer;
}

static inline int scene_graph_layer_get(const SceneGraph* graph, Node node) {
    int index = scene_graph_index_get(graph, node);
    return graph->nodes[index].layer;
}

static inline SceneNode* scene_graph_node_get(SceneGraph* graph, Node node) {
    int index = scene_graph_index_get(graph, node);
    return &graph->nodes[index];
}

static inline Node scene_graph_sibling_get(const SceneGraph* graph, Node node) {
    int index = scene_graph_index_get(graph, node);
    return index != -1 ? graph->nodes[index].next_sibling : -1;
}

static inline void scene_graph_sibling_set(SceneGraph* graph, Node node, Node child) {
    int index                        = scene_graph_index_get(graph, node);
    graph->nodes[index].next_sibling = child;
}

static inline void scene_graph_first_child_set(SceneGraph* graph, Node node, Node child) {
    int index                       = scene_graph_index_get(graph, node);
    graph->nodes[index].first_child = child;
}

static inline Node scene_graph_first_child_get(const SceneGraph* graph, Node node) {
    int index = scene_graph_index_get(graph, node);
    return index != -1 ? graph->nodes[index].first_child : -1;
}

static inline Node scene_graph_parent_get(SceneGraph* graph, Node node) {
    int index = scene_graph_index_get(graph, node);
    return graph->nodes[index].parent;
}

static inline void scene_graph_parent_set(SceneGraph* graph, Node node, Node parent) {
    int index                  = scene_graph_index_get(graph, node);
    graph->nodes[index].parent = parent;
}

static inline void scene_graph_node_destroy(SceneGraph* graph, Node node) {
    graph->nodes_to_destroy[graph->nodes_to_destroy_count++] = node;
}

static inline Position scene_graph_position_get(SceneGraph* graph, Node node) {
    int index = scene_graph_index_get(graph, node);
    return graph->world_positions[index];
}

static inline Position scene_graph_local_position_get(SceneGraph* graph, Node node) {
    int index = scene_graph_index_get(graph, node);
    return graph->local_positions[index];
}

static inline void scene_graph_local_position_set(SceneGraph* graph, Node node, Position position) {
    graph->updated_nodes[graph->updated_nodes_count++] = (UpdatedSceneNode){
        .node = node,
        .x    = position.x,
        .y    = position.y,
        .type = NODE_LOCAL,
    };
}

static inline void scene_graph_position_set(SceneGraph* graph, Node node, Position position) {
    graph->updated_nodes[graph->updated_nodes_count++] = (UpdatedSceneNode){
        .node = node,
        .x    = position.x,
        .y    = position.y,
        .type = NODE_WORLD,
    };
}

#endif  // LIB_SCENE_GRAPH_NODE_H_
