/*
 * Fault injector - main header
 */

#include <stdlib.h>

#ifndef INJECTOR_H
#define INJECTOR_H

#if POSIX
#define strtok_s strtok_r
#endif

/**
 * @brief An injection campaign's results.
 * 
 * For each possible outcome, count the number 
 * of simulations resulting in that outcome. 
 */
typedef struct injectionResults
{
    int nCrash, nHang, nSilent, nDelay, nError;
} injectionResults_t;

/**
 * @brief An injection campaign with its results
 */
typedef struct injectionCampaign
{
    // name of the target structure
    char *targetStructure;
    // number of injections to be performed
    int nInjections;
    // time instant of the injection, possibly with variance
    unsigned long medTimeRange, variance;
    // results of the injection
    injectionResults_t res;
    // a character representing the time distribution of
    // the injection ('u': uniform, 'g': gaussian)
    char distribution;
} injectionCampaign_t;

#define INJECTOR_ENABLED 1

#define TYPE_STRUCT_VALUE 1
#define TYPE_VARIABLE_VALUE 2
#define TYPE_LIST_VALUE 4
#define TYPE_ARRAY_VALUE 8
#define TYPE_POINTER_VALUE 16

/**
 * target_type_t defines the type of an injection target.
 * 
 * Types are represented as powers of 2 which allows to
 * compute new types as an OR function of these 
 * base types.
 */
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
    // unique id of the target
    int id;
    // name of the target
    char name[64];
    // start address of the target
    void *address;
    // size of an element
    unsigned int size;
    // number of elements (only for arrays)
    unsigned int nmemb;
    // type of the target (OR of TYPE_* constants)
    unsigned int type;

    struct target_s *parent;
    struct target_s *content;
    struct target_s *next;
};

typedef struct target_s target_t;

/**
 * @brief Read injection targets (global variables) from tasks.c
 * 
 * Targets are appended to the provided targets list argument if
 * it is not NULL. Otherwise a new list is created. 
 * 
 * @param list is the current list of targets (possibly NULL)
 * @return target_t* is the resulting list
 */
target_t *read_tasks_targets(target_t *list);

/**
 * @brief Read injection targets (global variables) from timers.c
 * 
 * Targets are appended to the provided targets list argument if
 * it is not NULL. Otherwise a new list is created. 
 * 
 * @param list is the current list of targets (possibly NULL)
 * @return target_t* is the resulting list
 */
target_t *read_timer_targets(target_t *list);

/**
 * @brief Create a target instance
 * 
 * Dinamically instantiate and populate a target instance.
 * 
 * @return target_t* is the created target
 */
target_t *create_target(const char *name, void *address, target_type_t type,
                        unsigned int size, target_t *content, target_t *next,
                        target_t *parent, unsigned int nmemb);

/**
 * @brief Pretty print a target type
 * 
 * @param type is the type to print
 * @param buffer is the target buffer
 */
void pretty_print_target_type(unsigned int type, char *buffer);

/**
 * @brief Free a list of dynamiccaly allocated targets
 * 
 * @param target is the list of target
 */
void freeInjectionTargets(target_t *target);

// #define NAME_OF(var) (#var)
// #var is substitued by the preprocessor with the variable's name
#define nameof(var) (strrchr(#var, '>') ? (1 + strrchr(#var, '>')) : #var)

/**
 * The following MACROs allow to append new append new targets to a targets
 * list using a clean syntax. 
 */

// append a fault injection target with a nmemb value different from 1
#define APPEND_ARRAY_TARGET(target, var, type, parent, nmemb)                                                                          \
    {                                                                                                                                  \
        target = create_target(nameof(var), (void *)&(var), type, sizeof(var[0]), NULL, target, parent, sizeof(var) / sizeof(var[0])); \
    };

// append a fault injection target
#define APPEND_PTR_TARGET(target, var, type, parent)                                                                     \
    {                                                                                                                    \
        target = create_target(nameof(var), (void *)&(var), type | TYPE_POINTER, sizeof(*var), NULL, target, parent, 1); \
    };

// append a fault injection target
#define APPEND_TARGET(target, var, type, parent)                                                         \
    {                                                                                                    \
        target = create_target(nameof(var), (void *)&(var), type, sizeof(var), NULL, target, parent, 1); \
    };

/**
 * @brief Injector thread function
 * 
 * @param arg is the parameter(s) to the thread
 * @return void* 
 */
void *injectorFunction(void *arg);

#endif