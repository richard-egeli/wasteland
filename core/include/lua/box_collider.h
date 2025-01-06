#ifndef CORE_INCLUDE_LUA_BOX_COLLIDER_H_
#define CORE_INCLUDE_LUA_BOX_COLLIDER_H_

#include "box2d/id.h"

typedef struct lua_State lua_State;

typedef struct BoxCollider {
    b2ShapeId shape_id;
    int on_collision_enter_ref;
    int on_collision_exit_ref;
} BoxCollider;

int box_collider_create(lua_State* L);

void register_box_collider_api(lua_State* L);

#endif  // CORE_INCLUDE_LUA_BOX_COLLIDER_H_
