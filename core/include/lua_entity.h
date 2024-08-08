#ifndef CORE_INCLUDE_LUA_ENTITY_H_
#define CORE_INCLUDE_LUA_ENTITY_H_

typedef struct lua_State lua_State;

void lua_entity_create_notify(int x, int y, const char* name);

void lua_entity_register(lua_State* L);

#endif  // CORE_INCLUDE_LUA_ENTITY_H_
