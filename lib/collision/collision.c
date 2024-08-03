#include "collision/collision.h"

static inline bool box_rect_overlap(Rect r1, Rect r2) {
    return !((r2.x > r1.x + r1.w) || (r1.x > r2.x + r2.w) ||
             (r2.y > r1.y + r1.h) || (r1.y > r2.y + r2.h));
}

inline Rect box_collider_rect(const BoxCollider *col) {
    return (Rect){
        .x = col->position.x - col->origin.x,
        .y = col->position.y - col->origin.y,
        .w = col->size.x,
        .h = col->size.y,
    };
}

inline Rect box_collider_bounds(const BoxCollider *col) {
    return (Rect){
        .x = col->position.x + col->velocity.x - col->origin.x,
        .y = col->position.y + col->velocity.y - col->origin.y,
        .w = col->size.x,
        .h = col->size.y,
    };
}

bool box_collider_overlap(BoxCollider *b1, BoxCollider *b2) {
    const Rect r1 = box_collider_bounds(b1);
    const Rect r2 = box_collider_bounds(b2);
    return box_rect_overlap(r1, r2);
}

static bool box_collider_slope(BoxCollider *b1, BoxCollider *b2) {
    const int max = 2;
    float pos     = b1->position.y + b1->size.y;
    float off     = pos - b2->position.y;
    if (off <= max) {
        b1->position.y -= off;
        return true;
    }

    return false;
}

bool box_collider_resolve(BoxCollider *b1, BoxCollider *b2) {
    if (box_collider_overlap(b1, b2)) {
        Point velocity = b1->velocity;

        b1->velocity.y = 0;
        b1->velocity.x = velocity.x;
        if (box_collider_overlap(b1, b2)) {
            if (!box_collider_slope(b1, b2)) {
                if (b1->position.x < b2->position.x) {
                    float pos  = b1->position.x + b1->size.x + b1->velocity.x;
                    float over = pos - b2->position.x;
                    velocity.x = b1->velocity.x - over - 0.0016f;
                    b1->collision.right = true;
                } else {
                    float pos          = b1->position.x + b1->velocity.x;
                    float over         = b2->position.x + b2->size.x - pos;
                    velocity.x         = b1->velocity.x + over + 0.0016f;
                    b1->collision.left = true;
                }
            }
        }

        b1->velocity.x = 0;
        b1->velocity.y = velocity.y;
        if (box_collider_overlap(b1, b2)) {
            if (b1->position.y < b2->position.y) {
                float pos  = b1->position.y + b1->size.y + b1->velocity.y;
                float over = pos - b2->position.y;
                velocity.y = b1->velocity.y - over - 0.005f;
                b1->collision.bottom = true;
            } else {
                float pos         = b1->position.y + b1->velocity.y;
                float over        = b2->position.y + b2->size.y - pos;
                velocity.y        = b1->velocity.y + over + 0.005f;
                b1->collision.top = true;
            }
        }

        b1->velocity = velocity;
        return true;
    } else {
        return false;
    }
}

void box_collider_update(BoxCollider *collider) {
    collider->position.x += collider->velocity.x;
    collider->position.y += collider->velocity.y;
    collider->velocity.y /= 1.1f;
    collider->velocity.x = 0;
}

BoxCollider box_collider_new(float width, float height) {
    return (BoxCollider){
        .size     = {width, height},
        .origin   = {0},
        .velocity = {0},
        .position = {0},
    };
}
