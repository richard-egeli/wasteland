#ifndef CORE_INCLUDE_LEVEL_H_
#define CORE_INCLUDE_LEVEL_H_

#include <raylib.h>
#include <stddef.h>
#include <stdint.h>

typedef struct HashMap HashMap;

typedef struct Array Array;

typedef struct Tilemap Tilemap;

typedef struct SparseGrid SparseGrid;

typedef struct BoxCollider BoxCollider;

typedef struct Level {
    Tilemap* tilemap;
    SparseGrid* sparse_grid;
    Array* entities;
    HashMap* textures;
} Level;

Level* level_new(void);

void level_load(Level* level, const char* path, const char* level_id);

void level_update(Level* level);

#endif  // CORE_INCLUDE_LEVEL_H_
