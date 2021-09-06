#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "fork_internal.h"

int runFreeRTOSInjection(freeRTOSInstance *instance, const const char *injectorPath, const char *target, const double time)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        // TODO: need better error handling
        return 1;
    }

    // father process: simply return
    if (pid) {
        instance->pid = pid;
        return 0;
    }

    char buf[64];
    sprintf(buf, "%.2f", time);

    const char *args = {injectorPath, "--run", target, buf, NULL};
    execv(injectorPath, args); // execv should never return

    // TODO: need better error handling
    return 2;
}

int waitFreeRTOSInjection (const freeRTOSInstance *instance) {
    int exitCode;
    waitpid(instance->pid, &exitCode, NULL);

    return exitCode;
}