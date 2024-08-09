#ifndef COLLISION_BOX_COLLIDER_H_
#define COLLISION_BOX_COLLIDER_H_

#include <stdbool.h>
#include <stdint.h>

#include "collision/collision_defs.h"

typedef struct BoxCollider {
    uint64_t id;
    uint32_t mask;
    Point origin;
    Point position;
    Point velocity;
    Point size;
    bool debug;
    struct {
        bool top;
        bool bottom;
        bool left;
        bool right;
    } collision;
    ColliderType type;
    bool trigger;
    struct {
        float accum;
        float force;
        bool enabled;
    } gravity;
    void (*on_collision)(struct BoxCollider* this);
} BoxCollider;

Rect box_collider_rect(const BoxCollider* col);

Rect box_collider_bounds(const BoxCollider* col);

bool box_collider_overlap(BoxCollider* b1, BoxCollider* b2);

void box_collider_resolve(BoxCollider* b1, BoxCollider* b2);

void box_collider_update(BoxCollider* collider);

void box_collider_free(BoxCollider* this);

BoxCollider* box_collider_new(float x, float y, float width, float height);

#endif  // COLLISION_BOX_COLLIDER_H_
