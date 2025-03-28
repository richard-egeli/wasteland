#ifndef CORE_INCLUDE_LUA_DYNAMIC_BODY_H_
#define CORE_INCLUDE_LUA_DYNAMIC_BODY_H_

#include <stddef.h>

#include "box2d/id.h"

typedef struct World World;
typedef struct lua_State lua_State;

typedef struct DynamicBody {
    b2BodyId id;
} DynamicBody;

int dynamic_body_create(lua_State* L);

void register_dynamic_body_api(lua_State* L);

#endif  // CORE_INCLUDE_LUA_DYNAMIC_BODY_H_
