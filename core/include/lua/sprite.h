#ifndef CORE_INCLUDE_LUA_SPRITE_H_
#define CORE_INCLUDE_LUA_SPRITE_H_

typedef struct lua_State lua_State;

typedef struct SpriteSheet SpriteSheet;

typedef struct Sprite {
    int row;
    int col;
    int texture_id;
    int width;
    int height;
    float step_x;
    float step_y;
    SpriteSheet* spritesheet;
} Sprite;

void sprite_draw(const Sprite* sprite, float x, float y);

Sprite* sprite_parse(lua_State* L, int node, int idx, Sprite* sprite);

int sprite_create(lua_State* L);

void register_sprite_api(lua_State* L);

#endif  // CORE_INCLUDE_LUA_SPRITE_H_
