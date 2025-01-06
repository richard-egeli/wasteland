#include "lua/event_handler.h"

#include <assert.h>
#include <stdio.h>

#include "box2d/box2d.h"
#include "box2d/types.h"
#include "lua.h"
#include "lua/entity.h"
#include "lua/world.h"
#include "scene-graph/scene-graph.h"

static int get_collision_enter_ref(Entity* entity) {
    switch (entity->type) {
        case ENTITY_TYPE_BOX_COLLIDER:
            return entity->box_collider.on_collision_enter_ref;
        default:
            break;
    }

    assert("Invalid entity type for collision event");
    return -1;
}

static int get_collision_exit_ref(Entity* entity) {
    switch (entity->type) {
        case ENTITY_TYPE_BOX_COLLIDER:
            return entity->box_collider.on_collision_exit_ref;
        default:
            break;
    }

    assert("Invalid entity type for collision event");
    return -1;
}

static void try_sensor_event_func(lua_State* L, int ref, Entity* sensor, Entity* visitor) {
    if (ref != -1) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
        assert(lua_isfunction(L, -1) && "Not a valid function");

        lua_rawgeti(L, LUA_REGISTRYINDEX, sensor->self_ref);
        lua_rawgeti(L, LUA_REGISTRYINDEX, visitor->self_ref);
        if (lua_pcall(L, 2, 0, 0) != LUA_OK) {
            // Handle error
            printf("Lua error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);  // Remove error message
        }
    }
}

void handle_collision_enter_events(World* world) {
    b2SensorEvents events = b2World_GetSensorEvents(world->id);

    for (int i = 0; i < events.beginCount; i++) {
        b2SensorBeginTouchEvent evt = events.beginEvents[i];
        b2BodyId body               = b2Shape_GetBody(evt.visitorShapeId);
        Entity* sensor              = b2Shape_GetUserData(evt.sensorShapeId);
        Entity* entity              = b2Body_GetUserData(body);
        int ref                     = get_collision_enter_ref(sensor);

        try_sensor_event_func(world->L, ref, sensor, entity);
    }
}

void handle_collision_exit_events(World* world) {
    b2SensorEvents events = b2World_GetSensorEvents(world->id);

    for (int i = 0; i < events.endCount; i++) {
        b2SensorEndTouchEvent evt = events.endEvents[i];
        b2BodyId body             = b2Shape_GetBody(evt.visitorShapeId);
        Entity* sensor            = b2Shape_GetUserData(evt.sensorShapeId);
        Entity* entity            = b2Body_GetUserData(body);
        int ref                   = get_collision_exit_ref(sensor);

        try_sensor_event_func(world->L, ref, sensor, entity);
    }
}

void handle_movement_events(World* world) {
    b2BodyEvents events = b2World_GetBodyEvents(world->id);
    for (int j = 0; j < events.moveCount; j++) {
        b2BodyMoveEvent evt = events.moveEvents[j];
        Entity* entity      = evt.userData;
        float x             = evt.transform.p.x;
        float y             = evt.transform.p.y;
        scene_graph_position_set(world->graph, entity->node, (Position){x, y});
    }
}
