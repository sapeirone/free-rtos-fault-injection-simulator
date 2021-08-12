#include "../../FreeRTOS/Source/injector/include/injector.h"

void injectorFunction(target_t *target)
{

    srand(time(NULL));

    sleep(5000); //confirm wait for all system variables in free rtos are initialized

    int offset = rand()%target->size;
    
    int *memAddr = target->address;

    *memAddr ^= (1 << offset);
}