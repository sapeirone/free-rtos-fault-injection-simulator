#include "../../FreeRTOS/Source/injector/include/injector.h"
#include "sleep.h"

void injectorFunction(void * target, unsigned long timeInj, unsigned long offsetByte, unsigned long offsetBit)
{
    sleepNanoseconds(timeInj);

    *((char*)target + offsetByte) ^= (1<<offsetBit);
}