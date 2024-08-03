#ifndef LIB_COLLISION_SPATIAL_GRID_H_
#define LIB_COLLISION_SPATIAL_GRID_H_

#include "collision/collision_defs.h"

#ifndef SPATIAL_GRID_SIZE
#define SPATIAL_GRID_SIZE 128
#endif

typedef struct SpatialGrid SPGrid;

typedef struct BoxCollider BoxCollider;

typedef struct SPGridIter SPGridIter;

void spgrid_insert(SPGrid* this, BoxCollider* box);

SPGridIter* spgrid_iter(const SPGrid* this, Rect rect);

void* spgrid_iter_next(SPGridIter** this);

void spgrid_free(SPGrid* this);

SPGrid* spgrid_new(void);

#endif  // LIB_COLLISION_SPATIAL_GRID_H_
