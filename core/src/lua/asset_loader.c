#include "lua/asset_loader.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "assets.h"
#include "lauxlib.h"
#include "lua.h"
#include "spritesheet.h"

static Animation** parse_spritesheet_animations(lua_State* L, int idx, size_t* length) {
    size_t anim_len = lua_objlen(L, idx);
    if (anim_len == 0) return NULL;

    Animation** anims = malloc(sizeof(Animation*) * anim_len);
    assert(anims != NULL && "Animation cannot be NULL!");

    for (size_t i = 1; i <= anim_len; i++) {
        Animation* anim = malloc(sizeof(*anim));
        assert(anim != NULL && "Animation cannot be NULL!");

        lua_rawgeti(L, idx, i);
        assert(lua_istable(L, -1) && "Animation entry not valid");

        lua_getfield(L, -1, "name");
        const char* name = lua_tostring(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "loop");
        bool loop = lua_toboolean(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "frames");
        size_t anim_frames_len = lua_objlen(L, -1);

        anim->name             = name;
        anim->loop             = loop;
        anim->frames           = NULL;
        anim->frames_length    = 0;

        if (anim_frames_len > 0) {
            AnimationFrame* frames = malloc(sizeof(*frames) * anim_frames_len);
            assert(frames != NULL && "Animation frame cannot be NULL!");

            for (size_t j = 1; j <= anim_frames_len; j++) {
                lua_rawgeti(L, -1, j);
                assert(lua_istable(L, -1) && "Animation frame is not valid");

                lua_getfield(L, -1, "row");
                int row = lua_tointeger(L, -1);
                lua_pop(L, 1);

                lua_getfield(L, -1, "column");
                int column = lua_tointeger(L, -1);
                lua_pop(L, 1);

                lua_getfield(L, -1, "duration");
                float duration = lua_tonumber(L, -1);
                lua_pop(L, 1);

                lua_getfield(L, -1, "invert");
                bool invert = lua_toboolean(L, -1);
                lua_pop(L, 1);

                lua_pop(L, 1);  // Pop the table

                frames[j - 1] = (AnimationFrame){
                    .row      = row,
                    .col      = column,
                    .invert   = invert,
                    .duration = duration,
                };
            }

            anim->frames        = frames;
            anim->frames_length = anim_frames_len;
        }

        lua_pop(L, 1);  // Pop the frames field
        lua_pop(L, 1);  // Pop the table

        anims[i - 1] = anim;
    }

    *length = anim_len;
    return anims;
}

static int load_spritesheet(lua_State* L) {
    assert(lua_gettop(L) == 4 && "Parameters (path, rows, cols) invalid");
    assert(lua_isstring(L, 1) && "Path is not a valid string");
    assert(lua_isnumber(L, 2) && "Cols is not a valid integer");
    assert(lua_isnumber(L, 3) && "Rows is not a valid integer");
    assert(lua_istable(L, 4) && "Animations is not a valid table");

    size_t len          = 0;
    const char* path    = lua_tolstring(L, 1, &len);
    const int cols      = lua_tointeger(L, 2);
    const int rows      = lua_tointeger(L, 3);

    size_t anims_length = 0;
    Animation** anims   = parse_spritesheet_animations(L, 4, &anims_length);

    SpriteSheet* sheet  = spritesheet_new(path, rows, cols);
    assert(sheet != NULL && "Sprite sheet couldn't be loaded");

    for (int i = 0; i < anims_length; i++) {
        spritesheet_anim_add(sheet, anims[i]->name, anims[i]);
    }

    if (anims != NULL) {
        // NOTE: Just an array of pointers to cleanup, because
        // all the animation pointers have been moved into the
        // spritesheet anim hashmap instead
        free(anims);
    }

    int sheet_id = asset_add(sheet);
    lua_pushnumber(L, sheet_id);
    return 1;
}

static luaL_Reg asset_loader_api[] = {
    {"load_spritesheet", load_spritesheet},
    {NULL, NULL},
};

void register_asset_loader_api(lua_State* L) {
    lua_newtable(L);
    luaL_setfuncs(L, asset_loader_api, 0);
    lua_setglobal(L, "AssetLoader");
}
