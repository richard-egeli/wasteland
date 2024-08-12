#ifndef LIB_COLLISION_SPARSE_OBJECT_H_
#define LIB_COLLISION_SPARSE_OBJECT_H_

#include <stdbool.h>
typedef struct SparseObject SparseObject;

typedef struct BoxCollider BoxCollider;

typedef struct AABB {
    float xmin;
    float ymin;
    float xmax;
    float ymax;
} AABB;

AABB sparse_object_aabb_get(const SparseObject* object);

void sparse_object_aabb_update(SparseObject* object);

bool sparse_object_aabb_overlap(const SparseObject* s1, const SparseObject* s2);

void sparse_object_free(SparseObject* object);

SparseObject* sparse_object_new(BoxCollider* collider);

#endif  // LIB_COLLISION_SPARSE_OBJECT_H_
