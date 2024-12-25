#ifndef CORE_INCLUDE_LUA_SPRITE_H_
#define CORE_INCLUDE_LUA_SPRITE_H_

typedef struct lua_State lua_State;

typedef struct Sprite {
    int cell_index;
    int id;
    int rows;
    int cols;
    int width;
    int height;
} Sprite;

int sprite_parse(lua_State* L, int idx);

void sprite_register_api(lua_State* L);

#endif  // CORE_INCLUDE_LUA_SPRITE_H_
