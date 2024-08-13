#include "entity.h"

#include <stdlib.h>

#include "collision/box_collider.h"
#include "collision/sparse_grid.h"

void entity_add_collider(SparseGrid* world, Entity* this, BoxCollider* collider) {
    this->collider = spgrid_insert(world, collider);
}

void entity_free(Entity* this) {
    /*if (this->collider) {*/
    /*    box_collider_free(this->collider);*/
    /*    this->collider = NULL;*/
    /*}*/

    free(this);
}

Entity* entity_new() {
    Entity* entity = calloc(1, sizeof(*entity));
    return entity;
}
