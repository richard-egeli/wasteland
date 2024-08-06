#ifndef LIB_LDTK_LDTK_ENTITY_H_
#define LIB_LDTK_LDTK_ENTITY_H_

#include <stdbool.h>
#include <stddef.h>

typedef struct LDTK_Layer LDTK_Layer;

typedef struct yyjson_val yyjson_val;

typedef struct Array Array;

typedef struct LDTK_GridPoint {
    int cx;
    int cy;
} LDTK_GridPoint;

typedef struct LDTK_FieldInstance {
    const char* __identifier;
    void* __tile;
    const char* __type;

    union {
        int int32;
        float float32;
        const char* string;
        bool boolean;
        LDTK_GridPoint grid_point;
        void* entity_reference_info;
        void* tileset_rect;
    } value;
} LDTK_FieldInstance;

typedef struct LDTK_Entity {
    int __grid[2];
    const char* __identifier;
    float __pivot[2];
    const char* __smart_color;
    Array* __tags;
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
    LDTK_FieldInstance* field_instances;
    size_t field_instances_length;
    int height;
    const char* iid;
    int px[2];
    int width;
} LDTK_Entity;

void ldtk_entity_parse(LDTK_Layer* this, yyjson_val* root);

#endif  // LIB_LDTK_LDTK_ENTITY_H_
