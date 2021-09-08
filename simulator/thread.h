#ifndef INJECTOR_THREAD_H
#define INJECTOR_THREAD_H

#include "thread_internal.h"

#define INJECTOR_THREAD_SUCCESS 1
#define INJECTOR_THREAD_FAILURE -1

typedef struct thData_s
{
    void *address;
    unsigned long injTime, timeoutNs, offsetByte, offsetBit;
} thData_t;

int launchThread(void* (*function) (void*),
                 const void *address,
                 const unsigned long injTime,
                 const unsigned long timeoutNs,
                 const unsigned long offsetByte,
                 const unsigned long offsetBit,
                 thread_t *id);

int detachThread(thread_t *id);

#endif