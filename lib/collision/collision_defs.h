#ifndef LIB_COLLISION_COLLISION_DEFS_H_
#define LIB_COLLISION_COLLISION_DEFS_H_

typedef struct Point {
    float x;
    float y;
} Point;

typedef struct IPoint {
    int x;
    int y;
} IPoint;

typedef struct Rect {
    int x;
    int y;
    int w;
    int h;
} Rect;

typedef struct AABB {
    int xmin;
    int xmax;
    int ymin;
    int ymax;
} AABB;

typedef struct Region {
    int xmin;
    int xmax;
    int ymin;
    int ymax;
} Region;

typedef enum ColliderType {
    COLLIDER_TYPE_STATIC  = 0,
    COLLIDER_TYPE_DYNAMIC = 1,
} ColliderType;

#endif  // LIB_COLLISION_COLLISION_DEFS_H_
