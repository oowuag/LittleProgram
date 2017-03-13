#include <windows.h>
#include <stdio.h>
#include <stdlib.h>


class WriteFileThread
{
protected:
	TCHAR			name[MAX_PATH];
	HANDLE			hevent;
	volatile BOOL	quit_flg;
	HANDLE			handle;
	DWORD			thread_id;

	static void ThreadProc(LPVOID p);
	void ExecRun();
	BOOL Join(DWORD msec);

public:
	WriteFileThread();
	virtual ~WriteFileThread();
	
	void StartThread(LPCTSTR name = TEXT("Unknown Thread"));
	BOOL StopThread(DWORD msec = INFINITE);
	BOOL Wait(DWORD msec = INFINITE);
	void Notify();
	inline BOOL IsAlive();
	inline BOOL CheckQuit();
	inline LPCTSTR GetName();
	inline DWORD GetThreadID();
	virtual void Run();
};

WriteFileThread::WriteFileThread()
{
	handle		= NULL;
	thread_id	= 0; 
	quit_flg	= FALSE; 
	name[0]		= TEXT('\0');
	hevent		= ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

WriteFileThread::~WriteFileThread()
{
	StopThread();
	if(hevent) ::CloseHandle(hevent);
}


VOID WriteFileThread::ThreadProc(LPVOID p)
{
	if(p == NULL) 
	{
		return ;
	}
	((WriteFileThread*)p)->ExecRun();
}

void WriteFileThread::ExecRun()
{
	Run();

	::CloseHandle(handle);
	handle = NULL;
	thread_id = 0;
}


BOOL WriteFileThread::Wait(DWORD msec)
{
	DWORD start_time = ::GetTickCount();

	while(1)
	{
		DWORD wait_time;

		if (msec == INFINITE)
		{
			wait_time = INFINITE;
		}
		else
		{
			DWORD loss = ::GetTickCount() - start_time;
			wait_time = (msec > loss) ? (msec - loss) : 0;
		}
		switch(::MsgWaitForMultipleObjectsEx(1, &hevent, wait_time, QS_ALLINPUT, MWMO_INPUTAVAILABLE))
		{
		case WAIT_OBJECT_0:
			return TRUE;

		case WAIT_TIMEOUT:
			return FALSE;

		default:
			break;
		}
	}
}

void WriteFileThread::Notify()
{
	::SetEvent(hevent);
}


void WriteFileThread::StartThread(LPCTSTR p)
{
	if (handle)	return;	

	if (p == NULL)
	{
		p = TEXT("Unknown Thread");
	}
	lstrcpyn(name, p, MAX_PATH);
	name[MAX_PATH - 1] = TEXT('\0');

	DWORD tid;

	quit_flg  = FALSE;															
	handle = ::CreateThread( NULL, 0,(LPTHREAD_START_ROUTINE)ThreadProc, (LPVOID)this, CREATE_SUSPENDED, &tid);
	::ResumeThread(handle);
}

BOOL WriteFileThread::Join(DWORD msec)
{
	if (!handle) return TRUE;

	switch(::WaitForSingleObject(handle, msec))
	{
	case WAIT_OBJECT_0:
		return TRUE;

	case WAIT_TIMEOUT:
	default:
		return FALSE;
	}
}

BOOL WriteFileThread::StopThread(DWORD msec)
{
	if (!handle)	return TRUE;
	quit_flg = TRUE;
	Notify();

	BOOL ret = Join(msec);
	return ret;
}


inline BOOL WriteFileThread::IsAlive()
{
	return (handle != NULL);
}

inline BOOL WriteFileThread::CheckQuit()
{
	return (quit_flg);
}

inline DWORD WriteFileThread::GetThreadID()
{
	return (thread_id);
}

inline LPCTSTR WriteFileThread::GetName()
{
	return name;
}

void WriteFileThread::Run()
{
}


DWORD ThreadProc (LPVOID lpdwThreadParam );

//Global variable Shared by all threads
int nGlobalCount = 0;
//Main function which starts out each thread
int __cdecl main( int argc, char **argv)
{ 
	int i, nThreads = 5;
	DWORD dwThreadId;
	//Determine the number of threads to start
	if (argc > 1) {
		nThreads = atoi( argv[1]);
	}

	//Set the global count to number of threads
	nGlobalCount = nThreads;
	//Start the threads
	for (i=1; i<= nThreads; i++) {
		//printf("i - %d\n",i);
		if (CreateThread(NULL, //Choose default security
			0, //Default stack size
			(LPTHREAD_START_ROUTINE)&ThreadProc,
			//Routine to execute
			(LPVOID) &i, //Thread parameter
			0, //Immediately run the thread
			&dwThreadId //Thread Id
			) == NULL)
		{
			printf("Error Creating Thread#: %d\n",i);
			return(1);
		}
		else
		{
			printf("Global Thread Count: %d %d %d\n", nGlobalCount, nThreads, i);
			Sleep(1000);
		}
	}
	return 0;
}

//Thread Routine
DWORD ThreadProc (LPVOID lpdwThreadParam ) 
{
	//Print Thread Number
	printf ("Thread #: %d\n", *((int*)lpdwThreadParam));
	//Reduce the count
	nGlobalCount--;
	//ENd of thread
	return 0;
}