// Primary thread waits till child thread terminates
#include <windows.h>
#include <stdio.h>

// Thread counts till PassVal
void ThreadMain(LONG PassVal)
{
	LONG i;

	for(i=0;i<PassVal;i++)
	{
		wprintf(L"ThreadMain() - count #%d\n", i);
	}
}

// Main process & thread
int main(void)
{
	DWORD ThreadID, dwRet;
	HANDLE hTh;

	// Creates a thread to execute within the virtual address space of the calling process
	hTh=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadMain,(LPVOID)100,0,&ThreadID);

	if(hTh==NULL)
	{
		wprintf(L"CreateThread() failed, error %u\n", GetLastError());
		return 1;
	}
	else
		wprintf(L"CreateThread() is OK, thread ID %u\n", ThreadID);

	// Blocks/wait till thread terminates
	// Waits until the specified object is in the signaled state or
	// the time-out interval elapses.
	// The INFINITE parameter make the function return only when the object is signaled
	wprintf(L"Waiting the child thread terminates/signaled...\n");
	dwRet = WaitForSingleObject(hTh,50);
	wprintf(L"WaitForSingleObject() returns value is 0X%.8X\n", dwRet);

	switch(dwRet)
	{
	case WAIT_ABANDONED:
		wprintf(L"Mutex object was not released by the thread that\n"
			L"owned the mutex object before the owning thread terminates...\n");
		break;
	case WAIT_OBJECT_0:
		wprintf(L"The child thread state was signaled!\n");
		break;
	case WAIT_TIMEOUT:
		wprintf(L"Time-out interval elapsed, and the child thread's state is nonsignaled.\n");
		break;
	case WAIT_FAILED:
		wprintf(L"WaitForSingleObject() failed, error %u\n", GetLastError());
		ExitProcess(0);
	}

	if(CloseHandle(hTh) != 0)
		wprintf(L"hTh's child thread handle was closed successfully!\n");
	else
		wprintf(L"Failed to close hTh child thread handle, error %u\n", GetLastError());

	wprintf(L"Main process & thread ready to exit...\n");

	return 0;
}