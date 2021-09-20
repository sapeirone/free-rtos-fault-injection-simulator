#include "sleep.h"
#include "stdio.h"
#include "windows.h"
#include "simulator.h"

HANDLE wakeInjEv = INVALID_HANDLE_VALUE;
int eventIsSet = 0;

void sleepNanoseconds(unsigned long ns)
{
	while (ulGetRunTimeCounterValue() < ns);
	return;
}

void injectorWait(){
	if(!eventIsSet){
		wakeInjEv = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (wakeInjEv == NULL)
			ERR_PRINT("CreateEvent failed (%d)\n", GetLastError());
		eventIsSet++;
	}		

	WaitForSingleObject(wakeInjEv, INFINITE);

	CloseHandle(wakeInjEv);
}

void wakeInjector(){
	if (eventIsSet-- == 1 && !SetEvent(wakeInjEv) && !SwitchToThread()) 
    {
        ERR_PRINT("SetEvent failed (%d)\n", GetLastError());
        return;
    }
}