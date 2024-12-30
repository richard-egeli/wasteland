#ifndef CORE_INCLUDE_LUA_UTILS_H_
#define CORE_INCLUDE_LUA_UTILS_H_

#include <stdbool.h>
#include <stddef.h>

typedef struct lua_State lua_State;

bool is_custom_property(const char* name, const char* filter[], size_t length);

/**
 * @param L The state of LUA
 * @param name The name of the metatable
 * @param idx Index of the custom properties on the stack
 * @param filter Filter for standard properties you don't want copied
 * @param length Length of the filter array
 */
void setup_metatable(lua_State* L, const char* name, int idx, const char* filter[], size_t length);

#endif  // CORE_INCLUDE_LUA_UTILS_H_
