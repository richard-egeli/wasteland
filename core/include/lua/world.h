#ifndef INCLUDE_LUA_WORLD_H_
#define INCLUDE_LUA_WORLD_H_

#include <stddef.h>

typedef struct SceneGraph SceneGraph;

typedef struct lua_State lua_State;

extern SceneGraph* worlds[1024];
extern size_t worlds_count;

void register_world_api(lua_State* L);

#endif  // INCLUDE_LUA_WORLD_H_
