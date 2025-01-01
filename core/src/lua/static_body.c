#include "lua/static_body.h"

#include <assert.h>
#include <stdlib.h>

#include "box2d/box2d.h"
#include "box2d/collision.h"
#include "box2d/math_functions.h"
#include "box2d/types.h"
#include "lauxlib.h"
#include "lua.h"
#include "lua/entity.h"
#include "lua/utils.h"
#include "lua/world.h"

int static_body_set_position(lua_State* L) {
    assert(lua_gettop(L) == 3 && "Wrong number of arguments (self,x,y)");
    assert(lua_isuserdata(L, 1) && "Userdata must be first");
    assert(lua_isnumber(L, 2) && "X position must be second");
    assert(lua_isnumber(L, 3) && "Y position must be third");

    Entity* entity = *(Entity**)lua_touserdata(L, 1);
    assert(entity->type == ENTITY_TYPE_STATIC_BODY && "Wrong user data!");
    float x = lua_tonumber(L, 2);
    float y = lua_tonumber(L, 3);

    b2Body_SetTransform(entity->static_body.id, (b2Vec2){x, y}, b2Rot_identity);
    return 0;
}

int static_body_get_position(lua_State* L) {
    assert(lua_gettop(L) == 1 && "Wrong number of arguments (self)");
    assert(lua_isuserdata(L, 1) && "Invalid userdata");

    Entity* entity = *(Entity**)lua_touserdata(L, 1);
    assert(entity->type == ENTITY_TYPE_STATIC_BODY && "Wrong type of userdata");

    b2Vec2 pos = b2Body_GetPosition(entity->static_body.id);
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

int static_body_create(lua_State* L) {
    assert(lua_isuserdata(L, 1) && "Userdata must be first");
    assert(lua_istable(L, 2) && "A valid table must be second");

    World* world = *(World**)lua_touserdata(L, 1);

    lua_getfield(L, 2, "x");
    float x = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "y");
    float y = lua_tonumber(L, -1);
    lua_pop(L, 1);

    b2BodyDef body_def     = b2DefaultBodyDef();
    body_def.type          = b2_staticBody;
    body_def.position      = (b2Vec2){x, y};
    body_def.fixedRotation = true;
    b2BodyId body_id       = b2CreateBody(world->id, &body_def);

    b2Polygon box          = b2MakeBox(16, 16);
    b2ShapeDef shape       = b2DefaultShapeDef();
    shape.isSensor         = true;

    b2CreatePolygonShape(body_id, &shape, &box);

    Entity* entity = malloc(sizeof(*entity));
    assert(entity != NULL && "Entity cannot be NULL");

    entity->static_body.id = body_id;
    entity->type           = ENTITY_TYPE_STATIC_BODY;
    b2Body_SetUserData(body_id, entity);

    lua_getfield(L, 2, "on_collision_enter");
    if (lua_isfunction(L, -1)) {
        entity->static_body.on_collision_enter_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
        lua_pop(L, 1);
    }

    lua_getfield(L, 2, "on_collision_exit");
    if (lua_isfunction(L, -1)) {
        entity->static_body.on_collision_exit_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
        lua_pop(L, 1);
    }

    Entity** entity_ptr = lua_newuserdata(L, sizeof(*entity_ptr));
    assert(entity_ptr != NULL && "Entity pointer cannot be NULL!");

    *entity_ptr = entity;
    entity_setup(L, entity, 2);
    entity_setup_update(L, entity, 2);
    setup_metatable(L, "StaticBody", 2, NULL, 0);
    lua_pushvalue(L, -1);
    entity->self_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    return 1;
}

// Index/newindex handlers
static int static_body_index(lua_State* L) {
    luaL_getmetatable(L, "StaticBody");
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1)) return 1;
    lua_pop(L, 2);

    lua_getfenv(L, 1);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    return 1;
}

static int static_body_newindex(lua_State* L) {
    lua_getfenv(L, 1);
    lua_pushvalue(L, 2);
    lua_pushvalue(L, 3);
    lua_rawset(L, -3);
    return 0;
}

static luaL_Reg static_body_functions[] = {
    {"__index", static_body_index},
    {"__newindex", static_body_newindex},
    {"set_position", static_body_set_position},
    {"get_position", static_body_get_position},
    {NULL, NULL},
};

void register_static_body_api(lua_State* L) {
    luaL_newmetatable(L, "StaticBody");
    luaL_setfuncs(L, static_body_functions, 0);
    lua_pop(L, 1);
}
