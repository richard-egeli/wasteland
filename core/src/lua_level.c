#include "lua_level.h"

#include <string.h>

#include "global.h"
#include "lauxlib.h"
#include "level.h"
#include "lua.h"
#include "lua_entity.h"
#include "lua_helper.h"
#include "lualib.h"

#define META_TABLE "LevelObjectMetaTable"

typedef struct LevelObject {
    Level* level;
} LevelObject;

static int lua_level_index(lua_State* L) {
    lua_getfenv(L, 1);
    lua_pushvalue(L, 2);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {
        return 1;
    }

    lua_pop(L, 2);

    lua_getmetatable(L, 1);
    lua_pushvalue(L, 2);
    lua_gettable(L, -2);

    return 1;
}

static int lua_level_gc(lua_State* L) {
    LevelObject* object = lua_touserdata(L, 1);
    if (object->level) {
        level_free(object->level);
        object->level = NULL;
    }

    return 0;
}

static int lua_level_newindex(lua_State* L) {
    lua_getfenv(L, 1);
    lua_pushvalue(L, 2);
    lua_pushvalue(L, 3);
    lua_settable(L, -3);

    return 0;
}

static int lua_callback_reference = -1;

static void lua_entity_callback(int x,
                                int y,
                                const char* id,
                                LDTK_Field* fields,
                                size_t len) {
    lua_State* L = global.state;
    lua_rawgeti(L, LUA_REGISTRYINDEX, lua_callback_reference);

    size_t params = 3;
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushstring(L, id);
    params += lua_entity_push_fields(L, fields, len);

    if (lua_pcall(L, params, 0, 0) != LUA_OK) {
        fprintf(stderr, "Error calling callback: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

static int lua_level_entities_load(lua_State* L) {
    LevelObject* object = lua_touserdata(L, 1);

    size_t length       = 0;
    const char* key     = luaL_checklstring(L, 2, &length);

    if (key != NULL && length > 0 && lua_isfunction(L, 3)) {
        lua_pushvalue(L, 3);
        lua_callback_reference = luaL_ref(L, LUA_REGISTRYINDEX);

        level_entities_load(object->level, key, lua_entity_callback);
        lua_callback_reference = -1;
    }

    return 0;
}

static int lua_level_spawn_entity(lua_State* L) {
    LevelObject* object = lua_touserdata(L, 1);
    lua_entity_create(object->level, L);
    return 1;
}

static int lua_level_load(lua_State* L) {
    size_t pathlen       = 0;
    size_t level_idlen   = 0;
    const char* path     = luaL_checklstring(L, 1, &pathlen);
    const char* level_id = luaL_checklstring(L, 2, &level_idlen);

    if (path != NULL && pathlen > 0 && level_id != NULL && level_idlen > 0) {
        Level* level = level_load(path, level_id);

        if (level != NULL) {
            LevelObject* object = lua_newuserdata(L, sizeof(*object));
            object->level       = level;
            luaL_getmetatable(L, META_TABLE);
            lua_setmetatable(L, -2);
            return 1;
        }
    }

    lua_pushnil(L);
    return 1;
}

void lua_level_register(lua_State* L) {
    luaL_newmetatable(L, META_TABLE);
    lua_pushcfunction(L, lua_level_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, lua_level_index);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, lua_level_newindex);
    lua_setfield(L, -2, "__newindex");

    lua_pushcfunction(L, lua_level_entities_load);
    lua_setfield(L, -2, "load_entities");

    lua_pushcfunction(L, lua_level_spawn_entity);
    lua_setfield(L, -2, "spawn_entity");

    lua_pop(L, 1);

    lua_register(L, "load_level", lua_level_load);
}
