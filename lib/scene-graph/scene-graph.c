#include "scene-graph.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scene-graph/graph-sort.h"

void scene_graph_remove_destroyed_nodes(SceneGraph *graph) {
    for (int i = 0; i < graph->nodes_to_destroy_count; i++) {
        // Swap the node with the last node in the array to keep it packed
        Node node                            = graph->nodes_to_destroy[i];
        int node_index                       = graph->node_indices[node];
        int last_node_index                  = graph->nodes_count - 1;
        graph->nodes[node_index]             = graph->nodes[last_node_index];
        graph->node_indices[last_node_index] = node_index;
        graph->node_indices[node_index]      = -1;
        graph->nodes_count--;

        // Swap and potentially remove the game object
        int game_object_index = graph->game_object_indices[node_index];
        if (game_object_index != -1) {
            GameObject *object = &graph->game_objects[game_object_index];
            if (object->destroy != NULL) {
                object->destroy(graph, object);
            }

            int last_game_object_index             = graph->game_objects_count - 1;
            graph->game_objects[game_object_index] = graph->game_objects[last_game_object_index];
            graph->game_object_indices[last_game_object_index] = game_object_index;
            graph->game_object_indices[node_index]             = -1;
            graph->game_objects_count--;
        }

        // Swap and potentially remove the drawable
        int drawable_index = graph->drawable_indices[node_index];
        if (drawable_index != -1) {
            Drawable *drawable = &graph->drawables[drawable_index];
            if (drawable->destroy != NULL) {
                drawable->destroy(graph, drawable);
            }

            int last_drawable_index                      = graph->drawables_count - 1;
            graph->drawables[drawable_index]             = graph->drawables[last_node_index];
            graph->drawable_indices[last_drawable_index] = drawable_index;
            graph->drawable_indices[node_index]          = -1;
            graph->drawables_count--;
        }
    }
}

static int scene_graph_next_index(int *array, int *index) {
    assert(array != NULL && "Array cannot be NULL");
    assert(index != NULL && "Index cannot be NULL");

    int start = *index;
    while (array[*index] != -1) {
        *index = (*index + 1) % MAX_NODES;
        if (*index == start) {
            assert(0 && "Overflow");
        }
    }

    int result = *index;
    *index     = (result + 1) % MAX_NODES;
    return result;
}

inline static void scene_graph_compute_node_position(SceneGraph *graph, int index) {
    if (graph->nodes[index].parent != -1) {
        int parent_index              = graph->node_indices[graph->nodes[index].parent];
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
    int index = graph->node_indices[node->id];

    // Compute the current node's position
    scene_graph_compute_node_position(graph, index);

    // Traverse over children
    int next = node->first_child;
    while (next != -1) {
        SceneNode *child = &graph->nodes[graph->node_indices[next]];
        scene_graph_compute_position_recursive(graph, child);
        next = child->next_sibling;
    }
}

void scene_graph_compute_positions(SceneGraph *graph) {
    assert(graph != NULL && "Graph must not be NULL");

    for (int i = 0; i < graph->updated_nodes_count; i++) {
        UpdatedSceneNode update = graph->updated_nodes[i];

        assert(update.index >= 0 && update.index < graph->nodes_count && "Overflow node index");

        int index = graph->node_indices[update.index];
        assert(index >= 0 && index < graph->nodes_count && "Invalid graph index?");

        SceneNode *node = &graph->nodes[index];

        if (update.type == NODE_WORLD && node->parent != -1) {
            assert(node->parent >= 0 && node->parent < graph->nodes_count && "Overflow parent");

            Position pw_pos = graph->world_positions[graph->node_indices[node->parent]];
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

        scene_graph_compute_position_recursive(graph, node);
    }

    graph->updated_nodes_count = 0;
}

GameObject *scene_graph_game_object_new(SceneGraph *graph, Node node) {
    assert(node >= 0 && node < MAX_NODES && "Invalid Node?");
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
    assert(node >= 0 && node < MAX_NODES && "Invalid Node?");
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
        graph->nodes[0] = (SceneNode){
            .id           = 0,
            .parent       = -1,
            .first_child  = -1,
            .next_sibling = -1,
        };

        graph->node_indices[0] = 0;
        graph->nodes_count     = 1;
        graph->node_next_index = 1;
        return 0;
    }

    int index              = scene_graph_next_index(graph->node_indices, &graph->node_next_index);
    SceneNode *parent_node = &graph->nodes[graph->node_indices[parent]];
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
        .parent       = parent_node->id,
        .id           = index,
        .next_sibling = -1,
        .first_child  = -1,
    };

    // Track the new node in the updated list
    graph->node_indices[index] = graph->nodes_count;
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
        graph->node_indices[i]        = -1;
        graph->drawable_indices[i]    = -1;
        graph->game_object_indices[i] = -1;
    }

    return graph;
}
