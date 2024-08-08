#include "lua_entity.h"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <stdlib.h>

#include "array/array.h"
#include "collision/box_collider.h"
#include "collision/sparse_grid.h"
#include "entity.h"
#include "global.h"
#include "level.h"
#include "texture.h"

#define META_TABLE "EntityMetaTable"

#define GET_INT(L, key)                   \
    ({                                    \
        lua_getfield(L, -1, key);         \
        int value = lua_tointeger(L, -1); \
        lua_pop(L, 1);                    \
        value;                            \
    })

static int lua_entity_move(lua_State* L) {
    Entity* entity = (Entity*)luaL_checkudata(L, 1, META_TABLE);
    float x        = luaL_checknumber(L, 2);
    float y        = luaL_checknumber(L, 3);
    entity->collider->velocity.x += x;
    entity->collider->velocity.y += y;
    return 0;
}

void lua_entity_create_notify(int x, int y, const char* name) {
    lua_getglobal(global.state, "on_entity_spawn");

    if (lua_isfunction(global.state, -1)) {
        lua_pushnumber(global.state, x);
        lua_pushnumber(global.state, y);
        lua_pushstring(global.state, name);

        if (lua_pcall(global.state, 3, 0, 0) != LUA_OK) {
            const char* err = lua_tostring(global.state, -1);
            fprintf(stderr, "error lua func on_entity_spawn: %s\n", err);
            lua_pop(global.state, 1);
        }
    } else {
        fprintf(stderr, "on_entity_spawn is not a function\n");
        lua_pop(global.state, 1);
    }
}

static int lua_entity_create(lua_State* L) {
    Entity* entity = calloc(1, sizeof(Entity));

    printf("Creating Entity\n");
    entity->position.x = luaL_checknumber(L, 1);
    entity->position.y = luaL_checknumber(L, 2);
    entity->collider   = NULL;

    lua_getfield(L, 3, "sprite");
    if (!lua_isnil(L, -1)) {
        lua_getfield(L, -1, "texture");
        entity->sprite.texture = texture_load(lua_tostring(L, -1));
        lua_pop(L, 1);

        entity->sprite.cell_x      = GET_INT(L, "cell_x");
        entity->sprite.cell_y      = GET_INT(L, "cell_y");
        entity->sprite.cell_width  = GET_INT(L, "cell_width");
        entity->sprite.cell_height = GET_INT(L, "cell_height");
        entity->sprite.sort_point  = GET_INT(L, "sort_point");
        entity->sprite.grid_size   = GET_INT(L, "grid_size");
    }
    lua_pop(L, 1);

    lua_getfield(L, 3, "box_collider");
    if (!lua_isnil(L, -1)) {
        int x                  = GET_INT(L, "x");
        int y                  = GET_INT(L, "y");
        int w                  = GET_INT(L, "w");
        int h                  = GET_INT(L, "h");
        entity->collider       = box_collider_new(x, y, w, h);
        entity->collider->type = GET_INT(L, "type");
        spgrid_insert(global.level->sparse_grid, entity->collider);
    }
    lua_pop(L, 1);

    array_push(global.level->entities, entity);
    lua_pushlightuserdata(L, entity);
    luaL_getmetatable(L, META_TABLE);
    if (!lua_istable(L, -1)) {
        return luaL_error(L, "Metatable not found %s", META_TABLE);
    }
    lua_setmetatable(L, -2);

    return 1;
}

void lua_entity_register(lua_State* L) {
    luaL_newmetatable(L, META_TABLE);

    lua_pushfstring(L, "__index");
    lua_newtable(L);

    lua_pushcfunction(L, lua_entity_move);
    lua_setfield(L, -2, "move");

    lua_settable(L, -3);
    lua_pop(L, 1);

    lua_register(L, "create_entity", lua_entity_create);
}
