#ifndef ASEPRITE_ASE_DEFS_H_
#define ASEPRITE_ASE_DEFS_H_

#include <stdint.h>

#define BYTE   uint8_t
#define WORD   uint16_t
#define SHORT  int16_t
#define DWORD  uint32_t
#define LONG   int32_t
#define FLOAT  float
#define DOUBLE double
#define QWORD  uint64_t
#define LONG64 int64_t

#define PACKED __attribute__((packed))

typedef struct PACKED ASE_String {
    WORD length;
    BYTE characters[0];
} ASE_String;

typedef struct PACKED ASE_Point {
    LONG x;
    LONG y;
} ASE_Point;

typedef struct PACKED ASE_Size {
    LONG width;
    LONG height;
} ASE_Size;

typedef struct PACKED ASE_Rect {
    ASE_Point origin;
    ASE_Size size;
} ASE_Rect;

typedef struct PACKED ASE_Pixel {
    union {
        BYTE rgba[4];
        BYTE grayscale[2];
        BYTE indexed;
    };
} ASE_Pixel;

typedef struct PACKED ASE_Tile {
    union {
        BYTE bit8;
        WORD bit16;
        DWORD bit32;
    };
} ASE_Tile;

typedef struct PACKED ASE_Header {
    DWORD file_size;
    WORD magic_number;  /// always 0xA5E0
    WORD frames;
    WORD width_in_pixels;
    WORD height_in_pixels;
    WORD color_depth;
    DWORD flags;
    WORD speed;
    DWORD set0;
    DWORD set1;
    BYTE palette_entry;
    BYTE reserved[3];
    WORD number_of_colors;
    BYTE pixel_width;
    BYTE pixel_height;
    SHORT x;
    SHORT y;
    WORD grid_width;
    WORD grid_height;
    BYTE future_use[84];
} ASE_Header;

typedef struct PACKED ASE_FrameHeader {
    DWORD bytes_in_frame;
    WORD magic_number;          /// always 0xF1FA
    WORD old_number_of_chunks;  /// old field for number of chunks
    WORD frame_duration;
    BYTE reserved[2];
    DWORD number_of_chunks;  /// if zero use the old field
} ASE_FrameHeader;

typedef struct PACKED ASE_ChunkHeader {
    DWORD chunk_size;
    WORD chunk_type;
    BYTE chunk_data[0];
} ASE_ChunkHeader;

typedef struct PACKED ASE_OldPaletteChunk {
    WORD number_of_packets;
    struct {
        BYTE number_to_skip;
        BYTE numer_of_colors;
        struct {
            BYTE red;
            BYTE green;
            BYTE blue;
        } colors[0];
    } foreach[0];
} ASE_OldPaletteChunk;

typedef enum ASE_LayerFlags {
    ASE_LAYER_FLAGS_VISIBLE               = 1,
    ASE_LAYER_FLAGS_EDITABLE              = 2,
    ASE_LAYER_FLAGS_LOCK_MOVEMENT         = 4,
    ASE_LAYER_FLAGS_BACKGROUND            = 8,
    ASE_LAYER_FLAGS_PREFER_LINKED_CELS    = 16,
    ASE_LAYER_FLAGS_LAYER_GROUP_COLLAPSED = 32,
    ASE_LAYER_FLAGS_REFERENCE_LAYER       = 64,
} ASE_LayerFlags;

typedef enum ASE_LayerType {
    ASE_LAYER_TYPE_NORMAL,
    ASE_LAYER_TYPE_GROUP,
    ASE_LAYER_TYPE_TILEMAP,
} ASE_LayerType;

typedef enum ASE_BlendMode {
    ASE_BLEND_MODE_NORMAL      = 0,
    ASE_BLEND_MODE_MULTIPLY    = 1,
    ASE_BLEND_MODE_SCREEN      = 2,
    ASE_BLEND_MODE_OVERLAY     = 3,
    ASE_BLEND_MODE_DARKEN      = 4,
    ASE_BLEND_MODE_LIGHTEN     = 5,
    ASE_BLEND_MODE_COLOR_DODGE = 6,
    ASE_BLEND_MODE_COLOR_BURN  = 7,
    ASE_BLEND_MODE_HARD_LIGHT  = 8,
    ASE_BLEND_MODE_SOFT_LIGHT  = 9,
    ASE_BLEND_MODE_DIFFERENCE  = 10,
    ASE_BLEND_MODE_EXCLUSION   = 11,
    ASE_BLEND_MODE_HUE         = 12,
    ASE_BLEND_MODE_SATURATION  = 13,
    ASE_BLEND_MODE_COLOR       = 14,
    ASE_BLEND_MODE_LUMINOSITY  = 15,
    ASE_BLEND_MODE_ADDITION    = 16,
    ASE_BLEND_MODE_SUBTRACT    = 17,
    ASE_BLEND_MODE_DIVIDE      = 18,
} ASE_BlendMode;

typedef struct PACKED ASE_LayerChunk {
    WORD flags;
    WORD layer_type;
    WORD layer_child_level;
    WORD default_layer_width_in_pixels;
    WORD default_layer_height_in_pixels;
    WORD blend_mode;
    BYTE opacity;  /// valid only if header flags field has bit 1 set
    BYTE for_future_use[3];
    ASE_String layer_name;
    DWORD tileset_index;  /// only if layer type == 2
} ASE_LayerChunk;

typedef enum ASE_CelType {
    ASE_CEL_TYPE_RAW_IMAGE          = 0,
    ASE_CEL_TYPE_LINKED_CEL         = 1,
    ASE_CEL_TYPE_COMPRESSED_IMAGE   = 2,
    ASE_CEL_TYPE_COMPRESSED_TILEMAP = 3,
} ASE_CelType;

typedef struct PACKED ASE_CelChunk {
    WORD layer_index;
    SHORT x;
    SHORT y;
    BYTE opacity;
    WORD cel_type;
    SHORT z_index;
    BYTE for_future_use[5];
    union {
        struct {
            WORD width_in_pixels;
            WORD height_in_pixels;
            ASE_Pixel raw_pixel_data[0];
        } raw_image;

        struct {
            WORD frame_position_to_link;
        } linked_cell;

        struct {
            WORD width_in_pixels;
            WORD height_in_pixels;
            ASE_Pixel raw_cel[0];
        } compressed_image;

        struct {
            WORD width_in_tiles;
            WORD height_in_tiles;
            WORD bits_per_tile;
            DWORD bitmask_for_tile_id;
            DWORD bitmask_for_x_flip;
            DWORD bitmask_for_y_flip;
            DWORD bitmask_for_diagonal_flip;
            BYTE reserved[10];
            ASE_Tile tiles[0];
        } compressed_tilemap;
    };
} ASE_CelChunk;

#endif  // ASEPRITE_ASE_DEFS_H_
