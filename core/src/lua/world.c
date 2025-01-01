#include "lua/world.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "box2d/box2d.h"
#include "box2d/types.h"
#include "lauxlib.h"
#include "lua.h"
#include "lua/dynamic_body.h"
#include "lua/entity.h"
#include "lua/sprite.h"
#include "lua/static_body.h"
#include "scene-graph/scene-graph.h"

World* worlds[1024] = {0};
size_t worlds_count = 0;

static int create_world(lua_State* L) {
    World* world = malloc(sizeof(*world));
    assert(world != NULL && "World cannot be NULL!");
    SceneGraph* graph    = scene_graph_new();
    World** world_ptr    = lua_newuserdata(L, sizeof(World*));

    b2WorldDef world_def = b2DefaultWorldDef();
    world_def.gravity    = (b2Vec2){0, 0};
    b2WorldId id         = b2CreateWorld(&world_def);
    scene_graph_node_new(graph, NODE_NULL);

    world->id              = id;
    world->graph           = graph;
    world->L               = L;
    *world_ptr             = world;
    worlds[worlds_count++] = world;

    luaL_getmetatable(L, "World");
    lua_setmetatable(L, -2);

    // Create an empty table on the userdata named children
    lua_pushstring(L, "children");
    lua_newtable(L);
    lua_settable(L, -3);

    return 1;
}

// Index/newindex handlers
static int world_index(lua_State* L) {
    luaL_getmetatable(L, "World");
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1)) return 1;
    lua_pop(L, 2);

    lua_getfenv(L, 1);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    return 1;
}

static int world_newindex(lua_State* L) {
    lua_getfenv(L, 1);
    lua_pushvalue(L, 2);
    lua_pushvalue(L, 3);
    lua_rawset(L, -3);
    return 0;
}

static luaL_Reg world_functions[] = {
    {"__index", world_index},
    {"__newindex", world_newindex},
    {"new", create_world},
    {"create_entity", entity_create},
    {"create_dynamic_body", dynamic_body_create},
    {"create_static_body", static_body_create},
    {"create_sprite", sprite_create},
    {NULL, NULL},
};

void register_world_api(lua_State* L) {
    // World API
    luaL_newmetatable(L, "World");
    luaL_setfuncs(L, world_functions, 0);

    lua_newtable(L);
    lua_pushcfunction(L, create_world);
    lua_setfield(L, -2, "new");

    lua_setglobal(L, "World");
    lua_pop(L, 1);
}
