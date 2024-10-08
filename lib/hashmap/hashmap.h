#ifndef LIB_HASHMAP_HASHMAP_H_
#define LIB_HASHMAP_HASHMAP_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct HashMap HashMap;

typedef struct HashMapIter HashMapIter;

void* hmap_iter_next(HashMapIter* iter, const char** key);

void* hmap_iter_first(HashMapIter* iter, const char** key);

HashMapIter* hmap_iter(const HashMap* this);

size_t hmap_capacity(const HashMap* this);

bool hmap_has(const HashMap* this, const char* key, uint32_t len);

bool hmap_put(HashMap* this, const char* key, uint32_t len, void* value);

bool hmap_take(HashMap* this, const char* key, uint32_t len, void** value);

bool hmap_get(const HashMap* this, const char* key, uint32_t len, void** value);

void hmap_free(HashMap* this);

HashMap* hmap_new(void);

#endif  // LIB_HASHMAP_HASHMA_H_
