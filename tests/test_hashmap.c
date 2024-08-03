#include <stdlib.h>
#include <string.h>
#include <unity.h>

#include "hashmap/hashmap.h"

static const char* hmap_element = "Hello, World!";

void tearDown() {
}
void setUp() {
}

static void test_hashmap_put(void) {
    HashMap* map    = hmap_new();
    const char* key = "MyItemKey";
    const char* ptr = NULL;

    TEST_ASSERT_TRUE(hmap_put(map, key, (void*)hmap_element));
    TEST_ASSERT_TRUE(hmap_get(map, key, (const void**)&ptr));
    TEST_ASSERT_EQUAL_CHAR_ARRAY(hmap_element, ptr, strlen(hmap_element));
    hmap_free(map);
}

static void test_hashmap_take(void) {
    HashMap* map    = hmap_new();
    const char* key = "TestItemKey";
    void* ptr       = NULL;

    TEST_ASSERT_TRUE(hmap_put(map, key, (void*)hmap_element));
    TEST_ASSERT_TRUE(hmap_take(map, key, &ptr));
    TEST_ASSERT_EQUAL_CHAR_ARRAY(hmap_element, ptr, strlen(hmap_element));
    TEST_ASSERT_FALSE(hmap_take(map, key, ptr));
    hmap_free(map);
}

static void test_hashmap_multiple_put(void) {
    HashMap* map = hmap_new();
    for (int i = 0; i < 100; i++) {
        char buf[32];
        size_t* v = malloc(sizeof(*v));
        *v        = i * 1024;

        snprintf(buf, sizeof(buf), "My Key %d\n", i);
        TEST_ASSERT_TRUE(hmap_put(map, buf, v));
    }

    TEST_ASSERT_EQUAL(137, hmap_capacity(map));

    for (int i = 0; i < 100; i++) {
        char buf[32];
        snprintf(buf, sizeof(buf), "My Key %d\n", i);
        size_t* value = NULL;
        TEST_ASSERT_TRUE(hmap_take(map, buf, (void**)&value));
        TEST_ASSERT_EQUAL(1024 * i, *value);
        free(value);
    }

    TEST_ASSERT_EQUAL(31, hmap_capacity(map));
    hmap_free(map);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_hashmap_put);
    RUN_TEST(test_hashmap_take);
    RUN_TEST(test_hashmap_multiple_put);

    return UNITY_END();
}
