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
//���ǰѹ�����Ŀ�ŵ�һ�������У���һ���ź���֪ͨ�̳߳أ��̳߳�������һ���߳�ȡ��������Ŀ��ִ�У�ִ�����֮���̷߳����̳߳أ������ȴ��µĹ�����Ŀ��
//�̳߳����̵߳������ǹ̶��ģ�Ԥ�ȴ����õģ����õ��̣߳�ֱ�������̳߳ص�ʱ����Щ�̲߳Żᱻ���١�
//�̳߳����̻߳�ù�����Ŀ�Ļ����Ǿ��ȵģ�����ģ���û���ر�ķ�ʽ��֤��һ���߳̾�����������Ȼ�ù�����Ŀ�Ļ��ᡣ
//���ң�ͬһʱ�̿��Բ������е��߳���Ŀû���κ��޶�����ʵ�ϣ������ǵ�ִ�м����������ʾ�����У����е��̶߳�����ִ�С�
//���棬����������һ�£����ͬ��������ϵͳ�ṩ���̳߳�����������ġ�

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