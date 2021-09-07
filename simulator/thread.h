#ifndef INJECTOR_THREAD_H
#define INJECTOR_THREAD_H

#define INJECTOR_THREAD_SUCCESS 1
#define INJECTOR_THREAD_FAILURE -1

typedef struct thData_s {
    void * address;
    unsigned long injTime, offsetByte, offsetBit;
} thData_t;

int launchThread(void * function, void * address, unsigned long injTime, 
                unsigned long offsetByte, unsigned long offsetBit, thread_t * id);

int detachThread(thread_t * id);

#endif