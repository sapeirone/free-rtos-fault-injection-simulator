#include <FreeRTOS.h>
#include <task.h>

#include <stdio.h>

#include "simulator.h"

int mustEnd = 0;

void *injectorFunction(void *arg)
{
    thData_t *data = (thData_t *)arg;

    DEBUG_PRINT("Requested injection address: %p\n", data->address);
    DEBUG_PRINT("Requested injection time: %lu\n", data->injTime);
    DEBUG_PRINT("Requested injection offset byte: %lu\n", data->offsetByte);
    DEBUG_PRINT("Requested injection offset bit: %lu\n", data->offsetBit);

    sleepNanoseconds(data->injTime);
    //injectorWait ();

    unsigned long long currentTime = ulGetRunTimeCounterValue();

    DEBUG_PRINT("Performing the injection at time %lu...\n", currentTime);
    DEBUG_PRINT("Injection delay: %d (%d - %d) \n", ((signed) currentTime - (signed) data->injTime), (signed) currentTime, (signed) data->injTime);
    if (data->isList) {
        List_t *list = (List_t*)data->address;
        ListItem_t *item = list->pxIndex;
        if (data->listPosition < list->uxNumberOfItems) {
            for (int i = 0; i < data->listPosition && item; i++) {
                item = item->pxNext;
            }

            if (item) {
                *((char *)item + data->offsetByte) ^= (1 << data->offsetBit);        
            }
        }
    } else {
        *((char *)data->address + data->offsetByte) ^= (1 << data->offsetBit);
    }
    DEBUG_PRINT("Injection completed\n");

    DEBUG_PRINT("Waiting the execution timeout\n");
    sleepNanoseconds(data->timeoutNs - currentTime);

    DEBUG_PRINT("The execution timeout expired\n");
    vPortGenerateSimulatedInterrupt( 5 );
    vTaskEndScheduler();

    // vTaskEndScheduler should NOT return
    DEBUG_PRINT("injectorFunction is executing past vTaskEndScheduler!!!\n");

    return NULL;
}