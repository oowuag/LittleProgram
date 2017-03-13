// The main thread waits till the first child terminates
#include <windows.h>
#include <stdio.h>
#include <iostream>
using namespace std;

#define THREADCOUNT   2

#define EVENTCOUNT   2


HANDLE g_Event[EVENTCOUNT]; 




/////Thread counts till MaxValue
void ThreadMain(WCHAR * ThreadInfo)
{
	WCHAR Name[20];
	DWORD MaxValue = 50, i;

	swscanf_s(ThreadInfo,L"%S %d", Name, sizeof(Name), &MaxValue, sizeof(MaxValue));

	//for(i=0;i<MaxValue;++i)
	//{
	//	wprintf(L"%S - pass #%d\n", Name, i);
	//}

	while(1)
	{
		SetEvent(g_Event[0]);
		//SetEvent(g_Event[1]);
		Sleep(500);
	}
}

//Main
int wmain(void)
{

	DWORD i, dwRet;
	WCHAR *Buf[THREADCOUNT]= {L"Thread#1", L"Thread#2"};
	HANDLE hThr[THREADCOUNT];

	g_Event[0] = CreateEvent(NULL,FALSE, TRUE, "Event0"); 
	g_Event[1] = CreateEvent(NULL,FALSE, TRUE, "Event1"); 


	for(i=0;i<THREADCOUNT;i++)
	{
		DWORD ThreadId;

		hThr[i] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadMain, (LPVOID)Buf[i],0,&ThreadId);

		if(hThr[i]== NULL)
		{
			wprintf(L"CreateThread() failed, error %u\n", GetLastError());
			ExitProcess(1);
			return 1;
		}
		else
			wprintf(L"CreateThread() is OK, thread ID %u\n", ThreadId);
	}

	while (1)
	{	
		dwRet=WaitForMultipleObjects(THREADCOUNT,g_Event,FALSE,1000);

		cout << ::GetTickCount() << endl;

		switch(dwRet)
		{
			// hThr[0] was signaled
		case WAIT_OBJECT_0 + 0:
			// TODO: Perform tasks required by this event
			wprintf(L"First event was signaled...\n");
			break;

			// hThr[1] was signaled
		case WAIT_OBJECT_0 + 1:
			// TODO: Perform tasks required by this event
			wprintf(L"Second event was signaled...\n");
			break;

			// Time out
		case WAIT_TIMEOUT:
			wprintf(L"The waiting is timed out...\n");
			break;

			// Return value is invalid.
		default:
			wprintf(L"Waiting failed, error %d...\n", GetLastError());
			ExitProcess(0);
		}     

		//::SetEvent(hThr[0]);
		//::SetEvent(hThr[1]);

	}



	// close all handles to threads
	for(i=0;i<THREADCOUNT;i++)
	{
		if(CloseHandle(hThr[i]) != 0)
			wprintf(L"hTr's handle was closed successfully!\n");
		else
			wprintf(L"Failed to close hTr's handle, error %u\n", GetLastError());
	}

	ExitProcess(0);
	return 0;
}
