#undef UNICODE
#undef _UNICODE

#include <windows.h>
#include <string.h>

#include "../thread.h"



int launchThread(void* (*function) (void*),
                const void * address,
                const unsigned long injTime,
                const unsigned long timeoutNs,
                const unsigned long offsetByte,
                const unsigned long offsetBit,
                thread_t * id){
    HANDLE thread = INVALID_HANDLE_VALUE;
    DWORD thID;
    
    // TODO: at the moment there is NOT a corresponding free operation
    thData_t *data = (thData_t *)malloc(sizeof(thData_t));
    data->address = address;
    data->injTime = injTime;
    data->timeoutNs = timeoutNs;
    data->offsetByte = offsetByte;
    data->offsetBit = offsetBit;

    thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) function, (VOID *) data, 0, &thID);
    if(thread == INVALID_HANDLE_VALUE){
        return INJECTOR_THREAD_FAILURE;
    }
    id->thread_id = thread;
    return INJECTOR_THREAD_SUCCESS;
}

int detachThread(thread_t * id){
    CloseHandle(id->thread_id);
    return INJECTOR_THREAD_SUCCESS;
}