#include "spatial_grid.h"

#include <stdio.h>
#include <stdlib.h>

#include "array/array.h"
#include "collision/collision.h"
#include "hashmap/hashmap.h"

typedef struct SpatialGrid {
    HashMap* grid_map;
} SPGrid;

typedef struct SPGridIter {
    int index;
    Array* array;
    struct SPGridIter* next;
} SPGridIter;

void spgrid_insert(SPGrid* this, BoxCollider* box) {
    Rect b      = box_collider_rect(box);
    int x_start = (b.x) / SPATIAL_GRID_SIZE;
    int y_start = (b.y) / SPATIAL_GRID_SIZE;
    int x_end   = (b.x + b.w) / SPATIAL_GRID_SIZE;
    int y_end   = (b.y + b.h) / SPATIAL_GRID_SIZE;

    char grid_key[64];
    for (int y = y_start; y <= y_end; y++) {
        for (int x = x_start; x <= x_end; x++) {
            Array* array;
            snprintf(grid_key, sizeof(grid_key), "%d,%d", x, y);

            if (!hmap_get(this->grid_map, grid_key, (void*)&array)) {
                array = array_new();
                if (!hmap_put(this->grid_map, grid_key, array)) {
                    perror("failed to insert new array into spatial grid");
                    array_free(array);
                    return;
                }
            }

            if (!array_has(array, box)) {
                array_push(array, box);
            }
        }
    }
}

SPGridIter* spgrid_iter(const SPGrid* this, Rect b) {
    int x_start      = (b.x) / SPATIAL_GRID_SIZE;
    int y_start      = (b.y) / SPATIAL_GRID_SIZE;
    int x_end        = (b.x + b.w) / SPATIAL_GRID_SIZE;
    int y_end        = (b.y + b.h) / SPATIAL_GRID_SIZE;
    SPGridIter* iter = NULL;

    char grid_key[64];
    for (int y = y_start; y <= y_end; y++) {
        for (int x = x_start; x <= x_end; x++) {
            Array* array;
            snprintf(grid_key, sizeof(grid_key), "%d,%d", x, y);

            if (hmap_get(this->grid_map, grid_key, (void**)&array)) {
                SPGridIter* new_iter = malloc(sizeof(*new_iter));
                if (new_iter != NULL) {
                    new_iter->next  = iter;
                    new_iter->array = array;
                    new_iter->index = 0;
                    iter            = new_iter;
                }
            }
        }
    }

    return iter;
}

void* spgrid_iter_next(SPGridIter** this) {
    while (*this != NULL) {
        SPGridIter* it = *this;
        while (it->index < array_length(it->array)) {
            void* value = array_get(it->array, it->index);
            it->index += 1;
            if (value != NULL) {
                return value;
            }
        }

        *this = it->next;
        free(it);
    }

    return NULL;
}

void spgrid_free(SPGrid* this) {
    hmap_free(this->grid_map);
    free(this);
}

SPGrid* spgrid_new(void) {
    SPGrid* sp = malloc(sizeof(*sp));
    if (sp == NULL) {
        perror("failed to create new spatial grid");
        return NULL;
    }

    sp->grid_map = hmap_new();
    if (sp->grid_map == NULL) {
        perror("failed to create grid map for spatial grid");
        free(sp);
        return NULL;
    }

    return sp;
}
