#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <conio.h>
#include "XSleep.h"

DWORD WINAPI TestSleepTheadFunc(LPVOID param)
{
	printf("Going to sleep...\n");

	time_t startTime = time(NULL);

	XSleep(2000);	// THIS IS THE MAIN FUNCTION CALL

	printf("Waking up...\n");
		
	time_t endTime = time(NULL);

	printf("I slept for %.0f MSecs\n", difftime(endTime, startTime));

	return 0;
}

void main()
{
	DWORD threadId;

	for(int i = 0; i < 2; i++)
	{	
		CreateThread(NULL, 0, TestSleepTheadFunc, NULL, 0, &threadId);
	}

	getch();
}
