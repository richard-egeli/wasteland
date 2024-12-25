#ifndef CORE_INCLUDE_SPRITESHEET_H_
#define CORE_INCLUDE_SPRITESHEET_H_

#include <stdbool.h>
#include <stddef.h>

#include "raylib.h"

typedef struct HashMap HashMap;

typedef struct AnimationFrame {
    int row;
    int col;
    float duration;
    bool invert;
} AnimationFrame;

typedef struct Animation {
    const char* name;
    AnimationFrame* frames;
    size_t frames_length;
    bool loop;
} Animation;

typedef struct SpriteSheet {
    Texture2D texture;
    int rows;
    int cols;

    HashMap* animations;
} SpriteSheet;

Animation* spritesheet_anim_get(SpriteSheet* sheet, const char* name);

Animation* spritesheet_anim_delete(SpriteSheet* sheet, const char* name);

void spritesheet_anim_add(SpriteSheet* sheet, const char* name, Animation* anim);

void spritesheet_delete(SpriteSheet* sheet);

void spritesheet_draw(const SpriteSheet* sheet, Vector2 position, int index);

SpriteSheet* spritesheet_new(const char* filepath, int rows, int cols);

#endif  // CORE_INCLUDE_SPRITESHEET_H_
