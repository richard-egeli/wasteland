#include "collision/sparse_object.h"

#include <stdio.h>
#include <stdlib.h>

#include "collision/box_collider.h"

AABB sparse_object_aabb_get(const SparseObject* this) {
    return this->aabb;
}

void sparse_object_aabb_update(SparseObject* this) {
    const BoxCollider* box = this->collider;
    this->aabb.xmin        = box->position.x + box->origin.x;
    this->aabb.ymin        = box->position.y + box->origin.y;
    this->aabb.xmax        = this->aabb.xmin + box->size.x;
    this->aabb.ymax        = this->aabb.ymin + box->size.y;
}

bool sparse_object_aabb_overlap(const SparseObject* s1, const SparseObject* s2) {
    return !(s1->aabb.xmin > s2->aabb.xmax || s1->aabb.xmax < s2->aabb.xmin ||
             s1->aabb.ymin > s2->aabb.ymax || s1->aabb.ymax < s2->aabb.ymin);
}

Region sparse_object_region_get(const SparseObject* this) {
    return this->region;
}

bool sparse_object_region_moved(const SparseObject* this, int region_size) {
    AABB current;
    AABB original          = this->aabb;
    const BoxCollider* box = this->collider;

    original.xmin /= region_size;
    original.ymin /= region_size;
    original.xmax /= region_size;
    original.ymax /= region_size;

    current.xmin = (box->position.x + box->origin.x) / region_size;
    current.ymin = (box->position.y + box->origin.y) / region_size;
    current.xmax = (current.xmin + box->size.x) / region_size;
    current.ymax = (current.ymin + box->size.y) / region_size;

    return !(original.xmin == current.xmin && original.ymin == current.ymin &&
             original.xmax == current.xmax && original.ymax == current.ymax);
}

void sparse_object_region_update(SparseObject* this, int region_size) {
    this->region.xmin = this->aabb.xmin / region_size;
    this->region.xmax = this->aabb.xmax / region_size;
    this->region.ymin = this->aabb.ymin / region_size;
    this->region.ymax = this->aabb.ymax / region_size;
}

void sparse_object_free(SparseObject* this) {
    free(this);
}

SparseObject* sparse_object_new(BoxCollider* collider, int region_size) {
    SparseObject* object = malloc(sizeof(*object));
    if (object != NULL) {
        object->collider = collider;
        sparse_object_aabb_update(object);
        sparse_object_region_update(object, region_size);
    }

    return object;
}
