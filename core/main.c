
#include <assert.h>
#include <enet/enet.h>
#include <lauxlib.h>
#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "box2d/box2d.h"
#include "global.h"
#include "lua/asset_loader.h"
#include "lua/dynamic_body.h"
#include "lua/entity.h"
#include "lua/input.h"
#include "lua/world.h"
#include "scene-graph/parallel-graph-sort.h"
#include "scene-graph/scene-graph.h"
#include "texture.h"
#include "thpool/thpool.h"

static int error_handler(lua_State* L) {
    const char* msg = lua_tostring(L, 1);
    luaL_traceback(L, L, msg, 1);  // This adds the stack trace
    fprintf(stderr, "Lua Error: %s\n", lua_tostring(L, -1));
    return 1;
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
    register_input_api(L);
    register_entity_api(L);
    register_dynamic_body_api(L);

    lua_pushcfunction(L, error_handler);  // Push error handler
    int error_handler_index = lua_gettop(L);

    if (luaL_loadfile(L, "scripts/assets.lua") != LUA_OK) {
        fprintf(stderr, "Error: %s\n", lua_tostring(L, -1));
        lua_close(L);
        exit(EXIT_FAILURE);
    }

    lua_pcall(L, 0, 0, error_handler_index);

    if (luaL_loadfile(L, file) != LUA_OK) {
        fprintf(stderr, "Error: %s\n", lua_tostring(L, -1));
        lua_close(L);
        exit(EXIT_FAILURE);
    }

    lua_pcall(L, 0, 0, error_handler_index);
}

int main(void) {
    InitWindow(1280, 720, "Witch");
    SetTargetFPS(60);
    srand((unsigned)time(NULL));

    texture_init();

    lua_init("scripts/main.lua");

    for (int i = 0; i < worlds_count; i++) {
        scene_graph_compute_positions(worlds[i]->graph);
    }

    uint64_t count    = 0;
    double total_time = 0.0;

    threadpool pool   = thpool_init(16);

    while (!WindowShouldClose()) {
        for (int i = 0; i < worlds_count; i++) {
            scene_graph_update(worlds[i]->graph);

            b2World_Step(worlds[i]->id, GetFrameTime(), 4);
            for (int j = 0; j < dynamic_body_movements_length; j++) {
                Movement m = dynamic_body_movements[j];
                b2Vec2 p   = b2Body_GetPosition(m.body_id);
                scene_graph_position_set(worlds[i]->graph, m.node, (Position){p.x, p.y});
            }

            dynamic_body_movements_length = 0;

            scene_graph_compute_positions(worlds[i]->graph);
            scene_graph_ysort_parallel(worlds[i]->graph, pool);
        }

        BeginDrawing();
        ClearBackground(WHITE);

        for (int i = 0; i < worlds_count; i++) {
            scene_graph_render(worlds[i]->graph);
        }

        DrawFPS(10, 10);
        EndDrawing();
    }

    texture_free();
    CloseWindow();
}
