#include "scene-graph.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

inline static void scene_graph_compute_node_position(SceneGraph *graph, int index) {
    if (graph->nodes[index].parent != -1) {
        int parent_index              = graph->indices[graph->nodes[index].parent];
        Position world_pos            = graph->world_positions[parent_index];
        Position local_pos            = graph->local_positions[index];
        graph->world_positions[index] = (Position){
            .x = world_pos.x + local_pos.x,
            .y = world_pos.y + local_pos.y,
        };
    } else {
        graph->world_positions[index] = graph->local_positions[index];
    }
}

static void scene_graph_compute_position_recursive(SceneGraph *graph, SceneNode *node) {
    int index = graph->indices[node->index];

    // Compute the current node's position
    scene_graph_compute_node_position(graph, index);

    // Traverse over children
    int next = node->first_child;
    while (next != -1) {
        SceneNode *child = &graph->nodes[graph->indices[next]];
        scene_graph_compute_position_recursive(graph, child);
        next = child->next_sibling;
    }
}

void scene_graph_compute_positions(SceneGraph *graph, Node node) {
    SceneNode *scene_node = &graph->nodes[graph->indices[node]];
    scene_graph_compute_position_recursive(graph, scene_node);
}

static int scene_graph_next_free_index(SceneGraph *graph) {
    assert(graph != NULL && "Scene graph cannot be NULL");
    assert(graph->nodes_count < MAX_NODES && "Scene graph overflow");

    int start = graph->next_index;
    while (graph->indices[graph->next_index] != -1) {
        graph->next_index = (graph->next_index + 1) % MAX_NODES;
        if (graph->next_index == start) {
            assert(0 && "Scene graph is full?");
        }
    }

    int index         = graph->next_index;
    graph->next_index = (graph->next_index + 1) % MAX_NODES;
    return index;
}

Node scene_graph_node_new(SceneGraph *graph, Node parent) {
    assert(graph != NULL && "Scene graph cannot be NULL");
    assert(graph->nodes_count < MAX_NODES && "Scene graph overflow");

    // If root node just initialize a basic node
    if (parent == NODE_NULL) {
        assert(graph->nodes_count == 0 && "Scene graph root node already exists");
        graph->nodes[0] = (SceneNode){
            .index        = 0,
            .parent       = -1,
            .first_child  = -1,
            .next_sibling = -1,
        };

        graph->indices[0]  = 0;
        graph->nodes_count = 1;
        graph->next_index  = 1;
        return 0;
    }

    int index              = scene_graph_next_free_index(graph);
    SceneNode *parent_node = &graph->nodes[graph->indices[parent]];
    if (parent_node->first_child == -1) {
        parent_node->first_child = index;
    } else {
        assert(parent_node->first_child < MAX_NODES);
        assert(parent_node->first_child >= 0);

        // Find the last sibling of the parent
        SceneNode *lastChild = &graph->nodes[parent_node->first_child];
        while (lastChild->next_sibling != -1) {
            lastChild = &graph->nodes[lastChild->next_sibling];
        }

        // Append as a sibling
        lastChild->next_sibling = index;
    }

    // Add the new node to the graph
    graph->nodes[graph->nodes_count] = (SceneNode){
        .parent       = parent_node->index,
        .index        = index,
        .next_sibling = -1,
        .first_child  = -1,
    };

    // Track the new node in the updated list
    graph->indices[index] = graph->nodes_count;
    graph->nodes_count++;

    // Update world positions
    scene_graph_compute_positions(graph, parent_node->index);

    return index;
}

SceneGraph *scene_graph_new(void) {
    SceneGraph *graph = calloc(sizeof(SceneGraph), 1);
    assert(graph != NULL && "Scene graph cannot be null");

    for (int i = 0; i < MAX_NODES; i++) {
        graph->indices[i] = -1;
    }

    return graph;
}
