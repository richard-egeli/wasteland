
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

#include "global.h"
#include "level.h"
#include "lua_funcs.h"
#include "scene-graph/scene-graph.h"
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
    DrawRectangle(position.x, position.y, 32, 32, (Color){r, g, b, 0xFF});
}

static void move_node(SceneGraph* graph, GameObject* object) {
    // Get the current position
    Position current = scene_graph_position_get(graph, object->node);

    // Define boundaries for the movement
    const int width_max  = 1280 - 16;  // Minus size of the node
    const int height_max = 720 - 16;   // Minus size of the node

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

    SceneGraph* graph      = scene_graph_new();
    Node root              = scene_graph_node_new(graph, NODE_NULL);
    Node player            = scene_graph_node_new(graph, root);
    GameObject* player_obj = scene_graph_game_object_new(graph, player);
    Drawable* player_draw  = scene_graph_drawable_new(graph, player);

    Node child             = scene_graph_node_new(graph, player);
    Drawable* child_draw   = scene_graph_drawable_new(graph, child);
    scene_graph_position_set(graph, child, (Position){0, 40});

    player_obj->update = move_player;
    player_draw->draw  = draw_player;
    child_draw->draw   = draw_child;

    for (int i = 0; i < 24; i++) {
        Node node       = scene_graph_node_new(graph, root);
        Drawable* draw  = scene_graph_drawable_new(graph, node);
        GameObject* obj = scene_graph_game_object_new(graph, node);

        int x           = rand() % 1266;
        int y           = rand() % 704;

        scene_graph_position_set(graph, node, (Position){x, y});
        // obj->update = move_node;
        draw->draw = draw_node;
        draw->data = malloc(3);
        uint8_t* v = (uint8_t*)draw->data;
        v[0]       = rand() % 256;
        v[1]       = rand() % 256;
        v[2]       = rand() % 256;
    }

    scene_graph_compute_positions(graph);

    while (!WindowShouldClose()) {
        scene_graph_update(graph);
        scene_graph_compute_positions(graph);

        BeginDrawing();
        ClearBackground(WHITE);
        scene_graph_render(graph);
        EndDrawing();
    }

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

        Position pos = scene_graph_position_get(graph, player);

        scene_graph_position_set(graph, root, (Position){pos.x + 0.1f, 0.0f});

        scene_graph_update(graph);
        scene_graph_compute_positions(graph);

        BeginDrawing();
        ClearBackground(WHITE);

        BeginMode2D(global.camera);
        BeginBlendMode(BLEND_CUSTOM_SEPARATE);
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
