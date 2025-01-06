#include "lua/box_collider.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "box2d/box2d.h"
#include "box2d/collision.h"
#include "box2d/id.h"
#include "box2d/types.h"
#include "lauxlib.h"
#include "lua.h"
#include "lua/entity.h"
#include "lua/utils.h"
#include "lua/world.h"
#include "scene-graph/scene-graph.h"

static b2Polygon box_collider_make(float x, float y, float width, float height) {
    assert(width > 0.0f && "Width must be greater than 0");
    assert(height > 0.0f && "Height must be greater than 0");

    b2Polygon shape   = {0};
    shape.count       = 4;
    shape.vertices[0] = (b2Vec2){x, y};
    shape.vertices[1] = (b2Vec2){x + width, y};
    shape.vertices[2] = (b2Vec2){x + width, y + height};
    shape.vertices[3] = (b2Vec2){x, y + height};
    shape.normals[0]  = (b2Vec2){0.0f, -1.0f};
    shape.normals[1]  = (b2Vec2){1.0f, 0.0f};
    shape.normals[2]  = (b2Vec2){0.0f, 1.0f};
    shape.normals[3]  = (b2Vec2){-1.0f, 0.0f};
    shape.radius      = 0.0f;
    shape.centroid    = b2Vec2_zero;
    return shape;
}

static bool box_collider_parent_body_id(World* world, Entity* entity, b2BodyId* id) {
    Node parent = entity->node;

    while (parent != NODE_NULL) {
        Entity* ent = scene_graph_userdata_get(world->graph, parent);

        switch (ent ? ent->type : -1) {
            case ENTITY_TYPE_STATIC_BODY:
                *id = ent->static_body.id;
                return true;
            case ENTITY_TYPE_DYNAMIC_BODY:
                *id = ent->dynamic_body.id;
                return true;
            default:
                parent = scene_graph_parent_get(world->graph, parent);
                continue;
        }
    }

    return false;
}

int box_collider_create(lua_State* L) {
    assert(lua_gettop(L) == 2 && "Invalid arguments (world, def)");
    assert(lua_isuserdata(L, 1) && "First argument must be userdata");
    assert(lua_istable(L, 2) && "Second argument must be a table");

    World* world = *(World**)lua_touserdata(L, 1);
    assert(world != NULL && "World cannot be NULL!");

    lua_getfield(L, 2, "parent");
    assert(lua_isuserdata(L, -1) && "Parent must be defined as valid userdata");
    Entity* parent = *(Entity**)lua_touserdata(L, -1);
    assert(parent != NULL && "Parent cannot be NULL");

    lua_getfield(L, 2, "x");
    float x = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "y");
    float y = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "width");
    float width = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "height");
    float height = lua_tonumber(L, -1);
    lua_pop(L, 1);

    b2BodyId body_id;
    assert(box_collider_parent_body_id(world, parent, &body_id) && "Cannot be false");

    b2Polygon box    = box_collider_make(x, y, width, height);
    b2ShapeDef shape = b2DefaultShapeDef();
    shape.isSensor   = true;

    lua_getfield(L, 2, "trigger");
    shape.isSensor = lua_toboolean(L, -1);
    lua_pop(L, 1);

    b2ShapeId shape_id = b2CreatePolygonShape(body_id, &shape, &box);

    Entity* entity     = malloc(sizeof(*entity));
    assert(entity != NULL && "Entity cannot be NULL");

    Entity** entity_ptr = lua_newuserdata(L, sizeof(*entity_ptr));
    assert(entity_ptr != NULL && "Entity pointer cannot be NULL!");

    lua_getfield(L, 2, "on_collision_enter");
    if (lua_isfunction(L, -1)) {
        entity->box_collider.on_collision_enter_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
        lua_pop(L, 1);
    }

    lua_getfield(L, 2, "on_collision_exit");
    if (lua_isfunction(L, -1)) {
        entity->box_collider.on_collision_exit_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
        lua_pop(L, 1);
    }

    entity_setup(L, entity, 2);
    entity_setup_update(L, entity, 2);
    setup_metatable(L, "BoxCollider", 2, NULL, 0);

    entity->self_ref              = luaL_ref(L, LUA_REGISTRYINDEX);
    entity->box_collider.shape_id = shape_id;
    entity->type                  = ENTITY_TYPE_BOX_COLLIDER;
    *entity_ptr                   = entity;
    b2Shape_SetUserData(shape_id, entity);

    return 1;
}

static int box_collider_index(lua_State* L) {
    luaL_getmetatable(L, "BoxCollider");
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1)) return 1;
    lua_pop(L, 2);

    lua_getfenv(L, 1);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    return 1;
}

static int box_collider_newindex(lua_State* L) {
    lua_getfenv(L, 1);
    lua_pushvalue(L, 2);
    lua_pushvalue(L, 3);
    lua_rawset(L, -3);
    return 0;
}

static luaL_Reg box_collider_funcs[] = {
    {"__newindex", box_collider_newindex},
    {"__index", box_collider_index},
    {NULL, NULL},
};

void register_box_collider_api(lua_State* L) {
    luaL_newmetatable(L, "BoxCollider");
    luaL_setfuncs(L, box_collider_funcs, 0);
    luaL_setmetatable(L, "BoxCollider");
}
