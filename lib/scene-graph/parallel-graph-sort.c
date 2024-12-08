#include "parallel-graph-sort.h"

#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "scene-graph/scene-graph.h"
#include "thpool/thpool.h"

typedef struct Arguments {
    SceneGraph* graph;
    SceneNode* nodes;
    int nodes_count;
} Arguments;

typedef struct NodePos {
    int node;
    int layer;
    float y;
} NodePos;

static int compare_by_y(const void* a, const void* b) {
    const NodePos* v1 = a;
    const NodePos* v2 = b;
    const float ay    = v1->y;
    const float by    = v2->y;

    return (ay > by) - (ay < by);
}

static void sort_children(SceneGraph* graph, Node node) {
    int count = scene_graph_node_get(graph, node)->children_count;

    if (count > 0) {
        NodePos* nodes = malloc(count * sizeof(NodePos));
        assert(nodes != NULL);

        int child = scene_graph_first_child_get(graph, node);
        for (int i = 0; i < count; i++) {
            int index = scene_graph_index_get(graph, child);
            nodes[i]  = (NodePos){
                 .y    = graph->world_positions[index].y,
                 .node = child,
            };

            child = scene_graph_sibling_get(graph, child);
        }

        mergesort(nodes, count, sizeof(NodePos), compare_by_y);

        // Update links
        for (int i = 0; i < count - 1; i++) {
            scene_graph_sibling_set(graph, nodes[i].node, nodes[i + 1].node);
        }

        scene_graph_first_child_set(graph, node, nodes[0].node);
        scene_graph_sibling_set(graph, nodes[count - 1].node, NODE_NULL);

        free(nodes);
    }
}

static void scene_graph_populate_array(SceneGraph* graph,
                                       Node first_node,
                                       SceneNode* nodes,
                                       int* node_count) {
    int stack[100];
    int stack_len = 0;
    int node      = first_node;

    while (node != NODE_NULL) {
        int index    = *node_count;
        nodes[index] = *scene_graph_node_get(graph, node);
        (*node_count)++;

        Node first_child = scene_graph_first_child_get(graph, node);

        if (first_child != NODE_NULL) {
            // add the node to the stack
            stack[stack_len++] = node;
            node               = first_child;
        } else {
            // No children, try to move to next sibling
            node = scene_graph_sibling_get(graph, node);

            // If no sibling, backtrack up the stack until we find a node with an unvisited sibling
            while (node == NODE_NULL && stack_len > 0) {
                stack_len--;  // Pop the stack
                node = scene_graph_sibling_get(graph, stack[stack_len]);
            }
        }
    }
}

static void scene_graph_parallel(void* arg) {
    Arguments* arguments = arg;
    SceneGraph* graph    = arguments->graph;
    SceneNode* nodes     = arguments->nodes;

    for (int i = 0; i < arguments->nodes_count; i++) {
        sort_children(graph, nodes[i].id);
    }
}

void scene_graph_ysort_parallel(SceneGraph* graph, threadpool pool) {
    assert(graph != NULL && "Graph cannot be NULL");

    int thread_count = 8;
    Arguments args[thread_count];
    int nodes_per_thread = graph->nodes_count / thread_count;
    int remaining_nodes  = graph->nodes_count % thread_count;

    for (int i = 0; i < thread_count; i++) {
        int thread_nodes = nodes_per_thread;
        if (i == thread_count - 1) {
            thread_nodes += remaining_nodes;
        }

        args[i] = (Arguments){
            .nodes       = &graph->nodes[i * nodes_per_thread],
            .nodes_count = thread_nodes,
            .graph       = graph,
        };

        thpool_add_work(pool, scene_graph_parallel, &args[i]);
    }

    // Now collect into temp arrays
    SceneNode* temp_nodes          = malloc(sizeof(SceneNode) * MAX_NODES);
    Position* temp_world_positions = malloc(sizeof(Position) * MAX_NODES);
    Position* temp_local_positions = malloc(sizeof(Position) * MAX_NODES);
    Drawable* temp_drawables       = malloc(sizeof(Drawable) * MAX_NODES);
    int temp_node_count            = 0;
    int temp_drawable_count        = 0;

    thpool_wait(pool);

    scene_graph_populate_array(graph, 0, temp_nodes, &temp_node_count);

    for (int i = 0; i < temp_node_count; i++) {
        int old_node_idx = scene_graph_index_get(graph, temp_nodes[i].id);
        int old_draw_idx = graph->drawable_indices[temp_nodes[i].id];

        graph->nodes[i]  = temp_nodes[i];
        scene_graph_index_set(graph, temp_nodes[i].id, i);

        temp_local_positions[i] = graph->local_positions[old_node_idx];
        temp_world_positions[i] = graph->world_positions[old_node_idx];

        if (old_draw_idx != NODE_NULL) {
            temp_drawables[temp_drawable_count++] = graph->drawables[old_draw_idx];
        }
    }

    for (int i = 0; i < temp_node_count; i++) {
        graph->local_positions[i] = temp_local_positions[i];
        graph->world_positions[i] = temp_world_positions[i];
    }

    for (int i = 0; i < temp_drawable_count; i++) {
        graph->drawables[i]                             = temp_drawables[i];
        graph->drawable_indices[temp_drawables[i].node] = i;
    }

    free(temp_nodes);
    free(temp_drawables);
    free(temp_local_positions);
    free(temp_world_positions);
}
