#include <stdio.h>
#include <Windows.h>

class A{
};

class B{
};

int main()
{
	CHAR szLog[200];
	CHAR szTmp[10];
	memset(szLog, 0, sizeof(szLog));

	sprintf(szLog, "NPGpsHandler:NotifyCanBusInfo:");

	for(int i=0; i<10; i++)
	{
		sprintf(szTmp, "%0x", 5);
		strcat(szLog, szTmp);
	}
	strcat(szLog, "\n");

	A a;
	A* pa;
	B* pb;
	BYTE* pv = &a;
	//pv = static_cast<void*>(pa);
	//pa = static_cast<A*>(pv);

	//pb = static_cast<B*>(pa);


	printf("%*.*s", 10, 10, szLog );

	OutputDebugString(szLog);


}