#ifndef CORE_INCLUDE_SPRITE_H_
#define CORE_INCLUDE_SPRITE_H_

#include <raylib.h>
#include <stdint.h>

typedef struct Sprite {
    Texture texture;
    uint16_t cell_x;
    uint16_t cell_y;
    uint16_t cell_width;
    uint16_t cell_height;
    uint16_t grid_size;
    uint16_t sort_point;
} Sprite;

#endif  // CORE_INCLUDE_SPRITE_H_
