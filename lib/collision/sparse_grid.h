#ifndef LIB_COLLISION_SPARSE_GRID_H_
#define LIB_COLLISION_SPARSE_GRID_H_

#include "collision/collision_defs.h"

#ifndef SPARSE_GRID_SIZE
#define SPARSE_GRID_SIZE 128
#endif

typedef struct SPGrid SPGrid;

typedef struct BoxCollider BoxCollider;

typedef struct SPGridIter SPGridIter;

void spgrid_insert(SPGrid* this, BoxCollider* box);

void spgrid_remove(SPGrid* this, const BoxCollider* box);

void spgrid_resolve(SPGrid* this, float delta);

SPGridIter* spgrid_iter(const SPGrid* this, Rect rect);

void* spgrid_iter_next(SPGridIter** this);

void spgrid_free(SPGrid* this);

SPGrid* spgrid_new(void);

#endif  // LIB_COLLISION_SPARSE_GRID_H_
