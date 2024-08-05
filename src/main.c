#include <raylib.h>
#include <rlgl.h>
#include <stdio.h>

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

    LDTK_Level* level   = ldtk_level_get(root, "Your_typical_2D_platformer");
    LDTK_Layer* layer   = ldtk_layer_get(level, "Collisions");

    Tilemap* tilemap    = tilemap_from_ldtk(level);
    RenderTexture rt    = LoadRenderTexture(1280, 768);
    SPGrid* grid        = spgrid_new();
    BoxCollider* player = box_collider_new(112, 190, 15, 15);

    player->type        = COLLIDER_TYPE_DYNAMIC;
    player->gravity.enabled = true;

    spgrid_insert(grid, player);

    LDTK_IntGridValue* ig;
    LDTK_IntGridIter ig_iter = ldtk_intgrid_iter(level, "Collisions");
    while ((ig = ldtk_intgrid_next(&ig_iter))) {
        BoxCollider* col = box_collider_new(ig->x, ig->y, ig->s, ig->s);
        spgrid_insert(grid, col);
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
        float scale   = 2.4;
        Rectangle src = {0, 0, rt.texture.width, -rt.texture.height};
        Rectangle dst = {0, 0, 1280 * scale, 768 * scale};
        DrawTexturePro(rt.texture, src, dst, (Vector2){0}, 0, WHITE);

        EndMode2D();

        EndDrawing();
    }

    CloseWindow();
}
