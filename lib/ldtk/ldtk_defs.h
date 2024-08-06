#ifndef LIB_LDTK_LDTK_DEFS_H_
#define LIB_LDTK_LDTK_DEFS_H_

#define STRING(json, key)                      \
    ({                                         \
        void* obj = yyjson_obj_get(json, key); \
        obj ? yyjson_get_str(obj) : NULL;      \
    })

#define BOOL(json, key)                        \
    ({                                         \
        void* obj = yyjson_obj_get(json, key); \
        obj ? yyjson_get_bool(obj) : false;    \
    })

#define INT(json, key)                         \
    ({                                         \
        void* obj = yyjson_obj_get(json, key); \
        obj ? yyjson_get_int(obj) : 0;         \
    })

#define FLOAT(json, key)                       \
    ({                                         \
        void* obj = yyjson_obj_get(json, key); \
        obj ? yyjson_get_num(obj) : 0;         \
    })

#define VEC2(array, json, key)                   \
    {                                            \
        void* obj = yyjson_obj_get(json, key);   \
        size_t idx, max;                         \
        yyjson_val* val;                         \
        yyjson_arr_foreach(obj, idx, max, val) { \
            array[idx] = yyjson_get_num(val);    \
        }                                        \
    }

#define STRING_ARRAY(array, json, key)                       \
    {                                                        \
        void* obj = yyjson_obj_get(json, key);               \
        if (array == NULL) {                                 \
            perror("failed to alloc string array");          \
        }                                                    \
        size_t idx, max;                                     \
        yyjson_val* value;                                   \
        yyjson_arr_foreach(obj, idx, max, value) {           \
            array_push(array, (void*)yyjson_get_str(value)); \
        }                                                    \
    }

#endif  // LIB_LDTK_LDTK_DEFS_H_
