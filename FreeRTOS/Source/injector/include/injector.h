/*
 * Fault injector - main header
 */

#include <stdlib.h>

#ifndef INJECTOR_H
#define INJECTOR_H

#define INJECTOR_ENABLED 1

typedef enum
{
    STRUCT,
    VARIABLE,
    LIST
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

target_t* read_tasks_targets(target_t *list);

target_t *create_target(const char *name, void *address, unsigned int size, target_t *content, target_t *next);


#endif