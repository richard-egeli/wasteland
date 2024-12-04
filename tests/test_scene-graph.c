
#include <stdlib.h>
#include <unity.h>

#include "scene-graph/graph-sort.h"
#include "scene-graph/scene-graph.h"

void setUp() {
}
void tearDown() {
}

static void simple_insert(void) {
    SceneGraph* graph = scene_graph_new();
    Node root         = scene_graph_node_new(graph, NODE_NULL);
    Node next         = scene_graph_node_new(graph, root);

    TEST_ASSERT_EQUAL_INT(0, root);
    TEST_ASSERT_EQUAL_INT(1, next);

    free(graph);
}

static void simple_move(void) {
    SceneGraph* graph = scene_graph_new();
    Node root         = scene_graph_node_new(graph, NODE_NULL);
    Node next         = scene_graph_node_new(graph, root);

    scene_graph_local_position_set(graph, root, (Position){10, 0});
    scene_graph_compute_positions(graph);

    Position world_position = scene_graph_position_get(graph, next);
    Position local_position = scene_graph_local_position_get(graph, next);

    TEST_ASSERT_EQUAL_FLOAT(10.f, world_position.x);
    TEST_ASSERT_EQUAL_FLOAT(0.f, world_position.y);

    TEST_ASSERT_EQUAL_FLOAT(0.f, local_position.x);
    TEST_ASSERT_EQUAL_FLOAT(0.f, local_position.y);

    scene_graph_local_position_set(graph, root, (Position){20, 10});
    scene_graph_compute_positions(graph);

    world_position = scene_graph_position_get(graph, next);
    local_position = scene_graph_local_position_get(graph, next);

    TEST_ASSERT_EQUAL_FLOAT(20.f, world_position.x);
    TEST_ASSERT_EQUAL_FLOAT(10.f, world_position.y);

    TEST_ASSERT_EQUAL_FLOAT(0.f, local_position.x);
    TEST_ASSERT_EQUAL_FLOAT(0.f, local_position.y);
    free(graph);
}

static void graph_sorting(void) {
    SceneGraph* graph = scene_graph_new();

    Node root         = scene_graph_node_new(graph, NODE_NULL);
    Node c1           = scene_graph_node_new(graph, root);
    Node c2           = scene_graph_node_new(graph, root);

    TEST_ASSERT_EQUAL(graph->nodes[1].id, 1);
    TEST_ASSERT_EQUAL(graph->nodes[2].id, 2);

    scene_graph_position_set(graph, c2, (Position){0, -10});
    scene_graph_compute_positions(graph);
    scene_graph_ysort(graph);

    TEST_ASSERT_EQUAL(graph->nodes[1].id, 2);
    TEST_ASSERT_EQUAL(graph->nodes[2].id, 1);

    free(graph);
}

static void graph_check_stable_sort(void) {
    SceneGraph* graph = scene_graph_new();

    // Create a root node and three child nodes
    Node root = scene_graph_node_new(graph, NODE_NULL);
    Node c1   = scene_graph_node_new(graph, root);
    Node c2   = scene_graph_node_new(graph, root);
    Node c3   = scene_graph_node_new(graph, root);

    // Assign positions with same y-values
    scene_graph_position_set(graph, c1, (Position){0, 10});
    scene_graph_position_set(graph, c2, (Position){0, 10});
    scene_graph_position_set(graph, c3, (Position){0, 10});

    // Trigger sorting explicitly
    scene_graph_compute_positions(graph);
    scene_graph_ysort(graph);

    // Verify that the relative order is maintained for equal y-values
    TEST_ASSERT_EQUAL(graph->nodes[1].id, c1);
    TEST_ASSERT_EQUAL(graph->nodes[2].id, c2);
    TEST_ASSERT_EQUAL(graph->nodes[3].id, c3);

    // Add nodes with distinct y-values
    Node c4 = scene_graph_node_new(graph, root);
    Node c5 = scene_graph_node_new(graph, root);
    scene_graph_position_set(graph, c4, (Position){0, 5});   // Higher y-value
    scene_graph_position_set(graph, c5, (Position){0, 15});  // Lower y-value

    TEST_ASSERT_EQUAL(graph->nodes[4].id, c4);
    TEST_ASSERT_EQUAL(graph->nodes[5].id, c5);

    // Trigger sorting explicitly again
    scene_graph_compute_positions(graph);
    scene_graph_ysort(graph);

    // Verify sort order: c4 (y=15), c1/c2/c3 (y=10, stable order), c5 (y=5)
    TEST_ASSERT_EQUAL(graph->nodes[1].id, c4);  // Highest y
    TEST_ASSERT_EQUAL(graph->nodes[2].id, c1);  // First among equals
    TEST_ASSERT_EQUAL(graph->nodes[3].id, c2);  // Second among equals
    TEST_ASSERT_EQUAL(graph->nodes[4].id, c3);  // Third among equals
    TEST_ASSERT_EQUAL(graph->nodes[5].id, c5);  // Lowest y

    free(graph);
}

static void move_parent_and_child_local(void) {
    SceneGraph* graph = scene_graph_new();

    Node root         = scene_graph_node_new(graph, NODE_NULL);
    Node next         = scene_graph_node_new(graph, root);

    scene_graph_local_position_set(graph, root, (Position){10, 10});
    scene_graph_local_position_set(graph, next, (Position){20, 20});
    scene_graph_compute_positions(graph);

    Position rw_pos = scene_graph_position_get(graph, root);
    Position nw_pos = scene_graph_position_get(graph, next);
    Position rl_pos = scene_graph_local_position_get(graph, root);
    Position nl_pos = scene_graph_local_position_get(graph, next);

    TEST_ASSERT_EQUAL_FLOAT(10.f, rw_pos.x);
    TEST_ASSERT_EQUAL_FLOAT(10.f, rw_pos.y);
    TEST_ASSERT_EQUAL_FLOAT(30.f, nw_pos.x);
    TEST_ASSERT_EQUAL_FLOAT(30.f, nw_pos.y);

    TEST_ASSERT_EQUAL_FLOAT(10.f, rl_pos.x);
    TEST_ASSERT_EQUAL_FLOAT(10.f, rl_pos.y);
    TEST_ASSERT_EQUAL_FLOAT(20.f, nl_pos.x);
    TEST_ASSERT_EQUAL_FLOAT(20.f, nl_pos.y);

    free(graph);
}

static void delete_node_from_graph(void) {
    SceneGraph* graph = scene_graph_new();

    Node root         = scene_graph_node_new(graph, NODE_NULL);
    Node children[20];
    for (int i = 0; i < 20; i++) {
        children[i] = scene_graph_node_new(graph, root);
    }

    for (int i = 0; i < 10; i++) {
        scene_graph_node_destroy(graph, children[i]);
    }

    scene_graph_remove_destroyed_nodes(graph);

    for (int i = 1; i < 11; i++) {
        int id = 21 - i;

        TEST_ASSERT_EQUAL(id, graph->nodes[i].id);
    }

    free(graph);
}

static void delete_node_with_game_object(void) {
    SceneGraph* graph = scene_graph_new();

    Node root         = scene_graph_node_new(graph, NODE_NULL);
    Node children[20];
    GameObject* game_objects[20];
    for (int i = 0; i < 20; i++) {
        children[i]     = scene_graph_node_new(graph, root);
        game_objects[i] = scene_graph_game_object_new(graph, children[i]);
    }

    for (int i = 0; i < 20; i++) {
        int node_id = children[i];
    }

    for (int i = 0; i < 10; i++) {
        scene_graph_node_destroy(graph, children[i]);
    }

    scene_graph_remove_destroyed_nodes(graph);

    for (int i = 1; i < 11; i++) {
        int id                = 21 - i;

        int game_object_index = graph->game_object_indices[id];

        TEST_ASSERT_EQUAL(id, graph->nodes[i].id);
        TEST_ASSERT_EQUAL(id, graph->game_objects[game_object_index].node);
    }

    free(graph);
}

static void delete_node_with_drawable(void) {
    SceneGraph* graph = scene_graph_new();

    Node root         = scene_graph_node_new(graph, NODE_NULL);
    Node children[20];
    Drawable* game_objects[20];
    for (int i = 0; i < 20; i++) {
        children[i]     = scene_graph_node_new(graph, root);
        game_objects[i] = scene_graph_game_object_new(graph, children[i]);
    }

    for (int i = 0; i < 10; i++) {
        scene_graph_node_destroy(graph, children[i]);
    }

    scene_graph_remove_destroyed_nodes(graph);

    for (int i = 1; i < 11; i++) {
        int id = 21 - i;

        TEST_ASSERT_EQUAL(id, graph->nodes[i].id);
        TEST_ASSERT_EQUAL(id, graph->game_objects[i].node);
    }

    free(graph);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(simple_insert);
    RUN_TEST(simple_move);
    RUN_TEST(move_parent_and_child_local);
    RUN_TEST(graph_sorting);
    RUN_TEST(graph_check_stable_sort);
    RUN_TEST(delete_node_from_graph);
    RUN_TEST(delete_node_with_game_object);
    return UNITY_END();
}
