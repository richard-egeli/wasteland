#ifndef CORE_INCLUDE_LUA_ENTITY_H_
#define CORE_INCLUDE_LUA_ENTITY_H_

#include "lua/animator.h"
#include "lua/box_collider.h"
#include "lua/dynamic_body.h"
#include "lua/sprite.h"
#include "lua/static_body.h"
typedef struct lua_State lua_State;

typedef struct GameObject GameObject;
typedef struct SceneGraph SceneGraph;
typedef struct World World;

typedef enum EntityType {
    ENTITY_TYPE_DYNAMIC_BODY,
    ENTITY_TYPE_STATIC_BODY,
    ENTITY_TYPE_BOX_COLLIDER,
    ENTITY_TYPE_ANIMATOR,
    ENTITY_TYPE_SPRITE,
} EntityType;

typedef struct Entity {
    int node;
    int self_ref;
    int update_ref;
    lua_State* L;
    World* weak_world_ptr;
    EntityType type;
    union {
        DynamicBody dynamic_body;
        StaticBody static_body;
        BoxCollider box_collider;
        Animator animator;
        Sprite sprite;
    };
} Entity;

int entity_set_position(lua_State* L);

int entity_get_position(lua_State* L);

void entity_parent_position(Entity* entity);

void entity_call_update(SceneGraph* graph, GameObject* object);

void entity_call_load(lua_State* L, Entity* entity, int idx);

void entity_setup_metatable(lua_State* L, Entity* entity, int idx);

void entity_setup_update(lua_State* L, Entity* entity, int idx);

void entity_setup(lua_State* L, Entity* entity, int idx);

int entity_get_parent_id(lua_State* L, int idx);

int entity_create(lua_State* L);

void register_entity_api(lua_State* L);

#endif  // CORE_INCLUDE_LUA_ENTITY_H_
