
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

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(simple_insert);
    RUN_TEST(simple_move);
    RUN_TEST(move_parent_and_child_local);
    RUN_TEST(graph_sorting);
    return UNITY_END();
}
