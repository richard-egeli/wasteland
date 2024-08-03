#include <stdlib.h>
#include <unity.h>

#include "array/array.h"

void setUp() {
}
void tearDown() {
}

static void test_array_push(void) {
    Array* array = array_new();

    for (int i = 0; i < 1000; i++) {
        int* value = malloc(sizeof(int));
        *value     = i;
        array_push(array, value);
        TEST_ASSERT_EQUAL(i + 1, array_length(array));
    }

    TEST_ASSERT_EQUAL(1000, array_length(array));

    for (int i = 0; i < array_length(array); i++) {
        int* value = array_get(array, i);
        TEST_ASSERT_EQUAL(i, *value);
    }

    const int max = array_length(array);
    for (int i = 0; i < max; i++) {
        int* value = array_pop(array);
        TEST_ASSERT_EQUAL(max - i - 1, *value);
        free(value);
    }

    TEST_ASSERT_EQUAL(0, array_length(array));
    array_free(array);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_array_push);
    return UNITY_END();
}
