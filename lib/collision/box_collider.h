#ifndef COLLISION_BOX_COLLIDER_H_
#define COLLISION_BOX_COLLIDER_H_

#include <stdbool.h>

#include "collision/collision_defs.h"

typedef struct BoxCollider {
    Point origin;
    Point position;
    Point velocity;
    Point size;
    struct {
        bool top;
        bool bottom;
        bool left;
        bool right;
    } collision;
    ColliderType type;
    float gravity;
} BoxCollider;

Rect box_collider_rect(const BoxCollider* col);

Rect box_collider_bounds(const BoxCollider* col);

bool box_collider_overlap(BoxCollider* b1, BoxCollider* b2);

void box_collider_resolve(BoxCollider* b1, BoxCollider* b2);

void box_collider_update(BoxCollider* collider);

BoxCollider box_collider_new(float width, float height);

#endif  // COLLISION_BOX_COLLIDER_H_