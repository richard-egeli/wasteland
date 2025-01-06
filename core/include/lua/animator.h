#ifndef CORE_INCLUDE_LUA_ANIMATOR_H_
#define CORE_INCLUDE_LUA_ANIMATOR_H_

#include <stdbool.h>

typedef struct Sprite Sprite;

typedef struct lua_State lua_State;

typedef struct Animation Animation;

typedef struct Entity Entity;

typedef struct Animator {
    Entity* entity;
    Animation* animation;
    int index;
    bool active;
    float time_multiplier;
    float time;
} Animator;

int animator_create(lua_State* L);

void register_animator_api(lua_State* L);

#endif  // CORE_INCLUDE_LUA_ANIMATOR_H_
