#include "scene-graph.h"

#include <assert.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scene-graph/graph-sort.h"

void scene_graph_remove_destroyed_nodes(SceneGraph *graph) {
    for (int i = 0; i < graph->nodes_to_destroy_count; i++) {
        // Swap the node with the last node in the array to keep it packed
        Node node                = graph->nodes_to_destroy[i];
        int node_index           = scene_graph_index_get(graph, node);
        int last_node_index      = graph->nodes_count - 1;
        graph->nodes[node_index] = graph->nodes[last_node_index];

        scene_graph_index_set(graph, last_node_index, node_index);
        scene_graph_index_set(graph, node_index, NODE_NULL);
        graph->nodes_count--;

        // Swap and potentially remove the game object
        int game_object_index = graph->game_object_indices[node_index];
        if (game_object_index != NODE_NULL) {
            GameObject *object = &graph->game_objects[game_object_index];
            if (object->destroy != NULL) {
                object->destroy(graph, object);
            }

            int last_game_object_index             = graph->game_objects_count - 1;
            graph->game_objects[game_object_index] = graph->game_objects[last_game_object_index];
            graph->game_object_indices[last_game_object_index] = game_object_index;
            graph->game_object_indices[node_index]             = NODE_NULL;
            graph->game_objects_count--;
        }

        // Swap and potentially remove the drawable
        int drawable_index = graph->drawable_indices[node_index];
        if (drawable_index != NODE_NULL) {
            Drawable *drawable = &graph->drawables[drawable_index];
            if (drawable->destroy != NULL) {
                drawable->destroy(graph, drawable);
            }

            int last_drawable_index                      = graph->drawables_count - 1;
            graph->drawables[drawable_index]             = graph->drawables[last_drawable_index];
            graph->drawable_indices[last_drawable_index] = drawable_index;
            graph->drawable_indices[node_index]          = NODE_NULL;
            graph->drawables_count--;
        }
    }
}

static int scene_graph_next_index(int *array, int *index) {
    assert(array != NULL && "Array cannot be NULL");
    assert(index != NULL && "Index cannot be NULL");

    int start = *index;

    while (array[*index] != NODE_NULL) {
        *index = (*index + 1) % MAX_NODES;
        if (*index == start) {
            assert(0 && "Overflow");
        }
    }

    int result = *index;
    *index     = (result + 1) % MAX_NODES;
    return result;
}

inline static void scene_graph_compute_node_position(SceneGraph *graph, Node node) {
    int index  = scene_graph_index_get(graph, node);
    int parent = scene_graph_parent_get(graph, node);
    if (parent != NODE_NULL) {
        int parent_index              = scene_graph_index_get(graph, parent);
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

static void scene_graph_compute_position_recursive(SceneGraph *graph, Node node) {
    // Compute the current node's position
    scene_graph_compute_node_position(graph, node);

    // Traverse over children
    int next = scene_graph_first_child_get(graph, node);
    while (next != NODE_NULL) {
        scene_graph_compute_position_recursive(graph, next);
        next = scene_graph_sibling_get(graph, next);
    }
}

void scene_graph_compute_positions(SceneGraph *graph) {
    assert(graph != NULL && "Graph must not be NULL");

    for (int i = 0; i < graph->updated_nodes_count; i++) {
        UpdatedSceneNode update = graph->updated_nodes[i];
        assert(update.node >= 0 && update.node < graph->nodes_count && "Overflow node index");

        int parent = scene_graph_parent_get(graph, update.node);
        int index  = scene_graph_index_get(graph, update.node);
        if (update.type == NODE_WORLD && parent != NODE_NULL) {
            assert(parent >= 0 && parent < graph->nodes_count && "Overflow parent");

            int parent_index              = scene_graph_index_get(graph, parent);
            Position pw_pos               = graph->world_positions[parent_index];
            graph->local_positions[index] = (Position){
                .x = update.x - pw_pos.x,
                .y = update.y - pw_pos.y,
            };
        } else {
            graph->local_positions[index] = (Position){
                .x = update.x,
                .y = update.y,
            };
        }

        scene_graph_compute_position_recursive(graph, update.node);
    }

    graph->updated_nodes_count = 0;
}

GameObject *scene_graph_game_object_new(SceneGraph *graph, Node node) {
    assert(node >= 0 && "Invalid Node?");
    assert(graph->game_objects_count < MAX_NODES && "Game object overflow");

    graph->game_object_indices[node] = graph->game_objects_count;
    GameObject *object               = &graph->game_objects[graph->game_objects_count++];
    object->node                     = node;
    object->data                     = NULL;
    object->update                   = NULL;
    object->destroy                  = NULL;
    return object;
}

Drawable *scene_graph_drawable_new(SceneGraph *graph, Node node) {
    assert(node >= 0 && "Invalid Node?");
    assert(graph->drawables_count < MAX_NODES && "Drawable overflow");

    graph->drawable_indices[node] = graph->drawables_count;
    Drawable *drawable            = &graph->drawables[graph->drawables_count++];
    drawable->node                = node;
    drawable->data                = NULL;
    drawable->destroy             = NULL;
    drawable->draw                = NULL;
    return drawable;
}

Node scene_graph_node_new(SceneGraph *graph, Node parent) {
    assert(graph != NULL && "Scene graph cannot be NULL");
    assert(graph->nodes_count < MAX_NODES && "Scene graph overflow");

    // If root node just initialize a basic node
    if (parent == NODE_NULL) {
        assert(graph->nodes_count == 0 && "Scene graph root node already exists");

        Node node       = 0;
        graph->nodes[0] = (SceneNode){
            .id = node,
        };

        scene_graph_index_set(graph, node, 0);
        scene_graph_sibling_set(graph, node, NODE_NULL);
        scene_graph_parent_set(graph, node, NODE_NULL);
        scene_graph_first_child_set(graph, node, NODE_NULL);

        graph->nodes_count     = 1;
        graph->node_next_index = 1;
        return 0;
    }

    int index = scene_graph_next_index(graph->node_indices, &graph->node_next_index);

    if (scene_graph_first_child_get(graph, parent) == NODE_NULL) {
        scene_graph_first_child_set(graph, parent, index);
    } else {
        Node child_node = scene_graph_first_child_get(graph, parent);
        assert(child_node > 0);

        Node current      = child_node;
        Node last_sibling = current;
        while (current != NODE_NULL) {
            last_sibling = current;
            current      = scene_graph_sibling_get(graph, current);
        }
        scene_graph_sibling_set(graph, last_sibling, index);
    }

    // Add the new node to the graph
    graph->nodes[graph->nodes_count] = (SceneNode){
        .id             = index,
        .parent         = parent,
        .first_child    = NODE_NULL,
        .next_sibling   = NODE_NULL,
        .children_count = 0,
        .layer          = 0,
    };

    scene_graph_index_set(graph, index, index);

    // Track the new node in the updated list
    scene_graph_node_get(graph, parent)->children_count++;
    graph->nodes_count++;

    return index;
}

void scene_graph_update(SceneGraph *graph) {
    for (int i = 0; i < graph->game_objects_count; i++) {
        GameObject *obj = &graph->game_objects[i];
        if (obj->update != NULL) {
            obj->update(graph, obj);
        }
    }
}

void scene_graph_render(SceneGraph *graph) {
    for (int i = 0; i < graph->drawables_count; i++) {
        Drawable *obj = &graph->drawables[i];
        if (obj->draw != NULL) {
            obj->draw(graph, obj);
        }
    }
}

SceneGraph *scene_graph_new(void) {
    SceneGraph *graph = calloc(sizeof(SceneGraph), 1);
    assert(graph != NULL && "Scene graph cannot be null");

    for (int i = 0; i < MAX_NODES; i++) {
        scene_graph_index_set(graph, i, NODE_NULL);
        graph->node_indices[i]        = NODE_NULL;
        graph->drawable_indices[i]    = NODE_NULL;
        graph->game_object_indices[i] = NODE_NULL;
        graph->nodes[i]               = (SceneNode){
                          .id             = NODE_NULL,
                          .parent         = NODE_NULL,
                          .first_child    = NODE_NULL,
                          .next_sibling   = NODE_NULL,
                          .children_count = 0,
                          .layer          = 0,
        };
    }

    return graph;
}
