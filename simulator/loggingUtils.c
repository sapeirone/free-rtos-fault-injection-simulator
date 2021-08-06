#include "loggingUtils.h"
#define LENBUF 1024

void loggingFunction(int logCause){
    /*
    static signed char buffer[LENBUF];
    vTaskGetRunTimeStats(buffer);
    printf( "\nTask\t\tAbs\t\t\t%%\n" );
    printf( "-------------------------------------------------------------\n" );
    vPrintMultipleLines( buffer );
    */
    switch(logCause){
        case 0:
            printf("Case 0\n");
            break;
        case 1:
            printf("Case 1\n");
            break;
        default:
            printf("Default case\n");
            break;
    }
}