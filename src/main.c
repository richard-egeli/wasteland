#include <raylib.h>
#include <stdio.h>

#include "collision/box_collider.h"
#include "collision/collision_defs.h"
#include "collision/sparse_grid.h"

int main() {
    InitWindow(1280, 768, "SAT");
    SetTargetFPS(60);

    Camera2D camera = {
        .offset   = {0},
        .target   = {0},
        .rotation = 0,
        .zoom     = 1.0,
    };

    SPGrid* grid   = spgrid_new();
    BoxCollider c1 = box_collider_new(32, 32);
    BoxCollider c2 = box_collider_new(32, 32);
    c1.type        = COLLIDER_TYPE_DYNAMIC;
    c1.position    = (Point){0, 268};
    c2.type        = COLLIDER_TYPE_DYNAMIC;
    c2.position    = (Point){64, 268};

    BoxCollider floor[64];
    size_t count = sizeof(floor) / sizeof(*floor);
    for (int i = 0; i < count; i++) {
        floor[i]          = box_collider_new(32, 32);
        floor[i].position = (Point){32 * i, 300};
        box_collider_update(&floor[i]);
        spgrid_insert(grid, &floor[i]);
    }

    spgrid_remove(grid, &floor[10]);
    spgrid_remove(grid, &floor[11]);

    floor[10].position = (Point){300, 268};
    floor[11].position = (Point){300, 236};

    spgrid_insert(grid, &floor[10]);
    spgrid_insert(grid, &floor[11]);
    spgrid_insert(grid, &c1);
    spgrid_insert(grid, &c2);

    printf("%f %f\n", c1.velocity.x, c1.velocity.y);

    float gravity = 0.1f;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        if (IsKeyDown(KEY_D)) c1.velocity.x += 3;
        if (IsKeyDown(KEY_A)) c1.velocity.x -= 3;

        if (IsKeyDown(KEY_RIGHT)) c2.velocity.x += 3;
        if (IsKeyDown(KEY_LEFT)) c2.velocity.x -= 3;

        if (c1.collision.bottom && IsKeyPressed(KEY_W)) {
            c1.velocity.y -= 10;
        }

        if (c2.collision.bottom && IsKeyPressed(KEY_UP)) {
            c2.velocity.y -= 10;
        }

        spgrid_resolve(grid, GetFrameTime());

        BeginMode2D(camera);

        for (int y = 0; y < 10; y++) {
            for (int x = 0; x < 10; x++) {
                DrawRectangleLines(x * 128, y * 128, 128, 128, RED);
            }
        }

        for (int i = 0; i < count; i++) {
            Rect r2 = box_collider_rect(&floor[i]);
            DrawRectangle(r2.x, r2.y, r2.w, r2.h, WHITE);
        }

        box_collider_update(&c1);
        box_collider_update(&c2);
        Rect r1 = box_collider_rect(&c1);
        DrawRectangle(r1.x, r1.y, r1.w, r1.h, GREEN);
        Rect r2 = box_collider_rect(&c2);
        DrawRectangle(r2.x, r2.y, r2.w, r2.h, GREEN);

        EndMode2D();

        EndDrawing();
    }

    return 0;
}
