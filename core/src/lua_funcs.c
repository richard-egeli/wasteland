#include "lua_funcs.h"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <raylib.h>

#include "global.h"
#include "lua_action.h"
#include "lua_camera.h"
#include "lua_entity.h"
#include "lua_level.h"

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
    lua_register(L, "get_mouse_world_position", lua_get_mouse_world_positon);
    lua_register(L, "get_time", lua_get_time);
    lua_action_register(L);
    lua_camera_register(L);
    lua_entity_register(L);
    lua_level_register(L);
}
