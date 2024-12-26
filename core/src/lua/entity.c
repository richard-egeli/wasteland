#include "lua/entity.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "lauxlib.h"
#include "lua.h"
#include "lua/sprite.h"
#include "lua/world.h"
#include "scene-graph/scene-graph.h"

static const char* standard_entity_props[] = {
    "sprite",
    "update",
    "parent",
    "load",
};

static bool is_custom_property(const char* name) {
    size_t count = sizeof(standard_entity_props) / sizeof(*standard_entity_props);
    for (int i = 0; i < count; i++) {
        if (strcmp(name, standard_entity_props[i]) == 0) {
            return false;
        }
    }

    return true;
}

static void draw_sprite(SceneGraph* graph, Drawable* drawable) {
    Sprite* sprite    = drawable->data;
    Position position = scene_graph_position_get(graph, drawable->node);
    sprite_draw(sprite, position.x, position.y);
}

static int set_position(lua_State* L) {
    assert(lua_gettop(L) == 3 && "Invalid arguments (entity, x, y)");
    assert(lua_isuserdata(L, 1) && "Invalid entity argument");
    assert(lua_isnumber(L, 2) && "Invalid X argument");
    assert(lua_isnumber(L, 3) && "Invalid Y argument");

    Entity* entity = *(Entity**)lua_touserdata(L, 1);
    float x        = lua_tonumber(L, 2);
    float y        = lua_tonumber(L, 3);

    scene_graph_position_set(entity->weak_graph_ptr, entity->id, (Position){x, y});
    return 0;
}

static int get_position(lua_State* L) {
    assert(lua_gettop(L) == 1 && "Invalid arguments (entity)");
    assert(lua_isuserdata(L, 1) && "Invalid entity argument");

    Entity* entity    = *(Entity**)lua_touserdata(L, 1);
    Position position = scene_graph_position_get(entity->weak_graph_ptr, entity->id);

    lua_pushnumber(L, position.x);
    lua_pushnumber(L, position.y);
    return 2;
}

static void update_entity(SceneGraph* graph, GameObject* object) {
    Entity* entity = object->data;
    lua_State* L   = entity->L;

    // Get the update function
    lua_rawgeti(L, LUA_REGISTRYINDEX, entity->update_ref);
    if (!lua_isfunction(L, -1)) {
        fprintf(stderr, "Error: Lua reference does not point to a function\n");
        lua_pop(L, 1);
        return;
    }

    // Get the self table
    lua_rawgeti(L, LUA_REGISTRYINDEX, entity->self_ref);

    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        fprintf(stderr, "Error calling Lua function: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

static int get_parent_id(lua_State* L, int idx) {
    lua_getfield(L, idx, "parent");
    int parent_id = NODE_NULL;

    if (!lua_isnil(L, -1) && lua_isuserdata(L, -1)) {
        Entity** parent_ptr = lua_touserdata(L, -1);
        if (parent_ptr && *parent_ptr) {
            parent_id = (*parent_ptr)->id;
        }
    }
    lua_pop(L, 1);
    return parent_id;
}

static void setup_update_function(lua_State* L, SceneGraph* graph, Entity* entity, int idx) {
    lua_getfield(L, idx, "update");
    if (lua_isfunction(L, -1)) {
        entity->update_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        GameObject* object = scene_graph_game_object_new(graph, entity->id);
        object->update     = update_entity;
        object->data       = entity;
    } else {
        lua_pop(L, 1);
    }
}

static void setup_entity_metatable(lua_State* L, Entity* entity, int props_table_idx) {
    // Create new metatable for this instance
    lua_createtable(L, 0, 2);

    // Set up inheritance from Entity metatable
    luaL_getmetatable(L, "Entity");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -4);

    // Copy custom properties
    lua_pushnil(L);
    while (lua_next(L, props_table_idx) != 0) {
        const char* name = lua_tostring(L, -2);
        if (is_custom_property(name)) {
            lua_pushvalue(L, -2);  // key
            lua_pushvalue(L, -2);  // value
            lua_settable(L, -5);   // set in metatable
        }
        lua_pop(L, 1);
    }

    lua_pop(L, 1);  // Pop Entity metatable
    lua_setmetatable(L, -2);
}

static void call_load_function(lua_State* L, Entity* entity, int world_idx) {
    lua_getfield(L, 2, "load");
    if (lua_isfunction(L, -1)) {
        lua_pushvalue(L, world_idx);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            fprintf(stderr, "Error calling Lua function: %s\n", lua_tostring(entity->L, -1));
            lua_pop(entity->L, 1);
        }
    }
}

int entity_create(lua_State* L) {
    // Parameter validation
    assert(lua_gettop(L) == 2 && "Parameters (world, entity) invalid");
    assert(lua_isuserdata(L, 1) && "First parameter not a valid entity");
    assert(lua_istable(L, 2) && "Second parameter not a valid table");

    // Get scene graph and create entity
    SceneGraph* graph = *(SceneGraph**)lua_touserdata(L, 1);
    Entity* entity    = malloc(sizeof(Entity));
    assert(entity != NULL && "Entity cannot be NULL!");

    // Initialize entity
    entity->L              = L;
    entity->update_ref     = LUA_NOREF;
    entity->weak_graph_ptr = graph;

    // Create node in scene graph
    int parent_id = get_parent_id(L, 2);
    entity->id    = scene_graph_node_new(graph, parent_id);
    if (entity->id < 0) {
        free(entity);
        return luaL_error(L, "Failed to create scene graph node");
    }

    // Setup components
    setup_update_function(L, graph, entity, 2);

    Sprite* sprite = sprite_parse(L, entity->id, 2);
    if (sprite != NULL) {
        Drawable* draw = scene_graph_drawable_new(graph, entity->id);
        draw->data     = sprite;
        draw->draw     = draw_sprite;
    }

    call_load_function(L, entity, 1);

    // Create and setup userdata
    Entity** entity_ptr = lua_newuserdata(L, sizeof(Entity*));
    *entity_ptr         = entity;

    setup_entity_metatable(L, entity, 2);

    // Store reference to the fully setup userdata
    lua_pushvalue(L, -1);
    entity->self_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    return 1;
}

static luaL_Reg entity_functions[] = {
    {"set_position", set_position},
    {"get_position", get_position},
    {NULL, NULL},
};

static int entity_index(lua_State* L) {
    // Check if it's a method first
    luaL_getmetatable(L, "Entity");
    lua_pushvalue(L, 2);  // Push the key
    lua_rawget(L, -2);    // Look up in metatable
    if (!lua_isnil(L, -1)) {
        return 1;
    }
    lua_pop(L, 2);  // Pop nil and metatable

    lua_pushvalue(L, 2);
    lua_gettable(L, 1);

    return 1;
}

void register_entity_api(lua_State* L) {
    luaL_newmetatable(L, "Entity");
    luaL_setfuncs(L, entity_functions, 0);
    lua_pushcfunction(L, entity_index);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}
