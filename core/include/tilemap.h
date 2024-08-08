#ifndef CORE_INCLUDE_TILEMAP_H_
#define CORE_INCLUDE_TILEMAP_H_

#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct Array Array;

typedef struct HashMap HashMap;

typedef struct LDTK_Level LDTK_Level;

typedef struct Tileset {
    int uid;
    Texture texture;
} Tileset;

typedef struct Tile {
    int id;
    int pos_x;
    int pos_y;
    int src_x;
    int src_y;
    bool flip_x;
    bool flip_y;
    float opacity;
} Tile;

typedef struct Layer {
    Tile* tiles;
    size_t tiles_length;
    size_t grid_size;
    Texture texture;
    float opacity;
} Layer;

typedef struct Tilemap {
    size_t grid_width;
    size_t grid_height;
    HashMap* tilesets;
    Array* layers;
    const Layer* active_layer;
} Tilemap;

typedef struct TileIter {
    size_t tile_index;
    size_t layer_idx;
    Tilemap* tilemap;
} TileIter;

void tilemap_begin(Tilemap* this, const Layer* layer);

void tilemap_end(Tilemap* this);

void tilemap_tile_draw(const Tilemap* this, const Tile* tile);

Tile* tilemap_tile_next(TileIter* it);

TileIter tilemap_tile_iter(Tilemap* this);

Tilemap* tilemap_from_ldtk(const LDTK_Level* level);

#endif  // CORE_INCLUDE_TILEMAP_H_
