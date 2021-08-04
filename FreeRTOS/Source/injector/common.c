#include "injector.h"

target_t *create_target(const char *name, void *address, target_type_t type,
                        unsigned int size, target_t *content, target_t *next)
{
    target_t *target = (target_t *)malloc(sizeof(target_t));

    if (target)
    {
        target->name = name;
        target->address = address;
        target->type = type;
        target->size = size;
        target->content = content;
        target->next = next;
    }

    return target;
}