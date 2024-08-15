#include "lua_camera.h"

#include "global.h"
#include "lauxlib.h"

static int lua_camera_move(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    global.camera.target.x += x;
    global.camera.target.y += y;
    return 0;
}

void lua_camera_register(lua_State* L) {
    lua_register(L, "move_camera", lua_camera_move);
}
