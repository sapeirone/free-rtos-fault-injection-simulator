#undef UNICODE
#undef _UNICODE

#include <windows.h>
#include <string.h>

#include "../thread.h"

int launchInjectorThread(void *(*function)(void *),
                         const thData_t *injectorArgs,
                         thread_t *id)
{
    HANDLE thread = INVALID_HANDLE_VALUE;
    DWORD thID;

    thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)function, (VOID *)injectorArgs, 0, &thID);
    if (thread == INVALID_HANDLE_VALUE)
    {
        return INJECTOR_THREAD_FAILURE;
    }

    SetThreadPriority(thread, THREAD_PRIORITY_TIME_CRITICAL);
    id->thread_id = thread;
    return INJECTOR_THREAD_SUCCESS;
}

int detachThread(thread_t *id)
{
    CloseHandle(id->thread_id);
    return INJECTOR_THREAD_SUCCESS;
}