#include "collision/sparse_grid.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array/array.h"
#include "collision/box_collider.h"
#include "collision/collision_defs.h"
#include "collision/sparse_object.h"
#include "hashmap/hashmap.h"

typedef enum SparseEventType {
    SPARSE_EVENT_TYPE_INSERT,
    SPARSE_EVENT_TYPE_REMOVE,
} SparseEventType;

typedef struct SparseEvent {
    SparseEventType type;
    SparseObject* object;
    struct SparseEvent* next;
} SparseEvent;

typedef struct SparseGridIter {
    int index;
    Array* array;
    HashMapIter* hmap_iter;
} SparseGridIter;

typedef struct SparseGrid {
    HashMap* grid_map;
    Array* weak_dyn_ref;
    SparseEvent* events;
    SparseObject* id_lookup[INT16_MAX];
    size_t id_index;
} SparseGrid;

static Array* sparse_grid_region_get(SparseGrid* this, int x, int y) {
    char grid_key[48];
    Array* array = NULL;
    uint32_t len = sprintf(grid_key, "%d,%d", x, y);
    if (!hmap_get(this->grid_map, grid_key, len, (void**)&array)) {
        array = array_new();
        if (!hmap_put(this->grid_map, grid_key, len, array)) {
            perror("failed to insert new array into sparse grid");
            array_free(array);
            array = NULL;
        }
    }

    return array;
}

static void spgrid_handle_event_remove(SparseGrid* this, SparseEvent* event) {
    SparseObject* obj = event->object;
    Region region     = sparse_object_region_get(obj, SPARSE_GRID_SIZE);

    for (int y = region.ymin; y <= region.ymax; y++) {
        for (int x = region.xmin; x <= region.xmax; x++) {
            Array* array = sparse_grid_region_get(this, x, y);

            for (int i = 0; i < array_length(array); i++) {
                if (obj == array_get(array, i)) {
                    void* last = array_get(array, array_length(array) - 1);
                    array_set(array, i, last);
                    array_pop(array);
                    break;
                }
            }
        }
    }

    if (obj->collider->type == COLLIDER_TYPE_DYNAMIC) {
        for (int i = 0; i < array_length(this->weak_dyn_ref); i++) {
            if (obj == array_get(this->weak_dyn_ref, i)) {
                size_t len = array_length(this->weak_dyn_ref);
                void* last = array_get(this->weak_dyn_ref, len - 1);
                array_set(this->weak_dyn_ref, i, last);
                array_pop(this->weak_dyn_ref);
            }
        }
    }

    box_collider_free(obj->collider);
    free(obj);
}

static void spgrid_handle_event_insert(SparseGrid* this, SparseEvent* event) {
    SparseObject* obj = event->object;
    Region region     = sparse_object_region_get(obj, SPARSE_GRID_SIZE);

    for (int y = region.ymin; y <= region.ymax; y++) {
        for (int x = region.xmin; x <= region.xmax; x++) {
            Array* array = sparse_grid_region_get(this, x, y);
            array_push(array, obj);
        }
    }

    if (obj->collider->type == COLLIDER_TYPE_DYNAMIC) {
        array_push(this->weak_dyn_ref, obj);
    }
}

static void spgrid_handle_events(SparseGrid* this) {
    SparseEvent* event = this->events;
    while (event != NULL) {
        SparseEvent* temp = event;
        switch (event->type) {
            case SPARSE_EVENT_TYPE_INSERT:
                spgrid_handle_event_insert(this, event);
                break;
            case SPARSE_EVENT_TYPE_REMOVE:
                spgrid_handle_event_remove(this, event);
                break;
        }

        event = event->next;
        free(temp);
    }

    this->events = NULL;
}

ColliderID spgrid_insert(SparseGrid* this, BoxCollider* box) {
    SparseEvent* event = malloc(sizeof(*event));
    if (event != NULL) {
        SparseObject* object = sparse_object_new(box, SPARSE_GRID_SIZE);

        if (object != NULL) {
            const size_t max = sizeof(this->id_lookup) / sizeof(void*);

            for (int i = 0; i < max; i++) {
                int idx = (this->id_index + i) % max;
                if (idx == 0) continue;

                if (this->id_lookup[idx] == NULL) {
                    event->next          = this->events;
                    event->type          = SPARSE_EVENT_TYPE_INSERT;
                    event->object        = object;
                    object->collider     = box;
                    this->events         = event;
                    this->id_lookup[idx] = object;
                    return idx;
                }
            }

            free(object);
        }

        free(event);
    }

    return 0;
}

void spgrid_remove(SparseGrid* this, ColliderID id) {
    SparseEvent* event = malloc(sizeof(*event));
    if (event != NULL) {
        SparseObject* object = this->id_lookup[id];
        if (object != NULL) {
            event->type         = SPARSE_EVENT_TYPE_REMOVE;
            event->object       = object;
            event->next         = this->events;
            this->events        = event;
            this->id_lookup[id] = NULL;
            return;
        }

        free(event);
    }
}

Point spgrid_collider_position(SparseGrid* this, ColliderID id) {
    const SparseObject* object = this->id_lookup[id];
    return (Point){
        .x = object->collider->position.x,
        .y = object->collider->position.y,
    };
}

void spgrid_collider_move(SparseGrid* this, ColliderID id, float x, float y) {
    SparseObject* object = this->id_lookup[id];
    if (object != NULL && object->collider != NULL) {
        object->collider->velocity.x += x;
        object->collider->velocity.y += y;
    }
}

void spgrid_resolve(SparseGrid* this, float delta) {
    size_t length = array_length(this->weak_dyn_ref);
    for (int i = 0; i < length; i++) {
        SparseObject* obj     = array_get(this->weak_dyn_ref, i);
        BoxCollider* box      = obj->collider;

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

    // resolve all collisions on the grid
    for (int i = 0; i < length; i++) {
        SparseObject* obj = array_get(this->weak_dyn_ref, i);
        BoxCollider* b1   = obj->collider;

        Region region     = sparse_object_region_get(obj, SPARSE_GRID_SIZE);
        for (int y = region.ymin; y <= region.ymax; y++) {
            for (int x = region.xmin; x <= region.xmax; x++) {
                Array* array = sparse_grid_region_get(this, x, y);

                for (int i = 0; i < array_length(array); i++) {
                    SparseObject* o2 = array_get(array, i);
                    if (obj->collider != o2->collider) {
                        box_collider_resolve(obj->collider, o2->collider);
                    }
                }
            }
        }

        box_collider_update(b1);
        sparse_object_aabb_update(obj);

        if (sparse_object_region_moved(obj, SPARSE_GRID_SIZE)) {
            Region region = sparse_object_region_get(obj, SPARSE_GRID_SIZE);
            for (int y = region.ymin; y <= region.ymax; y++) {
                for (int x = region.xmin; x <= region.xmax; x++) {
                    Array* array = sparse_grid_region_get(this, x, y);

                    for (int i = 0; i < array_length(array); i++) {
                        if (obj == array_get(array, i)) {
                            size_t idx         = array_length(array) - 1;
                            SparseObject* last = array_get(array, idx);
                            array_set(array, i, last);
                            array_pop(array);
                            break;
                        }
                    }
                }
            }

            sparse_object_region_update(obj, SPARSE_GRID_SIZE);
            region = sparse_object_region_get(obj, SPARSE_GRID_SIZE);
            for (int y = region.ymin; y <= region.ymax; y++) {
                for (int x = region.xmin; x <= region.xmax; x++) {
                    Array* array = sparse_grid_region_get(this, x, y);
                    array_push(array, obj);
                }
            }
        }
    }

    spgrid_handle_events(this);
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

    const size_t max  = sizeof(this->id_lookup) / sizeof(void*);
    for (int i = 0; i < max; i++) {
        SparseObject* object = this->id_lookup[i];

        if (object != NULL) {
            if (object->collider != NULL) {
                box_collider_free(object->collider);
                object->collider = NULL;
            }

            free(object);
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

    memset(sp->id_lookup, 0, sizeof(sp->id_lookup));
    sp->id_index = 0;
    sp->events   = NULL;
    return sp;
}
