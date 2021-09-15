#pragma once

#define DEBUG
// #undef DEBUG

#define GOLDEN_FILE_PATH "golden.txt"
int isGolden;

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
#define INJECTOR_THREAD_LAUNCH_FAILURE_EXIT_CODE 2
#define GENERIC_ERROR_CODE 42

#ifdef DEBUG
#define DEBUG_PRINT(format, ...) \
    printf("[DEBUG] " format, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(x) \
	do                 \
	{                  \
	} while (0)
#endif