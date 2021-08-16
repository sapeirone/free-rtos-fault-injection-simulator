#include "../../FreeRTOS/Source/injector/include/injector.h"

void injectorFunction(target_t *target)
{

    srand(time(NULL));

    sleep(3000); //confirm wait for all system variables in free rtos are initialized

    int offsetByte = rand()%target->size;

    int bit
    
    int *memAddr = target->address;

    *memAddr ^= (1 << offset);
}