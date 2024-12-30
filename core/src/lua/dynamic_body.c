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

Movement dynamic_body_movements[1024];
size_t dynamic_body_movements_length;

int dynamic_body_move(lua_State* L) {
    assert(lua_isuserdata(L, 1) && "Userdata cannot be NULL");
    assert(lua_isnumber(L, 2) && "X coordinate invalid");
    assert(lua_isnumber(L, 3) && "Y coordinate invalid");

    DynamicBody* body = *(DynamicBody**)lua_touserdata(L, 1);
    float x           = lua_tonumber(L, 2);
    float y           = lua_tonumber(L, 3);

    b2WorldId id      = body->entity.weak_world_ptr->id;
    b2Body_SetLinearVelocity(body->id, (b2Vec2){x, y});
    dynamic_body_movements[dynamic_body_movements_length++] = (Movement){
        .body_id = body->id,
        .node    = body->entity.node,
    };

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

    DynamicBody* body = malloc(sizeof(*body));
    assert(body != NULL && "Body cannot be NULL!");

    body->id               = body_id;

    DynamicBody** body_ptr = lua_newuserdata(L, sizeof(*body_ptr));
    *body_ptr              = body;

    entity_setup(L, &body->entity, 2);

    Sprite* sprite = sprite_parse(L, body->entity.node, 2);
    if (sprite != NULL) {
        Drawable* draw = scene_graph_drawable_new(world->graph, body->entity.node);
        draw->data     = sprite;
        draw->draw     = draw_sprite;
    }
    entity_setup_update(L, &body->entity, 2);

    setup_metatable(L, "DynamicBody", 2, NULL, 0);
    lua_pushvalue(L, -1);
    body->entity.self_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    return 1;
}

static luaL_Reg dynamic_body_functions[] = {
    {"move", dynamic_body_move},
    {NULL, NULL},
};

void register_dynamic_body_api(lua_State* L) {
    luaL_newmetatable(L, "DynamicBody");
    luaL_setfuncs(L, dynamic_body_functions, 0);
    lua_pop(L, 1);
}
