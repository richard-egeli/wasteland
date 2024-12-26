#include "lua/input.h"

#include <assert.h>

#include "lauxlib.h"
#include "lua.h"
#include "raylib.h"

static int input_register_action(lua_State* L) {
    assert(lua_gettop(L) == 2 && "Wrong args (name, keys)");
    assert(lua_isstring(L, 1) && "First argument is wrong");
    assert(lua_istable(L, 2) && "Second argument is wrong");

    return 0;
}

static int input_is_released(lua_State* L) {
    assert(lua_gettop(L) == 1 && "Wrong args (key)");
    assert(lua_isnumber(L, 1) && "Wrong argument type (number)");
    lua_pushboolean(L, IsKeyReleased(lua_tointeger(L, 1)));
    return 1;
}

static int input_is_pressed(lua_State* L) {
    assert(lua_gettop(L) == 1 && "Wrong args (key)");
    assert(lua_isnumber(L, 1) && "Wrong argument type (number)");
    lua_pushboolean(L, IsKeyPressed(lua_tointeger(L, 1)));
    return 1;
}

static int input_is_down(lua_State* L) {
    assert(lua_gettop(L) == 1 && "Wrong args (key)");
    assert(lua_isnumber(L, 1) && "Wrong argument type (number)");
    lua_pushboolean(L, IsKeyDown(lua_tointeger(L, 1)));
    return 1;
}

static int input_is_up(lua_State* L) {
    assert(lua_gettop(L) == 1 && "Wrong args (key)");
    assert(lua_isnumber(L, 1) && "Wrong argument type (number)");
    lua_pushboolean(L, IsKeyUp(lua_tointeger(L, 1)));
    return 1;
}

static luaL_Reg input_functions[] = {
    {"register_action", input_register_action},
    {"is_released", input_is_released},
    {"is_pressed", input_is_pressed},
    {"is_down", input_is_down},
    {"is_up", input_is_up},
    {NULL, NULL},
};

void register_input_api(lua_State* L) {
    luaL_newmetatable(L, "Input");
    luaL_setfuncs(L, input_functions, 0);
    lua_setglobal(L, "Input");
}
