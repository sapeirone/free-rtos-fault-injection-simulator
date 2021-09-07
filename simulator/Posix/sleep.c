#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include "sleep.h"

#define ONE_SEC_IN_NS (1000 * 1000 * 1000)
#define TIMESPEC_FROM_NS(ns) {ns / ONE_SEC_IN_NS, ns % ONE_SEC_IN_NS}

static void maskSignals();
static void unmaskSignals();

void sleepNanoseconds(unsigned long ns)
{
    // block all signals (with the exception of SIGINT)
    maskSignals();

    struct timespec request = TIMESPEC_FROM_NS(ns);
    struct timespec remaining;

    int ret = nanosleep(&request, &remaining);
    if (ret != 0)
    {
        char buffer[256];
        sprintf(buffer, "nanosleep returned an error. Remaining: %d, %d", remaining.tv_sec, remaining.tv_nsec);
        perror(buffer);
    }

    // unblock all signals (with the exception of SIGINT)
    unmaskSignals();
}

static void maskSignals()
{
    sigset_t xAllSignals, old;
    sigfillset(&xAllSignals);
    sigdelset(&xAllSignals, SIGINT);
    pthread_sigmask(SIG_BLOCK, &xAllSignals, &old);
}

static void unmaskSignals()
{
    sigset_t xAllSignals, old;
    sigfillset(&xAllSignals);
    sigdelset(&xAllSignals, SIGINT);
    pthread_sigmask(SIG_UNBLOCK, &xAllSignals, &old);
}