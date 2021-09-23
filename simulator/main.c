/*
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/******************************************************************************
 * This project provides two demo applications.  A simple blinky style project,
 * and a more comprehensive test and demo application.  The
 * mainCREATE_SIMPLE_BLINKY_DEMO_ONLY setting is used to select between the two.
 * The simply blinky demo is implemented and described in main_blinky.c.  The
 * more comprehensive test and demo application is implemented and described in
 * main_full.c.
 *
 * This file implements the code that is not demo specific, including the
 * hardware setup and FreeRTOS hook functions.
 *
 *******************************************************************************
 */

/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#pragma warning(disable : 4996) // _CRT_SECURE_NO_WARNINGS

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "sleep.h"

#include "simulator.h"
#include "benchmark/benchmark.h"

extern struct myStringStruct array[MAXARRAY];
extern signed char loggerTrace[TRACELEN][LENBUF];

/* Global variables */
extern int isGolden;
extern int eventIsSet;

/* This demo uses heap_5.c, and these constants define the sizes of the regions
that make up the total heap.  heap_5 is only used for test and example purposes
as this demo could easily create one large heap region instead of multiple
smaller heap regions - in which case heap_4.c would be the more appropriate
choice.  See http://www.freertos.org/a00111.html for an explanation. */
#define mainREGION_1_SIZE 8201
#define mainREGION_2_SIZE 29905
#define mainREGION_3_SIZE 7807

/*-----------------------------------------------------------*/

extern void mainSetup(void);
extern void mainRun(void);

/*
 * This demo uses heap_5.c, so start by defining some heap regions.  It is not
 * necessary for this demo to use heap_5, as it could define one large heap
 * region.  Heap_5 is only used for test and example purposes.  See
 * http://www.freertos.org/a00111.html for an explanation.
 */
static void prvInitialiseHeap(void);

/*
 * Prototypes for the standard FreeRTOS application hook (callback) functions
 * implemented within this file.  See http://www.freertos.org/a00016.html .
 */
void vApplicationMallocFailedHook(void);
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName);
void vApplicationTickHook(void);
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize);
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize);

/*-----------------------------------------------------------*/

/* When configSUPPORT_STATIC_ALLOCATION is set to 1 the application writer can
use a callback function to optionally provide the memory required by the idle
and timer tasks.  This is the stack that will be used by the timer task.  It is
declared here, as a global, so it can be checked by a test that is implemented
in a different file. */
StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

/* Notes if the trace is running or not. */
static BaseType_t xTraceRunning = pdTRUE;
unsigned long injTime = -1;

static void printApplicationArguments(int argc, char **argv);

static void printInjectionTarget(FILE *output, target_t *target, int depth);

/**
 * Scan the injection targets list and return the matching one.
 * The returned value is allocated on the heap and must be properly
 * freed after use.
 * 
 * Example of valid toSearch strings:
 *  - xYieldPending
 *  - pxCurrentTCB.uxPriority
 * 
 * @param target_t *target is the list of available targets
 * @param char *toSearch is the query string to look for
 * 
 * @return an injection target (or NULL otherwise).
 */
thData_t *getInjectionTarget(target_t *target, const char *toSearch);

static void runSimulator(const thData_t *injectionArgs);
static void writeGoldenFile();

static void execCmdList(int argc, char **argv);
static void execCmdRun(int argc, char **argv);
static void execCmdGolden(int argc, char **argv);
static void execInjectionCampaign(int argc, char **argv);

static int readGoldenExecutionTime(unsigned long *value);
static int readInjectionCampaignList(const char *filename, injectionCampaign_t **campaignList);

static int traceOutputIsCorrect();
static int executionResultIsCorrect();

static void printProgressBar(double percentage);
static void printMany(FILE *fp, char c, int number);
static void printStatistics(injectionCampaign_t *injectionCampaigns, int nInjectionCampaigns);

/**
 * List of injection targets for the current instance of the 
 * FreeRTOS simulator.
 * The list is initialized in main.
 */
static target_t *targets;

/*-----------------------------------------------------------*/

int main(int argc, char **argv)
{
	setbuf(stdout, 0);
	setbuf(stderr, 0);

	// print the application arguments for debugging purposes
	printApplicationArguments(argc, argv);

	if (argc < 2)
	{
		// at least one argument is expected
		ERR_PRINT("Please specify a command argument --(list|run|golden)\n");
		return INVALID_NUMBER_OF_PARAMETERS_EXIT_CODE;
	}

	/**
	 * Read the available injection targets from the tasks and timer modules.
	 * 
	 * TODO: possibly add more injection targets.
	 */
	targets = read_tasks_targets(NULL);
	targets = read_timer_targets(targets);

	if (strcmp(argv[1], CMD_LIST) == 0)
	{
		execCmdList(argc, argv);
	}
	else if (strcmp(argv[1], CMD_RUN) == 0)
	{
		execCmdRun(argc, argv);
	}
	else if (strcmp(argv[1], CMD_GOLDEN) == 0)
	{
		execCmdGolden(argc, argv);
	}
	else if (strcmp(argv[1], CMD_CAMPAIGN) == 0)
	{
		// TODO: option --j number of parallel instances of FreeRTOS + Injector to run simultaneously
		execInjectionCampaign(argc, argv);
	}

	return SUCCESSFUL_EXECUTION_EXIT_CODE;
}
/*-----------------------------------------------------------*/

static void execCmdList(int argc, char **argv)
{
	FILE *output = stdout;

	if (argc == 3)
	{
		/**
		 * Open the target output file
		 */
		output = fopen(argv[2], "w");
		if (output == NULL)
		{
			char buf[256];
			sprintf(buf, "Error opening the target file %s", argv[2]);
			perror(buf);

			exit(INVALID_NUMBER_OF_PARAMETERS_EXIT_CODE);
		}
	}

	printInjectionTarget(output, targets, 0);

	freeInjectionTargets(targets);
	exit(SUCCESSFUL_EXECUTION_EXIT_CODE);
}

static void execCmdRun(int argc, char **argv)
{
	if (argc != 6)
	{
		ERR_PRINT("Invalid number of arguments for %s.\n", CMD_RUN);
		exit(INVALID_NUMBER_OF_PARAMETERS_EXIT_CODE);
	}

	// read the golden runtime from the golden file
	FILE *golden = fopen(GOLDEN_FILE_PATH, "r");
	if (!golden)
	{
		ERR_PRINT("%s not found\n", GOLDEN_FILE_PATH);
		exit(GENERIC_ERROR_EXIT_CODE);
	}

	unsigned long goldenExecTime;
	if (fscanf(golden, "%lu", &goldenExecTime) != 1)
	{
		ERR_PRINT("Cannot read the golden execution time\n");

		fclose(golden);
		exit(GENERIC_ERROR_EXIT_CODE);
	}
	DEBUG_PRINT("Execution timeout is %lu\n", goldenExecTime);

	// TODO: replace with strol (atol does NOT detect errors)
	injTime = atol(argv[3]);
	unsigned long offsetByte = atol(argv[4]);
	unsigned long offsetBit = atol(argv[5]);

	thData_t *injection = getInjectionTarget(targets, argv[2]);

	if (!injection)
	{
		ERR_PRINT("Cannot find the injection target %s\n", argv[2]);

		fclose(golden);
		exit(GENERIC_ERROR_EXIT_CODE);
	}

	// create a wrapper for the injection parameters
	injection->injTime = injTime;
	injection->offsetByte = offsetByte;
	injection->offsetBit = offsetBit;
	injection->timeoutNs = 3 * goldenExecTime;

	runSimulator(injection);

	free(injection);
}

static void execCmdGolden(int argc, char **argv)
{
	if (argc != 2)
	{
		ERR_PRINT("The --golden command does not support additional parameters");
		exit(INVALID_NUMBER_OF_PARAMETERS_EXIT_CODE);
	}

	// run the simulator without specifying an injection target
	runSimulator(NULL);
}

/**
 * Execute the --campaign command.
 * 
 * Expected parameters:
 * ./sim --campaign /path/to/input/file.csv [-y] [--no-pg-bar]
 */
static void execInjectionCampaign(int argc, char **argv)
{
	if (argc < 3 || argc > 5)
	{
		ERR_PRINT("Invalid number of arguments for %s.\n", CMD_CAMPAIGN);
		exit(INVALID_NUMBER_OF_PARAMETERS_EXIT_CODE);
	}

	const char *input = argv[2];
	char choice = '0';
	int pgBarEnabled = 1;

	if ((argc > 3 && strcmp(argv[3], "-y") == 0) || (argc > 4 && strcmp(argv[4], "-y") == 0))
	{
		choice = 'y';
	}
	if ((argc > 3 && strcmp(argv[3], "--no-pg-bar") == 0) || (argc > 4 && strcmp(argv[4], "--no-pg-bar") == 0))
	{
		pgBarEnabled = 0;
	}

	/**
	 * Read from file the injection details which is the target structure, 
	 * how many injections have to be tested, the median of the time range, 
	 * the variance over the time range and the distribution (by default a 
	 * uniform distribution).
	 * 
	 * Prototype of the input .csv:
	 *   char * targetStructure, int nInjections, double medTimeRange, double variance, char * distr
	 * 
	 * Returns a list of struct injection campaigns.
	*/
	injectionCampaign_t *injectionCampaigns;
	int nInjectionCampaigns = readInjectionCampaignList(argv[2], &injectionCampaigns);

	unsigned long nTotalInjections = 0;
	double estTotTimeMin = 0.0, estTotTimeMax = 0.0;

	unsigned long nanoGoldenEx = 0;
	if (readGoldenExecutionTime(&nanoGoldenEx) != 0)
	{
		ERR_PRINT("Couldn't open golden execution results file.\n");
		exit(GENERIC_ERROR_EXIT_CODE);
	}

	/**
	 * Print out an time extimation of minimum and maximum execution 
	 * times for the whole simulation.
	 * 
	 * The user must input (Y/n) to confirm execution.
	 */
	fprintf(stdout, "\nEstimated execution times:\n");

	printMany(stdout, '-', 117);
	fprintf(stdout, "\n| %-30s | %10s | %8s | %10s | %5s | %5s | %12s | %12s |\n",
			"Target", "Time (ns)", "nExecs", "tMed", "var", "distr", "estTimeMin", "estTimeMax");

	for (int i = 0; i < nInjectionCampaigns; ++i)
	{
		double estTimeMin = (1.0 * injectionCampaigns[i].nInjections * nanoGoldenEx) / (1000.0 * 1000.0 * 1000.0);
		//300% of golden execution time, for each injection in the campaign
		double estTimeMax = estTimeMin * 3.0;

		printMany(stdout, '-', 117);
		fprintf(stdout, "\n| %-30s | %10d | %8d | %10lu | %5lu | %5c | %10.2f s | %10.2f s |\n",
				injectionCampaigns[i].targetStructure,
				injectionCampaigns[i].medTimeRange,
				injectionCampaigns[i].nInjections,
				injectionCampaigns[i].medTimeRange,
				injectionCampaigns[i].variance,
				injectionCampaigns[i].distribution,
				estTimeMin, estTimeMax);

		estTotTimeMin += estTimeMin;
		estTotTimeMax += estTimeMax;
		nTotalInjections += injectionCampaigns[i].nInjections;
	}

	printMany(stdout, '-', 117);
	fprintf(stdout, "\n%-30s     %10s    %8s   %10s   %5s   %5s   %10.2f s   %10.2f s \n\n",
			"Total estimated time", "-", "-", "-", "-", "-", estTotTimeMin, estTotTimeMax);

	// require user confirmation
	while (choice != 'y' && choice != 'n')
	{
		fprintf(stdout, "Continue? (y|n): ");
		fscanf(stdin, "%c", &choice);
		if (choice == 'n')
		{
			fprintf(stdout, "Aborting...");
			return;
		}
	}

	/**
	 * For each line in the input .csv, generate a injection campaign.
	 * A for loop launches the iFork() of the forefather. 
	 * Each father (son of the forefather), launches a thread instance of the FreeRTOS + Injector. 
	 * The forefather waits for the father: if the return value of the wait is different from 0, the 
	 * forefather adds 1 to the "crash" entry for that campaign. 
	 * Each father awaits the 300% golden execution time and then reads the trace, unless the 
	 * FreeRTOS returned by itself sooner. The father decides which termination has been performed 
	 * and increases the relative statistic in the memory mapped file for that campaign.
	 * Completing injection campaigns advances a general completion bar.
	 */

	// initialize the random seed
	srand((unsigned int)time(NULL));
	int nCurrentInjection = 0;

	for (int i = 0; i < nInjectionCampaigns; ++i)
	{
		// For each injection campaign
		injectionCampaign_t *campaign = injectionCampaigns + i;
		memset(&campaign->res, 0, sizeof(injectionResults_t));

		for (int j = 0; j < campaign->nInjections; ++j)
		{
			// For each injection in a campaign
			nCurrentInjection++;
			DEBUG_PRINT("Running injection n. %lu/%lu...\n", nCurrentInjection, nTotalInjections);

			if (pgBarEnabled)
			{
				/* Progress bar */
				printProgressBar(((double)nCurrentInjection / nTotalInjections));
			}

			thData_t *inj = getInjectionTarget(targets, campaign->targetStructure);
			if (inj == NULL)
			{
				ERR_PRINT("No target with name %s was found.\n", campaign->targetStructure);
				exit(GENERIC_ERROR_EXIT_CODE);
			}

			target_t *injTarget = inj->target;

			// verify the median injection time does not exceed the
			// execution time of the golden simulation
			if (campaign->medTimeRange > nanoGoldenEx)
			{
				ERR_PRINT("Invalid injection time for target %s\n", campaign->targetStructure);
				exit(GENERIC_ERROR_EXIT_CODE);
			}

			unsigned long offsetByte;
			if (inj->isList) {
				offsetByte = rand() % sizeof(ListItem_t); //select byte to inject
				DEBUG_PRINT("Selecting target as list item");
			}
			else if (IS_TYPE_POINTER(inj->target->type) && !inj->isPointer)
			{
				offsetByte = rand() % (sizeof(char *)); //select byte to inject
				DEBUG_PRINT("2: %d", sizeof(char *));
			}
			else
			{
				offsetByte = rand() % injTarget->size; //select byte to inject
				DEBUG_PRINT("1: %d", injTarget->size);
			}

			unsigned long offsetBit = rand() % 8;				 //select bit to inject
			unsigned long injTime;

			double total = 0;

			// pick a distribution
			switch (campaign->distribution)
			{
			case 'g':

				//gaussian distribution approximated starting from the Irwin-Hall distribution
				for (int gaussian = 0; gaussian < 12; ++gaussian)
				{
					total += rand() % 1000;
				}
				total = (total - 6000) / 1000;

				injTime = total * campaign->variance / 6 + campaign->medTimeRange;

				if(injTime<0){
					injTime=injTime+campaign->variance/2;
				}

				break;

			case 't':

				//triangular distribution
				for (int gaussian = 0; gaussian < 12; ++gaussian)
				{
					total += rand() % 1000;
				}
				total = (total - 1000) / 1000;

				injTime = total * campaign->variance + campaign->medTimeRange;

				if(injTime<0){
					injTime=injTime+campaign->variance/2;
				}

				break;

			case 'u':
			default:
				injTime = campaign->medTimeRange;

				// compute the width of the injection time range
				int lowerWidth = min(campaign->medTimeRange, campaign->variance);
				int upperWidth = min(campaign->variance, nanoGoldenEx - campaign->medTimeRange);
				int range = max(1, lowerWidth + upperWidth);
				injTime = (rand() % range) - lowerWidth + (signed)campaign->medTimeRange;
			}

			freeRTOSInstance instance;
			int ret = runFreeRTOSInjection(&instance, argv[0], campaign->targetStructure, injTime, offsetByte, offsetBit);
			if (ret < 0)
			{
				ERR_PRINT("Couldn't create child process.\n");
				exit(GENERIC_ERROR_EXIT_CODE);
			}

			// Father process
			unsigned int exitCode = waitFreeRTOSInjection(&instance);
			DEBUG_PRINT("Injection n. %lu/%lu completed with exit code %u...\n\n", nCurrentInjection, nTotalInjections, exitCode);

			switch (exitCode)
			{
			case EXECUTION_RESULT_HANG_EXIT_CODE:
				campaign->res.nHang++;
				break;
			case EXECUTION_RESULT_ERROR_EXIT_CODE:
				campaign->res.nError++;
				break;
			case EXECUTION_RESULT_DELAY_EXIT_CODE:
				campaign->res.nDelay++;
				break;
			case EXECUTION_RESULT_SILENT_EXIT_CODE:
				campaign->res.nSilent++;
				break;
			case EXECUTION_RESULT_CRASH_EXIT_CODE:
			default:
				// printf("%u\n", exitCode);
				campaign->res.nCrash++;
			}
		}
	}

	printStatistics(injectionCampaigns, nInjectionCampaigns);
}

void vApplicationMallocFailedHook(void)
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c, heap_2.c or heap_4.c is being used, then the
	size of the	heap available to pvPortMalloc() is defined by
	configTOTAL_HEAP_SIZE in FreeRTOSConfig.h, and the xPortGetFreeHeapSize()
	API function can be used to query the size of free heap space that remains
	(although it does not provide information on how the remaining heap might be
	fragmented).  See http://www.freertos.org/a00111.html for more
	information. */
	vAssertCalled(__LINE__, __FILE__);
}
/*-----------------------------------------------------------*/

extern int mustEnd;

void vApplicationIdleHook(void)
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If application tasks make use of the
	vTaskDelete() API function to delete themselves then it is also important
	that vApplicationIdleHook() is permitted to return to its calling function,
	because it is the responsibility of the idle task to clean up memory
	allocated by the kernel to any task that has since deleted itself. */

	/* Uncomment the following code to allow the trace to be stopped with any
	key press.  The code is commented out by default as the kbhit() function
	interferes with the run time behaviour. */
	/*
		if( _kbhit() != pdFALSE )
		{
			if( xTraceRunning == pdTRUE )
			{
				vTraceStop();
				prvSaveTraceFile();
				xTraceRunning = pdFALSE;
			}
		}
	*/

	/* If the only task remaining is the IDLE task, terminate the scheduler */
	if (isIdleHighlander())
	{
		if (isGolden)
		{
			writeGoldenFile();
		}
		vPortGenerateSimulatedInterrupt(5);
		vTaskEndScheduler();
		ERR_PRINT("Executing past vTaskEndScheduler.\n"); // Never executed
	}
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
	(void)pcTaskName;
	(void)pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected.  This function is
	provided as an example only as stack overflow checking does not function
	when running the FreeRTOS Windows port. */
	vAssertCalled(__LINE__, __FILE__);
}
/*-----------------------------------------------------------*/

void vApplicationTickHook(void)
{
	/* This function will be called by each tick interrupt if
	configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
	added here, but the tick hook is called from an interrupt context, so
	code must not attempt to block, and only the interrupt safe FreeRTOS API
	functions can be used (those that end in FromISR()). */

#ifdef WIN32
	int eventIsSet = 1;

	if ((!isGolden) && (eventIsSet == 1) && (ulGetRunTimeCounterValue() >= injTime))
	{
		//fprintf(stdout, "isGolden = %d eventIsSet = %d runTimeCounterValue = %lu injTime = %lu\n", isGolden, eventIsSet, runTimeCounterValue, injTime);
		wakeInjector();
	}
#endif
}
/*-----------------------------------------------------------*/

void vApplicationDaemonTaskStartupHook(void)
{
	/* This function will be called once only, when the daemon task starts to
	execute	(sometimes called the timer task).  This is useful if the
	application includes initialisation code that would benefit from executing
	after the scheduler has been started. */
}
/*-----------------------------------------------------------*/

void vAssertCalled(unsigned long ulLine, const char *const pcFileName)
{
	static BaseType_t xPrinted = pdFALSE;
	volatile uint32_t ulSetToNonZeroInDebuggerToContinue = 0;

	/* Called if an assertion passed to configASSERT() fails.  See
	http://www.freertos.org/a00110.html#configASSERT for more information. */

	/* Parameters are not used. */
	(void)ulLine;
	(void)pcFileName;

	// printf("ASSERT! Line %ld, file %s, GetLastError() %ld\r\n", ulLine, pcFileName, GetLastError());
	ERR_PRINT("ASSERT! Line %ld, file %s\r\n", ulLine, pcFileName);

	exit(EXECUTION_RESULT_CRASH_EXIT_CODE);
}
/*-----------------------------------------------------------*/

static void prvInitialiseHeap(void)
{
	/* The Windows demo could create one large heap region, in which case it would
	be appropriate to use heap_4.  However, purely for demonstration purposes,
	heap_5 is used instead, so start by defining some heap regions.  No
	initialisation is required when any other heap implementation is used.  See
	http://www.freertos.org/a00111.html for more information.

	The xHeapRegions structure requires the regions to be defined in start address
	order, so this just creates one big array, then populates the structure with
	offsets into the array - with gaps in between and messy alignment just for test
	purposes. */
	static uint8_t ucHeap[configTOTAL_HEAP_SIZE];
	volatile uint32_t ulAdditionalOffset = 19; /* Just to prevent 'condition is always true' warnings in configASSERT(). */
	const HeapRegion_t xHeapRegions[] =
		{
			/* Start address with dummy offsets						Size */
			{ucHeap + 1, mainREGION_1_SIZE},
			{ucHeap + 15 + mainREGION_1_SIZE, mainREGION_2_SIZE},
			{ucHeap + 19 + mainREGION_1_SIZE + mainREGION_2_SIZE, mainREGION_3_SIZE},
			{NULL, 0}};

	/* Sanity check that the sizes and offsets defined actually fit into the
	array. */
	configASSERT((ulAdditionalOffset + mainREGION_1_SIZE + mainREGION_2_SIZE + mainREGION_3_SIZE) < configTOTAL_HEAP_SIZE);

	/* Prevent compiler warnings when configASSERT() is not defined. */
	(void)ulAdditionalOffset;

	vPortDefineHeapRegions(xHeapRegions);
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
	/* If the buffers to be provided to the Idle task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
	static StaticTask_t xIdleTaskTCB;
	static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

	/* Pass out a pointer to the StaticTask_t structure in which the Idle task's
	state will be stored. */
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

	/* Pass out the array that will be used as the Idle task's stack. */
	*ppxIdleTaskStackBuffer = uxIdleTaskStack;

	/* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
	Note that, as the array is necessarily of type StackType_t,
	configMINIMAL_STACK_SIZE is specified in words, not bytes. */
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
{
	/* If the buffers to be provided to the Timer task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
	static StaticTask_t xTimerTaskTCB;

	/* Pass out a pointer to the StaticTask_t structure in which the Timer
	task's state will be stored. */
	*ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

	/* Pass out the array that will be used as the Timer task's stack. */
	*ppxTimerTaskStackBuffer = uxTimerTaskStack;

	/* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
	Note that, as the array is necessarily of type StackType_t,
	configMINIMAL_STACK_SIZE is specified in words, not bytes. */
	*pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

static void printInjectionTarget(FILE *output, target_t *target, int depth)
{
	char typeBuffer[256];

	// Format the target type (example: "POINTER | LIST")
	pretty_print_target_type(target->type, typeBuffer);

	for (int i = 0; i < depth; i++)
	{
		fprintf(output, i == 0 ? "|--" : "--");
	}

	const char *fmt = "%-30s (address: 0x%08x, size: %2d B, nmemb: %d, type: %s)\n";
	fprintf(output, fmt, target->name, target->address, target->size, target->nmemb, typeBuffer);

	// recursively call printInjectionTarget over the target's children, if any
	if (target->content)
	{
		printInjectionTarget(output, target->content, depth + 1);
	}

	// recursively call printInjectionTarget over the next target
	if (target->next)
	{
		printInjectionTarget(output, target->next, depth);
	}
}

thData_t *getInjectionTarget(target_t *list, const char *targetName)
{
	if (!list || !targetName) {
		// invalid parameters
		return NULL;
	}
	
	char *_targetName = strdup(targetName);
	// split the query string and extract parent and child references
	char *parentNode = NULL, *childNode = NULL;
	parentNode = strtok_s(_targetName, ".", &childNode);

	if (!parentNode)
	{
		// invalid toSearch string
		free(_targetName);
		return NULL;
	}

	char *rest = NULL, *tok = NULL;
	int index1 = INT_MIN, index2 = INT_MIN;
	int parentIsDereference = 0, childIsDereference = 0;
	
	// check if the parent node is expressed as a dereference 
	if (parentNode[0] == '*') {
		parentIsDereference = 1;
		parentNode++; // skip the first character
	}

	parentNode = strtok_s(parentNode, "[", &rest);

	if (*rest != '\0' && rest != NULL)
	{
		sscanf(rest, "%d", &index1);
		tok = strtok_s(rest, "[", &rest);
	}

	if (*rest != '\0' && rest != NULL)
		sscanf(rest, "%d", &index2);

	if (childNode != NULL && childNode[0] != '\0')
	{
		childNode = strtok_s(childNode, "[", &rest);
		if (*rest != '\0' && rest != NULL)
			sscanf(rest, "%d", &index2);

		// check if the child node is expressed as a dereference 
		if (childNode[0] == '*') {
			childIsDereference = 1;
			childNode++; // skip the first character
		}
	}

	thData_t *data = (thData_t*) malloc(sizeof(thData_t));
	memset(data, 0, sizeof(thData_t));

	target_t *tmp = list;
	while (tmp)
	{
		if (strcmp(tmp->name, parentNode) == 0)
		{
			// the strings are matching

			if (*childNode && !IS_TYPE_STRUCT(tmp->type)) {
				// child node specified but tmp is not a father injection target
				// example: xDelayedTaskList1->my_field is NOT valid
				ERR_PRINT("Invalid injection target %s\n", targetName);
				break;
			}

			if (parentIsDereference && !IS_TYPE_POINTER(tmp->type))
			{
				// invalid state: attempt at dereferencing a non-pointer target
				// example: *pxReadyTasksLists is not valid
				ERR_PRINT("Invalid injection target: attempt at dereferencing a non-pointer target %s\n", targetName);
				break;
			}

			data->address = tmp->address; // compute the final injection address

			if (parentIsDereference ||
				(IS_TYPE_POINTER(tmp->type) && IS_TYPE_STRUCT(tmp->type) && *childNode))
			{
				// example: *pxDelayedTaskList and *pxCurrentTCB are valid
				// example: pxCurrentTCB.pxStack is valid since pxCurrentTCB points to a struct and a child is specified
				// address = (unsigned long)*((void **)tmp->address);
				data->isPointer = 1;
			}

			if (IS_TYPE_ARRAY(tmp->type))
			{
				// tmp is an array
				if (index1 >= 0) {
					// select item in position index1
					data->address = (void*) (((unsigned long) data->address) + (tmp->size * index1));
				} else if (index1 == -1) {
					// randomly select a target inside the array
					data->address = (void*) (((unsigned long) data->address) + (tmp->size * (rand() % tmp->nmemb)));
				}
			}

			if (!(*childNode))
			{
				// no child reference specified => return the current node
				if (IS_TYPE_LIST(tmp->type) &&
					((IS_TYPE_ARRAY(tmp->type) && index2 >= -1) || (!IS_TYPE_ARRAY(tmp->type) && index1 >= -1))) {

					data->isList = 1;
					if (IS_TYPE_ARRAY(tmp->type))
					{
						data->listPosition = max(index2, -1);
					}
					else
					{
						data->listPosition = max(index1, -1);
					}
				}

				data->address = (void*) data->address;
				data->target = tmp;

				free(_targetName);
				return data;
			}

			// child reference specified => look for a matching child node
			for (target_t *child = tmp->content; child; child = child->next)
			{
				if (strcmp(child->name, childNode) == 0)
				{
					unsigned long innerAddress = (unsigned long) child->address;
					
					if (IS_TYPE_ARRAY(child->type) && index2 >= 0)
					{
						// tmp is an array
						if (index2 >= 0) {
							// select item in position index1
							innerAddress = innerAddress + (child->size * index2);
						} else if (index2 == -1) {
							// randomly select a target inside the array
							innerAddress = innerAddress + (child->size * (rand() % child->nmemb));
						}
					}

					if (data->isPointer) {
						data->offset = (void*) innerAddress;
					} else {
						data->address = (void*) ((unsigned long) data->address + innerAddress);
					}

					data->target = child;

					free(_targetName);
					return data;
				}
			}
		}

		// iterate
		tmp = tmp->next;
	}

	// no target found
	free(_targetName);
	return NULL;
}

static void printApplicationArguments(int argc, char **argv)
{
	char buffer[1024];

	sprintf(buffer, "%s", "Application arguments: ");
	for (int i = 0; i < argc; i++)
	{
		sprintf(buffer, "%s %s", buffer, argv[i]);
	}

	DEBUG_PRINT("%s \n", buffer);
}

static void runSimulator(const thData_t *injectionArgs)
{
	/* Launch the FreeRTOS */
	prvInitialiseHeap();

	DEBUG_PRINT("Calling mainSetup...\n");
	mainSetup();
	DEBUG_PRINT("Call to mainSetup completed\n");

	if (injectionArgs)
	{
		// the simulation should perform an injection

		// create the injection thread
		thread_t injectorThread;
		int resultCode = launchInjectorThread(&injectorFunction, injectionArgs, &injectorThread);

		if (resultCode == INJECTOR_THREAD_FAILURE)
		{
			ERR_PRINT("Injector thread launch failure.\n");
			exit(INJECTOR_THREAD_LAUNCH_FAILURE_EXIT_CODE);
		}

		// detach the injector thread
		detachThread(&injectorThread);
	}
	else
	{
		isGolden = 1;
	}

	DEBUG_PRINT("Calling mainRun...\n");
	mainRun();
	DEBUG_PRINT("Call to mainRun completed\n");

	if (isGolden)
		exit(EXIT_SUCCESS);

	/**
	 * Check trace and determine the outcome of the simulation.
	 * Refer to simulator.h for the exit codes values.
	 */
	int result = 50;
	unsigned long nanoGoldenEx = 0, execTime = 0;
	nanoGoldenEx = injectionArgs->timeoutNs / 3;

	sscanf(loggerTrace[TRACELEN - 1], "%lu", &execTime);

	/*
	printf( "###################################################################\n" ); // Nice
    for(int i = 0; i<TRACELEN; ++i){
        printf( "%d\t%s\n", i, loggerTrace[i] );
    }*/

	if (traceOutputIsCorrect())
	{ // Correct Trace output, ISR worked
		if (executionResultIsCorrect())
		{										  // Execution result is correct
			if (execTime < (1.05 * nanoGoldenEx)) // Silent execution, correct output
			{
				exit(EXECUTION_RESULT_SILENT_EXIT_CODE);
			}
			else // Delayed execution, correct output
			{
				exit(EXECUTION_RESULT_DELAY_EXIT_CODE);
			}
		}
		else // Execution result is not correct
		{
			if (execTime < (1.05 * nanoGoldenEx)) // Error execution, incorrect output
			{
				exit(EXECUTION_RESULT_ERROR_EXIT_CODE);
			}
			else // Hang execution, incorrect output
			{
				exit(EXECUTION_RESULT_HANG_EXIT_CODE);
			}
		}
	}
	else // Incorrect Trace output, ISR didn't work
	{
		exit(EXECUTION_RESULT_CRASH_EXIT_CODE);
	}

	/* This should never be executed */
	ERR_PRINT("BIG DANGER!\nSomehow, the execution of the RTOS produced an unexpected output.\n");
	exit(EXIT_FAILURE);
}

static int traceOutputIsCorrect()
{
	char *tmpBuffer[4];

	char *rest;
	char *token = strtok_s(loggerTrace[TRACELEN - 2], "\t", &rest);
	tmpBuffer[0] = strtok_s(rest, "\t", &rest);
	tmpBuffer[1] = strtok_s(rest, "\t", &rest);

	token = strtok_s(loggerTrace[TRACELEN - 1], "\t", &rest);
	tmpBuffer[2] = strtok_s(rest, "\t", &rest);
	tmpBuffer[3] = strtok_s(rest, "\t", &rest);

	return (/*strncmp(tmpBuffer[0], "[IN]", 4) == 0 && strncmp(tmpBuffer[1], "IDLE", 4) == 0 &&*/
			strncmp(tmpBuffer[2], "[RIF]", 5) == 0 && strncmp(tmpBuffer[3], "IDLE", 4) == 0);
}

static int executionResultIsCorrect()
{
	int result = 1;
	FILE *goldenfp = fopen(GOLDEN_FILE_PATH, "r");
	if (goldenfp == NULL)
	{
		ERR_PRINT("Couldn't open %s for reading.\n", GOLDEN_FILE_PATH);
		exit(EXIT_FAILURE);
	}

	char buffer[LENBUF];
	fscanf(goldenfp, "%s\n", buffer); // Ingore first line, the goldenExecutionTime
	for (int i = 0; i < MAXARRAY; i++)
	{
		fscanf(goldenfp, "%s\n", buffer);
		if (strcmp(buffer, array[i].qstring) != 0)
		{
			result = 0;
			break;
		}
	}
	fclose(goldenfp);

	return result;
}

static void writeGoldenFile()
{
	unsigned long goldenTime = ulGetRunTimeCounterValue();
	DEBUG_PRINT("Golden execution time: %lu.\n", goldenTime);

	FILE *goldenfp = fopen(GOLDEN_FILE_PATH, "w");
	if (goldenfp == NULL)
	{
		ERR_PRINT("Couldn't open %s for writing.\n", GOLDEN_FILE_PATH);
		exit(EXIT_FAILURE);
	}

	fprintf(goldenfp, "%lu\n", goldenTime);
	for (int i = 0; i < MAXARRAY; i++)
	{
		fprintf(goldenfp, "%s\n", array[i].qstring);
	}
	fclose(goldenfp);
}

/**
 * Read from file the injection details which is the target structure, how many injections have 
 * to be tested, the median of the time range, the variance over the time range and the distribution 
 * (by default a Gaussian).
 * 
 * Prototype of the input .csv:
 *     char * targetStructure, int nInjections, double medTimeRange, double variance, char * distr
 * 
 * Returns the number of injection campaigns.
 */
static int readInjectionCampaignList(const char *filename, injectionCampaign_t **list)
{
	int index = 0;
	char icBuffer[LENBUF];

	FILE *inputCampaign = fopen(filename, "r");
	if (inputCampaign == NULL)
	{
		ERR_PRINT("Couldn't open input file input.csv.\n");
		return 1;
	}

	*list = (injectionCampaign_t *)malloc(0);
	while (fgets(icBuffer, LENBUF - 1, inputCampaign) != NULL)
	{
		if (icBuffer[0] == '#')
		{
			continue;
		}

		*list = (injectionCampaign_t *)realloc(*list, (index + 1) * sizeof(injectionCampaign_t));

		injectionCampaign_t *campaign = *list + index;

		char *rest;
		// TODO: add more error handling
		char *token = strtok_s(icBuffer, ",", &rest);
		campaign->targetStructure = strdup(token);

		// read the number of injections
		token = strtok_s(rest, ",", &rest);
		if (atol(token) >= 0)
			campaign->nInjections = atol(token);

		// read the injection time
		token = strtok_s(rest, ",", &rest);
		if (atol(token) >= 0)
			campaign->medTimeRange = atol(token);

		// read the injection time variance
		token = strtok_s(rest, ",", &rest);
		if (atol(token) >= 0)
			campaign->variance = atol(token);

		// read the distribution
		token = strtok_s(rest, ",", &rest);
		campaign->distribution = token[0];

		DEBUG_PRINT("Read injection campaign %s\n", campaign->targetStructure);

		++index;
	}
	fclose(inputCampaign);

	return index;
}

static void printProgressBar(double percentage)
{
	double barLen = 40;
	static int rot = 0;
	fprintf(stdout, "\r                                                   \r");
	fprintf(stdout, "[");
	for (double i = 0; i < barLen; ++i)
	{
		if (i < barLen * percentage)
			fprintf(stdout, "#");
		else
			fprintf(stdout, " ");
	}
	if (percentage == 1)
		fprintf(stdout, "] %.0f%%\n", 100 * percentage);
	else
	{
		switch (rot)
		{
		case 0:
			fprintf(stdout, "] %.0f%% |", 100 * percentage);
			rot++;
			break;
		case 1:
			fprintf(stdout, "] %.0f%% /", 100 * percentage);
			rot++;
			break;
		case 2:
			fprintf(stdout, "] %.0f%% -", 100 * percentage);
			rot++;
			break;
		case 3:
			fprintf(stdout, "] %.0f%% \\", 100 * percentage);
			rot = 0;
			break;
		}
	}
#if defined(DEBUG) || defined(OUTPUT_VERBOSE)
	fprintf(stdout, "\n");
#endif
}

/**
 * Read the golden execution time from the golden file and store
 * it in the region pointed by the value parameter.
 * 
 * Returns:
 *  0 if the value was read correctly,
 *  > 0 otherwise.
 */
static int readGoldenExecutionTime(unsigned long *value)
{
	FILE *fp = fopen(GOLDEN_FILE_PATH, "r");

	if (fp == NULL)
	{
		return 1;
	}

	int nRead = fscanf(fp, "%lu", value);

	fclose(fp);

	// check the number of values read
	if (nRead != 1)
	{
		// error during the reading operation
		return 2;
	}

	// ok
	return 0;
}

static void printMany(FILE *fp, char c, int number)
{
	if (fp)
	{
		for (int i = 0; i < number; i++)
		{
			fputc(c, fp);
		}
	}
}

static void printStatistics(injectionCampaign_t *injectionCampaigns, int nInjectionCampaigns)
{
	printMany(stdout, '-', 128);
	fprintf(stdout, "\n| %-30s | %10s | %13s | %10s | %10s | %10s | %10s | %10s |\n",
			"Target", "Time (ns)", "# Injections", "Silent %", "Delay %", "Error %", "Hang %", "Crash %");

	for (int i = 0; i < nInjectionCampaigns; ++i)
	{
		printMany(stdout, '-', 128);
		fprintf(stdout, "\n| %-30s | %10d | %13d | %9.2f%% | %9.2f%% | %9.2f%% | %9.2f%% | %9.2f%% |\n",
				injectionCampaigns[i].targetStructure,
				injectionCampaigns[i].medTimeRange,
				injectionCampaigns[i].nInjections,
				(100.0 * injectionCampaigns[i].res.nSilent) / injectionCampaigns[i].nInjections,
				(100.0 * injectionCampaigns[i].res.nDelay) / injectionCampaigns[i].nInjections,
				(100.0 * injectionCampaigns[i].res.nError) / injectionCampaigns[i].nInjections,
				(100.0 * injectionCampaigns[i].res.nHang) / injectionCampaigns[i].nInjections,
				(100.0 * injectionCampaigns[i].res.nCrash) / injectionCampaigns[i].nInjections);
	}
	printMany(stdout, '-', 128);
	fprintf(stdout, "\n");
}