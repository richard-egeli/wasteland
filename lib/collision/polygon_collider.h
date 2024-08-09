#ifndef LIB_COLLISION_POLYGON_COLLIDER_H_
#define LIB_COLLISION_POLYGON_COLLIDER_H_

#include <stdbool.h>
#include <stddef.h>

#include "collision/collision_defs.h"

typedef struct Polygon {
    Point position;
    float angle;
    Point* points;
    size_t points_length;
    Point* model;
    size_t model_length;
    bool overlap;
} Polygon;

typedef struct Triangle {
    Point p1, p2, p3;
} Triangle;

bool polygon_overlap(Polygon* r1, Polygon* r2);

void polygon_update(Polygon* p1);

Polygon* polygon_create(Point* model, size_t length);

#endif  // LIB_COLLISION_POLYGON_COLLIDER_H_
