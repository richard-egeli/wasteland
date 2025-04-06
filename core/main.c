
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
#include "lua/animator.h"
#include "lua/asset_loader.h"
#include "lua/box_collider.h"
#include "lua/dynamic_body.h"
#include "lua/entity.h"
#include "lua/event_handler.h"
#include "lua/input.h"
#include "lua/sprite.h"
#include "lua/static_body.h"
#include "lua/world.h"
#include "mpx/layer.h"
#include "mpx/mpx.h"
#include "mpx/objectgroup.h"
#include "mpx/tiledata.h"
#include "mpx/tileset.h"
#include "scene-graph/parallel-graph-sort.h"
#include "scene-graph/scene-graph.h"
#include "thpool/thpool.h"
#include "vec/vec.h"

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
    register_static_body_api(L);
    register_box_collider_api(L);
    register_animator_api(L);
    register_sprite_api(L);

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

// Define the comparison function for sorting
int compare_by_ypos(const void* a, const void* b) {
    mpx_object_t* obj_a = *(mpx_object_t**)a;
    mpx_object_t* obj_b = *(mpx_object_t**)b;

    // Sort by ypos in ascending order
    if (obj_a->ypos < obj_b->ypos) return -1;
    if (obj_a->ypos > obj_b->ypos) return 1;
    return 0;
}

static void map_sort_objects(mpx_t* mpx) {
    for (size_t i = 0; i < vec_length(mpx->objectgroups); i++) {
        mpx_objectgroup_t* group = mpx->objectgroups[i];

        // Sort the objects in this group by ypos
        qsort(group->objects, vec_length(group->objects), sizeof(mpx_object_t*), compare_by_ypos);
    }
}

static void map_draw_objects(mpx_t* mpx, Texture* textures) {
    for (size_t i = 0; i < vec_length(mpx->objectgroups); i++) {
        mpx_objectgroup_t* group = mpx->objectgroups[i];
        if (!group->visible) continue;

        for (size_t j = 0; j < vec_length(group->objects); j++) {
            mpx_object_t* object   = group->objects[j];
            uint16_t tileset_id    = mpx_tile_tileset_get(object->tile);
            uint16_t tile_id       = mpx_tile_index_get(object->tile);
            mpx_tileset_t* tileset = mpx->tilesets[tileset_id];
            Rectangle sourceRect   = {
                  .x      = (tile_id % tileset->columns) * tileset->tilewidth,
                  .y      = (tile_id / tileset->columns) * tileset->tileheight,
                  .width  = tileset->tilewidth,
                  .height = tileset->tileheight,
            };

            printf("%d\n", tileset_id);
            Texture texture  = textures[tileset_id];
            Vector2 position = {object->xpos, object->ypos - tileset->tileheight};
            DrawTextureRec(texture, sourceRect, position, WHITE);
        }
    }
}

static void map_init(const char* file) {
    Texture textures[128] = {0};
    mpx_t* mpx            = mpx_load(file);
    if (mpx == NULL) {
        TraceLog(LOG_ERROR, "MPX: [%s] Map failed to load", file);
        exit(1);
    }

    TraceLog(LOG_INFO,
             "MPX: [%s] Map loaded successfully (Tilesets %zu | Layers %zu)",
             file,
             vec_length(mpx->tilesets),
             vec_length(mpx->layers));

    const char* root = "../mpxtiledconverter/assets";
    char* path       = malloc(1024);
    for (size_t i = 0; i < vec_length(mpx->tilesets); i++) {
        mpx_tileset_t* tileset = mpx->tilesets[i];
        snprintf(path, 1024, "%s/%s", root, tileset->filepath);
        textures[i] = LoadTexture(path);
    }

    // sleep(0xFFFFFFFF);
    free(path);

    map_sort_objects(mpx);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);

        size_t max = 0;
        size_t dw  = 0;
        size_t tw  = 0;
        size_t id  = 0;

        for (size_t i = 0; i < vec_length(mpx->layers); i++) {
            mpx_layer_t* layer   = mpx->layers[i];
            mpx_tiledata_t* data = layer->data;
            if (!layer->visible) continue;

            for (int j = 0; j < data->length; j++) {
                uint32_t tile = data->tiles[j];
                if (tile == MPX_TILE_NULL) continue;

                uint16_t tileset_id    = mpx_tile_tileset_get(tile);
                uint16_t tile_id       = mpx_tile_index_get(tile);
                mpx_tileset_t* tileset = mpx->tilesets[tileset_id];

                int xpos               = (j % data->width) * mpx->tilewidth;
                int ypos               = (j / data->width) * mpx->tileheight;
                Texture texture        = textures[tileset_id];

                Rectangle sourceRect   = {
                      .x      = (tile_id % tileset->columns) * tileset->tilewidth,
                      .y      = (tile_id / tileset->columns) * tileset->tileheight,
                      .width  = tileset->tilewidth,
                      .height = tileset->tileheight,
                };

                // Position on screen remains the same
                Vector2 position = {
                    .x = xpos,
                    .y = ypos,
                };

                DrawTextureRec(texture, sourceRect, position, WHITE);
            }
        }

        map_draw_objects(mpx, textures);

        DrawFPS(10, 10);
        EndDrawing();
    }
}

int main(void) {
    InitWindow(1280, 720, "Witch");
    SetWindowState(FLAG_WINDOW_TOPMOST | FLAG_WINDOW_RESIZABLE);
    SetWindowFocused();
    SetTargetFPS(60);
    srand((unsigned)time(NULL));

    lua_init("scripts/main.lua");
    map_init("../mpxtiledconverter/assets/fuck.mpx");
    return 0;

    for (int i = 0; i < worlds_count; i++) {
        scene_graph_compute_positions(worlds[i]->graph);
    }

    uint64_t count        = 0;
    threadpool pool       = thpool_init(16);
    const float time_step = 1.0f / 60.0f;

    while (!WindowShouldClose()) {
        for (int i = 0; i < worlds_count; i++) {
            World* world = worlds[i];

            scene_graph_update(world->graph);
            b2World_Step(world->id, time_step, 8);
            handle_collision_enter_events(world);
            handle_collision_exit_events(world);
            handle_movement_events(world);

            scene_graph_compute_positions(world->graph);
            scene_graph_ysort_parallel(world->graph, pool);
        }

        BeginDrawing();
        ClearBackground(WHITE);

        for (int i = 0; i < worlds_count; i++) {
            scene_graph_render(worlds[i]->graph);
        }

        DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
}
