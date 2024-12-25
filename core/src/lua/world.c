#include "lua/world.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "lauxlib.h"
#include "lua.h"
#include "scene-graph/scene-graph.h"

SceneGraph* worlds[1024] = {0};
size_t worlds_count      = 0;

static void draw_entity(SceneGraph* graph, Drawable* drawable) {
}

static void update_entity(SceneGraph* graph, GameObject* object) {
    Entity* entity = object->data;
    lua_rawgeti(entity->L, LUA_REGISTRYINDEX, entity->update_ref);

    // Check if the item on the top of the stack is a function
    if (!lua_isfunction(entity->L, -1)) {
        fprintf(stderr, "Error: Lua reference does not point to a function\n");
        lua_pop(entity->L, 1);  // Pop the non-function value off the stack
        return;
    }

    // push the entity on the stack
    lua_rawgeti(entity->L, LUA_REGISTRYINDEX, entity->self_ref);
    // Call the function with 0 arguments and expect 0 return values
    if (lua_pcall(entity->L, 1, 0, 0) != LUA_OK) {
        fprintf(stderr, "Error calling Lua function: %s\n", lua_tostring(entity->L, -1));
        lua_pop(entity->L, 1);  // Pop the error message off the stack
    }
}

static int create_entity(lua_State* L) {
    assert(lua_gettop(L) == 2 && "Parameters (world, entity) invalid");
    assert(lua_isuserdata(L, 1) && "First parameter not a valid entity");
    assert(lua_istable(L, 2) && "Second parameter not a valid table");

    SceneGraph* graph = *(SceneGraph**)lua_touserdata(L, 1);
    Entity* entity    = malloc(sizeof(Entity));
    assert(entity != NULL && "Entity cannot be NULL!");

    entity->L          = L;
    entity->update_ref = LUA_NOREF;
    entity->render_ref = LUA_NOREF;
    entity->self_ref   = LUA_NOREF;
    int parent_id      = NODE_NULL;

    lua_pushvalue(L, 1);
    entity->self_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_getfield(L, 2, "parent");
    if (!lua_isnil(L, -1)) {
        if (lua_isuserdata(L, -1)) {
            Entity** parent_ptr = lua_touserdata(L, -1);
            if (parent_ptr && *parent_ptr) {
                parent_id = (*parent_ptr)->id;
            }
        }

        lua_pop(L, 1);
    } else {
        lua_pop(L, 1);
    }

    // Create node in scene graph with specified parent
    entity->id = scene_graph_node_new(graph, parent_id);
    if (entity->id < 0) {
        free(entity);
        return luaL_error(L, "Failed to create scene graph node");
    }

    // Get update function if it exists
    lua_getfield(L, 2, "update");
    if (lua_isfunction(L, -1)) {
        entity->update_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        GameObject* object = scene_graph_game_object_new(graph, entity->id);
        object->update     = update_entity;
        object->data       = entity;
    } else {
        lua_pop(L, 1);
    }

    // Get render function if it exists
    lua_getfield(L, 2, "render");
    if (lua_isfunction(L, -1)) {
        entity->render_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        Drawable* draw     = scene_graph_drawable_new(graph, entity->id);
        draw->draw         = draw_entity;
        draw->data         = entity;
    } else {
        lua_pop(L, 1);
    }

    lua_getfield(L, 2, "load");
    if (lua_isfunction(L, -1)) {
        lua_pushvalue(L, 1);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            fprintf(stderr, "Error calling Lua function: %s\n", lua_tostring(entity->L, -1));
            lua_pop(entity->L, 1);
        }
    }

    Entity** entity_ptr = lua_newuserdata(L, sizeof(Entity*));
    *entity_ptr         = entity;
    return 1;
}

static int create_world(lua_State* L) {
    SceneGraph* graph      = scene_graph_new();
    SceneGraph** graph_ptr = lua_newuserdata(L, sizeof(SceneGraph*));
    *graph_ptr             = graph;
    worlds[worlds_count++] = graph;

    luaL_getmetatable(L, "World");
    lua_setmetatable(L, -2);

    return 1;
}

static luaL_Reg world_functions[] = {
    {"new", create_world},
    {"create_entity", create_entity},
    {NULL, NULL},
};

void register_world_api(lua_State* L) {
    luaL_newmetatable(L, "World");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, world_functions, 0);

    lua_newtable(L);
    lua_pushcfunction(L, create_world);
    lua_setfield(L, -2, "new");

    lua_setglobal(L, "World");
}
