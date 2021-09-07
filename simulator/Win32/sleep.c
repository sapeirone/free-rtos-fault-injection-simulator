#include "sleep.h"
#include "stdio.h"
#include "windows.h"

void sleepNanoseconds(unsigned long ns)
{
    LONGLONG llns = (LONGLONG) ns / 100LL;
	HANDLE timer;
	LARGE_INTEGER li;
	if(!(timer = CreateWaitableTimer(NULL, TRUE, NULL)))
		fprintf(stdout, "sleepNanoseconds failure");
	li.QuadPart = -llns;
	if(!SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE)){
		CloseHandle(timer);
		fprintf(stdout, "sleepNanoseconds failure");
	}
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
	return;
}
