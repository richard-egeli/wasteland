#include "level.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array/array.h"
#include "collision/box_collider.h"
#include "collision/sparse_grid.h"
#include "entity.h"
#include "global.h"
#include "ldtk/ldtk.h"
#include "lua_entity.h"
#include "tilemap.h"

void level_entities_load(Level* level, const char* key, entity_callback cb) {
    LDTK_Entity* ent;
    LDTK_Level* l            = level->ldtk.level;
    LDTK_EntityIter ent_iter = ldtk_entity_iter(l, key);
    while ((ent = ldtk_entity_next(&ent_iter))) {
        int x              = ent->px[0];
        int y              = ent->px[1];
        const char* id     = ent->__identifier;
        LDTK_Field* fields = ent->field_instances;
        size_t len         = ent->field_instances_length;
        cb(x, y, id, fields, len);
    }
}

Level* level_load(const char* path, const char* level_id) {
    LDTK_Root* ldtk_root = ldtk_load(path);

    if (ldtk_root != NULL) {
        Level* level           = calloc(1, sizeof(Level));
        LDTK_Level* ldtk_level = ldtk_level_get(ldtk_root, level_id);
        if (ldtk_level == NULL) {
            perror("tried to load unknown level inside ldtk map");
            ldtk_free(ldtk_root);
            free(level);
            return NULL;
        }

        level->tilemap     = tilemap_from_ldtk(ldtk_level);
        level->sparse_grid = spgrid_new();
        level->entities    = array_new();
        level->ldtk.level  = ldtk_level;
        level->ldtk.root   = ldtk_root;
        global.level       = level;

        LDTK_IntGridValue* ig;
        LDTK_IntGridIter ig_iter = ldtk_intgrid_iter(ldtk_level, "Collisions");
        while ((ig = ldtk_intgrid_next(&ig_iter))) {
            BoxCollider* col = box_collider_new(ig->x, ig->y, ig->s, ig->s);
            spgrid_insert(level->sparse_grid, col);
        }

        return level;
    }

    return NULL;
}

static int ysort(const void* a, const void* b) {
    const Entity* e1 = *(Entity**)a;
    const Entity* e2 = *(Entity**)b;
    float p1         = e1->position.y + e1->sprite.sort_point;
    float p2         = e2->position.y + e2->sprite.sort_point;

    if (p1 > p2) return +1;
    if (p1 < p2) return -1;

    if (e1->position.x > e2->position.x) return +1;
    if (e1->position.x < e2->position.x) return -1;

    return ((uint64_t)e1 > (uint64_t)e2) ? +1 : -1;
}

void level_free(Level* this) {
    for (int i = 0; i < array_length(this->entities); i++) {
        Entity* entity = array_get(this->entities, i);
        if (entity->collider) {
            spgrid_remove(this->sparse_grid, entity->collider);
        }

        entity_free(entity);
    }

    spgrid_free(this->sparse_grid);
    tilemap_free(this->tilemap);
    array_free(this->entities);
    ldtk_free(this->ldtk.root);
}

void level_update(Level* level) {
    spgrid_resolve(level->sparse_grid, GetFrameTime());
    array_sort(level->entities, ysort);

    Tile* tile;
    TileIter it = tilemap_tile_iter(level->tilemap);
    while ((tile = tilemap_tile_next(&it))) {
        tilemap_tile_draw(level->tilemap, tile);
    }

    for (int i = 0; i < array_length(level->entities); i++) {
        Entity* ent = array_get(level->entities, i);
        if (ent->destroyed) continue;

        if (ent != NULL) {
            if (ent->collider != NULL) {
                ent->position.x = ent->collider->position.x;
                ent->position.y = ent->collider->position.y;
            }

            Vector2 org   = {0};
            Rectangle src = {
                .x      = ent->sprite.cell_x,
                .y      = ent->sprite.cell_y,
                .width  = ent->sprite.cell_width,
                .height = ent->sprite.cell_height,
            };

            Rectangle dst = {
                .x      = ent->position.x,
                .y      = ent->position.y,
                .width  = src.width,
                .height = src.height,
            };

            DrawTexturePro(ent->sprite.texture, src, dst, org, 0, WHITE);

#ifndef NDEBUG
            if (ent->collider && ent->collider->debug) {
                Rect r = box_collider_rect(ent->collider);
                DrawRectangleLines(r.x, r.y, r.w, r.h, GREEN);
            }
#endif
        }
    }
}
