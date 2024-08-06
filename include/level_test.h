#ifndef INCLUDE_LEVEL_TEST_H_
#define INCLUDE_LEVEL_TEST_H_

typedef struct Tilemap Tilemap;

typedef struct SPGrid SPGrid;

typedef struct Level_Test {
    Tilemap* tilemap;
    SPGrid* grid;
} Level_Test;

Level_Test level_test_load(void);

void level_test_update(const Level_Test* level);

#endif  // INCLUDE_LEVEL_TEST_H_
