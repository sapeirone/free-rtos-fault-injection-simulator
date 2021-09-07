#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "../thread.h"

typedef struct {
    pthread_t thread_id;
} thread_t;

int launchThread(void * function,
                void * address,
                unsigned long injTime,
                unsigned long offsetByte,
                unsigned long offsetBit,
                thread_t * id){
    pthread_t thread_id;
    thData_t data = (thData_t) {address, injTime, offsetByte, offsetBit};
    if(pthread_create(&thread_id, NULL, function, (void *)data) != 0){
        return INJECTOR_THREAD_FAILURE;
    }
    id->thread_id = thread;
    return INJECTOR_THREAD_SUCCESS;
}

int detachThread(thread_t * id){
    pthread_detach(id->thread_id);
    return INJECTOR_THREAD_SUCCESS;
}