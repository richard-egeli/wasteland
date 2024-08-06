#include "ldtk.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yyjson.h>

#include "array/array.h"
#include "ldtk/ldtk_defs.h"
#include "ldtk/ldtk_entity.h"

static void ldtk_parse_tile_instance(LDTK_Layer* layer, yyjson_val* root) {
    yyjson_val* grid_tiles         = yyjson_obj_get(root, "gridTiles");
    yyjson_val* auto_layer_tiles   = yyjson_obj_get(root, "autoLayerTiles");
    layer->auto_layer_tiles_length = yyjson_arr_size(auto_layer_tiles);
    layer->grid_tiles_length       = yyjson_arr_size(grid_tiles);

    LDTK_Tile* tiles;
    yyjson_val* json_tiles;
    if (layer->auto_layer_tiles_length > layer->grid_tiles_length) {
        size_t size = layer->auto_layer_tiles_length * sizeof(LDTK_Tile);
        layer->auto_layer_tiles = malloc(size);
        json_tiles              = auto_layer_tiles;
        tiles                   = layer->auto_layer_tiles;
    } else {
        size_t size       = layer->grid_tiles_length * sizeof(LDTK_Tile);
        layer->grid_tiles = malloc(size);
        json_tiles        = grid_tiles;
        tiles             = layer->grid_tiles;
    }

    size_t idx, max;
    yyjson_val* json;
    yyjson_arr_foreach(json_tiles, idx, max, json) {
        LDTK_Tile tile;

        tile.a = FLOAT(json, "a");
        tile.t = FLOAT(json, "t");
        tile.f = INT(json, "f");
        VEC2(tile.px, json, "px");
        VEC2(tile.src, json, "src");

        tiles[idx] = tile;
    }
}

static void ldtk_parse_layer_instances(LDTK_Level* level, yyjson_val* root) {
    if (root != NULL) {
        yyjson_val* json;
        size_t idx, max;
        yyjson_arr_foreach(root, idx, max, json) {
            LDTK_Layer* li = malloc(sizeof(*li));
            if (li == NULL) {
                perror("failed to allocate layer instance");
                return;
            }

            li->__c_height           = INT(json, "__cHei");
            li->__c_width            = INT(json, "__cWid");
            li->__grid_size          = INT(json, "__gridSize");
            li->__identifier         = STRING(json, "__identifier");
            li->__opacity            = FLOAT(json, "__opacity");
            li->__px_total_offset_x  = INT(json, "__pxTotalOffsetX");
            li->__px_total_offset_y  = INT(json, "__pxTotalOffsetY");
            li->__tileset_def_uid    = INT(json, "__tilesetDefUid");
            li->__tileset_rel_path   = STRING(json, "__tilesetRelPath");
            li->__type               = STRING(json, "__type");
            li->iid                  = STRING(json, "iid");
            li->layer_def_uid        = INT(json, "layerDefUid");
            li->level_id             = INT(json, "levelId");
            li->override_tileset_uid = INT(json, "overrideTilesetUid");
            li->px_offset_x          = INT(json, "pxOffsetX");
            li->px_offset_y          = INT(json, "pxOffsetY");
            li->visible              = BOOL(json, "visible");
            li->entity_instances     = array_new();
            li->int_grid_csv         = NULL;
            li->int_grid_csv_length  = 0;

            {  // parse int grid
                yyjson_val* int_grid = yyjson_obj_get(json, "intGridCsv");
                size_t length        = yyjson_arr_size(int_grid);

                if (length > 0) {
                    li->int_grid_csv_length = length;
                    li->int_grid_csv        = malloc(sizeof(int) * length);
                    if (li->int_grid_csv == NULL) {
                        perror("failed to allocate int_grid_csv");
                        free(li);
                        return;
                    }

                    size_t idx, max;
                    yyjson_val* json;
                    yyjson_arr_foreach(int_grid, idx, max, json) {
                        li->int_grid_csv[idx] = yyjson_get_int(json);
                    }
                }
            }

            ldtk_parse_tile_instance(li, json);
            ldtk_entity_parse(li, json);
            array_push(level->layer_instances, li);
        }
    }
}

static void ldtk_parse_levels(LDTK_Root* this, yyjson_val* root) {
    size_t idx, max;
    yyjson_val* json;
    yyjson_val* levels  = yyjson_obj_get(root, "levels");
    this->levels_length = yyjson_arr_size(levels);
    this->levels        = malloc(this->levels_length * sizeof(void*));

    yyjson_arr_foreach(levels, idx, max, json) {
        LDTK_Level* level = malloc(sizeof(*level));
        if (level == NULL) {
            perror("failed to malloc new ldtk level");
            return;
        }

        level->__bg_color        = STRING(json, "__bgColor");
        level->bg_rel_path       = STRING(json, "bgRelPath");
        level->external_rel_path = STRING(json, "externalRelPath");
        level->identifier        = STRING(json, "identifier");
        level->iid               = STRING(json, "iid");
        level->px_height         = INT(json, "pxHei");
        level->px_width          = INT(json, "pxWid");
        level->world_depth       = INT(json, "worldDepth");
        level->world_x           = INT(json, "worldX");
        level->world_y           = INT(json, "worldY");
        level->__bg_pos          = NULL;
        level->__neighbours      = array_new();
        level->layer_instances   = array_new();
        level->field_instances   = array_new();

        yyjson_val* bg_pos       = yyjson_obj_get(json, "__bgPos");
        if (bg_pos != NULL) {
            LDTK_LevelBackgroundPosition* bg = malloc(sizeof(*bg));
            if (bg != NULL) {
                VEC2(bg->crop_rect, json, "cropRect");
                VEC2(bg->scale, json, "scale");
                VEC2(bg->top_left_px, json, "topLeftPx");
                level->__bg_pos = bg;
            }
        }

        yyjson_val* neighbours = yyjson_obj_get(json, "__neighbours");
        if (neighbours != NULL) {
            size_t idx, max;
            yyjson_val* json;
            yyjson_arr_foreach(neighbours, idx, max, json) {
                LDTK_LevelNeighbour* nb = malloc(sizeof(*nb));
                if (nb != NULL) {
                    nb->dir       = STRING(json, "dir");
                    nb->level_iid = STRING(json, "levelIid");
                    array_push(level->__neighbours, nb);
                }
            }
        }

        yyjson_val* layer_instances = yyjson_obj_get(json, "layerInstances");
        ldtk_parse_layer_instances(level, layer_instances);

        this->levels[idx] = level;
    }
}

static void ldtk_parse_root(LDTK_Root* root, yyjson_val* json) {
    root->bg_color          = STRING(json, "bgColor");
    root->json_version      = STRING(json, "jsonVersion");
    root->iid               = STRING(json, "iid");
    root->external_levels   = BOOL(json, "externalLevels");
    root->world_grid_width  = INT(json, "worldGridWidth");
    root->world_grid_height = INT(json, "worldGridHeight");
    root->world_layout      = INT(json, "worldLayout");
    ldtk_parse_levels(root, json);
}

static size_t ldtk_load_file_content(const char* path, char** content) {
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "failed to load file: %s\n", path);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    rewind(file);

    char* buffer = malloc(length + 1);
    if (buffer == NULL) {
        perror("failed to malloc buffer for fread");
        fclose(file);
        return 0;
    }

    fread(buffer, length, 1, file);
    fclose(file);

    *content = buffer;
    return length;
}

LDTK_IntGridValue* ldtk_intgrid_next(LDTK_IntGridIter* iter) {
    while (iter->index < iter->layer->int_grid_csv_length) {
        iter->value.v = iter->layer->int_grid_csv[iter->index];
        if (iter->value.v > 0) {
            const size_t size  = iter->layer->__grid_size;
            const size_t width = iter->layer->__c_width;
            iter->value.s      = size;
            iter->value.x      = (iter->index % width) * size;
            iter->value.y      = (iter->index / width) * size;
            iter->index++;
            return &iter->value;
        }

        iter->index++;
    }

    return NULL;
}

LDTK_IntGridIter ldtk_intgrid_iter(const LDTK_Level* level, const char* layer) {
    return (LDTK_IntGridIter){
        .layer = ldtk_layer_get(level, layer),
        .value = {0},
        .index = 0,
    };
}

LDTK_Layer* ldtk_layer_get(const LDTK_Level* level, const char* key) {
    for (int i = 0; i < array_length(level->layer_instances); i++) {
        LDTK_Layer* layer = array_get(level->layer_instances, i);
        if (strcmp(layer->__identifier, key) == 0) {
            return layer;
        }
    }

    return NULL;
}

LDTK_Level* ldtk_level_get(const LDTK_Root* root, const char* key) {
    for (int i = 0; i < root->levels_length; i++) {
        LDTK_Level* level = root->levels[i];
        if (strcmp(level->identifier, key) == 0) {
            return level;
        }
    }

    return NULL;
}

LDTK_Root* ldtk_load(const char* path) {
    char* content = NULL;
    size_t length = ldtk_load_file_content(path, &content);
    if (length <= 0 || content == NULL) {
        perror("failed to load file content");
        return NULL;
    }

    yyjson_doc* doc = yyjson_read(content, length, 0);
    if (doc == NULL) {
        perror("failed to parse json file\n");
        free(content);
        return NULL;
    }

    yyjson_val* json = yyjson_doc_get_root(doc);
    free(content);

    LDTK_Root* root = malloc(sizeof(*root));
    if (root == NULL) {
        perror("failed to malloc LDTK_Map");
        yyjson_doc_free(doc);
        return NULL;
    }

    ldtk_parse_root(root, json);
    return root;
}
