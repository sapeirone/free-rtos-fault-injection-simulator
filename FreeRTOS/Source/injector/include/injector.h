/*
 * Fault injector - main header
 */

#include <stdlib.h>

#ifndef INJECTOR_H
#define INJECTOR_H

#define INJECTOR_ENABLED 1

typedef enum
{
    TYPE_STRUCT = 0,
    TYPE_VARIABLE = 1,
    TYPE_LIST = 2
} target_type_t;

struct target_s
{
    const char *name;
    void *address;
    unsigned int size;
    target_type_t type;
    struct target_s *content;
    struct target_s *next;
};

typedef struct target_s target_t;

target_t *read_tasks_targets(target_t *list);

target_t *create_target(const char *name, void *address, target_type_t type,
                        unsigned int size, target_t *content, target_t *next);

// #define NAME_OF(var) (#var)
#define nameof(var) (strrchr(#var, '>') ? (1 + strrchr(#var, '>')) : #var)

#define APPEND_TARGET(target, var, type)                                                      \
    {                                                                                         \
        target = create_target(nameof(var), (void *)&(var), type, sizeof(var), NULL, target); \
    };

#endif