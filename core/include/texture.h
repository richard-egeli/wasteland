#ifndef CORE_INCLUDE_TEXTURE_H_
#define CORE_INCLUDE_TEXTURE_H_

#include <raylib.h>

Texture texture_load(const char* filepath);

void texture_unload(Texture texture);

Texture texture_from_uid(int uid);

void texture_link_uid(const char* filepath, int uid);

void texture_free(void);

void texture_init(void);

#endif  // CORE_INCLUDE_TEXTURE_H_
