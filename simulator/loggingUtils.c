#include "loggingUtils.h"
#define LENBUF 1024

void loggingFunction(void){
    static signed char buffer[LENBUF];
    vTaskGetRunTimeStats(buffer);
    printf( "\nTask\t\tAbs\t\t\t%%\n" );
    printf( "-------------------------------------------------------------\n" );
    vPrintMultipleLines( cStringBuffer );
}