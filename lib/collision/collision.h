#ifndef COLLISION_COLLISION_H_
#define COLLISION_COLLISION_H_

#include <stdbool.h>

typedef struct Point {
    float x;
    float y;
} Point;

typedef struct Rect {
    float x;
    float y;
    float w;
    float h;
} Rect;

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
} BoxCollider;

bool box_collider_overlap(BoxCollider* b1, BoxCollider* b2);

bool box_collider_resolve(BoxCollider* b1, BoxCollider* b2);

void box_collider_update(BoxCollider* collider);

Rect box_collider_rect(const BoxCollider* col);

BoxCollider box_collider_new(float width, float height);

#endif  // COLLISION_COLLISION_H_
