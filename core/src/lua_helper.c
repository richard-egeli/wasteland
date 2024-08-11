#include "lua_helper.h"

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

const char* lua_read_string(lua_State* L, const char* key) {
    size_t length;
    lua_getfield(L, -1, key);
    const char* str = lua_tolstring(L, -1, &length);
    lua_pop(L, 1);
    return str;
}

int lua_read_int(lua_State* L, const char* key) {
    lua_getfield(L, -1, key);
    int v = lua_tointeger(L, -1);
    lua_pop(L, 1);
    return v;
}

float lua_read_float(lua_State* L, const char* key) {
    lua_getfield(L, -1, key);
    int f = lua_tonumber(L, -1);
    lua_pop(L, 1);
    return f;
}

bool lua_read_bool(lua_State* L, const char* key) {
    lua_getfield(L, -1, key);
    bool b = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return b;
}
