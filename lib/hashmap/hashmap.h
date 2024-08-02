#ifndef LIB_HASHMAP_HASHMAP_H_
#define LIB_HASHMAP_HASHMAP_H_

#include <stdbool.h>
#include <stddef.h>

typedef struct HashMap HashMap;

size_t hmap_capacity(const HashMap* this);

bool hmap_has(const HashMap* this, const char* key);

bool hmap_put(HashMap* this, const char* key, void* value);

bool hmap_take(HashMap* this, const char* key, void** value);

bool hmap_get(const HashMap* this, const char* key, void** value);

void hmap_free(HashMap* this);

HashMap* hmap_new(void);

#endif  // LIB_HASHMAP_HASHMA_H_
