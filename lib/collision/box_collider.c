#include "collision/box_collider.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "collision/collision_defs.h"

static inline bool box_rect_overlap(Rect r1, Rect r2) {
    return !((r2.x > r1.x + r1.w) || (r1.x > r2.x + r2.w) ||
             (r2.y > r1.y + r1.h) || (r1.y > r2.y + r2.h));
}

static inline Point box_position(const BoxCollider *box) {
    return (Point){
        box->position.x + box->origin.x,
        box->position.y + box->origin.y,
    };
}

inline Rect box_collider_rect(const BoxCollider *col) {
    return (Rect){
        .x = col->position.x + col->origin.x,
        .y = col->position.y + col->origin.y,
        .w = col->size.x,
        .h = col->size.y,
    };
}

inline Rect box_collider_bounds(const BoxCollider *col) {
    return (Rect){
        .x = col->position.x + col->velocity.x + col->origin.x,
        .y = col->position.y + col->velocity.y + col->origin.y,
        .w = col->size.x,
        .h = col->size.y,
    };
}

bool box_collider_overlap(BoxCollider *b1, BoxCollider *b2) {
    const Rect r1 = box_collider_bounds(b1);
    const Rect r2 = box_collider_rect(b2);
    return box_rect_overlap(r1, r2);
}

static bool box_collider_slope(BoxCollider *b1, BoxCollider *b2) {
    if (b1->gravity.enabled) {
        const int max = 2;
        float pos     = b1->position.y + b1->size.y;
        float off     = pos - b2->position.y;
        if (off <= max) {
            b1->position.y -= off;
            return true;
        }
    }

    return false;
}

void box_collider_resolve(BoxCollider *p1, BoxCollider *p2) {
    BoxCollider *b1 = p1;
    BoxCollider *b2 = p2;

    // make sure that they collide on the same layers
    if (!(b1->mask & b2->mask)) return;

    // we don't check if triggers overlap, only if non-triggers overlap triggers
    if (b1->trigger) return;

    if (box_collider_overlap(b1, b2)) {
        Point velocity = b1->velocity;

        if (b2->trigger) {
            if (b2->on_collision) {
                b2->on_collision(b2, b1);
            }

            return;
        }

        // try to solve whichever has the highest velocity
        // this is a temp fix for colliding with dynamic objects, and it barely
        // works
        if (b1->type == COLLIDER_TYPE_DYNAMIC &&
            b2->type == COLLIDER_TYPE_DYNAMIC) {
            Point v1 = b1->velocity;
            Point v2 = b2->velocity;
            float m1 = sqrtf(v1.x * v1.x + v1.y * v1.y);
            float m2 = sqrtf(v2.x * v2.x + v2.y * v2.y);
            if (m2 > m1) {
                b1 = p2;
                b2 = p1;
            }
        }

        b1->velocity.y = 0;
        b1->velocity.x = velocity.x;
        if (box_collider_overlap(b1, b2)) {
            if (!box_collider_slope(b1, b2)) {
                Point p1 = box_position(b1);
                Point p2 = box_position(b2);
                if (p1.x < p2.x) {
                    float pos           = p1.x + b1->size.x + b1->velocity.x;
                    float over          = pos - p2.x;
                    velocity.x          = b1->velocity.x - over - 0.00016f;
                    b1->collision.right = true;
                } else {
                    float pos          = p1.x + b1->velocity.x;
                    float over         = p2.x + b2->size.x - pos;
                    velocity.x         = b1->velocity.x + over + 0.00016f;
                    b1->collision.left = true;
                }
            }
        }

        b1->velocity.x = 0;
        b1->velocity.y = velocity.y;
        if (box_collider_overlap(b1, b2)) {
            Point p1 = box_position(b1);
            Point p2 = box_position(b2);
            if (p1.y < p2.y) {
                float pos            = p1.y + b1->size.y + b1->velocity.y;
                float over           = pos - p2.y;
                velocity.y           = b1->velocity.y - over - 0.00016f;
                b1->collision.bottom = true;
            } else {
                float pos         = p1.y + b1->velocity.y;
                float over        = p2.y + b2->size.y - pos;
                velocity.y        = b1->velocity.y + over + 0.00016f;
                b1->collision.top = true;
            }
        }

        b1->velocity = velocity;
    }
}

void box_collider_update(BoxCollider *collider) {
    collider->position.x += collider->velocity.x;
    collider->position.y += collider->velocity.y;
    collider->velocity.x = 0;
    if (collider->gravity.enabled) {
        collider->velocity.y /= 1.1f;
    } else {
        collider->velocity.y = 0;
    }
}

void box_collider_free(BoxCollider *this) {
    free(this);
}

BoxCollider *box_collider_new(float x, float y, float width, float height) {
    BoxCollider *col   = calloc(1, sizeof(*col));
    col->mask          = 0xFFFFFFFF;
    col->size          = (Point){width, height};
    col->position      = (Point){x, y};
    col->type          = COLLIDER_TYPE_STATIC;
    col->gravity.force = 0.98f;
    return col;
}
