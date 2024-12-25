#ifndef CORE_INCLUDE_ASSETS_H_
#define CORE_INCLUDE_ASSETS_H_

typedef struct SpriteSheet SpriteSheet;

extern SpriteSheet** spritesheet_assets;

void* asset_get(int key);

int asset_add(void* data);

#endif  // CORE_INCLUDE_ASSETS_H_
