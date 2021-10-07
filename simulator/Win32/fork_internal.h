#pragma once

#include <windows.h>

typedef struct
{
    HANDLE watchdog;
    HANDLE procHandle;
} freeRTOSInstance;