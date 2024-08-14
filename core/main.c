
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
#include <time.h>

#include "action.h"
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

static void temp() {
    Camera3D camera   = {0};
    camera.projection = CAMERA_PERSPECTIVE;
    camera.fovy       = 30.0f;
    camera.position   = (Vector3){-10, 10, 0};
    camera.target     = (Vector3){0, 0, 0};
    camera.up         = (Vector3){0, 1, 0};

    /*Shader shader      = LoadShader("assets/shaders/jitter.vs", "assets/shaders/jitter.fs");*/
    Texture background = LoadTexture("assets/test/png/Level_0.png");
    SetTextureFilter(background, TEXTURE_FILTER_POINT);

    while (!WindowShouldClose()) {
        Vector3 movement = {0};
        if (IsKeyDown(KEY_W)) movement.x += 0.05f;
        if (IsKeyDown(KEY_S)) movement.x -= 0.05f;
        if (IsKeyDown(KEY_A)) movement.y -= 0.05f;
        if (IsKeyDown(KEY_D)) movement.y += 0.05f;

        UpdateCameraPro(&camera, movement, (Vector3){0}, -GetMouseWheelMove());

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(camera);
        /*BeginShaderMode(shader);*/

        rlSetTexture(background.id);

        /*Matrix mvp     = GetCameraMatrix(camera);*/
        /*float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};*/
        /**/
        /*SetShaderValue(shader, GetShaderLocation(shader, "color"), color, SHADER_UNIFORM_VEC4);*/
        /*SetShaderValueMatrix(shader, GetShaderLocation(shader, "modelViewProjection"), mvp);*/
        /*SetShaderValueTexture(shader, GetShaderLocation(shader, "texture0"), background);*/
        /**/
        /*float texel_size[2] = {1.0f / background.width, 1.0f / background.height};*/
        /*SetShaderValue(shader,*/
        /*               GetShaderLocation(shader, "texelSize"),*/
        /*               texel_size,*/
        /*               SHADER_UNIFORM_VEC2);*/

        rlBegin(RL_QUADS);
        rlColor4ub(255, 255, 255, 255);

        float scale = 20;

        rlTexCoord2f(0.0, 1.0);
        rlVertex3f(-1.0 * scale, 0.0, -1.0 * scale);

        rlTexCoord2f(1.0, 1.0);
        rlVertex3f(-1.0 * scale, 0.0, 1.0 * scale);

        rlTexCoord2f(1.0, 0.0);
        rlVertex3f(1.0 * scale, 0.0, 1.0 * scale);

        rlTexCoord2f(0.0, 0.0);
        rlVertex3f(1.0 * scale, 0.0, -1.0 * scale);

        rlEnd();
        rlSetTexture(0);

        /*EndShaderMode();*/
        EndMode3D();
        EndDrawing();
    }

    exit(EXIT_SUCCESS);
}

int main(void) {
    InitWindow(1280, 720, "Temp Window");
    SetTargetFPS(60);

    /*temp();*/

    texture_init();

    global.camera.offset   = (Vector2){0};
    global.camera.target   = (Vector2){0};
    global.camera.rotation = 0;
    global.camera.zoom     = 2.0;

    action_register("move_up", 'w');
    action_register("move_left", 'a');
    action_register("move_right", 'd');
    action_register("move_down", 's');
    action_register("space", ' ');

    // NOTE: fix for blending of alpha colors on render textures
    rlSetBlendFactorsSeparate(0x0302, 0x0303, 1, 0x0303, 0x8006, 0x8006);

    RenderTexture rt = LoadRenderTexture(1280, 720);
    lua_init("scripts/main.lua");

    UI_Base* ui_base      = (void*)ui_new(UI_TYPE_BASE);
    UI_Button* ui_host    = (void*)ui_new(UI_TYPE_BUTTON);
    UI_Button* ui_client  = (void*)ui_new(UI_TYPE_BUTTON);

    ui_host->base.texture = texture_load("assets/button.png");
    /*ui_host->onclick        = setup_host;*/

    ui_client->base.texture = texture_load("assets/button.png");
    /*ui_client->onclick      = setup_client;*/
    ui_client->base.y = 64;

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

        BeginDrawing();
        ClearBackground(WHITE);

        BeginMode2D(global.camera);
        BeginBlendMode(BLEND_CUSTOM_SEPARATE);
        float scale   = 2.4;
        Rectangle src = {0, 0, rt.texture.width, -rt.texture.height};
        Rectangle dst = {0, 0, GetScreenWidth(), GetScreenHeight()};

        if (global.level) {
            level_update(global.level);
        }

        DrawTexturePro(rt.texture, src, dst, (Vector2){0}, 0, WHITE);
        DrawFPS(0, 0);

        EndBlendMode();
        EndMode2D();
        EndDrawing();
    }

    lua_close(global.state);
    texture_free();
    CloseWindow();
}
