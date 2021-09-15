/*
 * Fault injector - main header
 */

#include <stdlib.h>

#ifndef INJECTOR_H
#define INJECTOR_H

#if POSIX
#define strtok_s strtok_r
#endif

typedef struct injectionResults
{
	int nCrash, nHang, nSilent, nDelay, nError;
} injectionResults_t;

typedef struct injectionCampaign
{
	char *targetStructure;
	int nInjections;
	unsigned long medTimeRange, variance;
	injectionResults_t res;
	char distribution;
} injectionCampaign_t;

#define INJECTOR_ENABLED 1

#define TYPE_STRUCT_VALUE 1
#define TYPE_VARIABLE_VALUE 2
#define TYPE_LIST_VALUE 4
#define TYPE_ARRAY_VALUE 8
#define TYPE_POINTER_VALUE 16

typedef enum
{
    TYPE_STRUCT = TYPE_STRUCT_VALUE,
    TYPE_VARIABLE = TYPE_VARIABLE_VALUE,
    TYPE_LIST = TYPE_LIST_VALUE,
    TYPE_ARRAY = TYPE_ARRAY_VALUE,
    TYPE_POINTER = TYPE_POINTER_VALUE
} target_type_t;

#define IS_TYPE_STRUCT(type) (type & TYPE_STRUCT_VALUE)
#define IS_TYPE_VARIABLE(type) (type & TYPE_VARIABLE_VALUE)
#define IS_TYPE_LIST(type) (type & TYPE_LIST_VALUE)
#define IS_TYPE_ARRAY(type) (type & TYPE_ARRAY_VALUE)
#define IS_TYPE_POINTER(type) (type & TYPE_POINTER_VALUE)

struct target_s
{
    int id;
    const char *name;
    void *address;
    unsigned int size;  // size of an element
    unsigned int nmemb; // number of elements (only for arrays)
    unsigned int type;
    struct target_s *content;
    struct target_s *next;
};

typedef struct target_s target_t;

/**
 * Read injection targets (global variables) from tasks.c
 */
target_t *read_tasks_targets(target_t *list);

/**
 * Read injection targets (global variables) from timers.c
 */
target_t *read_timer_targets(target_t *list);

target_t *create_target(const char *name, void *address, target_type_t type,
                        unsigned int size, target_t *content, target_t *next, unsigned int nmemb);

void pretty_print_target_type(unsigned int type, char *buffer);

void freeInjectionTargets(target_t *target);

// #define NAME_OF(var) (#var)
#define nameof(var) (strrchr(#var, '>') ? (1 + strrchr(#var, '>')) : #var)

// append a fault injection target with a nmemb value different from 1
#define APPEND_ARRAY_TARGET(target, var, type, nmemb)                                                                          \
    {                                                                                                                          \
        target = create_target(nameof(var), (void *)&(var), type, sizeof(var[0]), NULL, target, sizeof(var) / sizeof(var[0])); \
    };

// append a fault injection target
#define APPEND_TARGET(target, var, type)                                                         \
    {                                                                                            \
        target = create_target(nameof(var), (void *)&(var), type, sizeof(var), NULL, target, 1); \
    };

void* injectorFunction(void *arg);

#endif