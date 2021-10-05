#pragma once

#include <unistd.h>
#include <time.h>

typedef struct
{
    timer_t watchdog;
    pid_t pid;
} freeRTOSInstance;