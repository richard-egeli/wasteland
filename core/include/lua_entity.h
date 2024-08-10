#ifndef CORE_INCLUDE_LUA_ENTITY_H_
#define CORE_INCLUDE_LUA_ENTITY_H_

#include <stddef.h>

typedef struct lua_State lua_State;

typedef struct LDTK_Field LDTK_Field;

void lua_entity_create_notify(int x,
                              int y,
                              const char* name,
                              LDTK_Field* fields,
                              size_t len);

void lua_entity_register(lua_State* L);

#endif  // CORE_INCLUDE_LUA_ENTITY_H_
