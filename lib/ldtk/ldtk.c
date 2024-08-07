#include "ldtk.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yyjson.h>

#include "array/array.h"

#define STRING(json, key)                                              \
    ({                                                                 \
        void* obj = yyjson_obj_get(json, key);                         \
        obj&& yyjson_is_str(obj) ? strdup(yyjson_get_str(obj)) : NULL; \
    })

#define BOOL(json, key)                        \
    ({                                         \
        void* obj = yyjson_obj_get(json, key); \
        obj ? yyjson_get_bool(obj) : false;    \
    })

#define INT(json, key)                         \
    ({                                         \
        void* obj = yyjson_obj_get(json, key); \
        obj ? yyjson_get_int(obj) : 0;         \
    })

#define FLOAT(json, key)                       \
    ({                                         \
        void* obj = yyjson_obj_get(json, key); \
        obj ? yyjson_get_num(obj) : 0;         \
    })

#define VEC2(array, json, key)                   \
    {                                            \
        void* obj = yyjson_obj_get(json, key);   \
        size_t idx, max;                         \
        yyjson_val* val;                         \
        yyjson_arr_foreach(obj, idx, max, val) { \
            array[idx] = yyjson_get_num(val);    \
        }                                        \
    }

#define STRING_ARRAY(array, json, key)                       \
    {                                                        \
        void* obj = yyjson_obj_get(json, key);               \
        if (array == NULL) {                                 \
            perror("failed to alloc string array");          \
        }                                                    \
        size_t idx, max;                                     \
        yyjson_val* value;                                   \
        yyjson_arr_foreach(obj, idx, max, value) {           \
            array_push(array, (void*)yyjson_get_str(value)); \
        }                                                    \
    }

#define FREE(el) \
    if (el != NULL) free(el)

static void ldtk_entity_parse(LDTK_Layer* this, yyjson_val* root) {
    yyjson_val* instances         = yyjson_obj_get(root, "entityInstances");
    this->entity_instances_length = yyjson_arr_size(instances);
    size_t capacity = sizeof(LDTK_Entity) * this->entity_instances_length;
    if (capacity == 0) {
        this->entity_instances_length = 0;
        this->entity_instances        = NULL;
        return;
    }

    void* temp = malloc(capacity);
    if (temp == NULL) {
        perror("failed to realloc entity instances");
        return;
    }

    this->entity_instances = temp;

    size_t idx, max;
    yyjson_val* json;
    yyjson_arr_foreach(instances, idx, max, json) {
        LDTK_Entity it   = {0};

        it.__identifier  = STRING(json, "__identifier");
        it.__smart_color = STRING(json, "__smartColor");
        it.iid           = STRING(json, "iid");
        it.width         = INT(json, "width");
        it.height        = INT(json, "height");
        it.def_uid       = INT(json, "defUid");
        it.__world_x     = INT(json, "__worldX");
        it.__world_y     = INT(json, "__worldY");

        VEC2(it.__grid, json, "__grid");
        VEC2(it.__pivot, json, "__pivot");
        VEC2(it.px, json, "px");

        yyjson_val* tags = yyjson_obj_get(json, "__tags");
        if (tags && yyjson_is_arr(tags)) {
            it.__tags_length = yyjson_arr_size(tags);
            it.__tags        = malloc(sizeof(char*) * it.__tags_length);
            if (it.__tags == NULL) {
                fprintf(stderr, "failed to allocate __tags");
            } else {
                size_t idx, max;
                yyjson_val* tag;
                yyjson_arr_foreach(tags, idx, max, tag) {
                    if (yyjson_is_str(tag)) {
                        it.__tags[idx] = strdup(yyjson_get_str(tag));
                    } else {
                        it.__tags[idx] = NULL;
                    }
                }
            }
        }

        yyjson_val* tile = yyjson_obj_get(json, "__tile");
        if (tile != NULL) {
            it.__tile.tileset_uid = INT(tile, "tilesetUid");
            it.__tile.x           = INT(tile, "x");
            it.__tile.y           = INT(tile, "y");
            it.__tile.w           = INT(tile, "w");
            it.__tile.h           = INT(tile, "h");
        }

        this->entity_instances[idx] = it;
    }
}

static void ldtk_parse_tile(LDTK_Layer* layer, yyjson_val* root) {
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

static void ldtk_parse_layer(LDTK_Level* level, yyjson_val* root) {
    yyjson_val* json;
    size_t idx, max;
    yyjson_arr_foreach(root, idx, max, json) {
        LDTK_Layer li           = {0};

        li.__c_height           = INT(json, "__cHei");
        li.__c_width            = INT(json, "__cWid");
        li.__grid_size          = INT(json, "__gridSize");
        li.__identifier         = STRING(json, "__identifier");
        li.__opacity            = FLOAT(json, "__opacity");
        li.__px_total_offset_x  = INT(json, "__pxTotalOffsetX");
        li.__px_total_offset_y  = INT(json, "__pxTotalOffsetY");
        li.__tileset_def_uid    = INT(json, "__tilesetDefUid");
        li.__tileset_rel_path   = STRING(json, "__tilesetRelPath");
        li.__type               = STRING(json, "__type");
        li.iid                  = STRING(json, "iid");
        li.layer_def_uid        = INT(json, "layerDefUid");
        li.level_id             = INT(json, "levelId");
        li.override_tileset_uid = INT(json, "overrideTilesetUid");
        li.px_offset_x          = INT(json, "pxOffsetX");
        li.px_offset_y          = INT(json, "pxOffsetY");
        li.visible              = BOOL(json, "visible");

        {  // parse int grid
            yyjson_val* int_grid = yyjson_obj_get(json, "intGridCsv");
            size_t length        = yyjson_arr_size(int_grid);

            if (length > 0) {
                li.int_grid_csv_length = length;
                li.int_grid_csv        = malloc(sizeof(int) * length);
                if (li.int_grid_csv == NULL) {
                    perror("failed to allocate int_grid_csv");
                    return;
                }

                size_t idx, max;
                yyjson_val* json;
                yyjson_arr_foreach(int_grid, idx, max, json) {
                    li.int_grid_csv[idx] = yyjson_get_int(json);
                }
            }
        }

        ldtk_parse_tile(&li, json);
        ldtk_entity_parse(&li, json);
        level->layer_instances[idx] = li;
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

        level->__bg_color             = STRING(json, "__bgColor");
        level->bg_rel_path            = STRING(json, "bgRelPath");
        level->external_rel_path      = STRING(json, "externalRelPath");
        level->identifier             = STRING(json, "identifier");
        level->iid                    = STRING(json, "iid");
        level->px_height              = INT(json, "pxHei");
        level->px_width               = INT(json, "pxWid");
        level->world_depth            = INT(json, "worldDepth");
        level->world_x                = INT(json, "worldX");
        level->world_y                = INT(json, "worldY");
        level->__bg_pos               = NULL;
        level->__neighbours           = NULL;
        level->__neighbours_length    = 0;
        level->layer_instances        = NULL;
        level->layer_instances_length = 0;
        level->field_instances        = NULL;
        level->field_instances        = NULL;
        level->field_instances_length = 0;

        yyjson_val* bg_pos            = yyjson_obj_get(json, "__bgPos");
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

            level->__neighbours_length = yyjson_arr_size(neighbours);
            if (level->__neighbours_length > 0) {
                size_t n_size   = sizeof(LDTK_Neighbour);
                size_t capacity = n_size * level->__neighbours_length;
                void* temp      = malloc(capacity);
                if (temp == NULL) {
                    fprintf(stderr, "failed to malloc neighbour");
                    exit(EXIT_FAILURE);
                }

                level->__neighbours = temp;
                yyjson_arr_foreach(neighbours, idx, max, json) {
                    LDTK_Neighbour nb;
                    nb.dir                   = STRING(json, "dir");
                    nb.level_iid             = STRING(json, "levelIid");
                    level->__neighbours[idx] = nb;
                }
            }
        }

        yyjson_val* layers = yyjson_obj_get(json, "layerInstances");
        if (layers != NULL && yyjson_is_arr(layers)) {
            size_t size                   = yyjson_arr_size(layers);
            level->layer_instances_length = size;
            level->layer_instances        = malloc(sizeof(LDTK_Layer) * size);
            if (level->layer_instances == NULL) {
                fprintf(stderr, "failed to allocate layer_instances");
            } else {
                ldtk_parse_layer(level, layers);
            }
        }

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
    root->levels            = NULL;
    root->levels_length     = 0;
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

LDTK_Entity* ldtk_entity_next(LDTK_EntityIter* iter) {
    while (iter->index < iter->layer->entity_instances_length) {
        LDTK_Entity* ent = &iter->layer->entity_instances[iter->index];
        iter->index++;
        return ent;
    }

    return NULL;
}

LDTK_EntityIter ldtk_entity_iter(const LDTK_Level* level, const char* layer) {
    return (LDTK_EntityIter){
        .layer  = ldtk_layer_get(level, layer),
        .entity = NULL,
        .index  = 0,
    };
}

LDTK_Layer* ldtk_layer_get(const LDTK_Level* level, const char* key) {
    for (int i = 0; i < level->layer_instances_length; i++) {
        LDTK_Layer* layer = &level->layer_instances[i];
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

void ldtk_entity_free(LDTK_Entity* entity) {
    FREE(entity->__identifier);
    FREE(entity->__smart_color);
    FREE(entity->iid);

    for (int i = 0; i < entity->__tags_length; i++) {
        FREE(entity->__tags[i]);
    }

    FREE(entity->field_instances);
    FREE(entity->__tags);
}

void ldtk_layer_free(LDTK_Layer* layer) {
    FREE(layer->__identifier);
    FREE(layer->__tileset_rel_path);
    FREE(layer->__type);
    FREE(layer->iid);

    for (int i = 0; i < layer->entity_instances_length; i++) {
        ldtk_entity_free(&layer->entity_instances[i]);
    }

    FREE(layer->entity_instances);
    FREE(layer->int_grid_csv);
    FREE(layer->auto_layer_tiles);
    FREE(layer->grid_tiles);
}

void ldtk_level_free(LDTK_Level* level) {
    FREE(level->__bg_color);
    FREE(level->__bg_pos);
    FREE(level->bg_rel_path);
    FREE(level->external_rel_path);
    FREE(level->identifier);
    FREE(level->iid);

    for (int i = 0; i < level->__neighbours_length; i++) {
        LDTK_Neighbour* neighbour = &level->__neighbours[i];
        FREE(neighbour->level_iid);
        FREE(neighbour->dir);
    }

    for (int i = 0; i < level->layer_instances_length; i++) {
        ldtk_layer_free(&level->layer_instances[i]);
    }

    FREE(level->layer_instances);
    FREE(level->__neighbours);
    free(level);
}

void ldtk_free(LDTK_Root* root) {
    assert(root != NULL);

    for (int i = 0; i < root->levels_length; i++) {
        ldtk_level_free(root->levels[i]);
    }

    FREE(root->levels);
    FREE(root->bg_color);
    FREE(root->json_version);
    FREE(root->iid);
    FREE(root);
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
    yyjson_doc_free(doc);
    return root;
}
