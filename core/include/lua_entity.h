#ifndef CORE_INCLUDE_LUA_ENTITY_H_
#define CORE_INCLUDE_LUA_ENTITY_H_

#include <stddef.h>

typedef struct lua_State lua_State;

typedef struct LDTK_Field LDTK_Field;

typedef struct Level Level;

size_t lua_entity_push_fields(lua_State* L, LDTK_Field* fields, size_t len);

void lua_entity_create(Level* level, lua_State* L);

void lua_entity_register(lua_State* L);

#endif  // CORE_INCLUDE_LUA_ENTITY_H_
