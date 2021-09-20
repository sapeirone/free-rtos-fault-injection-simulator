#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include "sleep.h"
#include "wait_for_event.h"

#define ONE_SEC_IN_NS (1000 * 1000 * 1000)
#define TIMESPEC_FROM_NS(ns) {ns / ONE_SEC_IN_NS, ns % ONE_SEC_IN_NS}

void sleepNanoseconds(unsigned long ns)
{
    struct timespec request = TIMESPEC_FROM_NS(ns);

    nanosleep(&request, NULL);
}

struct event *injectionEvent;

void injectorWait () {
    injectionEvent = event_create();
    event_wait(injectionEvent);
}

void wakeInjector () {
    event_signal(injectionEvent);
}