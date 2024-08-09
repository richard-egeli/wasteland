#include "polygon_collider.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

bool polygon_overlap(Polygon* r1, Polygon* r2) {
    Polygon* p1           = r1;
    Polygon* p2           = r2;

    float min_overlap     = INFINITY;
    float smallest_axis_x = 0.0f;
    float smallest_axis_y = 0.0f;

    for (int shape = 0; shape < 2; shape++) {
        if (shape == 1) {
            p1 = r2;
            p2 = r1;
        }

        for (int a = 0; a < p1->points_length; a++) {
            int b          = (a + 1) % p1->points_length;
            float axis_x   = -(p1->points[b].y - p1->points[a].y);
            float axis_y   = p1->points[b].x - p1->points[a].x;
            float axis_vec = sqrtf(axis_x * axis_x + axis_y * axis_y);
            float min_r1 = INFINITY, max_r1 = -INFINITY;
            float min_r2 = INFINITY, max_r2 = -INFINITY;

            axis_x /= axis_vec;
            axis_y /= axis_vec;

            for (int i = 0; i < p1->points_length; i++) {
                float q = (p1->points[i].x * axis_x + p1->points[i].y * axis_y);
                min_r1  = fminf(min_r1, q);
                max_r1  = fmaxf(max_r1, q);
            }

            for (int i = 0; i < p2->points_length; i++) {
                float q = (p2->points[i].x * axis_x + p2->points[i].y * axis_y);
                min_r2  = fminf(min_r2, q);
                max_r2  = fmaxf(max_r2, q);
            }

            float overlap = fminf(max_r1, max_r2) - fmaxf(min_r1, min_r2);
            if (overlap < 0) {
                return false;
            }

            if (overlap < min_overlap) {
                min_overlap     = overlap;
                smallest_axis_x = axis_x;
                smallest_axis_y = axis_y;
            }
        }
    }

    float x   = r2->position.x - r1->position.x;
    float y   = r2->position.y - r1->position.y;
    float mag = sqrtf(x * x + y * y);
    if (mag > 0) {
        float move_x = smallest_axis_x * min_overlap;
        float move_y = smallest_axis_y * min_overlap;

        if ((x * move_x + y * move_y) < 0) {
            move_x = -move_x;
            move_y = -move_y;
        }

        r1->position.x -= move_x;
        r1->position.y -= move_y;
    }

    return false;
}

void polygon_update(Polygon* p1) {
    for (int i = 0; i < p1->model_length; i++) {
        Point model   = p1->model[i];
        float angle   = p1->angle;
        float x       = (model.x * cosf(angle)) - (model.y * sinf(angle));
        float y       = (model.x * sinf(angle)) + (model.y * cosf(angle));
        p1->overlap   = false;
        p1->points[i] = (Point){
            .x = (x + p1->position.x),
            .y = (y + p1->position.y),
        };
    }
}

Polygon* polygon_create(Point* model, size_t length) {
    Polygon* poly       = malloc(sizeof(*poly));

    const size_t size   = sizeof(Point) * length;
    poly->points        = malloc(size);
    poly->model         = malloc(size);
    poly->position      = (Point){0};
    poly->points_length = length;
    poly->model_length  = length;
    poly->angle         = 0.f;
    memcpy(poly->model, model, size);

    return poly;
}
