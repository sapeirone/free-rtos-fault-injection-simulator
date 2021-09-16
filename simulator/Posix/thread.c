#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "../simulator.h"

int launchInjectorThread(void *(*function)(void *),
                 const thData_t *injectionArgs,
                 thread_t *id)
{
    DEBUG_PRINT("launchInjectorThread called...\n");
    pthread_t thread_id;
    pthread_attr_t attrs;

    pthread_attr_init(&attrs);
    pthread_attr_setschedpolicy(&attrs, SCHED_FIFO);
    
    if (pthread_create(&thread_id, &attrs, function, (void *)injectionArgs) != 0)
    {
        return INJECTOR_THREAD_FAILURE;
    }

    // DEBUG_PRINT("yielding...\n");
    // pthread_yield();
    id->thread_id = thread_id;
    return INJECTOR_THREAD_SUCCESS;
}

int detachThread(thread_t *id)
{
    pthread_detach(id->thread_id);
    return INJECTOR_THREAD_SUCCESS;
}