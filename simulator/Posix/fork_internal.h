#pragma once

#include <unistd.h>
#include <time.h>

/**
 * Posix implementation of a freeRTOSInstance 
 */
typedef struct
{
    // watchdog timer used to kill the Free RTOS instance
    // if it stops responding
    timer_t watchdog;
    // process identifier of the Free RTOS instance
    pid_t pid;
} freeRTOSInstance;