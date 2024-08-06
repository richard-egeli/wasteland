#include "ldtk/ldtk_entity.h"

#include <stdlib.h>

#include "array/array.h"
#include "ldtk/ldtk.h"
#include "ldtk/ldtk_defs.h"
#include "yyjson.h"

void ldtk_entity_free(LDTK_Entity* entity) {
    array_free(entity->__tags);
    free(entity);
}

void ldtk_entity_parse(LDTK_Layer* this, yyjson_val* root) {
    yyjson_val* instances = yyjson_obj_get(root, "entityInstances");

    size_t idx, max;
    yyjson_val* json;
    yyjson_arr_foreach(instances, idx, max, json) {
        LDTK_Entity* it   = malloc(sizeof(*it));

        it->__tags        = array_new();
        it->__identifier  = STRING(json, "__identifier");
        it->__smart_color = STRING(json, "__smartColor");
        it->iid           = STRING(json, "iid");
        it->width         = INT(json, "width");
        it->height        = INT(json, "height");
        it->def_uid       = INT(json, "defUid");
        it->__world_x     = INT(json, "__worldX");
        it->__world_y     = INT(json, "__worldY");

        printf("%d %d\n", it->__world_x, it->__world_y);

        STRING_ARRAY(it->__tags, json, "__tags");
        VEC2(it->__grid, json, "__grid");
        VEC2(it->px, json, "px");

        yyjson_val* tile = yyjson_obj_get(json, "__tile");
        if (tile != NULL) {
            it->__tile.tileset_uid = INT(tile, "tilesetUid");
            it->__tile.x           = INT(tile, "x");
            it->__tile.y           = INT(tile, "y");
            it->__tile.w           = INT(tile, "w");
            it->__tile.h           = INT(tile, "h");
        }

        array_push(this->entity_instances, it);
    }
}
