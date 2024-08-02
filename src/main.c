#include <math.h>
#include <raylib.h>
#include <stdio.h>

#include "collision/collision.h"

int main() {
    InitWindow(1280, 768, "SAT");
    SetTargetFPS(60);

    BoxCollider c1 = box_collider_new(32, 32);

    BoxCollider floor[128];
    size_t count = sizeof(floor) / sizeof(*floor);
    for (int i = 0; i < count; i++) {
        floor[i]          = box_collider_new(32, 32);
        floor[i].position = (Point){32 * i, 300};
        box_collider_update(&floor[i]);
    }

    floor[10].position = (Point){200, 299};
    floor[11].position = (Point){180, 298};

    float gravity      = 0.1f;

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
        for (int i = 0; i < count; i++) {
            Color color = WHITE;
            if (box_collider_resolve(&c1, &floor[i])) {
                gravity = 0.1f;
                color   = RED;
            }

            Rect r2 = box_collider_rect(&floor[i]);
            DrawRectangle(r2.x, r2.y, r2.w, r2.h, color);
        }

        box_collider_update(&c1);
        Rect r1 = box_collider_rect(&c1);
        DrawRectangle(r1.x, r1.y, r1.w, r1.h, GREEN);

        EndDrawing();
    }

    return 0;
}
