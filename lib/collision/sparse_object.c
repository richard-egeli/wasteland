#include "collision/sparse_object.h"

#include <stdlib.h>

#include "collision/box_collider.h"

typedef struct SparseObject {
    AABB aabb;
    BoxCollider* collider;
} SparseObject;

AABB sparse_object_aabb_get(const SparseObject* object) {
    return object->aabb;
}

void sparse_object_aabb_update(SparseObject* object) {
    if (object != NULL && object->collider != NULL) {
        const BoxCollider* box = object->collider;
        object->aabb.xmin      = box->position.x + box->origin.x;
        object->aabb.ymin      = box->position.y + box->origin.y;
        object->aabb.xmax      = object->aabb.xmin + box->size.x;
        object->aabb.ymax      = object->aabb.ymin + box->size.y;
    }
}

bool sparse_object_aabb_overlap(const SparseObject* s1, const SparseObject* s2) {
    return !(s1->aabb.xmin > s2->aabb.xmax || s1->aabb.xmax < s2->aabb.xmin ||
             s1->aabb.ymin > s2->aabb.ymax || s1->aabb.ymax < s2->aabb.ymin);
}

void sparse_object_free(SparseObject* object) {
    free(object);
}

SparseObject* sparse_object_new(BoxCollider* collider) {
    SparseObject* object = malloc(sizeof(*object));
    if (object != NULL) {
        object->collider = collider;
        sparse_object_aabb_update(object);
    }

    return object;
}
