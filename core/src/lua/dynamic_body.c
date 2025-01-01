#include "lua/dynamic_body.h"

#include <assert.h>
#include <stdlib.h>

#include "box2d/box2d.h"
#include "box2d/types.h"
#include "lauxlib.h"
#include "lua.h"
#include "lua/entity.h"
#include "lua/sprite.h"
#include "lua/utils.h"
#include "lua/world.h"
#include "scene-graph/scene-graph.h"

int dynamic_body_set_position(lua_State* L) {
    assert(lua_gettop(L) == 3 && "Wrong number of arguments (self,x,y)");
    assert(lua_isuserdata(L, 1) && "Userdata must be first");
    assert(lua_isnumber(L, 2) && "X position must be second");
    assert(lua_isnumber(L, 3) && "Y position must be third");

    Entity* entity = *(Entity**)lua_touserdata(L, 1);
    assert(entity->type == ENTITY_TYPE_DYNAMIC_BODY && "Wrong user data!");
    float x = lua_tonumber(L, 2);
    float y = lua_tonumber(L, 3);

    b2Body_SetTransform(entity->dynamic_body.id, (b2Vec2){x, y}, b2Rot_identity);
    return 0;
}

int dynamic_body_get_position(lua_State* L) {
    assert(lua_gettop(L) == 1 && "Wrong number of arguments (self)");
    assert(lua_isuserdata(L, 1) && "Invalid userdata");

    Entity* entity = *(Entity**)lua_touserdata(L, 1);
    assert(entity->type == ENTITY_TYPE_DYNAMIC_BODY && "Wrong type of userdata");

    b2Vec2 pos = b2Body_GetPosition(entity->dynamic_body.id);
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

int dynamic_body_move(lua_State* L) {
    assert(lua_isuserdata(L, 1) && "Userdata cannot be NULL");
    assert(lua_isnumber(L, 2) && "X coordinate invalid");
    assert(lua_isnumber(L, 3) && "Y coordinate invalid");

    Entity* entity = *(Entity**)lua_touserdata(L, 1);
    assert(entity->type == ENTITY_TYPE_DYNAMIC_BODY && "Entity type is wrong");

    float x      = lua_tonumber(L, 2);
    float y      = lua_tonumber(L, 3);

    b2WorldId id = entity->weak_world_ptr->id;
    b2Body_SetLinearVelocity(entity->dynamic_body.id, (b2Vec2){x, y});
    return 0;
}

static void draw_sprite(SceneGraph* graph, Drawable* drawable) {
    Sprite* sprite    = drawable->data;
    Position position = scene_graph_position_get(graph, drawable->node);
    sprite_draw(sprite, position.x, position.y);
}

int dynamic_body_create(lua_State* L) {
    assert(lua_isuserdata(L, 1));

    World* world           = *(World**)lua_touserdata(L, 1);
    b2BodyDef body_def     = b2DefaultBodyDef();

    body_def.type          = b2_dynamicBody;
    body_def.fixedRotation = true;
    b2BodyId body_id       = b2CreateBody(world->id, &body_def);
    b2Polygon box          = b2MakeBox(16, 16);
    b2ShapeDef shape       = b2DefaultShapeDef();
    b2CreatePolygonShape(body_id, &shape, &box);

    Entity* entity = malloc(sizeof(*entity));
    assert(entity != NULL && "entity cannot be NULL!");

    entity->type                                = ENTITY_TYPE_DYNAMIC_BODY;
    entity->dynamic_body.id                     = body_id;
    entity->dynamic_body.on_collision_enter_ref = -1;
    entity->dynamic_body.on_collision_exit_ref  = -1;
    b2Body_SetUserData(body_id, entity);

    lua_getfield(L, 2, "on_collision_enter");
    if (lua_isfunction(L, -1)) {
        entity->dynamic_body.on_collision_enter_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
        lua_pop(L, 1);
    }

    lua_getfield(L, 2, "on_collision_exit");
    if (lua_isfunction(L, -1)) {
        entity->dynamic_body.on_collision_exit_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
        lua_pop(L, 1);
    }

    Entity** entity_ptr = lua_newuserdata(L, sizeof(*entity));
    *entity_ptr         = entity;

    entity_setup(L, entity, 2);
    entity_setup_update(L, entity, 2);

    setup_metatable(L, "DynamicBody", 2, NULL, 0);
    lua_pushvalue(L, -1);
    entity->self_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    return 1;
}

// Index/newindex handlers
static int dynamic_body_index(lua_State* L) {
    luaL_getmetatable(L, "DynamicBody");
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1)) return 1;
    lua_pop(L, 2);

    lua_getfenv(L, 1);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    return 1;
}

static int dynamic_body_newindex(lua_State* L) {
    lua_getfenv(L, 1);
    lua_pushvalue(L, 2);
    lua_pushvalue(L, 3);
    lua_rawset(L, -3);
    return 0;
}

static luaL_Reg dynamic_body_functions[] = {
    {"__index", dynamic_body_index},
    {"__newindex", dynamic_body_newindex},
    {"set_position", dynamic_body_set_position},
    {"get_position", dynamic_body_get_position},
    {"move", dynamic_body_move},
    {NULL, NULL},
};

void register_dynamic_body_api(lua_State* L) {
    luaL_newmetatable(L, "DynamicBody");
    luaL_setfuncs(L, dynamic_body_functions, 0);
    lua_pop(L, 1);
}
