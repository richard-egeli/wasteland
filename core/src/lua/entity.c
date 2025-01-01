#include "lua/entity.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "lauxlib.h"
#include "lua.h"
#include "lua/sprite.h"
#include "lua/utils.h"
#include "lua/world.h"
#include "scene-graph/scene-graph.h"

static const char* standard_props[] = {
    "sprite",
    "update",
    "parent",
    "load",
};

static const size_t standard_props_length = sizeof(standard_props) / sizeof(*standard_props);

static void draw_sprite(SceneGraph* graph, Drawable* drawable) {
    Sprite* sprite    = drawable->data;
    Position position = scene_graph_position_get(graph, drawable->node);
    sprite_draw(sprite, position.x, position.y);
}

int entity_set_position(lua_State* L) {
    assert(lua_gettop(L) == 3 && "Invalid arguments (entity, x, y)");
    assert(lua_isuserdata(L, 1) && "Invalid entity argument");
    assert(lua_isnumber(L, 2) && "Invalid X argument");
    assert(lua_isnumber(L, 3) && "Invalid Y argument");

    Entity* entity    = *(Entity**)lua_touserdata(L, 1);
    SceneGraph* graph = entity->weak_world_ptr->graph;
    float x           = lua_tonumber(L, 2);
    float y           = lua_tonumber(L, 3);

    scene_graph_position_set(graph, entity->node, (Position){x, y});
    return 0;
}

int entity_get_position(lua_State* L) {
    assert(lua_gettop(L) == 1 && "Invalid arguments (entity)");
    assert(lua_isuserdata(L, 1) && "Invalid entity argument");

    Entity* entity    = *(Entity**)lua_touserdata(L, 1);
    SceneGraph* graph = entity->weak_world_ptr->graph;
    Position position = scene_graph_position_get(graph, entity->node);

    lua_pushnumber(L, position.x);
    lua_pushnumber(L, position.y);
    return 2;
}

void entity_call_update(SceneGraph* graph, GameObject* object) {
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

int entity_get_parent_id(lua_State* L, int idx) {
    lua_getfield(L, idx, "parent");
    int parent_id = NODE_ROOT;

    if (!lua_isnil(L, -1) && lua_isuserdata(L, -1)) {
        Entity** parent_ptr = lua_touserdata(L, -1);
        if (parent_ptr && *parent_ptr) {
            parent_id = (*parent_ptr)->node;
        }
    }

    lua_pop(L, 1);
    return parent_id;
}

void entity_setup_update(lua_State* L, Entity* entity, int idx) {
    lua_getfield(L, idx, "update");
    if (lua_isfunction(L, -1)) {
        SceneGraph* graph  = entity->weak_world_ptr->graph;
        entity->update_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        GameObject* object = scene_graph_game_object_new(graph, entity->node);
        object->update     = entity_call_update;
        object->data       = entity;
    } else {
        lua_pop(L, 1);
    }
}

void entity_call_load(lua_State* L, Entity* entity, int world_idx) {
    lua_getfield(L, 2, "load");
    if (lua_isfunction(L, -1)) {
        lua_pushvalue(L, world_idx);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            fprintf(stderr, "Error calling Lua function: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
}

void entity_setup(lua_State* L, Entity* entity, int idx) {
    assert(entity != NULL && "Entity cannot be NULL!");
    World* world      = *(World**)lua_touserdata(L, 1);
    SceneGraph* graph = world->graph;

    // Initialize entity
    entity->L              = L;
    entity->update_ref     = LUA_NOREF;
    entity->self_ref       = LUA_NOREF;
    entity->weak_world_ptr = world;

    // Create node in scene graph
    int parent_id = entity_get_parent_id(L, 2);
    entity->node  = scene_graph_node_new(graph, parent_id);
    assert(entity->node >= 0 && "Node cannot be INVALID");
}

int entity_create(lua_State* L) {
    // Parameter validation
    assert(lua_gettop(L) == 2 && "Parameters (world, entity) invalid");
    assert(lua_isuserdata(L, 1) && "First parameter not a valid entity");
    assert(lua_istable(L, 2) && "Second parameter not a valid table");

    // // Get scene graph and create entity
    World* world      = *(World**)lua_touserdata(L, 1);
    SceneGraph* graph = world->graph;
    Entity* entity    = malloc(sizeof(Entity));
    assert(entity != NULL && "Entity cannot be NULL!");

    entity_setup(L, entity, 2);

    Sprite* sprite = sprite_parse(L, entity->node, 2);
    if (sprite != NULL) {
        Drawable* draw = scene_graph_drawable_new(graph, entity->node);
        draw->data     = sprite;
        draw->draw     = draw_sprite;
    }

    // Create and setup userdata
    Entity** entity_ptr = lua_newuserdata(L, sizeof(Entity*));
    *entity_ptr         = entity;

    entity_setup_update(L, entity, 2);
    // entity_call_load(L, entity, 2);
    setup_metatable(L, "Entity", 2, standard_props, standard_props_length);

    // Store reference to the fully setup userdata
    lua_pushvalue(L, -1);
    entity->self_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    return 1;
}

static luaL_Reg entity_functions[] = {
    {"set_position", entity_set_position},
    {"get_position", entity_get_position},
    {NULL, NULL},
};

void register_entity_api(lua_State* L) {
    luaL_newmetatable(L, "Entity");
    luaL_setfuncs(L, entity_functions, 0);
    lua_pop(L, 1);
}
