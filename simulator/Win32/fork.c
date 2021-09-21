#undef UNICODE
#undef _UNICODE

#pragma warning(disable : 4996) // _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <string.h>
#include <stdio.h>

#include "../fork.h"
#include "fork_internal.h"

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

    SetPriorityClass(procInfo.hProcess, NORMAL_PRIORITY_CLASS);
    instance->procHandle = procInfo.hProcess;
    return FREE_RTOS_FORK_SUCCESS;
}

int waitFreeRTOSInjection(const freeRTOSInstance *instance)
{
    WaitForSingleObject(instance->procHandle, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeProcess(instance->procHandle, &exitCode);

    return (unsigned int) exitCode;
}