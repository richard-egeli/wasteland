#ifndef LIB_COLLISION_SPARSE_GRID_H_
#define LIB_COLLISION_SPARSE_GRID_H_

#include <stdint.h>

#include "collision/collision_defs.h"

#ifndef SPARSE_GRID_SIZE
#define SPARSE_GRID_SIZE 128
#endif

typedef struct SparseObject SparseObject;

typedef struct SparseGrid SparseGrid;

typedef struct BoxCollider BoxCollider;

typedef struct SparseGridIter SparseGridIter;

typedef uint64_t ColliderID;

Point spgrid_collider_position(SparseGrid* this, ColliderID id);

void spgrid_collider_set_position(SparseGrid* this, ColliderID id, int x, int y);

void spgrid_collider_move(SparseGrid* this, ColliderID id, float x, float y);

ColliderID spgrid_insert(SparseGrid* this, BoxCollider* box);

void spgrid_remove(SparseGrid* this, ColliderID id);

void spgrid_resolve(SparseGrid* this, float delta);

SparseGridIter* spgrid_iter(SparseGrid* this);

SparseObject* spgrid_iter_next(SparseGridIter* iter);

void spgrid_free(SparseGrid* this);

SparseGrid* spgrid_new(void);

#endif  // LIB_COLLISION_SPARSE_GRID_H_
