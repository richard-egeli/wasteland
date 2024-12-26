#include "lua/world.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lauxlib.h"
#include "lua.h"
#include "lua/entity.h"
#include "lua/sprite.h"
#include "scene-graph/scene-graph.h"

SceneGraph* worlds[1024] = {0};
size_t worlds_count      = 0;

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
    {"create_entity", entity_create},
    {NULL, NULL},
};

void register_world_api(lua_State* L) {
    // World API
    luaL_newmetatable(L, "World");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, world_functions, 0);

    lua_newtable(L);
    lua_pushcfunction(L, create_world);
    lua_setfield(L, -2, "new");

    lua_setglobal(L, "World");
    lua_pop(L, 1);
}
