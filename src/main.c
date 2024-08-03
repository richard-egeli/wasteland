#include <math.h>
#include <raylib.h>
#include <stdio.h>

#include "collision/collision.h"
#include "collision/spatial_grid.h"

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
    BoxCollider floor[600];
    size_t count = sizeof(floor) / sizeof(*floor);
    for (int i = 0; i < count; i++) {
        floor[i]          = box_collider_new(32, 32);
        floor[i].position = (Point){32 * i, 300};
        box_collider_update(&floor[i]);
        spgrid_insert(grid, &floor[i]);
    }

    float gravity = 0.1f;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        gravity = fminf(gravity + 9.8f * GetFrameTime() * 0.1, 5);

        if (c1.velocity.y < 0) {
            c1.velocity.y += gravity;
            gravity = 0.1f;
        } else {
            c1.velocity.y += gravity;
        }

        if (IsKeyDown(KEY_S)) c1.velocity.y += 1;
        if (IsKeyDown(KEY_D)) c1.velocity.x += 3;
        if (IsKeyDown(KEY_A)) c1.velocity.x -= 3;

        if (c1.collision.bottom && IsKeyPressed(KEY_W)) {
            c1.velocity.y -= 10;
            printf("Jumping!\n");
        }

        c1.collision.top    = false;
        c1.collision.bottom = false;
        c1.collision.right  = false;
        c1.collision.left   = false;

        double start        = GetTime();
        BoxCollider* col;
        SPGridIter* iter = spgrid_iter(grid, box_collider_bounds(&c1));
        while ((col = spgrid_iter_next(&iter))) {
            if (box_collider_resolve(&c1, col)) {
                gravity = 0.1f;
            }
        }

        double end = GetTime() - start;
        printf("NEW WAY: %f\n", end);  // 0.000004

        BeginMode2D(camera);

        for (int y = 0; y < 10; y++) {
            for (int x = 0; x < 10; x++) {
                DrawRectangleLines(x * 128, y * 128, 128, 128, RED);
            }
        }

        // double start = GetTime();
        // for (int i = 0; i < count; i++) {
        //     if (box_collider_resolve(&c1, &floor[i])) {
        //         gravity = 0.1f;
        //     }
        // }
        //
        // double end = GetTime() - start;
        // printf("OLD WAY: %f\n", end);  // 0.000160

        for (int i = 0; i < count; i++) {
            Rect r2 = box_collider_rect(&floor[i]);
            DrawRectangle(r2.x, r2.y, r2.w, r2.h, WHITE);
        }

        box_collider_update(&c1);
        Rect r1 = box_collider_rect(&c1);
        DrawRectangle(r1.x, r1.y, r1.w, r1.h, GREEN);

        EndMode2D();

        EndDrawing();
    }

    return 0;
}
