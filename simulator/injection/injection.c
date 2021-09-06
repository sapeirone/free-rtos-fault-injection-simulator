#include "../../FreeRTOS/Source/injector/include/injector.h"

void injectorFunction(target_t *target, int timeInj, int delta)
{

    srand(time(NULL));                              //generate random seed

    int offsetByte = rand()%target->size;           //select byte to inject

    int offsetBit = rand()%8;                       //select bit to inject

    int randTime= rand()%delta;                     //if delta!=0 select time in interval

    randTime=rand()%2?randTime:-randTime;           //choose if before or after selected time
    
    sleep(timeInj+randTime);                        //ready, waiting for injection
    
    int *memAddr = target->address;                 

    *memAddr ^= (1 << offsetByte*offsetBit);        //inject
}