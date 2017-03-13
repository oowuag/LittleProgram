#include <iostream>
#include <Windows.h>
using namespace std;

int main()
{
	//DWORD bt = 1;
	//BYTE dw = bt;
	//for (int i=0; i < 10; i++)
	//{
	//	//OutputDebugString("abc");
	//}

	FILE *fp1;
	FILE *fp2;
	fp1 = fopen("C:\\a.txt","w");
	fp2 = fopen("C:\\b.txt","w");

	fprintf(fp1, "%s\n", "aaaaaaaaa");
	fprintf(fp2, "%s\n", "bbbbbbbbb");

	fclose(fp1);
	fclose(fp2);

	return 0;
}