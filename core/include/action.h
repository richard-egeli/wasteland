#ifndef CORE_INCLUDE_ACTION_H_
#define CORE_INCLUDE_ACTION_H_

#include <stdbool.h>

typedef struct Action {
    int keys_bound[8];
} Action;

void action_register(const char* action, int key);

bool action_up(const char* action);

bool action_down(const char* action);

bool action_pressed(const char* action);

bool action_released(const char* action);

#endif  // CORE_INCLUDE_ACTION_H_
