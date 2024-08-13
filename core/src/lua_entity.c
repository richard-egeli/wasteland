#include "lua_entity.h"

#include <assert.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array/array.h"
#include "collision/box_collider.h"
#include "collision/sparse_grid.h"
#include "entity.h"
#include "global.h"
#include "ldtk/ldtk.h"
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

#define GET_BOOL(L, key)                   \
    ({                                     \
        lua_getfield(L, -1, key);          \
        bool value = lua_toboolean(L, -1); \
        lua_pop(L, 1);                     \
        value;                             \
    })

typedef struct EntityObject {
    Entity* entity;
    Level* level;
} EntityObject;

static int lua_entity_move(lua_State* L) {
    EntityObject* object = lua_touserdata(L, 1);
    Entity* entity       = object->entity;
    Level* level         = object->level;

    if (entity != NULL) {
        float x = luaL_checknumber(L, 2);
        float y = luaL_checknumber(L, 3);

        if (entity->collider) {
            spgrid_collider_move(level->sparse_grid, entity->collider, x, y);
        } else {
            entity->position.x += x;
            entity->position.y += y;
        }
    }

    return 0;
}

static int lua_entity_set_position(lua_State* L) {
    EntityObject* object = lua_touserdata(L, 1);
    Entity* entity       = object->entity;
    Level* level         = object->level;
    if (entity != NULL) {
        float x = luaL_checknumber(L, 2);
        float y = luaL_checknumber(L, 3);

        if (entity->collider) {
            spgrid_collider_set_position(level->sparse_grid, entity->collider, (int)x, (int)y);
        } else {
            entity->position.x = x;
            entity->position.y = y;
        }
    }

    return 0;
}

static int lua_entity_get_position(lua_State* L) {
    EntityObject* object = lua_touserdata(L, 1);
    Entity* entity       = object->entity;

    if (entity != NULL) {
        lua_pushnumber(L, entity->position.x);
        lua_pushnumber(L, entity->position.y);
    }

    return 2;
}

static int lua_entity_get_id(lua_State* L) {
    EntityObject* object = lua_touserdata(L, 1);
    lua_pushlightuserdata(L, object->entity);
    return 1;
}

static int lua_entity_mouse_direction(lua_State* L) {
    EntityObject* object = lua_touserdata(L, 1);
    Entity* entity       = object->entity;

    if (entity != NULL) {
        Vector2 mouse_pos = GetMousePosition();
        mouse_pos.x /= 2;
        mouse_pos.y /= 2;

        float x  = entity->position.x - mouse_pos.x;
        float y  = entity->position.y - mouse_pos.y;
        float m  = sqrtf(x * x + y * y);
        float nx = x / m;
        float ny = y / m;
        lua_pushnumber(L, nx);
        lua_pushnumber(L, ny);
    } else {
        lua_pushnumber(L, 0);
        lua_pushnumber(L, 0);
    }
    return 2;
}

static void lua_push_int(LDTK_Field field, void* udata) {
    lua_State* L = udata;
    lua_pushstring(L, field.__identifier);
    lua_pushinteger(L, field.value.int32);
}

static void lua_push_float(LDTK_Field field, void* udata) {
    lua_State* L = udata;
    lua_pushstring(L, field.__identifier);
    lua_pushnumber(L, field.value.float32);
}

static void lua_push_boolean(LDTK_Field field, void* udata) {
    lua_State* L = udata;
    lua_pushstring(L, field.__identifier);
    lua_pushboolean(L, field.value.boolean);
}

static void lua_push_string(LDTK_Field field, void* udata) {
    lua_State* L = udata;
    lua_pushstring(L, field.__identifier);
    lua_pushstring(L, field.value.string);
}

static void lua_push_point(LDTK_Field field, void* udata) {
    lua_State* L = udata;
    lua_pushstring(L, field.__identifier);
    lua_newtable(L);
    lua_pushstring(L, "x");
    lua_pushnumber(L, field.value.point.x);
    lua_settable(L, -3);

    lua_pushstring(L, "y");
    lua_pushnumber(L, field.value.point.y);
    lua_settable(L, -3);
}

static void lua_push_int_array(LDTK_Field field, void* udata) {
    lua_State* L = udata;
    lua_pushstring(L, field.__identifier);
    lua_newtable(L);
    for (int i = 0; i < field.value.int32_array.length; i++) {
        lua_pushinteger(L, field.value.int32_array.ptr[i]);
        lua_rawseti(L, -2, i + 1);
    }
}

static void lua_push_string_array(LDTK_Field field, void* udata) {
    lua_State* L = udata;
    lua_pushstring(L, field.__identifier);
    lua_newtable(L);
    for (int i = 0; i < field.value.string_array.length; i++) {
        lua_pushstring(L, field.value.string_array.ptr[i]);
        lua_rawseti(L, -2, i + 1);
    }
}

static void lua_push_point_array(LDTK_Field field, void* udata) {
    lua_State* L = udata;
    lua_pushstring(L, field.__identifier);
    lua_newtable(L);
    for (int i = 0; i < field.value.point_array.length; i++) {
        lua_newtable(L);
        lua_pushstring(L, "x");
        lua_pushnumber(L, field.value.point_array.ptr[i].x);
        lua_settable(L, -3);

        lua_pushstring(L, "y");
        lua_pushnumber(L, field.value.point_array.ptr[i].y);
        lua_settable(L, -3);
        lua_rawseti(L, -2, i + 1);
    }
}

static void lua_push_float_array(LDTK_Field field, void* udata) {
    lua_State* L = udata;
    lua_pushstring(L, field.__identifier);
    lua_newtable(L);
    for (int i = 0; i < field.value.float32_array.length; i++) {
        lua_pushnumber(L, field.value.float32_array.ptr[i]);
        lua_rawseti(L, -2, i + 1);
    }
}

static void lua_push_boolean_array(LDTK_Field field, void* udata) {
    lua_State* L = udata;
    lua_pushstring(L, field.__identifier);
    lua_newtable(L);
    for (int i = 0; i < field.value.boolean_array.length; i++) {
        lua_pushboolean(L, field.value.boolean_array.ptr[i]);
        lua_rawseti(L, -2, i + 1);
    }
}

size_t lua_entity_push_fields(lua_State* L, LDTK_Field* fields, size_t len) {
    size_t count            = 0;
    LDTK_FieldParser parser = {
        .userdata      = L,
        .int32         = lua_push_int,
        .float32       = lua_push_float,
        .boolean       = lua_push_boolean,
        .point         = lua_push_point,
        .string        = lua_push_string,
        .color         = lua_push_string,
        .int32_array   = lua_push_int_array,
        .float32_array = lua_push_float_array,
        .boolean_array = lua_push_boolean_array,
        .point_array   = lua_push_point_array,
        .string_array  = lua_push_string_array,
        .color_array   = lua_push_string_array,
    };

    for (int i = 0; i < len; i++) {
        if (ldtk_field_parse(&parser, fields[i])) {
            count += 2;
        }
    }

    return count;
}

static void lua_collision_notify(BoxCollider* b1, BoxCollider* b2) {
    lua_getglobal(global.state, "collision");
    if (lua_isfunction(global.state, -1)) {
        lua_pushlightuserdata(global.state, (Entity*)b1->id);
        lua_pushlightuserdata(global.state, (Entity*)b2->id);
        if (lua_pcall(global.state, 2, 0, 0) != LUA_OK) {
            fprintf(stderr, "Error: %s\n", lua_tostring(global.state, -1));
        }
    } else {
        lua_pop(global.state, -1);
    }
}

static int lua_entity_gc(lua_State* L) {
    EntityObject* object = lua_touserdata(L, 1);

    if (object && object->entity) {
        Entity* entity = object->entity;
        Level* level   = object->level;
        if (entity->collider) {
            spgrid_remove(level->sparse_grid, entity->collider);
        }

        int idx = array_find(level->entities, entity);
        if (idx >= 0) {
            size_t last_idx = array_length(level->entities) - 1;
            void* last      = array_get(level->entities, last_idx);
            array_set(level->entities, idx, last);
            array_pop(level->entities);
        }

        entity_free(entity);
        object->entity = NULL;
    }

    return 0;
}

static int lua_entity_destroy(lua_State* L) {
    EntityObject* object = lua_touserdata(L, 1);

    if (object && object->entity) {
        Entity* entity    = object->entity;
        Level* level      = object->level;
        entity->destroyed = true;

        if (entity->collider) {
            spgrid_remove(level->sparse_grid, entity->collider);
            entity->collider = 0;
        }
    }

    return 0;
}

void lua_entity_create(Level* level, lua_State* L) {
    EntityObject* object = lua_newuserdata(L, sizeof(*object));
    Entity* entity       = calloc(1, sizeof(Entity));

    object->level        = level;
    object->entity       = entity;
    entity->position.x   = luaL_checknumber(L, 2);
    entity->position.y   = luaL_checknumber(L, 3);
    entity->collider     = 0;

    lua_newtable(L);     // Create a new table for this entity's environment
    lua_setfenv(L, -2);  // Set this table as the environment for the userdata

    lua_getfield(L, 4, "sprite");
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

    lua_getfield(L, 4, "box_collider");
    if (!lua_isnil(L, -1)) {
        int x = 0;
        int y = 0;
        int w = 0;
        int h = 0;

        lua_getfield(L, -1, "position");
        if (!lua_isnil(L, -1)) {
            x += GET_INT(L, "x");
            y += GET_INT(L, "y");
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "size");
        if (!lua_isnil(L, -1)) {
            w += GET_INT(L, "x");
            h += GET_INT(L, "y");
        }
        lua_pop(L, 1);

        x += entity->position.x;
        y += entity->position.y;
        BoxCollider* collider  = box_collider_new(x, y, w, h);
        collider               = box_collider_new(x, y, w, h);
        collider->type         = GET_INT(L, "type");
        collider->mask         = GET_INT(L, "mask");
        collider->debug        = GET_BOOL(L, "debug");
        collider->trigger      = GET_BOOL(L, "trigger");
        collider->id           = (uint64_t)entity;
        collider->on_collision = lua_collision_notify;

        lua_getfield(L, -1, "origin");
        if (!lua_isnil(L, -1)) {
            collider->origin.x = GET_INT(L, "x");
            collider->origin.y = GET_INT(L, "y");
        }

        entity->collider = spgrid_insert(level->sparse_grid, collider);

        if (collider->mask == 6) {
            printf("ID OF PLAYER %llu\n", entity->collider);
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    array_push(level->entities, entity);
    luaL_getmetatable(L, META_TABLE);
    if (!lua_istable(L, -1)) {
        luaL_error(L, "Metatable not found %s", META_TABLE);
    }
    lua_setmetatable(L, -2);
}

static int lua_entity_index(lua_State* L) {
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

static int lua_entity_newindex(lua_State* L) {
    lua_getfenv(L, 1);
    lua_pushvalue(L, 2);
    lua_pushvalue(L, 3);
    lua_settable(L, -3);

    return 0;
}

void lua_entity_register(lua_State* L) {
    luaL_newmetatable(L, META_TABLE);

    lua_pushcfunction(L, lua_entity_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, lua_entity_index);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, lua_entity_newindex);
    lua_setfield(L, -2, "__newindex");

    lua_pushcfunction(L, lua_entity_get_id);
    lua_setfield(L, -2, "get_id");

    lua_pushcfunction(L, lua_entity_move);
    lua_setfield(L, -2, "move");

    lua_pushcfunction(L, lua_entity_get_position);
    lua_setfield(L, -2, "get_position");

    lua_pushcfunction(L, lua_entity_set_position);
    lua_setfield(L, -2, "set_position");

    lua_pushcfunction(L, lua_entity_mouse_direction);
    lua_setfield(L, -2, "get_mouse_direction");

    lua_pushcfunction(L, lua_entity_destroy);
    lua_setfield(L, -2, "destroy");

    lua_pop(L, 1);
}
