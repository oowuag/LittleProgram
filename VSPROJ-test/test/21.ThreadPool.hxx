#include <windows.h>
#include <Wdm.h>
#include <iostream>
#include <vector>
using namespace std;

/************************************************************************/
/* Test Our own thread pool.                                            */
/************************************************************************/

typedef struct _THREAD_POOL
{
	HANDLE QuitEvent;
	HANDLE WorkItemSemaphore;

	LONG WorkItemCount;
	LIST_ENTRY WorkItemHeader;
	CRITICAL_SECTION WorkItemLock;

	LONG ThreadNum;
	HANDLE *ThreadsArray;

}THREAD_POOL, *PTHREAD_POOL;

typedef VOID (*WORK_ITEM_PROC)(PVOID Param);

typedef struct _WORK_ITEM
{
	LIST_ENTRY List;

	WORK_ITEM_PROC UserProc;
	PVOID UserParam;

}WORK_ITEM, *PWORK_ITEM;


DWORD WINAPI WorkerThread(PVOID pParam)
{
	PTHREAD_POOL pThreadPool = (PTHREAD_POOL)pParam;
	HANDLE Events[2];

	Events[0] = pThreadPool->QuitEvent;
	Events[1] = pThreadPool->WorkItemSemaphore;

	for(;;)
	{
		DWORD dwRet = WaitForMultipleObjects(2, Events, FALSE, INFINITE);

		if(dwRet == WAIT_OBJECT_0)
			break;

		//
		// execute user's proc.
		//

		else if(dwRet == WAIT_OBJECT_0 +1)
		{
			PWORK_ITEM pWorkItem;
			PLIST_ENTRY pList;

			EnterCriticalSection(&pThreadPool->WorkItemLock);
			_ASSERT(!IsListEmpty(&pThreadPool->WorkItemHeader));
			pList = RemoveHeadList(&pThreadPool->WorkItemHeader);
			LeaveCriticalSection(&pThreadPool->WorkItemLock);

			pWorkItem = CONTAINING_RECORD(pList, WORK_ITEM, List);
			pWorkItem->UserProc(pWorkItem->UserParam);

			InterlockedDecrement(&pThreadPool->WorkItemCount);
			free(pWorkItem);
		}

		else
		{
			_ASSERT(0);
			break;
		}
	}

	return 0;
}

BOOL InitializeThreadPool(PTHREAD_POOL pThreadPool, LONG ThreadNum)
{
	pThreadPool->QuitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	pThreadPool->WorkItemSemaphore = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
	pThreadPool->WorkItemCount = 0;
	InitializeListHead(&pThreadPool->WorkItemHeader);
	InitializeCriticalSection(&pThreadPool->WorkItemLock);
	pThreadPool->ThreadNum = ThreadNum;
	pThreadPool->ThreadsArray = (HANDLE*)malloc(sizeof(HANDLE) * ThreadNum);

	for(int i=0; i<ThreadNum; i++)
	{
		pThreadPool->ThreadsArray[i] = CreateThread(NULL, 0, WorkerThread, pThreadPool, 0, NULL);
	}

	return TRUE;
}

VOID DestroyThreadPool(PTHREAD_POOL pThreadPool)
{
	SetEvent(pThreadPool->QuitEvent);

	for(int i=0; i<pThreadPool->ThreadNum; i++)
	{
		WaitForSingleObject(pThreadPool->ThreadsArray[i], INFINITE);
		CloseHandle(pThreadPool->ThreadsArray[i]);
	}

	free(pThreadPool->ThreadsArray);

	CloseHandle(pThreadPool->QuitEvent);
	CloseHandle(pThreadPool->WorkItemSemaphore);
	DeleteCriticalSection(&pThreadPool->WorkItemLock);

	while(!IsListEmpty(&pThreadPool->WorkItemHeader))
	{
		PWORK_ITEM pWorkItem;
		PLIST_ENTRY pList;

		pList = RemoveHeadList(&pThreadPool->WorkItemHeader);
		pWorkItem = CONTAINING_RECORD(pList, WORK_ITEM, List);

		free(pWorkItem);
	}
}

BOOL PostWorkItem(PTHREAD_POOL pThreadPool, WORK_ITEM_PROC UserProc, PVOID UserParam)
{
	PWORK_ITEM pWorkItem = (PWORK_ITEM)malloc(sizeof(WORK_ITEM));
	if(pWorkItem == NULL)
		return FALSE;

	pWorkItem->UserProc = UserProc;
	pWorkItem->UserParam = UserParam;

	EnterCriticalSection(&pThreadPool->WorkItemLock);
	InsertTailList(&pThreadPool->WorkItemHeader, &pWorkItem->List);
	LeaveCriticalSection(&pThreadPool->WorkItemLock);

	InterlockedIncrement(&pThreadPool->WorkItemCount);
	ReleaseSemaphore(pThreadPool->WorkItemSemaphore, 1, NULL);

	return TRUE;
}

VOID UserProc1(PVOID dwParam)
{
	WorkItem(dwParam);
}

void TestSimpleThreadPool(BOOL bWaitMode, LONG ThreadNum)
{
	THREAD_POOL ThreadPool;    
	InitializeThreadPool(&ThreadPool, ThreadNum);

	CompleteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	BeginTime = GetTickCount();
	ItemCount = 20;

	for(int i=0; i<20; i++)
	{
		PostWorkItem(&ThreadPool, UserProc1, (PVOID)bWaitMode);
	}

	WaitForSingleObject(CompleteEvent, INFINITE);
	CloseHandle(CompleteEvent);

	DestroyThreadPool(&ThreadPool);
}
//我们把工作项目放到一个队列中，用一个信号量通知线程池，线程池中任意一个线程取出工作项目来执行，执行完毕之后，线程返回线程池，继续等待新的工作项目。
//线程池中线程的数量是固定的，预先创建好的，永久的线程，直到销毁线程池的时候，这些线程才会被销毁。
//线程池中线程获得工作项目的机会是均等的，随机的，并没有特别的方式保证哪一个线程具有特殊的优先获得工作项目的机会。
//而且，同一时刻可以并发运行的线程数目没有任何限定。事实上，在我们的执行计算任务的演示代码中，所有的线程都并发执行。
//下面，我们再来看一下，完成同样的任务，系统提供的线程池是如何运作的。

/************************************************************************/
/* QueueWorkItem Test.                                                  */
/************************************************************************/

DWORD BeginTime;
LONG  ItemCount;
HANDLE CompleteEvent;

int compute()
{
	srand(BeginTime);

	for(int i=0; i<20 *1000 * 1000; i++)
		rand();

	return rand();
}

DWORD WINAPI WorkItem(LPVOID lpParameter)
{
	BOOL bWaitMode = (BOOL)lpParameter;

	if(bWaitMode)
		Sleep(1000);
	else
		compute();

	if(InterlockedDecrement(&ItemCount) == 0)
	{
		printf("Time total %d second.\n", GetTickCount() - BeginTime);
		SetEvent(CompleteEvent);
	}

	return 0;
}

void TestWorkItem(BOOL bWaitMode, DWORD Flag)
{
	CompleteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	BeginTime = GetTickCount();
	ItemCount = 20;

	for(int i=0; i<20; i++)
	{
		QueueUserWorkItem(WorkItem, (PVOID)bWaitMode, Flag);
	}    

	WaitForSingleObject(CompleteEvent, INFINITE);
	CloseHandle(CompleteEvent);
}


int main()
{
	return 0;
}