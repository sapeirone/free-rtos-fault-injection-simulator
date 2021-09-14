#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "../thread.h"

int launchInjectorThread(void *(*function)(void *),
                 const thData_t *injectionArgs,
                 thread_t *id)
{
    pthread_t thread_id;
    
    if (pthread_create(&thread_id, NULL, function, (void *)injectionArgs) != 0)
    {
        return INJECTOR_THREAD_FAILURE;
    }
    id->thread_id = thread_id;
    return INJECTOR_THREAD_SUCCESS;
}

int detachThread(thread_t *id)
{
    pthread_detach(id->thread_id);
    return INJECTOR_THREAD_SUCCESS;
}