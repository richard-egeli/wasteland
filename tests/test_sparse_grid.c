#include <unity.h>

#include "collision/box_collider.h"
#include "collision/sparse_grid.h"

void setUp() {
}

void tearDown() {
}

static void test_sparse_grid_remove(void) {
    SparseGrid* grid = spgrid_new();

    BoxCollider* colliders[100];
    for (int i = 0; i < 100; i++) {
        colliders[i] = box_collider_new(64 * i, 64 * i, 16, 16);
        spgrid_insert(grid, colliders[i]);
    }

    for (int i = 25; i < 75; i++) {
        spgrid_remove(grid, colliders[i]);
    }

    int count = 0;
    BoxCollider* box;
    SparseGridIter* iter = spgrid_iter(grid);
    while ((box = spgrid_iter_next(iter))) {
        TEST_ASSERT_NOT_NULL(box);
        count++;
    }

    TEST_ASSERT_EQUAL(50, count);
}

static void test_sparse_grid_iter(void) {
    SparseGrid* grid = spgrid_new();

    for (int y = 0; y < 100; y++) {
        for (int x = 0; x < 100; x++) {
            BoxCollider* box = box_collider_new(64 * x, 64 * y, 16, 16);
            spgrid_insert(grid, box);
        }
    }

    BoxCollider* box;
    SparseGridIter* iter = spgrid_iter(grid);
    int count            = 0;
    while ((box = spgrid_iter_next(iter))) {
        TEST_ASSERT_NOT_NULL(box);
        box_collider_free(box);
        count++;
    }

    TEST_ASSERT_EQUAL(100 * 100, count);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_sparse_grid_iter);
    RUN_TEST(test_sparse_grid_remove);

    return UNITY_END();
}
