#ifndef LIB_COLLISION_SPARSE_OBJECT_H_
#define LIB_COLLISION_SPARSE_OBJECT_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "collision/collision_defs.h"

typedef struct SparseObject SparseObject;

typedef struct BoxCollider BoxCollider;

typedef struct SparseObject {
    AABB aabb;
    Region region;
    BoxCollider* collider;
} SparseObject;

AABB sparse_object_aabb_get(const SparseObject* this);

void sparse_object_aabb_update(SparseObject* this);

bool sparse_object_aabb_overlap(const SparseObject* s1, const SparseObject* s2);

Region sparse_object_region_get(const SparseObject* this, size_t region_size);

bool sparse_object_region_moved(const SparseObject* this, size_t region_size);

void sparse_object_region_update(SparseObject* this, size_t region_size);

void sparse_object_free(SparseObject* this);

SparseObject* sparse_object_new(BoxCollider* collider, size_t region_size);

#endif  // LIB_COLLISION_SPARSE_OBJECT_H_
