#ifndef CORE_INCLUDE_ENTITY_H_
#define CORE_INCLUDE_ENTITY_H_

#include <raylib.h>
#include <stdbool.h>

#include "sprite.h"

typedef struct BoxCollider BoxCollider;

typedef struct Entity {
    Sprite sprite;
    Vector2 position;
    BoxCollider* collider;
    bool destroyed;
} Entity;

void entity_add_collider(Entity* this, int x, int y, int w, int h);

void entity_free(Entity* this);

Entity* entity_new(void);

#endif  // CORE_INCLUDE_ENTITY_H_
