#include <Windows.h>
#include <stdio.h>

struct xxx{
	BYTE a1;
	WORD a;
	DWORD x;
	WORD aX;
	BYTE b;
};

int main(int argc, char * argv[])
{

	//xxx x;
	//int i = (BYTE*)&x.b - (BYTE*)&x.a +1;

	//memset(&x.a, 0, i);

	WORD x = 65534;
	WORD y = 2;
	WORD z = 65535 + y - x ;
	printf("%d",z);
	

	return 0;
}