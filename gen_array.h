#ifndef __GEN_ARRAY_H__
#define __GEN_ARRAY_H__

#include <stdlib.h>
#include <assert.h>

#define INIT_SIZE 64

#define ARRAY_STRUCT(NAME, TYPE)                                             \
    struct array_##NAME {                                                    \
        size_t size;                                                         \
        size_t alloc;                                                        \
        TYPE *data;                                                          \
    }

#define ARRAY_FUNCS(NAME, TYPE)                                              \
    void array_##NAME##_init(struct array_##NAME *array)                     \
    {                                                                        \
        array->size = 0;                                                     \
        array->alloc = INIT_SIZE;                                            \
        array->data = calloc(array->alloc, sizeof(TYPE));                    \
        if (array->data == NULL) {                                           \
            perror("calloc");                                                \
            exit(EXIT_FAILURE);                                              \
        }                                                                    \
    }                                                                        \
                                                                             \
    void array_##NAME##_append(struct array_##NAME *array, TYPE value)       \
    {                                                                        \
        *(array->data + array->size++) = value;                              \
                                                                             \
        if (array->size == array->alloc) {                                   \
            array->alloc *= 2;                                               \
            TYPE *new = realloc(array->data, array->size * sizeof(*new));    \
            if (new == NULL) {                                               \
                perror("realloc");                                           \
                exit(EXIT_FAILURE);                                          \
            }                                                                \
            array->data = new;                                               \
        }                                                                    \
    }                                                                        \
                                                                             \
    void array_##NAME##_free(struct array_##NAME *array)                     \
    {                                                                        \
        if (!array) {                                                        \
            return;                                                          \
        };                                                                   \
        free(array->data);                                                   \
        array->size = array->alloc = 0;                                      \
        array->data = NULL;                                                  \
    }

#endif /* __GEN_ARRAY_H__ */
