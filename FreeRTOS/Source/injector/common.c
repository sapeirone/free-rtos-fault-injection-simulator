#include <string.h>

#include "injector.h"

target_t *create_target(const char *name, void *address, target_type_t type,
                        unsigned int size, target_t *content, target_t *next,
                        unsigned int nmemb)
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
        target->nmemb = nmemb;
    }

    return target;
}

void pretty_print_target_type(unsigned int type, char *buffer)
{
    buffer[0] = '\0';
    const char *type_literals[] = {
        "STRUCT",
        "VARIABLE",
        "LIST",
        "ARRAY",
        "POINTER"};

    for (unsigned int index = 0; index <= 5; index++) // todo replace with constant
    {
        if (type & (1 << index))
        {
            if (buffer[0])
                strcat(buffer, " | ");
            strcat(buffer, type_literals[index]);
        }
    }

    int n = strlen(buffer);
    if (n && buffer[n - 1] == ' ')
        buffer[n - 2] = '\0';
}

void freeInjectionTargets(target_t *target)
{
    if (target->next)
    {
        freeInjectionTargets(target->next);
    }

    if (target->content)
    {
        freeInjectionTargets(target->content);
    }

    free(target);
}