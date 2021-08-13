#include "loggingUtils.h"
#include "FreeRTOS.h"
#include "task.h"
#define LENBUF 1024

void loggingFunction(int logCause){
    
    unsigned long runTimeCounterValue = ulGetRunTimeCounterValue();
    static signed char bufferTCB[ LENBUF ];
    vTaskGetCurrentTCBStats(bufferTCB);

    switch(logCause){
        case 0:
            printf("%lu\t[OUT]\t%s\n", runTimeCounterValue, bufferTCB);
            break;
        case 1:
            printf("%lu\t[IN]\t%s\n", runTimeCounterValue, bufferTCB);
            break;
        default:
            printf("Default case\n");
            break;
    }
}