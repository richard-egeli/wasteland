#include "lua/animator.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "hashmap/hashmap.h"
#include "lauxlib.h"
#include "lua.h"
#include "lua/entity.h"
#include "lua/utils.h"
#include "lua/world.h"
#include "raylib.h"
#include "scene-graph/scene-graph.h"
#include "spritesheet.h"

static void animator_update(SceneGraph* graph, GameObject* object) {
    Entity* entity = object->data;
    assert(entity->type == ENTITY_TYPE_ANIMATOR && "Entity type is wrong?");

    if (entity->animator.active) {
        entity->animator.time += GetFrameTime() * entity->animator.time_multiplier;
        if (entity->animator.entity != NULL && entity->animator.animation != NULL) {
            Animator* animator   = &entity->animator;
            Sprite* sprite       = &animator->entity->sprite;
            Animation* anim      = animator->animation;
            AnimationFrame frame = anim->frames[animator->index];

            if (entity->animator.time >= frame.duration) {
                animator->index           = (animator->index + 1) % anim->frames_length;
                animator->time            = 0.0f;

                AnimationFrame* new_frame = &anim->frames[animator->index];
                sprite->row               = new_frame->row;
                sprite->col               = new_frame->col;
            }
        }
    }

    if (entity->update_ref != LUA_NOREF) {
        entity_call_update(graph, object);
    }
}

static void animator_setup_update(lua_State* L, Entity* entity, int idx) {
    SceneGraph* graph  = entity->weak_world_ptr->graph;
    GameObject* object = scene_graph_game_object_new(graph, entity->node);
    object->update     = animator_update;
    object->data       = entity;

    lua_getfield(L, idx, "update");
    if (lua_isfunction(L, -1)) {
        entity->update_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
        lua_pop(L, 1);
    }
}

int animator_create(lua_State* L) {
    assert(lua_gettop(L) == 2 && "Invalid arguments (world,definition)");
    assert(lua_isuserdata(L, 1) && "First argument must be valid userdata");
    assert(lua_istable(L, 2) && "Second argument must be a valid table");

    World* world = *(World**)lua_touserdata(L, 1);
    assert(world != NULL && "World cannot be NULL");

    Entity* entity = malloc(sizeof(*entity));
    assert(entity != NULL && "Entity cannot be NULL");

    Entity** entity_ptr = lua_newuserdata(L, sizeof(Entity*));
    *entity_ptr         = entity;

    entity_setup(L, entity, 2);
    animator_setup_update(L, entity, 2);
    setup_metatable(L, "Animator", 2, NULL, 0);

    Node parent_node = scene_graph_parent_get(world->graph, entity->node);
    Entity* parent   = scene_graph_userdata_get(world->graph, parent_node);
    assert(parent->type == ENTITY_TYPE_SPRITE && "Parent of animator must be of type Sprite");

    entity->type                     = ENTITY_TYPE_ANIMATOR;
    entity->animator.entity          = parent;
    entity->animator.time_multiplier = 1.0;
    entity->animator.time            = 0.0;
    entity->animator.active          = true;
    entity->animator.animation       = NULL;
    entity->animator.index           = 0;

    return 1;
}

int animator_play(lua_State* L) {
    assert(lua_gettop(L) == 2 && "Invalid arguments(userdata,name)");
    assert(lua_isuserdata(L, 1) && "First argument must be valid userdata");
    assert(lua_isstring(L, 2) && "Second argument must be a valid string");

    Entity* entity = *(Entity**)lua_touserdata(L, 1);
    assert(entity != NULL && "entity cannot be NULL");
    assert(entity->type == ENTITY_TYPE_ANIMATOR && "Entity type is not animator");

    size_t length      = 0;
    const char* name   = lua_tolstring(L, 2, &length);
    Animator* animator = &entity->animator;
    Entity* sprite     = animator->entity;
    SpriteSheet* sheet = sprite->sprite.spritesheet;
    assert(sprite != NULL && "Sprite cannot be NULL");
    assert(sheet != NULL && "Spritesheet cannot be NULL");

    HashMap* anim_map    = sheet->animations;
    Animation* animation = NULL;

    if (hmap_get(anim_map, name, length, (void**)&animation)) {
        animator->animation = animation;
        animator->time      = 0.0;
        animator->index     = 0;

        if (animation->frames_length > 0) {
            AnimationFrame* frame = &animation->frames[0];
            sprite->sprite.row    = frame->row;
            sprite->sprite.col    = frame->col;
        }
    }

    return 0;
}

static int animator_index(lua_State* L) {
    luaL_getmetatable(L, "Animator");
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1)) return 1;
    lua_pop(L, 2);

    lua_getfenv(L, 1);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    return 1;
}

static int animator_newindex(lua_State* L) {
    lua_getfenv(L, 1);
    lua_pushvalue(L, 2);
    lua_pushvalue(L, 3);
    lua_rawset(L, -3);
    return 0;
}

static luaL_Reg animator_api_funcs[] = {
    {"__newindex", animator_newindex},
    {"__index", animator_index},
    {"play", animator_play},
    {NULL, NULL},
};

void register_animator_api(lua_State* L) {
    luaL_newmetatable(L, "Animator");
    luaL_setfuncs(L, animator_api_funcs, 0);
    luaL_setmetatable(L, "Animator");
}
