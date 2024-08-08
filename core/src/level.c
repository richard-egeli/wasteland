#include "level.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array/array.h"
#include "collision/box_collider.h"
#include "collision/sparse_grid.h"
#include "entity.h"
#include "ldtk/ldtk.h"
#include "lua_entity.h"
#include "tilemap.h"

Level* level_new(void) {
    Level* level       = calloc(1, sizeof(*level));
    level->tilemap     = NULL;
    level->sparse_grid = spgrid_new();
    level->entities    = array_new();
    return level;
}

void level_load(Level* level, const char* path, const char* level_id) {
    LDTK_Root* ldtk_root   = ldtk_load(path);
    LDTK_Level* ldtk_level = ldtk_level_get(ldtk_root, level_id);
    level->tilemap         = tilemap_from_ldtk(ldtk_level);

    LDTK_Entity* ent;
    LDTK_EntityIter ent_iter = ldtk_entity_iter(ldtk_level, "Entities");
    while ((ent = ldtk_entity_next(&ent_iter))) {
        Entity* entity             = calloc(1, sizeof(*entity));
        entity->position.x         = ent->__world_x;
        entity->position.y         = ent->__world_y;
        entity->sprite.cell_x      = ent->__tile.x;
        entity->sprite.cell_y      = ent->__tile.y;
        entity->sprite.cell_width  = ent->__tile.w;
        entity->sprite.cell_height = ent->__tile.h;
        entity->sprite.grid_size   = 16;

        lua_entity_create_notify(ent->__world_x,
                                 ent->__world_y,
                                 ent->__identifier);
    }

    LDTK_IntGridValue* ig;
    LDTK_IntGridIter ig_iter = ldtk_intgrid_iter(ldtk_level, "Collisions");
    while ((ig = ldtk_intgrid_next(&ig_iter))) {
        BoxCollider* col = box_collider_new(ig->x, ig->y, ig->s, ig->s);
        spgrid_insert(level->sparse_grid, col);
    }

    ldtk_free(ldtk_root);
}

static int ysort(const void* a, const void* b) {
    const Entity* e1 = *(Entity**)a;
    const Entity* e2 = *(Entity**)b;
    float p1         = e1->position.y + e1->sprite.sort_point;
    float p2         = e2->position.y + e2->sprite.sort_point;
    return p1 - p2;
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
        }
    }
}
