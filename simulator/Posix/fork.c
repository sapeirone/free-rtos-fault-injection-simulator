#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <sys/resource.h>
#include <sched.h>
#include <signal.h>
#include <time.h>

#include "../simulator.h"
#include "fork_internal.h"

#define WATCHDOG_TIMEOUT_SEC 1

/**
 * @brief Run a watchdog timer for the the requested process.
 * 
 * Setup a watchdog timer that kills the child process running a
 * FreeRTOS simulation if it exceeds a maximum execution time.
 * The timeout is specified by WATCHDOG_TIMEOUT_SEC.
 * 
 * @param pid is the identifier of the process to watch
 * @param timerId is a pointer to the timer
 * @return int is zero if the watchdog was created successfully, 
 * false otherwise
 */
static int run_watchdog_timer(pid_t pid, timer_t *timerId);

/**
 * @brief A killer.
 * 
 * @param val is the pid of the process to kill
 */
static void watchdog_func(union sigval val);


int runFreeRTOSInjection(freeRTOSInstance *instance,
                         const char *injectorPath,
                         const char *target,
                         const unsigned long time,
                         const unsigned long offsetByte,
                         const unsigned long offsetBit)
{
    // fork a child process for the Free RTOS simulation
    pid_t pid = fork();

    if (pid < 0)
    {
        return FREE_RTOS_FORK_FAILURE;
    }

    // father process: simply return
    if (pid)
    {
        if (run_watchdog_timer(pid, &instance->watchdog) != 0) {
            // creation of the timer failed
            // kill the child and return an error
            kill(pid, SIGKILL);
            return FREE_RTOS_FORK_FAILURE;
        }

        instance->pid = pid;
        return FREE_RTOS_FORK_SUCCESS;
    }

    // try to increase the scheduling priority of the threads
    // of the current process to realtime (this requires root privileges)
    struct sched_param p;
    p.sched_priority = sched_get_priority_max(SCHED_RR);
    sched_setscheduler(getpid(), SCHED_RR, &p); // fail silently

    // increase the process's scheduling priority
    setpriority(PRIO_PROCESS, getpid(), -20);

    char timeBuffer[16];
    sprintf(timeBuffer, "%ld", time);

    char offsetByteBuffer[16];
    sprintf(offsetByteBuffer, "%ld", offsetByte);

    char offsetBitBuffer[16];
    sprintf(offsetBitBuffer, "%ld", offsetBit);

    // start the simulation (--run command of the main process)
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

static int run_watchdog_timer(pid_t pid, timer_t *timerId) {
    // setup an event that calls the watchdog function after
    // the timeout expired
    struct sigevent sig;
    sig.sigev_notify = SIGEV_THREAD; // run the handler as a new thread
    // when the timer expires watchdog_func is called
    sig.sigev_notify_function = &watchdog_func;
    // with pid as parameter
    sig.sigev_value.sival_int = pid;
    sig.sigev_notify_attributes = NULL;

    // create the timer using the CLOCK_MONOTONIC clock (the same used
    // by run-time-stats-utils)
    if (timer_create(CLOCK_MONOTONIC, &sig, timerId) != 0) {
        ERR_PRINT("timer_create failed");
        return 1;
    }

    // setup the timer duration
    struct itimerspec in, out;
    in.it_value.tv_sec = WATCHDOG_TIMEOUT_SEC;
    in.it_value.tv_nsec = 0;
    // this interval is NOT periodic
    in.it_interval.tv_sec = 0;
    in.it_interval.tv_nsec = 0;

    if (timer_settime(*timerId, 0, &in, &out) != 0) {
        ERR_PRINT("timer_settime failed");
        return 1;
    }

    return 0;
}

static void watchdog_func(union sigval val) {
    DEBUG_PRINT("Killing %d...\n", val.sival_int);
    if (kill((pid_t) val.sival_int, SIGKILL) != 0) {
        ERR_PRINT("watchdog timer failed to kill pid %d: this should not happen!\n", val.sival_int);
    }
}

int waitFreeRTOSInjection(const freeRTOSInstance *instance)
{
    int exitCode;
    
    // wait for the instance to terminate
    waitpid(instance->pid, &exitCode, 0);

    // stop the watchdog timer
    timer_delete(instance->watchdog);

    if (WIFEXITED(exitCode)) {
        // instance exited normally => return the status code
        return WEXITSTATUS(exitCode);
    } else {
        // instance crashed
        return -1;
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

    // stop the watchdog timer
    timer_delete(instances[pos].watchdog);

    // check the exit code
    if (WIFEXITED(_exitCode)) {
        // instance exited normally => return the status code
        *exitCode = WEXITSTATUS(_exitCode);
    } else {
        // instance crashed
        *exitCode = -1; 
    }

    // return the position of the child that returned
    return pos;
}