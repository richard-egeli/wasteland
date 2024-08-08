#include "action.h"

#include <ctype.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

#include "global.h"
#include "hashmap/hashmap.h"

void action_register(const char* key, int keycode) {
    HashMap* map = global.actions;
    if (map == NULL) {
        global.actions = hmap_new();
        map            = global.actions;
    }

    Action* action = NULL;
    if (hmap_get(map, key, (void**)&action)) {
        size_t size = sizeof(action->keys_bound) / sizeof(*action->keys_bound);
        for (int i = 0; i < size; i++) {
            if (action->keys_bound[i] == 0) {
                action->keys_bound[i] = keycode;
            }
        }
    } else {
        action = calloc(1, sizeof(*action));
        if (action == NULL) {
            perror("failed to allocate action");
            return;
        }

        action->keys_bound[0] = toupper(keycode);
        if (!hmap_put(map, key, action)) {
            perror("failed to put action in hashmap");
            free(action);
        }
    }
}

bool action_up(const char* key) {
    Action* action = NULL;

    if (hmap_get(global.actions, key, (void**)&action)) {
        size_t size = sizeof(action->keys_bound) / sizeof(*action->keys_bound);
        for (int i = 0; i < size; i++) {
            int key = action->keys_bound[i];
            if (key == 0) break;

            if (IsKeyUp(key)) {
                return true;
            }
        }
    }

    return false;
}

bool action_down(const char* key) {
    Action* action = NULL;

    if (hmap_get(global.actions, key, (void**)&action)) {
        size_t size = sizeof(action->keys_bound) / sizeof(*action->keys_bound);
        for (int i = 0; i < size; i++) {
            int key = action->keys_bound[i];
            if (key == 0) break;

            if (IsKeyDown(key)) {
                return true;
            }
        }
    }

    return false;
}

bool action_released(const char* key) {
    Action* action = NULL;

    if (hmap_get(global.actions, key, (void**)&action)) {
        size_t size = sizeof(action->keys_bound) / sizeof(*action->keys_bound);
        for (int i = 0; i < size; i++) {
            int key = action->keys_bound[i];
            printf("Key %d\n", key);
            if (key == 0) break;

            if (IsKeyReleased(key)) {
                return true;
            }
        }
    }

    return false;
}

bool action_pressed(const char* key) {
    Action* action = NULL;

    if (hmap_get(global.actions, key, (void**)&action)) {
        size_t size = sizeof(action->keys_bound) / sizeof(*action->keys_bound);
        for (int i = 0; i < size; i++) {
            int key = action->keys_bound[i];
            printf("Key %d\n", key);
            if (key == 0) break;

            if (IsKeyPressed(key)) {
                return true;
            }
        }
    }

    return false;
}
