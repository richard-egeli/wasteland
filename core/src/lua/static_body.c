#include "lua/static_body.h"

#include <assert.h>
#include <stdlib.h>

#include "box2d/box2d.h"
#include "box2d/collision.h"
#include "box2d/types.h"
#include "lauxlib.h"
#include "lua.h"
#include "lua/entity.h"
#include "lua/utils.h"
#include "lua/world.h"

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

    entity_setup(L, entity, 2);
    entity_setup_update(L, entity, 2);
    setup_metatable(L, "Entity", 2, NULL, 0);
    lua_pushvalue(L, -1);
    entity->self_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    return 1;
}

void register_static_body_api(lua_State* L) {
}
