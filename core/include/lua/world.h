#ifndef INCLUDE_LUA_WORLD_H_
#define INCLUDE_LUA_WORLD_H_

#include <stddef.h>

#include "box2d/id.h"

typedef struct SceneGraph SceneGraph;

typedef struct lua_State lua_State;

typedef struct World {
    b2WorldId id;
    SceneGraph* graph;
    lua_State* L;
} World;

extern World* worlds[1024];
extern size_t worlds_count;

void register_world_api(lua_State* L);

#endif  // INCLUDE_LUA_WORLD_H_
