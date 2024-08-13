#ifndef COLLISION_BOX_COLLIDER_H_
#define COLLISION_BOX_COLLIDER_H_

#include <stdbool.h>
#include <stdint.h>

#include "collision/collision_defs.h"

typedef struct BoxCollider {
    uint64_t id;
    uint32_t mask;
    IPoint origin;
    IPoint position;
    IPoint size;
    Point velocity;
    bool debug;
    bool trigger;
    bool enabled;
    ColliderType type;
    void (*on_collision)(struct BoxCollider* this, struct BoxCollider* target);
    struct {
        bool top;
        bool bottom;
        bool left;
        bool right;
    } collision;
    struct {
        float accum;
        float force;
        bool enabled;
    } gravity;
} BoxCollider;

Rect box_collider_rect(const BoxCollider* col);

Rect box_collider_bounds(const BoxCollider* col);

bool box_collider_overlap(BoxCollider* b1, BoxCollider* b2);

void box_collider_resolve_y(BoxCollider* p1, BoxCollider* p2);

void box_collider_resolve_x(BoxCollider* p1, BoxCollider* p2);

void box_collider_resolve(BoxCollider* b1, BoxCollider* b2);

void box_collider_update(BoxCollider* collider);

void box_collider_free(BoxCollider* this);

BoxCollider* box_collider_new(int x, int y, int width, int height);

#endif  // COLLISION_BOX_COLLIDER_H_
