#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include "sleep.h"

#define ONE_SEC_IN_NS (1000 * 1000 * 1000)
#define TIMESPEC_FROM_NS(ns) {ns / ONE_SEC_IN_NS, ns % ONE_SEC_IN_NS}

void sleepNanoseconds(unsigned long ns)
{
    struct timespec request = TIMESPEC_FROM_NS(ns);

    while (nanosleep(&request, &request) != 0)
    {
        /*char buffer[256];
        sprintf(buffer, "nanosleep returned an error. Remaining: %d, %d", request.tv_sec, request.tv_nsec);
        perror(buffer);*/
    }
}
