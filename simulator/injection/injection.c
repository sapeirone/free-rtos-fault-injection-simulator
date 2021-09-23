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
        List_t *list;
        if (data->isPointer)
            list = *((List_t**)data->address);
        else 
            list = (List_t*)data->address;

        ListItem_t *item = list->pxIndex;

        int position = (data->listPosition >= 0) ? data->listPosition : (rand() % list->uxNumberOfItems);
        if (position < list->uxNumberOfItems) {
            for (int i = 0; i < position && item; i++) {
                item = item->pxNext;
            }

            if (item) {
                *((char *)item + data->offsetByte) ^= (1 << data->offsetBit);        
            }
        }
    }
    else if (data->isPointer)
    {
        // The selected injection target is a pointer.
        // The injection target has to dereference the pointer,
        // possibly add the offset (nonzero only for struct targets)
        // and add the offset byte.
        //
        // Example: pxCurrentTCB.uxPriority
        // data->address contains the address of pxCurrentTCB
        // *((char **)data->address is the address of the current TCB stored in pxCurrentTCB
        // data->offset is the offset of the field uxPriority wrt to start of TCB struct
        // data->offsetByte is the byte inside the uxPriority field which is the target of the injection
        //
        // The sum of these three terms is the address of the selected byte.
        *(*((char **)data->address) + (unsigned long)data->offset + data->offsetByte) ^= (1 << data->offsetBit);
    }
    else
    {
        // Standard case: sum the injection address and the offset byte.
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