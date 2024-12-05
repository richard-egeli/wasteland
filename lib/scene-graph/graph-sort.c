#include "graph-sort.h"

#include <limits.h>
#include <stdlib.h>

#include "scene-graph.h"

typedef struct {
    int node_id;
    int next_sibling;
} StackFrame;

SceneGraph* global_graph_context;

static int compare_by_y(const void* a, const void* b) {
    const int* da     = (const void*)a;
    const int* db     = (const void*)b;
    SceneGraph* graph = (void*)global_graph_context;

    const float ay    = graph->world_positions[graph->node_indices[*da]].y;
    const float by    = graph->world_positions[graph->node_indices[*db]].y;

    return (ay > by) - (ay < by);
}

static void scene_graph_children_ysort(SceneGraph* graph, Node node, SceneNode* nodes, int* count) {
    nodes[(*count)++] = graph->nodes[graph->node_indices[node]];

    int siblings[MAX_NODES];
    int siblings_length = 0;
    int sibling_id      = graph->nodes[graph->node_indices[node]].first_child;
    while (sibling_id != -1) {
        siblings[siblings_length++] = sibling_id;
        sibling_id                  = graph->nodes[graph->node_indices[sibling_id]].next_sibling;
    }

    global_graph_context = graph;
    mergesort(siblings, siblings_length, sizeof(int), compare_by_y);

    for (int i = 0; i < siblings_length; i++) {
        int id = siblings[i];
        scene_graph_children_ysort(graph, id, nodes, count);
    }
}

void scene_graph_ysort(SceneGraph* graph) {
    int node_count = 0;
    SceneNode temp_nodes[MAX_NODES];
    Position temp_world_positions[MAX_NODES];
    Position temp_local_positions[MAX_NODES];
    Drawable temp_drawables[MAX_NODES];
    int temp_drawable_length = 0;

    scene_graph_children_ysort(graph, 0, temp_nodes, &node_count);

    for (int i = 0; i < node_count; i++) {
        int old_node_idx                      = graph->node_indices[temp_nodes[i].id];
        int old_draw_idx                      = graph->drawable_indices[temp_nodes[i].id];

        graph->nodes[i]                       = temp_nodes[i];
        graph->node_indices[temp_nodes[i].id] = i;

        temp_local_positions[i]               = graph->local_positions[old_node_idx];
        temp_world_positions[i]               = graph->world_positions[old_node_idx];

        if (old_draw_idx != -1) {
            temp_drawables[temp_drawable_length++] = graph->drawables[old_draw_idx];
        }
    }

    for (int i = 0; i < node_count; i++) {
        graph->local_positions[i] = temp_local_positions[i];
        graph->world_positions[i] = temp_world_positions[i];
    }

    for (int i = 0; i < temp_drawable_length; i++) {
        graph->drawables[i]                             = temp_drawables[i];
        graph->drawable_indices[temp_drawables[i].node] = i;
    }
}
