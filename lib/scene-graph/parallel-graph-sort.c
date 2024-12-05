#include "parallel-graph-sort.h"

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "scene-graph/scene-graph.h"
#include "thread-pool/thread-pool.h"

SceneGraph* global_graph_context;

static int compare_by_y(const void* a, const void* b) {
    const int* v1     = a;
    const int* v2     = b;
    SceneGraph* graph = (void*)global_graph_context;
    const float ay    = graph->world_positions[graph->node_indices[*v1]].y;
    const float by    = graph->world_positions[graph->node_indices[*v2]].y;

    return (ay > by) - (ay < by);
}

static void sort_children(SceneGraph* graph, SceneNode* node) {
    int count = 0;
    int child = node->first_child;

    // First count children
    while (child != -1) {
        SceneNode* child_node = &graph->nodes[graph->node_indices[child]];
        child                 = graph->nodes[graph->node_indices[child]].next_sibling;
        count++;
    }

    if (count > 0) {
        int* nodes = malloc(count * sizeof(int));
        assert(nodes != NULL);

        // Fill array
        child = node->first_child;
        for (int i = 0; i < count; i++) {
            nodes[i] = child;
            child    = graph->nodes[graph->node_indices[child]].next_sibling;
        }

        mergesort(nodes, count, sizeof(int), compare_by_y);

        // Update links
        for (int i = 0; i < count - 1; i++) {
            SceneNode* n1    = &graph->nodes[graph->node_indices[nodes[i]]];
            SceneNode* n2    = &graph->nodes[graph->node_indices[nodes[i + 1]]];
            n1->next_sibling = n2->id;
        }

        node->first_child                                                = nodes[0];
        graph->nodes[graph->node_indices[nodes[count - 1]]].next_sibling = -1;

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

    while (node != -1) {
        int index    = *node_count;
        int node_idx = graph->node_indices[node];
        nodes[index] = graph->nodes[node_idx];
        (*node_count)++;

        int first_child = graph->nodes[node_idx].first_child;

        if (first_child != -1) {
            // add the node to the stack
            stack[stack_len++] = node;
            node               = first_child;
        } else {
            // No children, try to move to next sibling
            node = graph->nodes[node_idx].next_sibling;

            // If no sibling, backtrack up the stack until we find a node with an unvisited sibling
            while (node == -1 && stack_len > 0) {
                stack_len--;  // Pop the stack
                node = graph->nodes[graph->node_indices[stack[stack_len]]].next_sibling;
            }
        }
    }
}

typedef struct Arguments {
    SceneGraph* graph;
    SceneNode* nodes;
    int nodes_count;
} Arguments;

static void scene_graph_parallel(void* arg) {
    Arguments* arguments = arg;
    SceneGraph* graph    = arguments->graph;
    SceneNode* nodes     = arguments->nodes;

    for (int i = 0; i < arguments->nodes_count; i++) {
        SceneNode* node = &nodes[i];
        sort_children(graph, node);
    }
}

void scene_graph_ysort_parallel(SceneGraph* graph, ThreadPool* pool) {
    assert(graph != NULL && "Graph cannot be NULL");
    global_graph_context = graph;

    // Now collect into temp arrays
    SceneNode* temp_nodes          = malloc(sizeof(SceneNode) * MAX_NODES);
    Position* temp_world_positions = malloc(sizeof(Position) * MAX_NODES);
    Position* temp_local_positions = malloc(sizeof(Position) * MAX_NODES);
    Drawable* temp_drawables       = malloc(sizeof(Drawable) * MAX_NODES);
    int temp_node_count            = 0;
    int temp_drawable_count        = 0;

    int thread_count               = graph->nodes_count / 1000;
    int manual_count               = graph->nodes_count % 1000;
    int manual_offset              = 0;
    Arguments args[thread_count];

    if (pool != NULL) {
        manual_offset = thread_count * 1000;
        for (int i = 0; i < thread_count; i++) {
            args[i] = (Arguments){
                .nodes       = &graph->nodes[i * 1000],
                .nodes_count = 1000,
                .graph       = graph,
            };

            thread_pool_push(pool, scene_graph_parallel, &args[i]);
        }
    }

    for (int i = manual_offset; i < graph->nodes_count; i++) {
        SceneNode* node = &graph->nodes[i];
        sort_children(graph, node);
    }

    thread_pool_wait_all(pool);

    int head = atomic_load(&pool->head);
    int tail = atomic_load(&pool->tail);

    scene_graph_populate_array(graph, 0, temp_nodes, &temp_node_count);

    for (int i = 0; i < temp_node_count; i++) {
        int old_node_idx                      = graph->node_indices[temp_nodes[i].id];
        int old_draw_idx                      = graph->drawable_indices[temp_nodes[i].id];

        graph->nodes[i]                       = temp_nodes[i];
        graph->node_indices[temp_nodes[i].id] = i;

        temp_local_positions[i]               = graph->local_positions[old_node_idx];
        temp_world_positions[i]               = graph->world_positions[old_node_idx];

        if (old_draw_idx != -1) {
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
