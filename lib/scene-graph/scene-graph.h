#ifndef LIB_SCENE_GRAPH_NODE_H_
#define LIB_SCENE_GRAPH_NODE_H_

#include <stdint.h>

#define MAX_NODES 10000
#define NODE_NULL -1

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
    int index;
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
    int game_objects_next_index;
    int game_objects_count;

    // NOTE: Drawables
    Drawable drawables[MAX_NODES];
    int drawable_indices[MAX_NODES];
    int drawables_next_index;
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

static inline void scene_graph_node_destroy(SceneGraph* graph, Node node) {
    graph->nodes_to_destroy[graph->nodes_to_destroy_count++] = node;
}

static inline Node scene_graph_parent_get(SceneGraph* graph, Node node) {
    return graph->nodes[graph->node_indices[node]].parent;
}

static inline Position scene_graph_position_get(SceneGraph* graph, Node node) {
    return graph->world_positions[graph->node_indices[node]];
}

static inline Position scene_graph_local_position_get(SceneGraph* graph, Node node) {
    return graph->local_positions[graph->node_indices[node]];
}

static inline void scene_graph_local_position_set(SceneGraph* graph, Node node, Position position) {
    graph->updated_nodes[graph->updated_nodes_count++] = (UpdatedSceneNode){
        .index = node,
        .x     = position.x,
        .y     = position.y,
        .type  = NODE_LOCAL,
    };
}

static inline void scene_graph_position_set(SceneGraph* graph, Node node, Position position) {
    graph->updated_nodes[graph->updated_nodes_count++] = (UpdatedSceneNode){
        .index = node,
        .x     = position.x,
        .y     = position.y,
        .type  = NODE_WORLD,
    };
}

#endif  // LIB_SCENE_GRAPH_NODE_H_
