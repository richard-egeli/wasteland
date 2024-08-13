#ifndef CORE_INCLUDE_ENTITY_H_
#define CORE_INCLUDE_ENTITY_H_

#include <raylib.h>
#include <stdbool.h>

#include "collision/sparse_grid.h"
#include "sprite.h"

typedef struct BoxCollider BoxCollider;
typedef struct SparseGrid SparseGrid;

typedef struct Entity {
    Sprite sprite;
    Vector2 position;
    ColliderID collider;
    bool destroyed;
} Entity;

void entity_add_collider(SparseGrid* world, Entity* this, BoxCollider* collider);

void entity_free(Entity* this);

Entity* entity_new(void);

#endif  // CORE_INCLUDE_ENTITY_H_
