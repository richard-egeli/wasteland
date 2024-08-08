#include "lua_funcs.h"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include "action.h"
#include "global.h"
#include "lua_entity.h"

static int lua_action_down(lua_State* L) {
    size_t length;
    const char* key = luaL_checklstring(L, 1, &length);
    lua_pushboolean(L, action_down(key));
    return 1;
}

void lua_register_functions(lua_State* L) {
    lua_register(L, "action_down", lua_action_down);
    lua_entity_register(L);
}
