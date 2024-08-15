#include "lua_action.h"

#include <lauxlib.h>

#include "action.h"

static int lua_action_down(lua_State* L) {
    size_t length;
    const char* key = luaL_checklstring(L, 1, &length);
    lua_pushboolean(L, action_down(key));
    return 1;
}

static int lua_action_up(lua_State* L) {
    size_t length;
    const char* key = luaL_checklstring(L, 1, &length);
    lua_pushboolean(L, action_up(key));
    return 1;
}

static int lua_action_pressed(lua_State* L) {
    size_t length;
    const char* key = luaL_checklstring(L, 1, &length);
    lua_pushboolean(L, action_pressed(key));
    return 1;
}

static int lua_action_released(lua_State* L) {
    size_t length;
    const char* key = luaL_checklstring(L, 1, &length);
    lua_pushboolean(L, action_released(key));
    return 1;
}

static int lua_action_setup(lua_State* L) {
    size_t length    = 0;
    const char* name = luaL_checklstring(L, 1, &length);
    int keycode      = luaL_checknumber(L, 2);
    action_register(name, keycode);

    printf("Registered: %s %c\n", name, keycode);
    return 0;
}

void lua_action_register(lua_State* L) {
    lua_register(L, "action_register", lua_action_setup);
    lua_register(L, "action_up", lua_action_up);
    lua_register(L, "action_down", lua_action_down);
    lua_register(L, "action_pressed", lua_action_pressed);
    lua_register(L, "action_released", lua_action_released);
}
