#include "sleep.h"
#include "windows.h"

void sleepNanoseconds(unsigned long ns)
{
    LONGLONG llns = (LONGLONG) ns / 100LL;
	HANDLE timer;
	LARGE_INTEGER li;
	if(!(timer = CreateWaitableTimer(NULL, TRUE, NULL)))
		return FALSE;
	li.QuadPart = -llns;
	if(!SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE)){
		CloseHandle(timer);
		return FALSE;
	}
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
	return;
}
