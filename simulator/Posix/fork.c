#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <sys/resource.h>
#include <sched.h>

#include "../fork.h"
#include "fork_internal.h"

int runFreeRTOSInjection(freeRTOSInstance *instance,
                         const char *injectorPath,
                         const char *target,
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

    struct sched_param p;
    p.sched_priority = sched_get_priority_max(SCHED_RR);
    sched_setscheduler(getpid(), SCHED_RR, &p); // fail silently

    setpriority(PRIO_PROCESS, getpid(), -20);

    char timeBuffer[16];
    sprintf(timeBuffer, "%d", time);

    char offsetByteBuffer[16];
    sprintf(offsetByteBuffer, "%d", offsetByte);

    char offsetBitBuffer[16];
    sprintf(offsetBitBuffer, "%d", offsetBit);

    char *args[7] = {
        injectorPath, "--run",
        target, timeBuffer,
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

    if (WIFEXITED(exitCode)) {
        return WEXITSTATUS(exitCode);
    } else {
        return -1; // TODO: add more cases
    }
}

int waitFreeRTOSInjections(const freeRTOSInstance *instances, int size, int *exitCode) {
    int _exitCode;

    // wait for any child process
    pid_t pid = waitpid(-1, &_exitCode, 0);
    if (pid < 0) {
        // unexpected error of waitpid function
        return -1;
    }

    // find the corresponding pid in the instances array
    int pos;
    for (pos = 0; pos < size && instances[pos].pid != pid; pos++);

    if (pos == size) {
        // pid is not in the array: this is unexpected!
        return -1;
    }

    // check the exit code
    if (WIFEXITED(_exitCode)) {
        *exitCode = WEXITSTATUS(_exitCode);
    } else {
        *exitCode = -1; // TODO: add more cases
    }

    // return the position of the child that returned
    return pos;
}