// The main thread waits till the first child terminates
#include <windows.h>
#include <stdio.h>

#define THREADCOUNT   3

/////Thread counts till MaxValue
void ThreadMain(WCHAR * ThreadInfo)
{
	WCHAR Name[20];
	DWORD MaxValue = 50, i;

	swscanf_s(ThreadInfo,L"%S %d", Name, sizeof(Name), &MaxValue, sizeof(MaxValue));

	for(i=0;i<MaxValue;++i)
	{
		wprintf(L"%S - pass #%d\n", Name, i);
	}
}

//Main
int wmain(void)
{
	HANDLE hThr[THREADCOUNT];
	DWORD i, dwRet;

	WCHAR *Buf[THREADCOUNT]= {L"Thread#1 10", L"Thread#2 20", L"Thread#3 30"};

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

	// Blocks/waits till all child threads are finished
	// If 3rd TRUE, the function returns when the state of all objects in the handle array is signaled.
	// If FALSE, the function returns when the state of any one of the objects is set to signaled
	// The INFINITE - the function will return only when the specified objects are signaled.
	dwRet=WaitForMultipleObjects(THREADCOUNT,hThr,FALSE,2);

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

		// hThr[2] was signaled
	case WAIT_OBJECT_0 + 2:
		// TODO: Perform tasks required by this event
		wprintf(L"Third event was signaled...\n");
		break;

		// ...

		// Time out
	case WAIT_TIMEOUT:
		wprintf(L"The waiting is timed out...\n");
		break;

		// Return value is invalid.
	default:
		wprintf(L"Waiting failed, error %d...\n", GetLastError());
		ExitProcess(0);
	}     

	/*
	if(dwRet>=WAIT_OBJECT_0 && dwRet<WAIT_OBJECT_0+THREADCOUNT)
	{
	wprintf(L"The first thread terminated with index: %u\n",dwRet-WAIT_OBJECT_0);
	}
	*/

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
