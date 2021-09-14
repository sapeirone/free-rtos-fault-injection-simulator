#include <stdio.h>
#include "loggingUtils.h"
#include "FreeRTOS.h"
#include "task.h"

signed char loggerTrace[TRACELEN][LENBUF];
int index = 0;

void loggingFunction(int logCause){
    
    unsigned long runTimeCounterValue = ulGetRunTimeCounterValue();
    static signed char bufferTCB[LENBUF];
    static signed char bufferStr[LENBUF];
    vTaskGetCurrentTCBStats(bufferTCB);

    switch(logCause){
        case 0:
            sprintf(bufferStr, "%lu\t[OUT]\t%s", runTimeCounterValue, bufferTCB);
            writeToLoggerTrace(bufferStr);
            break;
        case 1:
            sprintf(bufferStr, "%lu\t[IN]\t%s", runTimeCounterValue, bufferTCB);
            writeToLoggerTrace(bufferStr);
            break;
        case 2:
            sprintf(bufferStr, "%lu\t[QSF]\t%s", runTimeCounterValue, bufferTCB);
            writeToLoggerTrace(bufferStr);
            break;
        case 3:
            sprintf(bufferStr, "%lu\t[QRF]\t%s", runTimeCounterValue, bufferTCB);
            writeToLoggerTrace(bufferStr);
            break;
        case 4:
            sprintf(bufferStr, "%lu\t[SIF]\t%s", runTimeCounterValue, bufferTCB);
            writeToLoggerTrace(bufferStr);
            break;
        case 5:
            sprintf(bufferStr, "%lu\t[RIF]\t%s", runTimeCounterValue, bufferTCB);
            writeToLoggerTrace(bufferStr);
            break;
        default:
            printf("Trace Hook macro called logger with an invalid argument\n");
            break;
    }
}

void writeToLoggerTrace(signed char * strToWrite){
    if(index < TRACELEN - 1){
        strcpy(loggerTrace[index], strToWrite);
        ++index;
    } else {
        for(int i = 0; i <= TRACELEN-2; ++i){
            strcpy(loggerTrace[i], loggerTrace[i+1]);
        }
        strcpy(loggerTrace[index], strToWrite);
    }
    // Debug
    /*
    printf("#####################################################################\n");
    for(int i = 0; i<TRACELEN; ++i){
        printf("%d\t", i);
        printf(loggerTrace[i]);
        printf("\n");
    }   */
}

void printTrace(){
    printf("#####################################################################\n");
    for(int i = 0; i<TRACELEN; ++i){
        printf("%d\t", i);
        printf(loggerTrace[i]);
        printf("\n");
    }
}