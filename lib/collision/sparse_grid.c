#include "collision/sparse_grid.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "array/array.h"
#include "collision/box_collider.h"
#include "collision/collision_defs.h"
#include "hashmap/hashmap.h"

typedef struct {
    int x_start;
    int y_start;
    int x_end;
    int y_end;
} SPRegion;

typedef struct DynamicCollider {
    SPRegion region;
    BoxCollider* collider;
} DynamicCollider;

typedef struct SPGrid {
    HashMap* grid_map;
    Array* dynamic_colliders;
} SPGrid;

typedef struct SPGridIter {
    int index;
    Array* array;
    struct SPGridIter* next;
} SPGridIter;

static inline SPRegion spgrid_region(const Rect r) {
    return (SPRegion){
        .x_start = (r.x) / SPARSE_GRID_SIZE,
        .y_start = (r.y) / SPARSE_GRID_SIZE,
        .x_end   = (r.x + r.w) / SPARSE_GRID_SIZE,
        .y_end   = (r.y + r.h) / SPARSE_GRID_SIZE,
    };
}

static inline bool spgrid_region_equal(SPRegion r1, SPRegion r2) {
    return r1.x_start == r2.x_start && r1.x_end == r2.x_end &&
           r1.y_start == r2.y_start && r1.y_end == r2.y_end;
}

void spgrid_insert(SPGrid* this, BoxCollider* box) {
    SPRegion r = spgrid_region(box_collider_rect(box));

    char grid_key[64];
    for (int y = r.y_start; y <= r.y_end; y++) {
        for (int x = r.x_start; x <= r.x_end; x++) {
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

    if (box->type == COLLIDER_TYPE_DYNAMIC) {
        DynamicCollider* dyn = malloc(sizeof(*dyn));
        if (dyn == NULL) {
            perror("failed to allocate dyn collider on spatial grid insert");
            return;
        }

        dyn->region   = r;
        dyn->collider = box;
        array_push(this->dynamic_colliders, dyn);
    }
}

void spgrid_remove(SPGrid* this, const BoxCollider* box) {
    SPRegion r = spgrid_region(box_collider_rect(box));

    char grid_key[64];
    for (int y = r.y_start; y <= r.y_end; y++) {
        for (int x = r.x_start; x <= r.x_end; x++) {
            Array* array;
            snprintf(grid_key, sizeof(grid_key), "%d,%d", x, y);

            if (hmap_get(this->grid_map, grid_key, (void**)&array)) {
                for (int i = 0; i < array_length(array); i++) {
                    void* ptr = array_get(array, i);
                    if (ptr == box) {
                        void* last = array_get(array, array_length(array) - 1);
                        array_set(array, i, last);
                        array_pop(array);
                        break;
                    }
                }
            }
        }
    }

    if (box->type == COLLIDER_TYPE_DYNAMIC) {
        for (int i = 0; i < array_length(this->dynamic_colliders); i++) {
            DynamicCollider* dyn = array_get(this->dynamic_colliders, i);
            if (dyn->collider == box) {
                size_t len = array_length(this->dynamic_colliders);
                void* last = array_get(this->dynamic_colliders, len - 1);
                array_set(this->dynamic_colliders, i, last);
                array_pop(this->dynamic_colliders);
                free(dyn);
            }
        }
    }
}

void spgrid_resolve(SPGrid* this, float delta) {
    size_t length = array_length(this->dynamic_colliders);
    for (int i = 0; i < length; i++) {
        DynamicCollider* col  = array_get(this->dynamic_colliders, i);
        BoxCollider* box      = col->collider;
        Rect rect             = box_collider_bounds(box);

        box->collision.top    = false;
        box->collision.bottom = false;
        box->collision.right  = false;
        box->collision.left   = false;
        box->gravity          = fminf(box->gravity + 9.8f * 0.2 * delta, 6);

        if (box->velocity.y < 0) {
            box->velocity.y += box->gravity;
            box->gravity = 0.1f;
        } else {
            box->velocity.y += box->gravity;
        }

        if (!spgrid_region_equal(col->region, spgrid_region(rect))) {
            spgrid_remove(this, box);
            spgrid_insert(this, box);
        }
    }

    for (int i = 0; i < length; i++) {
        DynamicCollider* col = array_get(this->dynamic_colliders, i);
        BoxCollider *b1 = col->collider, *b2 = NULL;
        SPGridIter* iter = spgrid_iter(this, box_collider_bounds(b1));

        while ((b2 = spgrid_iter_next(&iter))) {
            if (b1 != b2) {
                box_collider_resolve(b1, b2);
            }
        }

        if (b1->collision.bottom) {
            b1->gravity = 0.1f;
        }
    }
}

SPGridIter* spgrid_iter(const SPGrid* this, Rect b) {
    SPRegion r       = spgrid_region(b);
    SPGridIter* iter = NULL;

    char grid_key[64];
    for (int y = r.y_start; y <= r.y_end; y++) {
        for (int x = r.x_start; x <= r.x_end; x++) {
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

    sp->dynamic_colliders = array_new();
    if (sp->dynamic_colliders == NULL) {
        perror("failed to create dynamic collider array for spatial grid");
        hmap_free(sp->grid_map);
        free(sp);
        return NULL;
    }

    return sp;
}
