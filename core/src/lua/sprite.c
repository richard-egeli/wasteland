#include "lua/sprite.h"

#include <assert.h>

#include "assets.h"
#include "lauxlib.h"
#include "lua.h"
#include "spritesheet.h"

static int sprite_set_cell(lua_State* L) {
    assert(lua_gettop(L) == 3 && "Not enough arguments (sprite, col, row)");
    assert(lua_isuserdata(L, 1) && "Invalid first argument");
    assert(lua_isnumber(L, 2) && "Invalid second argument");
    assert(lua_isnumber(L, 3) && "Invalid third argument");

    const Sprite* sprite = *(Sprite**)lua_touserdata(L, 1);
    const int col        = lua_tointeger(L, 2);
    const int row        = lua_tointeger(L, 3);

    return 0;
}

static luaL_Reg sprite_func_api[] = {
    {"set_cell", sprite_set_cell},
    {NULL, NULL},
};

int sprite_parse(lua_State* L, int idx) {
    assert(lua_istable(L, idx) && "Sprite is not a valid table");
    lua_getfield(L, idx, "sprite");
    if (lua_isnil(L, 1)) {
        lua_pop(L, 1);
        return 0;
    }

    const int id       = lua_tointeger(L, -1);
    SpriteSheet* sheet = asset_get(id);
    assert(sheet != NULL && "Sprite sheet cannot be NULL!");
    lua_pop(L, 1);

    return (Sprite){
        .cols       = sheet->cols,
        .rows       = sheet->rows,
        .id         = sheet->texture.id,
        .width      = sheet->texture.width,
        .height     = sheet->texture.height,
        .cell_index = 0,
    };
}

void sprite_register_api(lua_State* L) {
    luaL_newmetatable(L, "Sprite");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, sprite_func_api, 0);
}
