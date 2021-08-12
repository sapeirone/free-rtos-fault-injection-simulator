#include "../../FreeRTOS/Source/include/injector.h"

void injectorFunction(target_t *target)
{

    srand(time(NULL));
    
    sleep(5000); //confirm wait for all system variables in free rtos are initialized

    int offset = rand()%target->size;
    
    void *memAddr = target->address;

    *memAddr ^= (1 << *memAddr + offset);
}