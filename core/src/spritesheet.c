#include "spritesheet.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap/hashmap.h"
#include "raylib.h"

Animation* spritesheet_anim_get(SpriteSheet* sheet, const char* name) {
    Animation* result = NULL;
    bool got          = hmap_get(sheet->animations, name, strlen(name), (void**)&result);
    assert(got && "Failed to get animation");
    return result;
}

Animation* spritesheet_anim_delete(SpriteSheet* sheet, const char* name) {
    Animation* result = NULL;
    bool deleted      = hmap_take(sheet->animations, name, strlen(name), (void**)&result);
    assert(deleted && "Deletion of animation cannot fail");
    return result;
}

void spritesheet_anim_add(SpriteSheet* sheet, const char* name, Animation* anim) {
    bool added = hmap_put(sheet->animations, name, strlen(name), anim);
    assert(added && "Failed to add animation to Sprite sheet!");
}

void spritesheet_draw(const SpriteSheet* sheet, Vector2 position, int index) {
    assert(index >= 0 && index < sheet->cols * sheet->rows && "Index out of bounds");

    int x          = index % sheet->cols;
    int y          = index / sheet->cols;
    float w        = (float)sheet->texture.width / sheet->cols;
    float h        = (float)sheet->texture.height / sheet->cols;
    Rectangle rect = {
        .x      = (float)x * w,
        .y      = (float)y * h,
        .width  = w,
        .height = h,
    };

    Rectangle dst = {
        .x      = position.x,
        .y      = position.y,
        .width  = w,
        .height = h,
    };

    DrawTexturePro(sheet->texture, rect, dst, (Vector2){0, 0}, 0.0f, WHITE);
}

void spritesheet_delete(SpriteSheet* sheet) {
    HashMapIter* iter = hmap_iter(sheet->animations);
    const char* key   = NULL;
    while (hmap_iter_next(iter, &key) != NULL) {
        Animation* anim;
        if (hmap_take(sheet->animations, key, strlen(key), (void**)&anim)) {
            free(anim->frames);
            free(anim);
        }
    }

    free(sheet);
}

SpriteSheet* spritesheet_new(const char* filepath, int rows, int cols) {
    Texture2D texture = LoadTexture(filepath);
    if (texture.id == 0) {
        return NULL;
    }

    SpriteSheet* sheet = malloc(sizeof(*sheet));
    assert(sheet != NULL && "Sprite sheet cannot be NULL!");
    sheet->texture    = texture;
    sheet->rows       = rows;
    sheet->cols       = cols;
    sheet->animations = hmap_new();

    return sheet;
}
