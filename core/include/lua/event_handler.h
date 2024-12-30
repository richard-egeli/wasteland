#ifndef CORE_INCLUDE_LUA_COLLISION_EVENTS_H_
#define CORE_INCLUDE_LUA_COLLISION_EVENTS_H_

typedef struct World World;

void handle_collision_enter_events(World* world);

void handle_collision_exit_events(World* world);

void handle_movement_events(World* world);

#endif  // CORE_INCLUDE_LUA_COLLISION_EVENTS_H_
