#include "level_test.h"

#include <stdio.h>

#include "array/array.h"
#include "collision/box_collider.h"
#include "collision/sparse_grid.h"
#include "ldtk/ldtk.h"
#include "ldtk/tilemap.h"

Level_Test level_test_load() {
    LDTK_Root* root   = ldtk_load("assets/test.ldtk");
    LDTK_Level* level = ldtk_level_get(root, "Level_0");

    Tilemap* tilemap  = tilemap_from_ldtk(level);
    SPGrid* grid      = spgrid_new();

    LDTK_IntGridValue* ig;
    LDTK_IntGridIter ig_iter = ldtk_intgrid_iter(level, "Collisions");
    while ((ig = ldtk_intgrid_next(&ig_iter))) {
        BoxCollider* col = box_collider_new(ig->x, ig->y, ig->s, ig->s);
        spgrid_insert(grid, col);
    }

    return (Level_Test){
        .tilemap = tilemap,
        .grid    = grid,
    };
}

void level_test_update(const Level_Test* level) {
    spgrid_resolve(level->grid, GetFrameTime());

    Tile* tile;
    TileIter it = tilemap_tile_iter(level->tilemap);
    while ((tile = tilemap_tile_next(&it))) {
        tilemap_tile_draw(level->tilemap, tile);
    }

    for (int i = 0; i < array_length(level->tilemap->entities); i++) {
        const Entity* ent = array_get(level->tilemap->entities, i);

        if (ent != NULL && ent->texture != NULL) {
            Rectangle src = ent->source;
            Rectangle dst = {
                ent->position.x,
                ent->position.y,
                ent->source.width,
                ent->source.height,
            };

            DrawTexturePro(*ent->texture, src, dst, (Vector2){0}, 0, WHITE);
        }
    }
}
