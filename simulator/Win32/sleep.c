#include "sleep.h"
#include "stdio.h"
#include "windows.h"
#include "simulator.h"

HANDLE wakeInjEv;

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

void injectorWait(){
	wakeInjEv = CreateEvent(NULL, TRUE, TRUE, NULL);

    if (wakeInjEv == NULL) 
    { 
        ERR_PRINT("CreateEvent failed (%d)\n", GetLastError());
    }

	WaitForSingleObject(wakeInjEv, INFINITE);

	closeHandle(wakeInjEv);
}

void wakeInjector(){
	if (!SetEvent(wakeInjEv) ) 
    {
        ERR_PRINT("SetEvent failed (%d)\n", GetLastError());
        return;
    }
}