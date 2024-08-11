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

typedef struct LDTK_Field LDTK_Field;

typedef struct LDTK_Level LDTK_Level;

typedef struct LDTK_Root LDTK_Root;

typedef struct Level {
    Tilemap* tilemap;
    SparseGrid* sparse_grid;
    Array* entities;

    struct {
        LDTK_Root* root;
        LDTK_Level* level;
    } ldtk;
} Level;

typedef void (*entity_callback)(int x,
                                int y,
                                const char* name,
                                LDTK_Field* fields,
                                size_t len);

void level_free(Level* this);

Level* level_load(const char* path, const char* level_id);

void level_entities_load(Level* level, const char* key, entity_callback cb);

void level_collider_load(Level* level, const char* key);

void level_update(Level* level);

#endif  // CORE_INCLUDE_LEVEL_H_
