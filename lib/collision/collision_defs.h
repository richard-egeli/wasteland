#ifndef LIB_COLLISION_COLLISION_DEFS_H_
#define LIB_COLLISION_COLLISION_DEFS_H_

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

typedef enum ColliderType {
    COLLIDER_TYPE_STATIC  = 0,
    COLLIDER_TYPE_DYNAMIC = 1,
} ColliderType;

#endif  // LIB_COLLISION_COLLISION_DEFS_H_
