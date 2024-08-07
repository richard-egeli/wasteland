#include "ldtk/tilemap.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array/array.h"
#include "hashmap/hashmap.h"
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
    DrawTexturePro(*layer->texture, src, dst, (Vector2){0}, 0, col);
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
            char uid[32];
            snprintf(uid, sizeof(uid), "%d", layer->__tileset_def_uid);

            if (!hmap_has(tilemap->tilesets, uid)) {
                Texture* texture = malloc(sizeof(Texture));
                if (texture == NULL) {
                    fprintf(stderr, "failed to malloc texture\n");
                    exit(EXIT_FAILURE);
                }

                char prefixed_filepath[256];
                const char* prefix = LDTK_ASSET_PREFIX;
                const char* path   = layer->__tileset_rel_path;
                snprintf(prefixed_filepath, 256, "%s/%s", prefix, path);
                *texture = LoadTexture(prefixed_filepath);
                hmap_put(tilemap->tilesets, uid, texture);
            }
        }
    }
}

Tilemap* tilemap_from_ldtk(const LDTK_Level* level) {
    Tilemap* tilemap     = malloc(sizeof(*tilemap));

    tilemap->grid_width  = level->px_width;
    tilemap->grid_height = level->px_height;
    tilemap->layers      = array_new();
    tilemap->entities    = array_new();
    tilemap->tilesets    = hmap_new();

    tilemap_load_textures(tilemap, level);

    for (int j = 0; j < level->layer_instances_length; j++) {
        LDTK_Layer* layer       = &level->layer_instances[j];
        Layer* new_layer        = malloc(sizeof(*layer));

        new_layer->tiles        = NULL;
        new_layer->tiles_length = 0;
        new_layer->grid_size    = layer->__grid_size;
        new_layer->opacity      = layer->__opacity;

        if (layer->__tileset_rel_path != NULL) {
            char uid[32];
            snprintf(uid, sizeof(uid), "%d", layer->__tileset_def_uid);
            hmap_get(tilemap->tilesets, uid, (void**)&new_layer->texture);
        }

        if (layer->auto_layer_tiles_length > 0) {
            new_layer->tiles_length = layer->auto_layer_tiles_length;
            new_layer->tiles = malloc(sizeof(Tile) * new_layer->tiles_length);

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

        if (layer->grid_tiles_length > 0) {
            new_layer->tiles_length = layer->grid_tiles_length;
            new_layer->tiles = malloc(sizeof(Tile) * new_layer->tiles_length);

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

        for (int i = 0; i < layer->entity_instances_length; i++) {
            Entity* entity        = malloc(sizeof(*entity));
            LDTK_Entity* ldtk_ent = &layer->entity_instances[i];

            entity->position.x    = ldtk_ent->__world_x;
            entity->position.y    = ldtk_ent->__world_y;
            entity->source.x      = ldtk_ent->__tile.x;
            entity->source.y      = ldtk_ent->__tile.y;
            entity->source.width  = ldtk_ent->__tile.w;
            entity->source.height = ldtk_ent->__tile.h;

            char uid[32];
            snprintf(uid, sizeof(uid), "%d", ldtk_ent->__tile.tileset_uid);
            if (!hmap_get(tilemap->tilesets, uid, (void**)&entity->texture)) {
                fprintf(stderr, "failed to get uid %s\n", uid);
                entity->texture = NULL;
            }

            array_push(tilemap->entities, entity);
        }

        array_push(tilemap->layers, new_layer);
    }

    array_reverse(tilemap->layers);
    return tilemap;
}
