#include <Windows.h>
#include <iostream>
using namespace std;

CRITICAL_SECTION	m_csSnsProc;

int main()
{
	InitializeCriticalSection( &m_csSnsProc );

	while(1)
	{

		EnterCriticalSection( &m_csSnsProc );

		cout << "1" << endl;

		EnterCriticalSection( &m_csSnsProc );

		cout << "2" << endl;

		EnterCriticalSection( &m_csSnsProc );

		cout << "3" << endl;

		static bool bOnce = true;
		if(bOnce)
		{	
			bOnce = false;
			LeaveCriticalSection( &m_csSnsProc );
			continue;
		}

		//LeaveCriticalSection( &m_csSnsProc );
	}

	DeleteCriticalSection( &m_csSnsProc );
}