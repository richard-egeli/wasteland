
#include <assert.h>
#include <complex.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <raylib.h>
#include <rlgl.h>
#include <stdio.h>
#include <stdlib.h>

#include "action.h"
#include "array/array.h"
#include "collision/box_collider.h"
#include "collision/collision_defs.h"
#include "collision/sparse_grid.h"
#include "entity.h"
#include "global.h"
#include "level.h"
#include "lua_funcs.h"
#include "texture.h"

static void on_collision(BoxCollider* box) {
    printf("Colliding\n");
}

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
    global.state = luaL_newstate();
    if (global.state == NULL) {
        fprintf(stderr, "Cannot create Lua state\n");
        exit(EXIT_FAILURE);
    }

    luaL_openlibs(global.state);
    lua_register_functions(global.state);

    if (luaL_loadfile(global.state, file)) {
        fprintf(stderr, "Error: %s\n", lua_tostring(global.state, -1));
        lua_close(global.state);
        exit(EXIT_FAILURE);
    }

    lua_pcall(global.state, 0, 0, 0);
}

// static Entity* create_player(void) {
//     RenderTexture rt = LoadRenderTexture(14, 14);
//     BeginTextureMode(rt);
//     ClearBackground(RED);
//     EndTextureMode();
//
//     Entity* player             = calloc(1, sizeof(*player));
//     player->collider           = box_collider_new(112, 190, 14, 14);
//     player->collider->type     = COLLIDER_TYPE_DYNAMIC;
//     player->sprite.texture     = rt.texture;
//     player->sprite.cell_width  = 14;
//     player->sprite.cell_height = 14;
//     player->sprite.grid_size   = 14;
//     player->sprite.sort_point  = 14;
//
//     return player;
// }

int main(void) {
    InitWindow(1280, 768, "Temp Window");
    SetTargetFPS(60);

    texture_init();

    lua_init("scripts/main.lua");
    // lua_register_functions();
    Level* level = level_new();
    // Entity* player = create_player();

    // array_push(level->entities, player);
    // spgrid_insert(level->sparse_grid, player->collider);

    // global.player          = player;
    global.camera.offset   = (Vector2){0};
    global.camera.target   = (Vector2){0};
    global.camera.rotation = 0;
    global.camera.zoom     = 1.0;
    global.level           = level;

    action_register("move_up", 'w');
    action_register("move_left", 'a');
    action_register("move_right", 'd');
    action_register("move_down", 's');

    // NOTE: fix for blending of alpha colors on render textures
    rlSetBlendFactorsSeparate(0x0302, 0x0303, 1, 0x0303, 0x8006, 0x8006);

    RenderTexture rt = LoadRenderTexture(1280, 768);

    level_load(level, "assets/test.ldtk", "Level_0");

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_UP)) move_camera_up();
        if (IsKeyDown(KEY_DOWN)) move_camera_down();
        if (IsKeyDown(KEY_LEFT)) move_camera_left();
        if (IsKeyDown(KEY_RIGHT)) move_camera_right();

        lua_getglobal(global.state, "update");
        if (lua_isfunction(global.state, -1)) {
            lua_pushnumber(global.state, GetFrameTime());
            if (lua_pcall(global.state, 1, 0, 0) != LUA_OK) {
                fprintf(stderr, "Error: %s\n", lua_tostring(global.state, -1));
            }
        } else {
            lua_pop(global.state, -1);
        }

        BeginTextureMode(rt);
        ClearBackground(WHITE);
        BeginBlendMode(BLEND_CUSTOM_SEPARATE);

        level_update(level);
        EndBlendMode();

        EndTextureMode();
        BeginDrawing();
        ClearBackground(WHITE);

        BeginMode2D(global.camera);
        float scale   = 2.4;
        Rectangle src = {0, 0, rt.texture.width, -rt.texture.height};
        Rectangle dst = {0, 0, 1280 * scale, 768 * scale};
        DrawTexturePro(rt.texture, src, dst, (Vector2){0}, 0, WHITE);

        EndMode2D();
        EndDrawing();
    }

    lua_close(global.state);
    CloseWindow();
}
