#include <raylib.h>
#include <rlgl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array/array.h"
#include "collision/box_collider.h"
#include "collision/collision_defs.h"
#include "collision/sparse_grid.h"
#include "ldtk/ldtk.h"
#include "ldtk/tilemap.h"

static void on_collision(BoxCollider* box) {
    printf("Colliding\n");
}

int main(void) {
    InitWindow(1280, 768, "Temp Window");
    SetTargetFPS(60);

    LDTK_Root* root = ldtk_load(
        "assets/samples/Typical_2D_platformer_example.ldtk");

    LDTK_Level* level = NULL;
    LDTK_Layer* layer = NULL;

    for (int i = 0; i < array_length(root->levels); i++) {
        LDTK_Level* l = array_get(root->levels, i);

        if (strcmp(l->identifier, "Your_typical_2D_platformer") == 0) {
            level = l;
            break;
        }
    }

    for (int i = 0; i < array_length(level->layer_instances); i++) {
        LDTK_Layer* l = array_get(level->layer_instances, i);

        if (strcmp(l->__identifier, "Collisions") == 0) {
            layer = l;
            break;
        }
    }

    Tilemap* tilemap        = tilemap_from_ldtk(level);
    RenderTexture rt        = LoadRenderTexture(1280, 768);
    SPGrid* grid            = spgrid_new();
    BoxCollider* player     = box_collider_new(0, 0, 16, 16);

    player->type            = COLLIDER_TYPE_DYNAMIC;
    player->gravity.enabled = true;

    spgrid_insert(grid, player);

    for (int i = 0; i < layer->int_grid_csv_length; i++) {
        if (layer->int_grid_csv[i] > 0) {
            int x            = (i % layer->__c_width) * 16;
            int y            = (i / layer->__c_width) * 16;

            BoxCollider* col = box_collider_new(x, y, 16, 16);
            spgrid_insert(grid, col);
            DrawRectangle(x, y, 16, 16, RED);
        }
    }

    // NOTE: fix for blending of alpha colors on render textures
    rlSetBlendFactorsSeparate(0x0302, 0x0303, 1, 0x0303, 0x8006, 0x8006);

    Camera2D camera = {
        .offset   = {0},
        .target   = {0},
        .rotation = 0,
        .zoom     = 1.0,
    };

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_UP)) camera.target.y -= 3.0;
        if (IsKeyDown(KEY_DOWN)) camera.target.y += 3.0;
        if (IsKeyDown(KEY_LEFT)) camera.target.x -= 3.0;
        if (IsKeyDown(KEY_RIGHT)) camera.target.x += 3.0;
        if (IsKeyDown(KEY_D)) player->velocity.x += 2.0f;
        if (IsKeyDown(KEY_A)) player->velocity.x -= 2.0f;

        if (IsKeyPressed(KEY_W)) player->velocity.y -= 10;

        spgrid_resolve(grid, GetFrameTime());

        BeginTextureMode(rt);
        ClearBackground(WHITE);

        BeginBlendMode(BLEND_CUSTOM_SEPARATE);

        Tile* tile;
        TileIter it = tilemap_tile_iter(tilemap);
        while ((tile = tilemap_tile_next(&it))) {
            tilemap_tile_draw(tilemap, tile);
        }

        DrawRectangle(player->position.x, player->position.y, 16, 16, RED);
        EndBlendMode();

        EndTextureMode();
        BeginDrawing();
        ClearBackground(WHITE);

        BeginMode2D(camera);
        float scale   = 2;
        Rectangle src = {0, 0, rt.texture.width, -rt.texture.height};
        Rectangle dst = {0, 0, 1280 * scale, 768 * scale};
        DrawTexturePro(rt.texture, src, dst, (Vector2){0}, 0, WHITE);

        EndMode2D();

        EndDrawing();
    }

    CloseWindow();
}
