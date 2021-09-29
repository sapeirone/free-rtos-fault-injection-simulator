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
                         const char *target,
                         const unsigned long time,
                         const unsigned long offsetByte,
                         const unsigned long offsetBit);

/**
 * Wait for a FreeRTOS instance to complete
 */
int waitFreeRTOSInjection(const freeRTOSInstance *instance);

/**
 * Wait for one FreeRTOS instance to complete.
 * 
 * const freeRTOSInstance *instances is the array of instances to wait
 * int size is the number of elements in the instances array
 * int *exitCode is a pointer to the exit code of the terminated instance
 * 
 * The function returns the position of the instance that terminated.
 * In case of errors, -1 is returned.
 */
int waitFreeRTOSInjections(const freeRTOSInstance *instances, int size, int *exitCode);

#endif