#ifndef CORE_INCLUDE_LUA_STATIC_BODY_H_
#define CORE_INCLUDE_LUA_STATIC_BODY_H_

#include "box2d/id.h"

typedef struct lua_State lua_State;

typedef struct StaticBody {
    b2BodyId id;
    int on_collision_enter_ref;
    int on_collision_exit_ref;
} StaticBody;

int static_body_create(lua_State* L);

void register_static_body_api(lua_State* L);

#endif  // CORE_INCLUDE_LUA_STATIC_BODY_H_
