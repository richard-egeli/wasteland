#include "entity.h"

#include <stdlib.h>

#include "collision/box_collider.h"

static void entity_tree_01(void) {
}

void entity_add_collider(Entity* this, int x, int y, int w, int h) {
    BoxCollider* col = box_collider_new(x, y, w, h);
    if (this->collider != NULL) {
        free(this->collider);
    }

    this->collider = col;
}

void entity_free(Entity* this) {
    if (this->collider) {
        box_collider_free(this->collider);
        this->collider = NULL;
    }

    free(this);
}

Entity* entity_new() {
    Entity* entity = calloc(1, sizeof(*entity));
    return entity;
}
