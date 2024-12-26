#include "lua/sprite.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "assets.h"
#include "lauxlib.h"
#include "lua.h"
#include "raylib.h"
#include "rlgl.h"
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

void sprite_draw(const Sprite* sprite, float x, float y) {
    Rectangle src = {
        .x      = sprite->col * sprite->step_x,
        .y      = sprite->row * sprite->step_y,
        .width  = sprite->step_x,
        .height = sprite->step_y,
    };

    Rectangle dst = {
        .x      = x,
        .y      = y,
        .width  = sprite->step_x,
        .height = sprite->step_y,
    };

    float width          = sprite->width;
    float height         = sprite->height;
    Vector2 top_left     = (Vector2){x, y};
    Vector2 top_right    = (Vector2){x + dst.width, y};
    Vector2 bottom_left  = (Vector2){x, y + dst.height};
    Vector2 bottom_right = (Vector2){x + dst.width, y + dst.height};

    rlSetTexture(sprite->texture_id);
    rlBegin(RL_QUADS);

    rlColor4ub(255, 255, 255, 255);
    rlNormal3f(0.0f, 0.0f, 1.0f);  // Normal vector pointing towards viewer

    // Top-left corner for texture and quad
    rlTexCoord2f(src.x / width, src.y / height);
    rlVertex2f(top_left.x, top_left.y);

    // Bottom-left corner for texture and quad
    rlTexCoord2f(src.x / width, (src.y + src.height) / height);
    rlVertex2f(bottom_left.x, bottom_left.y);

    // Bottom-right corner for texture and quad
    rlTexCoord2f((src.x + src.width) / width, (src.y + src.height) / height);
    rlVertex2f(bottom_right.x, bottom_right.y);

    // Top-right corner for texture and quad
    rlTexCoord2f((src.x + src.width) / width, src.y / height);
    rlVertex2f(top_right.x, top_right.y);

    rlEnd();
    rlSetTexture(0);
}

Sprite* sprite_parse(lua_State* L, int node, int idx) {
    assert(lua_istable(L, idx) && "Sprite is not a valid table");
    lua_getfield(L, idx, "sprite");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return NULL;
    }

    const int id       = lua_tointeger(L, -1);
    SpriteSheet* sheet = asset_get(id);
    assert(sheet != NULL && "Sprite sheet cannot be NULL!");
    lua_pop(L, 1);

    Sprite* sprite = malloc(sizeof(*sprite));
    assert(sprite != NULL && "Sprite cannot be NULL!");
    sprite->texture_id = sheet->texture.id;
    sprite->step_x     = (float)sheet->texture.width / sheet->cols;
    sprite->step_y     = (float)sheet->texture.height / sheet->rows;
    sprite->width      = sheet->texture.width;
    sprite->height     = sheet->texture.height;
    sprite->col        = 0;
    sprite->row        = 0;

    return sprite;
}

void sprite_register_api(lua_State* L) {
    luaL_newmetatable(L, "Sprite");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, sprite_func_api, 0);
}
