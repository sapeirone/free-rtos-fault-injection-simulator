#undef UNICODE
#undef _UNICODE

#pragma warning(disable : 4996) // _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <string.h>
#include <stdio.h>

#include "../simulator.h"
#include "fork_internal.h"

#define ONE_SEC_IN_NS (1000 * 1000 * 1000)

static int runWatchdogTimer(LPHANDLE procHandle, LPHANDLE timerId);

int runFreeRTOSInjection(freeRTOSInstance *instance,
                         const char *injectorPath,
                         const char *target,
                         const unsigned long time,
                         const unsigned long offsetByte,
                         const unsigned long offsetBit)
{
    STARTUPINFO startupInfo;
    memset(&startupInfo, 0, sizeof(startupInfo));

    PROCESS_INFORMATION procInfo;

    char buffer[256];
    sprintf(buffer, "%s --run %s %d %d %d", injectorPath, target, time, offsetByte, offsetBit);

    BOOL result = CreateProcessA(
        NULL,                  // No module name (use command line)
        buffer,                // Command line
        NULL,                  // Process handle not inheritable
        NULL,                  // Thread handle not inheritable
        FALSE,                 // Set handle inheritance to FALSE
        NORMAL_PRIORITY_CLASS, // creation flags
        NULL,                  // Use parent's environment block
        NULL,                  // Use parent's starting directory
        &startupInfo,          // Pointer to STARTUPINFO structure
        &procInfo              // Pointer to PROCESS_INFORMATION structure
    );

    if (!result)
    {
        return FREE_RTOS_FORK_FAILURE;
    }

    SetPriorityClass(procInfo.hProcess, REALTIME_PRIORITY_CLASS);
    instance->procHandle = procInfo.hProcess;

    // setup the watchdog timer for the child process
    if (runWatchdogTimer(&instance->procHandle, &instance->watchdog) != 0)
    {
        ERR_PRINT("Cannot create watchdog timer for pid %d", GetProcessId(instance->procHandle));
        // error creating the watchdog timer
        // kill the child and return
        if (TerminateProcess(instance->procHandle, 1) == 0)
        {
            ERR_PRINT("Cannot terminate proc %p\n", instance->procHandle);
        }

        // wait the killed child process
        WaitForSingleObject(instance->procHandle, INFINITE);
        return FREE_RTOS_FORK_FAILURE;
    }

    return FREE_RTOS_FORK_SUCCESS;
}

int waitFreeRTOSInjection(const freeRTOSInstance *instance)
{
    while (WaitForSingleObjectEx(instance->procHandle, INFINITE, TRUE) == WAIT_IO_COMPLETION);

    DWORD exitCode = 0;
    GetExitCodeProcess(instance->procHandle, &exitCode);

    return (unsigned int)exitCode;
}

int waitFreeRTOSInjections(const freeRTOSInstance *instances, int size, int *exitCode)
{
    // Copy the wrapped HANDLEs in an array of HANDLEs
    HANDLE *instancesToWait;
    instancesToWait = (HANDLE *)malloc(sizeof(HANDLE) * 2 * size);
    for (int i = 0; i < size; ++i)
    {
        // even indices => proc handles
        instancesToWait[2 * i] = instances[i].procHandle;
        // odd indices => timer handles
        instancesToWait[2 * i + 1] = instances[i].watchdog;
    }

    // Wait for the first child process to exit
    DWORD waitReturnIndex;
    while ((waitReturnIndex = WaitForMultipleObjects(2 * size, instancesToWait, FALSE, 20)) == WAIT_TIMEOUT);

    if (waitReturnIndex == WAIT_FAILED)
    {
        // unexpected error of WaitForMultipleObjects function
        ERR_PRINT("WaitForMultipleObjectsEx return WAIT_FAILED (err_code=%d)", GetLastError());
        return -1;
    }

    DWORD index = waitReturnIndex - WAIT_OBJECT_0;
    if (index >= 2 * size || index < 0)
    {
        // HANDLE index is not in the array: this is unexpected!
        return -1;
    }

    freeRTOSInstance *instance = instances + index / 2;
    if (index % 2 == 1)
    {
        // odd index was signalled => watchdog expired => kill the corresponding process
        TerminateProcess(instance->procHandle, 1);
    }

    GetExitCodeProcess(instance->procHandle, (LPDWORD)exitCode);

    // kill the corresponding watchdog timer
    DEBUG_PRINT("Cancelling the watchdog timer\n");
    CancelWaitableTimer(instance->watchdog);
    CloseHandle(instance->watchdog);

    free(instancesToWait);

    // return the position of the child that returned
    return index / 2;
}

static int runWatchdogTimer(LPHANDLE procHandle, LPHANDLE timerId)
{
    LONGLONG llns = (LONGLONG)2 * ONE_SEC_IN_NS / 100LL; // 1 s
    LARGE_INTEGER li;

    // create the watchdog timer
    if ((*timerId = CreateWaitableTimer(NULL, TRUE, NULL)) == NULL)
    {
        return 1;
    }

    li.QuadPart = -llns;
    // start the watchdog timer
    if (SetWaitableTimer(*timerId, &li, 0, NULL, NULL, FALSE) == 0)
    {
        CloseHandle(*timerId);
        return 1;
    }

    return 0;
}
