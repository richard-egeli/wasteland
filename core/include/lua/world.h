#ifndef INCLUDE_LUA_WORLD_H_
#define INCLUDE_LUA_WORLD_H_

#include <stddef.h>

typedef struct SceneGraph SceneGraph;

typedef struct lua_State lua_State;

extern SceneGraph* worlds[1024];
extern size_t worlds_count;

typedef struct Entity {
    int id;
    int self_ref;
    int update_ref;
    int render_ref;
    int parent;
    lua_State* L;
} Entity;

typedef struct World {
    SceneGraph* graph;
} World;

void register_world_api(lua_State* L);

#endif  // INCLUDE_LUA_WORLD_H_
