#include "collision/box_collider.h"

#include <math.h>
#include <stdlib.h>

#include "collision/collision_defs.h"

static inline bool box_rect_overlap(Rect r1, Rect r2) {
    return !((r2.x >= r1.x + r1.w) || (r1.x >= r2.x + r2.w) || (r2.y >= r1.y + r1.h) ||
             (r1.y >= r2.y + r2.h));
}

static inline IPoint box_position(const BoxCollider *box) {
    return (IPoint){
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
    int xv = (int)col->velocity.x;
    int yv = (int)col->velocity.y;

    return (Rect){
        .x = col->position.x + col->origin.x + xv,
        .y = col->position.y + col->origin.y + yv,
        .w = col->size.x,
        .h = col->size.y,
    };
}

bool box_collider_overlap(BoxCollider *b1, BoxCollider *b2) {
    if (b1->enabled && b2->enabled) {
        const Rect r1 = box_collider_bounds(b1);
        const Rect r2 = box_collider_rect(b2);
        return box_rect_overlap(r1, r2);
    }

    return false;
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

void box_collider_resolve_x(BoxCollider *p1, BoxCollider *p2) {
    BoxCollider *b1 = p1;
    BoxCollider *b2 = p2;

    if (!(b1->mask & b2->mask) || b1->trigger) return;

    float temp     = b1->velocity.y;
    b1->velocity.y = 0;

    if (box_collider_overlap(b1, b2)) {
        if (b2->trigger) {
            if (b2->on_collision) {
                b2->on_collision(b2, b1);
            }

            b1->velocity.y = temp;
            return;
        }

        if (b1->type == COLLIDER_TYPE_DYNAMIC && b2->type == COLLIDER_TYPE_DYNAMIC) {
            Point v1 = b1->velocity;
            Point v2 = b2->velocity;
            float m1 = sqrtf(v1.x * v1.x + v1.y * v1.y);
            float m2 = sqrtf(v2.x * v2.x + v2.y * v2.y);
            if (m2 > m1) {
                b1 = p2;
                b2 = p1;
            }
        }

        IPoint p1 = box_position(b1);
        IPoint p2 = box_position(b2);
        if (p1.x < p2.x) {
            float speed = b1->velocity.x;
            float pos   = (float)(p1.x + b1->size.x) + speed;
            float over  = pos - (float)p2.x;
            b1->velocity.x -= over;
            b1->collision.right = true;
        } else {
            float speed = b1->velocity.x;
            float pos   = (float)p1.x + speed;
            float over  = (float)(p2.x + b2->size.x) - pos;
            b1->velocity.x += over;
            b1->collision.left = true;
        }
    }

    b1->velocity.y = temp;
}

void box_collider_resolve_y(BoxCollider *p1, BoxCollider *p2) {
    BoxCollider *b1 = p1;
    BoxCollider *b2 = p2;

    if (!(b1->mask & b2->mask) || b1->trigger) return;

    if (box_collider_overlap(b1, b2)) {
        if (b2->trigger) {
            if (b2->on_collision) {
                b2->on_collision(b2, b1);
            }
            return;
        }

        if (b1->type == COLLIDER_TYPE_DYNAMIC && b2->type == COLLIDER_TYPE_DYNAMIC) {
            Point v1 = b1->velocity;
            Point v2 = b2->velocity;
            float m1 = sqrtf(v1.x * v1.x + v1.y * v1.y);
            float m2 = sqrtf(v2.x * v2.x + v2.y * v2.y);
            if (m2 > m1) {
                b1 = p2;
                b2 = p1;
            }
        }

        IPoint p1 = box_position(b1);
        IPoint p2 = box_position(b2);

        if (p1.y < p2.y) {
            float speed = b1->velocity.y;
            float pos   = (float)p1.y + speed;
            float over  = (float)pos - (p2.y - b1->size.y);
            b1->velocity.y -= over;
            b1->collision.bottom = true;
        } else {
            float speed = b1->velocity.y;
            float pos   = (float)p1.y + speed;
            float over  = (float)(p2.y + b2->size.y) - pos;
            b1->velocity.y += over;
            b1->collision.top = true;
        }
    }
}

void box_collider_update(BoxCollider *collider) {
    collider->position.x += (int)collider->velocity.x;
    collider->velocity.x -= (int)collider->velocity.x;

    collider->position.y += (int)collider->velocity.y;

    if (collider->gravity.enabled) {
        collider->velocity.y /= 1.1f;
    } else {
        collider->velocity.y -= (int)collider->velocity.y;
    }
}

void box_collider_free(BoxCollider *this) {
    free(this);
}

BoxCollider *box_collider_new(int x, int y, int width, int height) {
    BoxCollider *col   = calloc(1, sizeof(*col));
    col->mask          = 0xFFFFFFFF;
    col->size          = (IPoint){width, height};
    col->position      = (IPoint){x, y};
    col->type          = COLLIDER_TYPE_STATIC;
    col->gravity.force = 0.98f;
    col->enabled       = true;
    return col;
}
