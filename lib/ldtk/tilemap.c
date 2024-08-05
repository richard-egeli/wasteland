#include "ldtk/tilemap.h"

#include <stdio.h>
#include <stdlib.h>

#include "array/array.h"
#include "ldtk/ldtk.h"

void tilemap_begin(Tilemap* this, const Layer* layer) {
    this->active_layer = layer;
}

void tilemap_end(Tilemap* this) {
    this->active_layer = NULL;
}

void tilemap_tile_draw(const Tilemap* this, const Tile* tile) {
    const Layer* layer = this->active_layer;
    int b              = layer->grid_size;
    int sw             = tile->flip_x ? -b : b;
    int sh             = tile->flip_y ? -b : b;
    Color col          = {
                 .r = 255,
                 .g = 255,
                 .b = 255,
                 .a = 255 * (tile->opacity * layer->opacity),
    };

    Rectangle src = {tile->src_x, tile->src_y, sw, sh};
    Rectangle dst = {tile->pos_x, tile->pos_y, b, b};
    DrawTexturePro(layer->tileset, src, dst, (Vector2){0}, 0, col);
}

Tile* tilemap_tile_next(TileIter* it) {
    while (it->layer_idx < array_length(it->tilemap->layers)) {
        const Layer* layer = array_get(it->tilemap->layers, it->layer_idx);
        tilemap_begin(it->tilemap, layer);

        while (it->tile_index < layer->tiles_length) {
            return &layer->tiles[it->tile_index++];
        }

        it->tile_index = 0;
        it->layer_idx++;
    }

    tilemap_end(it->tilemap);
    return NULL;
}

TileIter tilemap_tile_iter(Tilemap* this) {
    return (TileIter){
        .tile_index = 0,
        .layer_idx  = 0,
        .tilemap    = this,
    };
}

Tilemap* tilemap_from_ldtk(const LDTK_Level* level) {
    Tilemap* tilemap     = malloc(sizeof(*tilemap));

    tilemap->grid_width  = level->px_width;
    tilemap->grid_height = level->px_height;
    tilemap->layers      = array_new();
    tilemap->tilesets    = array_new();

    for (int j = 0; j < array_length(level->layer_instances); j++) {
        LDTK_Layer* layer = array_get(level->layer_instances, j);
        Layer* new_layer  = malloc(sizeof(*layer));

        char buffer[256];
        const char* prefix = LDTK_ASSET_PREFIX;
        const char* path   = layer->__tileset_rel_path;
        snprintf(buffer, 256, "%s/%s", prefix, path);

        new_layer->tiles        = NULL;
        new_layer->tiles_length = 0;
        new_layer->grid_size    = layer->__grid_size;
        new_layer->tileset      = LoadTexture(buffer);
        new_layer->opacity      = layer->__opacity;

        size_t auto_size        = array_length(layer->auto_layer_tiles);
        size_t grid_size        = array_length(layer->grid_tiles);

        if (auto_size > 0) {
            new_layer->tiles_length = auto_size;
            new_layer->tiles        = malloc(sizeof(Tile) * auto_size);

            for (int i = 0; i < auto_size; i++) {
                LDTK_Tile* tile     = array_get(layer->auto_layer_tiles, i);
                new_layer->tiles[i] = (Tile){
                    .id      = tile->t,
                    .pos_x   = tile->px[0],
                    .pos_y   = tile->px[1],
                    .src_x   = tile->src[0],
                    .src_y   = tile->src[1],
                    .flip_x  = tile->f & (1 << 0),
                    .flip_y  = tile->f & (1 << 1),
                    .opacity = tile->a,
                };
            }
        }

        if (grid_size > 0) {
            new_layer->tiles_length = grid_size;
            new_layer->tiles        = malloc(sizeof(Tile) * grid_size);

            for (int i = 0; i < grid_size; i++) {
                LDTK_Tile* tile     = array_get(layer->grid_tiles, i);
                new_layer->tiles[i] = (Tile){
                    .id      = tile->t,
                    .pos_x   = tile->px[0],
                    .pos_y   = tile->px[1],
                    .src_x   = tile->src[0],
                    .src_y   = tile->src[1],
                    .flip_x  = tile->f & (1 << 0),
                    .flip_y  = tile->f & (1 << 1),
                    .opacity = tile->a,
                };
            }
        }

        array_push(tilemap->layers, new_layer);
    }

    array_reverse(tilemap->layers);
    return tilemap;
}
