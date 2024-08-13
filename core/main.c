
#include <assert.h>
#include <complex.h>
#include <enet/enet.h>
#include <lauxlib.h>
#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <raylib.h>
#include <rlgl.h>
#include <stdio.h>
#include <stdlib.h>

#include "action.h"
#include "collision/box_collider.h"
#include "collision/collision_defs.h"
#include "collision/sparse_grid.h"
#include "global.h"
#include "level.h"
#include "lua_funcs.h"
#include "texture.h"
#include "ui/ui.h"

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

    if (luaL_loadfile(global.state, file) != LUA_OK) {
        fprintf(stderr, "Error: %s\n", lua_tostring(global.state, -1));
        lua_close(global.state);
        exit(EXIT_FAILURE);
    }

    lua_pcall(global.state, 0, 0, 0);
}

static void setup_host(UI_Base* base, void* userdata) {
    printf("Join Host\n");
}

static void setup_client(UI_Base* base, void* userdata) {
    printf("Join Client\n");
}

static void collision_test(void) {
    BoxCollider* ids[10000];
    size_t length       = 0;

    SparseGrid* grid    = spgrid_new();
    BoxCollider* ground = box_collider_new(0, 688, 1280, 32);
    spgrid_insert(grid, ground);

    double last_spawned = 0;
    while (!WindowShouldClose()) {
        float x                   = (16 * length) % 1280;
        BoxCollider* collider     = box_collider_new(x, 0, 8, 8);
        collider->type            = COLLIDER_TYPE_DYNAMIC;
        collider->gravity.enabled = true;
        spgrid_insert(grid, collider);
        last_spawned = GetTime();
        ids[length]  = collider;
        length++;

        float start = GetTime();
        spgrid_resolve(grid, GetFrameTime());
        float total = GetTime() - start;
        if (total >= 4.0) CloseWindow();

        BeginDrawing();
        ClearBackground(WHITE);

        char time[128];
        snprintf(time, sizeof(time), "Objects: %zu\nTime: %.2f", length, total);
        DrawText(time, 4, 4, 20, BLACK);

        for (int i = 0; i < length; i++) {
            BoxCollider* box = ids[i];
            DrawRectangle(box->position.x, box->position.y, box->size.x, box->size.y, RED);
        }

        DrawRectangle(0, 688, 1280, 32, BLACK);

        EndDrawing();
    }

    printf("Simulating %zu Objects!\n", length);
    exit(EXIT_SUCCESS);
}

int main(void) {
    InitWindow(1280, 720, "Temp Window");
    SetTargetFPS(60);

    texture_init();

    global.camera.offset   = (Vector2){0};
    global.camera.target   = (Vector2){0};
    global.camera.rotation = 0;
    global.camera.zoom     = 1.0;

    collision_test();

    action_register("move_up", 'w');
    action_register("move_left", 'a');
    action_register("move_right", 'd');
    action_register("move_down", 's');
    action_register("space", ' ');

    // NOTE: fix for blending of alpha colors on render textures
    rlSetBlendFactorsSeparate(0x0302, 0x0303, 1, 0x0303, 0x8006, 0x8006);

    RenderTexture rt = LoadRenderTexture(1280, 720);

    lua_init("scripts/main.lua");

    UI_Base* ui_base        = (void*)ui_new(UI_TYPE_BASE);
    UI_Button* ui_host      = (void*)ui_new(UI_TYPE_BUTTON);
    UI_Button* ui_client    = (void*)ui_new(UI_TYPE_BUTTON);

    ui_host->base.texture   = texture_load("assets/button.png");
    ui_host->onclick        = setup_host;

    ui_client->base.texture = texture_load("assets/button.png");
    ui_client->onclick      = setup_client;
    ui_client->base.y       = 64;

    ui_addchild(ui_base, (void*)ui_host);
    ui_addchild(ui_base, (void*)ui_client);

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_UP)) move_camera_up();
        if (IsKeyDown(KEY_DOWN)) move_camera_down();
        if (IsKeyDown(KEY_LEFT)) move_camera_left();
        if (IsKeyDown(KEY_RIGHT)) move_camera_right();

        if (IsMouseButtonPressed(0)) {
            float x = GetMouseX() / global.camera.zoom;
            float y = GetMouseY() / global.camera.zoom;
            ui_onclick(ui_base, x, y);
        }

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

        if (global.level) {
            level_update(global.level);
        }

        EndBlendMode();

        EndTextureMode();
        BeginDrawing();
        ClearBackground(WHITE);

        BeginMode2D(global.camera);
        float scale   = 2.4;
        Rectangle src = {0, 0, rt.texture.width, -rt.texture.height};
        Rectangle dst = {0, 0, GetScreenWidth(), GetScreenHeight()};
        DrawTexturePro(rt.texture, src, dst, (Vector2){0}, 0, WHITE);
        DrawFPS(0, 0);

        ui_draw(ui_base);

        EndMode2D();
        EndDrawing();
    }

    lua_close(global.state);
    texture_free();
    CloseWindow();
}
