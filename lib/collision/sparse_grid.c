#include "collision/sparse_grid.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array/array.h"
#include "collision/box_collider.h"
#include "collision/collision_defs.h"
#include "hashmap/hashmap.h"

typedef struct {
    int x_start;
    int y_start;
    int x_end;
    int y_end;
} SparseRegion;

typedef struct DynamicCollider {
    SparseRegion region;
    BoxCollider* collider;
} DynamicCollider;

typedef struct SparseGridIter {
    int index;
    Array* array;
    HashMapIter* hmap_iter;
} SparseGridIter;

typedef struct SparseGrid {
    HashMap* grid_map;
    Array* weak_dyn_ref;
} SparseGrid;

typedef struct SparseObject {
    void* value;
} SparseObject;

typedef struct SparseGridRegionIter {
    int index;
    Array* array;
    struct SparseGridRegionIter* next;
} SparseGridRegionIter;

static inline SparseRegion spgrid_region(const Rect r) {
    return (SparseRegion){
        .x_start = (r.x) / SPARSE_GRID_SIZE,
        .y_start = (r.y) / SPARSE_GRID_SIZE,
        .x_end   = (r.x + r.w) / SPARSE_GRID_SIZE,
        .y_end   = (r.y + r.h) / SPARSE_GRID_SIZE,
    };
}

static inline bool spgrid_region_equal(SparseRegion r1, SparseRegion r2) {
    return r1.x_start == r2.x_start && r1.x_end == r2.x_end &&
           r1.y_start == r2.y_start && r1.y_end == r2.y_end;
}

static SparseGridRegionIter* spgrid_region_iter(SparseGrid* this, Rect b) {
    SparseRegion r             = spgrid_region(b);
    SparseGridRegionIter* iter = NULL;

    char grid_key[64];
    for (int y = r.y_start; y <= r.y_end; y++) {
        for (int x = r.x_start; x <= r.x_end; x++) {
            Array* array;
            snprintf(grid_key, sizeof(grid_key), "%d,%d", x, y);

            if (hmap_get(this->grid_map, grid_key, (void**)&array)) {
                SparseGridRegionIter* new_iter = malloc(sizeof(*new_iter));
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

static void* spgrid_region_iter_next(SparseGridRegionIter** this) {
    while (*this != NULL) {
        SparseGridRegionIter* it = *this;
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

static Array* sparse_grid_region_get(SparseGrid* this, int x, int y) {
    char grid_key[48];
    Array* array = NULL;
    snprintf(grid_key, sizeof(grid_key), "%d,%d", x, y);
    if (!hmap_get(this->grid_map, grid_key, (void**)&array)) {
        array = array_new();
        if (!hmap_put(this->grid_map, grid_key, array)) {
            perror("failed to insert new array into sparse grid");
            array_free(array);
            array = NULL;
        }
    }

    return array;
}

void spgrid_insert(SparseGrid* this, BoxCollider* box) {
    SparseRegion r = spgrid_region(box_collider_rect(box));

    char grid_key[64];
    for (int y = r.y_start; y <= r.y_end; y++) {
        for (int x = r.x_start; x <= r.x_end; x++) {
            Array* array      = sparse_grid_region_get(this, x, y);
            SparseObject* obj = malloc(sizeof(SparseObject));
            obj->value        = box;
            array_push(array, obj);
        }
    }

    // TODO: naive approach currently, improve performance on insert
    if (box->type == COLLIDER_TYPE_DYNAMIC) {
        bool has = false;
        for (int i = 0; i < array_length(this->weak_dyn_ref); i++) {
            DynamicCollider* dyn = array_get(this->weak_dyn_ref, i);
            if (dyn->collider == box) {
                has = true;
                break;
            }
        }

        if (!has) {
            DynamicCollider* dyn = malloc(sizeof(*dyn));
            if (dyn == NULL) {
                perror("failed to allocate dyn collider on sparse grid insert");
                return;
            }

            dyn->region   = r;
            dyn->collider = box;
            array_push(this->weak_dyn_ref, dyn);
        }
    }
}

void spgrid_remove(SparseGrid* this, const BoxCollider* box) {
    SparseRegion r = spgrid_region(box_collider_rect(box));

    for (int y = r.y_start; y <= r.y_end; y++) {
        for (int x = r.x_start; x <= r.x_end; x++) {
            Array* array = sparse_grid_region_get(this, x, y);

            for (int i = 0; i < array_length(array); i++) {
                SparseObject* obj = array_get(array, i);
                if (obj->value == box) {
                    void* last = array_get(array, array_length(array) - 1);
                    array_set(array, i, last);
                    array_pop(array);
                    free(obj);
                    break;
                }
            }
        }
    }

    if (box->type == COLLIDER_TYPE_DYNAMIC) {
        for (int i = 0; i < array_length(this->weak_dyn_ref); i++) {
            DynamicCollider* dyn = array_get(this->weak_dyn_ref, i);
            if (dyn->collider == box) {
                free(dyn);

                size_t len = array_length(this->weak_dyn_ref);
                void* last = array_get(this->weak_dyn_ref, len - 1);
                array_set(this->weak_dyn_ref, i, last);
                array_pop(this->weak_dyn_ref);
            }
        }
    }
}

void spgrid_resolve(SparseGrid* this, float delta) {
    size_t length = array_length(this->weak_dyn_ref);
    for (int i = 0; i < length; i++) {
        DynamicCollider* col  = array_get(this->weak_dyn_ref, i);
        BoxCollider* box      = col->collider;

        box->collision.top    = false;
        box->collision.bottom = false;
        box->collision.right  = false;
        box->collision.left   = false;

        if (box->gravity.enabled) {
            box->gravity.accum += box->gravity.force * delta;
            box->gravity.accum = fminf(box->gravity.accum, 0.98f);

            if (box->velocity.y < 0) {
                box->velocity.y += box->gravity.accum;
                box->gravity.accum = 0.1f;
            } else {
                box->velocity.y += box->gravity.accum;
            }
        }
    }

    for (int i = 0; i < length; i++) {
        DynamicCollider* col       = array_get(this->weak_dyn_ref, i);
        BoxCollider* b1            = col->collider;
        Rect bounds                = box_collider_bounds(b1);
        SparseGridRegionIter* iter = spgrid_region_iter(this, bounds);

        SparseObject* obj          = NULL;
        while ((obj = spgrid_region_iter_next(&iter))) {
            BoxCollider* b2 = obj->value;
            if (b1 != b2) {
                box_collider_resolve(b1, obj->value);
            }
        }

        if (b1->collision.bottom) {
            b1->gravity.accum = 0.1f;
        }

        box_collider_update(b1);

        Rect rect = box_collider_bounds(b1);
        if (!spgrid_region_equal(col->region, spgrid_region(rect))) {
            for (int y = col->region.y_start; y <= col->region.y_end; y++) {
                for (int x = col->region.x_start; x <= col->region.x_end; x++) {
                    Array* array = sparse_grid_region_get(this, x, y);

                    for (int i = 0; i < array_length(array); i++) {
                        SparseObject* obj = array_get(array, i);
                        if (obj->value == b1) {
                            size_t idx         = array_length(array) - 1;
                            SparseObject* last = array_get(array, idx);
                            array_set(array, i, last);
                            array_pop(array);
                            free(obj);
                            break;
                        }
                    }
                }
            }

            col->region = spgrid_region(rect);
            spgrid_insert(this, b1);
        }
    }
}

SparseGridIter* spgrid_iter(SparseGrid* this) {
    SparseGridIter* it = malloc(sizeof(*it));
    if (it != NULL) {
        it->hmap_iter = hmap_iter(this->grid_map);
        it->array     = hmap_iter_first(it->hmap_iter, NULL);
        it->index     = 0;
        return it;
    }

    perror("failed to allocate sparse grid iter");
    return NULL;
}

SparseObject* spgrid_iter_next(SparseGridIter* this) {
    if (this != NULL) {
        while (this->array) {
            while (this->index < array_length(this->array)) {
                void* result = array_get(this->array, this->index);
                this->index += 1;
                return result;
            }

            this->array = hmap_iter_next(this->hmap_iter, NULL);
            this->index = 0;
        }

        free(this);
    }

    return NULL;
}

void spgrid_free(SparseGrid* this) {
    HashMapIter* h_it = hmap_iter(this->grid_map);
    Array* bucket;
    size_t size = 0;
    while ((bucket = hmap_iter_next(h_it, NULL))) {
        size += array_length(bucket);
    }

    if (size > 0) {
        uint64_t* addr = malloc(size * sizeof(*addr));
        if (addr != NULL) {
            size_t addr_len = 0;
            SparseObject* obj;
            SparseGridIter* iter = spgrid_iter(this);
            while ((obj = spgrid_iter_next(iter))) {
                bool has = false;
                for (int i = 0; i < addr_len; i++) {
                    if (addr[i] == (uint64_t)obj->value) {
                        has = true;
                        break;
                    }
                }

                if (!has) {
                    addr[addr_len] = (uint64_t)obj->value;
                    addr_len++;
                }
            }

            for (int i = 0; i < addr_len; i++) {
                spgrid_remove(this, (BoxCollider*)addr[i]);
                box_collider_free((BoxCollider*)addr[i]);
            }

            free(addr);
        }
    }

    {  // free every array bucket in the hashmap
        Array* bucket;
        HashMapIter* iter = hmap_iter(this->grid_map);
        while ((bucket = hmap_iter_next(iter, NULL))) {
            array_free(bucket);
        }
    }

    array_free(this->weak_dyn_ref);
    hmap_free(this->grid_map);
    free(this);
}

SparseGrid* spgrid_new(void) {
    SparseGrid* sp = malloc(sizeof(*sp));
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

    sp->weak_dyn_ref = array_new();
    if (sp->weak_dyn_ref == NULL) {
        perror("failed to create dynamic collider array for spatial grid");
        hmap_free(sp->grid_map);
        free(sp);
        return NULL;
    }

    return sp;
}
