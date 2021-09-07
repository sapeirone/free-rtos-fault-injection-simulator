/*
 * FreeRTOS V202107.00
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
#pragma warning(disable:4996) // _CRT_SECURE_NO_WARNINGS

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Platform dependent ASM utilities */
#include "asm.h"

#include "injector.h"
#include "fork.h"
#include "thread.h"
#include "loggingUtils.h"
extern signed char loggerTrace[TRACELEN][LENBUF];

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

static void listInjectionTargets(const char *outputFilename, const target_t *target);
static void printInjectionTarget(FILE *output, const target_t *target, const int depth);

target_t *getInjectionTarget(target_t *target, char *toSearch);

static void runInjection(const void *address, const unsigned long injTime, const unsigned long offsetByte, const unsigned long offsetBit);

#define CMD_LIST "--list"
#define CMD_RUN "--run"

/*-----------------------------------------------------------*/

int main(int argc, char **argv)
{
	// print the application arguments for debugging purposes
	printApplicationArguments(argc, argv);

	/**
	 * Read the available injection targets from the tasks and timer modules.
	 * 
	 * TODO: possibly add more injection targets.
	 */
	target_t *targets = read_tasks_targets(NULL);
	targets = read_timer_targets(targets);

	if (argc > 1 && strcmp(argv[1], CMD_LIST) == 0)
	{
		listInjectionTargets("targets.txt", targets);

		freeInjectionTargets(targets);
		exit(0);
	}
	else if (argc > 1 && strcmp(argv[1], CMD_RUN) == 0)
	{
		if (argc != 6)
		{
			fprintf(stdout, "Invalid number of arguments for --run.\n");
			return 7;
		}

		// (void * function, void * address, unsigned long injTime, unsigned long offsetByte, unsigned long offsetBit, thread_t * id)
		const void *address = (void *)atol(argv[2]);
		const unsigned long injTime = atol(argv[3]);
		const unsigned long offsetByte = atol(argv[4]);
		const unsigned long offsetBit = atol(argv[5]);

		runInjection(address, injTime, offsetBit, offsetBit);
	}

	/*
	// Example of a freeRTOS injector execution using runFreeRTOSInjection and waitFreeRTOSInjection
	freeRTOSInstance instance;
	target_t target = targets[0];
	if (runFreeRTOSInjection(&instance, argv[0], target.address, 1000) == FREE_RTOS_FORK_SUCCESS) {
		int exitCode = waitFreeRTOSInjection(&instance);
		printf("The FreeRTOS instance completed with status code %d.\n", exitCode);
	}

	return 0;
	*/

	/*
	Option --Force re-executes forcibly the golden execution
	Option --List prints out all the possible targets for the injection campaign (fork + data collection)
	Option --j number of parallel instances of FreeRTOS + Injector to run simultaneously
	*/

	/*
	Generate the golden execution process and collect results if no previous
	golden execution output has been found or if --Force is used
	*/
	FILE *gefp = NULL;
	gefp = fopen("golden.txt", "r");

	if (gefp == NULL || (argc > 1 && strcmp(argv[1], "--force") == 0))
	{
		freeRTOSInstance instance;
		int pid = runFreeRTOSInjection(&instance, argv[0], NULL, 0, 0, 0);
		if (pid < 0)
		{
			fprintf(stdout, "Couldn't create golden execution process.\n");
			return 7;
		}
		else if (pid > 0)
		{ // Father process
			waitFreeRTOSInjection(&instance);
		}
		else
		{ // Child process
			prvInitialiseHeap();
			main_blinky();
		}
	}

	if(gefp != NULL)
		fclose(gefp);

	/*
	Read from file the injection details which is the target structure, how many injections have
	to be tested, the median of the time range, the variance over the time range and the distribution
	(by default a Gaussian).
	Prototype of the input .csv:
	char * targetStructure, int nInjections, double medTimeRange, double variance, char * distr
	Returns a list of struct injection campaigns.
	*/
	int icI = 0;
	FILE *icfp = NULL;
	char icBuffer[LENBUF];
	injectionCampaign_t *icS = (injectionCampaign_t *)malloc(0);

	icfp = fopen("input.csv", "r");
	if (icfp == NULL)
	{
		fprintf(stderr, "Couldn't open input file.\n");
		return 1;
	}

	while (fgets(icBuffer, LENBUF - 1, icfp) != NULL)
	{
		icS = (injectionCampaign_t *)realloc(icS, (icI + 1) * sizeof(injectionCampaign_t));
		sscanf(icBuffer, "%s;%d;%lu;%lu;%s;", icS[icI].targetStructure, &icS[icI].nInjections,
			   &icS[icI].medTimeRange, &icS[icI].variance, icS[icI].distr);
		++icI;
	}
	fclose(icfp);

	/*
	Print out an time extimation of minimum and maximum execution times for the whole simulation.
	The user must input (Y/n) to confirm execution.
	*/
	FILE *tefp = NULL;
	char teBuffer[LENBUF];
	unsigned long nTicksGoldenEx = 0, estTotTimeMin = 0, estTotTimeMax = 0;

	tefp = fopen("golden.txt", "r");
	if (tefp == NULL)
	{
		fprintf(stderr, "Couldn't open golden execution results file.\n");
		return 2;
	}
	fgets(teBuffer, LENBUF - 1, tefp);
	sscanf(teBuffer, "%lu", &nTicksGoldenEx);

	fprintf(stdout, "Estimated execution times:\nTarget\tnExecs\ttMed\tvar\tdistr\testTimeMin\testTimeMax");
	for (int i = 0; i < icI; ++i)
	{
		unsigned long estTimeMin = icS[i].nInjections * nTicksGoldenEx;
		unsigned long estTimeMax = estTimeMin * 3; //300% of golden execution time, for each injection in the campaign
		fprintf(stdout, "%s\t%d\t%lu\t%lu\t%s\t%lu\t%lu\n", icS[icI].targetStructure, icS[icI].nInjections,
				icS[icI].medTimeRange, icS[icI].variance, icS[icI].distr, estTimeMin, estTimeMax);
		estTotTimeMin += estTimeMin;
		estTotTimeMax += estTimeMax;
	}
	fprintf(stdout, "TOTAL\t_\t_\t_\t_\t%lu\t%lu", estTotTimeMin, estTotTimeMax);

	fclose(tefp);

	/*
	For each line in the input .csv, generate a injection campaign.
	A for loop launches the iFork() of the forefather.
	Each father (son of the forefather), launches a thread instance of the FreeRTOS + Injector.
	The forefather waits for the father: if the return value of the wait is different from 0, the
	forefatehr adds 1 to the "crash" entry for that campaign.
	Each father awaits the 300% golden execution time and then reads the trace, unless the FreeRTOS returned
	by itself sooner. The father decides which termination has been performed and increases the relative
	statistic in the memory mapped file for that campaign.
	Completing injection campaigns advances a general completion bar.
	*/
	for (int i = 0; i < icI; ++i)
	{ // For each injection campaign
		for (int j = 0; j < icS[i].nInjections; ++j)
		{ // For each injection in a campaign
			target_t *injTarget = getInjectionTarget(targets, icS[i].targetStructure);
			if (injTarget == NULL)
			{
				fprintf(stderr, "No target with name %s was found.\n", icS[i].targetStructure);
				return 4;
			}

			srand((unsigned int) time(NULL));									 //generate random seed
			unsigned long offsetByte = rand() % injTarget->size; //select byte to inject
			unsigned long offsetBit = rand() % 8;				 //select bit to inject
			unsigned long injTime;

			switch (icS[i].distr[0])
			{ //Pick a distribution
			case 'g':
			case 'G': // Gaussian
				// TODO
				break;
			case 'u':
			case 'U':														// Uniform
				injTime = icS[i].medTimeRange;								//if delta!=0 select time in interval
				rand() % 2 ? (injTime += icS[i].variance) : (injTime = abs(injTime - (signed long) icS[i].variance)); //choose if before or after selected time
				break;
			default:
				fprintf(stderr, "No distribution with name %s is available.\n", icS[i].distr);
				return 5;
			}

			freeRTOSInstance instance;
			int pid = runFreeRTOSInjection(&instance, argv[0], injTarget->address, injTime, offsetByte, offsetBit);
			if (pid < 0)
			{
				fprintf(stdout, "Couldn't create child process.\n");
				return 6;
			}
			else if (pid > 0)
			{ // Father process
				waitFreeRTOSInjection(&instance);
			}
		}
	}

	/* Free the icS structure of injection campaigns allocated when reading the "input.csv" file */
	free(icS);

	/*
	Once all the injection campaigns have been completed, the forefather opens the output file and
	prints on screen some statistics.
	*/
	FILE *stfp = NULL;
	injectionCampaign_t stBuffer;

	stfp = fopen("results.dat", "rb");
	if (stfp == NULL)
	{
		fprintf(stderr, "Couldn't open golden execution results file.\n");
		return 3;
	}

	fprintf(stdout, "Execution statistics:\nTarget\tCrash\tHang\tSilent\tDelay\tNoError\n");
	for (int i = 0; i < icI; ++i)
	{
		fread(&stBuffer, sizeof(injectionCampaign_t), 1, stfp);
		fprintf(stdout, "%s\t%.3lu\t%.3lu\t%.3lu\t%.3lu\t%.3lu\n", stBuffer.targetStructure,
				stBuffer.res.nCrash / stBuffer.nInjections, stBuffer.res.nHang / stBuffer.nInjections,
				stBuffer.res.nSilent / stBuffer.nInjections, stBuffer.res.nDelay / stBuffer.nInjections,
				stBuffer.res.nNoError / stBuffer.nInjections);
	}

	// terminate
	fclose(stfp);

	return 0;
}
/*-----------------------------------------------------------*/

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
	printf("ASSERT! Line %ld, file %s\r\n", ulLine, pcFileName);

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

static void listInjectionTargets(const char *outputFilename, const target_t *targets)
{
	char buf[256];

	/**
	 * Open the target output file
	 */
	FILE *outputFile = fopen(outputFilename, "w");
	if (outputFile == NULL)
	{
		sprintf(buf, "Error opening the target file %s", outputFilename);
		perror(buf);
		exit(1);
	}

	/**
	 * Iterate over the available injection targets
	 */
	printInjectionTarget(outputFile, targets, 0);

	fclose(outputFile);
}

static void printInjectionTarget(FILE *output, const target_t *target, const int depth)
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
	printf("Application arguments: ");
	for (int i = 0; i < argc; i++)
	{
		printf("%s ", argv[i]);
	}
	printf("\n");
}

static void runInjection(const void *address, const unsigned long injTime, const unsigned long offsetByte, const unsigned long offsetBit)
{
	thread_t thID;
	if (launchThread(&injectorFunction, address, injTime, offsetByte, offsetBit, &thID) == INJECTOR_THREAD_FAILURE)
	{
		fprintf(stdout, "Injectior thread launch failure.\n");
	}
	detachThread(&thID);

	/* Launch the FreeRTOS */
	prvInitialiseHeap();
	main_blinky();

	/* Check trace and determine the outcome of the simulation */
	int result = 0, flag = 0;
	FILE *tefp = NULL;
	char teBuffer[LENBUF];
	unsigned long nTicksGoldenEx = 0, delay = 0, execTime = 0;
	fgets(teBuffer, LENBUF - 1, tefp);
	sscanf(teBuffer, "%lu", &nTicksGoldenEx);

	for (int i = 0; i < TRACELEN; ++i)
	{
		char tmpBuffer[LENBUF];
		sscanf(loggerTrace[i], "\t\t%s", tmpBuffer);
		if (strncmp(tmpBuffer, "Idle", 4) != 0)
		{
			flag = 1;
			break;
		}
	}

	execTime = sscanf(loggerTrace[TRACELEN - 1], "%lu", &execTime);
	delay = abs(nTicksGoldenEx - execTime);

	fclose(tefp);

	/* TODO: Working trace analyzer */

	/* Open the memory mapped file and increase the relative counter */

	exit(42);
}