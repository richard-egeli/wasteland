#ifndef LIB_ARRAY_ARRAY_H_
#define LIB_ARRAY_ARRAY_H_

#include <stdbool.h>
#include <stddef.h>

#define ARRAY_DEFAULT_CAPACITY 16

typedef struct Array Array;

size_t array_length(const Array* this);

void* array_last(const Array* this);

int array_find(const Array* this, const void* element);

void array_sort(Array* this, int (*comp)(const void* a, const void* b));

void array_reverse(Array* this);

bool array_has(const Array* this, const void* element);

void* array_get(const Array* this, size_t index);

void array_set(Array* this, size_t index, void* element);

void array_push(Array* this, void* element);

void* array_pop(Array* this);

void array_free(Array* this);

Array* array_new(void);

#endif  // LIB_ARRAY_ARRAY_H_
