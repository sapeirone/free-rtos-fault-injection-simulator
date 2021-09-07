#include "../../FreeRTOS/Source/injector/include/injector.h"

void injectorFunction(void * target, unsigned long timeInj, unsigned long position)
{
    nanosleep(timeInj);          //ready, waiting for injection
    
    target ^= (1 << position);   //inject
}