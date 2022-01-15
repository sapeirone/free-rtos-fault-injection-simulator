#ifndef LOGGING_UTILS
#define LOGGING_UTILS
#define LENBUF 1024
#define TRACELEN 10

extern signed char loggerTrace[TRACELEN][LENBUF];

/**
 * @brief Write a log entry to the trace.
 *  
 * Valid values for the `logCause` parameter:
 *  - 0 TASK_SWITCHED_OUT
 *  - 1 TASK_SWITCHED_IN
 *  - 2 QUEUE_SEND_FAILED
 *  - 3 QUEUE_RECEIVE_FAILED
 *  - 4 QUEUE_SEND_FROM_ISR_FAILED
 *  - 5 QUEUE_RECEIVE_FROM_ISR_FAILED
 * 
 * @param logCause identifies to event to be log
 */
void loggingFunction(int logCause);

/**
 * @brief Write a string to the trace
 * 
 * @param strToWrite is the string to write
 */
void writeToLoggerTrace(signed char * strToWrite);

/**
 * @brief Print the trace
 */
void printTrace();

#endif