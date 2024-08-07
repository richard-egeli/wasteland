
#include "hashmap.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASHMAP_DEFAULT_CAPACITY 31
#define HASHMAP_MAX_LOAD_FACTOR  0.75
#define HASHMAP_MIN_LOAD_FACTOR  0.1

typedef struct KeyValue {
    struct KeyValue* next;
    uint32_t hash;
    char* key;
    void* value;
} KeyValue;

typedef struct HashMap {
    KeyValue** list;
    size_t capacity;
    size_t length;
} HashMap;

typedef struct HashMapIter {
    const HashMap* map;
    KeyValue* next;
    size_t index;
} HashMapIter;

static const size_t hmap_prime_table[] = {
    31,
    67,
    137,
    277,
    557,
    1117,
    2237,
    4481,
    8963,
    17929,
    35863,
    71741,
    143483,
    286973,
    573953,
    1147921,
};

static uint32_t hmap_hash(const char* str) {
    uint32_t h     = 0;
    uint32_t len   = strlen(str);
    const size_t m = 0x5bd1e995;
    const int r    = 24;

    while (len >= 4) {
        uint32_t k = *(uint32_t*)str;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        str += 4;
        len -= 4;
    }

    switch (len) {
        case 3:
            h ^= str[2] << 16;
        case 2:
            h ^= str[1] << 8;
        case 1:
            h ^= str[0];
            h *= m;
    }

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}

static inline bool hmap_exists(const HashMap* map,
                               const char* key,
                               uint32_t hash) {
    uint32_t index = hash % map->capacity;
    KeyValue* p    = map->list[index];
    while (p != NULL) {
        if (strcmp(key, p->key) == 0) {
            return true;
        }

        p = p->next;
    }

    return false;
}

static inline size_t hmap_next_prime(const HashMap* this) {
    size_t result = this->capacity;
    size_t count  = sizeof(hmap_prime_table) / sizeof(*hmap_prime_table);
    for (int i = 0; i < count; i++) {
        size_t temp = hmap_prime_table[i];
        if (temp > result) {
            result = temp;
            break;
        }
    }

    return result;
}

static inline size_t hmap_prev_prime(const HashMap* this) {
    size_t result = HASHMAP_DEFAULT_CAPACITY;
    size_t count  = sizeof(hmap_prime_table) / sizeof(*hmap_prime_table);
    for (int i = 0; i < count; i++) {
        size_t temp = hmap_prime_table[i];
        if (temp >= result) {
            break;
        }

        result = temp;
    }

    return result;
}

static inline void hmap_resize(HashMap* this, size_t new_capacity) {
    size_t old_capacity = this->capacity;
    KeyValue** old      = this->list;
    KeyValue** new      = calloc(new_capacity, sizeof(void*));
    if (new == NULL) {
        perror("failed to allocate hashmap resize");
        return;
    }

    for (int old_idx = 0; old_idx < old_capacity; old_idx++) {
        KeyValue* ptr = old[old_idx];
        while (ptr != NULL) {
            size_t new_idx = ptr->hash % new_capacity;
            KeyValue* next = ptr->next;
            ptr->next      = new[new_idx];
            new[new_idx]   = ptr;
            ptr            = next;
        }
    }

    this->capacity = new_capacity;
    this->list     = new;
    free(old);
}

static inline void try_hmap_expand(HashMap* this) {
    const float load_factor = (float)this->length / this->capacity;
    if (load_factor >= HASHMAP_MAX_LOAD_FACTOR) {
        size_t new_capacity = hmap_next_prime(this);
        hmap_resize(this, new_capacity);
    }
}

static inline void try_hmap_shrink(HashMap* this) {
    if (this->capacity == HASHMAP_DEFAULT_CAPACITY) return;

    const float load_factor = (float)this->length / this->capacity;
    if (load_factor < HASHMAP_MIN_LOAD_FACTOR) {
        size_t new_capacity = hmap_prev_prime(this);
        hmap_resize(this, new_capacity);
    }
}

static inline void hmap_key_free(KeyValue* kv) {
    free(kv->key);
    free(kv);
}

void* hmap_iter_next(HashMapIter* iter, const char** key) {
    while (iter->index < iter->map->capacity) {
        iter->next = iter->map->list[iter->index];
        if (iter->next == NULL) {
            iter->index++;
            continue;
        }

        iter->index++;
        void* value = iter->next->value;
        if (key != NULL) *key = iter->next->key;

        return value;
    }

    free(iter);
    return NULL;
}

HashMapIter* hmap_iter(const HashMap* this) {
    HashMapIter* iter = malloc(sizeof(*iter));
    iter->next        = NULL;
    iter->map         = this;
    iter->index       = 0;

    return iter;
}

size_t hmap_capacity(const HashMap* this) {
    return this->capacity;
}

bool hmap_has(const HashMap* this, const char* key) {
    return hmap_exists(this, key, hmap_hash(key));
}

bool hmap_put(HashMap* this, const char* key, void* value) {
    uint32_t hash = hmap_hash(key);

    if (!hmap_exists(this, key, hash)) {
        KeyValue* p = malloc(sizeof(*p));
        if (p == NULL) {
            perror("failed to allocate key/value to put element into hashmap");
            return false;
        }

        char* key_copy = strdup(key);
        if (key_copy == NULL) {
            perror("failed to copy key to new heap allocation");
            free(p);
            return false;
        }

        int index         = hash % this->capacity;
        p->key            = key_copy;
        p->value          = value;
        p->hash           = hash;
        p->next           = this->list[index];
        this->list[index] = p;
        this->length++;
        try_hmap_expand(this);
        return true;
    }

    return false;
}

bool hmap_take(HashMap* this, const char* str, void** value) {
    const uint32_t hash = hmap_hash(str) % this->capacity;
    KeyValue* current   = this->list[hash];
    KeyValue* previous  = NULL;

    while (current != NULL) {
        if (strcmp(str, current->key) == 0) {
            if (previous != NULL) {
                previous->next = current->next;
            } else {
                this->list[hash] = current->next;
            }

            *value = current->value;
            this->length--;
            hmap_key_free(current);
            try_hmap_shrink(this);
            return true;
        }

        previous = current;
        current  = current->next;
    }

    return false;
}

bool hmap_get(const HashMap* this, const char* str, void** value) {
    const uint32_t hash = hmap_hash(str) % this->capacity;
    const KeyValue* v   = this->list[hash];

    while (v != NULL) {
        if (strcmp(str, v->key) == 0) {
            *value = v->value;
            return true;
        }

        v = v->next;
    }

    return false;
}

void hmap_free(HashMap* this) {
    for (int i = 0; i < this->capacity; i++) {
        KeyValue* next = this->list[i];
        while (next != NULL) {
            KeyValue* temp = next;
            next           = next->next;
            free(temp);
        }
    }

    free(this->list);
    free(this);
}

HashMap* hmap_new(void) {
    HashMap* map = malloc(sizeof(*map));
    if (map == NULL) {
        perror("failed to allocate new hashmap");
        return NULL;
    }

    map->capacity = HASHMAP_DEFAULT_CAPACITY;
    map->list     = calloc(map->capacity, sizeof(void*));
    map->length   = 0;
    if (map->list == NULL) {
        perror("failed to allocate new list for hashmap");
        free(map);
        return NULL;
    }
    return map;
}
