#pragma once

#define DEBUG
#undef DEBUG
#define OUTPUT_VERBOSE
#undef OUTPUT_VERBOSE

#define GOLDEN_FILE_PATH "golden.txt"
int isGolden;

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

#ifdef POSIX 
#include <unistd.h>
#endif

/* Platform dependent ASM utilities */
#include "asm.h"

#include "injector.h"
#include "fork.h"
#include "thread.h"
#include "loggingUtils.h"
#include "sleep.h"

#define CMD_LIST "--list"
#define CMD_RUN "--run"
#define CMD_CAMPAIGN "--campaign"
#define CMD_GOLDEN "--golden"

// exit codes:
#define SUCCESSFUL_EXECUTION_EXIT_CODE 0
#define INVALID_NUMBER_OF_PARAMETERS_EXIT_CODE 1
#define INVALID_PARAMETERS_EXIT_CODE 2
#define INJECTOR_THREAD_LAUNCH_FAILURE_EXIT_CODE 3
#define GENERIC_ERROR_EXIT_CODE 10

// FreeRTOS execution exit codes:
#define EXECUTION_RESULT_SILENT_EXIT_CODE 42
#define EXECUTION_RESULT_DELAY_EXIT_CODE 44
#define EXECUTION_RESULT_ERROR_EXIT_CODE 46
#define EXECUTION_RESULT_HANG_EXIT_CODE 48
#define EXECUTION_RESULT_CRASH_EXIT_CODE 50

#ifdef DEBUG

#ifdef POSIX
#define DEBUG_PRINT(format, ...) do {\
	char __buffer[1024];\
	sprintf(__buffer, "[DEBUG] " format, ##__VA_ARGS__);\
	write(STDOUT_FILENO, __buffer, strlen(__buffer));\
} while(0);
#else // POSIX
#define DEBUG_PRINT(format, ...) \
	fprintf(stdout, "[DEBUG] " format, ##__VA_ARGS__);
#endif // POSIX

#else // DEBUG
#define DEBUG_PRINT(...) \
	do                 \
	{                  \
	} while (0)
#endif // DEBUG

#ifdef OUTPUT_VERBOSE

#ifdef POSIX
#define OUTPUT_PRINT(format, ...) do {\
	char __buffer[1024];\
	sprintf(__buffer, "[OUTPUT] " format, ##__VA_ARGS__);\
	write(STDOUT_FILENO, __buffer, strlen(__buffer));\
} while(0);
#else // POSIX
#define OUTPUT_PRINT(format, ...) \
	fprintf(stdout, "[OUTPUT] " format, ##__VA_ARGS__);
#endif // POSIX

#else // OUTPUT_VERBOSE
#define OUTPUT_PRINT(...) \
	do                 \
	{                  \
	} while (0)
#endif // OUTPUT_VERBOSE

#ifdef POSIX
#define ERR_PRINT(format, ...) do {\
    char __buffer[1024];\
	sprintf(__buffer, "[ERROR ] " format, ##__VA_ARGS__);\
	write(STDERR_FILENO, __buffer, strlen(__buffer));\
} while(0);
#else
#define ERR_PRINT(format, ...) \
	fprintf(stderr, "[ERROR ] " format, ##__VA_ARGS__);
#endif

// useful macros
#undef min
#define min(a, b) ((a < b) ? a : b)
#undef max
#define max(a, b) ((a > b) ? a : b)
