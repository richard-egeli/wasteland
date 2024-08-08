#ifndef CORE_INCLUDE_LUA_FUNCS_H_
#define CORE_INCLUDE_LUA_FUNCS_H_

typedef struct lua_State lua_State;

void lua_register_functions(lua_State* L);

#endif  // CORE_INCLUDE_LUA_FUNCS_H_
