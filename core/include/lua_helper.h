#ifndef CORE_INCLUDE_LUA_HELPER_H_
#define CORE_INCLUDE_LUA_HELPER_H_

#include <stdbool.h>
typedef struct lua_State lua_State;

const char* lua_read_string(lua_State* L, const char* key);

int lua_read_int(lua_State* L, const char* key);

float lua_read_float(lua_State* L, const char* key);

bool lua_read_bool(lua_State* L, const char* key);

#endif  // CORE_INCLUDE_LUA_HELPER_H_
