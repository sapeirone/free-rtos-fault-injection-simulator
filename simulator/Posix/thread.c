#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "../thread.h"

int launchThread(void *(*function)(void *),
                 const void *address,
                 const unsigned long injTime,
                 const unsigned long timeoutNs,
                 const unsigned long offsetByte,
                 const unsigned long offsetBit,
                 thread_t *id)
{
    pthread_t thread_id;

    // TODO: at the moment there is NOT a corresponding free operation
    thData_t *data = (thData_t *)malloc(sizeof(thData_t));
    data->address = address;
    data->injTime = injTime;
    data->timeoutNs = timeoutNs;
    data->offsetByte = offsetByte;
    data->offsetBit = offsetBit;
    
    if (pthread_create(&thread_id, NULL, function, (void *)data) != 0)
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