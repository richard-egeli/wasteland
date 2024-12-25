
#include <assert.h>
#include <enet/enet.h>
#include <lauxlib.h>
#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <raylib.h>
#include <rlgl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "box2d/box2d.h"
#include "global.h"
#include "lua/asset_loader.h"
#include "lua/world.h"
#include "scene-graph/graph-sort.h"
#include "scene-graph/parallel-graph-sort.h"
#include "scene-graph/scene-graph.h"
#include "texture.h"
#include "thpool/thpool.h"

static void move_camera_left(void) {
    global.camera.target.x -= 3.0;
}

static void move_camera_right(void) {
    global.camera.target.x += 3.0;
}

static void move_camera_up(void) {
    global.camera.target.y -= 3.0;
}

static void move_camera_down(void) {
    global.camera.target.y += 3.0;
}

static void lua_init(const char* file) {
    lua_State* L = luaL_newstate();
    if (L == NULL) {
        fprintf(stderr, "Cannot create Lua state\n");
        exit(EXIT_FAILURE);
    }

    luaL_openlibs(L);
    register_world_api(L);
    register_asset_loader_api(L);

    if (luaL_loadfile(L, "scripts/assets.lua") != LUA_OK) {
        fprintf(stderr, "Error: %s\n", lua_tostring(L, -1));
        lua_close(L);
        exit(EXIT_FAILURE);
    }

    lua_pcall(L, 0, 0, 0);

    if (luaL_loadfile(L, file) != LUA_OK) {
        fprintf(stderr, "Error: %s\n", lua_tostring(L, -1));
        lua_close(L);
        exit(EXIT_FAILURE);
    }

    lua_pcall(L, 0, 0, 0);
}

static void move_player(SceneGraph* graph, GameObject* object) {
    float speed       = 2;
    Position movement = {0};

    if (IsKeyDown(KEY_W)) movement.y -= 1.0;
    if (IsKeyDown(KEY_S)) movement.y += 1.0;
    if (IsKeyDown(KEY_A)) movement.x -= 1.0;
    if (IsKeyDown(KEY_D)) movement.x += 1.0;

    Position current = scene_graph_position_get(graph, object->node);
    Position new     = {
            .x = current.x + movement.x * speed,
            .y = current.y + movement.y * speed,
    };

    scene_graph_position_set(graph, object->node, new);
}

static void draw_player(SceneGraph* graph, Drawable* drawable) {
    Position position = scene_graph_position_get(graph, drawable->node);
    DrawRectangle(position.x, position.y, 32, 32, RED);
}

static void draw_child(SceneGraph* graph, Drawable* drawable) {
    Position position = scene_graph_position_get(graph, drawable->node);
    DrawRectangle(position.x, position.y, 32, 32, BLUE);
}

static void draw_node(SceneGraph* graph, Drawable* drawable) {
    Position position = scene_graph_position_get(graph, drawable->node);

    uint8_t r         = ((uint8_t*)drawable->data)[0];
    uint8_t g         = ((uint8_t*)drawable->data)[1];
    uint8_t b         = ((uint8_t*)drawable->data)[2];
    DrawRectangle(position.x, position.y, 8, 8, (Color){r, g, b, 0xFF});
}

static void move_node(SceneGraph* graph, GameObject* object) {
    // Get the current position
    Position current = scene_graph_position_get(graph, object->node);

    // Define boundaries for the movement
    const int width_max  = 1280 - 8;  // Minus size of the node
    const int height_max = 720 - 8;   // Minus size of the node

    // Generate random deltas for movement
    int dx = (rand() % 7) - 3;  // Random value between -3 and +3
    int dy = (rand() % 7) - 3;  // Random value between -3 and +3

    // Update the position with boundaries
    current.x = current.x + dx;
    current.y = current.y + dy;

    if (current.x < 0) current.x = 0;
    if (current.y < 0) current.y = 0;
    if (current.x > width_max) current.x = width_max;
    if (current.y > height_max) current.y = height_max;

    // Set the new position in the scene graph
    scene_graph_position_set(graph, object->node, current);
}

int main(void) {
    InitWindow(1280, 720, "Witch");
    SetTargetFPS(60);
    srand((unsigned)time(NULL));

    texture_init();

    lua_init("scripts/main.lua");

    for (int i = 0; i < worlds_count; i++) {
        scene_graph_compute_positions(worlds[i]);
    }

    uint64_t count       = 0;
    double total_time    = 0.0;

    threadpool pool      = thpool_init(16);
    b2WorldDef world_def = b2DefaultWorldDef();
    b2WorldId world      = b2CreateWorld(&world_def);

    while (!WindowShouldClose()) {
        for (int i = 0; i < worlds_count; i++) {
            scene_graph_update(worlds[i]);
            scene_graph_compute_positions(worlds[i]);
            scene_graph_ysort_parallel(worlds[i], pool);
        }

        BeginDrawing();
        ClearBackground(WHITE);

        for (int i = 0; i < worlds_count; i++) {
            scene_graph_render(worlds[i]);
        }

        DrawFPS(10, 10);
        EndDrawing();
    }

    texture_free();
    CloseWindow();
}
