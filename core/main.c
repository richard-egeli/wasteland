
#include <assert.h>
#include <enet/enet.h>
#include <lauxlib.h>
#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <math.h>
#include <raylib.h>
#include <rlgl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

static float calculate_rotated_height(float width, float height, float angle) {
    float radian_angle = angle * (M_PI / 180.0f);  // Convert angle to radians
    float sin_angle    = fabs(sin(radian_angle));
    float cos_angle    = fabs(cos(radian_angle));

    // Calculate the new height after rotation
    float rotated_height = (width * sin_angle) + (height * cos_angle);

    return rotated_height;
}

static void draw_cell_from_texture(Texture texture,
                                   int xcell,
                                   int ycell,
                                   int xmax,
                                   int ymax,
                                   Vector2 position,
                                   float rotation,
                                   float scale) {
    float wcell  = (float)texture.width / (float)xmax;
    float hcell  = (float)texture.height / (float)ymax;
    float txmin  = (float)xcell / (float)xmax;
    float tymin  = (float)ycell / (float)ymax;
    float txmax  = txmin + (1.0f / (float)xmax);
    float tymax  = tymin + (1.0f / (float)ymax);

    float sx     = wcell * scale;
    float sy     = hcell * scale;
    float height = calculate_rotated_height(sx, sy, 45.0) / 2;

    rlDisableDepthMask();
    rlPushMatrix();
    rlSetTexture(texture.id);
    rlTranslatef(position.y, height, position.x);
    rlRotatef(rotation, 0.0f, 0.0f, 1.0f);

    rlBegin(RL_QUADS);
    rlColor4ub(255, 255, 255, 255);

    rlTexCoord2f(txmin, tymax);
    rlVertex3f(-1.0 * sx, 0.0, -1.0 * sy);

    rlTexCoord2f(txmax, tymax);
    rlVertex3f(-1.0 * sx, 0.0, 1.0 * sy);

    rlTexCoord2f(txmax, tymin);
    rlVertex3f(1.0 * sx, 0.0, 1.0 * sy);

    rlTexCoord2f(txmin, tymin);
    rlVertex3f(1.0 * sx, 0.0, -1.0 * sy);

    rlEnd();
    rlSetTexture(0);
    rlPopMatrix();

    rlEnableDepthMask();
}

static void draw_quad_with_texture(Texture texture, float rotation, float scale) {
    rlSetTexture(texture.id);
    rlPushMatrix();
    rlRotatef(rotation, 0.0f, 0.0f, 1.0f);

    rlBegin(RL_QUADS);
    rlColor4ub(255, 255, 255, 255);

    float sx = texture.width * scale;
    float sy = texture.height * scale;

    rlTexCoord2f(0.0, 1.0);
    rlVertex3f(-1.0 * sx, 0.0, -1.0 * sy);

    rlTexCoord2f(1.0, 1.0);
    rlVertex3f(-1.0 * sx, 0.0, 1.0 * sy);

    rlTexCoord2f(1.0, 0.0);
    rlVertex3f(1.0 * sx, 0.0, 1.0 * sy);

    rlTexCoord2f(0.0, 0.0);
    rlVertex3f(1.0 * sx, 0.0, -1.0 * sy);

    rlEnd();
    rlPopMatrix();
    rlSetTexture(0);
}

typedef struct Object {
    Texture texture;
    Vector2 position;
} Object;

static int sort_objects(const void* p1, const void* p2) {
    const Object* o1 = *(Object**)p1;
    const Object* o2 = *(Object**)p2;

    if (o1->position.y < o2->position.y) {
        return 1;
    } else if (o1->position.y > o2->position.y) {
        return -1;
    }

    return 0;
}

static void temp() {
    Camera3D camera    = {0};
    camera.projection  = CAMERA_PERSPECTIVE;
    camera.fovy        = 10.0f;
    camera.position    = (Vector3){-10, 10, 0};
    camera.target      = (Vector3){0, 0, 0};
    camera.up          = (Vector3){0, 1, 0};

    Shader shader      = LoadShader(0, "assets/shaders/jitter.glsl");
    Texture background = LoadTexture("assets/test/png/Level_0.png");

    Object player      = {
             .position = {0},
             .texture  = LoadTexture("assets/Unarmed_Idle_full.png"),
    };

    Object empty = {
        .position = {0},
        .texture  = LoadTexture("assets/Unarmed_Idle_full.png"),

    };

    Object* objects[] = {
        &player,
        &empty,
    };

    size_t objects_length = sizeof(objects) / sizeof(void*);

    while (!WindowShouldClose()) {
        const float camera_speed = 0.5f;
        const float player_speed = 0.5f;
        Vector3 movement         = {0};
        if (IsKeyDown(KEY_UP)) movement.x += camera_speed;
        if (IsKeyDown(KEY_DOWN)) movement.x -= camera_speed;
        if (IsKeyDown(KEY_LEFT)) movement.y -= camera_speed;
        if (IsKeyDown(KEY_RIGHT)) movement.y += camera_speed;

        if (IsKeyDown(KEY_A)) player.position.x -= player_speed;
        if (IsKeyDown(KEY_D)) player.position.x += player_speed;
        if (IsKeyDown(KEY_S)) player.position.y -= player_speed;
        if (IsKeyDown(KEY_W)) player.position.y += player_speed;

        UpdateCameraPro(&camera, movement, (Vector3){0}, -GetMouseWheelMove());
        qsort(objects, objects_length, sizeof(void*), sort_objects);

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(camera);
        BeginShaderMode(shader);

        const float scale = 0.1f;
        draw_quad_with_texture(background, 0.0, scale);

        for (int i = 0; i < objects_length; i++) {
            draw_cell_from_texture(objects[i]->texture,
                                   0,
                                   0,
                                   12,
                                   4,
                                   objects[i]->position,
                                   45,
                                   scale);
        }
        /*draw_cell_from_texture(player, 0, 0, 12, 4, (Vector2){0}, 45, 0.1f);*/

        EndShaderMode();
        EndMode3D();
        EndDrawing();
    }

    exit(EXIT_SUCCESS);
}

static float lerp(float v0, float v1, float t) {
    return (1 - t) * v0 + t * v1;
}

static Vector2 vector_lerp(Vector2 v0, Vector2 v1, float t) {
    return (Vector2){
        .x = (1 - t) * v0.x + t * v1.x,
        .y = (1 - t) * v1.x * t * v1.y,
    };
}

int main(void) {
    InitWindow(1280, 720, "Temp Window");
    SetTargetFPS(60);

    temp();

    texture_init();

    global.camera.offset   = (Vector2){0};
    global.camera.target   = (Vector2){0};
    global.camera.rotation = 0;
    global.camera.zoom     = 2.0;

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
