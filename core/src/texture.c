#include "texture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap/hashmap.h"

static HashMap* textures;
static HashMap* uid_lookup;

Texture texture_load(const char* filepath) {
    Texture* texture = NULL;
    if (hmap_get(textures, filepath, (void**)&texture)) {
        return *texture;
    }

    Texture temp = LoadTexture(filepath);
    if (temp.id == 0) {
        return temp;
    }

    texture = malloc(sizeof(*texture));
    if (texture == NULL) {
        fprintf(stderr, "failed to allocate texture: %s\n", filepath);
        UnloadTexture(temp);
        return (Texture){0};
    }

    *texture = temp;
    if (!hmap_put(textures, filepath, texture)) {
        fprintf(stderr, "failed to cache texture: %s\n", filepath);
        UnloadTexture(temp);
        free(texture);
        return (Texture){0};
    }

    return *texture;
}

void texture_link_uid(const char* filepath, int uid) {
    char uid_str[32];
    snprintf(uid_str, sizeof(uid_str), "%d", uid);

    char* p = strdup(filepath);
    if (!hmap_put(uid_lookup, uid_str, p)) {
        free(p);
    }
}

Texture texture_from_uid(int uid) {
    char uid_str[32];
    snprintf(uid_str, sizeof(uid_str), "%d", uid);
    char* path = NULL;

    if (hmap_get(uid_lookup, uid_str, (void**)&path)) {
        Texture* texture = NULL;
        if (hmap_get(textures, path, (void**)&texture)) {
            return *texture;
        }
    }

    return (Texture){0};
}

void texture_unload(Texture texture) {
}

void texture_free() {
    HashMapIter* h_it = hmap_iter(uid_lookup);
    const char* key;
    while (hmap_iter_next(h_it, &key)) {
        char* temp;
        if (hmap_take(uid_lookup, key, (void**)&temp)) {
            free(temp);
        }
    }

    h_it = hmap_iter(textures);
    while (hmap_iter_next(h_it, &key)) {
        Texture* temp;
        if (hmap_take(textures, key, (void**)&temp)) {
            free(temp);
        }
    }

    hmap_free(textures);
    hmap_free(uid_lookup);
}

void texture_init() {
    textures   = hmap_new();
    uid_lookup = hmap_new();
}
