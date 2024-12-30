#ifndef CORE_INCLUDE_LUA_ENTITY_H_
#define CORE_INCLUDE_LUA_ENTITY_H_

typedef struct lua_State lua_State;

typedef struct GameObject GameObject;
typedef struct SceneGraph SceneGraph;
typedef struct World World;

typedef struct Entity {
    int node;
    int self_ref;
    int update_ref;
    lua_State* L;
    World* weak_world_ptr;
} Entity;

void entity_call_update(SceneGraph* graph, GameObject* object);

void entity_call_load(lua_State* L, Entity* entity, int idx);

void entity_setup_metatable(lua_State* L, Entity* entity, int idx);

void entity_setup_update(lua_State* L, Entity* entity, int idx);

void entity_setup(lua_State* L, Entity* entity, int idx);

int entity_get_parent_id(lua_State* L, int idx);

int entity_create(lua_State* L);

void register_entity_api(lua_State* L);

#endif  // CORE_INCLUDE_LUA_ENTITY_H_
