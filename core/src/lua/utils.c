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
    luaL_getmetatable(L, name);
    lua_setmetatable(L, -2);

    lua_newtable(L);
    lua_pushnil(L);
    while (lua_next(L, idx) != 0) {
        const char* name = lua_tostring(L, -2);
        if (is_custom_property(name, filter, length)) {
            lua_pushvalue(L, -2);  // key
            lua_pushvalue(L, -2);  // value
            lua_rawset(L, -5);     // set in env table
        }
        lua_pop(L, 1);
    }

    // Create an empty table on the userdata named children
    lua_pushstring(L, "children");
    lua_newtable(L);
    lua_settable(L, -3);

    lua_setfenv(L, -2);
}
