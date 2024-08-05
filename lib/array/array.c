#include "array/array.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Array {
    void** elements;
    size_t length;
    size_t capacity;
} Array;

static void try_array_expand(Array* this) {
    if (this->length >= this->capacity - 1) {
        size_t capacity = this->capacity * 1.5;
        void** temp     = realloc(this->elements, capacity * sizeof(void*));
        if (temp == NULL) {
            perror("failed to expand(realloc) array");
            return;
        }

        this->capacity = capacity;
        this->elements = temp;
    }
}

static void try_array_shrink(Array* this) {
    if (this->capacity <= ARRAY_DEFAULT_CAPACITY) return;

    size_t length   = this->length * 1.5;
    size_t capacity = this->capacity / 1.5;
    if (length < capacity) {
        void** temp = realloc(this->elements, capacity * sizeof(void*));
        if (temp == NULL) {
            perror("failed to shrink(realloc) array");
            return;
        }

        this->capacity = capacity;
        this->elements = temp;
    }
}

size_t array_length(const Array* this) {
    return this->length;
}

int array_find(const Array* this, const void* element) {
    for (int i = 0; i < array_length(this); i++) {
        if (array_get(this, i) == element) {
            return i;
        }
    }

    return -1;
}

void array_reverse(Array* this) {
    size_t length = array_length(this);
    for (int i = 0; i < length / 2; i++) {
        size_t first = i;
        size_t last  = (length - 1) - i;
        void* p1     = array_get(this, first);
        void* p2     = array_get(this, last);
        array_set(this, first, p2);
        array_set(this, last, p1);
    }
}

bool array_has(const Array* this, const void* element) {
    for (int i = 0; i < array_length(this); i++) {
        void* value = array_get(this, i);
        if (value == element) {
            return true;
        }
    }
    return false;
}

void* array_get(const Array* this, size_t index) {
    assert(index < this->length);
    return this->elements[index];
}

void array_set(Array* this, size_t index, void* element) {
    assert(index < this->length);
    this->elements[index] = element;
}

void array_push(Array* this, void* element) {
    size_t idx          = this->length;
    this->elements[idx] = element;
    this->length++;

    try_array_expand(this);
}

void* array_pop(Array* this) {
    if (this->length > 0) {
        size_t idx          = this->length - 1;
        void* element       = this->elements[idx];
        this->elements[idx] = NULL;
        this->length--;

        try_array_shrink(this);
        return element;
    }

    return NULL;
}

void array_free(Array* this) {
    for (int i = 0; i < this->length; i++) {
        void* ptr = this->elements[i];
        if (ptr != NULL) {
            free(ptr);
        }
    }

    free(this->elements);
    free(this);
}

Array* array_new() {
    Array* array    = malloc(sizeof(*array));
    array->elements = calloc(ARRAY_DEFAULT_CAPACITY, sizeof(void*));
    array->capacity = ARRAY_DEFAULT_CAPACITY;
    array->length   = 0;
    return array;
}
