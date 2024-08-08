#ifndef CORE_INCLUDE_GLOBAL_H_
#define CORE_INCLUDE_GLOBAL_H_

#include <raylib.h>

typedef struct HashMap HashMap;
typedef struct lua_State lua_State;
typedef struct BoxCollider BoxCollider;
typedef struct Entity Entity;
typedef struct Level Level;

typedef struct Global {
    HashMap* worlds;
    HashMap* actions;
    Entity* player;
    lua_State* state;
    Camera2D camera;
    Level* level;
} Global;

extern Global global;

#endif  // CORE_INCLUDE_GLOBAL_H_
