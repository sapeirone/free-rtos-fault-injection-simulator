#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include "../fork.h"
#include "fork_internal.h"

int runFreeRTOSInjection(freeRTOSInstance *instance,
                         const char *injectorPath,
                         const void *target,
                         const unsigned long time,
                         const unsigned long offsetByte,
                         const unsigned long offsetBit)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        return FREE_RTOS_FORK_FAILURE;
    }

    // father process: simply return
    if (pid)
    {
        instance->pid = pid;
        return FREE_RTOS_FORK_SUCCESS;
    }

    char timeBuffer[16];
    sprintf(timeBuffer, "%d", time);

    char targetBuffer[16];
    sprintf(targetBuffer, "%d", target);

    char offsetByteBuffer[16];
    sprintf(offsetByteBuffer, "%d", offsetByte);

    char offsetBitBuffer[16];
    sprintf(offsetBitBuffer, "%d", offsetBit);

    char *args[6] = {
        injectorPath, "--run",
        targetBuffer, timeBuffer,
        offsetByteBuffer, offsetBitBuffer,
        NULL};
    execv(injectorPath, args);

    // execv should never return
    perror("child: execv failed");

    return FREE_RTOS_FORK_FAILURE;
}

int waitFreeRTOSInjection(const freeRTOSInstance *instance)
{
    int exitCode;
    waitpid(instance->pid, &exitCode, 0);

    return WEXITSTATUS(exitCode);
}