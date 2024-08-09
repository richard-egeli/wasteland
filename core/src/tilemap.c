#include "tilemap.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array/array.h"
#include "ldtk/ldtk.h"
#include "texture.h"

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
    DrawTexturePro(layer->texture, src, dst, (Vector2){0}, 0, col);
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

void tilemap_load_textures(Tilemap* tilemap, const LDTK_Level* level) {
    for (int j = 0; j < level->layer_instances_length; j++) {
        const LDTK_Layer* layer = &level->layer_instances[j];

        if (layer->__tileset_rel_path != NULL) {
            int uid = layer->__tileset_def_uid;
            char path[256];
            const char* prefix   = LDTK_ASSET_PREFIX;
            const char* rel_path = layer->__tileset_rel_path;
            snprintf(path, sizeof(path), "%s/%s", prefix, rel_path);
            texture_load(path);
            texture_link_uid(path, uid);
        }
    }
}

void tilemap_free(Tilemap* this) {
    for (int i = 0; i < array_length(this->layers); i++) {
        Layer* layer = array_get(this->layers, i);
        free(layer->tiles);
        free(layer);
    }

    array_free(this->layers);
    free(this);
}

Tilemap* tilemap_from_ldtk(const LDTK_Level* level) {
    Tilemap* tilemap     = malloc(sizeof(*tilemap));

    tilemap->grid_width  = level->px_width;
    tilemap->grid_height = level->px_height;
    tilemap->layers      = array_new();

    tilemap_load_textures(tilemap, level);

    for (int j = 0; j < level->layer_instances_length; j++) {
        LDTK_Layer* layer = &level->layer_instances[j];
        Layer* new_layer  = malloc(sizeof(*layer));
        if (new_layer == NULL) {
            perror("failed to malloc new layer in tilemap");
            continue;
        }

        new_layer->tiles        = NULL;
        new_layer->tiles_length = 0;
        new_layer->grid_size    = layer->__grid_size;
        new_layer->opacity      = layer->__opacity;

        if (layer->__tileset_rel_path != NULL) {
            char path[256];
            const char* prefix   = LDTK_ASSET_PREFIX;
            const char* rel_path = layer->__tileset_rel_path;

            snprintf(path, sizeof(path), "%s/%s", prefix, rel_path);
            new_layer->texture = texture_load(path);
        }

        if (layer->auto_layer_tiles_length > 0) {
            new_layer->tiles_length = layer->auto_layer_tiles_length;
            new_layer->tiles = malloc(sizeof(Tile) * new_layer->tiles_length);
            if (new_layer->tiles != NULL) {
                for (int i = 0; i < new_layer->tiles_length; i++) {
                    LDTK_Tile* tile     = &layer->auto_layer_tiles[i];
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
        }

        else if (layer->grid_tiles_length > 0) {
            new_layer->tiles_length = layer->grid_tiles_length;
            new_layer->tiles = malloc(sizeof(Tile) * new_layer->tiles_length);

            if (new_layer->tiles != NULL) {
                for (int i = 0; i < new_layer->tiles_length; i++) {
                    LDTK_Tile* tile     = &layer->grid_tiles[i];
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
        }

        array_push(tilemap->layers, new_layer);
    }

    array_reverse(tilemap->layers);
    return tilemap;
}
