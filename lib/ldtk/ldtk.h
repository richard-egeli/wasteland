#ifndef LIB_LDTK_LDTK_H_
#define LIB_LDTK_LDTK_H_

#include <stdbool.h>
#include <stddef.h>

#ifndef LDTK_ASSET_PREFIX
#define LDTK_ASSET_PREFIX "assets"
#endif

#define LDTK_INT          "Int"
#define LDTK_FLOAT        "Float"
#define LDTK_STRING       "String"
#define LDTK_BOOL         "Boolean"
#define LDTK_POINT        "Point"
#define LDTK_COLOR        "Color"
#define LDTK_INT_ARRAY    "Array<Int>"
#define LDTK_FLOAT_ARRAY  "Array<Float>"
#define LDTK_STRING_ARRAY "Array<String>"
#define LDTK_BOOL_ARRAY   "Array<Boolean>"
#define LDTK_POINT_ARRAY  "Array<Point>"
#define LDTK_COLOR_ARRAY  "Array<Color>"

typedef struct LDTK_Point {
    float x;
    float y;
} LDTK_Point;

typedef struct LDTK_Definition {
} LDTK_Definition;

typedef struct LDTK_LevelNeighbour {
    char* dir;
    char* level_iid;
} LDTK_Neighbour;

typedef struct LDTK_Tile {
    float a;     // alpha/opacity of the tile
    int f;       // flip bits, bit0 flip x, bit1 flip y
    int px[2];   // pixel coordinates of the tiel in the layer
    int src[2];  // pixel coordinates of the tile in the tileset
    int t;       // tile id in the corresponding tileset
} LDTK_Tile;

typedef struct LDTK_Field {
    char* __identifier;
    char* __type;

    union {
        int int32;
        float float32;
        char* string;
        char* color;
        bool boolean;
        LDTK_Point point;
        void* entity_reference_info;
        void* tileset_rect;
        struct {
            int* ptr;
            size_t length;
        } int32_array;
        struct {
            float* ptr;
            size_t length;
        } float32_array;
        struct {
            bool* ptr;
            size_t length;
        } boolean_array;
        struct {
            LDTK_Point* ptr;
            size_t length;
        } point_array;
        struct {
            char** ptr;
            size_t length;
        } string_array;
        struct {
            char** ptr;
            size_t length;
        } color_array;
    } value;
} LDTK_Field;

typedef struct LDTK_Entity {
    int __grid[2];
    char* __identifier;
    float __pivot[2];
    char* __smart_color;
    char** __tags;
    size_t __tags_length;
    struct {
        int tileset_uid;
        int x;
        int y;
        int w;
        int h;
    } __tile;
    int __world_x;
    int __world_y;
    int def_uid;
    LDTK_Field* field_instances;
    size_t field_instances_length;
    int height;
    char* iid;
    int px[2];
    int width;
} LDTK_Entity;

typedef struct LDTK_Layer {
    int __c_height;
    int __c_width;
    int __grid_size;
    char* __identifier;
    float __opacity;
    int __px_total_offset_x;
    int __px_total_offset_y;
    int __tileset_def_uid;
    char* __tileset_rel_path;
    char* __type;
    LDTK_Tile* auto_layer_tiles;
    size_t auto_layer_tiles_length;
    LDTK_Entity* entity_instances;
    size_t entity_instances_length;
    LDTK_Tile* grid_tiles;
    size_t grid_tiles_length;
    char* iid;
    int* int_grid_csv;
    size_t int_grid_csv_length;
    int layer_def_uid;
    int level_id;
    int override_tileset_uid;
    int px_offset_x;
    int px_offset_y;
    bool visible;
} LDTK_Layer;

typedef struct LDTK_LevelBackgroundPosition {
    float crop_rect[4];
    float scale[2];
    int top_left_px[2];
} LDTK_LevelBackgroundPosition;

typedef struct LDTK_Level {
    char* __bg_color;
    LDTK_LevelBackgroundPosition* __bg_pos;
    LDTK_Neighbour* __neighbours;
    size_t __neighbours_length;
    char* bg_rel_path;
    char* external_rel_path;
    LDTK_Field* field_instances;
    size_t field_instances_length;
    char* identifier;
    char* iid;
    LDTK_Layer* layer_instances;
    size_t layer_instances_length;
    int px_height;
    int px_width;
    int uid;
    int world_depth;
    int world_x;
    int world_y;
} LDTK_Level;

typedef enum LDTK_WorldLayout {
    LDTK_WORLD_LAYOUT_NULL,
    LDTK_WORLD_LAYOUT_FREE,
    LDTK_WORLD_LAYOUT_GRID_VANIA,
    LDTK_WORLD_LAYOUT_LINEAR_HORIZONTAL,
    LDTK_WORLD_LAYOUT_LINEAR_VERTICAL,
} LDTK_WorldLayout;

typedef struct LDTK_World {
    const char* identifier;
    const char* iid;
    LDTK_Level** levels;
    int world_grid_height;
    int world_grid_width;
    LDTK_WorldLayout world_layout;
} LDTK_World;

typedef struct LDTK_FieldParser {
    void* userdata;
    void (*int32)(LDTK_Field field, void* userdata);
    void (*float32)(LDTK_Field field, void* userdata);
    void (*boolean)(LDTK_Field field, void* userdata);
    void (*point)(LDTK_Field field, void* userdata);
    void (*string)(LDTK_Field field, void* userdata);
    void (*color)(LDTK_Field field, void* userdata);
    void (*int32_array)(LDTK_Field field, void* userdata);
    void (*float32_array)(LDTK_Field field, void* userdata);
    void (*boolean_array)(LDTK_Field field, void* userdata);
    void (*point_array)(LDTK_Field field, void* userdata);
    void (*string_array)(LDTK_Field field, void* userdata);
    void (*color_array)(LDTK_Field field, void* userdata);
} LDTK_FieldParser;

typedef struct LDTK_Root {
    char* bg_color;
    bool external_levels;
    char* iid;
    char* json_version;
    LDTK_Level** levels;
    size_t levels_length;
    int world_grid_height;
    int world_grid_width;
    LDTK_WorldLayout world_layout;
    LDTK_World** worlds;
} LDTK_Root;

typedef struct LDTK_IntGridValue {
    int v;
    int x;
    int y;
    int s;
} LDTK_IntGridValue;

typedef struct LDTK_IntGridIter {
    const LDTK_Layer* layer;
    LDTK_IntGridValue value;
    size_t index;
} LDTK_IntGridIter;

typedef struct LDTK_EntityIter {
    const LDTK_Layer* layer;
    LDTK_Entity* entity;
    size_t index;
} LDTK_EntityIter;

bool ldtk_field_parse(const LDTK_FieldParser* parser, LDTK_Field field);

LDTK_IntGridValue* ldtk_intgrid_next(LDTK_IntGridIter* iter);

LDTK_IntGridIter ldtk_intgrid_iter(const LDTK_Level* level, const char* layer);

LDTK_Entity* ldtk_entity_next(LDTK_EntityIter* iter);

LDTK_EntityIter ldtk_entity_iter(const LDTK_Level* level, const char* layer);

LDTK_Layer* ldtk_layer_get(const LDTK_Level* level, const char* key);

LDTK_Level* ldtk_level_get(const LDTK_Root* root, const char* key);

void ldtk_free(LDTK_Root* root);

LDTK_Root* ldtk_load(const char* path);

#endif  // LIB_LDTK_LDTK_H_
