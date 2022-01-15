#include <stdio.h>
#include <string.h>
#include "loggingUtils.h"
#include "FreeRTOS.h"
#include "task.h"
#include "simulator.h"
#include "sleep.h"
#pragma warning(disable : 4996) // _CRT_SECURE_NO_WARNINGS

signed char loggerTrace[TRACELEN][LENBUF];

extern unsigned long injTime;
extern int eventIsSet;

void loggingFunction(int logCause) {
    static int didReceiveISR = 0;
    unsigned long runTimeCounterValue = ulGetRunTimeCounterValue();
    static signed char bufferTCB[LENBUF];

    // allocate a larger bufferStr to accomodate bufferTCB and something else
    static signed char bufferStr[2 * LENBUF];
    vTaskGetCurrentTCBStats(bufferTCB);

    if (didReceiveISR)
        // an event 'send|receive from isr' signals that the execution is completed
        // no need to record new events
        return;

    switch (logCause) {
    case 0: // task switched out
        sprintf(bufferStr, "%lu\t[OUT]\t%s", runTimeCounterValue, bufferTCB);
        writeToLoggerTrace(bufferStr);
        break;
    case 1: // task switched in
        sprintf(bufferStr, "%lu\t[IN]\t%s", runTimeCounterValue, bufferTCB);
        writeToLoggerTrace(bufferStr);
        break;
    case 2: // queue send failed
        sprintf(bufferStr, "%lu\t[QSF]\t%s", runTimeCounterValue, bufferTCB);
        writeToLoggerTrace(bufferStr);
        break;
    case 3: // queue receive failed
        sprintf(bufferStr, "%lu\t[QRF]\t%s", runTimeCounterValue, bufferTCB);
        writeToLoggerTrace(bufferStr);
        break;
    case 4: // queue send from isr
        sprintf(bufferStr, "%lu\t[SIF]\t%s", runTimeCounterValue, bufferTCB);
        writeToLoggerTrace(bufferStr);
        didReceiveISR++;
        break;
    case 5: // queue receive from isr
        sprintf(bufferStr, "%lu\t[RIF]\t%s", runTimeCounterValue, bufferTCB);
        writeToLoggerTrace(bufferStr);
        didReceiveISR++;
        break;
    default:
        printf("Trace Hook macro called logger with an invalid argument\n");
        break;
    }
}

void writeToLoggerTrace(signed char *strToWrite) {
    static int index = 0;

    if (index < TRACELEN - 1) {
        strcpy(loggerTrace[index], strToWrite);
        ++index;
    } else {
        for (int i = 0; i <= TRACELEN - 2; ++i) {
            strcpy(loggerTrace[i], loggerTrace[i + 1]);
        }
        strcpy(loggerTrace[index], strToWrite);
    }
}

void printTrace()
{
    OUTPUT_PRINT("###################################################################\n"); // Nice
    for (int i = 0; i < TRACELEN; ++i) {
        OUTPUT_PRINT("%d\t%s\n", i, loggerTrace[i]);
    }
}