#pragma once

#include <windows.h>

/**
 * Win32 implementation of a freeRTOSInstance 
 */
typedef struct
{
    // watchdog timer used to kill the Free RTOS instance
    // if it stops responding
    HANDLE watchdog;
    // process handle of the Free RTOS instance
    HANDLE procHandle;
} freeRTOSInstance;