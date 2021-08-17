#ifndef LOGGING_UTILS
#define LOGGING_UTILS
#define LENBUF 1024
#define TRACELEN 10

extern signed char loggerTrace[TRACELEN][LENBUF];

void loggingFunction(int logCause);
void writeToLoggerTrace(signed char * strToWrite);

#endif