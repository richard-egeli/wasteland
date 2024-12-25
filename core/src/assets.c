#include "assets.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

static void** assets;
static size_t assets_length;

void* asset_get(int key) {
    assert(key >= 0 && key < assets_length && "Key out of bounds");
    return assets[key];
}

int asset_add(void* data) {
    int index = assets_length;
    assets_length += 1;
    assets        = realloc(assets, sizeof(void*) * assets_length);
    assets[index] = data;
    return index;
}
