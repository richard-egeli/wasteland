#include "lua/utils.h"

#include <stddef.h>
#include <string.h>

#include "lauxlib.h"
#include "lua.h"

bool is_custom_property(const char* name, const char* filter[], size_t length) {
    for (int i = 0; i < length; i++) {
        if (strcmp(name, filter[i]) == 0) {
            return false;
        }
    }

    return true;
}

void setup_metatable(lua_State* L, const char* name, int idx, const char* filter[], size_t length) {
    // Create new metatable for this instance
    lua_createtable(L, 0, 2);

    // Set up inheritance from Entity metatable
    luaL_getmetatable(L, name);
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -4);

    // Copy custom properties
    lua_pushnil(L);
    while (lua_next(L, idx) != 0) {
        const char* name = lua_tostring(L, -2);
        if (is_custom_property(name, filter, length)) {
            lua_pushvalue(L, -2);  // key
            lua_pushvalue(L, -2);  // value
            lua_settable(L, -5);   // set in metatable
        }
        lua_pop(L, 1);
    }

    lua_pop(L, 1);  // Pop Entity metatable
    lua_setmetatable(L, -2);
}
