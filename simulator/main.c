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
#pragma warning(disable : 4996) // _CRT_SECURE_NO_WARNINGS

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

#include "simulator.h"
#include "benchmark/benchmark.h"

extern struct myStringStruct array[MAXARRAY];
extern signed char loggerTrace[TRACELEN][LENBUF];

/* Global variables */
extern int isGolden;

/* This demo uses heap_5.c, and these constants define the sizes of the regions
that make up the total heap.  heap_5 is only used for test and example purposes
as this demo could easily create one large heap region instead of multiple
smaller heap regions - in which case heap_4.c would be the more appropriate
choice.  See http://www.freertos.org/a00111.html for an explanation. */
#define mainREGION_1_SIZE 8201
#define mainREGION_2_SIZE 29905
#define mainREGION_3_SIZE 7807

/*-----------------------------------------------------------*/

extern void main_blinky(void);

/*
 * Only the comprehensive demo uses application hook (callback) functions.  See
 * http://www.freertos.org/a00016.html for more information.
 */
void vFullDemoTickHookFunction(void);
void vFullDemoIdleFunction(void);

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

/*
 * Writes trace data to a disk file when the trace recording is stopped.
 * This function will simply overwrite any trace files that already exist.
 */
//static void prvSaveTraceFile(void);

/*-----------------------------------------------------------*/

/* When configSUPPORT_STATIC_ALLOCATION is set to 1 the application writer can
use a callback function to optionally provide the memory required by the idle
and timer tasks.  This is the stack that will be used by the timer task.  It is
declared here, as a global, so it can be checked by a test that is implemented
in a different file. */
StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

/* Notes if the trace is running or not. */
static BaseType_t xTraceRunning = pdTRUE;

typedef struct injectionResults
{
	int nCrash, nHang, nSilent, nDelay, nNoError;
} injectionResults_t;

typedef struct injectionCampaign
{
	char *targetStructure, *distr;
	int nInjections;
	unsigned long medTimeRange, variance;
	injectionResults_t res;
} injectionCampaign_t;

static void printApplicationArguments(int argc, char **argv);

static void printInjectionTarget(FILE *output, target_t *target, int depth);

target_t *getInjectionTarget(target_t *target, char *toSearch);

static void runSimulator(const thData_t *injectionArgs);
static void writeGoldenFile();

static void execCmdList(int argc, char **argv);
static void execCmdRun(int argc, char **argv);
static void execCmdGolden(int argc, char **argv);
static void execInjectionCampaign(int argc, char **argv);

static int readInjectionCampaignList(const char *filename, injectionCampaign_t **campaignList);

/**
 * List of injection targets for the current instance of the 
 * FreeRTOS simulator.
 * The list is initialized in main.
 */
static target_t *targets;

/*-----------------------------------------------------------*/

int main(int argc, char **argv)
{
	// print the application arguments for debugging purposes
	printApplicationArguments(argc, argv);

	if (argc < 2)
	{
		ERR_PRINT("Please specify a command argument --(list|run|golden)\n");
		return 1;
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
		// TODO: option --j number of parallel instances of FreeRTOS + Injector to run simultaneously
		execCmdRun(argc, argv);
	}
	else if (strcmp(argv[1], CMD_GOLDEN) == 0)
	{
		execCmdGolden(argc, argv);
	}
	else if (strcmp(argv[1], CMD_CAMPAIGN) == 0)
	{
		execInjectionCampaign(argc, argv);
	}

	return 0;
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
		exit(GENERIC_ERROR_CODE);
	}

	unsigned long goldenExecTime;
	if (fscanf(golden, "%lu", &goldenExecTime) != 1)
	{
		ERR_PRINT("Cannot read the golden execution time\n");

		fclose(golden);
		exit(GENERIC_ERROR_CODE);
	}
	DEBUG_PRINT("Execution timeout is %lu\n", goldenExecTime);

	// TODO: replace with strol (atol does NOT detect errors)
	unsigned long injTime = atol(argv[3]);
	unsigned long offsetByte = atol(argv[4]);
	unsigned long offsetBit = atol(argv[5]);

	target_t *injTarget = getInjectionTarget(targets, argv[2]);

	if (!injTarget)
	{
		ERR_PRINT("Cannot find the injection target\n");

		fclose(golden);
		exit(GENERIC_ERROR_CODE);
	}

	// create a wrapper for the injection parameters
	thData_t injection;
	injection.address = injTarget->address;
	injection.injTime = injTime;
	injection.offsetByte = offsetByte;
	injection.offsetBit = offsetBit;
	injection.timeoutNs = 3 * goldenExecTime;

	runSimulator(&injection);
}

static void execCmdGolden(int argc, char **argv)
{
	if (argc != 2)
	{
		ERR_PRINT("The --golden command does not support additional parameters");
		exit(INVALID_NUMBER_OF_PARAMETERS_EXIT_CODE);
	}

	runSimulator(NULL);
}

static void execInjectionCampaign(int argc, char **argv)
{
	if (argc != 3)
	{
		ERR_PRINT("Invalid number of arguments for %s.\n", CMD_CAMPAIGN);
		exit(INVALID_NUMBER_OF_PARAMETERS_EXIT_CODE);
	}

	const char *input = argv[2];

	/*
	Read from file the injection details which is the target structure, how many injections have
	to be tested, the median of the time range, the variance over the time range and the distribution
	(by default a Gaussian).
	Prototype of the input .csv:
	char * targetStructure, int nInjections, double medTimeRange, double variance, char * distr
	Returns a list of struct injection campaigns.
	*/
	injectionCampaign_t *injectionCampaigns;
	int nInjectionCampaigns = readInjectionCampaignList(argv[2], &injectionCampaigns);

	// TODO: extract the following piece of code as a function
	FILE *tefp = NULL;
	char teBuffer[LENBUF];
	unsigned long nanoGoldenEx = 0;
	unsigned long nTotalInjections = 0;
	double estTotTimeMin = 0.0, estTotTimeMax = 0.0;

	tefp = fopen(GOLDEN_FILE_PATH, "r");
	if (tefp == NULL)
	{
		ERR_PRINT("Couldn't open golden execution results file.\n");
		return 2;
	}
	fgets(teBuffer, LENBUF - 1, tefp);
	sscanf(teBuffer, "%lu", &nanoGoldenEx);

	/**
	 * Print out an time extimation of minimum and maximum execution times for the whole simulation.
	 * The user must input (Y/n) to confirm execution.
	 */
	fprintf(stdout, "\nEstimated execution times:\n");

	for (int i = 0; i < 104; i++)
		fputc('-', stdout);
	fprintf(stdout, "\n| %-30s | %8s | %10s | %5s | %5s | %12s | %12s |\n", "Target",
			"nExecs", "tMed", "var", "distr", "estTimeMin", "estTimeMax");

	for (int i = 0; i < nInjectionCampaigns; ++i)
	{
		double estTimeMin = (injectionCampaigns[i].nInjections * nanoGoldenEx) / (1000.0 * 1000.0 * 1000.0);
		//300% of golden execution time, for each injection in the campaign
		double estTimeMax = estTimeMin * 3.0;

		for (int i = 0; i < 104; i++)
			fputc('-', stdout);
		fprintf(stdout, "\n| %-30s | %8d | %10lu | %5lu | %5s | %10.2f s | %10.2f s |\n", injectionCampaigns[i].targetStructure,
				injectionCampaigns[i].nInjections,
				injectionCampaigns[i].medTimeRange,
				injectionCampaigns[i].variance,
				injectionCampaigns[i].distr,
				estTimeMin, estTimeMax);

		estTotTimeMin += estTimeMin;
		estTotTimeMax += estTimeMax;
		nTotalInjections += injectionCampaigns[i].nInjections;
	}

	fclose(tefp);

	for (int i = 0; i < 104; i++)
		fputc('-', stdout);
	fprintf(stdout, "\n%-30s     %8s   %10s   %5s   %5s   %10.2f s   %10.2f s \n\n",
			"Total estimated time", "-", "-", "-", "-", estTotTimeMin, estTotTimeMax);

	char choice = 0;
	while (choice != 'y' && choice != 'n') {
		fprintf(stdout, "Continue? (y|n): ");
		fscanf(stdin, "%c", &choice);
		if (choice == 'n') {
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
	int nCurrentInjection = 0;
	for (int i = 0; i < nInjectionCampaigns; ++i)
	{ 
		// For each injection campaign
		injectionCampaign_t campaign = injectionCampaigns[i];

		for (int j = 0; j < campaign.nInjections; ++j)
		{ 
			// For each injection in a campaign
			nCurrentInjection++;
			DEBUG_PRINT("Running injection n. %lu/%lu\n", nCurrentInjection, nTotalInjections);

			target_t *injTarget = getInjectionTarget(targets, campaign.targetStructure);
			if (injTarget == NULL)
			{
				ERR_PRINT("No target with name %s was found.\n", campaign.targetStructure);
				return 4;
			}

			srand((unsigned int)time(NULL));					 //generate random seed
			unsigned long offsetByte = rand() % injTarget->size; //select byte to inject
			unsigned long offsetBit = rand() % 8;				 //select bit to inject
			unsigned long injTime;

			switch (injectionCampaigns[i].distr[0])
			{ //Pick a distribution
			case 'g':
			case 'G': // Gaussian
				// TODO
				break;
			case 'u':
			case 'U':																								 // Uniform
				injTime = injectionCampaigns[i].medTimeRange;	
				// TODO: fix me																	 //if delta!=0 select time in interval
				rand() % 2 ? (injTime += injectionCampaigns[i].variance) : (injTime = abs(injTime - (signed long)campaign.variance)); //choose if before or after selected time
				break;
			default:
				ERR_PRINT("No distribution with name %s is available.\n", campaign.distr);
				return 5;
			}

			freeRTOSInstance instance;
			int pid = runFreeRTOSInjection(&instance, argv[0], injTarget->name, injTime, offsetByte, offsetBit);
			if (pid < 0)
			{
				ERR_PRINT("Couldn't create child process.\n");
				return 6;
			}
			else if (pid > 0)
			{ // Father process
				waitFreeRTOSInjection(&instance);
			}
		}
	}
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

	taskENTER_CRITICAL();
	{
		/* Stop the trace recording. */
		if (xPrinted == pdFALSE)
		{
			xPrinted = pdTRUE;
			if (xTraceRunning == pdTRUE)
			{
				// vTraceStop(); // TODO: check this
				// prvSaveTraceFile();
			}
		}

		/* Cause debugger break point if being debugged. */
		// __debugbreak();

		/* You can step out of this function to debug the assertion by using
		the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
		value. */
		while (ulSetToNonZeroInDebuggerToContinue == 0)
		{
			ASM_NOP;
			ASM_NOP;
		}
	}
	taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

// static void prvSaveTraceFile(void)
// {
// 	FILE *pxOutputFile;

// 	fopen_s(&pxOutputFile, "Trace.dump", "wb");

// 	if (pxOutputFile != NULL)
// 	{
// 		fwrite(RecorderDataPtr, sizeof(RecorderDataType), 1, pxOutputFile);
// 		fclose(pxOutputFile);
// 		printf("\r\nTrace output saved to Trace.dump\r\n");
// 	}
// 	else
// 	{
// 		printf("\r\nFailed to create trace dump file\r\n");
// 	}
// }
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

	for (target_t *child = target->content; child; child = child->next)
	{
		printInjectionTarget(output, child, depth + 1);
	}

	if (target->next)
	{
		printInjectionTarget(output, target->next, depth);
	}
}

target_t *getInjectionTarget(target_t *target, char *toSearch)
{

	target_t *tmp = target;
	while (tmp->next != NULL)
	{
		if (strcmp(tmp->name, toSearch) == 0)
		{
			return tmp;
		}

		for (target_t *child = tmp->content; child; child = child->next)
		{
			if (strcmp(tmp->name, toSearch) == 0)
			{
				return tmp;
			}
		}
		tmp = tmp->next;
	}

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

	/* Launch the FreeRTOS */
	prvInitialiseHeap();
	main_blinky();

	DEBUG_PRINT("Call to main_blinky completed\n");

	if(isGolden)
		exit(EXIT_SUCCESS);

	/* Check trace and determine the outcome of the simulation.
	 * Crash = 50
	 * Hang = 48
	 * Error = 46
	 * Delay = 44
	 * Silent = 42
	 */
	int result = 50;
	unsigned long nanoGoldenEx = 0, delay = 0, execTime = 0;
	nanoGoldenEx = injectionArgs->timeoutNs / 3;

	execTime = sscanf(loggerTrace[TRACELEN - 1], "%lu", &execTime);
	delay = abs(nanoGoldenEx - execTime);

	if(traceOutputIsCorrect()){			// Correct Trace output, ISR worked
		if(executionResultIsCorrect()){	// Execution result is correct
			if(delay < 5000)			// Silent execution, correct output
			{ 
				exit(42);
			}
			else						// Delayed execution, correct output
			{ 
				exit(44);
			}
		}
		else							// Execution result is not correct
		{
			if(delay < 5000)			// Error execution, incorrect output
			{ 
				exit(46);
			}
			else						// Hang execution, incorrect output
			{ 
				exit(48);
			}
		}
	}
	else 								// Incorrect Trace output, ISR didn't work
	{
		exit(50);
	}

	/* This should never be executed */
	ERR_PRINT("BIG DANGER!\nSomehow, the execution of the RTOS produced an unexpected output.\n");
	exit(EXIT_FAILURE);
}

int traceOutputIsCorrect(){
	char tmpBuffer[4][LENBUF];
	sscanf(loggerTrace[TRACELEN-2], "\t%s\t%s", tmpBuffer[0], tmpBuffer[1]);
	sscanf(loggerTrace[TRACELEN-1], "\t%s\t%s", tmpBuffer[2], tmpBuffer[3]);
	return (strncmp(tmpBuffer[0], "[IN]", 4) == 0 && strncmp(tmpBuffer[1], "IDLE", 4) == 0 &&
	   strncmp(tmpBuffer[2], "[RIF]", 5) == 0 && strncmp(tmpBuffer[3], "IDLE", 4) == 0);
}

int executionResultIsCorrect(){
	int result = 1;
	FILE *goldenfp = fopen(GOLDEN_FILE_PATH, "w");
	if (goldenfp == NULL)
	{
		ERR_PRINT("Couldn't open %s for writing.\n", GOLDEN_FILE_PATH);
		exit(EXIT_FAILURE);
	}

	char buffer[LENBUF];
	fscanf(goldenfp, "%s\n", buffer); // Ingore first line, the goldenExecutionTime
	for(int i = 0; i < MAXARRAY; i++){
		fscanf(goldenfp, "%s\n", buffer);
		if(strcmp(buffer, array[i].qstring) != 0){
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
	for(int i = 0; i < MAXARRAY; i++){
      fprintf(stdout, "%s\n", i, array[i].qstring);
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
		*list = (injectionCampaign_t *)realloc(*list, (index + 1) * sizeof(injectionCampaign_t));

		injectionCampaign_t *campaign = *list + index;

		char targetStructure[256];
		unsigned long nInjections;
		unsigned long medTimeRange;
		unsigned long variance;
		char distr;

		char *rest;
		// TODO: add more error handling
		char *token = strtok_s(icBuffer, ",", &rest);
		campaign->targetStructure = strdup(token);

		// read the number of injections
		token = strtok_s(rest, ",", &rest);
		campaign->nInjections = atol(token);

		// read the injection time
		token = strtok_s(rest, ",", &rest);
		campaign->medTimeRange = atol(token);

		// read the injection time variance
		token = strtok_s(rest, ",", &rest);
		campaign->variance = atol(token);

		// read the distribution
		token = strtok_s(rest, ",", &rest);
		token[1] = '\0';
		campaign->distr = strdup(token);

		DEBUG_PRINT("Read injection campaign %s\n", campaign->targetStructure);

		++index;
	}
	fclose(inputCampaign);

	return index;
}