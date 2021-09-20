#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "../simulator.h"

static void maskSignals();
static void unmaskSignals();

pthread_t injectorThreadId;

int launchInjectorThread(void *(*function)(void *),
                 const thData_t *injectionArgs,
                 thread_t *id)
{

    DEBUG_PRINT("launchInjectorThread called...\n");
    pthread_attr_t attrs;

    pthread_attr_init(&attrs);
    pthread_attr_setschedpolicy(&attrs, SCHED_FIFO);
    
    // the new thread inherits the signal mask from the parent
    sigset_t xAllSignals, old;
    sigfillset(&xAllSignals);    
    pthread_sigmask(SIG_SETMASK, &xAllSignals, &old);

    if (pthread_create(&injectorThreadId, &attrs, function, (void *)injectionArgs) != 0)
    {
        return INJECTOR_THREAD_FAILURE;
    }

    // restore the previous signal mask
    pthread_sigmask(SIG_SETMASK, &old, NULL);

    // DEBUG_PRINT("yielding...\n");
    // pthread_yield();
    id->thread_id = injectorThreadId;
    return INJECTOR_THREAD_SUCCESS;
}

int detachThread(thread_t *id)
{
    pthread_detach(id->thread_id);
    return INJECTOR_THREAD_SUCCESS;
}
