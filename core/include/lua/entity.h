#ifndef CORE_INCLUDE_LUA_ENTITY_H_
#define CORE_INCLUDE_LUA_ENTITY_H_

typedef struct lua_State lua_State;

typedef struct SceneGraph SceneGraph;

typedef struct Entity {
    int id;
    int self_ref;
    int update_ref;
    int parent;
    lua_State* L;
    SceneGraph* weak_graph_ptr;
} Entity;

int entity_create(lua_State* L);

void register_entity_api(lua_State* L);

#endif  // CORE_INCLUDE_LUA_ENTITY_H_
