#include "lua_funcs.h"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <raylib.h>

#include "action.h"
#include "global.h"
#include "lua_entity.h"

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

static int lua_get_mouse_world_positon(lua_State* L) {
    Vector2 pos = GetScreenToWorld2D(GetMousePosition(), global.camera);
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

static int lua_get_time(lua_State* L) {
    lua_pushnumber(L, GetTime());
    return 1;
}

void lua_register_functions(lua_State* L) {
    lua_register(L, "action_up", lua_action_up);
    lua_register(L, "action_down", lua_action_down);
    lua_register(L, "action_pressed", lua_action_pressed);
    lua_register(L, "action_released", lua_action_released);
    lua_register(L, "get_mouse_world_position", lua_get_mouse_world_positon);
    lua_register(L, "get_time", lua_get_time);
    lua_entity_register(L);
}
