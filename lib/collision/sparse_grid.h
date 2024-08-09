#ifndef LIB_COLLISION_SPARSE_GRID_H_
#define LIB_COLLISION_SPARSE_GRID_H_

#ifndef SPARSE_GRID_SIZE
#define SPARSE_GRID_SIZE 128
#endif

typedef struct SparseObject SparseObject;

typedef struct SparseGrid SparseGrid;

typedef struct BoxCollider BoxCollider;

typedef struct SparseGridIter SparseGridIter;

void spgrid_insert(SparseGrid* this, BoxCollider* box);

void spgrid_remove(SparseGrid* this, const BoxCollider* box);

void spgrid_resolve(SparseGrid* this, float delta);

SparseGridIter* spgrid_iter(SparseGrid* this);

SparseObject* spgrid_iter_next(SparseGridIter* iter);

void spgrid_free(SparseGrid* this);

SparseGrid* spgrid_new(void);

#endif  // LIB_COLLISION_SPARSE_GRID_H_
