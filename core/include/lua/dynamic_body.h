#ifndef CORE_INCLUDE_LUA_DYNAMIC_BODY_H_
#define CORE_INCLUDE_LUA_DYNAMIC_BODY_H_

#include <stddef.h>

#include "box2d/id.h"
#include "entity.h"
#include "scene-graph/scene-graph.h"

typedef struct World World;
typedef struct lua_State lua_State;

typedef struct DynamicBody {
    b2BodyId id;
    Entity entity;
} DynamicBody;

typedef struct Movement {
    Node node;
    b2BodyId body_id;
} Movement;

extern Movement dynamic_body_movements[1024];
extern size_t dynamic_body_movements_length;

int dynamic_body_create(lua_State* L);

void register_dynamic_body_api(lua_State* L);

#endif  // CORE_INCLUDE_LUA_DYNAMIC_BODY_H_
