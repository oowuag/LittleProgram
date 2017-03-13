#include <windows.h>
#include <stdio.h>

void main()
{
	SYSTEMTIME st, lt;

	GetSystemTime(&st);
	GetLocalTime(&lt);

	printf("The system time is: %02d:%02d\n", st.wHour, st.wMinute);
	printf(" The local time is: %02d:%02d\n", lt.wHour, lt.wMinute);


	printf(" GetTickCount: %d\n", GetTickCount()/1000);
	
}
