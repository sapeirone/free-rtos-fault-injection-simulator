#undef UNICODE
#undef _UNICODE

#include <windows.h>
#include <string.h>

#include "../fork.h"
#include "fork_internal.h"

int runFreeRTOSInjection(freeRTOSInstance *instance, const const char *injectorPath, const char *target, const double time)
{
    STARTUPINFO startupInfo;
    memset(&startupInfo, 0, sizeof(startupInfo));

    PROCESS_INFORMATION procInfo;

    char buffer[256];
    sprintf(buffer, "%s --run %s %.2f", injectorPath, target, time);

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
        &processInformation);  // Pointer to PROCESS_INFORMATION structure
    );

    if (!result)
    {
        return FREE_RTOS_FORK_FAILURE;
    }

    instance->procHandle = processInformation.hProcess;
    return FREE_RTOS_FORK_SUCCESS;
}

int waitFreeRTOSInjection(const freeRTOSInstance *instance)
{
    WaitForSingleObject(instance->procHandle);

    int exitCode;
    GetExitCodeProcess(instance->procHandle, &exitCode);

    return exitCode;
}