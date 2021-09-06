#include "fork_internal.h"

int runFreeRTOSInjection(freeRTOSInstance *instance,
                         const char *injectorPath,
                         const char *target,
                         const double time);

int waitFreeRTOSInjection(const freeRTOSInstance *instance);