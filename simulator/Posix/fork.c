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
                         const unsigned long time)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        // TODO: need better error handling
        return 1;
    }

    // father process: simply return
    if (pid)
    {
        instance->pid = pid;
        return FREE_RTOS_FORK_SUCCESS;
    }

    char timeBuffer[64];
    sprintf(timeBuffer, "%d", time);

    char targetBuffer[64];
    sprintf(targetBuffer, "0x%08x", target);

    char *args[5] = {injectorPath, "--run", targetBuffer, timeBuffer, NULL};
    // const char *args[2] = {"--run", NULL};
    printf("child: executing %s\n", injectorPath);
    printf("child: executing %s\n", targetBuffer);
    printf("child: executing %s\n", timeBuffer);
    execv("./build/sim", args); // execv should never return
    perror("child: execv failed");

    // TODO: need better error handling
    return FREE_RTOS_FORK_FAILURE;
}

int waitFreeRTOSInjection(const freeRTOSInstance *instance)
{
    int exitCode;
    waitpid(instance->pid, &exitCode, 0);

    return WEXITSTATUS(exitCode);
}