#ifndef LIB_SCENE_GRAPH_NODE_H_
#define LIB_SCENE_GRAPH_NODE_H_

#define MAX_NODES 4096
#define NODE_NULL -1

typedef int Node;

typedef struct {
    float x;
    float y;
} Position;

typedef struct {
    int index;
    int parent;
    int first_child;
    int next_sibling;
} SceneNode;

typedef struct GameObject {
    void (*update)(struct GameObject* object);
    void (*destroy)(struct GameObject* object);
    void (*start)(struct GameObject* object);
    void* data;
} GameObject;

typedef struct {
    int next_index;
    int nodes_count;
    int indices[MAX_NODES];
    SceneNode nodes[MAX_NODES];
    Position local_positions[MAX_NODES];
    Position world_positions[MAX_NODES];
    SceneNode updated_nodes[MAX_NODES];
    GameObject game_objects[MAX_NODES];
} SceneGraph;

Node scene_graph_node_new(SceneGraph* graph, Node parent);

void scene_graph_compute_positions(SceneGraph* graph, Node node);

SceneGraph* scene_graph_new(void);

static inline Node scene_graph_parent_get(SceneGraph* graph, Node node) {
    return graph->nodes[graph->indices[node]].parent;
}

static inline Position scene_graph_position_get(SceneGraph* graph, Node node) {
    return graph->world_positions[graph->indices[node]];
}

static inline Position scene_graph_local_position_get(SceneGraph* graph, Node node) {
    return graph->local_positions[graph->indices[node]];
}

static inline void scene_graph_local_position_set(SceneGraph* graph, Node node, Position position) {
    graph->local_positions[graph->indices[node]] = position;
    scene_graph_compute_positions(graph, node);
}

static inline void scene_graph_position_set(SceneGraph* graph, Node node, Position position) {
    int index  = graph->indices[node];
    int parent = graph->nodes[index].parent;
    if (parent != -1) {
        int parent_index    = graph->indices[parent];
        Position parent_pos = graph->world_positions[parent_index];
        Position world_pos  = (Position){
             .x = position.x - parent_pos.x,
             .y = position.y - parent_pos.y,
        };

        scene_graph_local_position_set(graph, node, world_pos);
    } else {
        // Root node so just set the position
        scene_graph_local_position_set(graph, node, position);
    }
}

#endif  // LIB_SCENE_GRAPH_NODE_H_
