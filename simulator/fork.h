#ifndef INJECTOR_FORK_H
#define INJECTOR_FORK_H

#include "fork_internal.h"

#define FREE_RTOS_FORK_SUCCESS 1
#define FREE_RTOS_FORK_FAILURE -1

/**
 * Create a new FreeRTOS instance.
 * 
 * Parameters:
 *  - freeRTOSInstance *instance encapsulates the platform-dependent
 *    informations on the created FreeRTOS injector instance (pid on
 *    posix systems, handle on Windows).
 *  - const char *injectorPath is the path to the injector executable.
 *  - const long *target is the name of the injection target.
 *  - const unsigned long time is the exact time (in nanoseconds) 
 *    at which the injector should perform the injection
 * 
 * Return value:
 *  - ret > 0: the instance was created successfully
 *  - ret < 0: an error occured
 */
int runFreeRTOSInjection(freeRTOSInstance *instance,
                         const char *injectorPath,
                         const void *target,
                         const unsigned long time);

/**
 * Wait for a FreeRTOS instance to complete
 */
int waitFreeRTOSInjection(const freeRTOSInstance *instance);

#endif