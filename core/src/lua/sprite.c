#include "lua/sprite.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assets.h"
#include "lauxlib.h"
#include "lua.h"
#include "lua/entity.h"
#include "lua/utils.h"
#include "lua/world.h"
#include "raylib.h"
#include "rlgl.h"
#include "scene-graph/scene-graph.h"
#include "spritesheet.h"

static int sprite_set_cell(lua_State* L) {
    assert(lua_gettop(L) == 3 && "Not enough arguments (sprite, col, row)");
    assert(lua_isuserdata(L, 1) && "Invalid first argument");
    assert(lua_isnumber(L, 2) && "Invalid second argument");
    assert(lua_isnumber(L, 3) && "Invalid third argument");

    const Sprite* sprite = *(Sprite**)lua_touserdata(L, 1);
    const int col        = lua_tointeger(L, 2);
    const int row        = lua_tointeger(L, 3);

    return 0;
}

void sprite_draw(const Sprite* sprite, float x, float y) {
    float ratio   = GetScreenHeight() / 360.f;
    Rectangle src = {
        .x      = sprite->col * sprite->step_x,
        .y      = sprite->row * sprite->step_y,
        .width  = sprite->step_x,
        .height = sprite->step_y,
    };

    // Scale both position and dimensions
    float scaled_x       = x * ratio;
    float scaled_y       = y * ratio;
    float scaled_width   = sprite->step_x * ratio;
    float scaled_height  = sprite->step_y * ratio;

    float width          = sprite->width;
    float height         = sprite->height;
    Vector2 top_left     = (Vector2){scaled_x, scaled_y};
    Vector2 top_right    = (Vector2){scaled_x + scaled_width, scaled_y};
    Vector2 bottom_left  = (Vector2){scaled_x, scaled_y + scaled_height};
    Vector2 bottom_right = (Vector2){scaled_x + scaled_width, scaled_y + scaled_height};

    rlSetTexture(sprite->texture_id);
    rlBegin(RL_QUADS);
    rlColor4ub(255, 255, 255, 255);
    rlNormal3f(0.0f, 0.0f, 1.0f);

    // Top-left corner
    rlTexCoord2f(src.x / width, src.y / height);
    rlVertex2f(top_left.x, top_left.y);

    // Bottom-left corner
    rlTexCoord2f(src.x / width, (src.y + src.height) / height);
    rlVertex2f(bottom_left.x, bottom_left.y);

    // Bottom-right corner
    rlTexCoord2f((src.x + src.width) / width, (src.y + src.height) / height);
    rlVertex2f(bottom_right.x, bottom_right.y);

    // Top-right corner
    rlTexCoord2f((src.x + src.width) / width, src.y / height);
    rlVertex2f(top_right.x, top_right.y);

    rlEnd();
    rlSetTexture(0);
}

static void sprite_draw_ptr(SceneGraph* graph, Drawable* drawable) {
    Entity* entity    = drawable->data;
    Position position = scene_graph_position_get(graph, entity->node);
    sprite_draw(&entity->sprite, position.x, position.y);
}

Sprite* sprite_parse(lua_State* L, int node, int idx, Sprite* sprite) {
    assert(lua_istable(L, idx) && "Sprite is not a valid table");
    lua_getfield(L, idx, "sprite");
    assert(lua_isnumber(L, -1) && "Sprite not a valid ID");

    const int id       = lua_tointeger(L, -1);
    SpriteSheet* sheet = asset_get(id);
    assert(sheet != NULL && "Sprite sheet cannot be NULL!");
    lua_pop(L, 1);

    sprite->texture_id  = sheet->texture.id;
    sprite->step_x      = (float)sheet->texture.width / sheet->cols;
    sprite->step_y      = (float)sheet->texture.height / sheet->rows;
    sprite->width       = sheet->texture.width;
    sprite->height      = sheet->texture.height;
    sprite->spritesheet = sheet;
    sprite->col         = 0;
    sprite->row         = 0;

    return sprite;
}

int sprite_create(lua_State* L) {
    assert(lua_gettop(L) == 2 && "Wrong arguments (world, table)");
    assert(lua_isuserdata(L, 1) && "First argument must be userdata");
    assert(lua_istable(L, 2) && "Second argument must be a table");

    World* world = *(World**)lua_touserdata(L, 1);

    lua_getfield(L, 2, "col");
    int col = lua_tointeger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "row");
    int row = lua_tointeger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "x");
    float x = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "y");
    float y = lua_tonumber(L, -1);
    lua_pop(L, 1);

    Entity* entity = malloc(sizeof(*entity));
    assert(entity != NULL && "Entity cannot be NULL");

    Entity** entity_ptr = lua_newuserdata(L, sizeof(*entity_ptr));
    assert(entity_ptr != NULL && "Entity pointer cannot be NULL");

    *entity_ptr = entity;
    entity_setup(L, entity, 2);
    entity_setup_update(L, entity, 2);
    setup_metatable(L, "Sprite", 2, NULL, 0);

    sprite_parse(L, entity->node, 2, &entity->sprite);
    Drawable* draw = scene_graph_drawable_new(world->graph, entity->node);
    Position p     = scene_graph_position_get(world->graph, entity->node);
    scene_graph_local_position_set(world->graph, entity->node, (Position){x, y});

    draw->data         = entity;
    draw->draw         = sprite_draw_ptr;
    entity->type       = ENTITY_TYPE_SPRITE;
    entity->self_ref   = luaL_ref(L, LUA_REGISTRYINDEX);
    entity->sprite.col = col;
    entity->sprite.row = row;

    scene_graph_userdata_set(world->graph, entity->node, entity);

    return 1;
}

// Index/newindex handlers
static int sprite_index(lua_State* L) {
    luaL_getmetatable(L, "Sprite");
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1)) return 1;
    lua_pop(L, 2);

    lua_getfenv(L, 1);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    return 1;
}

static int sprite_newindex(lua_State* L) {
    lua_getfenv(L, 1);
    lua_pushvalue(L, 2);
    lua_pushvalue(L, 3);
    lua_rawset(L, -3);
    return 0;
}

static luaL_Reg sprite_func_api[] = {
    {"__newindex", sprite_newindex},
    {"__index", sprite_index},
    {"set_cell", sprite_set_cell},
    {"set_position", entity_set_position},
    {"get_position", entity_get_position},
    {NULL, NULL},
};

void register_sprite_api(lua_State* L) {
    luaL_newmetatable(L, "Sprite");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, sprite_func_api, 0);
    lua_pop(L, 1);
}
