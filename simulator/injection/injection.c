#include <FreeRTOS.h>
#include <task.h>

#include <stdio.h>

#include "../../FreeRTOS/Source/injector/include/injector.h"
#include "sleep.h"
#include "thread.h"

void* injectorFunction(void *arg)
{
    thData_t *data = (thData_t*) arg;

    printf("injectorFunction called with args: %lu, %lu, %lu, %lu, %lu\n", data->address,
           data->injTime,
           data->timeoutNs, data->offsetByte, data->offsetBit);

    sleepNanoseconds(data->injTime);

    printf("Performing the injection...\n");
    *((char *)data->address + data->offsetByte) ^= (1 << data->offsetBit);
    printf("Injection completed\n");

    sleepNanoseconds(data->timeoutNs - data->injTime);

    printf("Goodbye!\n");
    vTaskEndScheduler();
}