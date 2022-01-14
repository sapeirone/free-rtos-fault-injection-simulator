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

    // the injector thread should not receive signals
    // => mask all the signals before creating the new thread
    sigset_t xAllSignals, old;
    sigfillset(&xAllSignals);
    pthread_sigmask(SIG_SETMASK, &xAllSignals, &old);

    // create a new thread for the injector
    if (pthread_create(&injectorThreadId, &attrs, function, (void *)injectionArgs) != 0)
    {
        return INJECTOR_THREAD_FAILURE;
    }

    // restore the previous signal mask for the current thread
    pthread_sigmask(SIG_SETMASK, &old, NULL);

    id->thread_id = injectorThreadId;
    return INJECTOR_THREAD_SUCCESS;
}

int detachThread(thread_t *id)
{
    pthread_detach(id->thread_id);
    return INJECTOR_THREAD_SUCCESS;
}
