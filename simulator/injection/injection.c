#include "../../FreeRTOS/Source/injector/include/injector.h"

void injectorFunction(void * target, unsigned long timeInj, unsigned long offsetByte, unsigned long offsetBit)
{
    nanosleep(timeInj);          //ready, waiting for injection

    *(target + offsetByte) ^= (1<<offsetBit)
}